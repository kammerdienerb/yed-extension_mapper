#!/usr/bin/env bash
gcc -o extension_mapper.so extension_mapper.c $(yed --print-cflags --print-ldflags)
