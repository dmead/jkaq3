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
// tr_models.c -- model loading and caching

#include "tr_local.h"

#define	LL( x ) x = LittleLong( x )

static qboolean R_LoadMD3(model_t *mod, int lod, void *buffer, const char *name );
static qboolean R_LoadMD4(model_t *mod, void *buffer, const char *name );
#ifdef RAVENMD4
static qboolean R_LoadMDR(model_t *mod, void *buffer, int filesize, const char *name );
#endif

/*
====================
R_RegisterMD3
====================
*/
qhandle_t R_RegisterMD3(const char *name, model_t *mod)
{
	union {
		unsigned *u;
		void *v;
	} buf;
	int			lod;
	int			ident;
	qboolean	loaded = qfalse;
	int			numLoaded;
	char filename[MAX_QPATH], namebuf[MAX_QPATH+20];
	char *fext, defex[] = "md3";

	numLoaded = 0;

	strcpy(filename, name);

	fext = strchr(filename, '.');
	if(!fext)
		fext = defex;
	else
	{
		*fext = '\0';
		fext++;
	}

	for (lod = MD3_MAX_LODS - 1 ; lod >= 0 ; lod--)
	{
		if(lod)
			Com_sprintf(namebuf, sizeof(namebuf), "%s_%d.%s", filename, lod, fext);
		else
			Com_sprintf(namebuf, sizeof(namebuf), "%s.%s", filename, fext);

		ri.FS_ReadFile( namebuf, &buf.v );
		if(!buf.u)
			continue;
		
		ident = LittleLong(* (unsigned *) buf.u);
		if (ident == MD4_IDENT)
			loaded = R_LoadMD4(mod, buf.u, name);
		else
		{
			if (ident == MD3_IDENT)
				loaded = R_LoadMD3(mod, lod, buf.u, name);
			else
				ri.Printf(PRINT_WARNING,"R_RegisterMD3: unknown fileid for %s\n", name);
		}
		
		ri.FS_FreeFile(buf.v);

		if(loaded)
		{
			mod->numLods++;
			numLoaded++;
		}
		else
			break;
	}

	if(numLoaded)
	{
		// duplicate into higher lod spots that weren't
		// loaded, in case the user changes r_lodbias on the fly
		for(lod--; lod >= 0; lod--)
		{
			mod->numLods++;
			mod->md3[lod] = mod->md3[lod + 1];
		}

		return mod->index;
	}

#ifdef _DEBUG
	ri.Printf(PRINT_WARNING,"R_RegisterMD3: couldn't load %s\n", name);
#endif

	mod->type = MOD_BAD;
	return 0;
}

#ifdef RAVENMD4
/*
====================
R_RegisterMDR
====================
*/
qhandle_t R_RegisterMDR(const char *name, model_t *mod)
{
	union {
		unsigned *u;
		void *v;
	} buf;
	int	ident;
	qboolean loaded = qfalse;
	int filesize;

	filesize = ri.FS_ReadFile(name, (void **) &buf.v);
	if(!buf.u)
	{
		mod->type = MOD_BAD;
		return 0;
	}
	
	ident = LittleLong(*(unsigned *)buf.u);
	if(ident == MDR_IDENT)
		loaded = R_LoadMDR(mod, buf.u, filesize, name);

	ri.FS_FreeFile (buf.v);
	
	if(!loaded)
	{
		ri.Printf(PRINT_WARNING,"R_RegisterMDR: couldn't load mdr file %s\n", name);
		mod->type = MOD_BAD;
		return 0;
	}
	
	return mod->index;
}
#endif

/*
====================
R_RegisterIQM
====================
*/
qhandle_t R_RegisterIQM(const char *name, model_t *mod)
{
	union {
		unsigned *u;
		void *v;
	} buf;
	qboolean loaded = qfalse;
	int filesize;

	filesize = ri.FS_ReadFile(name, (void **) &buf.v);
	if(!buf.u)
	{
		mod->type = MOD_BAD;
		return 0;
	}
	
	loaded = R_LoadIQM(mod, buf.u, filesize, name);

	ri.FS_FreeFile (buf.v);
	
	if(!loaded)
	{
		ri.Printf(PRINT_WARNING,"R_RegisterIQM: couldn't load iqm file %s\n", name);
		mod->type = MOD_BAD;
		return 0;
	}
	
	return mod->index;
}

/*
====================
R_RegisterGLM
====================
*/
qhandle_t R_RegisterGLM(const char *name, model_t *mod)
{
	union {
		unsigned *u;
		void *v;
	} buf;
	qboolean loaded = qfalse;
	int filesize;

	filesize = ri.FS_ReadFile(name, (void **) &buf.v);
	if(!buf.u)
	{
		mod->type = MOD_BAD;
		return 0;
	}
	
	loaded = R_LoadGLM(mod, buf.u, filesize, name);

	ri.FS_FreeFile (buf.v);
	
	if(!loaded)
	{
		ri.Printf(PRINT_WARNING,"R_RegisterGLM: couldn't load glm file %s\n", name);
		mod->type = MOD_BAD;
		return 0;
	}
	
	return mod->index;
}

/*
====================
R_RegisterGLA
====================
*/
qhandle_t R_RegisterGLA(const char *name, model_t *mod)
{
	union {
		unsigned *u;
		void *v;
	} buf;
	qboolean loaded = qfalse;
	int filesize;

	filesize = ri.FS_ReadFile(name, (void **) &buf.v);
	if(!buf.u)
	{
		mod->type = MOD_BAD;
		return 0;
	}
	
	loaded = R_LoadGLA(mod, buf.u, filesize, name);

	ri.FS_FreeFile (buf.v);
	
	if(!loaded)
	{
		ri.Printf(PRINT_WARNING,"R_RegisterGLA: couldn't load gla file %s\n", name);
		mod->type = MOD_BAD;
		return 0;
	}
	
	return mod->index;
}


typedef struct
{
	char *ext;
	qhandle_t (*ModelLoader)( const char *, model_t * );
} modelExtToLoaderMap_t;

// Note that the ordering indicates the order of preference used
// when there are multiple models of different formats available
static modelExtToLoaderMap_t modelLoaders[ ] =
{
	{ "glm", R_RegisterGLM },
	{ "iqm", R_RegisterIQM },
#ifdef RAVENMD4
	{ "mdr", R_RegisterMDR },
#endif
	{ "md4", R_RegisterMD3 },
	{ "md3", R_RegisterMD3 }
};

static int numModelLoaders = ARRAY_LEN(modelLoaders);

//===============================================================================

/*
** R_GetModelByHandle
*/
model_t	*R_GetModelByHandle( qhandle_t index ) {
	model_t		*mod;

	// out of range gets the defualt model
	if ( index < 1 || index >= tr.numModels ) {
		return tr.models[0];
	}

	mod = tr.models[index];

	return mod;
}

//===============================================================================

