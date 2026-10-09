#!/bin/bash
# Start the Muse Gadget pet state bridge (run once).
# Kills any previous watcher, starts a fresh one in the background.
pkill -f gadget-pet-watcher 2>/dev/null
sleep 0.3
nohup bash "$HOME/gadget-pet-watcher.sh" > "$HOME/gadget-pet-watcher.log" 2>&1 &
echo "BRIDGE_OK - pet watcher running; update ~/gadget-state.txt to change the pet. Stop with: bash ~/gadget-bridge-stop.sh"
