# Goat Simulator — Old 3DS Homebrew Port

A lightweight, top-down "Goat Simulator"-style chaos game built for Old 3DS / 2DS
using **citro2d** (2D only, by design — keeps a rock-solid framerate on original
hardware instead of straining its GPU with 3D).

## Gameplay
- **Circle Pad** — move the goat
- **A** — headbutt dash (launch nearby props into chaos)
- **START** — quit
- Ram objects around the level before the 60-second timer runs out. Dashing
  into props scores more than just bumping them, and chaining hits builds a
  combo multiplier (up to x5).
- Bottom screen shows controls; top screen is the play area.

## Requirements to build
You need [devkitPro](https://devkitpro.org/wiki/Getting_Started) with the 3DS
toolchain installed:

```bash
# via devkitPro pacman (after installing devkitPro's package manager)
sudo dkp-pacman -S 3ds-dev
```

That pulls in `devkitARM`, `libctru`, and `citro2d`/`citro3d`, which this
project depends on.

Make sure your environment has:
```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
```
(Add these to your `.bashrc`/`.zshrc` if the installer didn't already.)

## Building (.3dsx — for Homebrew Launcher)
From this project folder:
```bash
make
```
This produces `goat3ds.3dsx` (and a `.smdh` icon file) in the project root.

## Building (.cia — installable app)
A `.cia` also needs `bannertool` and `makerom`, which come from devkitPro's
**general-tools** package:
```bash
sudo dkp-pacman -S general-tools
```
Then, from this project folder:
```bash
make cia
```
This produces `goat3ds.cia`. Install it with any CIA installer
(e.g. [FBI](https://github.com/Steveice10/FBI)) by copying it to your SD card
and installing from there, or over the network with `3dslink`/QR install
depending on what FBI supports on your firmware.

A placeholder icon and banner are included under `meta/` (`icon.png`,
`banner.png`, `banner.wav`) — swap those for real art any time, they're just
plain PNG/WAV files.

**Heads up:** I generated this project's `.cia` build path (the RSF template,
icon/banner pipeline, and `makerom` invocation) without being able to
actually run it — this sandbox has no devkitARM/makerom toolchain or network
access, so I can't compile or verify it end-to-end. It follows the standard
devkitPro pattern, but if `make cia` throws an RSF parsing error, compare
`meta/app.rsf` against devkitPro's official CIA example
(https://github.com/devkitPro/3ds-examples) — that's the authoritative
reference if anything here doesn't quite match your toolchain version.

## Running on real Old 3DS hardware
1. Put `goat3ds.3dsx` in `/3ds/` on your SD card.
2. Launch it from the **Homebrew Launcher** (via whatever entrypoint your
   console uses — e.g. a save-file exploit for your firmware version).

## Running on Citra / other emulators
Most 3DS emulators can load `.3dsx` files directly, or you can convert to
`.cia` with `3dslink`/`makerom` if you want an installable format — not
included here to keep the build simple, but devkitPro's `general-tools`
package covers it if you want that later.

## Notes on this port
- Deliberately 2D top-down rather than full 3D: Old 3DS's GPU is modest, and
  citro2d keeps this locked at 60fps without any asset pipeline (no models,
  textures, or animation rigs needed — it's all drawn with primitives).
- Physics is a simple impulse/friction model — goat and props are circles,
  collisions apply knockback, props bounce off each other and screen edges.
- No sound yet. If you want SFX, `ndsp`/`opusdec` or simple PCM playback via
  `csnd` can be added — ask and I can extend this.
- Easy tuning: `MAX_OBJECTS`, `GAME_TIME`, dash strength (`220.0f`), and max
  speeds are all just constants near the top of `source/main.c`.
