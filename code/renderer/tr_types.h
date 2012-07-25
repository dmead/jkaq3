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
//
#ifndef __TR_TYPES_H
#define __TR_TYPES_H


#define	MAX_DLIGHTS		32		// can't be increased, because bit flags are used on surfaces

#define	ENTITYNUM_BITS		10		// can't be increased without changing drawsurf bit packing (QSORT_ENTITYNUM_SHIFT)
#define	MAX_ENTITIES		((1<<ENTITYNUM_BITS) - 1)

// renderfx flags
#define	RF_MINLIGHT		0x00001		// allways have some light (viewmodel, some items)
#define	RF_THIRD_PERSON		0x00002		// don't draw through eyes, only mirrors (player bodies, chat sprites)
#define	RF_FIRST_PERSON		0x00004		// only draw through eyes (view weapon, damage blood blob)
#define	RF_DEPTHHACK		0x00008		// for view weapon Z crunching
#define RF_NODEPTH			0x00010	// No depth at all (seeing through walls)

#define RF_VOLUMETRIC		0x00020	// fake volumetric shading

#define	RF_NOSHADOW		0x00040		// don't add stencil shadows

#define RF_LIGHTING_ORIGIN	0x00080		// use refEntity->lightingOrigin instead of refEntity->origin
						// for lighting.  This allows entities to sink into the floor
						// with their origin going solid, and allows all parts of a
						// player to get the same lighting

#define	RF_SHADOW_PLANE		0x00100		// use refEntity->shadowPlane
#define	RF_WRAP_FRAMES		0x00200		// mod the model frames by the maxframes to allow continuous

#define RF_FORCE_ENT_ALPHA	0x00400 // override shader alpha settings
#define RF_RGB_TINT			0x00800 // override shader rgb settings

#define	RF_SHADOW_ONLY		0x01000	//add surfs for shadowing but don't draw them -rww

#define	RF_DISTORTION		0x02000	//area distortion effect -rww

#define RF_FORKED			0x04000	// override lightning to have forks
#define RF_TAPERED			0x08000	// lightning tapers
#define RF_GROW				0x10000	// lightning grows from start to end during its life

#define RF_DISINTEGRATE1	0x20000	// does a procedural hole-ripping thing.
#define RF_DISINTEGRATE2	0x40000	// does a procedural hole-ripping thing with scaling at the ripping point

#define RF_SETANIMINDEX		0x80000	//use backEnd.currentEntity->e.skinNum for R_BindAnimatedImage

#define RF_ALPHA_DEPTH		0x100000 //depth write on alpha model

#define RF_FORCEPOST		0x200000 //force it to post-render -rww

#define RF_CROSSHAIR		0x400000		// This item is a cross hair and will draw over everything similar to
						// DEPTHHACK in stereo rendering mode, with the difference that the
						// projection matrix won't be hacked to reduce the stereo separation as
						// is done for the gun.

// refdef flags
#define RDF_NOWORLDMODEL	0x0001		// used for player configuration screen
#define RDF_HYPERSPACE		0x0004		// teleportation effect

#define RDF_SKYBOXPORTAL	0x0008
#define RDF_DRAWSKYBOX		0x0010		// the above marks a scene as being a 'portal sky'.  this flag says to draw it or not

#define RDF_AUTOMAP			0x0020		//means this scene is to draw the automap -rww
#define	RDF_NOFOG			0x0040		//no global fog in this scene (but still brush fog) -rww

typedef struct {
	vec3_t		xyz;
	float		st[2];
	byte		modulate[4];
} polyVert_t;

typedef struct poly_s {
	qhandle_t			hShader;
	int					numVerts;
	polyVert_t			*verts;
} poly_t;

