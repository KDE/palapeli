/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "components.h"

Palapeli::CopyComponent::CopyComponent(Palapeli::Puzzle* puzzle)
	: m_puzzle(puzzle)
{
}

Palapeli::PuzzleComponent* Palapeli::CopyComponent::cast(Type type) const
{
	//this casts only data
	if (type != Metadata && type != Contents && type != CreationContext)
		return 0;
	//get component from other puzzle
	const Palapeli::PuzzleComponent* cmp = m_puzzle->get(type);
	switch (type)
	{
		case Metadata:
			return new Palapeli::MetadataComponent(dynamic_cast<const Palapeli::MetadataComponent*>(cmp)->metadata);
		case Contents:
			return new Palapeli::ContentsComponent(dynamic_cast<const Palapeli::ContentsComponent*>(cmp)->contents);
		case CreationContext:
			return new Palapeli::CreationContextComponent(dynamic_cast<const Palapeli::CreationContextComponent*>(cmp)->creationContext);
		//the following just to suppress warnings
		default:
			return 0;
	}
}
