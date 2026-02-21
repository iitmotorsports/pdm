#! /usr/bin/env bash

cleanup() {
  echo "Shutting down CAN server"
  sudo ip link set slcan0 down
  sudo ip link delete slcan0
  sudo pkill slcand
}

trap cleanup EXIT

PORT=$(ls /dev/serial/by-id/ | grep -i ftdi)
if [ -z "$PORT" ]; then
  echo "CANdapter not found!"
  exit 1
fi
PORT="/dev/serial/by-id/$PORT"
DEVICE=$(basename $(readlink -f $PORT))

echo "Found CANdapter $DEVICE"

sudo slcan_attach -f -s4 -o "$PORT"
sudo slcand "$DEVICE" slcan0
sudo ip link set slcan0 up
candump slcan0
sudo ip link set slcan0 down
sudo slcan_attach -c "$PORT"