typedef enum {
	RT_MODEL,
	RT_POLY,
	RT_SPRITE,
	RT_ORIENTED_QUAD,
	RT_BEAM,
	RT_SABER_GLOW,
	RT_ELECTRICITY,
	RT_PORTALSURFACE,		// doesn't draw anything, just info for portals
	RT_LINE,
	RT_ORIENTEDLINE,
	RT_CYLINDER,
	RT_ENT_CHAIN,
	RT_RAIL_CORE,
	RT_RAIL_RINGS,
	RT_LIGHTNING,

	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

typedef struct miniRefEntity_s 
{
	refEntityType_t		reType;
	int					renderfx;

	qhandle_t			hModel;				// opaque type outside refresh

	// most recent data
	vec3_t				axis[3];			// rotation vectors
	qboolean			nonNormalizedAxes;	// axis are not normalized, i.e. they have scale
	vec3_t				origin;				// also used as MODEL_BEAM's "from"

	// previous data for frame interpolation
	vec3_t				oldorigin;			// also used as MODEL_BEAM's "to"

	// texturing
	qhandle_t			customShader;		// use one image for the entire thing

	// misc
	byte				shaderRGBA[4];		// colors used by rgbgen entity shaders
	vec2_t				shaderTexCoord;		// texture coordinates used by tcMod entity modifiers

	// extra sprite information
	float				radius;
	float				rotation;			// size 2 for RT_CYLINDER or number of verts in RT_ELECTRICITY

	// misc
	float		shaderTime;			// subtracted from refdef time to control effect start times
	int			frame;				// also used as MODEL_BEAM's diameter

} miniRefEntity_t;

#pragma warning (disable : 4201 )
typedef struct {
	// this stucture must remain identical as the miniRefEntity_t
	//
	//
	refEntityType_t		reType;
	int					renderfx;

	qhandle_t			hModel;				// opaque type outside refresh

	// most recent data
	vec3_t				axis[3];			// rotation vectors
	qboolean			nonNormalizedAxes;	// axis are not normalized, i.e. they have scale
	vec3_t				origin;				// also used as MODEL_BEAM's "from"

	// previous data for frame interpolation
	vec3_t				oldorigin;			// also used as MODEL_BEAM's "to"

	// texturing
	qhandle_t			customShader;		// use one image for the entire thing

	// misc
	byte				shaderRGBA[4];		// colors used by rgbgen entity shaders
	vec2_t				shaderTexCoord;		// texture coordinates used by tcMod entity modifiers

	// extra sprite information
	float				radius;
	float				rotation;

	// misc
	float		shaderTime;			// subtracted from refdef time to control effect start times
	int			frame;				// also used as MODEL_BEAM's diameter
	//
	//
	// end miniRefEntity_t

	//
	//
	// specific full refEntity_t data
	//
	//

	// most recent data
	vec3_t		lightingOrigin;		// so multi-part models can be lit identically (RF_LIGHTING_ORIGIN)
	float		shadowPlane;		// projection shadows go here, stencils go slightly lower

	// previous data for frame interpolation
	int			oldframe;
	float		backlerp;			// 0.0 = current, 1.0 = old

	// texturing
	int			skinNum;			// inline skin index
	qhandle_t	customSkin;			// NULL for default skin

	// texturing
	union	
	{
//		int			skinNum;		// inline skin index
//		ivec3_t		terxelCoords;	// coords of patch for RT_TERXELS	
		struct
		{
			int		miniStart;
			int		miniCount;
		} uMini;
	} uRefEnt;

	// extra sprite information
	union {
		struct 
		{
			float rotation;
			float radius;
			byte  vertRGBA[4][4];
		} sprite;
		struct 
		{
			float width;
			float width2;
			float stscale;
		} line;
		struct	// that whole put-the-opening-brace-on-the-same-line-as-the-beginning-of-the-definition coding style is fecal
		{
			float	width;
			vec3_t	control1;
			vec3_t	control2;
		} bezier;
		struct
		{
			float width;
			float width2;
			float stscale;
			float height;
			float bias;
			qboolean wrap;
		} cylinder;
		struct 
		{
			float width;
			float deviation;
			float stscale;
			qboolean wrap;
			qboolean taper;
		} electricity;
	} data;

	float		endTime;
	float		saberLength;

/*
Ghoul2 Insert Start
*/
	vec3_t		angles;				// rotation angles - used for Ghoul2

	vec3_t		modelScale;			// axis scale for models
//	CGhoul2Info_v	*ghoul2;  		// has to be at the end of the ref-ent in order for it to be created properly
	void		*ghoul2;  		// has to be at the end of the ref-ent in order for it to be created properly
/*
Ghoul2 Insert End
*/
} refEntity_t;


#define	MAX_RENDER_STRINGS			8
#define	MAX_RENDER_STRING_LENGTH	32

typedef struct {
	int			x, y, width, height;
	float		fov_x, fov_y;
	vec3_t		vieworg;
	vec3_t		viewangles;
	vec3_t		viewaxis[3];		// transformation matrix
	int			viewContents;		// world contents at vieworg

	// time in milliseconds for shader effects and other time dependent rendering issues
	int			time;

	int			rdflags;			// RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	byte		areamask[MAX_MAP_AREA_BYTES];

	// text messages for deform text shaders
	char		text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];
} refdef_t;


typedef enum {
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT
} stereoFrame_t;


/*
** glconfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum {
	TC_NONE,
	TC_S3TC,  // this is for the GL_S3_s3tc extension.
	TC_S3TC_ARB  // this is for the GL_EXT_texture_compression_s3tc extension.
} textureCompression_t;

typedef enum {
	GLDRV_ICD,					// driver is integrated with window system
								// WARNING: there are tests that check for
								// > GLDRV_ICD for minidriverness, so this
								// should always be the lowest value in this
								// enum set
	GLDRV_STANDALONE,			// driver is a non-3Dfx standalone driver
	GLDRV_VOODOO				// driver is a 3Dfx standalone driver
} glDriverType_t;

typedef enum {
	GLHW_GENERIC,			// where everthing works the way it should
	GLHW_3DFX_2D3D,			// Voodoo Banshee or Voodoo3, relevant since if this is
							// the hardware type then there can NOT exist a secondary
							// display adapter
	GLHW_RIVA128,			// where you can't interpolate alpha
	GLHW_RAGEPRO,			// where you can't modulate alpha on alpha textures
	GLHW_PERMEDIA2			// where you don't have src*dst
} glHardwareType_t;

typedef struct {
	const char				*renderer_string;
	const char				*vendor_string;
	const char				*version_string;
	const char				*extensions_string;

	int						maxTextureSize;			// queried from GL
	int						maxActiveTextures;		// multitexture ability (numTextureUnits)
	float					maxTextureFilterAnisotropy;

	int						colorBits, depthBits, stencilBits;

	qboolean				deviceSupportsGamma;
	textureCompression_t	textureCompression;
	qboolean				textureEnvAddAvailable;
	qboolean				clampToEdgeAvailable;	// jka

	int						vidWidth, vidHeight;

	int						displayFrequency;

	// synonymous with "does rendering consume the entire screen?", therefore
	// a Voodoo or Voodoo2 will have this set to TRUE, as will a Win32 ICD that
	// used CDS.
	qboolean				isFullscreen;
	qboolean				stereoEnabled;

} glconfig_t;

typedef struct {
	qboolean				smpActive;		// dual processor

	// q3 shit:
	glDriverType_t			driverType;
	glHardwareType_t		hardwareType;

	// aspect is the screen's physical width / height, which may be different
	// than scrWidth / scrHeight if the pixels are non-square
	// normal screens should be 4/3, but wide aspect monitors may be 16/9
	float					windowAspect;

} glconfig2_t;

#endif	// __TR_TYPES_H
