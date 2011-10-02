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
#ifndef __QFILES_H__
#define __QFILES_H__

//
// qfiles.h: quake file formats
// This file must be identical in the quake and utils directories
//

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

// surface geometry should not exceed these limits
#define	SHADER_MAX_VERTEXES	1000
#define	SHADER_MAX_INDEXES	(6*SHADER_MAX_VERTEXES)


// the maximum size of game relative pathnames
#define	MAX_QPATH		64

/*
========================================================================

QVM files

========================================================================
*/

#define	VM_MAGIC			0x12721444
#define	VM_MAGIC_VER2	0x12721445
typedef struct {
	int		vmMagic;

	int		instructionCount;

	int		codeOffset;
	int		codeLength;

	int		dataOffset;
	int		dataLength;
	int		litLength;			// ( dataLength - litLength ) should be byteswapped on load
	int		bssLength;			// zero filled memory appended to datalength

	//!!! below here is VM_MAGIC_VER2 !!!
	int		jtrgLength;			// number of jump table targets
} vmHeader_t;

/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

#define MD3_IDENT			(('3'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD3_VERSION			15

// limits
#define MD3_MAX_LODS		3
#define	MD3_MAX_TRIANGLES	8192	// per surface
#define MD3_MAX_VERTS		4096	// per surface
#define MD3_MAX_SHADERS		256		// per surface
#define MD3_MAX_FRAMES		1024	// per model
#define	MD3_MAX_SURFACES	32		// per model
#define MD3_MAX_TAGS		16		// per frame

// vertex scales
#define	MD3_XYZ_SCALE		(1.0/64)

typedef struct md3Frame_s {
	vec3_t		bounds[2];
	vec3_t		localOrigin;
	float		radius;
	char		name[16];
} md3Frame_t;

typedef struct md3Tag_s {
	char		name[MAX_QPATH];	// tag name
	vec3_t		origin;
	vec3_t		axis[3];
} md3Tag_t;

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
typedef struct {
	int		ident;				// 

	char	name[MAX_QPATH];	// polyset name

	int		flags;
	int		numFrames;			// all surfaces in a model should have the same

	int		numShaders;			// all surfaces in a model should have the same
	int		numVerts;

	int		numTriangles;
	int		ofsTriangles;

	int		ofsShaders;			// offset from start of md3Surface_t
	int		ofsSt;				// texture coords are common for all frames
	int		ofsXyzNormals;		// numVerts * numFrames

	int		ofsEnd;				// next surface follows
} md3Surface_t;

typedef struct {
	char			name[MAX_QPATH];
	int				shaderIndex;	// for in-game use
} md3Shader_t;

typedef struct {
	int			indexes[3];
} md3Triangle_t;

typedef struct {
	float		st[2];
} md3St_t;

typedef struct {
	short		xyz[3];
	short		normal;
} md3XyzNormal_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	int			flags;

	int			numFrames;
	int			numTags;			
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;			// offset for first frame
	int			ofsTags;			// numFrames * numTags
	int			ofsSurfaces;		// first surface, others follow

	int			ofsEnd;				// end of file
} md3Header_t;

/*
==============================================================================

MD4 file format

==============================================================================
*/

#define MD4_IDENT			(('4'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD4_VERSION			1
#define	MD4_MAX_BONES		128

typedef struct {
	int			boneIndex;		// these are indexes into the boneReferences,
	float		   boneWeight;		// not the global per-frame bone list
	vec3_t		offset;
} md4Weight_t;

typedef struct {
	vec3_t		normal;
	vec2_t		texCoords;
	int			numWeights;
	md4Weight_t	weights[1];		// variable sized
} md4Vertex_t;

typedef struct {
	int			indexes[3];
} md4Triangle_t;

typedef struct {
	int			ident;

	char		name[MAX_QPATH];	// polyset name
	char		shader[MAX_QPATH];
	int			shaderIndex;		// for in-game use

	int			ofsHeader;			// this will be a negative number

	int			numVerts;
	int			ofsVerts;

	int			numTriangles;
	int			ofsTriangles;

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;				// next surface follows
} md4Surface_t;

typedef struct {
	float		matrix[3][4];
} md4Bone_t;

typedef struct {
	vec3_t		bounds[2];			// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;				// dist from localOrigin to corner
	md4Bone_t	bones[1];			// [numBones]
} md4Frame_t;

