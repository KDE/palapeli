#!/bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h` rc.cpp -o $podir/palapeli.pot
rm -f rc.cpp
