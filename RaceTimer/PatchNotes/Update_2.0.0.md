Rewrite for multiplayer + dedicated server support by Robb. (No changes from prerelease)



----

If you enjoy my work, please consider my [completely optional tip jar](https://ko-fi.com/robb4). - Robb

## New Stuff

- Multiplayer and dedicated server support
  - Each player has their own independently running race timer, started/stopped when they cross start/finish lines.
  - If server hosts want to disable the ability to leave races, add `RaceTimer:ClearRaceTimer` to `disabledChatCommands` in [SML's configuration](https://docs.ficsit.app/satisfactory-modding/latest/SMLConfiguration.html#_sml_configuration_options).
- Completing a race now launches fireworks at the finish line
- Previously undocumented 'instant factory cart unfolding' feature is now tied to a [Mod Savegame Setting](https://docs.ficsit.app/satisfactory-modding/latest/ForUsers/ConfiguringMods.html#_mod_savegame_settings) (on by default)

## Changed Stuff

- Race time is tracked with greater precision
- `/ClearRaceTimer` command now responds with a chat message for feedback

## Fixed Stuff

- Fixed that factory carts parked inside of race start/end points couldn't be interacted with (their collision was blocked by the building)
