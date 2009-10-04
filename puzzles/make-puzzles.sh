#!/bin/sh

for MANIFEST in *.desktop; do
	TARGET=$(basename $MANIFEST .desktop).puzzle
	echo "Creating $TARGET"
	libpala-puzzlebuilder $MANIFEST $TARGET
done
