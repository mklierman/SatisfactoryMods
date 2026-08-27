# HypertubeDestinations project plan

## 1. Project goal

Allow every vanilla Hypertube Entrance to act as a named station. A player interacts
with an entrance, chooses another reachable station, enters the hypertube normally,
and is routed automatically through all intervening Hypertube Junctions.

The routing system must be:

- deterministic;
- server authoritative;
- independent for every player;
- safe when the network changes during travel;
- persistent across save/load; and
- compatible with listen servers and dedicated servers.

Bare open tube ends are outside the initial scope. A selectable destination must
have a Hypertube Entrance.

## 2. MVP definition

The first releasable version is complete when all of the following are true:

- Entrances can be named through an interaction UI.
- Names survive save/load and replicate to clients.
- The destination UI shows reachable named entrances in the same network.
- Selecting a destination creates a server-side route.
- The player is automatically routed through one or more junctions.
- Two players can take different routes through the same junction concurrently.
- Reversal, dismantling, invalid routes, and disconnects fall back safely.
- Listen-server and dedicated-server test cases pass.

Favorites, route previews, icons, permissions, and estimated travel time are
post-MVP unless they become necessary for usability.

## 3. Current implementation status

### Native foundation

- [x] `FHypertubeDestinationRecord` save/replication data.
- [x] `FHypertubeRouteDecision` and `FHypertubeRoutePlan` data.
- [x] Saved and replicated `AHypertubeDestinationSubsystem`.
- [x] Entrance registration, renaming, unavailability, and permanent removal APIs.
- [x] Server-side `UHypertubeDestinationRCO` rename/select RPCs.
- [x] Proximity and input validation for current RPCs.
- [x] BFS traversal through `IFGPipeHyperInterface`.
- [x] Junction decisions recorded as incoming and outgoing connection components.
- [x] Per-player active route storage.
- [x] Native hook targeting
  `AFGBuildablePipeHyperJunction::GetConnectionToTransitThrough`.
- [x] Reversal/stale-route fallback in the junction exit handler.
- [ ] Full clean compile after the latest fixes.
- [ ] Packaged-game runtime verification of the native hook.

### Unreal assets

- [ ] Game Instance Module asset.
- [ ] Game World Module asset.
- [ ] Hypertube Entrance Actor Mixin.
- [ ] Entrance interaction/use-state implementation.
- [ ] Destination selector widget.
- [ ] Rename widget/dialog.
- [ ] UI style, icons, and localization.

## 4. Intended architecture

### Authority boundaries

The server owns:

- authoritative destination records;
- destination names;
- network traversal and path calculation;
- active route plans;
- topology revision;
- route invalidation; and
- the final junction output override.

Clients own:

- widget presentation;
- local search, filtering, and sorting;
- input focus and menu lifecycle; and
- non-authoritative route status presentation.

Client requests that modify state must pass through
`UHypertubeDestinationRCO`.

### Runtime flow

```text
Entrance mixin Begin Play
    -> server registers AFGPipeHyperStart
    -> destination record replicates

Player interacts with entrance
    -> open destination widget
    -> read replicated destination summaries
    -> ask server for/select reachable destination

Server receives selection
    -> validate source and destination
    -> traverse IFGPipeHyperInterface graph
    -> build ordered junction decisions
    -> store route against AFGCharacterPlayer

Player reaches a junction exit
    -> native hook obtains vanilla exit
    -> subsystem checks the player's next decision
    -> override only that player's outgoing connection
    -> advance route cursor

Route completes or becomes invalid
    -> clear transient route state
    -> preserve safe vanilla behavior
```

## 5. Phase 1: establish a clean native build

### Tasks

- [ ] Build `FactoryGameEditor Win64 Development`.
- [ ] Resolve all UHT errors.
- [ ] Resolve all unity-build collisions.
- [ ] Resolve incomplete-type and include-order errors.
- [ ] Build a non-editor FactoryGame target or package through Alpakit so the
  `#if !WITH_EDITOR` junction hook is compiled.
- [ ] Confirm the plugin loads without startup warnings.
- [x] Add a dedicated log category such as `HypertubeDestinations_Log`.
- [x] Add startup logs for hook registration and subsystem availability.

### Acceptance criteria

- Editor target builds with zero HypertubeDestinations errors.
- Packaged target builds with the junction hook included.
- Game log reports successful module load and hook installation.
- No native hook is installed more than once during module lifetime.

## 6. Phase 2: register SML modules and networking

