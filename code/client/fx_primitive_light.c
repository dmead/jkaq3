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

static float flrand(float m1, float m2)
{
	return m1 + (float)rand()/((float)RAND_MAX/(m2-m1));
}

static void vecrandom(const vec3_t v1, const vec3_t v2, vec3_t result)
{
	int i;
	if(!result)
	{
		return;
	}
	for(i = 0; i < 3; i++)
	{
		result[i] = flrand(v1[i], v2[i]);
	}
}

qboolean FX_LightCull(fxPrimitive_t *_self)
{
	return qfalse;
}

void FX_LightUpdateVelocity(fxLight_t *_self)
{
	VectorMA(_self->velocity, cls.frametime * 0.001f, _self->accel, _self->velocity);
}

void FX_LightUpdateOrigin(fxLight_t *_self, float frac)
{
	vec3_t new_origin;
	float ftime, time2;
	int i;

	FX_LightUpdateVelocity(_self);

	ftime = cls.frametime * 0.001f;
	time2 = ftime * ftime * 0.5f;

	for ( i = 0 ; i < 3 ; i++ ) 
		new_origin[i] = _self->origin[i] + ftime * _self->velocity[i] + time2 * _self->velocity[i];

	VectorCopy( new_origin, _self->origin );
}

void FX_LightUpdateSize(fxLight_t *_self, float frac)
{
	_self->size = _self->startsize + _self->endsize * frac;
}

void FX_LightUpdateRGB(fxLight_t *_self, float frac)
{
	if(!VectorCompare(_self->startRGB, _self->endRGB))		// This changes based on time, so do some sort of magic lerping
	{
		// TODO: wave/clamp/nonlinear. BLAH.
		if(_self->RGBflags & FXTLF_NONLINEAR)
		{
			VectorCopy(_self->startRGB, _self->RGB);
		}
		else if(_self->RGBflags & FXTLF_LINEAR)
		{
			_self->RGB[0] = _self->startRGB[0] + frac * _self->endRGB[0];
			_self->RGB[1] = _self->startRGB[1] + frac * _self->endRGB[1];
			_self->RGB[2] = _self->startRGB[2] + frac * _self->endRGB[2];
		}
		else
		{
			VectorCopy(_self->startRGB, _self->RGB);
		}
	}
	else
		VectorCopy(_self->startRGB, _self->RGB);
#if 0
	float invfrac = 1.0f - frac;
	int i;

	for(i = 0; i < 3; i++)
	{
		_self->RGB[i] = Com_Clamp( 0.0f, 1.0f, ( _self->startRGB[i] * invfrac + _self->endRGB[i] * frac ) );
		
		/*
		_self->RGB[i] = ( _self->startRGB[i] * invfrac + _self->endRGB[i] * frac );

		// Has been explicitely flagged to use the alpha channel
		if ( !(m_flags & FXF_USE_ALPHA_CHAN) )
		{
			_self->RGB[i] *= _self->alpha;
		}

		if (_self->RGB[i] < 0.0f)
			_self->RGB[i] = 0.0f;

		if (_self->RGB[i] > 1.0f)
			_self->RGB[i] = 1.0f;
		*/
	}
#endif
}

qboolean FX_LightUpdate(fxPrimitive_t *_self)
{
	fxLight_t *l = (fxLight_t *)_self;
	float frac = (float)(fx_time - l->startTime ) / ( l->endTime - l->startTime );
	FX_LightUpdateOrigin(l, frac);
	FX_LightUpdateSize(l, frac);
	FX_LightUpdateRGB(l, frac);

	return qtrue;
}

qboolean FX_LightDraw(fxPrimitive_t *_self)
{
	fxLight_t *l = (fxLight_t *)_self;
	re.AddLightToScene(l->origin, l->size*5, l->RGB[0], l->RGB[1], l->RGB[2]);
	return qtrue;
}

void FX_LightInit(fxPrimitive_t *_self)
{
	fxLight_t *l = (fxLight_t *)_self;
	Com_Memset(l, 0, sizeof(fxLight_t));
	l->Update = FX_LightUpdate;
	l->Cull = FX_LightCull;
	l->Draw = FX_LightDraw;
}

void FX_CreateLight(const FXSegment_t *segment, vec3_t origin)
{
	FXLightSegment_t *light;
	fxLight_t *effect;

	if(!segment)
	{	// NULL segment pointer?
		return;
	}
	if(!segment->SegmentData.FXLightSegment)
	{	// No particle segment data?
		return;
	}
	if(segment->segmentType != EFXS_LIGHT)
	{	// Not a particle?
		return;
	}
	light = segment->SegmentData.FXLightSegment;
	effect = (fxLight_t *)malloc(sizeof(fxLight_t));
	FX_LightInit((fxPrimitive_t *)effect);
	effect->culldist = flrand(light->cullrange[0], light->cullrange[1]);
	effect->culldist *= effect->culldist; // allows for VLSquared
	effect->startTime = fx_time + Q_irand(light->delay[0], light->delay[1]);
	effect->endTime = effect->startTime + Q_irand(light->life[0], light->life[1]);
	VectorCopy(origin, effect->origin);

	if((segment->spawnflags & FXSFLAG_CHEAPORIGINCALC))
	{
		vec3_t offset;
		vecrandom(light->origin[0], light->origin[1], offset);
		VectorAdd(effect->origin, offset, effect->origin);
	}
	vecrandom(light->rgb.start.sv[0], light->rgb.start.sv[1], effect->startRGB);

	if(light->rgb.flags != 0 || !(light->rgb.flags & FXTLF_CONSTANT))
	{
		vecrandom(light->rgb.end.ev[0], light->rgb.end.ev[1], effect->endRGB);
	}
	else
	{
		VectorCopy(effect->startRGB, effect->endRGB);
	}
	//effect->RGBflags = light->rgb.flags;
	VectorCopy(effect->startRGB, effect->RGB);

	effect->startsize = effect->size = flrand(light->size.start.sf[0], light->size.start.sf[1]);

	if(light->size.flags != 0 || !(light->size.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		effect->endsize = flrand(light->size.end.ef[0], light->size.end.ef[1]);
	}
	else
	{
		effect->endsize = effect->startsize;
	}
	//effect->sizeFlags = light->size.flags;

	FX_AddPrimitive((fxPrimitive_t *)effect, effect->startTime, effect->endTime);
}
