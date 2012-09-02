#ifndef __TR_FX_H
#define __TR_FX_H
#include "tr_local.h"

// tr_fx_load.c

typedef int fxInt_t[2];					// Randomized
typedef float fxVec3_t[2][3];			// Randomized
typedef float fxFloat_t[2];				// Randomized
typedef struct
{
	char (*fields)[MAX_QPATH];						// Array of strings
	qhandle_t *fieldHandles;
	int numFields;
} fxShaderList_t;

#define EFFECTS_FOLDER "effects/"

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

typedef struct {
	// There's some metadata in the .efx file format...this is probably used for EffectsEd but can't be sure --eezstreet
	char filename[MAX_QPATH];
	int numSegments;
	FXSegment_t *segments;		// contains all the various segments
} FXFile_t;

extern int FX_numFXFiles;
extern FXFile_t *FX_fxHandles;


qboolean CFxScheduler_ParseEffect(char *fileName);
int CFxScheduler_RegisterEffect(const char *path);
void CFxScheduler_Init(void);
void CFxScheduler_Cleanup(void);

// tr_fx.c

typedef struct FXPlayingParticle_s FXPlayingParticle_t;

typedef struct FXPlayingParticle_s {
	// Timing control and killing effects
	int startTime;
	int endTime;
	int lastRenderTime;
	void (*death)(FXPlayingParticle_t *thisParticle);					// Played on effect death
	void (*think)(float phase, FXPlayingParticle_t *thisParticle);		// Thinking
	void (*render)(FXPlayingParticle_t *thisParticle);					// Render pass
	// Origin and culling
	vec3_t currentOrigin;
	vec3_t originalOrigin;
	float cullDist;
	// Size handling
	float currentSize;
	float startSize;
	float endSize;
	int sizeFlags;
	float sizeParameter;
	// Velocity
	vec3_t velocity;
	vec3_t acceleration;
	// Shaders, RGB and coloring
	qhandle_t handle;
	vec3_t currentRGB;
	vec3_t startRGB;
	vec3_t endRGB;
	int RGBflags;
	float RGBparameter;
	// Alpha
	float currentAlpha;
	float startAlpha;
	float endAlpha;
	int alphaFlags;
	float alphaParameter;
	// Rotation
	float rotationDelta;
	float rotationStart;
	float currentRotation;
} FXPlayingParticle_t;

void CFxScheduler_AddToScheduler(FXPlayingParticle_t *particle);
void CFxScheduler_RemoveFromScheduler(int effectID);
void CFxScheduler_RunSchedulerLoop(void);
void CFxScheduler_InitScheduler(void);
void CFxScheduler_FreeScheduler(void);
void CFxScheduler_PlayEffect( const char *file, vec3_t org, vec3_t fwd );
void CFxScheduler_PlayEffectID(qhandle_t handle, vec3_t origin, vec3_t dir);

// tr_fx_primitives.c

void CFxPrimitive_CreateLightPrimitive(FXSegment_t *segment, vec3_t origin);
void CFxPrimitives_CreateSoundPrimitive(FXSegment_t *segment, vec3_t origin);
void CFxPrimitive_CreateParticlePrimitive(FXSegment_t *segment, vec3_t origin, vec3_t dir);

#endif