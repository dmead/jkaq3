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

#ifndef FX_LOCAL_H
#define FX_LOCAL_H

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../renderer/tr_types.h"

#define EFFECTS_FOLDER "effects/"

typedef int fxInt_t[2];					// Randomized
typedef float fxVec3_t[2][3];			// Randomized
typedef float fxFloat_t[2];				// Randomized
typedef struct
{
	char (*fields)[MAX_QPATH];						// Array of strings
	qhandle_t *fieldHandles;
	int numFields;
} fxShaderList_t;

#define FX_Copy(a, b)		a[0]=a[1]=b
#define FXV_Copy(a, b)		a[0][0]=a[0][1]=a[0][2]=a[1][0]=a[1][1]=a[1][2]=b
#define FXV_Copy2(a,b)		VectorCopy(b, a[0]); VectorCopy(b, a[1]);

enum FXFlags {
	FXFLAG_NONE,
	FXFLAG_DEPTHHACK,						// Shown through walls and level geometry
	FXFLAG_USEMODEL,						// Uses a model? (emitters)
											// Tapers from start to end? (electricity)
	FXFLAG_USEBBOX,							// Uses a bounding box?
											// Grow from start->end during life? (electricity)
	FXFLAG_USEPHYSICS,						// Uses physics?
											// Electricity branching? (electricity)
	FXFLAG_EXPENSIVEPHYSICS,				// Uses expensive physics?
	FXFLAG_GHOUL2COLLISION,					// Collides with GHOUL2 models?
	FXFLAG_GHOUL2DECALS,					// Creates decals on GHOUL2 models?
	FXFLAG_IMPACTKILLS,						// Impact kills FX?
	FXFLAG_IMPACTFX,						// Impact creates FX?
	FXFLAG_DEATHFX,							// Creates death FX? (unknown)
	FXFLAG_USEALPHA,						// Uses the alpha field?
	FXFLAG_EMITFX,							// Emit effects? (emitter probably)
	FXFLAG_RELATIVE,						// Unknown.
	FXFLAG_SETSHADERTIME,					// Set shader time for animating textures?
	FXFLAG_PAPERPHYSICS,					// Unknown.
};

enum FXSpawnFlags {
	FXSFLAG_NONE,
	FXSFLAG_ORG2FROMTRACE,					// Use trace to find endpoint? (lines and electricity only)
	FXSFLAG_TRACEIMPACTFX,					// Emit effect at endpoint (lines/electricity only, and only if using trace to find endpoint)
	FXSFLAG_ORG2ISOFFSET,					// Don't use trace to find endpoint? (lines/electricity only)
	FXSFLAG_CHEAPORIGINCALC,				// Origin is absolute?
	FXSFLAG_CHEAPORIGIN2CALC,				// Endpoint is absolute? (lines and electricity only)
	FXSFLAG_ABSOLUTEVEL,					// Absolute velocity (ignores speed of the fx_runner)
	FXSFLAG_ABSOLUTEACCEL,					// Absolute accelation (ignores speed of the fx_runner)
	FXSFLAG_ORGONSPHERE,					// Origins use a sphere distribution?
	FXSFLAG_ORGONCYLINDER,					// Origins use a cylinder distribution?
	FXSFLAG_AXISFROMSPHERE,					// Set effect to offset direction? (ORGONSPHERE and ORGONCYLINDER only)
	FXSFLAG_RANDROTAROUNDFWD,				// Rotate randomly around the forward axis? (fxrunner only)
	FXSFLAG_EVENDISTRIBUTION,				// Use even delay distribution? (NOT LIGHTS or CAMERASHAKES)
	FXSFLAG_RGBCOMPONENTINTERPOLATION,		// Use color cube for RGB?
	FXSFLAG_AFFECTEDBYWIND,					// Affected by wind?
	FXSFLAG_UNUSED,
};

typedef enum {
	FXTLT_INT,
	FXTLT_FLOAT,
	FXTLT_VECTOR,
} FX_TimeLapseTypes_e;

