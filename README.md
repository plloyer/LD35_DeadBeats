# Deadbeats (LD35)

Unreal Engine 4 multiplayer arena game made for Ludum Dare 35 (April 2016).

- **Project:** `LD35/LD35.uproject` (C++ module in `LD35/Source/LD35`)
- **Game modes:** Arena and Capture Point. The C++ code includes AI controllers, projectiles, deflectors and traps.
- **Raw assets:** `Raw/` (source art, Git LFS)
- **Launch scripts:** `LD35/Start*.bat` (server, listen server, client, main menu)

## Setup

1. Install Git LFS, then clone: `git lfs install && git clone https://github.com/plloyer/LD35_DeadBeats.git`
2. The `.uproject` is tied to a source build of UE4 (engine GUID). To use another UE4 version, right-click it and pick *Switch Unreal Engine version*.
3. Generate the Visual Studio project files and build `LD35Editor`.
