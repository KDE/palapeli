#!/bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp` rc.cpp -o $podir/palapeli.pot
rm -f rc.cpp
