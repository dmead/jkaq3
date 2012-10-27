/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "client.h"
#include "fx_local.h"

void FX_CreateSound(const FXSegment_t *segment, vec3_t origin)
{
	FXSoundSegment_t *sfx;
	int i;
	int count;

	if(!segment)
	{	// NULL segment pointer?
		return;
	}
	if(!segment->SegmentData.FXSoundSegment)
	{	// No sound segment data?
		return;
	}
	if(segment->segmentType != EFXS_SOUND)
	{	// Not a sound?
		return;
	}
	sfx = segment->SegmentData.FXSoundSegment;

	count = Q_irand(sfx->count[0], sfx->count[1]);
	for(i = 0; i < count; i++)
	{
		S_StartSound( origin, -1, CHAN_AUTO, sfx->sound.fieldHandles[Q_irand(0, sfx->sound.numFields-1)] );
	}
}
