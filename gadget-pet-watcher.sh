#!/bin/bash
# Muse Gadget pet state bridge watcher.
# Watches ~/gadget-state.txt; whenever the file changes, relaunches the
# pet simulator with the new scenario so face/caption/ring follow Muse's work.
STATE="$HOME/gadget-state.txt"
SIM="$HOME/muse-gadget-sdk/esp32/simulator/build/muse_simulator"
LOG="$HOME/gadget-pet.log"

launch_pet() {
  pkill -x muse_simulator 2>/dev/null
  sleep 0.3
  MUSE_SIM_PET=1 nohup "$SIM" --scenario "$STATE" >>"$LOG" 2>&1 &
}

last_mtime=""
if [ -f "$STATE" ]; then
  last_mtime="$(stat -f %m "$STATE" 2>/dev/null)"
fi
launch_pet

while true; do
  if [ -f "$STATE" ]; then
    mtime="$(stat -f %m "$STATE" 2>/dev/null)"
    if [ -n "$mtime" ] && [ "$mtime" != "$last_mtime" ]; then
      last_mtime="$mtime"
      launch_pet
    fi
  fi
  sleep 2
done