/*
** R_AllocModel
*/
model_t *R_AllocModel( void ) {
	model_t		*mod;

	if ( tr.numModels == MAX_MOD_KNOWN ) {
		return NULL;
	}

	mod = ri.Hunk_Alloc( sizeof( *tr.models[tr.numModels] ), h_low );
	mod->index = tr.numModels;
	tr.models[tr.numModels] = mod;
	tr.numModels++;

	return mod;
}

CGhoul2Info_v *R_InitGhoul2Instance( void ) {
	CGhoul2Info_v *g2;

	if ( tr.numG2Instances == MAX_MOD_KNOWN ) {
		return NULL;
	}

	g2 = ri.Hunk_Alloc( sizeof( *tr.g2Instances[tr.numG2Instances] ), h_low );
	g2->index = tr.numG2Instances;
	tr.g2Instances[tr.numG2Instances] = g2;
	tr.numG2Instances++;

	return g2;
}

#if 0
/*
** R_AllocAnimState
*/
mg_animstate_t *R_AllocAnimState( void ) {
	mg_animstate_t *as = 0;
	
	if ( tr.numAnimStates == MAX_MOD_KNOWN ) {
		return NULL;
	}

	as = ri.Hunk_Alloc( sizeof( *tr.animstates[tr.numAnimStates] ), h_low );
	as->index = tr.numAnimStates;
	tr.animstates[tr.numAnimStates] = as;
	tr.numAnimStates++;
	//printf( "animAlloc index %d, numstates %d\n", as->index, tr.numAnimStates );
	
	return as;
}

mg_animstate_t *RE_AS_Fetch( qhandle_t index )
{

	mg_animstate_t *ts;
	if ( index < 1 || index >= tr.numAnimStates ) {
		//ri.Printf( PRINT_DEVELOPER, "animstate: request %d getting %d\n", index, 0 );
		ts = tr.animstates[0];
		return ts;
	}
	//printf( "index %d\n", index );
	//ri.Printf( PRINT_DEVELOPER, "animstate: request %d getting %d\n", index, index );
	ts = tr.animstates[index];
	//printf( "Fetching animAlloc %d | %d\n", ts->index, ts->numBones );
	return ts;
	/*
	int i=0;
	for(i=0;i<tr.numAnimStates;i++)
	{
		ts = tr.animstates[i];
		if( ts->ent_owner == index )
		{
			return ts;
		}
	}
	*/
}