typedef enum {
	FXTLF_NONE,
	FXTLF_CONSTANT,
	FXTLF_LINEAR,
	FXTLF_NONLINEAR,
	FXTLF_WAVE,
	FXTLF_CLAMP,
	FXTLF_RANDOM,
} FX_TimeLapseFlags_e;

typedef struct
{
	union {
		fxInt_t		si;
		fxFloat_t	sf;
		fxVec3_t	sv;
	} start;
	union {
		fxInt_t		ei;
		fxFloat_t	ef;
		fxVec3_t	ev;
	} end;
	fxInt_t		parameter;
	int			flags;
	FX_TimeLapseTypes_e		timelapseType;
} fxTimeLapse_t;

// --------------------------------------------------------------
// Data for all the various segment types go here
// --------------------------------------------------------------

typedef struct
{
	// -- Generation --
	fxInt_t		life;
	fxInt_t		delay;
	fxInt_t		intensity;
	fxVec3_t	origin;
	fxInt_t		radius;
	fxInt_t		cullrange;
} FXCameraShakeSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		delay;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxFloat_t	radius;
	fxFloat_t	height;
	fxTimeLapse_t size;
	fxTimeLapse_t size2;
	fxTimeLapse_t length;
	// -- Color --
	fxTimeLapse_t rgb;
	fxTimeLapse_t alpha;
	fxShaderList_t shader;
} FXCylinderSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		delay;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxFloat_t		radius;
	fxFloat_t		height;
	fxFloat_t		rotation;
	fxTimeLapse_t	size;
	// -- Color --
	fxTimeLapse_t	rgb;
	fxTimeLapse_t	alpha;
	fxShaderList_t	shader;
} FXDecalSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		delay;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxVec3_t	origin2;		// Specified endpoint
	fxFloat_t		radius;
	fxFloat_t		height;
	fxTimeLapse_t	size;
	// -- Electricity --
	fxInt_t		chaos;
	// -- Color --
	fxTimeLapse_t	rgb;
	fxTimeLapse_t	alpha;
	fxShaderList_t	shader;
} FXElectricitySegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxFloat_t		radius;
	fxFloat_t		height;
	fxTimeLapse_t	size;
	// -- Motion --
	fxInt_t		wind;
	fxVec3_t	velocity;
	fxVec3_t	acceleration;
	fxInt_t		gravity;
	// -- Physics --
	fxInt_t		bounce;
	fxVec3_t	mins;
	fxVec3_t	maxs;
	// -- Emitter --
	fxInt_t		density;
	fxInt_t		variance;
	// -- Model --
	fxVec3_t	angle;
	fxVec3_t	angleDelta;
} FXEmitterSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxTimeLapse_t	size;
	// -- Color --
	fxTimeLapse_t	rgb;
	fxTimeLapse_t	alpha;
	fxShaderList_t	shader;
} FXFlashSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		life;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxTimeLapse_t	size;
	// -- Color --
	fxTimeLapse_t	rgb;
} FXLightSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxVec3_t	origin2;	// Specified endpoint
	fxFloat_t		radius;
	fxFloat_t		height;
	fxTimeLapse_t	size;
	// -- Color --
	fxTimeLapse_t	rgb;
	fxTimeLapse_t	alpha;
	fxShaderList_t	shader;
} FXLineSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxFloat_t		radius;
	fxFloat_t		height;
	fxTimeLapse_t	size;
	// -- Motion --
	fxInt_t		wind;
	fxVec3_t	velocity;
	fxVec3_t	acceleration;
	fxInt_t		gravity;
	fxInt_t		rotation;
	fxInt_t		rotationDelta;
	// -- Color --
	fxTimeLapse_t	rgb;
	fxTimeLapse_t	alpha;
	fxShaderList_t	shader;
} FXOrientedParticleSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxFloat_t		radius;
	fxFloat_t		height;
	fxTimeLapse_t	size;
	// -- Motion --
	fxInt_t		wind;
	fxVec3_t	velocity;
	fxVec3_t	acceleration;
	fxInt_t		gravity;
	fxFloat_t		rotation;
	fxFloat_t		rotationDelta;
	// -- Color --
	fxTimeLapse_t	rgb;
	fxTimeLapse_t	alpha;
	fxShaderList_t	shader;
} FXParticleSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		count;
	fxFloat_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	// -- Sound --
	fxShaderList_t	sound;
} FXSoundSegment_t;

