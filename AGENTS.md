# Project Context

This fork of PufferLib is being used to build a puffer-native Four Rooms environment inspired by MiniGrid's FourRooms environment.

## Branches

- `dev` is the active project branch. Use it for local development, README/project polish, macOS/MPS experiments, and ongoing environment work.
- `four-rooms-4` is the PR branch. Keep it focused on upstreamable Four Rooms environment changes only. Do not bring README/project branding or macOS-specific build changes onto this branch unless explicitly requested.

## Four Rooms Environment

The Four Rooms code lives mainly in:

- `ocean/four_rooms/four_rooms.h`
- `ocean/four_rooms/binding.c`
- `config/four_rooms.ini`

Recent behavior changes:

- Episodes end only on timeout, not when the goal is reached.
- Reaching the goal gives reward and immediately respawns the goal at a new empty cell.
- Episode length is randomized between configured `min_steps` and `max_steps`.
- `score` tracks goals reached per episode.
- `perf` is normalized episode return.
- `episode_return` is the raw accumulated shaped reward.
- The agent is tracked by `agent_x`/`agent_y` and is no longer duplicated into the terrain grid.

Keep future env changes small and easy to cherry-pick between `dev` and `four-rooms-4`.

## Four Rooms Web Demo

The personal website demo is built from the standalone C entrypoint:

- `ocean/four_rooms/four_rooms.c`
- `resources/four_rooms/four_rooms_weights.bin`

The website copy lives in:

- `/Users/paulmerceur/Code/website/demos/four-rooms/`

The demo uses a raw PufferNet weight file, not the PyTorch `.bin` checkpoint directly. To refresh weights from the current same-architecture torch checkpoint, export only the matrices in PufferNet order:

1. `encoder.encoder.weight`
2. `decoder.decoder.weight` plus `decoder.value_function.weight` concatenated on axis 0
3. `network.layers.0.weight`
4. `network.layers.1.weight`

The current web build command uses the Homebrew Emscripten install with explicit paths:

```bash
mkdir -p build/web/four_rooms-lite
EMSDK_PYTHON=/opt/homebrew/bin/python3 \
EM_LLVM_ROOT=/opt/homebrew/Cellar/emscripten/6.0.2/libexec/llvm/bin \
EM_BINARYEN_ROOT=/opt/homebrew/Cellar/emscripten/6.0.2/libexec/binaryen \
EM_NODE_JS=/usr/local/bin/node \
emcc -o build/web/four_rooms-lite/game.html ocean/four_rooms/four_rooms.c \
  -O3 -Wall raylib-5.5_webassembly/lib/libraylib.a \
  -I./raylib-5.5_webassembly/include -I./src -I./vendor \
  -L. -L./raylib-5.5_webassembly/lib \
  -sASSERTIONS=2 -gsource-map -sUSE_GLFW=3 -sUSE_WEBGL2=1 \
  -sASYNCIFY -sFILESYSTEM -sFORCE_FILESYSTEM=1 \
  --shell-file vendor/minshell.html \
  -sINITIAL_MEMORY=64MB -sSTACK_SIZE=512KB \
  -DNDEBUG -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES3 \
  --preload-file resources/four_rooms@resources/four_rooms \
  --preload-file resources/shared/puffers_128.png@resources/shared/puffers_128.png
```

After building, copy the generated files into the website:

```bash
cp build/web/four_rooms-lite/game.html \
   build/web/four_rooms-lite/game.js \
   build/web/four_rooms-lite/game.wasm \
   build/web/four_rooms-lite/game.data \
   build/web/four_rooms-lite/game.wasm.map \
   /Users/paulmerceur/Code/website/demos/four-rooms/
```

Keep the copied `game.html` as the tiny local wrapper used in the website repo, not the generated raylib shell with external metadata/scripts. If `four_rooms.c`, build flags, or assets change, copy the rebuilt files and bump the iframe/cache query in the website `index.html`. If only the same-architecture weights change, rebuilding and copying `game.data` is usually enough.

Publish website changes from `/Users/paulmerceur/Code/website` with:

```bash
./publish.sh "message"
```
