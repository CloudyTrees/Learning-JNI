#!/bin/bash
u1="argument          meaning                             possible values"
u2="1                 generate static or dynamic lib      [s/d]"
u3="2                 C implementation file"
u4="3                 output library name w/o extension   \"lib\" automatically prefixed if dynamic"
u5="4                 path to jni.h"

prefix=$(echo $2 | awk -F'.c' '{print $1}')

if [ -z ${4+x} ]; then 
    jnilibpath=/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers
fi

if [ "$1" == "d" ]; then
    gcc $2 \
        -fpic \
        -shared \
        -I $jnilibpath \
        -o lib$3.dylib
elif [ "$1" == "s" ]; then
    gcc -c $2 \
        -fpic \
        -I $jnilibpath \
        -o $prefix.o
    ar rc lib$3.a $prefix.o
    rm $prefix.o
else
    echo "Usage:"
    echo "$u1"
    echo "$u2"
    echo "$u3"
    echo "$u4"
    echo "$u5"
fi