typedef struct {
	int			numSurfaces;
	int			ofsSurfaces;		// first surface, others follow
	int			ofsEnd;				// next lod follows
} md4LOD_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	// frames and bones are shared by all levels of detail
	int			numFrames;
	int			numBones;
	int			ofsBoneNames;		// char	name[ MAX_QPATH ]
	int			ofsFrames;			// md4Frame_t[numFrames]

	// each level of detail has completely separate sets of surfaces
	int			numLODs;
	int			ofsLODs;

	int			ofsEnd;				// end of file
} md4Header_t;

/*
 * Here are the definitions for Ravensoft's model format of md4. Raven stores their
 * playermodels in .mdr files, in some games, which are pretty much like the md4
 * format implemented by ID soft. It seems like ID's original md4 stuff is not used at all.
 * MDR is being used in EliteForce, JediKnight2 and Soldiers of Fortune2 (I think).
 * So this comes in handy for anyone who wants to make it possible to load player
 * models from these games.
 * This format has bone tags, which is similar to the thing you have in md3 I suppose.
 * Raven has released their version of md3view under GPL enabling me to add support
 * to this codebase. Thanks to Steven Howes aka Skinner for helping with example
 * source code.
 *
 * - Thilo Schulz (arny@ats.s.bawue.de)
 */

// If you want to enable support for Raven's .mdr / md4 format, uncomment the next
// line.
//#define RAVENMD4

#ifdef RAVENMD4

#define MDR_IDENT	(('5'<<24)+('M'<<16)+('D'<<8)+'R')
#define MDR_VERSION	2
#define	MDR_MAX_BONES	128

typedef struct {
	int			boneIndex;	// these are indexes into the boneReferences,
	float		   boneWeight;		// not the global per-frame bone list
	vec3_t		offset;
} mdrWeight_t;

typedef struct {
	vec3_t		normal;
	vec2_t		texCoords;
	int			numWeights;
	mdrWeight_t	weights[1];		// variable sized
} mdrVertex_t;

typedef struct {
	int			indexes[3];
} mdrTriangle_t;

typedef struct {
	int			ident;

	char		name[MAX_QPATH];	// polyset name
	char		shader[MAX_QPATH];
	int			shaderIndex;	// for in-game use

	int			ofsHeader;	// this will be a negative number

	int			numVerts;
	int			ofsVerts;

	int			numTriangles;
	int			ofsTriangles;

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;		// next surface follows
} mdrSurface_t;

typedef struct {
	float		matrix[3][4];
} mdrBone_t;

typedef struct {
	vec3_t		bounds[2];		// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;			// dist from localOrigin to corner
	char		name[16];
	mdrBone_t	bones[1];		// [numBones]
} mdrFrame_t;

typedef struct {
        unsigned char Comp[24]; // MC_COMP_BYTES is in MatComp.h, but don't want to couple
} mdrCompBone_t;

typedef struct {
        vec3_t          bounds[2];		// bounds of all surfaces of all LOD's for this frame
        vec3_t          localOrigin;		// midpoint of bounds, used for sphere cull
        float           radius;			// dist from localOrigin to corner
        mdrCompBone_t   bones[1];		// [numBones]
} mdrCompFrame_t;

typedef struct {
	int			numSurfaces;
	int			ofsSurfaces;		// first surface, others follow
	int			ofsEnd;				// next lod follows
} mdrLOD_t;

typedef struct {
        int                     boneIndex;
        char            name[32];
} mdrTag_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	// frames and bones are shared by all levels of detail
	int			numFrames;
	int			numBones;
	int			ofsFrames;			// mdrFrame_t[numFrames]

	// each level of detail has completely separate sets of surfaces
	int			numLODs;
	int			ofsLODs;

        int                     numTags;
        int                     ofsTags;

	int			ofsEnd;				// end of file
} mdrHeader_t;

#endif

/*===========================================================================
Ghoul2 file format (GLM/GLA)
============================================================================*/
typedef struct {
	//float matrix[3][4];
	vec4_t matrix[3];
} mgMatrix34_t;//mgMatrix34_t

typedef struct {
	float quat[7];
} mgQuat_t;//was glaBoneQuat_t

