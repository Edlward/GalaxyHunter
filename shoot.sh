#!/bin/bash

SERIAL_DEVICE="/dev/ttyUSB0"
if [ -b "$1" ]; then
  SERIAL_DEVICE="$1"
  shift
fi

if [ "$1" != "0" ] && [ "$1" != "1" ]; then
  echo "Usage: $0 [device] <opening-delay> <exposure_time> <shots> <delay_between_shots>"
  exit 1
fi

# 1 0: DTR (shoot)
# 0 0: Clear DTR
# 0 1: RTS (stop shooting)
# 1 1: Clear RTS

cleanup() {
  ./set-serial-signal "$SERIAL_DEVICE" 0 0
  ./set-serial-signal "$SERIAL_DEVICE" 1 1
}


trap cleanup SIGINT SIGTERM

for i in $( seq $3 ); do
  ./set-serial-signal /dev/ttyUSB0 0 0
  sleep "$1" && "./set-serial-signal"  "$SERIAL_DEVICE" 0 1
  sleep "0.3" && "./set-serial-signal" "$SERIAL_DEVICE" 0 0
  sleep "$2" && "./set-serial-signal"  "$SERIAL_DEVICE" 1 1
  sleep "$4"
done
