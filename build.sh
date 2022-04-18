#!/bin/bash

CC=g++
LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC glPendule.cpp $CPPFLAGS $LDFLAGS -o glPendule