qhandle_t RE_AS_Create( qhandle_t modelIndex, int entitynum )
{
	mg_animstate_t *as;
	model_t *mod;
	model_t *mod_anims;
	glmHeader_t *glm;
	qhandle_t hModel;
	qhandle_t as_found = 0;
	glaHeader_t *anims;
	int numSurfs;
	
	mod = R_GetModelByHandle( modelIndex );
	if( mod->type == MOD_GLM )
	{
		glm = mod->modelData;
		mod_anims = R_GetModelByHandle( glm->animIndex );
		if( mod_anims->type == MOD_GLA )
		{
			anims = mod_anims->modelData;
			numSurfs = glm->numSurfaces;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	
	for ( hModel = 1 ; hModel < tr.numAnimStates; hModel++ )
	{
		as = tr.animstates[hModel];
		if ( as->ent_owner == entitynum )
		{
			if( (as->numBones == anims->numBones) && (numSurfs == as->numSurfs) )
			{
				ri.Printf( PRINT_ALL, "Same number of surfs, bones, keeping animState!\n" );
				memset( as->surfList, 0, (4 * as->numSurfs) );
				memset( as->shaderList, 0, (4 * as->numSurfs) );
				return hModel;
			}
			else if( (as->numBones == anims->numBones) && (numSurfs != as->numSurfs) )
			{
				ri.Printf( PRINT_ALL, "Cleaning up animState, numSurfs doesn't match, but numBones does!\n" );
				if( numSurfs > as->numAllocSurfs )
				{
					ri.Printf( PRINT_ALL, "animState numSurfs is less than the incoming model!\n" );
					Z_Free( as->surfList );
					Z_Free( as->shaderList );
					as->sanitarySurfs = qfalse;
				}
				as_found = hModel;
				as = tr.animstates[as_found];
				R_SyncRenderThread();
				if( numSurfs > as->numAllocSurfs )
				{
					as->surfList = Z_Malloc( 4*numSurfs+4 );
					as->shaderList = Z_Malloc( 4*numSurfs+4 );
					as->numAllocSurfs = numSurfs;
				}
				as->numSurfs = numSurfs;
				memset( as->surfList, 0, (4 * as->numSurfs) );
				memset( as->shaderList, 0, (4 * as->numSurfs) );
				//Com_Printf( "AnimState (%d) Re-Inited, Owner %d\n", as->index, as->ent_owner );
				return as->index;
			}
			else
			{
				Z_Free( as->pose );
				Z_Free( as->resframe );
				Z_Free( as->newframe );
				Z_Free( as->oldframe );
				Z_Free( as->surfList );
				Z_Free( as->shaderList );
				as->sanitarySurfs = qfalse;
				as_found = hModel;
			}
		}
	}
	
	if( !as_found )
	{
		as = R_AllocAnimState();
	}
	else
	{
		as = tr.animstates[as_found];
	}
	
	R_SyncRenderThread();
	
	as->ent_owner = entitynum;
	as->numBones = anims->numBones;
	as->numSurfs = numSurfs;
	as->boneanim[0].bonenum = 0;
	as->boneanim[0].oldframe = 0;
	as->boneanim[0].frame = 0;
	as->boneanim[0].slerp = 0.0;
	as->boneanim[1].bonenum = -1;
	as->boneanim[1].oldframe = 0;
	as->boneanim[1].frame = 0;
	as->boneanim[1].slerp = 0.0;
	//( 32 * anims->numBones +1 );
	/*
	as->oldframe = ri.Hunk_Alloc( ( (28*as->numBones)+4 ), h_low );
	as->newframe = ri.Hunk_Alloc( ( (28*as->numBones)+4 ), h_low );
	as->resframe = ri.Hunk_Alloc( ( (28*as->numBones)+4 ), h_low );
	as->pose = ri.Hunk_Alloc( ( 48 * as->numBones +4 ), h_low );
	as->surfList = ri.Hunk_Alloc( ( 4 * as->numSurfs +4 ), h_low );
	*/
	as->oldframe = Z_Malloc( 28*as->numBones+4 );
	as->newframe = Z_Malloc( 28*as->numBones+4 );
	as->resframe = Z_Malloc( 28*as->numBones+4 );
	as->pose = Z_Malloc( 48*as->numBones+4 );
	as->surfList = Z_Malloc( 4*as->numSurfs+4 );
	as->shaderList = Z_Malloc( 4*as->numSurfs+4 );
	as->numAllocSurfs = as->numSurfs;
	memset( as->surfList, 0, (4 * as->numSurfs) );
	memset( as->shaderList, 0, (4 * as->numSurfs) );
	
	ri.Printf( PRINT_DEVELOPER, "AnimState (%d) Created, Owner %d\n", as->index, as->ent_owner );
	return as->index;
}
#endif

qboolean RE_Ghoul2Valid( void *ptr ) {
	if(ptr) {
		CGhoul2Info_v *g2 = (CGhoul2Info_v *)ptr;
		return g2 && (&g2->anims[0] || &g2->anims[1] || &g2->anims[2] || &g2->anims[3] );
	}
	return qfalse;
}

int	RE_CopyGhoul2(void *from, void *to, int modelIndex) {
#if 0
	CGhoul2Info_v *g2from = (CGhoul2Info_v *)from;
	CGhoul2Info_v *g2to = (CGhoul2Info_v *)to;

	if(g2from && g2to && (modelIndex >= 0 || modelIndex <= 3)) {
		if(g2from->model[modelIndex]) {
			g2to->model[modelIndex] = g2from->model[modelIndex];
			Com_Memcpy(g2to->model[modelIndex], &g2from->model[modelIndex], sizeof(model_t));
			return memcmp(g2to->model[modelIndex], &g2from->model[modelIndex], sizeof(model_t)) == 0;
		}
	}
#endif
	return 0;
}
void RE_CopyGhoul2Specific(void *from, int modelFrom, void *to, int modelTo) {
#if 0
	CGhoul2Info_v *g2from = (CGhoul2Info_v *)from;
	CGhoul2Info_v *g2to = (CGhoul2Info_v *)to;

	if(g2from && g2to && (modelFrom >= 0 || modelFrom <= 3) && (modelTo >= 0 || modelTo <= 3)) {
		if(g2from->model[modelFrom]) {
			g2to->model[modelTo] = g2from->model[modelFrom];
			//Com_Memcpy(g2to->model[modelTo], &g2from->model[modelFrom], sizeof(model_t));
		}
	}
#endif
}
void RE_DuplicateGhoul2(void *from, void **to) {
#if 0
	if( to ) {
		CGhoul2Info_v *g2from = (CGhoul2Info_v *)from;
		CGhoul2Info_v *g2to;
		int i;

		if ( ( g2to = R_InitGhoul2Instance() ) == NULL ) {
			ri.Printf( PRINT_WARNING, "RE_DuplicateGhoul2: R_InitGhoul2Instance() failed\n");
			*to = NULL;
			return;
		}

		if(!g2from || !g2to) {
			*to = NULL;
			return;
		}

		for( i = 0; i < 4; i++ ) {
			if(g2from->model[i]) {
				g2to->model[i] = g2from->model[i];
				//Com_Memcpy(g2to->model[i], &g2from->model[i], sizeof(model_t));
			}
		}

		g2to->ent_owner = g2from->ent_owner;

		*to = g2to;
	}
#endif
}

qboolean RE_Ghoul2ModelOnIndex( void *ptr, int modelIndex ) {
	if(ptr && (modelIndex >= 0 || modelIndex <= 3)) {
		CGhoul2Info_v *g2 = (CGhoul2Info_v *)ptr;
		return &(g2->anims[modelIndex]) ? qtrue : qfalse;
	}
	return qfalse;
}

void RE_GetGLAName( void *ptr, int modelindex, char *buffer ) {
	Q_strncpyz(buffer, "models/players/_humanoid/_humanoid", MAX_QPATH);
#if 0
	if( !ptr || !buffer || modelindex < 0 || modelindex > 3 )
		return;

	{
		CGhoul2Info_v *g2 = (CGhoul2Info_v *)ptr;

		if(g2->anims) {
			mg_animstate_t *anim = &g2->anims[modelindex];

			if(anim) {
				Q_strncpyz(buffer, anim->animName, MAX_QPATH);
				return;
			}
		}
	}

	*buffer = '\0';
#endif
}

void RE_CleanGhoul2( void **ptr ) {
#if 0
	if( ptr ) {
		CGhoul2Info_v	*g2 = (CGhoul2Info_v *)(*ptr);

		if(g2) {
			g2->model[0] = NULL;
			g2->model[1] = NULL;
			g2->model[2] = NULL;
			g2->model[3] = NULL;

			g2->ent_owner = -1;
			g2->index = -1;

			// TODO FREE THE MODEL G2 PTR
		}
	}
#endif
}

int RE_InitGhoul2Model( void **ptr, const char *filename, int modelIndex, qhandle_t customSkin, qhandle_t customShader, int modelFlags, int lodBias ) {
	if( ptr ) {
		CGhoul2Info_v	*g2;
		qhandle_t		hG2, hModel;
		model_t			*model;
		int				k;
		if ( !filename || !filename[0] ) {
			ri.Printf( PRINT_ALL, "RE_InitGhoul2Model: NULL name\n" );
			*ptr = NULL;
			return 0;
		}

		if ( strlen( filename ) >= MAX_QPATH ) {
			ri.Printf( PRINT_ALL, "Model name exceeds MAX_QPATH\n" );
			*ptr = NULL;
			return 0;
		}

		if ( modelIndex < 0 || modelIndex > 3 ) {
			ri.Printf( PRINT_ALL, "Modelindex out of bounds\n" );
			*ptr = NULL;
			return 0;
		}

		//
		// search the currently loaded models
		//
		for ( hG2 = 1 ; hG2 < tr.numG2Instances; hG2++ ) {
			g2 = tr.g2Instances[hG2];
			for ( k = 0; k < 4 ; k++ ) {
				if ( k == modelIndex && &g2->anims[k] && !Q_stricmp( g2->anims[k].meshName, filename ) ) {
					//if( g2->anims[k]->ent_owner == -1 ) {
					//	*ptr = NULL;
					//	return 0;
					//}
					*ptr = g2;
					return hG2;
				}
			}
		}

		// allocate a new model_t

		if ( ( g2 = R_InitGhoul2Instance() ) == NULL ) {
			ri.Printf( PRINT_WARNING, "RE_InitGhoul2Model: R_InitGhoul2Instance() failed for '%s'\n", filename);
			*ptr = NULL;
			return 0;
		}

		hModel = RE_RegisterModel(filename);
		model = R_GetModelByHandle(hModel);
		if(hModel > 0 && model) {
			glmHeader_t *glm = (glmHeader_t *)model->modelData;
			if(glm) {
				g2->anims[modelIndex].meshHandle = hModel;
				Q_strncpyz(g2->anims[modelIndex].meshName, filename, sizeof(g2->anims[modelIndex].meshName));
				g2->anims[modelIndex].animHandle = glm->animIndex;
				Q_strncpyz(g2->anims[modelIndex].animName, glm->animName, sizeof(g2->anims[modelIndex].animName));
			}
		}
		else {
			*ptr = NULL;
			return 0;
		}

		// make sure the render thread is stopped
		R_SyncRenderThread();
		*ptr = g2;
		return hG2;
	}
	return 0;
}

/*
====================
RE_RegisterModel

Loads in a model for the given name

Zero will be returned if the model fails to load.
An entry will be retained for failed models as an
optimization to prevent disk rescanning if they are
asked for again.
====================
*/
qhandle_t RE_RegisterModel( const char *name ) {
	model_t		*mod;
	qhandle_t	hModel;
	qboolean	orgNameFailed = qfalse;
	int			orgLoader = -1;
	int			i;
	char		localName[ MAX_QPATH ];
	const char	*ext;
	char		altName[ MAX_QPATH ];

	if ( !name || !name[0] ) {
		ri.Printf( PRINT_ALL, "RE_RegisterModel: NULL name\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		ri.Printf( PRINT_ALL, "Model name exceeds MAX_QPATH\n" );
		return 0;
	}

	//
	// search the currently loaded models
	//
	for ( hModel = 1 ; hModel < tr.numModels; hModel++ ) {
		mod = tr.models[hModel];
		if ( !strcmp( mod->name, name ) ) {
			if( mod->type == MOD_BAD ) {
				return 0;
			}
			return hModel;
		}
	}

	// allocate a new model_t

	if ( ( mod = R_AllocModel() ) == NULL ) {
		ri.Printf( PRINT_WARNING, "RE_RegisterModel: R_AllocModel() failed for '%s'\n", name);
		return 0;
	}

	// only set the name after the model has been successfully loaded
	Q_strncpyz( mod->name, name, sizeof( mod->name ) );


	// make sure the render thread is stopped
	R_SyncRenderThread();

	mod->type = MOD_BAD;
	mod->numLods = 0;

	//
	// load the files
	//
	Q_strncpyz( localName, name, MAX_QPATH );

	ext = COM_GetExtension( localName );

	// Ghoul 2 Anims, early out because we can't try loading something else
	if( *ext && !Q_stricmp( ext, "gla" ) )
	{
		hModel = R_RegisterGLA( localName, mod );
		if( hModel )
			return mod->index;
		return hModel;
	}

	if( *ext )
	{
		// Look for the correct loader and use it
		for( i = 0; i < numModelLoaders; i++ )
		{
			if( !Q_stricmp( ext, modelLoaders[ i ].ext ) )
			{
				// Load
				hModel = modelLoaders[ i ].ModelLoader( localName, mod );
				break;
			}
		}

		// A loader was found
		if( i < numModelLoaders )
		{
			if( !hModel )
			{
				// Loader failed, most likely because the file isn't there;
				// try again without the extension
				orgNameFailed = qtrue;
				orgLoader = i;
				COM_StripExtension( name, localName, MAX_QPATH );
			}
			else
			{
				// Something loaded
				return mod->index;
			}
		}
	}

	// Try and find a suitable match using all
	// the model formats supported
	for( i = 0; i < numModelLoaders; i++ )
	{
		if (i == orgLoader)
			continue;

		Com_sprintf( altName, sizeof (altName), "%s.%s", localName, modelLoaders[ i ].ext );

		// Load
		hModel = modelLoaders[ i ].ModelLoader( altName, mod );

		if( hModel )
		{
			if( orgNameFailed )
			{
				ri.Printf( PRINT_DEVELOPER, "WARNING: %s not present, using %s instead\n",
						name, altName );
			}

			break;
		}
	}

	return hModel;
}

/*
=================
R_LoadMD3
=================
*/
static qboolean R_LoadMD3 (model_t *mod, int lod, void *buffer, const char *mod_name ) {
	int					i, j;
	md3Header_t			*pinmodel;
    md3Frame_t			*frame;
	md3Surface_t		*surf;
	md3Shader_t			*shader;
	md3Triangle_t		*tri;
	md3St_t				*st;
	md3XyzNormal_t		*xyz;
	md3Tag_t			*tag;
	int					version;
	int					size;

	pinmodel = (md3Header_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != MD3_VERSION) {
		ri.Printf( PRINT_WARNING, "R_LoadMD3: %s has wrong version (%i should be %i)\n",
				 mod_name, version, MD3_VERSION);
		return qfalse;
	}

	mod->type = MOD_MESH;
	size = LittleLong(pinmodel->ofsEnd);
	mod->dataSize += size;
	mod->md3[lod] = ri.Hunk_Alloc( size, h_low );

	Com_Memcpy (mod->md3[lod], buffer, LittleLong(pinmodel->ofsEnd) );

    LL(mod->md3[lod]->ident);
    LL(mod->md3[lod]->version);
    LL(mod->md3[lod]->numFrames);
    LL(mod->md3[lod]->numTags);
    LL(mod->md3[lod]->numSurfaces);
    LL(mod->md3[lod]->ofsFrames);
    LL(mod->md3[lod]->ofsTags);
    LL(mod->md3[lod]->ofsSurfaces);
    LL(mod->md3[lod]->ofsEnd);

	if ( mod->md3[lod]->numFrames < 1 ) {
		ri.Printf( PRINT_WARNING, "R_LoadMD3: %s has no frames\n", mod_name );
		return qfalse;
	}
    
	// swap all the frames
    frame = (md3Frame_t *) ( (byte *)mod->md3[lod] + mod->md3[lod]->ofsFrames );
    for ( i = 0 ; i < mod->md3[lod]->numFrames ; i++, frame++) {
    	frame->radius = LittleFloat( frame->radius );
        for ( j = 0 ; j < 3 ; j++ ) {
            frame->bounds[0][j] = LittleFloat( frame->bounds[0][j] );
            frame->bounds[1][j] = LittleFloat( frame->bounds[1][j] );
	    	frame->localOrigin[j] = LittleFloat( frame->localOrigin[j] );
        }
	}

	// swap all the tags
    tag = (md3Tag_t *) ( (byte *)mod->md3[lod] + mod->md3[lod]->ofsTags );
    for ( i = 0 ; i < mod->md3[lod]->numTags * mod->md3[lod]->numFrames ; i++, tag++) {
        for ( j = 0 ; j < 3 ; j++ ) {
			tag->origin[j] = LittleFloat( tag->origin[j] );
			tag->axis[0][j] = LittleFloat( tag->axis[0][j] );
			tag->axis[1][j] = LittleFloat( tag->axis[1][j] );
			tag->axis[2][j] = LittleFloat( tag->axis[2][j] );
        }
	}

	// swap all the surfaces
	surf = (md3Surface_t *) ( (byte *)mod->md3[lod] + mod->md3[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->md3[lod]->numSurfaces ; i++) {

        LL(surf->ident);
        LL(surf->flags);
        LL(surf->numFrames);
        LL(surf->numShaders);
        LL(surf->numTriangles);
        LL(surf->ofsTriangles);
        LL(surf->numVerts);
        LL(surf->ofsShaders);
        LL(surf->ofsSt);
        LL(surf->ofsXyzNormals);
        LL(surf->ofsEnd);
		
		if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
			ri.Printf(PRINT_WARNING, "R_LoadMD3: %s has more than %i verts on a surface (%i).\n",
				mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
			return qfalse;
		}
		if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) {
			ri.Printf(PRINT_WARNING, "R_LoadMD3: %s has more than %i triangles on a surface (%i).\n",
				mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
			return qfalse;
		}
	
		// change to surface identifier
		surf->ident = SF_MD3;

		// lowercase the surface name so skin compares are faster
		Q_strlwr( surf->name );

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen( surf->name );
		if ( j > 2 && surf->name[j-2] == '_' ) {
			surf->name[j-2] = 0;
		}

        // register the shaders
        shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
        for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
            shader_t	*sh;

            sh = R_FindShader( shader->name, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				shader->shaderIndex = 0;
			} else {
				shader->shaderIndex = sh->index;
			}
        }

		// swap all the triangles
		tri = (md3Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
		for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
			LL(tri->indexes[0]);
			LL(tri->indexes[1]);
			LL(tri->indexes[2]);
		}

		// swap all the ST
        st = (md3St_t *) ( (byte *)surf + surf->ofsSt );
        for ( j = 0 ; j < surf->numVerts ; j++, st++ ) {
            st->st[0] = LittleFloat( st->st[0] );
            st->st[1] = LittleFloat( st->st[1] );
        }

		// swap all the XyzNormals
        xyz = (md3XyzNormal_t *) ( (byte *)surf + surf->ofsXyzNormals );
        for ( j = 0 ; j < surf->numVerts * surf->numFrames ; j++, xyz++ ) 
		{
            xyz->xyz[0] = LittleShort( xyz->xyz[0] );
            xyz->xyz[1] = LittleShort( xyz->xyz[1] );
            xyz->xyz[2] = LittleShort( xyz->xyz[2] );

            xyz->normal = LittleShort( xyz->normal );
        }


		// find the next surface
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}
    
	return qtrue;
}


#ifdef RAVENMD4

/*
=================
R_LoadMDR
=================
*/
static qboolean R_LoadMDR( model_t *mod, void *buffer, int filesize, const char *mod_name ) 
{
	int					i, j, k, l;
	mdrHeader_t			*pinmodel, *mdr;
	mdrFrame_t			*frame;
	mdrLOD_t			*lod, *curlod;
	mdrSurface_t			*surf, *cursurf;
	mdrTriangle_t			*tri, *curtri;
	mdrVertex_t			*v, *curv;
	mdrWeight_t			*weight, *curweight;
	mdrTag_t			*tag, *curtag;
	int					size;
	shader_t			*sh;

	pinmodel = (mdrHeader_t *)buffer;

	pinmodel->version = LittleLong(pinmodel->version);
	if (pinmodel->version != MDR_VERSION) 
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has wrong version (%i should be %i)\n", mod_name, pinmodel->version, MDR_VERSION);
		return qfalse;
	}

	size = LittleLong(pinmodel->ofsEnd);
	
	if(size > filesize)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDR: Header of %s is broken. Wrong filesize declared!\n", mod_name);
		return qfalse;
	}
	
	mod->type = MOD_MDR;

	LL(pinmodel->numFrames);
	LL(pinmodel->numBones);
	LL(pinmodel->ofsFrames);

	// This is a model that uses some type of compressed Bones. We don't want to uncompress every bone for each rendered frame
	// over and over again, we'll uncompress it in this function already, so we must adjust the size of the target md4.
	if(pinmodel->ofsFrames < 0)
	{
		// mdrFrame_t is larger than mdrCompFrame_t:
		size += pinmodel->numFrames * sizeof(frame->name);
		// now add enough space for the uncompressed bones.
		size += pinmodel->numFrames * pinmodel->numBones * ((sizeof(mdrBone_t) - sizeof(mdrCompBone_t)));
	}
	
	// simple bounds check
	if(pinmodel->numBones < 0 ||
		sizeof(*mdr) + pinmodel->numFrames * (sizeof(*frame) + (pinmodel->numBones - 1) * sizeof(*frame->bones)) > size)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
		return qfalse;
	}

	mod->dataSize += size;
	mod->modelData = mdr = ri.Hunk_Alloc( size, h_low );

	// Copy all the values over from the file and fix endian issues in the process, if necessary.
	
	mdr->ident = LittleLong(pinmodel->ident);
	mdr->version = pinmodel->version;	// Don't need to swap byte order on this one, we already did above.
	Q_strncpyz(mdr->name, pinmodel->name, sizeof(mdr->name));
	mdr->numFrames = pinmodel->numFrames;
	mdr->numBones = pinmodel->numBones;
	mdr->numLODs = LittleLong(pinmodel->numLODs);
	mdr->numTags = LittleLong(pinmodel->numTags);
	// We don't care about the other offset values, we'll generate them ourselves while loading.

	mod->numLods = mdr->numLODs;

	if ( mdr->numFrames < 1 ) 
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has no frames\n", mod_name);
		return qfalse;
	}

	/* The first frame will be put into the first free space after the header */
	frame = (mdrFrame_t *)(mdr + 1);
	mdr->ofsFrames = (int)((byte *) frame - (byte *) mdr);
		
	if (pinmodel->ofsFrames < 0)
	{
		mdrCompFrame_t *cframe;
				
		// compressed model...				
		cframe = (mdrCompFrame_t *)((byte *) pinmodel - pinmodel->ofsFrames);
		
		for(i = 0; i < mdr->numFrames; i++)
		{
			for(j = 0; j < 3; j++)
			{
				frame->bounds[0][j] = LittleFloat(cframe->bounds[0][j]);
				frame->bounds[1][j] = LittleFloat(cframe->bounds[1][j]);
				frame->localOrigin[j] = LittleFloat(cframe->localOrigin[j]);
			}

			frame->radius = LittleFloat(cframe->radius);
			frame->name[0] = '\0';	// No name supplied in the compressed version.
			
			for(j = 0; j < mdr->numBones; j++)
			{
				for(k = 0; k < (sizeof(cframe->bones[j].Comp) / 2); k++)
				{
					// Do swapping for the uncompressing functions. They seem to use shorts
					// values only, so I assume this will work. Never tested it on other
					// platforms, though.
					
					((unsigned short *)(cframe->bones[j].Comp))[k] =
						LittleShort( ((unsigned short *)(cframe->bones[j].Comp))[k] );
				}
				
				/* Now do the actual uncompressing */
				MC_UnCompress(frame->bones[j].matrix, cframe->bones[j].Comp);
			}
			
			// Next Frame...
			cframe = (mdrCompFrame_t *) &cframe->bones[j];
			frame = (mdrFrame_t *) &frame->bones[j];
		}
	}
	else
	{
		mdrFrame_t *curframe;
		
		// uncompressed model...
		//
    
		curframe = (mdrFrame_t *)((byte *) pinmodel + pinmodel->ofsFrames);
		
		// swap all the frames
		for ( i = 0 ; i < mdr->numFrames ; i++) 
		{
			for(j = 0; j < 3; j++)
			{
				frame->bounds[0][j] = LittleFloat(curframe->bounds[0][j]);
				frame->bounds[1][j] = LittleFloat(curframe->bounds[1][j]);
				frame->localOrigin[j] = LittleFloat(curframe->localOrigin[j]);
			}
			
			frame->radius = LittleFloat(curframe->radius);
			Q_strncpyz(frame->name, curframe->name, sizeof(frame->name));
			
			for (j = 0; j < (int) (mdr->numBones * sizeof(mdrBone_t) / 4); j++) 
			{
				((float *)frame->bones)[j] = LittleFloat( ((float *)curframe->bones)[j] );
			}
			
			curframe = (mdrFrame_t *) &curframe->bones[mdr->numBones];
			frame = (mdrFrame_t *) &frame->bones[mdr->numBones];
		}
	}
	
	// frame should now point to the first free address after all frames.
	lod = (mdrLOD_t *) frame;
	mdr->ofsLODs = (int) ((byte *) lod - (byte *)mdr);
	
	curlod = (mdrLOD_t *)((byte *) pinmodel + LittleLong(pinmodel->ofsLODs));
		
	// swap all the LOD's
	for ( l = 0 ; l < mdr->numLODs ; l++)
	{
		// simple bounds check
		if((byte *) (lod + 1) > (byte *) mdr + size)
		{
			ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
			return qfalse;
		}

		lod->numSurfaces = LittleLong(curlod->numSurfaces);
		
		// swap all the surfaces
		surf = (mdrSurface_t *) (lod + 1);
		lod->ofsSurfaces = (int)((byte *) surf - (byte *) lod);
		cursurf = (mdrSurface_t *) ((byte *)curlod + LittleLong(curlod->ofsSurfaces));
		
		for ( i = 0 ; i < lod->numSurfaces ; i++)
		{
			// simple bounds check
			if((byte *) (surf + 1) > (byte *) mdr + size)
			{
				ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
				return qfalse;
			}

			// first do some copying stuff
			
			surf->ident = SF_MDR;
			Q_strncpyz(surf->name, cursurf->name, sizeof(surf->name));
			Q_strncpyz(surf->shader, cursurf->shader, sizeof(surf->shader));
			
			surf->ofsHeader = (byte *) mdr - (byte *) surf;
			
			surf->numVerts = LittleLong(cursurf->numVerts);
			surf->numTriangles = LittleLong(cursurf->numTriangles);
			// numBoneReferences and BoneReferences generally seem to be unused
			
			// now do the checks that may fail.
			if ( surf->numVerts > SHADER_MAX_VERTEXES ) 
			{
				ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has more than %i verts on a surface (%i).\n",
					  mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
				return qfalse;
			}
			if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) 
			{
				ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has more than %i triangles on a surface (%i).\n",
					  mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
				return qfalse;
			}
			// lowercase the surface name so skin compares are faster
			Q_strlwr( surf->name );

			// register the shaders
			sh = R_FindShader(surf->shader, LIGHTMAP_NONE, qtrue);
			if ( sh->defaultShader ) {
				surf->shaderIndex = 0;
			} else {
				surf->shaderIndex = sh->index;
			}
			
			// now copy the vertexes.
			v = (mdrVertex_t *) (surf + 1);
			surf->ofsVerts = (int)((byte *) v - (byte *) surf);
			curv = (mdrVertex_t *) ((byte *)cursurf + LittleLong(cursurf->ofsVerts));
			
			for(j = 0; j < surf->numVerts; j++)
			{
				LL(curv->numWeights);
			
				// simple bounds check
				if(curv->numWeights < 0 || (byte *) (v + 1) + (curv->numWeights - 1) * sizeof(*weight) > (byte *) mdr + size)
				{
					ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
					return qfalse;
				}

				v->normal[0] = LittleFloat(curv->normal[0]);
				v->normal[1] = LittleFloat(curv->normal[1]);
				v->normal[2] = LittleFloat(curv->normal[2]);
				
				v->texCoords[0] = LittleFloat(curv->texCoords[0]);
				v->texCoords[1] = LittleFloat(curv->texCoords[1]);
				
				v->numWeights = curv->numWeights;
				weight = &v->weights[0];
				curweight = &curv->weights[0];
				
				// Now copy all the weights
				for(k = 0; k < v->numWeights; k++)
				{
					weight->boneIndex = LittleLong(curweight->boneIndex);
					weight->boneWeight = LittleFloat(curweight->boneWeight);
					
					weight->offset[0] = LittleFloat(curweight->offset[0]);
					weight->offset[1] = LittleFloat(curweight->offset[1]);
					weight->offset[2] = LittleFloat(curweight->offset[2]);
					
					weight++;
					curweight++;
				}
				
				v = (mdrVertex_t *) weight;
				curv = (mdrVertex_t *) curweight;
			}
						
			// we know the offset to the triangles now:
			tri = (mdrTriangle_t *) v;
			surf->ofsTriangles = (int)((byte *) tri - (byte *) surf);
			curtri = (mdrTriangle_t *)((byte *) cursurf + LittleLong(cursurf->ofsTriangles));
			
			// simple bounds check
			if(surf->numTriangles < 0 || (byte *) (tri + surf->numTriangles) > (byte *) mdr + size)
			{
				ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
				return qfalse;
			}

			for(j = 0; j < surf->numTriangles; j++)
			{
				tri->indexes[0] = LittleLong(curtri->indexes[0]);
				tri->indexes[1] = LittleLong(curtri->indexes[1]);
				tri->indexes[2] = LittleLong(curtri->indexes[2]);
				
				tri++;
				curtri++;
			}
			
			// tri now points to the end of the surface.
			surf->ofsEnd = (byte *) tri - (byte *) surf;
			surf = (mdrSurface_t *) tri;

			// find the next surface.
			cursurf = (mdrSurface_t *) ((byte *) cursurf + LittleLong(cursurf->ofsEnd));
		}

		// surf points to the next lod now.
		lod->ofsEnd = (int)((byte *) surf - (byte *) lod);
		lod = (mdrLOD_t *) surf;

		// find the next LOD.
		curlod = (mdrLOD_t *)((byte *) curlod + LittleLong(curlod->ofsEnd));
	}
	
	// lod points to the first tag now, so update the offset too.
	tag = (mdrTag_t *) lod;
	mdr->ofsTags = (int)((byte *) tag - (byte *) mdr);
	curtag = (mdrTag_t *) ((byte *)pinmodel + LittleLong(pinmodel->ofsTags));

	// simple bounds check
	if(mdr->numTags < 0 || (byte *) (tag + mdr->numTags) > (byte *) mdr + size)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
		return qfalse;
	}
	
	for (i = 0 ; i < mdr->numTags ; i++)
	{
		tag->boneIndex = LittleLong(curtag->boneIndex);
		Q_strncpyz(tag->name, curtag->name, sizeof(tag->name));
		
		tag++;
		curtag++;
	}
	
	// And finally we know the real offset to the end.
	mdr->ofsEnd = (int)((byte *) tag - (byte *) mdr);

	// phew! we're done.
	
	return qtrue;
}
#endif

