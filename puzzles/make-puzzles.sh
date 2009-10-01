#!/bin/sh

for MANIFEST in *.desktop; do
	TARGET=$(basename $MANIFEST .desktop).pala
	echo "Creating $TARGET"
	libpala-puzzlebuilder $MANIFEST $TARGET
done
