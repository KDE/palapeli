#!/bin/sh

for MANIFEST in *.desktop; do
	TARGET=$(basename $MANIFEST .desktop).pala
	libpala-puzzlebuilder $MANIFEST $TARGET
done
