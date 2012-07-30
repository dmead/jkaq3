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

#define LL( x ) x = LittleLong( x )
#define LF( f ) f = LittleFloat( f )

static qboolean GLM_CheckRange( glmHeader_t *header, int offset,
			    int count, int size ) {
	// return true if the range specified by offset, count and size
	// doesn't fit into the file
	return ( count <= 0 ||
		 offset < 0 ||
			  offset > header->ofsEnd ||
		 offset + count * size < 0 ||
					 offset + count * size > header->ofsEnd );
}

void Matrix34VectorRotate( vec4_t mata[3], vec3_t matb, vec3_t result )
{
	result[0] = (mata[0][0] * matb[0]) + (mata[0][1] * matb[1]) + (mata[0][2] * matb[2]);
	result[1] = (mata[1][0] * matb[0]) + (mata[1][1] * matb[1]) + (mata[1][2] * matb[2]);
	result[2] = (mata[2][0] * matb[0]) + (mata[2][1] * matb[1]) + (mata[2][2] * matb[2]);
}

qboolean R_LoadGLM( model_t *mod, void *buffer, int filesize, const char *mod_name ) {
	int					i, j, lodindex, surfindex;
	glmHeader_t		*header, *glm;
	glmLOD_t			*lod;
	glmLODSurfOffset_t	*lod_surf_ofs;
	glmSurfHierarchy_t	*surfh;
	glmVertexTexCoord_t *vcor;
	glmSurface_t		*surf;
	glmTriangle_t		*tri;
	glmVertex_t		*v;
	int					version;
	int					size;
	shader_t			*sh;
	vec3_t				tempVert;
	vec3_t				tempNorm;
	//int					frameSize;

	float zrots[3][4];
	qboolean do_zrot = qfalse;
	zrots[0][0] = 0.0;
	zrots[0][1] = -1.0;
	zrots[0][2] = 0.0;
	zrots[0][3] = 0.0;
	zrots[1][0] = 1.0;
	zrots[1][1] = 0.0;
	zrots[1][2] = 0.0;
	zrots[1][3] = 0.0;
	zrots[2][0] = 0.0;
	zrots[2][1] = 0.0;
	zrots[2][2] = 1.0;
	zrots[2][3] = 0.0;

	if( filesize < sizeof( glmHeader_t ) ) {
		return qfalse;
	}

	header = (glmHeader_t *)buffer;

	version = LittleLong( header->version );

	if( version != GLM_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadGLM: %s has wrong version (%i should be %i)\n", mod_name, version, GLM_VERSION );
		return qfalse;
	}

	size = LittleLong( header->ofsEnd );

	if( size > filesize ) {
		ri.Printf( PRINT_WARNING, "R_LoadGLM: Header of %s is broken. Wrong filesize declared!\n", mod_name );
		return qfalse;
	}

	mod->type = MOD_GLM;
	size = LittleLong(header->ofsEnd);
	mod->dataSize += size;
	//ri.Printf( PRINT_WARNING, "R_LoadGLM: %s alloc %d\n", mod_name, size );
	mod->modelData = ri.Hunk_Alloc( size, h_low );
	glm = (glmHeader_t *)mod->modelData;

	Com_Memcpy( glm, buffer, size );

	LL( glm->ident );
	LL( glm->animIndex );
	LL( glm->numBones );
	LL( glm->numLODs );
	LL( glm->ofsLODs );
	LL( glm->numSurfaces );
	LL( glm->ofsSurfHierarchy );

	if ( GLM_CheckRange( glm, glm->ofsSurfHierarchy, glm->numSurfaces, sizeof( glmSurfHierarchy_t ) ) ) {
		return qfalse;
	}

	// swap the surf Hierarchy!
	surfh = (glmSurfHierarchy_t *) ( (byte *)glm + glm->ofsSurfHierarchy );
	for( surfindex = 0; surfindex < glm->numSurfaces; surfindex++ )
	{
		Q_strlwr( surfh->name );
		LL(surfh->flags);
		Q_strlwr( surfh->shader );
		
		//ri.Printf( PRINT_ALL, "surf %d, name '%s' shader '%s'\n", surfindex, surfh->name, surfh->shader );
		
		sh = R_FindShader( surfh->shader, LIGHTMAP_NONE, qtrue );
		if ( sh->defaultShader ) {
			surfh->shaderIndex = 0;
		} else {
			LL(surfh->shaderIndex);
			surfh->shaderIndex = sh->index;
		}
		
		LL(surfh->parentIndex);
		LL(surfh->numChildren);
		for( i = 0; i < surfh->numChildren; i++ ) {
			LL(surfh->childIndexes[i]);
		}
		surfh = (glmSurfHierarchy_t *) ( (byte *)&surfh->childIndexes[surfh->numChildren] );
	}

	if ( GLM_CheckRange( glm, glm->ofsLODs, glm->numLODs, sizeof( glmLOD_t ) ) ) {
		return qfalse;
	}

	// swap all the LOD's
	lod = (glmLOD_t *) ( (byte *)glm + glm->ofsLODs );
	LL( lod->ofsEnd );
	//ri.Printf (PRINT_WARNING,"RE_RegisterModel: couldn't load %s\n", name);
	//ri.Printf (PRINT_WARNING,"lod ofsEnd is 0x%08x\n", lod->ofsEnd );
	for ( lodindex = 0 ; lodindex < glm->numLODs ; lodindex++ )
	{
		lod_surf_ofs = ( glmLODSurfOffset_t *) ((byte *)lod + 4 );
		for( i = 0; i < glm->numSurfaces; i++ )
		{
			LL(lod_surf_ofs->offsets[i]);
			//lod_surf_ofs->offsets[i] += 4;
			//printf( "lod %d surf ofs %d is 0x%08x\n", lodindex, lod_surf_ofs->offsets[i] );
		}
		
		// swap all the surfaces
		for ( i = 0 ; i < glm->numSurfaces ; i++) {
			surf = (glmSurface_t *) ( (byte *)lod_surf_ofs + lod_surf_ofs->offsets[i] );
			LL(surf->ident);
			LL(surf->thisSurfaceIndex);
			LL(surf->ofsHeader);
			LL(surf->numVerts);
			LL(surf->ofsVerts);
			LL(surf->numTriangles);
			LL(surf->ofsTriangles);
			LL(surf->numBoneReferences);
			LL(surf->ofsBoneReferences);
			LL(surf->ofsEnd);
			if(0) ri.Printf( PRINT_ALL, "surf %d, ident %d, index %d, numverts %d, numtris %d, num bone refs %d\n", i, surf->ident, surf->thisSurfaceIndex, surf->numVerts, surf->numTriangles, surf->numBoneReferences );
			
			// change to surface identifier
			surf->ident = SF_GLM;
			
			tri = (glmTriangle_t *) ( (byte *)surf + surf->ofsBoneReferences );
			for( j=0; j<surf->numBoneReferences; j++ )
			{
				LL(tri->indexes[j]);
				if(0) ri.Printf( PRINT_ALL, "j is %d\n", j );
				if(0) ri.Printf( PRINT_ALL, "surf %d, boneref %d is %d\n", i, j, tri->indexes[j] );
			}
			
			if(0) ri.Printf( PRINT_ALL, "swap the triangles!\n" );

			// swap all the triangles
			tri = (glmTriangle_t *) ( (byte *)surf + surf->ofsTriangles );
			for ( j = 0 ; j < surf->numTriangles ; j++ ) {
				LL(tri->indexes[0]);
				LL(tri->indexes[1]);
				LL(tri->indexes[2]);
				tri = (glmTriangle_t *)((byte *)tri + 12 );
			}
			
			
			v = (glmVertex_t *) ( (byte *)surf + surf->ofsVerts);
			for ( j = 0 ; j < surf->numVerts ; j++ )
			{
				v->normal[0] = LittleFloat( v->normal[0] );
				v->normal[1] = LittleFloat( v->normal[1] );
				v->normal[2] = LittleFloat( v->normal[2] );
				v->vertCoords[0] = LittleFloat( v->vertCoords[0] );
				v->vertCoords[1] = LittleFloat( v->vertCoords[1] );
				v->vertCoords[2] = LittleFloat( v->vertCoords[2] );
				
				if( do_zrot )
				{
					VectorCopy( v->normal, tempNorm );
					VectorCopy( v->vertCoords, tempVert );
					Matrix34VectorRotate( zrots, tempVert, v->vertCoords );
					Matrix34VectorRotate( zrots, tempNorm, v->normal );
				}
				
				LL(v->uiNmWeightsAndBoneIndexes);
				//v->texCoords[0] = LittleFloat( v->texCoords[0] );
				//v->texCoords[1] = LittleFloat( v->texCoords[1] );
				/*
				v->numWeights = LittleLong( v->numWeights );

				for ( k = 0 ; k < v->numWeights ; k++ ) {
					v->weights[k].boneIndex = LittleLong( v->weights[k].boneIndex );
					v->weights[k].boneWeight = LittleFloat( v->weights[k].boneWeight );
				   v->weights[k].offset[0] = LittleFloat( v->weights[k].offset[0] );
				   v->weights[k].offset[1] = LittleFloat( v->weights[k].offset[1] );
				   v->weights[k].offset[2] = LittleFloat( v->weights[k].offset[2] );
				}
				*/
				v = (glmVertex_t *)( ( byte *)v + 32 );
			}
			vcor = (glmVertexTexCoord_t *)( ( byte *)v );
			for ( j = 0 ; j < surf->numVerts ; j++ )
			{
				vcor->texCoords[0] = LittleFloat( vcor->texCoords[0] );
				vcor->texCoords[1] = LittleFloat( vcor->texCoords[1] );
				vcor = (glmVertexTexCoord_t *)( ( byte *)vcor + 8 );
			}
		}
		lod = (glmLOD_t *)( (byte *)lod + lod->ofsEnd );
	}

	glm->animIndex = 0;
	if(0) ri.Printf( PRINT_DEVELOPER, "glm (%s) animname is '%s'\n", mod_name, glm->animName );
	if( Q_strncmp( glm->animName, sDEFAULT_GLA_NAME, strlen(sDEFAULT_GLA_NAME) ) ) //if there's a difference
	{
		Q_strcat( glm->animName, MAX_QPATH, ".gla" );
		//Com_DPrintf ( "glm: trying to load animfile: '%s'\n", glm->animName );
		//ri.Printf( PRINT_ALL, "glm: trying to load animfile: '%s'\n", glm->animName );
		glm->animIndex = RE_RegisterModel( glm->animName );
	}
	else
	{
		glm->animIndex = 0;
	}

	return qtrue;
}

