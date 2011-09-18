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
#include "mdx_format.h"

#define LL( x ) x = LittleLong( x )

static qboolean MDX_CheckRange( mdxmHeader_t *header, int offset,
			    int count, int size ) {
	// return true if the range specified by offset, count and size
	// doesn't fit into the file
	return ( count <= 0 ||
		 offset < 0 ||
			  offset > header->ofsEnd ||
		 offset + count * size < 0 ||
					 offset + count * size > header->ofsEnd );
}

qboolean R_LoadMDXM( model_t *mod, void *buffer, int filesize, const char *mod_name ) {
	mdxmHeader_t *header;
	int size;
	int version;

	if( filesize < sizeof( mdxmHeader_t ) ) {
		return qfalse;
	}

	header = (mdxmHeader_t *)buffer;

	version = LittleLong( header->version );

	if( version != MDXM_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDXM: %s has wrong version (%i should be %i)\n", mod_name, version, MDXM_VERSION );
		return qfalse;
	}

	size = LittleLong( header->ofsEnd );

	if( size > filesize ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDXM: Header of %s is broken. Wrong filesize declared!\n", mod_name );
		return qfalse;
	}

	LL( header->ident );
	LL( header->animIndex );
	LL( header->numBones );
	LL( header->numLODs );
	LL( header->ofsLODs );
	LL( header->numSurfaces );
	LL( header->ofsSurfHierarchy );

	if ( MDX_CheckRange( header, header->ofsSurfHierarchy, header->numSurfaces, sizeof( mdxmSurfHierarchy_t ) ) ) {
		return qfalse;
	}

	if ( MDX_CheckRange( header, header->ofsLODs, header->numLODs, sizeof( mdxmLOD_t ) ) ) {
		return qfalse;
	}

	mod->type = MOD_MDX;

	return qtrue;
}

#if 0
qboolean R_LoadMDXA( model_t *mod, void *buffer, int filesize, const char *mod_name ) {
	mdxaHeader_t *header;
	mdxmLOD_t *lod;
	mdxmSurfHierarchy_t *surf_h;
	int size;
	int version;
	int i;

	if( filesize < sizeof( mdxmHeader_t ) ) {
		return qfalse;
	}

	header = (mdxaHeader_t *)buffer;

	version = LittleLong( header->version );

	if( version != MDXA_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDXA: %s has wrong version (%i should be %i)\n", mod_name, version, MDXA_VERSION );
		return qfalse;
	}

	size = LittleLong( header->ofsEnd );

	if( size > filesize ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDXA: Header of %s is broken. Wrong filesize declared!\n", mod_name );
		return qfalse;
	}

	return qtrue;
}
#endif
