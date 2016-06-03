#!/bin/bash

set -e

echo "Usage: ./generate-eclipse-project.sh <debug|release>. Debug is default"

workspace="eclipse-workspace"
project="usb-bootloader"
dir=$workspace/$project

mkdir -p $dir

cfg="Debug"
if [ "$1" == "release" ];
then
    cfg="Release"
    build_dir=$build_dir_prefix"/release"
fi

(
cd $dir
cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=$cfg ../../bootloader-usb

# Patching definitions for C++11 support
sed -i s/199711L/201103L/ .cproject
)

echo "Unpacking archive with metadata.tar.gz with .metadata directory if presented"
echo "Error here is normal if you have not metadata.tar.gz with eclipse settings"

tar -xvf metadata.tar.gz
mv .metadata $workspace
echo "done."