static qboolean GLA_CheckRange( glaHeader_t *header, int offset,
			    int count, int size ) {
	// return true if the range specified by offset, count and size
	// doesn't fit into the file
	return ( count <= 0 ||
		 offset < 0 ||
			  offset > header->ofsEnd ||
		 offset + count * size < 0 ||
					 offset + count * size > header->ofsEnd );
}

qboolean R_LoadGLA( model_t *mod, void *buffer, int filesize, const char *mod_name )
{
	int					i, j;
	glaHeader_t			*header, *gla;
	//glaSkelOffsets_t *skel_ofs;
	glaSkel_t	*skel;
	int numComps = 0;
	glaCompBone_t *compBone;
    //glmFrame_t			*frame;
	/*
	glmLOD_t			*lod;
	glmLODSurfOffset_t	*lod_surf_ofs;
	glmSurfHierarchy_t	*surfh;
	glmVertexTexCoord_t *vcor;
	glmSurface_t		*surf;
	glmTriangle_t		*tri;
	glmVertex_t			*v;
	*/
	int					version;
	int					size;
	vec4_t zrots[3];
	//vec4_t finRot[3];
	qboolean do_zrot = qtrue;
	
	zrots[0][0] = 0.0;
	zrots[0][1] = -1.0;
	zrots[0][2] = 0.0;
	zrots[0][3] = 0.0;
	zrots[1][0] = 1.0;
	zrots[1][1] = 0.0;
	zrots[1][2] = 0.0;
	zrots[1][3] = 0.0;
	zrots[2][0] = 0.0;
	zrots[2][1] = 0.0;
	zrots[2][2] = 1.0;
	zrots[2][3] = 0.0;

	if( filesize < sizeof( glaHeader_t ) ) {
		return qfalse;
	}

	header = (glaHeader_t *)buffer;

	version = LittleLong( header->version );

	if( version != GLA_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadGLA: %s has wrong version (%i should be %i)\n", mod_name, version, GLA_VERSION );
		return qfalse;
	}

	size = LittleLong( header->ofsEnd );

	if( size > filesize ) {
		ri.Printf( PRINT_WARNING, "R_LoadGLA: Header of %s is broken. Wrong filesize declared!\n", mod_name );
		return qfalse;
	}
	
	mod->type = MOD_GLA;
	size = LittleLong(header->ofsEnd);
	mod->dataSize += size;
	gla = mod->modelData = ri.Hunk_Alloc( size, h_low );

	Com_Memcpy( gla, buffer, LittleLong(header->ofsEnd) );

    LL(gla->ident);
    LL(gla->version);
	gla->fScale = LittleFloat( gla->fScale );
    LL(gla->numFrames);
    LL(gla->ofsFrames);
    LL(gla->numBones);
    LL(gla->ofsCompBonePool);
    LL(gla->ofsSkel);
    LL(gla->ofsEnd);
	
	/*
	skel_ofs = (glaSkelOffsets_t *) ( (byte *)&gla + sizeof( glaHeader_t ));
	for( i = 0; i < gla->numBones; i++ )
	{
		LL(skel_ofs->offsets[i]);
	}
	*/
	
#if 0
	if ( GLA_CheckRange( gla, gla->ofsSkel, gla->numBones, sizeof( glaSkel_t ) ) ) {
		return qfalse;
	}
#endif
	
	skel = (glaSkel_t *)((byte *)gla + gla->ofsSkel);
	for( i = 0; i < gla->numBones; i++ )
	{
		ri.Printf( PRINT_DEVELOPER, "R_LoadGLA: bone %d, %s\n", i, skel->name );
		LL(skel->flags);
		LL(skel->parent);
		/*
		skel->BasePoseMat.matrix[0][0] = LittleFloat( skel->BasePoseMat.matrix[0][0] ) / gla->fScale;
		skel->BasePoseMat.matrix[0][1] = LittleFloat( skel->BasePoseMat.matrix[0][1] ) / gla->fScale;
		skel->BasePoseMat.matrix[0][2] = LittleFloat( skel->BasePoseMat.matrix[0][2] ) / gla->fScale;
		skel->BasePoseMat.matrix[0][3] = LittleFloat( skel->BasePoseMat.matrix[0][3] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][0] = LittleFloat( skel->BasePoseMat.matrix[1][0] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][1] = LittleFloat( skel->BasePoseMat.matrix[1][1] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][2] = LittleFloat( skel->BasePoseMat.matrix[1][2] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][3] = LittleFloat( skel->BasePoseMat.matrix[1][3] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][0] = LittleFloat( skel->BasePoseMat.matrix[2][0] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][1] = LittleFloat( skel->BasePoseMat.matrix[2][1] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][2] = LittleFloat( skel->BasePoseMat.matrix[2][2] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][3] = LittleFloat( skel->BasePoseMat.matrix[2][3] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][0] = LittleFloat( skel->BasePoseMatInv.matrix[0][0] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][1] = LittleFloat( skel->BasePoseMatInv.matrix[0][1] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][2] = LittleFloat( skel->BasePoseMatInv.matrix[0][2] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][3] = LittleFloat( skel->BasePoseMatInv.matrix[0][3] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][0] = LittleFloat( skel->BasePoseMatInv.matrix[1][0] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][1] = LittleFloat( skel->BasePoseMatInv.matrix[1][1] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][2] = LittleFloat( skel->BasePoseMatInv.matrix[1][2] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][3] = LittleFloat( skel->BasePoseMatInv.matrix[1][3] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][0] = LittleFloat( skel->BasePoseMatInv.matrix[2][0] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][1] = LittleFloat( skel->BasePoseMatInv.matrix[2][1] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][2] = LittleFloat( skel->BasePoseMatInv.matrix[2][2] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][3] = LittleFloat( skel->BasePoseMatInv.matrix[2][3] ) / gla->fScale;
		*/
		skel->BasePoseMat.matrix[0][0] = LittleFloat( skel->BasePoseMat.matrix[0][0] ) / gla->fScale;
		skel->BasePoseMat.matrix[0][1] = LittleFloat( skel->BasePoseMat.matrix[0][1] ) / gla->fScale;
		skel->BasePoseMat.matrix[0][2] = LittleFloat( skel->BasePoseMat.matrix[0][2] ) / gla->fScale;
		skel->BasePoseMat.matrix[0][3] = LittleFloat( skel->BasePoseMat.matrix[0][3] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][0] = LittleFloat( skel->BasePoseMat.matrix[1][0] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][1] = LittleFloat( skel->BasePoseMat.matrix[1][1] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][2] = LittleFloat( skel->BasePoseMat.matrix[1][2] ) / gla->fScale;
		skel->BasePoseMat.matrix[1][3] = LittleFloat( skel->BasePoseMat.matrix[1][3] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][0] = LittleFloat( skel->BasePoseMat.matrix[2][0] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][1] = LittleFloat( skel->BasePoseMat.matrix[2][1] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][2] = LittleFloat( skel->BasePoseMat.matrix[2][2] ) / gla->fScale;
		skel->BasePoseMat.matrix[2][3] = LittleFloat( skel->BasePoseMat.matrix[2][3] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][0] = LittleFloat( skel->BasePoseMatInv.matrix[0][0] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][1] = LittleFloat( skel->BasePoseMatInv.matrix[0][1] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][2] = LittleFloat( skel->BasePoseMatInv.matrix[0][2] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[0][3] = LittleFloat( skel->BasePoseMatInv.matrix[0][3] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][0] = LittleFloat( skel->BasePoseMatInv.matrix[1][0] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][1] = LittleFloat( skel->BasePoseMatInv.matrix[1][1] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][2] = LittleFloat( skel->BasePoseMatInv.matrix[1][2] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[1][3] = LittleFloat( skel->BasePoseMatInv.matrix[1][3] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][0] = LittleFloat( skel->BasePoseMatInv.matrix[2][0] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][1] = LittleFloat( skel->BasePoseMatInv.matrix[2][1] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][2] = LittleFloat( skel->BasePoseMatInv.matrix[2][2] ) / gla->fScale;
		skel->BasePoseMatInv.matrix[2][3] = LittleFloat( skel->BasePoseMatInv.matrix[2][3] ) / gla->fScale;
		/*
		skel->BasePoseMat.matrix[0][0] /= gla->fScale;
		skel->BasePoseMat.matrix[0][1] /= gla->fScale;
		skel->BasePoseMat.matrix[0][2] /= gla->fScale;
		skel->BasePoseMat.matrix[1][0] /= gla->fScale;
		skel->BasePoseMat.matrix[1][1] /= gla->fScale;
		skel->BasePoseMat.matrix[1][2] /= gla->fScale;
		skel->BasePoseMat.matrix[2][0] /= gla->fScale;
		skel->BasePoseMat.matrix[2][1] /= gla->fScale;
		skel->BasePoseMat.matrix[2][2] /= gla->fScale;
		skel->BasePoseMat.matrix[0][3] /= gla->fScale;
		skel->BasePoseMat.matrix[1][3] /= gla->fScale;
		skel->BasePoseMat.matrix[2][3] /= gla->fScale;
		*/
		if( do_zrot )
		{
			//	Matrix34VectorRotate( zrots, tempVert, outXyz );
			//	Matrix34VectorRotate( zrots, tempNormal, outNormal );
			//Matrix34Shuffle( skel->BasePoseMat.matrix );
			//Matrix34Multiply( zrots, skel->BasePoseMat.matrix, finRot );
			//Matrix34Copy( finRot, skel->BasePoseMat.matrix );
		}
		
		LL(skel->numChildren);
		for( j = 0; j < skel->numChildren; j++ )
		{
			LL(skel->children[j]);
		}
		skel = (glaSkel_t *)((byte *)&skel->children[skel->numChildren] );
	}
	
	numComps = gla->ofsEnd - gla->ofsCompBonePool;
	numComps /= 2;
	//ri.Printf( PRINT_DEVELOPER, "gla -> numComps %d\n", numComps );
	
	compBone = (glaCompBone_t *)((byte *)gla + gla->ofsCompBonePool);
	for( i = 0; i < numComps; i++ )
	{
		compBone->a[i] = LittleShort( compBone->a[i] );
		//ri.Printf( PRINT_DEVELOPER, "comparr %d 0x%08x\n", i, compBone->a[i] );
	}
	//ri.Printf (PRINT_WARNING,"Read GLA Just Dandy!\n" );
	return qtrue;
}

/*==============================================================
  R_AddMyGhoulSurfaces
    This is where the system tosses MOD_GLM, surfaces are added here
==============================================================*/
void R_AddMyGhoulSurfaces( trRefEntity_t *ent ) {
	//model_t *animModel;
	//glaHeader_t *gla;
	glmHeader_t		*header;
//	model_t *model_anims;
	glmHeader_t *anims = 0;
	//glmSurface_t *surf;
	glmSurface_t *surface;
	glmSurfHierarchy_t *surfh;
	//mg_animstate_t *as = NULL;
	int animated = 0;
	glmLOD_t		*lod;
	glmLODSurfOffset_t	*lod_surf_ofs;
	shader_t		*shader;
	int				i;
	//int				cull;
	//int				lod;
	int				fogNum = 0;
	qboolean		personalModel;
	int newQuatDeal = 0;

	//mygBQArr_t *oldframe;
	//mygBQArr_t *newframe;
	//mygBQArr_t *resframe;
	//mygBoneArr_t *pose;
	//vec3_t	trans;
	//int bonewatch = 1;

	// don't add third_person objects if not in a portal
	personalModel = (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal;

	if( personalModel )
	{
		//ent->needZFail = qtrue;
	}

	header = tr.currentModel->modelData;
	if( header->animIndex != 0 )
	{
#if 0
		model_anims = R_GetModelByHandle( header->animIndex );

		if( model_anims->type == MOD_GLA )
		{
			anims = model_anims->gla;
			as = RE_AS_Fetch( ent->e.frame );
		}

		if( as->index == 0 )
		{
			anims = 0;
			animated = 0;
		}
		else
		{
			if( as->index != 0 )
			{
				animated = 1;
			}
		}
#endif
	}
	else
	{
		animated = 0;
	}
	
//	printf( "(GLM) myghoul model is " );
//	if(personalModel) printf( "personal model, " );
//	if(animated) printf( "animated." );

#if 0
	if( animated )
	{
		AnimStateAnimsPop( as, anims );
		MG_InheritQuats( anims, (mgQuat_t*)as->resframe->quat, 0 );
		if( !newQuatDeal ) MG_MatsPopulate( anims, (mgQuat_t*)as->resframe->quat, as->pose );
	}
#endif

	// compute LOD
	//lod = R_ComputeLOD( ent );
	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum.
	/*cull = R_CullModel ( header, ent );
	if ( cull == CULL_OUT ) {
		return;
	}*/
	// set up lighting now that we know we aren't culled
	if ( !personalModel || r_shadows->integer > 1 ) {
		R_SetupEntityLighting( &tr.refdef, ent );
	}

	// see if we are in a fog volume
	//fogNum = R_ComputeFogNum( header, ent );

	//ri.Printf( PRINT_DEVELOPER, "(I) Adding this MG Model\n" );
	lod = (glmLOD_t *)( (byte *)header + header->ofsLODs );
	lod_surf_ofs = ( glmLODSurfOffset_t *) ((byte *)lod + 4 );
	surfh = (glmSurfHierarchy_t *) ( (byte *)header + header->ofsSurfHierarchy );
	surface = (glmSurface_t *)( (byte *)lod_surf_ofs + lod_surf_ofs->offsets[0] );
	for ( i = 0 ; i < header->numSurfaces ; i++ ) {
		if( surfh->name[0] != '*' && strcmp( "stupidtriangle_off", surfh->name ) )
		{
			if ( ent->e.customShader ) {
				shader = R_GetShaderByHandle( ent->e.customShader );
			} else if ( ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins ) {
				skin_t *skin;
				int		j;

				skin = R_GetSkinByHandle( ent->e.customSkin );
				// match the surface name to something in the skin file
				shader = tr.defaultShader;
				for ( j = 0 ; j < skin->numSurfaces ; j++ ) {
					// the names have both been lowercased
					if ( !strcmp( skin->surfaces[j]->name, surfh->name ) ) {
						shader = skin->surfaces[j]->shader;
						break;
					}
				}
				if (shader == tr.defaultShader) {
					//ri.Printf( PRINT_DEVELOPER, "WARNING: no shader for surface %s in skin %s\n", surfh->name, skin->name);
				}
				else if (shader->defaultShader) {
					//ri.Printf( PRINT_DEVELOPER, "WARNING: shader %s in skin %s not found\n", shader->name, skin->name);
				}
			} else {
				shader = R_GetShaderByHandle( surfh->shaderIndex );
			}
			// we will add shadows even if the main object isn't visible in the view

			// stencil shadows can't do personal models unless I polyhedron clip
			if ( !personalModel ) {
				R_AddDrawSurf( (void *)surface, shader, 0, qfalse );
			}

			if ( !personalModel
				&& r_shadows->integer == 2
				&& fogNum == 0
				&& !(ent->e.renderfx & ( RF_NOSHADOW | RF_DEPTHHACK ) )
				&& shader->sort == SS_OPAQUE ) {
				R_AddDrawSurf( (void *)surface, tr.shadowShader, 0, qfalse );
			}

			if ( r_shadows->integer == 4
				&& fogNum == 0
				&& !(ent->e.renderfx & ( RF_NOSHADOW | RF_DEPTHHACK ) )
				&& shader->sort == SS_OPAQUE ) {
				R_AddDrawSurf( (void *)surface, tr.shadowShader, 0, qfalse );
			}

			// projection shadows work fine with personal models
			if ( r_shadows->integer == 3
				&& fogNum == 0
				&& (ent->e.renderfx & RF_SHADOW_PLANE )
				&& shader->sort == SS_OPAQUE ) {
				R_AddDrawSurf( (void *)surface, tr.projectionShadowShader, 0, qfalse );
			}
		}

		surface = (glmSurface_t *)( (byte *)surface + surface->ofsEnd );
		surfh = (glmSurfHierarchy_t *) ( (byte *)&surfh->childIndexes[surfh->numChildren] );
	}
}

/*==============================================================
  RB_SurfaceMyGhoul
    Called when a MyGhoul surf is asked to render.
==============================================================*/
void RB_SurfaceMyGhoul( surfaceType_t *surface )
{
	glmSurface_t	*surf = (glmSurface_t *)surface;
//	int				i, j, k;
	int				j, k;
	int				*triangles;
	int				indexes;
	int				baseIndex, baseVertex;
	int				numVerts;
	glmVertex_t		*v;
	glmVertexTexCoord_t *t;
	glmHeader_t		*header;
	model_t			*model_anims = NULL;
//	mdxaHeader_t	*anims;
	vec3_t	tempVert, tempNormal;
	int				animated = 0;
//	int				numWeights;
//	int				boneIndices[4];
//	float			boneWeights[4];
//	float			totalWeight = 0.0;
	//mgMatrix34_t		final;
	//mg_animstate_t	*as = NULL;
	//mgMatrix34_t	*pose = NULL;
	float	*outXyz, *outNormal;
//	vec3_t newXyz, newNormal;
	
	float zrots[3][4];
	int do_zrot = 1;
	
	zrots[0][0] = 0.0;
	zrots[0][1] = -1.0;
	zrots[0][2] = 0.0;
	zrots[0][3] = 0.0;
	zrots[1][0] = 1.0;
	zrots[1][1] = 0.0;
	zrots[1][2] = 0.0;
	zrots[1][3] = 0.0;
	zrots[2][0] = 0.0;
	zrots[2][1] = 0.0;
	zrots[2][2] = 1.0;
	zrots[2][3] = 0.0;
	
	header = (glmHeader_t *)((byte *)surf + surf->ofsHeader);
	RB_CHECKOVERFLOW( surf->numVerts, surf->numTriangles * 3 );
	
	triangles = (int *) ((byte *)surf + surf->ofsTriangles);
	indexes = surf->numTriangles * 3;
	baseIndex = tess.numIndexes;
	baseVertex = tess.numVertexes;
	
	outXyz = tess.xyz[baseVertex];
	outNormal = tess.normal[baseVertex];
	
	for (j = 0 ; j < indexes ; j++)
	{
		tess.indexes[baseIndex + j] = baseVertex + triangles[j];
	}
	tess.numIndexes += indexes;
#if 0
	if( header->animIndex != 0 )
	{
		model_anims = R_GetModelByHandle( header->animIndex );
		if( model_anims->type == MOD_GLA )
		{
			anims = model_anims->gla;
			as = RE_AS_Fetch( backEnd.currentEntity->e.frame );
		}

		if( as->index == 0 )
		{
			anims = 0;
			animated = 0;
		}
		else
		{
			animated = 1;
			pose = as->pose;
		}
	}
	else
#endif
	{
		animated = 0;
	}
	
	//animated = 0;
	
	numVerts = surf->numVerts;
	v = (glmVertex_t *)((byte *)surf + surf->ofsVerts);
	triangles = (int *)((byte *)surf + surf->ofsBoneReferences );
	
	for( k=0; k<numVerts; k++, outXyz+=4, outNormal+=4 )
	{
		tempVert[0] = v->vertCoords[0];
		tempVert[1] = v->vertCoords[1];
		tempVert[2] = v->vertCoords[2];
		tempNormal[0] = v->normal[0];
		tempNormal[1] = v->normal[1];
		tempNormal[2] = v->normal[2];

#if 0
		if( animated )
		{
			numWeights = GLM_GetVertNumWeights( v );
			final.matrix[0][0] = final.matrix[0][1] = final.matrix[0][2] = final.matrix[0][3] = final.matrix[1][0] = final.matrix[1][1] = final.matrix[1][2] = final.matrix[1][3] = final.matrix[2][0] = final.matrix[2][1] = final.matrix[2][2] = final.matrix[2][3] = 0.0;
			totalWeight = 0.0;
			for( i = 0; i < numWeights; i++ )
			{
				boneIndices[i] = triangles[( v->uiNmWeightsAndBoneIndexes >> ( 5 * i ) ) & ( ( 1 << 5 ) - 1 )];
				if( i == numWeights-1 )
				{
					boneWeights[i] = 1.0f-totalWeight;
				}
				else
				{
					boneWeights[i] = GLM_GetVertBoneWeight( v, i, v->BoneWeightings[i], v->uiNmWeightsAndBoneIndexes, totalWeight );
				}
				totalWeight += boneWeights[i];
				Matrix34AddScale( final.matrix, pose[boneIndices[i]].matrix, boneWeights[i] );
			}

			Matrix34VectorRotateAdd( final.matrix, tempVert, newXyz );
			Matrix34VectorRotate( final.matrix, tempNormal, newNormal );

			outXyz[0] = newXyz[0];
			outXyz[1] = newXyz[1];
			outXyz[2] = newXyz[2];

			outNormal[0] = newNormal[0];
			outNormal[1] = newNormal[1];
			outNormal[2] = newNormal[2];
		}
		else
#endif
		{
			if( do_zrot )
			{
				Matrix34VectorRotate( zrots, tempVert, outXyz );
				Matrix34VectorRotate( zrots, tempNormal, outNormal );
			}
			else
			{
				outXyz[0] = tempVert[0];
				outXyz[1] = tempVert[1];
				outXyz[2] = tempVert[2];

				outNormal[0] = tempNormal[0];
				outNormal[1] = tempNormal[1];
				outNormal[2] = tempNormal[2];
			}
		}
		v = (glmVertex_t *) ( (byte *)v + 32 );
	}
	
	t = (glmVertexTexCoord_t *)((byte *)surf + surf->ofsVerts + (numVerts*32));
	for ( j = 0; j < numVerts; j++ )
	{
		tess.texCoords[baseVertex + j][0][0] = t[j].texCoords[0];
		tess.texCoords[baseVertex + j][0][1] = t[j].texCoords[1];
		// FIXME: fill in lightmapST for completeness?
	}
	tess.numVertexes += numVerts;
}