/*
=================
R_LoadMD4
=================
*/

static qboolean R_LoadMD4( model_t *mod, void *buffer, const char *mod_name ) {
	int					i, j, k, lodindex;
	md4Header_t			*pinmodel, *md4;
    md4Frame_t			*frame;
	md4LOD_t			*lod;
	md4Surface_t		*surf;
	md4Triangle_t		*tri;
	md4Vertex_t			*v;
	int					version;
	int					size;
	shader_t			*sh;
	int					frameSize;

	pinmodel = (md4Header_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != MD4_VERSION) {
		ri.Printf( PRINT_WARNING, "R_LoadMD4: %s has wrong version (%i should be %i)\n",
				 mod_name, version, MD4_VERSION);
		return qfalse;
	}

	mod->type = MOD_MD4;
	size = LittleLong(pinmodel->ofsEnd);
	mod->dataSize += size;
	mod->modelData = md4 = ri.Hunk_Alloc( size, h_low );

	Com_Memcpy(md4, buffer, size);

    LL(md4->ident);
    LL(md4->version);
    LL(md4->numFrames);
    LL(md4->numBones);
    LL(md4->numLODs);
    LL(md4->ofsFrames);
    LL(md4->ofsLODs);
    md4->ofsEnd = size;

	if ( md4->numFrames < 1 ) {
		ri.Printf( PRINT_WARNING, "R_LoadMD4: %s has no frames\n", mod_name );
		return qfalse;
	}

    // we don't need to swap tags in the renderer, they aren't used
    
	// swap all the frames
	frameSize = (size_t)( &((md4Frame_t *)0)->bones[ md4->numBones ] );
    for ( i = 0 ; i < md4->numFrames ; i++) {
	    frame = (md4Frame_t *) ( (byte *)md4 + md4->ofsFrames + i * frameSize );
    	frame->radius = LittleFloat( frame->radius );
        for ( j = 0 ; j < 3 ; j++ ) {
            frame->bounds[0][j] = LittleFloat( frame->bounds[0][j] );
            frame->bounds[1][j] = LittleFloat( frame->bounds[1][j] );
	    	frame->localOrigin[j] = LittleFloat( frame->localOrigin[j] );
        }
		for ( j = 0 ; j < md4->numBones * sizeof( md4Bone_t ) / 4 ; j++ ) {
			((float *)frame->bones)[j] = LittleFloat( ((float *)frame->bones)[j] );
		}
	}

	// swap all the LOD's
	lod = (md4LOD_t *) ( (byte *)md4 + md4->ofsLODs );
	for ( lodindex = 0 ; lodindex < md4->numLODs ; lodindex++ ) {

		// swap all the surfaces
		surf = (md4Surface_t *) ( (byte *)lod + lod->ofsSurfaces );
		for ( i = 0 ; i < lod->numSurfaces ; i++) {
			LL(surf->ident);
			LL(surf->numTriangles);
			LL(surf->ofsTriangles);
			LL(surf->numVerts);
			LL(surf->ofsVerts);
			LL(surf->ofsEnd);
			
			if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
				ri.Printf(PRINT_WARNING, "R_LoadMD4: %s has more than %i verts on a surface (%i).\n",
					mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
				return qfalse;
			}
			if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) {
				ri.Printf(PRINT_WARNING, "R_LoadMD4: %s has more than %i triangles on a surface (%i).\n",
					mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
				return qfalse;
			}

			// change to surface identifier
			surf->ident = SF_MD4;

			// lowercase the surface name so skin compares are faster
			Q_strlwr( surf->name );
		
			// register the shaders
			sh = R_FindShader( surf->shader, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				surf->shaderIndex = 0;
			} else {
				surf->shaderIndex = sh->index;
			}

			// swap all the triangles
			tri = (md4Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
			for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
				LL(tri->indexes[0]);
				LL(tri->indexes[1]);
				LL(tri->indexes[2]);
			}

			// swap all the vertexes
			// FIXME
			// This makes TFC's skeletons work.  Shouldn't be necessary anymore, but left
			// in for reference.
			//v = (md4Vertex_t *) ( (byte *)surf + surf->ofsVerts + 12);
			v = (md4Vertex_t *) ( (byte *)surf + surf->ofsVerts);
			for ( j = 0 ; j < surf->numVerts ; j++ ) {
				v->normal[0] = LittleFloat( v->normal[0] );
				v->normal[1] = LittleFloat( v->normal[1] );
				v->normal[2] = LittleFloat( v->normal[2] );

				v->texCoords[0] = LittleFloat( v->texCoords[0] );
				v->texCoords[1] = LittleFloat( v->texCoords[1] );

				v->numWeights = LittleLong( v->numWeights );

				for ( k = 0 ; k < v->numWeights ; k++ ) {
					v->weights[k].boneIndex = LittleLong( v->weights[k].boneIndex );
					v->weights[k].boneWeight = LittleFloat( v->weights[k].boneWeight );
				   v->weights[k].offset[0] = LittleFloat( v->weights[k].offset[0] );
				   v->weights[k].offset[1] = LittleFloat( v->weights[k].offset[1] );
				   v->weights[k].offset[2] = LittleFloat( v->weights[k].offset[2] );
				}
				// FIXME
				// This makes TFC's skeletons work.  Shouldn't be necessary anymore, but left
				// in for reference.
				//v = (md4Vertex_t *)( ( byte * )&v->weights[v->numWeights] + 12 );
				v = (md4Vertex_t *)( ( byte * )&v->weights[v->numWeights]);
			}

			// find the next surface
			surf = (md4Surface_t *)( (byte *)surf + surf->ofsEnd );
		}

		// find the next LOD
		lod = (md4LOD_t *)( (byte *)lod + lod->ofsEnd );
	}

	return qtrue;
}



