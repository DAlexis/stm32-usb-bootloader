#!/bin/bash

set -e

echo "Usage: ./build.sh <debug|release>. Debug is default"

cfg="Debug"
build_dir_prefix="./build"
build_dir=$build_dir_prefix"/debug"
source_dir_from_build_dir="../../"

if [ "$1" == "release" ];
then
    cfg="Release"
    build_dir=$build_dir_prefix"/release"
fi

mkdir -p $build_dir
(
    cd $build_dir && cmake -DCMAKE_BUILD_TYPE=$cfg $source_dir_from_build_dir && make -j `nproc`
)
