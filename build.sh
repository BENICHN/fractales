#!/bin/bash

CC=g++
IMGUI_DIR="imgui-1.87"
SOURCES="gl$1.cpp"
SOURCES="$SOURCES $IMGUI_DIR/imgui.cpp $IMGUI_DIR/imgui_demo.cpp $IMGUI_DIR/imgui_draw.cpp $IMGUI_DIR/imgui_tables.cpp $IMGUI_DIR/imgui_widgets.cpp $IMGUI_DIR/backends/imgui_impl_opengl3.cpp $IMGUI_DIR/backends/imgui_impl_glut.cpp"
CPPFLAGS="-O2"
LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"
INCLUDEFLAGS="-I$IMGUI_DIR -I$IMGUI_DIR/backends"

$CC $SOURCES $CPPFLAGS $INCLUDEFLAGS $LDFLAGS -o gl$1
