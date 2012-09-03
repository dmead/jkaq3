#include "tr_fx.h"

FXPlayingParticle_t *runningEffects;
static int numRunningEffects;

// Add this effect to the list of objects that should be drawn
void CFxScheduler_AddToScheduler(FXPlayingParticle_t *particle)
{
	numRunningEffects++;
	runningEffects = (FXPlayingParticle_t *)realloc(runningEffects, sizeof(*runningEffects)*numRunningEffects);
	runningEffects[numRunningEffects-1] = *particle;
}

// Stop this object from drawing (either by life running out or hitting a surface under the right conditions)
void CFxScheduler_RemoveFromScheduler(int effectID)
{	// Don't call this function from within a particle's death function, the game already does this
	int i;
	if(effectID < numRunningEffects-1)
	{
		// Pop this one off the 'stack'
		for(i = effectID+1; i < numRunningEffects; i++)
		{
			runningEffects[i-1] = runningEffects[i];
		}
	}
	numRunningEffects--;
	runningEffects = (FXPlayingParticle_t *)realloc(runningEffects, sizeof(*runningEffects)*numRunningEffects);
}

// Perform actions on each particle
void CFxScheduler_RunSchedulerLoop(void)
{
	int i;

	for(i = 0; i < numRunningEffects; i++)
	{
		float phase = (backEnd.refdef.time - runningEffects[i].startTime)/ \
			((runningEffects[i].endTime > runningEffects[i].startTime) ? (runningEffects[i].endTime - runningEffects[i].startTime) : 0.0001f);				// Prevent divide by zero
		// Kill pass -- weed out any FX that shouldn't belong
		if(backEnd.refdef.time < runningEffects[i].startTime)
		{
			continue;	// Effect shouldn't be playing...just chillax for a bit.
		}
		if(phase > 1)
		{
			if(runningEffects[i].death)
			{
				runningEffects[i].death(&runningEffects[i]);
			}
			CFxScheduler_RemoveFromScheduler(i);
			continue;
		}
		// Think pass -- effects are moved about and handled accordingly
		if(runningEffects[i].think)
		{
			runningEffects[i].think(phase, &runningEffects[i]);
		}
		{
			// Check the culling on it
			vec3_t result;
			//float dist;
			VectorSubtract(backEnd.refdef.vieworg, runningEffects[i].currentOrigin, result);
			if(runningEffects[i].cullDist > 0 && VectorLengthSquared(result) > (runningEffects[i].cullDist*runningEffects[i].cullDist)*2)
			{
				// Saves on performance according to Ensiform --eez
				continue;
			}
			else if(runningEffects[i].cullDist <= 0 && VectorLengthSquared(result) > (8192*8192))
			{
				// Always cull after 8192
				continue;
			}
		}
		// Rendering pass -- render the running effects in their current state
		if(runningEffects[i].render)
		{
			runningEffects[i].render(&runningEffects[i]);
		}
	}
}

// Allocate a little bit of memory for the scheduler
void CFxScheduler_InitScheduler(void)
{
	runningEffects = (FXPlayingParticle_t *)malloc(sizeof(FXPlayingParticle_t));
	numRunningEffects = 0;
}

void CFxScheduler_FreeScheduler(void)
{
	free(runningEffects);
}

void CFxScheduler_PlayEffectID(qhandle_t handle, vec3_t origin, vec3_t dir)
{	// Last two args == unknown??
	int i;
	FXFile_t file;
	if(handle < 0 || handle > FX_numFXFiles)
	{
		Com_Error(ERR_DROP, "Attempted to play an invalid effect handle (%i)", handle);
		return;
	}

	file = FX_fxHandles[handle];
	for(i = 0; i < file.numSegments ; i++)
	{
		switch(file.segments[i].segmentType)
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
				CFxPrimitive_CreateLightPrimitive(&file.segments[i], origin);
				break;
			case EFXS_LINE:
				break;
			case EFXS_ORIENTEDPARTICLE:
				break;
			case EFXS_PARTICLE:
				CFxPrimitive_CreateParticlePrimitive(&file.segments[i], origin, dir);
				break;
			case EFXS_SOUND:
				CFxPrimitives_CreateSoundPrimitive(&file.segments[i], origin);
				break;
			case EFXS_TAIL:
				break;
		}
	}
}

void CFxScheduler_PlayEffect( const char *file, vec3_t org, vec3_t fwd )
{
	qhandle_t registeredFile = CFxScheduler_RegisterEffect(file);
	if(!registeredFile)
	{
		return;
	}
	CFxScheduler_PlayEffectID( registeredFile, org, fwd );
}
