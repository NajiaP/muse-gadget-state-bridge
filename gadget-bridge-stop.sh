#!/bin/bash
# Stop the Muse Gadget pet state bridge (watcher + pet simulator).
pkill -f gadget-pet-watcher 2>/dev/null
pkill -x muse_simulator 2>/dev/null
echo "BRIDGE_STOPPED"