//=============================================================================

/*
** RE_BeginRegistration
*/
void RE_BeginRegistration( glconfig_t *glconfigOut, glconfig2_t *glconfig2Out ) {

	R_Init();

	*glconfigOut = glConfig;
	*glconfig2Out = glConfig2;

	R_SyncRenderThread();

	tr.viewCluster = -1;		// force markleafs to regenerate
	R_ClearFlares();
	RE_ClearScene();

	tr.registered = qtrue;

	// NOTE: this sucks, for some reason the first stretch pic is never drawn
	// without this we'd see a white flash on a level load because the very
	// first time the level shot would not be drawn
//	RE_StretchPic(0, 0, 0, 0, 0, 0, 1, 1, 0);
}

//=============================================================================

/*
===============
R_ModelInit
===============
*/
void R_ModelInit( void ) {
	model_t		*mod;

	// leave a space for NULL model
	tr.numModels = 0;

	mod = R_AllocModel();
	mod->type = MOD_BAD;
}


/*
================
R_Modellist_f
================
*/
void R_Modellist_f( void ) {
	int		i, j;
	model_t	*mod;
	int		total;
	int		lods;

	total = 0;
	for ( i = 1 ; i < tr.numModels; i++ ) {
		mod = tr.models[i];
		lods = 1;
		for ( j = 1 ; j < MD3_MAX_LODS ; j++ ) {
			if ( mod->md3[j] && mod->md3[j] != mod->md3[j-1] ) {
				lods++;
			}
		}
		ri.Printf( PRINT_ALL, "%8i : (%i) %s\n",mod->dataSize, lods, mod->name );
		total += mod->dataSize;
	}
	ri.Printf( PRINT_ALL, "%8i : Total models\n", total );

#if	0		// not working right with new hunk
	if ( tr.world ) {
		ri.Printf( PRINT_ALL, "\n%8i : %s\n", tr.world->dataSize, tr.world->name );
	}
#endif
}


