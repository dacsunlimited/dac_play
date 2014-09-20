#!/bin/bash -xe
cd $WORKSPACE/bitshares_play
git submodule init
git submodule update
mkdir $WORKSPACE/build
cd $WORKSPACE/build
export BITSHARES_ROOT=$WORKSPACE
. ../bitshares_play/setenv.sh
cmake -DINCLUDE_QT_WALLET=OFF -DCMAKE_TOOLCHAIN_FILE=$WORKSPACE/toolchain.invictus/toolchain.invictus.cmake ../bitshares_play
make -j8
