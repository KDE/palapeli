#!/bin/sh
$EXTRACTRC `find . -name \*.ui -o -name \*.rc` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h` rc.cpp -o $podir/palapeli.pot
rm -f rc.cpp
