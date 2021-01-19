#!/bin/bash
#
# This script will create a folder structure containing PlatformIO
# compatible projects for each example under examples-pio.
#   Each project will utilize the same .ini found as prototype in this
#   folder.
# The output of this script can be added to the workspace of Visual Studio Code.
#
conf=`pwd`
examplespio="examples-pio"
target=`pwd`"/../${examplespio}"
test -e ${target} || mkdir ${target}

cd ../examples
for example in IotWebConf*; do
  if [ ! -e "$target/$example" ]; then
    mkdir "$target/$example"
    mkdir "$target/$example/src"
    mkdir "$target/$example/lib"
    if [ -e "$example/pio/platformio.ini" ]; then
      cp "$example/pio/platformio.ini" "$target/$example/platformio.ini"
    else
      cp "$conf/platformio-template.ini" "$target/$example/platformio.ini"
    fi
    ln -s "../../../examples/$example/$example.ino" "$target/$example/src/main.cpp"
    ln -s "../../.." "$target/$example/lib/IotWebConf"
    echo "		{"
    echo "			\"path\": \"${examplespio}/$example\""
    echo "		},"
  fi
done