//=============================================================================


/*
================
R_GetTag
================
*/
static md3Tag_t *R_GetTag( md3Header_t *mod, int frame, const char *tagName ) {
	md3Tag_t		*tag;
	int				i;

	if ( frame >= mod->numFrames ) {
		// it is possible to have a bad frame while changing models, so don't error
		frame = mod->numFrames - 1;
	}

	tag = (md3Tag_t *)((byte *)mod + mod->ofsTags) + frame * mod->numTags;
	for ( i = 0 ; i < mod->numTags ; i++, tag++ ) {
		if ( !strcmp( tag->name, tagName ) ) {
			return tag;	// found it
		}
	}

	return NULL;
}

#ifdef RAVENMD4
void R_GetAnimTag( mdrHeader_t *mod, int framenum, const char *tagName, md3Tag_t * dest) 
{
	int				i, j, k;
	int				frameSize;
	mdrFrame_t		*frame;
	mdrTag_t		*tag;

	if ( framenum >= mod->numFrames ) 
	{
		// it is possible to have a bad frame while changing models, so don't error
		framenum = mod->numFrames - 1;
	}

	tag = (mdrTag_t *)((byte *)mod + mod->ofsTags);
	for ( i = 0 ; i < mod->numTags ; i++, tag++ )
	{
		if ( !strcmp( tag->name, tagName ) )
		{
			Q_strncpyz(dest->name, tag->name, sizeof(dest->name));

			// uncompressed model...
			//
			frameSize = (intptr_t)( &((mdrFrame_t *)0)->bones[ mod->numBones ] );
			frame = (mdrFrame_t *)((byte *)mod + mod->ofsFrames + framenum * frameSize );

			for (j = 0; j < 3; j++)
			{
				for (k = 0; k < 3; k++)
					dest->axis[j][k]=frame->bones[tag->boneIndex].matrix[k][j];
			}

			dest->origin[0]=frame->bones[tag->boneIndex].matrix[0][3];
			dest->origin[1]=frame->bones[tag->boneIndex].matrix[1][3];
			dest->origin[2]=frame->bones[tag->boneIndex].matrix[2][3];				

			return;
		}
	}

	AxisClear( dest->axis );
	VectorClear( dest->origin );
	strcpy(dest->name,"");
}
#endif

