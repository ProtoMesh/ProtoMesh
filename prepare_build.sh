#!/usr/bin/env bash
ORIGIN=${PWD}

echo "Setting up submodules..."
git submodule init
git submodule update

echo "Concatenating ArduinoJson..."

cd deviceLib/json/ArduinoJson
chmod +x scripts/build-single-header.sh
./scripts/build-single-header.sh
cd ..
mv ArduinoJson-*.hpp ArduinoJson.hpp
rm -f ArduinoJson-*.h
rm -f smoketest.o