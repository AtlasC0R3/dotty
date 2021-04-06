#!/bin/bash
echo Building program using make on platform $1
make PLATFORM=$1

if [ -d "bin/resources/" ]; then
  rm -rf bin/resources/
fi
cp -r resources/ bin/resources/