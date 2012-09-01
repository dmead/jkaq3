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

///////////////////////////////////////////////////////////////////////////////////////////////////////
//																									 //
//																									 //
//									SOUND EFFECT SEGMENTS											 //
//																									 //
//																									 //
///////////////////////////////////////////////////////////////////////////////////////////////////////


static void CFXPRI_SoundDeath(FXPlayingParticle_t *part)
{
	// Play a sound
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
		memset(&part, 0, sizeof(part));
		part.cullDist = flrand(sfx->cullrange[0], sfx->cullrange[1]);
		part.startTime = backEnd.refdef.time + Q_irand(sfx->delay[0], sfx->delay[1]);
		part.endTime = part.startTime + 1;
		part.handle = sfx->sound.fieldHandles[Q_irand(0, sfx->sound.numFields)];
		vecrandom(sfx->origin[0], sfx->origin[1], &part.originalOrigin);
		CFxScheduler_AddToScheduler(&part);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//																									 //
//																									 //
//									LIGHT SEGMENTS													 //
//																									 //
//																									 //
///////////////////////////////////////////////////////////////////////////////////////////////////////

// This is really pretty easy, all it does is create a dynamic light on each render using RE_AddDynamicLightToScene

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
	RE_AddLightToScene(part->currentOrigin, part->currentSize*10, part->currentRGB[0], part->currentRGB[1], part->currentRGB[2]);
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
	memset(&part, 0, sizeof(part));

	part.cullDist = flrand(light->cullrange[0], light->cullrange[1]);
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

///////////////////////////////////////////////////////////////////////////////////////////////////////
//																									 //
//																									 //
//									PARTICLE SEGMENTS												 //
//																									 //
//																									 //
///////////////////////////////////////////////////////////////////////////////////////////////////////

static void CFxPrimitive_ParticleThink(float phase, FXPlayingParticle_t *part)
{
}

static void CFxPrimitive_ParticleRender(FXPlayingParticle_t *part)
{
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
	memset(&part, 0, sizeof(part));

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

	part.render = CFxPrimitive_ParticleRender;
	part.think = CFxPrimitive_ParticleThink;
	CFxScheduler_AddToScheduler(&part);
}