#define GLM_IDENT                      (('M'<<24)+('G'<<16)+('L'<<8)+'2')
#define GLA_IDENT                      (('A'<<24)+('G'<<16)+('L'<<8)+'2')
#define GLM_VERSION 6
#define GLA_VERSION 6
#define fG2_BONEWEIGHT_RECIPROCAL_MULT  ((float)(1.0f/1023.0f))
#define iG2_BITS_PER_BONEREF                    5
#define iMAX_G2_BONEREFS_PER_SURFACE    (1<<iG2_BITS_PER_BONEREF)       // (32)
#define iMAX_G2_BONEWEIGHTS_PER_VERT    4       // can't just be blindly increased, affects cache size etc
#define iG2_BONEWEIGHT_TOPBITS_SHIFT    ((iG2_BITS_PER_BONEREF * iMAX_G2_BONEWEIGHTS_PER_VERT) - 8)     // 8 bits because of 8 in the BoneWeight[] array entry
#define iG2_BONEWEIGHT_TOPBITS_AND              0x300   // 2 bits, giving 10 total, or 10 bits, for 1023/1024 above
#define sDEFAULT_GLA_NAME "*default"    // used when making special simple ghoul2 models, usually from MD3 files

//BEGIN GLA Headers
typedef struct {
	unsigned char Comp[14];
} glaCompQuatBone_t;

typedef struct {
	int                     ident;                          //      "IDP3" = MD3, "RDM5" = MDR, "2LGA"(GL2 Anim) = GLA
	int                     version;                        // 1,2,3 etc as per format revision
	char					name[MAX_QPATH];        // GLA name (eg "skeletons/marine")     // note: extension missing
	float					fScale;                         // will be zero if build before this field was defined, else scale it was built with
	int                     numFrames;
	int                     ofsFrames;                      // points at glaFrame_t array
	int                     numBones;                       // (no offset to these since they're inside the frames array)
	int                     ofsCompBonePool;        // offset to global compressed-bone pool that all frames use
	int                     ofsSkel;                        // offset to glaSkel_t info
	int                     ofsEnd;                         // EOF, which of course gives overall file size
} glaHeader_t;

typedef struct
{
	int offsets[1];
} glaSkelOffsets_t;

typedef struct
{
	char name[MAX_QPATH];
	unsigned int flags;
	int parent;
	mgMatrix34_t BasePoseMat;
	mgMatrix34_t BasePoseMatInv;
	int numChildren;
	int children[1];
} glaSkel_t;

typedef struct {
	int iIndex;
} glaIndex_t;

typedef struct {
	unsigned short a[7];
} glaCompBone_t;

typedef struct {
	glaCompBone_t compArray[1];
} glaCompBoneArray_t;

//BEGIN GLM Headers
typedef struct {
	int		ident;                          // "IDP3" = MD3, "RDM5" = MDR, "2LGM"(GL2 Mesh) = GLM   (cruddy char order I know, but I'm following what was there in other versions)
	int		version;                        // 1,2,3 etc as per format revision
	char	name[MAX_QPATH];        // model name (eg "models/players/marine.glm")  // note: extension supplied
	char	animName[MAX_QPATH];// name of animation file this mesh requires        // note: extension missing
	int                     animIndex;                      // filled in by game (carcass defaults it to 0)
	int                     numBones;                       // (for ingame version-checks only, ensure we don't ref more bones than skel file has)
	int                     numLODs;
	int                     ofsLODs;
	int                     numSurfaces;            // now that surfaces are drawn hierarchically, we have same # per LOD
	int                     ofsSurfHierarchy;
	int                     ofsEnd;                         // EOF, which of course gives overall file size
} glmHeader_t;

typedef struct {
	int offsets[1];         // variable sized (glmHeader_t->numSurfaces), each offset points to a glmSurfHierarchy_t below
} glmHierarchyOffsets_t;

typedef struct {
	char            name[MAX_QPATH];
	unsigned int flags;
	char            shader[MAX_QPATH];
	int                     shaderIndex;            // for in-game use (carcass defaults to 0)
	int                     parentIndex;            // this points to the index in the file of the parent surface. -1 if null/root
	int                     numChildren;            // number of surfaces which are children of this one
	int                     childIndexes[1];        // [glmSurfHierarch_t->numChildren] (variable sized)
} glmSurfHierarchy_t;

typedef struct {
// (used to contain numSurface/ofsSurfaces fields, but these are same per LOD level now)
//
	int                     ofsEnd;                         // offset to next LOD
} glmLOD_t;

typedef struct {        // added in GLM version 3 for ingame use at Jake's request
	int offsets[1];         // variable sized (glmHeader_t->numSurfaces), each offset points to surfaces below
} glmLODSurfOffset_t;

