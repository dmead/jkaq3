#include "tr_fx.h"

// This file is part of iojamp.
// This file contains all the rendering and spawning of different effect primitives. 'nuff said.

float flrand(float m1, float m2)
{
	return m1 + (float)rand()/((float)RAND_MAX/(m2-m1));
}

void vecrandom(vec3_t v1, vec3_t v2, vec3_t *result)
{
	int i;
	if(!result)
	{
		return;
	}
	for(i = 0; i < 3; i++)
	{
		(*result)[i] = flrand(v1[i], v2[i]);
	}
}

float lerp(float start, float end, float phase)
{
	return (start*(1-phase)+end*phase);
}

float coslerp(float start, float end, float phase)
{
	float phase2 = (1-cos(phase*M_PI))/2;
	return (start*(1-phase2)+end*phase2);
}

// The following 4 sections are ported straight from Raven's files.

//Sprite vertex template
const	vec3_t sprite_template[4] =
{
	{ -1, -1,  0,	},	//Top left
	{ -1,  1,  0,	},	//Bottom left
	{  1,  1,  0,	},	//Bottom right
	{  1, -1,  0,	},	//Top right
};

//Sprite UV template
const	float sprite_texture_template[][2] = 
{
	{  0.0f,  0.0f	},	//Top left
	{  0.0f,  1.0f	},	//Bottom left
	{  1.0f,  1.0f	},	//Bottom right
	{  1.0f,  0.0f	},	//Top right
};

const	vec3_t	quad_template[] = 
{
	{	-1.0f, -1.0f, 0.0f	},
	{	-1.0f,  1.0f, 0.0f	},
	{	 1.0f,  1.0f, 0.0f	},
	{	 1.0f, -1.0f, 0.0f	}
};

