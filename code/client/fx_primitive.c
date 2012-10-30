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

int			num_fx = 0;

fxState_t	fx_renderList[ MAX_PLAYING_EFFECTS ];
fxState_t	*fx_nextValid = &fx_renderList[0];

void FX_FreeEffect(fxState_t *state)
{
	num_fx--;

	free(state->effect);
	state->effect = NULL;
}

fxState_t *FX_FindNextValid( void )
{
	int i;
	for (i = 0; i < MAX_PLAYING_EFFECTS; i++)
	{
		fx_nextValid = ( ( fx_nextValid + 1 ) > &fx_renderList[ MAX_PLAYING_EFFECTS - 1 ] ) ? &fx_renderList[ 0 ] : fx_nextValid + 1;

		if ( fx_nextValid->effect == NULL )
			return fx_nextValid;
	}

	return NULL;
}

void FX_AddPrimitive(fxPrimitive_t *primitive, int starttime, int killtime)
{
	if ( ( fx_nextValid == NULL ) || ( fx_nextValid->effect ) )
	{
		fx_nextValid = &fx_renderList[ 0 ];
		FX_FreeEffect( fx_nextValid );
	}

	//If this effect is free, take it over
	if ( fx_nextValid->effect == NULL )
	{
		fx_nextValid->effect   = primitive;
		fx_nextValid->startTime = starttime;
		fx_nextValid->killTime = killtime;

		fx_nextValid = FX_FindNextValid();

		num_fx++;
	}
}

void FX_Scheduler_AddEffects(qboolean skybox)
{
	fxState_t *state;
	int i;

	for (i = 0; i < MAX_PLAYING_EFFECTS; i++)
	{
		if ( fx_renderList[ i ].effect == NULL )
			continue;

		state = &fx_renderList[ i ];

		if ( state->startTime > fx_time )
		{
			continue;
		}

		if ( state->killTime != -1 && (state->killTime < fx_time ) )
		{
			FX_FreeEffect( state );
			continue;
		}

		if( state->effect->Update(state->effect) == qfalse )
		{
			FX_FreeEffect( state );
			continue;
		}

		if( state->effect->Cull(state->effect) == qfalse )
		{
			state->effect->Draw(state->effect);
		}
	}
}
