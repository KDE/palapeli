#!/bin/sh

for MANIFEST in *.conf; do
	[ "$MANIFEST" = "default-collection.conf" ] && continue
	TARGET=$(basename $MANIFEST .conf).puzzle
	echo "Creating $TARGET"
	libpala-puzzlebuilder $MANIFEST $TARGET
done
