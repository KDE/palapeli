#!/bin/sh

for MANIFEST in *.conf; do
	TARGET=$(basename $MANIFEST .conf).puzzle
	echo "Creating $TARGET"
	libpala-puzzlebuilder $MANIFEST $TARGET
done
