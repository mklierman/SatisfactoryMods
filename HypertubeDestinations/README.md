# HypertubeDestinations development notes

## Native foundation

The C++ layer currently provides:

- a saved and replicated registry of named `AFGPipeHyperStart` actors;
- client-to-server RPCs for renaming an entrance and selecting a destination;
- breadth-first traversal through actors implementing `IFGPipeHyperInterface`;
- ordered, per-player junction decisions; and
- a native hook for `AFGBuildablePipeHyperJunction::GetConnectionToTransitThrough` in packaged builds.

The registry deliberately distinguishes streaming from dismantling. Call
`MarkEntranceUnavailable` from the mixin's End Play event. The native module hooks
`AFGBuildable::Dismantle_Implementation`, filters to `AFGPipeHyperStart`, and calls
`RemoveEntrance` before vanilla dismantling continues.

## Required Unreal assets

Create and configure these assets in the editor:

1. A Game World Module with `AHypertubeDestinationSubsystem` in `Mod Subsystems`.
2. A Game Instance Module with `UHypertubeDestinationRCO` in `Remote Call Objects`.
3. An Actor Mixin targeting
   `/Game/FactoryGame/Buildable/Factory/PipeHyperStart/Build_PipeHyperStart`.

Register the Game World Module, Game Instance Module, and Actor Mixin from the
HypertubeDestinations root game feature data/module assets as appropriate.

## Entrance mixin wiring

The entrance mixin should remain a thin adapter:

- **Begin Play:** get `AHypertubeDestinationSubsystem`, then call `RegisterEntrance`
  with the mixin target cast to `AFGPipeHyperStart`.
- **End Play:** call `MarkEntranceUnavailable`.
- **Interaction:** open the destination widget and pass it the entrance actor.

Do not store the authoritative destination name on the mixin. The subsystem owns
the saved and replicated record.

## Widget/RPC wiring

The widget reads `GetAvailableDestinations` from the local replicated subsystem.
Mutations must go through the owning player's `UHypertubeDestinationRCO`:

- `Server_SetDestinationName(SourceEntrance, Name)`
- `Server_SelectDestination(SourceEntrance, DestinationEntrance)`

The server validates proximity to the source entrance and calculates the route.

## Current limitations

- Only endpoints with a Hypertube Entrance can be named.
- Topology revision updates are currently tied to entrance removal/unavailability;
  tube and junction construction/dismantle hooks still need to be added.
- The active route is server-only. Owning-client prediction and route-status UI are
  follow-up work.
- Name uniqueness and player permissions are not yet enforced.
