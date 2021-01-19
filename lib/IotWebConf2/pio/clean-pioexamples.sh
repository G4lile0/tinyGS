#!/bin/bash
#
# This script will remove created PlatformIO compatible folder structure.
# Files are mostly references to original files, and the original files
#   will not be deleted. So generaly said it safe to run this script.
#   platformio.ini files will be removed alog with any additonal added
#   files.
#
cd ../examples-pio || exit 1

for example in IotWebConf*; do
  echo "Removing pio example ${example}"
  rm -rf ${example} || exit $?
done