### Create the Game World Module

- [ ] Create `RootGameWorld_HypertubeDestinations` derived from
  `UGameWorldModule`.
- [ ] Add `AHypertubeDestinationSubsystem` to `Mod Subsystems`.
- [ ] Confirm its replication policy is `SpawnOnServer_Replicate`.
- [ ] Register the Game World Module with the mod's root game feature data.

### Create the Game Instance Module

- [ ] Create `RootGameInstance_HypertubeDestinations` derived from
  `UGameInstanceModule`.
- [ ] Add `UHypertubeDestinationRCO` to `Remote Call Objects`.
- [ ] Later add the entrance Actor Mixin to `Blueprint Hooks`.
- [ ] Register the Game Instance Module with the mod's root game feature data.

### Verification

- [ ] In standalone, confirm one subsystem exists after a save loads.
- [ ] On a client, confirm the replicated subsystem becomes available.
- [ ] Confirm `GetRemoteCallObjectOfClass<UHypertubeDestinationRCO>()` returns a
  valid object for every connected player.
- [ ] Confirm dedicated server startup does not load client-only widget assets.

### Acceptance criteria

- Exactly one authoritative destination subsystem exists per game world.
- Client and server can execute a harmless test RPC.
- Late-joining clients receive the current destination array.

## 7. Phase 3: implement the Hypertube Entrance Actor Mixin

### Target

Target the concrete vanilla Blueprint actor:

```text
/Game/FactoryGame/Buildable/Factory/PipeHyperStart/Build_PipeHyperStart
```

The C++ parent remains `AFGPipeHyperStart`.

### Lifecycle wiring

- [ ] Create `Mixin_HypertubeDestination_PipeHyperStart`.
- [ ] On Begin Play, retrieve the mixin target and cast it to
  `AFGPipeHyperStart`.
- [ ] Retrieve `AHypertubeDestinationSubsystem`.
- [ ] On authority, call `RegisterEntrance`.
- [ ] If the subsystem is not available yet, retry on the next tick or use the
  subsystem availability mechanism instead of silently failing.
- [ ] On End Play, call `MarkEntranceUnavailable`.
- [x] Hook native `AFGBuildable::Dismantle_Implementation`, filter to
  `AFGPipeHyperStart`, and call `RemoveEntrance` before the entrance is destroyed.
  Actor Mixins cannot hook this native implementation from Blueprint, and their
  Blueprint End Play event does not expose `EEndPlayReason`.
- [ ] Do not enable mixin Tick after initialization is reliable.

### Identity/save behavior

- [ ] Verify a loaded entrance maps back to its original saved record.
- [ ] Verify dismantling permanently removes the record.
- [ ] Verify world partition streaming does not permanently delete the record.
- [ ] Test Blueprint Designer copies for duplicate IDs or reused actor references.
- [ ] Add explicit duplicate-record repair if Blueprint Designer testing reveals
  identity collisions.

### Acceptance criteria

- Building an entrance creates exactly one record.
- Streaming out/in does not create duplicates.
- Dismantling removes the destination.
- Save/load preserves the same name and destination ID.

## 8. Phase 4: add entrance interaction

Investigate the vanilla interaction interfaces already implemented by or usable on
`AFGPipeHyperStart`. Prefer the normal Satisfactory interaction prompt and menu
lifecycle over a global keybind.

### Tasks

- [ ] Identify the appropriate use/interact interface or vanilla hook point.
- [ ] Add an interaction prompt such as `Configure Destination`.
- [ ] Require the player to be within interaction range.
- [ ] Prevent opening duplicate widget instances.
- [ ] Close the widget when the entrance is dismantled, player dies, player moves
  too far away, or world travel begins.
- [ ] Preserve normal hypertube entry collision and behavior.
- [ ] Verify the interaction does not consume the player's normal ability to enter
  the tube.

### Acceptance criteria

- The prompt appears only at valid entrances.
- Interacting opens one widget owned by the correct player controller.
- Closing the widget restores input mode and cursor state.
- Normal entrance operation remains unchanged when no route is selected.

## 9. Phase 5: build a diagnostic UI before the polished UI

Create a minimal developer widget first. This reduces the amount of UI work needed
to validate routing.

### Diagnostic widget features

- [ ] Show the source entrance actor name and saved display name.
- [ ] List all replicated destination records.
- [ ] Rename the source through `Server_SetDestinationName`.
- [ ] Select a destination through `Server_SelectDestination`.
- [ ] Show success/failure text returned by the server.
- [ ] Add a clear-route action.
- [ ] Display route decision count for debugging.

