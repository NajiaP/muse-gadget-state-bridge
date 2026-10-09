# Muse Gadget State Bridge

A deliberately simple state bridge that lets an AI agent drive the display of Meta's [Muse Gadgets](https://github.com/facebookincubator/muse-gadget-sdk) simulator: the agent writes a small plain-text state file, a watcher notices the change, and the stock simulator relaunches as a borderless desktop pet showing the agent's current face, caption, and progress ring.

Built as part of the **Muse Gadgets Progress Companion** case study — a daily progress companion for tracking multi-thread product work. 📖 Full case study (with an interactive prototype): https://muse.ai/s/muse-gadgets-progress-companion-ss5x0yikxrxxxl4

## Why this exists

The public Muse Gadgets SDK has **no external progress trigger**. In the stock simulator, faces switch by keyboard (F1–F7, H), and the progress ring is conversation-phase progress driven by the firmware voice pipeline — there is no supported way for an outside program to say "show *thinking*, 50% done."

The simulator does, however, accept a `--scenario` file at launch. The state bridge turns that into a transport:

```
agent writes ~/gadget-state.txt  →  watcher polls mtime every 2 s
                                 →  on change, relaunch simulator with --scenario
                                 →  pet flips face / caption / ring (~1 s flicker)
```

## The state file protocol

The state file uses the simulator's scenario grammar — five keys, one per line:

| Key | Values | Meaning |
|---|---|---|
| `face=` | `boot` \| `idle` \| `listening` \| `thinking` \| `speaking` \| `error` \| `off` \| `happy` | Expression shown on the pet |
| `caption=` | Latin text only (the stock pixel font has no CJK) | Status line |
| `progress=` | `0..1` | The progress ring — note the key is `progress`, **not** `ring` |
| `wifi=` | `off` \| `no_network` \| `connecting` \| `connected` \| `failed` \| `not_nearby` | Connectivity state |
| `battery=` | `-1..100` | Battery level |

Example (`gadget-state.example.txt`, the file from the first live check-in):

```
face=happy
caption=Day 1 logged - docs at 50%
progress=0.5
wifi=connected
battery=100
```

> ⚠️ **An unknown or invalid key is fatal** — the simulator exits at startup with no visible error. This bridge's first live test failed exactly this way (an initial state file used `ring=0` instead of `progress=0`), and the failure was diagnosed from the watcher log and fixed by correcting the key. Treat the grammar as strict.

## How the watcher works

`gadget-pet-watcher.sh` is the entire transport:

- Polls `~/gadget-state.txt` every 2 seconds (macOS `stat -f %m`).
- On an mtime change, it kills the running simulator and relaunches it with `MUSE_SIM_PET=1` and `--scenario ~/gadget-state.txt`, logging to `~/gadget-pet.log`.
- `gadget-bridge-start.sh` starts the watcher once (killing any previous instance); `gadget-bridge-stop.sh` stops watcher + simulator.

The split is by design: the agent can only write the state file (data). The watcher is installed and launched by the user — the agent proposes state, the user's own machine commits it.

## Contents

| File | Role |
|---|---|
| `sim_board.c` | Patched simulator board file — a drop-in replacement for `esp32/simulator/src/sim_board.c` in the Muse Gadget SDK. Adds three env-controlled presentation modes, all off/neutral by default: `MUSE_SIM_ZOOM=<0.5–8>` explicit scale; auto-fit zoom to ~92% of the current display when no explicit zoom is set; `MUSE_SIM_PET=1` borderless, always-on-top, 0.55× zoom at the main display's bottom-right corner; `MUSE_SIM_FULLSCREEN=1`. The gadget UI always renders at device resolution (412×412); only window presentation changes. |
| `gadget-pet-watcher.sh` | The bridge transport (see above). |
| `gadget-bridge-start.sh` | Starts the watcher. Run: `bash ~/gadget-bridge-start.sh` |
| `gadget-bridge-stop.sh` | Stops watcher + simulator. |
| `gadget-state.example.txt` | Example state file in the scenario format. |
| `README.md` | This file. |

## Reproduce

1. Clone [facebookincubator/muse-gadget-sdk](https://github.com/facebookincubator/muse-gadget-sdk); replace `esp32/simulator/src/sim_board.c` with this repo's file; build the simulator per the SDK README (CMake; macOS build verified).
2. Copy the three `gadget-*.sh` scripts to `$HOME`; adjust the simulator path in the watcher if yours differs; write a state file from the example to `~/gadget-state.txt`.
3. `bash ~/gadget-bridge-start.sh` — the pet appears bottom-right. Edit `~/gadget-state.txt`; within ~2 s the pet relaunches with the new face / caption / ring.

## Known limitations

- **n = 1**: one user, one machine (Apple Silicon Mac, macOS 15.5), verified live in nightly use since 2026-10-09.
- **Crude by intention**: 2 s polling + full relaunch ≈ 1 s of flicker per state change. This is a mechanism proof, not a product channel; on real hardware, the same state vocabulary is meant to travel over the SDK's documented custom-command route.
- **Self-reported state**: the agent writes both the work status and its display. Progress shown is agent-reported and unverified by design — the companion displays check-ins, not telemetry.
- **No voice**: the simulator has no microphone path or paired session; voice round-trips require physical hardware.
- **Strict grammar**: unknown keys are fatal at startup (see the warning above).

## License note

`sim_board.c` is a derivative of Meta's Muse Gadget SDK sample code (Apache-2.0). The Jollybot avatar assets are Meta brand assets, are **not** covered by that license, and are not included in this repo. The bridge scripts and this README are provided as-is for reference.