typedef struct {
int                     ident;                          // this one field at least should be kept, since the game-engine may switch-case (but currently=0 in carcass)
int                     thisSurfaceIndex;       // 0...glmHeader_t->numSurfaces-1 (because of how ingame renderer works)
int                     ofsHeader;                      // this will be a negative number, pointing back to main header
int                     numVerts;
int                     ofsVerts;
int                     numTriangles;
int                     ofsTriangles;
                                        // Bone references are a set of ints representing all the bones
                                        // present in any vertex weights for this surface.  This is
                                        // needed because a model may have surfaces that need to be
                                        // drawn at different sort times, and we don't want to have
                                        // to re-interpolate all the bones for each surface.
                                        //
 int                     numBoneReferences;
 int                     ofsBoneReferences;
int                     ofsEnd;                         // next surface follows
} glmSurface_t;

typedef struct {
int                     indexes[3];
} glmTriangle_t;

typedef struct {
vec3_t                  normal;
vec3_t                  vertCoords;
// packed int...
unsigned int    uiNmWeightsAndBoneIndexes;
// 32 bits.  format: 
// 31 & 30:  0..3 (= 1..4) weight count
// 29 & 28 (spare)
// 2 bit pairs at 20,22,24,26 are 2-bit overflows from 4 BonWeights below (20=[0], 22=[1]) etc)
//  5-bits each (iG2_BITS_PER_BONEREF) for boneweights
// effectively a packed int, each bone weight converted from 0..1 float to 0..255 int...
//  promote each entry to float and multiply by fG2_BONEWEIGHT_RECIPROCAL_MULT to convert.
byte                    BoneWeightings[iMAX_G2_BONEWEIGHTS_PER_VERT];   // 4
} glmVertex_t;

// for each vert... (glmSurface_t->numVerts)  (seperated from glmVertex_t struct for cache reasons)
// {
// glmVertex_t - this is an array with number of verts from the surface definition as its bounds. It contains normal info, texture coors and number of weightings for this bone
typedef struct {
        vec2_t                  texCoords;
} glmVertexTexCoord_t;

/*
==============================================================================

  .BSP file format

==============================================================================
*/


#define BSP_IDENT   ( ( 'P' << 24 ) + ( 'S' << 16 ) + ( 'B' << 8 ) + 'R' )
		// little-endian "RBSP"

#define BSP_VERSION			1


// there shouldn't be any problem with increasing these values at the
// expense of more memory allocation in the utilities
#define	MAX_MAP_MODELS		0x400
#define	MAX_MAP_BRUSHES		0x8000
#define	MAX_MAP_ENTITIES	0x800
#define	MAX_MAP_ENTSTRING	0x40000
#define	MAX_MAP_SHADERS		0x400

#define	MAX_MAP_AREAS		0x100	// MAX_MAP_AREA_BYTES in q_shared must match!
#define	MAX_MAP_FOGS		0x100
#define	MAX_MAP_PLANES		0x20000
#define	MAX_MAP_NODES		0x20000
#define	MAX_MAP_BRUSHSIDES	0x20000
#define	MAX_MAP_LEAFS		0x20000
#define	MAX_MAP_LEAFFACES	0x20000
#define	MAX_MAP_LEAFBRUSHES 0x40000
#define	MAX_MAP_PORTALS		0x20000
#define	MAX_MAP_LIGHTING	0x800000
#define MAX_MAP_LIGHTGRID   0xFFFF // 65535
#define	MAX_MAP_LIGHTGRID_ARRAY	0x100000
#define	MAX_MAP_VISIBILITY	0x200000

#define	MAX_MAP_DRAW_SURFS	0x20000
#define	MAX_MAP_DRAW_VERTS	0x80000
#define	MAX_MAP_DRAW_INDEXES	0x80000


// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024

// the editor uses these predefined yaw angles to orient entities up or down
#define	ANGLE_UP			-1
#define	ANGLE_DOWN			-2

#define	LIGHTMAP_WIDTH		128
#define	LIGHTMAP_HEIGHT		128

#define MAX_WORLD_COORD		( 128*1024 )
#define MIN_WORLD_COORD		( -128*1024 )
#define WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

//=============================================================================


typedef struct {
	int		fileofs, filelen;
} lump_t;