const	float	quad_st_template[][2] = 
{
	{   0.0f,  0.0f	},
	{   0.0f,  1.0f	},
	{   1.0f,  1.0f	},
	{   1.0f,  0.0f	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//																									 //
//																									 //
//									SOUND EFFECT SEGMENTS											 //
//																									 //
//																									 //
///////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma region Sound Effects
#endif
extern void S_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx );
static void CFXPRI_SoundDeath(FXPlayingParticle_t *part)
{
	// Play a sound
	ri.StartSound(part->currentOrigin, -1, CHAN_AUTO, part->handle);
}

void CFxPrimitives_CreateSoundPrimitive(FXSegment_t *segment, vec3_t origin)
{	// Does not require dir
	int i;
	int countRand;
	FXSoundSegment_t *sfx;
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
	countRand = Q_irand(sfx->count[0], sfx->count[1]);
	for(i = 0; i < countRand; i++)
	{
		FXPlayingParticle_t part;
		Com_Memset(&part, 0, sizeof(part));
		part.cullDist = flrand(sfx->cullrange[0], sfx->cullrange[1])*2;
		part.cullDist *= part.cullDist; // allows for VLSquared
		part.startTime = backEnd.refdef.time + Q_irand(sfx->delay[0], sfx->delay[1]);
		part.endTime = part.startTime + 1;
		part.handle = sfx->sound.fieldHandles[Q_irand(0, sfx->sound.numFields-1)];
		vecrandom(sfx->origin[0], sfx->origin[1], &part.originalOrigin);
		if(!(segment->spawnflags & FXSFLAG_CHEAPORIGINCALC) || segment->spawnflags < 0)
		{
			// Cheap origin calculation if false - use origin listed
			VectorAdd(part.originalOrigin, origin, part.originalOrigin);
		}
		VectorCopy(part.originalOrigin, part.currentOrigin);
		part.death = CFXPRI_SoundDeath;
		CFxScheduler_AddToScheduler(&part);
	}
}
#ifdef WIN32
#pragma endregion
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////
//																									 //
//																									 //
//									LIGHT SEGMENTS													 //
//																									 //
//																									 //
///////////////////////////////////////////////////////////////////////////////////////////////////////

// This is really pretty easy, all it does is create a dynamic light on each render using RE_AddDynamicLightToScene
#ifdef WIN32
#pragma region Lights
#endif
static void CFxPrimitive_LightThink(float phase, FXPlayingParticle_t *part)
{
	// Only care about things that change here.
	int i;

	if(VectorCompare(part->startRGB, part->endRGB))		// This changes based on time, so do some sort of magic lerping
	{
		// TODO: wave/clamp/nonlinear. BLAH.
		for(i = 0; i < 3; i++)
		{
			part->currentRGB[i] = lerp(part->startRGB[i], part->endRGB[i], phase);
		}
	}
	if(part->startSize != part->endSize)
	{
		part->currentSize = coslerp(part->startSize, part->endSize, phase);
	}
}

static void CFxPrimitive_LightRender(FXPlayingParticle_t *part)
{
	RE_AddLightToScene(part->currentOrigin, part->currentSize*5, part->currentRGB[0], part->currentRGB[1], part->currentRGB[2]);
}

void CFxPrimitive_CreateLightPrimitive(FXSegment_t *segment, vec3_t origin)
{	// Does not require dir
	FXLightSegment_t *light;
	FXPlayingParticle_t part;
	if(!segment)
	{	// NULL segment pointer?
		return;
	}
	if(!segment->SegmentData.FXLightSegment)
	{	// No sound segment data?
		return;
	}
	if(segment->segmentType != EFXS_LIGHT)
	{	// Not a sound?
		return;
	}
	light = segment->SegmentData.FXLightSegment;
	Com_Memset(&part, 0, sizeof(part));

	part.cullDist = flrand(light->cullrange[0], light->cullrange[1])*2;
	part.cullDist *= part.cullDist; // allows for VLSquared
	part.startTime = backEnd.refdef.time + Q_irand(light->delay[0], light->delay[1]);
	part.endTime = part.startTime + Q_irand(light->life[0], light->life[1]);
	vecrandom(light->origin[0], light->origin[1], &part.originalOrigin);
	if(!(segment->spawnflags & FXSFLAG_CHEAPORIGINCALC) || segment->spawnflags < 0)
	{
		// Cheap origin calculation if false - use origin listed
		VectorAdd(part.originalOrigin, origin, part.originalOrigin);
	}
	VectorCopy(part.originalOrigin, part.currentOrigin);


	vecrandom(light->rgb.start.sv[0], light->rgb.start.sv[1], &part.startRGB);
	if(light->rgb.flags != 0 || !(light->rgb.flags & FXTLF_CONSTANT))
	{
		vecrandom(light->rgb.end.ev[0], light->rgb.end.ev[1], &part.endRGB);
	}
	else
	{
		VectorCopy(part.startRGB, part.endRGB);
	}
	part.RGBflags = light->rgb.flags;
	VectorCopy(part.startRGB, part.currentRGB);

	part.startSize = part.currentSize = flrand(light->size.start.sf[0], light->size.start.sf[1]);
	if(light->size.flags != 0 || !(light->size.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		part.endSize = flrand(light->size.end.ef[0], light->size.end.ef[1]);
	}
	else
	{
		part.endSize = part.startSize;
	}
	part.sizeFlags = light->size.flags;

	part.render = CFxPrimitive_LightRender;
	part.think = CFxPrimitive_LightThink;
	CFxScheduler_AddToScheduler(&part);
}
#ifdef WIN32
#pragma endregion
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////
//																									 //
//																									 //
//									PARTICLE SEGMENTS												 //
//																									 //
//																									 //
///////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma region Particles
#endif
static void CFxPrimitive_ParticleThink(float phase, FXPlayingParticle_t *part)
{
	// Only care about things that change here.
	int i;

	if(VectorCompare(part->startRGB, part->endRGB))		// This changes based on time, so do some sort of magic lerping
	{
		// TODO: wave/clamp/nonlinear. BLAH.
		if(part->RGBflags & FXTLF_NONLINEAR)
		{
			// Nonlinear
			/*if(phase > part->RGBparameter)
			{
				for(i = 0; i < 3; i++)
				{
					part->currentRGB[i] = lerp(part->startRGB[i], part->endRGB[i], 1.0f(1.0f-phase));
				}
			}
			else if(part->RGBflags & FXTLF_LINEAR)
			{
				// TODO: Nonlinear/Linear
				goto LinearParticleRGB;
			}*/
		}
		else if(part->RGBflags & FXTLF_LINEAR)
		{
//LinearParticleRGB:
			for(i = 0; i < 3; i++)
			{
				part->currentRGB[i] = lerp(part->startRGB[i], part->endRGB[i], phase);
			}
		}
	}
	if(part->startSize != part->endSize)
	{
		part->currentSize = coslerp(part->startSize, part->endSize, phase);
	}
	if(part->startAlpha != part->endAlpha)
	{
		part->currentAlpha = lerp(part->startAlpha, part->endAlpha, phase);
	}
	if(part->rotationStart+part->rotationDelta != part->rotationStart)
	{
		part->currentRotation = lerp(part->rotationStart, part->rotationStart+part->rotationDelta, phase);
	}
	// TODO: do movement
	VectorAdd(part->currentOrigin, part->velocity, part->currentOrigin);
}

static void CFxPrimitive_ParticleRender(FXPlayingParticle_t *part)
{
	polyVert_t verts[4];
	vec3_t axis[3];
	int i;
	float scale;
	qboolean notzeroaxis = qfalse;

	//if(part->lastRenderTime > backEnd.refdef.time - 5)
	//{
	//	return;
	//}

	scale = part->currentSize * 2.0f;

	for(i = 0; i < 3; i++)
	{
		VectorCopy(backEnd.viewParms.or.axis[i], axis[i]);
	}

	for(i = 0; i < 3; i++)
	{
		if(!VectorCompare(axis[i], vec3_origin)) {
			notzeroaxis = qtrue;
			break;
		}
	}

	if(notzeroaxis && part->currentRotation)
		RotateAroundDirection(axis, part->currentRotation);

	for(i = 0; i < 4; i++)
	{
		// Loop through each vert in the quad
		VectorMA( part->currentOrigin,	quad_template[i][0] * scale, axis[1], verts[i].xyz );
		VectorMA( verts[i].xyz, quad_template[i][1] * scale, axis[2], verts[i].xyz );

		//Setup the UVs
		verts[i].st[0] = quad_st_template[i][0];
		verts[i].st[1] = quad_st_template[i][1];

		//Setup the vertex modulation
		verts[i].modulate[0] = (byte)((part->currentRGB[0] * 255));
		verts[i].modulate[1] = (byte)((part->currentRGB[1] * 255));
		verts[i].modulate[2] = (byte)((part->currentRGB[2] * 255));

		// TODO: Use alpha chan? (copy from Elite Forces SDK?)
		// FIXME: NO EFFECT WTF
		{
			verts[i].modulate[3] = (byte)(part->currentAlpha * 255);
		}
	}

	RE_AddPolyToScene(part->handle, 4, verts, 1);

	//part->lastRenderTime = backEnd.refdef.time + 5;
}

void CFxPrimitive_CreateParticlePrimitive(FXSegment_t *segment, vec3_t origin, vec3_t dir)
{
	FXParticleSegment_t *particle;
	FXPlayingParticle_t part;
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
	Com_Memset(&part, 0, sizeof(part));

	part.cullDist = flrand(particle->cullrange[0], particle->cullrange[1])*2;
	part.cullDist *= part.cullDist; // allows for VLSquared
	part.startTime = backEnd.refdef.time + Q_irand(particle->delay[0], particle->delay[1]);
	part.endTime = part.startTime + Q_irand(particle->life[0], particle->life[1]);
	vecrandom(particle->origin[0], particle->origin[1], &part.originalOrigin);
	if(!(segment->spawnflags & FXSFLAG_CHEAPORIGINCALC) || segment->spawnflags < 0)
	{
		// Cheap origin calculation if false - use origin listed
		VectorAdd(part.originalOrigin, origin, part.originalOrigin);
	}
	VectorCopy(part.originalOrigin, part.currentOrigin);

	VectorCopy(particle->velocity[Q_irand(0,1)], part.velocity);
	VectorCopy(particle->acceleration[Q_irand(0,1)], part.acceleration);

	part.handle = particle->shader.fieldHandles[Q_irand(0, particle->shader.numFields-1)];


	vecrandom(particle->rgb.start.sv[0], particle->rgb.start.sv[1], &part.startRGB);
	if(particle->rgb.flags != 0 || !(particle->rgb.flags & FXTLF_CONSTANT))
	{
		vecrandom(particle->rgb.end.ev[0], particle->rgb.end.ev[1], &part.endRGB);
	}
	else
	{
		VectorCopy(part.startRGB, part.endRGB);
	}
	part.RGBflags = particle->rgb.flags;
	VectorCopy(part.startRGB, part.currentRGB);

	part.startSize = part.currentSize = flrand(particle->size.start.sf[0], particle->size.start.sf[1]);
	if(particle->size.flags != 0 || !(particle->size.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		part.endSize = flrand(particle->size.end.ef[0], particle->size.end.ef[1]);
	}
	else
	{
		part.endSize = part.startSize;
	}
	part.sizeFlags = particle->size.flags;

	part.startAlpha = part.currentAlpha = flrand(particle->alpha.start.sf[0], particle->alpha.start.sf[1]);
	if(particle->alpha.flags != 0 || !(particle->alpha.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		part.endAlpha = flrand(particle->alpha.end.ef[0], particle->alpha.end.ef[1]);
	}
	else
	{
		part.endAlpha = part.startAlpha;
	}
	part.alphaFlags = particle->alpha.flags;

	// Do rotation bits
	part.currentRotation = part.rotationStart = flrand(particle->rotation[0], particle->rotation[1]);
	part.rotationDelta = flrand(particle->rotationDelta[0], particle->rotationDelta[1]);

	part.velocity[0] = flrand(particle->velocity[0][0], particle->velocity[1][0]);
	part.velocity[1] = flrand(particle->velocity[0][1], particle->velocity[1][1]);
	part.velocity[2] = flrand(particle->velocity[0][2], particle->velocity[1][2]);

	part.render = CFxPrimitive_ParticleRender;
	part.think = CFxPrimitive_ParticleThink;
	CFxScheduler_AddToScheduler(&part);
}
#ifdef WIN32
#pragma endregion
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////
//																									 //
//																									 //
//									LINES															 //
//																									 //
//																									 //
///////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#ifdef WIN32
#pragma region Lines
#endif
static void CFxPrimitive_ParticleThink(float phase, FXPlayingParticle_t *part)
{
	// Only care about things that change here.
	int i;

	if(VectorCompare(part->startRGB, part->endRGB))		// This changes based on time, so do some sort of magic lerping
	{
		// TODO: wave/clamp/nonlinear. BLAH.
		if(part->RGBflags & FXTLF_NONLINEAR)
		{
			// Nonlinear
			/*if(phase > part->RGBparameter)
			{
				for(i = 0; i < 3; i++)
				{
					part->currentRGB[i] = lerp(part->startRGB[i], part->endRGB[i], 1.0f(1.0f-phase));
				}
			}
			else if(part->RGBflags & FXTLF_LINEAR)
			{
				// TODO: Nonlinear/Linear
				goto LinearParticleRGB;
			}*/
		}
		else if(part->RGBflags & FXTLF_LINEAR)
		{
			for(i = 0; i < 3; i++)
			{
				part->currentRGB[i] = lerp(part->startRGB[i], part->endRGB[i], phase);
			}
		}
	}
	if(part->startSize != part->endSize)
	{
		part->currentSize = coslerp(part->startSize, part->endSize, phase);
	}
	if(part->startAlpha != part->endAlpha)
	{
		part->currentAlpha = lerp(part->startAlpha, part->endAlpha, phase);
	}
	if(part->rotationStart+part->rotationDelta != part->rotationStart)
	{
		part->currentRotation = lerp(part->rotationStart, part->rotationStart+part->rotationDelta, phase);
	}
	// TODO: do movement
	VectorAdd(part->currentOrigin, part->velocity, part->currentOrigin);
}

static void CFxPrimitive_LineRender(FXPlayingParticle_t *part)
{
	polyVert_t verts[4];
	vec3_t axis[3];
	int i;
	float scale;

	if(part->lastRenderTime > backEnd.refdef.time - 50)
	{
		return;
	}

	scale = part->currentSize * 2.0f;

	for(i = 0; i < 3; i++)
	{
		VectorCopy(backEnd.viewParms.or.axis[i], axis[i]);
	}

	if(part->currentRotation)
		RotateAroundDirection(axis, part->currentRotation);

	for(i = 0; i < 4; i++)
	{
		// Loop through each vert in the quad
		VectorMA( part->currentOrigin,	sprite_template[i][0] * scale, axis[1], verts[i].xyz );
		VectorMA( verts[i].xyz, sprite_template[i][1] * scale, axis[2], verts[i].xyz );

		//Setup the UVs
		verts[i].st[0] = sprite_texture_template[i][0];
		verts[i].st[1] = sprite_texture_template[i][1];

		//Setup the vertex modulation
		verts[i].modulate[0] = (byte)(part->currentRGB[0] * 255);
		verts[i].modulate[1] = (byte)(part->currentRGB[1] * 255);
		verts[i].modulate[2] = (byte)(part->currentRGB[2] * 255);

		// TODO: Use alpha chan? (copy from Elite Forces SDK?)
		verts[i].modulate[3] = part->currentAlpha*255;
	}

	RE_AddPolyToScene(part->handle, 4, verts, 1);

	//part->lastRenderTime = backEnd.refdef.time + 50;
}

void CFxPrimitive_CreateLinePrimitive(FXSegment_t *segment, vec3_t origin, vec3_t dir)
{
	FXLineSegment_t *particle;
	FXPlayingParticle_t part;
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
	Com_Memset(&part, 0, sizeof(part));

	part.cullDist = flrand(particle->cullrange[0], particle->cullrange[1]);
	part.startTime = backEnd.refdef.time + Q_irand(particle->delay[0], particle->delay[1]);
	part.endTime = part.startTime + Q_irand(particle->life[0], particle->life[1]);
	vecrandom(particle->origin[0], particle->origin[1], &part.originalOrigin);
	if(!(segment->spawnflags & FXSFLAG_CHEAPORIGINCALC) || segment->spawnflags < 0)
	{
		// Cheap origin calculation if false - use origin listed
		VectorAdd(part.originalOrigin, origin, part.originalOrigin);
	}
	VectorCopy(part.originalOrigin, part.currentOrigin);

	VectorCopy(particle->velocity[Q_irand(0,1)], part.velocity);
	VectorCopy(particle->acceleration[Q_irand(0,1)], part.acceleration);

	part.handle = particle->shader.fieldHandles[Q_irand(0, particle->shader.numFields-1)];


	vecrandom(particle->rgb.start.sv[0], particle->rgb.start.sv[1], &part.startRGB);
	if(particle->rgb.flags != 0 || !(particle->rgb.flags & FXTLF_CONSTANT))
	{
		vecrandom(particle->rgb.end.ev[0], particle->rgb.end.ev[1], &part.endRGB);
	}
	else
	{
		VectorCopy(part.startRGB, part.endRGB);
	}
	part.RGBflags = particle->rgb.flags;
	VectorCopy(part.startRGB, part.currentRGB);

	part.startSize = part.currentSize = flrand(particle->size.start.sf[0], particle->size.start.sf[1]);
	if(particle->size.flags != 0 || !(particle->size.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		part.endSize = flrand(particle->size.end.ef[0], particle->size.end.ef[1]);
	}
	else
	{
		part.endSize = part.startSize;
	}
	part.sizeFlags = particle->size.flags;

	part.startAlpha = part.currentAlpha = flrand(particle->alpha.start.sf[0], particle->alpha.start.sf[1]);
	if(particle->alpha.flags != 0 || !(particle->alpha.flags & FXTLF_CONSTANT))
	{
		// TODO: make the distinction between wave, clamp, nonlinear, etc
		part.endAlpha = flrand(particle->alpha.end.ef[0], particle->alpha.end.ef[1]);
	}
	else
	{
		part.endAlpha = part.startAlpha;
	}
	part.alphaFlags = particle->alpha.flags;

	// Do rotation bits
	part.currentRotation = part.rotationStart = flrand(particle->rotation[0], particle->rotation[1]);
	part.rotationDelta = flrand(particle->rotationDelta[0], particle->rotationDelta[1]);

	part.velocity[0] = flrand(particle->velocity[0][0], particle->velocity[1][0]);
	part.velocity[1] = flrand(particle->velocity[0][1], particle->velocity[1][1]);
	part.velocity[2] = flrand(particle->velocity[0][2], particle->velocity[1][2]);

	part.render = CFxPrimitive_ParticleRender;
	part.think = CFxPrimitive_ParticleThink;
	CFxScheduler_AddToScheduler(&part);
}
#ifdef WIN32
#pragma endregion
#endif
#endif
