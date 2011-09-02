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
// stringed.c -- functions used to parse the string ed stuffz

#include "q_shared.h"
#include "qcommon.h"

/* This needs some rewriting with hashed methods so lookups dont take days */
/* Was a temp hack to load the files at all */

static	cvar_t		*se_debug;
static	cvar_t		*se_language;

typedef struct langfile_s {
	char version[MAX_STRING_TOKENS];
	char config[MAX_STRING_TOKENS];
	char filenotes[MAX_STRING_TOKENS];

	struct {
		char reference[MAX_STRING_TOKENS];
		char notes[MAX_STRING_TOKENS];
		char full[MAX_STRING_TOKENS];
	} translations[1024];
} langfile_t;

static langfile_t english[128];

void SE_Load( const char *title, int index, const char *language ) {
	union {
		char	*c;
		void	*v;
	} langfile;
	char *text_p, *token;
	int count = 0;

	FS_ReadFile( language, &langfile.v );
	if ( !langfile.c ) {
		Com_Printf( S_COLOR_YELLOW "SE_Load(): Couldn't load \"%s\"!\n", language );
		return;
	}

	text_p = langfile.c;

	do {
		token = COM_Parse( &text_p );
		if ( !strcmp( "ENDMARKER", token ) ) {
			break;
		}
		if ( !strcmp( "VERSION", token ) ) {
			token = COM_Parse( &text_p );
			Q_strncpyz( english[index].version, token, MAX_STRING_TOKENS );
			continue;
		}
		if ( !strcmp( "CONFIG", token ) ) {
			token = COM_Parse( &text_p );
			Q_strncpyz( english[index].config, token, MAX_STRING_TOKENS );
			continue;
		}
		if ( !strcmp( "FILENOTES", token ) ) {
			token = COM_Parse( &text_p );
			Q_strncpyz( english[index].filenotes, token, MAX_STRING_TOKENS );
			continue;
		}
		if ( !strcmp( "REFERENCE", token ) ) {
			token = COM_Parse( &text_p );
			Q_strncpyz( english[index].translations[count].reference, va("%s_%s", title, token), MAX_STRING_TOKENS );
			token = COM_Parse( &text_p );
			if ( !strcmp( "NOTES", token ) ) {
				token = COM_Parse( &text_p );
				Q_strncpyz( english[index].translations[count].notes, token, MAX_STRING_TOKENS );
				token = COM_Parse( &text_p );
				if ( !strcmp( "LANG_ENGLISH", token ) ) {
					token = COM_Parse( &text_p );
					Q_strncpyz( english[index].translations[count].full, token, MAX_STRING_TOKENS );
				}
			}
			else if ( !strcmp( "LANG_ENGLISH", token ) ) {
				token = COM_Parse( &text_p );
				Q_strncpyz( english[index].translations[count].full, token, MAX_STRING_TOKENS );
			}
			count++;

			continue;
		}
	} while ( token );

	FS_FreeFile( langfile.v );
}

// Compare is expected in FILENAME_REFERENCE format
// above stores FILENAME_(each reference) into each individual .reference
void SE_GetString( const char *compare, char *buffer, int bufferSize ) {
	int i, j;

	for( i = 0; i < 128; i++ ) {
		for( j = 0; j < 1024; j++ ) {
			if( !strcmp( compare, english[i].translations[j].reference ) ) {
				Q_strncpyz( buffer, english[i].translations[j].full, bufferSize );
				return;
			}
		}
	}
}

void SE_Init( void ) {
	char    **fileList;
	int numFiles, i;
	char	title[MAX_QPATH];

	se_debug = Cvar_Get( "se_debug", "0", CVAR_TEMP );
	se_language = Cvar_Get( "se_language", "english", CVAR_ARCHIVE|CVAR_LATCH );
	// Can only be changed on launch, or a filesystem restart (game change)

	fileList = FS_ListFiles( "strings/english", ".str", &numFiles );

	if( numFiles > 128 )
		numFiles = 128;

	for ( i = 0; i < numFiles; i++ ) {
		COM_StripExtension( fileList[i], title, MAX_QPATH );
		Q_strupr( title );
		SE_Load( title, i, va( "strings/english/%s", fileList[i] ) );
	}
}

