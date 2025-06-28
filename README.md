
# GCB - Game Core Bind

GCB is a background service that optimizes CPU resource allocation for games on AMD Ryzen X3D CPUs by forcing the game processes to execute only on specific CPU cores of the X3D CCD.

## Why GCB?

Even with the latest AMD drivers, the Windows scheduler often distributes game threads across both CCDs, even when this results in lower FPS. This affects CPUs like the Ryzen 9 7990X3D, 7950X3D, 9900X3D, and 9950X3D, which combine X3D and non-X3D CCDs.

While most threads run on the X3D CCD when the AMD driver is properly installed, some threads still get scheduled to the non-X3D CCD, resulting in lower performance.

GCB resolves this by binding the game processes to the X3D CCD, skipping SMT threads if desired.

Binding games to the X3D CCD can also lower power consumption, often by about 30 watts compared to no binding.

## Example - Cyberpunk 2077 on a Ryzen 9 9950X3D (non-GPU-limited scenario with RX 9070 XT)

-   **Only X3D CCD enabled in BIOS, SMT disabled:** ~330 FPS
-   **Only X3D CCD enabled in BIOS, SMT enabled:** ~281 FPS
-   **Default (AMD Driver installed):** ~250 FPS
-   **Manual Binding to CCD1 (non-X3D):** ~235 FPS
-   **Manual Binding to CCD0 (X3D):** ~290 FPS
-   **Manual Binding to CCD0 (X3D), SMT threads skipped:** ~305 FPS
 
## Features

-   Per-game core binding to specific CCDs
-   Option to skip SMT threads
-   Automatic detection of running games
-   Automatic detection of game foreground/background state
-   Disabling of desktop effects during gameplay (optional)
-   Automatic disabling of secondary monitors during gameplay (optional)
-   UDP messaging for custom router or peripheral integrations
-   Lua scripting support for full configuration and customization
-   `games.lua` to define game profiles and binding behavior   
-   `config.lua` for global options and preferences
    
### Lua Files

`config.lua` Global configuration options, should be adjusted to your liking:

```
Config = {
  HideConsole = true,
  DisableDesktopEffects = true,
  DisableNonPrimaryDisplays = true,
  SetCpuAffinity = true
}
```

`gcb.lua` Contains the core functionality exposed to Lua. You usually don't need to modify this.

`` Add your games and define per-game behavior. Already contains a broad range of games.

```
Games = {
  ["Cyberpunk 2077"] = {
    Binary = "Cyberpunk2077.exe",
    ["Core-Binding"] = { Mode = "X3D", SMT = false }
  },
  ["Quake Champions"] = {
    Binary = "QuakeChampions.exe",
    ["Core-Binding"] = { Mode = "NON-X3D" }
  }
}
```

`custom.example.lua` Example file demonstrating how to extend the tool. Must be renamed to `custom.lua` to take effect.

## Requirements

-   AMD Ryzen X3D CPU with dual CCDs (e.g., 7990X3D, 7950X3D, 9900X3D, 9950X3D)
-   Windows 11 (Windows 10 should work too)  

Linux support may follow soon.  
The source code compiles on Linux, but there is a lot of stuff that's not implemented.

## License

GPLv3