typedef struct
{
	// -- Generation --
	fxInt_t		delay;
	fxInt_t		count;
	fxInt_t		life;
	fxInt_t		cullrange;
	// -- Origin/Size --
	fxVec3_t	origin;
	fxFloat_t		radius;
	fxFloat_t		height;
	fxTimeLapse_t	size;
	// -- Length/Size2 --
	fxTimeLapse_t	length;
	// -- Motion --
	fxInt_t		wind;
	fxVec3_t	velocity;
	fxVec3_t	acceleration;
	fxInt_t		gravity;
	// -- Color --
	fxTimeLapse_t	rgb;
	fxTimeLapse_t	alpha;
	fxShaderList_t	shader;
} FXTailSegment_t;

// --------------------------------------------------------------
// Data for all the various segment types end
// --------------------------------------------------------------


enum FXSegments_e {
	EFXS_CAMERASHAKE,
	EFXS_CYLINDER,
	EFXS_DECAL,
	EFXS_ELECTRICITY,
	EFXS_EMITTER,
	EFXS_FLASH,
	EFXS_FXRUNNER,
	EFXS_LIGHT,
	EFXS_LINE,
	EFXS_ORIENTEDPARTICLE,
	EFXS_PARTICLE,
	EFXS_SOUND,
	EFXS_TAIL,
};

typedef struct {
	union {
		FXCameraShakeSegment_t			*FXCameraShakeSegment;
		FXCylinderSegment_t				*FXCylinderSegment;
		FXDecalSegment_t				*FXDecalSegment;
		FXElectricitySegment_t			*FXElectricitySegment;
		FXEmitterSegment_t				*FXEmitterSegment;
		FXFlashSegment_t				*FXFlashSegment;
		FXLightSegment_t				*FXLightSegment;
		FXLineSegment_t					*FXLineSegment;
		FXOrientedParticleSegment_t		*FXOrientedParticleSegment;
		FXParticleSegment_t				*FXParticleSegment;
		FXSoundSegment_t				*FXSoundSegment;
		FXTailSegment_t					*FXTailSegment;
	} SegmentData;
	int segmentType;
	int flags;
	int spawnflags;
} FXSegment_t;

typedef struct fxFile_s {
	char		name[MAX_QPATH];		// game path, including extension

	int			numSegments;
	FXSegment_t	*segments;		// contains all the various segments

	struct fxFile_s* next;
} fxFile_t;

extern fxFile_t parsedfile;
extern int fx_numFiles;
extern int fx_filesCapacity;
extern fxFile_t *fx_fileHandles;

typedef struct fxPrimitive_s fxPrimitive_t;

typedef struct fxPrimitive_s {
	// Must always be the same
	int		primitiveType;

	qboolean (*Cull)(fxPrimitive_t *_self);
	qboolean (*Update)(fxPrimitive_t *_self);
	qboolean (*Draw)(fxPrimitive_t *_self);

	float	culldist;

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	velocity;
	vec3_t	accel;

	int		flags;

	float	startalpha;
	float	endalpha;
	float	alpha;
	float	alphafreq;

	vec3_t	startRGB;
	vec3_t	endRGB;
	vec3_t	RGB;
	int		RGBflags;
	float	RGBparameter;

	float	startsize;
	float	endsize;
	float	size;
	int		sizeFlags;

	int		startTime;
	int		endTime;

	qhandle_t	shader;
	// End must always be the same
} fxPrimitive_t;

