#!/bin/sh
set -v
export BUILDDIR=`pwd`
ln -sfn "$BUILDDIR/delayloop" /usr/local/bin/delayloop
