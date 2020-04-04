#!/bin/sh
file_list=(
    structs.h
    main.cpp
    sqlite_routines.cpp
    sqlite_routines.h
    )
clang-format -style=file -i $file_list

