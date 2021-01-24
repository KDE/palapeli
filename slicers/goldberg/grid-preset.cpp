/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grid.h"

void PresetMode::generateGrid(GoldbergEngine *e, int piece_count) const {
	// mogrify the settings according to the preset.
	switch (e->m_quickpreset) {
		// case 0: use standard settings for everything.
		case 1: // very regular
			e->m_flip_threshold = 0;
			e->m_sigma_curviness = 0.07;
			e->m_sigma_basepos = 0.04;
			e->m_sigma_plugs = 0.10;
			break;
		case 2: // very diverse
			e->m_flip_threshold = 40;
			e->m_sigma_basepos = 0.8;
			e->m_sigma_plugs = 0.8;
			break;
		case 3: // large plugs
			e->m_plug_size = 1.25;
			// tweak the other settings a bit to reduce collision probability
			e->m_edge_curviness += 20;
			e->m_sigma_basepos = 0.08;
		default:
			break;
	}
	// ... and pass through to another generator.
	RectMode().generateGrid(e, piece_count);
}

