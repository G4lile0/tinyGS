#!/bin/bash
#
# This script will build all targets in pre-created PlatformIO
# compatible folder structure.
# Build will stop on any error.
#
cd ../examples-pio || exit 1

for example in IotWebConf*; do
  echo "Compiling ${example}"
  pio run --project-dir ${example} || exit $?
done