### Required native additions

- [ ] Add an explicit RPC/result path for selection success or failure.
- [ ] Define a route-selection result enum, for example:
  - Success
  - InvalidSource
  - InvalidDestination
  - SameDestination
  - SourceNotRegistered
  - DestinationNotRegistered
  - NotConnected
  - TooFarFromSource
  - SubsystemUnavailable
- [ ] Add a server RPC to clear the active route.
- [ ] Avoid using reliable RPCs for rapidly repeated UI refresh operations.

### Acceptance criteria

- Rename and selection work in standalone and as a remote client.
- Invalid selection produces a useful result instead of silently doing nothing.
- The diagnostic widget is sufficient to execute the routing test matrix.

## 10. Phase 6: validate and harden graph traversal

### Current algorithm

The planner performs BFS over connection states:

1. Begin at `Source->mConnection0->GetConnection()`.
2. Treat that connection as the connection through which the next hypertube actor
   was entered.
3. Call `IFGPipeHyperInterface::GetPossibleConnectionsToTransitionThrough`.
4. Follow each output's physical `GetConnection()`.
5. Record junction transitions as incoming/output component pairs.
6. Stop when the destination entrance connection is reached.

### Correctness tasks

- [ ] Test a single straight tube.
- [ ] Test a single junction with all three source/destination combinations.
- [ ] Test chained junctions.
- [ ] Test loops without infinite traversal.
- [ ] Test multiple equal-length paths.
- [ ] Test boosters.
- [ ] Test passthrough hypertubes.
- [ ] Test disconnected and partially dismantled networks.
- [ ] Test bidirectional routing between the same stations.
- [ ] Confirm `GetPossibleConnectionsToTransitionThrough` returns connections in
  a form compatible with the stored decision model.
- [ ] Confirm junction restrictions that depend on `AFGCharacterPlayer` are
  respected.

### Cost model

- [ ] Decide whether MVP routes by fewest pieces, fewest junctions, or shortest
  travel distance.
- [ ] If using distance, replace plain BFS with Dijkstra and use the returned float
  edge cost.
- [ ] Establish deterministic tie-breaking so the same topology yields the same
  route across runs.

Recommended MVP behavior: use shortest estimated distance with deterministic
connection-name tie-breaking. Retain BFS only if the returned distance values are
not reliable across all hypertube actor types.

### Safety limits

- [ ] Add a maximum visited-node limit.
- [ ] Add a maximum route-decision limit.
- [ ] Log and fail safely if malformed or modded networks exceed limits.
- [ ] Avoid holding strong references to unrelated graph actors after planning.

### Acceptance criteria

- All supported network shapes return the expected route.
- Loops terminate.
- Unreachable destinations fail without changing player state.
- Planning time remains imperceptible on a representative large network.

## 11. Phase 7: validate native junction routing

### Hook behavior

The hook calls vanilla `GetConnectionToTransitThrough` first. It overrides the
result only when the player has a matching next route decision.

### Tasks

- [ ] Log junction, player, vanilla exit, planned incoming connection, planned
  outgoing connection, and route cursor in development builds.
- [ ] Confirm the hook runs on server authority.
- [ ] Confirm the hook may also run during client prediction and document the
  observed call pattern.
- [ ] Ensure repeated calls for one junction do not advance the cursor twice.
- [ ] Confirm `VanillaExit == IncomingConnection` reliably identifies reversal.
- [ ] If it does not, incorporate `FFGPipeHyperJunctionPipeData::TravelRouteIndex`,
  distance direction, or the current junction path metadata.
- [ ] Clear routes when the expected junction does not match the actual junction.
- [ ] Clear routes on player death, disconnect, forced hypertube exit, and arrival.
- [ ] Preserve vanilla behavior whenever route data is missing or invalid.

### Concurrency test

- [ ] Create two destinations through one shared junction.
- [ ] Have two players select different destinations.
- [ ] Enter the shared junction at nearly the same time.
- [ ] Verify each player receives their own output with no mutation of shared
  junction state.

### Acceptance criteria

- One player follows a multi-junction route without pressing `E`.
- Manual reversal cancels automation safely.
- Simultaneous players can take conflicting paths independently.
- A player without a route experiences unmodified vanilla behavior.

## 12. Phase 8: topology tracking and route invalidation

The current topology revision changes only when entrance lifecycle APIs are called.
Expand it to cover the complete hypertube network.