typedef struct fxLine_s {
	// Must be same as fxPrimitive_t
	int		primitiveType;

	qboolean (*Cull)(fxPrimitive_t *_self);
	qboolean (*Update)(fxPrimitive_t *_self);
	qboolean (*Draw)(fxPrimitive_t *_self);

	float	culldist;

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	velocity;
	vec3_t	accel;

	int		flags;

	float	startalpha;
	float	endalpha;
	float	alpha;
	float	alphafreq;

	vec3_t	startRGB;
	vec3_t	endRGB;
	vec3_t	RGB;
	int		RGBflags;
	float	RGBparameter;

	float	startsize;
	float	endsize;
	float	size;
	int		sizeFlags;

	int		startTime;
	int		endTime;

	qhandle_t	shader;

	// End same as fxPrimitive_t

	// Line specific

	vec3_t	origin2;
	vec3_t	work_origin;
	float	stscale;
	qhandle_t	shader_endcap;
} fxLine_t;

typedef struct fxParticle_s {
	// Must be same as fxPrimitive_t
	int		primitiveType;

	qboolean (*Cull)(fxPrimitive_t *_self);
	qboolean (*Update)(fxPrimitive_t *_self);
	qboolean (*Draw)(fxPrimitive_t *_self);

	float	culldist;

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	velocity;
	vec3_t	accel;

	int		flags;

	float	startalpha;
	float	endalpha;
	float	alpha;
	float	alphafreq;

	vec3_t	startRGB;
	vec3_t	endRGB;
	vec3_t	RGB;
	int		RGBflags;
	float	RGBparameter;

	float	startsize;
	float	endsize;
	float	size;
	int		sizeFlags;

	int		startTime;
	int		endTime;

	qhandle_t	shader;
	// End same as fxPrimitive_t

	float	startrotation;
	float	deltarotation;
	float	rotation;
} fxParticle_t;

typedef struct fxLight_s {
	// Must be same as fxPrimitive_t
	int		primitiveType;

	qboolean (*Cull)(fxPrimitive_t *_self);
	qboolean (*Update)(fxPrimitive_t *_self);
	qboolean (*Draw)(fxPrimitive_t *_self);

	float	culldist;

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	velocity;
	vec3_t	accel;

	int		flags;

	float	startalpha;
	float	endalpha;
	float	alpha;
	float	alphafreq;

	vec3_t	startRGB;
	vec3_t	endRGB;
	vec3_t	RGB;
	int		RGBflags;
	float	RGBparameter;

	float	startsize;
	float	endsize;
	float	size;
	int		sizeFlags;

	int		startTime;
	int		endTime;

	qhandle_t	shader;
	// End same as fxPrimitive_t
} fxLight_t;

// Init
extern qboolean fx_init;
extern refdef_t *fx_refDef;
extern int fx_time;

#define MAX_FX_CULL 8192

void FX_Init( void );
void FX_Shutdown( void );

void FX_AdjustTime( int fxTime );

void FX_SystemInit( refdef_t *rd );
void FX_SystemShutdown( void );

void FX_InitFileMemory( int initialSize );
void FX_CleanupFileMemory( void );

// Load and Parse
fxFile_t *FX_FindEffect( const char *name );
fxFile_t *FX_GetByHandle( fxHandle_t hFX );
fxHandle_t FX_RegisterEffect( const char *name );
qboolean FX_ParseEffectFile( const char *fileName );
void FX_FilePrecache( void );

// Run

typedef struct fxState_s
{
	fxPrimitive_t	*effect;
	int				startTime;
	int				killTime;
} fxState_t;

#define	MAX_PLAYING_EFFECTS		2048

extern fxState_t	fx_renderList[ MAX_PLAYING_EFFECTS ];
extern fxState_t	*fx_nextValid;

void FX_FreeEffect( fxState_t *state );
void FX_AddPrimitive( fxPrimitive_t *primitive, int starttime, int killtime );
void FX_Scheduler_AddEffects( qboolean skybox );
void FX_PlayEffectID( fxHandle_t handle, vec3_t origin, vec3_t dir );
void FX_PlayEffect( const char *file, vec3_t origin, vec3_t fwd );

// Sound
void FX_CreateLight( const FXSegment_t *segment, vec3_t origin );
void FX_CreateParticle( const FXSegment_t *segment, vec3_t origin, vec3_t dir );
void FX_CreateSound( const FXSegment_t *segment, vec3_t origin );

#endif