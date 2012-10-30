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

qboolean FX_ParticleCull(fxPrimitive_t *_self)
{
	return qfalse;
}

void FX_ParticleUpdateVelocity(fxParticle_t *_self)
{
	VectorMA(_self->velocity, cls.frametime * 0.001f, _self->accel, _self->velocity);
}

void FX_ParticleUpdateOrigin(fxParticle_t *_self, float frac)
{
	vec3_t new_origin;
	float ftime, time2;
	int i;

	FX_ParticleUpdateVelocity(_self);

	ftime = cls.frametime * 0.001f;
	time2 = ftime * ftime * 0.5f;

	for ( i = 0 ; i < 3 ; i++ ) 
		new_origin[i] = _self->origin[i] + ftime * _self->velocity[i] + time2 * _self->velocity[i];

	VectorCopy( new_origin, _self->origin );

	//VectorAdd(_self->origin, _self->velocity, _self->origin);
}

void FX_ParticleUpdateRotation(fxParticle_t *_self, float frac)
{
	_self->rotation = _self->startrotation + frac * (_self->startrotation+_self->deltarotation);
}

void FX_ParticleUpdateSize(fxParticle_t *_self, float frac)
{
	_self->size = _self->startsize + _self->endsize * frac;
}

void FX_ParticleUpdateRGB(fxParticle_t *_self, float frac)
{
//	int i;

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
	for(i = 0; i < 3; i++)
	{
		// Has been explicitely flagged to use the alpha channel
		if(!(_self->flags & (1<<FXFLAG_USEALPHA)))
		{
			_self->RGB[i] *= _self->alpha;
		}

		if (_self->RGB[i] < 0.0f)
			_self->RGB[i] = 0.0f;

		if (_self->RGB[i] > 1.0f)
			_self->RGB[i] = 1.0f;
	}
#endif
}

void FX_ParticleUpdateAlpha(fxParticle_t *_self, float frac)
{
	_self->alpha = _self->startalpha + frac * _self->endalpha;
}

qboolean FX_ParticleUpdate(fxPrimitive_t *_self)
{
	fxParticle_t *p = (fxParticle_t *)_self;
	float frac = (float)(fx_time - p->startTime ) / ( p->endTime - p->startTime );
	FX_ParticleUpdateOrigin(p, frac);
	FX_ParticleUpdateRotation(p, frac);
	FX_ParticleUpdateSize(p, frac);
	FX_ParticleUpdateAlpha(p, frac);
	FX_ParticleUpdateRGB(p, frac);

	return qtrue;
}

static const	vec3_t	quad_template[] = 
{
	{	-1.0f, -1.0f, 0.0f	},
	{	-1.0f,  1.0f, 0.0f	},
	{	 1.0f,  1.0f, 0.0f	},
	{	 1.0f, -1.0f, 0.0f	}
};

static const	float	quad_st_template[][2] = 
{
	{   0.0f,  0.0f	},
	{   0.0f,  1.0f	},
	{   1.0f,  1.0f	},
	{   1.0f,  0.0f	}
};

qboolean FX_ParticleDraw(fxPrimitive_t *_self)
{
	fxParticle_t *p = (fxParticle_t *)_self;
	polyVert_t verts[4];
	vec3_t axis[3];
	int i;
	float scale;

	scale = p->size * 2.0f; // ef1 is 0.5f, but you have 2.0f

	for(i = 0; i < 3; i++)
	{
		VectorCopy(fx_refDef->viewaxis[i], axis[i]);
	}

	if(p->rotation)
		RotateAroundDirection(axis, p->rotation);

	for(i = 0; i < 4; i++)
	{
		// Loop through each vert in the quad
		VectorMA( p->origin,	quad_template[i][0] * scale, axis[1], verts[i].xyz );
		VectorMA( verts[i].xyz, quad_template[i][1] * scale, axis[2], verts[i].xyz );

		//Setup the UVs
		verts[i].st[0] = quad_st_template[i][0];
		verts[i].st[1] = quad_st_template[i][1];

		//Setup the vertex modulation
		verts[i].modulate[0] = (byte)((p->RGB[0] * 255));
		verts[i].modulate[1] = (byte)((p->RGB[1] * 255));
		verts[i].modulate[2] = (byte)((p->RGB[2] * 255));

		// TODO: Use alpha chan? (copy from Elite Forces SDK?)
		// FIXME: NO EFFECT WTF
		if((p->flags & (1<<FXFLAG_USEALPHA)))
		{
			verts[i].modulate[3] = (byte)(p->alpha * 255);
		}
		else
		{
			verts[i].modulate[3] = 255;
		}
	}

	re.AddPolyToScene(p->shader, 4, verts, 1);
	return qtrue;
}