### Relevant actors

- `AFGBuildablePipeHyper`
- `AFGBuildablePipeHyperJunction`
- `AFGBuildablePipeHyperAttachment`
- `AFGBuildablePipeHyperBooster`
- `AFGBuildablePassthroughPipeHyper`
- `AFGPipeHyperStart`

### Tasks

- [ ] Hook construction/Begin Play where needed.
- [ ] Hook `Dismantle_Implementation` for relevant buildables.
- [ ] Observe `UFGPipeConnectionComponentBase::SetConnection` and
  `ClearConnection` if actor lifecycle events are insufficient.
- [ ] Increment topology revision only on authority.
- [ ] Invalidate affected active routes.
- [ ] Initially clear all graph caches and active routes for correctness.
- [ ] Later narrow invalidation to the affected connected component if profiling
  shows a need.
- [ ] Notify affected players that their route was canceled.

### Acceptance criteria

- Adding or removing a tube/junction prevents use of stale route decisions.
- Dismantling a destination removes it from clients.
- Changing an unrelated network does not crash travelers.
- Invalidated travelers fall back to vanilla behavior or eject normally.

## 13. Phase 9: route lifecycle and multiplayer replication

### Server lifecycle

- [ ] Store active routes only for valid connected players.
- [ ] Clear route state on logout/disconnect.
- [ ] Clear route state on death or respawn.
- [ ] Clear route state on successful arrival.
- [ ] Clear route state on manual cancellation.
- [ ] Add route-selection rate limiting.

### Owning-client state

The server remains authoritative, but the owning client needs enough state for UI
feedback and possibly movement prediction.

- [ ] Add an owner-only replicated route summary or client RPC containing:
  - destination ID/name;
  - status;
  - remaining junction count; and
  - cancellation reason.
- [ ] Do not replicate the complete world graph.
- [ ] Do not expose other players' active routes.
- [ ] Determine whether client prediction requires the ordered connection plan.
- [ ] If prediction corrections are visible, replicate a compact owner-only route
  plan or hook the same decision on the owning client.

### Dedicated server

- [ ] Ensure widget classes are not loaded on dedicated server.
- [ ] Package the correct dedicated-server binaries.
- [ ] Test late join, reconnect, save/load, and server restart.

### Acceptance criteria

- Server route state cannot be spoofed by the client.
- Route UI accurately reflects server state.
- Dedicated-server clients route correctly without visible correction spikes.

## 14. Phase 10: polished destination UI

### Main selector

- [ ] Current station heading.
- [ ] Search field with immediate local filtering.
- [ ] Alphabetical sorting.
- [ ] Reachable destinations only by default.
- [ ] Optional disabled/unreachable section for diagnostics.
- [ ] Destination name.
- [ ] Approximate distance or junction count.
- [ ] Select/cancel buttons.
- [ ] Loading and empty states.
- [ ] Keyboard/controller navigation.
- [ ] Clear selected destination feedback.

### Rename flow

- [ ] Rename button for the current entrance.
- [ ] Maximum 64-character validation mirrored from the server.
- [ ] Trim leading/trailing whitespace.
- [ ] Reject empty names.
- [ ] Decide whether duplicate names are allowed.
- [ ] If duplicates are allowed, show a disambiguator such as distance or short ID.
- [ ] Display server rejection feedback.

### Visual integration

- [ ] Follow Satisfactory fonts, colors, spacing, and button behavior.
- [ ] Create localized text rather than hard-coded widget strings.
- [ ] Add a small route-active HUD indicator only if it materially improves clarity.
- [ ] Avoid a permanent HUD element for players not using the mod.

### Acceptance criteria

- The widget is usable with mouse/keyboard and controller.
- Lists remain responsive with hundreds of destinations.
- All state changes are confirmed by the server.
- UI closes and restores input reliably.

## 15. Phase 11: persistence and migration hardening

### Tasks

- [ ] Add an explicit mod save-data version.
- [ ] Define migration behavior before changing record layout.
- [ ] Sanitize null/stale entrance records after load.
- [ ] Detect duplicate destination GUIDs.
- [ ] Repair records after Blueprint Designer copies if needed.
- [ ] Confirm `GatherDependencies` does not create save dependency cycles.
- [ ] Confirm unloading an entrance does not erase its record.
- [ ] Test loading a save after the mod is removed and later reinstalled.
- [ ] Decide how names behave when an entrance is dismantled and rebuilt in the
  same location; MVP may treat it as a new station.