/*
================
R_LerpTag
================
*/
int R_LerpTag( orientation_t *tag, qhandle_t handle, int startFrame, int endFrame, 
					 float frac, const char *tagName ) {
	md3Tag_t	*start, *end;
#ifdef RAVENMD4
	md3Tag_t	start_space, end_space;
#endif
	int		i;
	float		frontLerp, backLerp;
	model_t		*model;

	model = R_GetModelByHandle( handle );
	if ( !model->md3[0] )
	{
#ifdef RAVENMD4
		if(model->type == MOD_MDR)
		{
			start = &start_space;
			end = &end_space;
			R_GetAnimTag((mdrHeader_t *) model->modelData, startFrame, tagName, start);
			R_GetAnimTag((mdrHeader_t *) model->modelData, endFrame, tagName, end);
		}
		else
#endif
		if( model->type == MOD_IQM ) {
			return R_IQMLerpTag( tag, (iqmData_t *) model->modelData,
					startFrame, endFrame,
					frac, tagName );
		} else if( model->type == MOD_GLM ) {
			//return R_
		} else {

			AxisClear( tag->axis );
			VectorClear( tag->origin );
			return qfalse;

		}
	}
	else
	{
		start = R_GetTag( model->md3[0], startFrame, tagName );
		end = R_GetTag( model->md3[0], endFrame, tagName );
		if ( !start || !end ) {
			AxisClear( tag->axis );
			VectorClear( tag->origin );
			return qfalse;
		}
	}
	
	frontLerp = frac;
	backLerp = 1.0f - frac;

	for ( i = 0 ; i < 3 ; i++ ) {
		tag->origin[i] = start->origin[i] * backLerp +  end->origin[i] * frontLerp;
		tag->axis[0][i] = start->axis[0][i] * backLerp +  end->axis[0][i] * frontLerp;
		tag->axis[1][i] = start->axis[1][i] * backLerp +  end->axis[1][i] * frontLerp;
		tag->axis[2][i] = start->axis[2][i] * backLerp +  end->axis[2][i] * frontLerp;
	}
	VectorNormalize( tag->axis[0] );
	VectorNormalize( tag->axis[1] );
	VectorNormalize( tag->axis[2] );
	return qtrue;
}