#define	LUMP_ENTITIES		0
#define	LUMP_SHADERS		1
#define	LUMP_PLANES			2
#define	LUMP_NODES			3
#define	LUMP_LEAFS			4
#define	LUMP_LEAFSURFACES	5
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10
#define	LUMP_DRAWINDEXES	11
#define	LUMP_FOGS			12
#define	LUMP_SURFACES		13
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define LUMP_LIGHTARRAY		17
#define HEADER_LUMPS		18

typedef struct {
	int			ident;
	int			version;

	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
	float		mins[3], maxs[3];
	int			firstSurface, numSurfaces;
	int			firstBrush, numBrushes;
} dmodel_t;

typedef struct {
	char		shader[MAX_QPATH];
	int			surfaceFlags;
	int			contentFlags;
} dshader_t;

// planes x^1 is allways the opposite of plane x

typedef struct {
	float		normal[3];
	float		dist;
} dplane_t;

typedef struct {
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
} dnode_t;

typedef struct {
	int			cluster;			// -1 = opaque cluster (do I still store these?)
	int			area;

	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			firstLeafSurface;
	int			numLeafSurfaces;

	int			firstLeafBrush;
	int			numLeafBrushes;
} dleaf_t;

typedef struct {
	int			planeNum;			// positive plane side faces out of the leaf
	int			shaderNum;
	int			drawSurfNum;
} dbrushside_t;

typedef struct {
	int			firstSide;
	int			numSides;
	int			shaderNum;		// the shader that determines the contents flags
} dbrush_t;

typedef struct {
	char		shader[MAX_QPATH];
	int			brushNum;
	int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
} dfog_t;

// Light Style Constants
#define	MAXLIGHTMAPS	4
#define LS_NORMAL		0x00
#define LS_UNUSED		0xfe
#define	LS_LSNONE		0xff //rww - changed name because it unhappily conflicts with a lightsaber state name and changing this is just easier
#define MAX_LIGHT_STYLES		64

typedef struct {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[MAXLIGHTMAPS][2];
	vec3_t		normal;
	byte		color[MAXLIGHTMAPS][4];
} mapVert_t;

typedef struct {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[MAXLIGHTMAPS][2];
	vec3_t		normal;
	byte		color[MAXLIGHTMAPS][4];
} drawVert_t;

typedef struct {
	byte		ambientLight[MAXLIGHTMAPS][3];
	byte		directLight[MAXLIGHTMAPS][3];
	byte		styles[MAXLIGHTMAPS];
	byte		latLong[2];
} dgrid_t;

#define mapVert_t_cleared(x) mapVert_t (x) = {{0, 0, 0}, {0, 0}, {0, 0}, {0, 0, 0}, {0, 0, 0, 0}}
#define drawVert_t_cleared(x) drawVert_t (x) = {{0, 0, 0}, {0, 0}, {0, 0}, {0, 0, 0}, {0, 0, 0, 0}}

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE
} mapSurfaceType_t;

typedef struct {
	int			shaderNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	byte		lightmapStyles[MAXLIGHTMAPS], vertexStyles[MAXLIGHTMAPS];
	int			lightmapNum[MAXLIGHTMAPS];
	int			lightmapX[MAXLIGHTMAPS], lightmapY[MAXLIGHTMAPS];
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[3];	// for patches, [0] and [1] are lodbounds

	int			patchWidth;
	int			patchHeight;
} dsurface_t;

/////////////////////////////////////////////////////////////
//
// Defines and structures required for fonts

#define GLYPH_COUNT			256

// Must match define in stmparse.h
#define STYLE_DROPSHADOW	0x80000000
#define STYLE_BLINK			0x40000000
#define	SET_MASK			0x00ffffff

typedef struct 
{
	short		width;					// number of pixels wide
	short		height;					// number of scan lines
	short		horizAdvance;			// number of pixels to advance to the next char
	short		horizOffset;			// x offset into space to render glyph
	int			baseline;				// y offset 
	float		s;						// x start tex coord
	float		t;						// y start tex coord
	float		s2;						// x end tex coord
	float		t2;						// y end tex coord
} glyphInfo_t;


// this file corresponds 1:1 with the "*.fontdat" files, so don't change it unless you're going to
//	recompile the fontgen util and regenerate all the fonts!
//
typedef struct dfontdat_s
{
	glyphInfo_t		mGlyphs[GLYPH_COUNT];

	short			mPointSize;
	short			mHeight;				// max height of font
	short			mAscender;
	short			mDescender;

	short			mKoreanHack;
} dfontdat_t;

#endif