void FX_ParticleInit(fxPrimitive_t *_self)
{
	fxParticle_t *p = (fxParticle_t *)_self;
	Com_Memset(p, 0, sizeof(fxParticle_t));
	p->Update = FX_ParticleUpdate;
	p->Cull = FX_ParticleCull;
	p->Draw = FX_ParticleDraw;
}

void FX_CreateParticle(const FXSegment_t *segment, vec3_t origin, vec3_t dir)
{
	FXParticleSegment_t *particle;
	fxParticle_t *effect;

	if(!segment)
	{	// NULL segment pointer?
		return;
	}
	if(!segment->SegmentData.FXParticleSegment)
	{	// No particle segment data?
		return;
	}
	if(segment->segmentType != EFXS_PARTICLE)
	{	// Not a particle?
		return;
	}
	particle = segment->SegmentData.FXParticleSegment;
	effect = (fxParticle_t *)malloc(sizeof(fxParticle_t));
	FX_ParticleInit((fxPrimitive_t *)effect);
	effect->culldist = flrand(particle->cullrange[0], particle->cullrange[1]);
	effect->culldist *= effect->culldist; // allows for VLSquared
	effect->startTime = fx_time + Q_irand(particle->delay[0], particle->delay[1]);
	effect->endTime = effect->startTime + Q_irand(particle->life[0], particle->life[1]);
	effect->flags = segment->flags;
	VectorCopy(origin, effect->origin);

	//if((segment->spawnflags & FXSFLAG_CHEAPORIGINCALC))
	{
		vec3_t offset;
		vecrandom(particle->origin[0], particle->origin[1], offset);
		VectorAdd(effect->origin, offset, effect->origin);
	}
	vecrandom(particle->rgb.start.sv[0], particle->rgb.start.sv[1], effect->startRGB);

	if(particle->rgb.flags != 0 || !(particle->rgb.flags & FXTLF_CONSTANT))
	{
		vecrandom(particle->rgb.end.ev[0], particle->rgb.end.ev[1], effect->endRGB);
	}
	else
	{
		VectorCopy(effect->startRGB, effect->endRGB);
	}
	//effect->RGBflags = light->rgb.flags;
	VectorCopy(effect->startRGB, effect->RGB);

	effect->startsize = effect->size = flrand(particle->size.start.sf[0], particle->size.start.sf[1]);

	if(particle->size.flags != 0 || !(particle->size.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		effect->endsize = flrand(particle->size.end.ef[0], particle->size.end.ef[1]);
	}
	else
	{
		effect->endsize = effect->startsize;
	}
	//effect->sizeFlags = light->size.flags;

	effect->startalpha = effect->alpha = flrand(particle->alpha.start.sf[0], particle->alpha.start.sf[1]);
	if(particle->alpha.flags != 0 || !(particle->alpha.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		effect->endalpha = flrand(particle->alpha.end.ef[0], particle->alpha.end.ef[1]);
	}
	else
	{
		effect->endalpha = effect->startalpha;
	}
	//effect->alphaFlags = particle->alpha.flags;

	effect->startrotation = effect->rotation = flrand(particle->rotation[0], particle->rotation[1]);
	effect->deltarotation = flrand(particle->rotationDelta[0], particle->rotationDelta[1]);

	vecrandom(particle->velocity[0], particle->velocity[1], effect->velocity);
	vecrandom(particle->acceleration[0], particle->acceleration[1], effect->accel);

	effect->shader = particle->shader.fieldHandles[Q_irand(0, particle->shader.numFields-1)];

	FX_AddPrimitive((fxPrimitive_t *)effect, effect->startTime, effect->endTime);
}