/*
====================
R_ModelBounds
====================
*/
void R_ModelBounds( qhandle_t handle, vec3_t mins, vec3_t maxs ) {
	model_t		*model;

	model = R_GetModelByHandle( handle );

	if(model->type == MOD_BRUSH) {
		VectorCopy( model->bmodel->bounds[0], mins );
		VectorCopy( model->bmodel->bounds[1], maxs );
		
		return;
	} else if (model->type == MOD_MESH) {
		md3Header_t	*header;
		md3Frame_t	*frame;

		header = model->md3[0];
		frame = (md3Frame_t *) ((byte *)header + header->ofsFrames);

		VectorCopy( frame->bounds[0], mins );
		VectorCopy( frame->bounds[1], maxs );
		
		return;
	} else if (model->type == MOD_MD4) {
		md4Header_t	*header;
		md4Frame_t	*frame;

		header = (md4Header_t *)model->modelData;
		frame = (md4Frame_t *) ((byte *)header + header->ofsFrames);

		VectorCopy( frame->bounds[0], mins );
		VectorCopy( frame->bounds[1], maxs );
		
		return;
#ifdef RAVENMD4
	} else if (model->type == MOD_MDR) {
		mdrHeader_t	*header;
		mdrFrame_t	*frame;

		header = (mdrHeader_t *)model->modelData;
		frame = (mdrFrame_t *) ((byte *)header + header->ofsFrames);

		VectorCopy( frame->bounds[0], mins );
		VectorCopy( frame->bounds[1], maxs );
		
		return;
#endif
	} else if(model->type == MOD_IQM) {
		iqmData_t *iqmData;
		
		iqmData = (iqmData_t *)model->modelData;

		if(iqmData->bounds)
		{
			VectorCopy(iqmData->bounds, mins);
			VectorCopy(iqmData->bounds + 3, maxs);
			return;
		}
	}

	VectorClear( mins );
	VectorClear( maxs );
}
