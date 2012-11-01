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

qboolean fx_init = qfalse;
refdef_t *fx_refDef = NULL;
int fx_time = 0;

cvar_t	*fx_countScale;
cvar_t	*fx_nearCull;
cvar_t	*fx_debug;

void FX_Init(void)
{
	fx_countScale = Cvar_Get("fx_countScale", "1", CVAR_ARCHIVE);
	fx_nearCull = Cvar_Get("fx_nearCull", "16", CVAR_ARCHIVE);
	Cvar_CheckRange( fx_nearCull, 0, MAX_FX_CULL, qtrue );
	fx_debug = Cvar_Get("fx_debug", "0", 0);

	// Set up memory for files
	FX_InitFileMemory(1024);
	fx_refDef = NULL;
	fx_init = qtrue;
}

void FX_Shutdown(void)
{
	// Cleanup
	FX_CleanupFileMemory();
	fx_init = qfalse;
}

void FX_AdjustTime(int fxTime)
{
	fx_time = fxTime;
}

void FX_SystemInit(refdef_t *rd)
{
	// Assign the refdef pointer
	fx_refDef = rd;
	fx_nextValid = &fx_renderList[0];
}

void FX_SystemShutdown(void)
{
	int i;

	for (i = 0; i < MAX_PLAYING_EFFECTS; i++)
	{
		if ( fx_renderList[ i ].effect == NULL )
			continue;

		FX_FreeEffect( &fx_renderList[i] );
	}
	fx_refDef = NULL;
}

void FX_Play(fxFile_t *fx, vec3_t origin, vec3_t dir)
{
	int i;
	// Assumes pointer is valid
	for(i = 0; i < fx->numSegments ; i++)
	{
		switch(fx->segments[i].segmentType)
		{
			case EFXS_CAMERASHAKE:
				break;
			case EFXS_CYLINDER:
				break;
			case EFXS_DECAL:
				break;
			case EFXS_ELECTRICITY:
				break;
			case EFXS_EMITTER:
				break;
			case EFXS_FLASH:
				break;
			case EFXS_FXRUNNER:
				break;
			case EFXS_LIGHT:
				FX_CreateLight(&fx->segments[i], origin);
				//CFxPrimitive_CreateLightPrimitive(&fx->segments[i], origin);
				break;
			case EFXS_LINE:
				break;
			case EFXS_ORIENTEDPARTICLE:
				break;
			case EFXS_PARTICLE:
				FX_CreateParticle(&fx->segments[i], origin, dir);
				//CFxPrimitive_CreateParticlePrimitive(&fx->segments[i], origin, dir);
				break;
			case EFXS_SOUND:
				FX_CreateSound(&fx->segments[i], origin);
				//fixme we should just play the sound now instead of pushing it as a playing
				//particle
				//CFxPrimitive_CreateSoundPrimitive(&fx->segments[i], origin);
				break;
			case EFXS_TAIL:
				break;
		}
	}
}

void FX_PlayEffectID(fxHandle_t handle, vec3_t origin, vec3_t dir)
{
	fxFile_t *fx;
	if(!fx_init)
		return;

	fx = FX_GetByHandle(handle);
	if(!fx)
		return;

	FX_Play( fx, origin, dir );
}

void FX_PlayEffect( const char *file, vec3_t origin, vec3_t fwd )
{
	fxFile_t *fx;
	if(!fx_init)
		return;

	fx = FX_FindEffect(file);
	if(!fx)
		return;

	FX_Play( fx, origin, fwd );
}