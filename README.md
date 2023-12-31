# minesweeper
Simple implementation of minesweeper using [raylib](https://www.raylib.com/)

## Controls
- Left click hidden tile to reveal
- Right click hidden tile to to set/unset flag

There is a debug mode which reveals all bombs upon pressing `R`, however it is only available when compiling with `#define CHEAT_MODE true`.

## Compile
Compile game using

```bash
$ make
```

and run using
```bash
$ ./minesweeper
```

Raylib must already have been built in advance ([instructions](https://github.com/raysan5/raylib/wiki)) and made available at a proper path (shared object file `.so` and `raylib.h` header), or installed through something like `apt-get` and `pacman`.

If you relocate the game executable, remember to also relocate the assets in `assets/` (must be in the shell's working directory).

## Credits/Attributions
Main credits to raylib. Additionally, the following assets have been sourced online:
- Bomb: https://www.vecteezy.com/vector-art/22276377-a-bomb-in-pixel-art-style
- Flagpole: https://www.flaticon.com/free-icon/achievement_4737015

![screenshot](screenshot.png)