### Acceptance criteria

- Names survive normal save/load and server restart.
- Corrupt or stale records are skipped without crashing.
- Future record changes have a documented migration path.

## 16. Phase 12: configuration and permissions

These are optional for MVP unless multiplayer testing demonstrates a need.

- [ ] Session setting: who may rename destinations.
- [ ] Session setting: allow duplicate names.
- [ ] Session setting: routing cost mode.
- [ ] Session setting: manual `E` behavior during an automated route.
- [ ] Session setting: maximum destination count.
- [ ] Optional per-client config for UI sorting and presentation.

Possible rename permission modes:

- anyone near the entrance;
- host/server administrators only; or
- original builder/owner where ownership information is available and reliable.

## 17. Test matrix

### Topology

- [ ] Straight tube, two entrances.
- [ ] One junction, every input/output combination.
- [ ] Three chained junctions.
- [ ] Branching tree.
- [ ] Closed loop.
- [ ] Multiple equal-cost paths.
- [ ] Booster in route.
- [ ] Passthrough in route.
- [ ] Disconnected destination.
- [ ] Destination dismantled before entry.
- [ ] Junction dismantled during travel.
- [ ] Tube dismantled during travel.

### Player behavior

- [ ] Enter immediately after selection.
- [ ] Delay before entering.
- [ ] Enter the source from an unexpected side/state.
- [ ] Reverse before the first junction.
- [ ] Reverse inside a junction.
- [ ] Press `E` during automation.
- [ ] Exit early.
- [ ] Die during travel.
- [ ] Disconnect during travel.
- [ ] Select a second route before using the first.

### Multiplayer

- [ ] Standalone.
- [ ] Listen server host.
- [ ] Listen server remote client.
- [ ] Dedicated server with one client.
- [ ] Dedicated server with multiple clients.
- [ ] Conflicting simultaneous junction routes.
- [ ] Late join after stations have been named.
- [ ] Client reconnect.
- [ ] Server save/restart.

### Scale/performance

- [ ] 50 destinations.
- [ ] 250 destinations.
- [ ] Large connected network with loops.
- [ ] Repeated widget opening.
- [ ] Repeated route calculations.
- [ ] Confirm no per-frame whole-network scans.

## 18. Diagnostics

- [x] Add `HypertubeDestinations_Log`.
- [ ] Add verbose logs for graph traversal behind a config/debug flag.
- [ ] Log route selection with source, destination, player, decision count, and
  topology revision.
- [ ] Log route cancellation reasons.
- [ ] Add a console command to dump registered destinations.
- [ ] Add a console command to dump a player's active route.
- [ ] Add an optional world debug draw for the selected route in development
  builds.
- [ ] Avoid logging every movement step in normal builds.

## 19. Packaging and release

- [ ] Move or junction the mod source into a version-controlled location such as
  `C:\Modding\SatisfactoryMods\HypertubeDestinations` because `Mods/*` is ignored
  by the SML repository.
- [ ] Add a dedicated Git repository or tracked folder.
- [ ] Update `.uplugin` description, category, author URLs, and semantic version.
- [ ] Add license files.
- [ ] Create final 128px icon and repository images.
- [ ] Package client and dedicated-server targets through Alpakit.
- [ ] Test installation through Satisfactory Mod Manager.
- [ ] Write user documentation:
  - naming a station;
  - selecting a destination;
  - cancellation/manual override behavior;
  - multiplayer permissions; and
  - known limitations.
- [ ] Prepare changelog and issue-report template.

### Release gates

- No crash in the topology and multiplayer matrices.
- No global junction state mutation.
- No stale route can force a player through a removed connection.
- Save/load and dedicated-server restart pass.
- Packaged binaries match the current Satisfactory/SML versions.

## 20. Recommended execution order

Follow this order to minimize rework:

1. Finish clean editor and packaged builds.
2. Register the subsystem and RCO module assets.
3. Implement the entrance mixin lifecycle.
4. Add diagnostic interaction/UI.
5. Validate graph traversal on representative networks.
6. Validate the native junction hook with one player.
7. Validate simultaneous players through one junction.
8. Add topology invalidation and route lifecycle cleanup.
9. Add server result messages and client route status.
10. Build the polished destination UI.
11. Harden persistence, configuration, and performance.
12. Complete the test matrix and package the release.

Do not invest heavily in final UI styling until the one-player and two-player
junction-routing gates both pass in a packaged game.
