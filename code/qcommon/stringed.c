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

/* TODO: This only loads on firsttime Com_Init, so a filesystem restart wont fix up files*/
/* Still seems to be having some issues with the hash-table because some of the values appear to be junk and cause a sig11 when trying to loop through hash list in FindString from SP_GetString */

static	cvar_t		*se_debug;
static	cvar_t		*se_language;

typedef unsigned int uInt;

// 2048000 // 2mb

#define MAX_POOL_SIZE	512000

static char		se_pool[MAX_POOL_SIZE];
static int		se_poolSize = 0;
static int		se_poolTail = MAX_POOL_SIZE;

void *SE_Alloc ( int size )
{
	se_poolSize = ((se_poolSize + 0x00000003) & 0xfffffffc);

	if (se_poolSize + size > se_poolTail)
	{
		Com_Error( ERR_FATAL, "SE_Alloc: buffer exceeded tail (%d > %d)", se_poolSize + size, se_poolTail);
		return 0;
	}

	se_poolSize += size;

	return &se_pool[se_poolSize-size];
}

char *SE_StringAlloc ( const char *source )
{
	char *dest;

	dest = SE_Alloc ( strlen ( source ) + 1 );
	strcpy ( dest, source );
	return dest;
}

qboolean SE_OutOfMemory ( void )
{
	return se_poolSize >= MAX_POOL_SIZE;
}

/*
================
FNV32
================
*/
uInt FNV32( const char *value, qboolean caseSensitive ) {
	uInt hval = 0x811c9dc5;
	const byte *s = (const byte *)(value);
	if ( caseSensitive ) {
		while ( *s != '\0' ) {
			hval ^= (uInt)(*s++);
			hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
		}
	} else {
		while ( *s != '\0' ) {
			hval ^= (uInt)(tolower(*s++));
			hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
		}
	}
	return hval;
}

#define MAX_TRANS_STRINGS	4096
//#define FILE_HASH_SIZE		1024
#define MAX_TRANS_STRING	4096

typedef struct stringEd_s {
	char *reference;
	char *translated;

	uInt refhash;
} stringEd_t;

stringEd_t strings[MAX_TRANS_STRINGS];
int numStrings;

static stringEd_t* FindString( const char *reference ) {
	stringEd_t *str, *max = &strings[numStrings];
	uInt hash = FNV32( reference, qfalse );

	if( reference == NULL || reference[0] == 0 ) {
		return NULL;
	}

	str = strings;

	for( ; str < max; str++ ) {
		if( !str->reference )
			continue;

		if( str->refhash == hash && !Q_stricmp( str->reference, reference ) ) {
			return str;
		}
	}

	return NULL;
}

void SE_Load( const char *title, const char *language ) {
	union {
		char	*c;
		void	*v;
	} langfile;
	char *text_p, *token;
	stringEd_t *str;
	char reference[MAX_QPATH];
	char translated[MAX_TRANS_STRING];

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
			continue;
		}
		if ( !strcmp( "CONFIG", token ) ) {
			token = COM_Parse( &text_p );
			continue;
		}
		if ( !strcmp( "FILENOTES", token ) ) {
			token = COM_Parse( &text_p );
			continue;
		}
		if ( !strcmp( "REFERENCE", token ) ) {
			token = COM_Parse( &text_p );
			Q_strncpyz( reference, va("%s_%s", title, token), MAX_QPATH );
			token = COM_Parse( &text_p );
			if ( !strcmp( "NOTES", token ) ) {
				token = COM_Parse( &text_p ); // skip over notes
				token = COM_Parse( &text_p ); // look for LANG_ENGLISH
				if ( !strcmp( "LANG_ENGLISH", token ) ) {
					token = COM_Parse( &text_p );
					Q_strncpyz( translated, token, MAX_TRANS_STRING );
				}
			}
			else if ( !strcmp( "LANG_ENGLISH", token ) ) {
				token = COM_Parse( &text_p );
				Q_strncpyz( translated, token, MAX_TRANS_STRING );
			}

			if( numStrings == MAX_TRANS_STRINGS ) {
				if(!com_errorEntered)
					Com_Error(ERR_FATAL, "MAX_TRANS_STRINGS hit");

				FS_FreeFile( langfile.v );
				return;
			}

			str = SE_Alloc( sizeof ( stringEd_t ) );

			str->reference = SE_StringAlloc( reference );
			str->refhash = FNV32( reference, qfalse );
			str->translated = SE_StringAlloc( translated );

			strings[numStrings] = *str;

			numStrings++;
			continue;
		}
	} while ( token );

	FS_FreeFile( langfile.v );
}

// Compare is expected in FILENAME_REFERENCE format
// above stores FILENAME_(each reference) into each individual .reference
int SE_GetString( const char *compare, char *buffer, int bufferSize ) {
	stringEd_t *str;

	if( !buffer )
		return 0;

	str = FindString( compare );

	if( str && str->translated && *str->translated ) {
		Q_strncpyz( buffer, str->translated, bufferSize );
		return 1;
	}
	else
		*buffer = '\0';

	return 0;
}

void SE_Shutdown( void ) {
}

void SE_Init( void ) {
	char    **fileList;
	int numFiles, i;
	char	title[MAX_QPATH];

	se_debug = Cvar_Get( "se_debug", "0", CVAR_TEMP );
	se_language = Cvar_Get( "se_language", "english", CVAR_ARCHIVE|CVAR_LATCH );

	fileList = FS_ListFiles( "strings/english", ".str", &numFiles );

	if( numFiles > 128 )
		numFiles = 128;

	for ( i = 0; i < numFiles; i++ ) {
		COM_StripExtension( fileList[i], title, MAX_QPATH );
		Q_strupr( title );
		SE_Load( title, va( "strings/english/%s", fileList[i] ) );
	}
}
