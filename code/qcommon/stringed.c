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

/* TODO: This only loads on firsttime Com_Init, so a filesystem restart or cvar change wont cause a reload*/

// Need larger buffer for credits and a few others
static	char	se_token[BIG_INFO_STRING];
static	char	se_parsename[MAX_TOKEN_CHARS];
static	int		se_lines;

char *SE_ParseExt( char **data_p, qboolean allowLineBreaks );

void SE_BeginParseSession( const char *name )
{
	se_lines = 0;
	Com_sprintf(se_parsename, sizeof(se_parsename), "%s", name);
}

int SE_GetCurrentParseLine( void )
{
	return se_lines;
}

char *SE_Parse( char **data_p )
{
	return SE_ParseExt( data_p, qtrue );
}

void SE_ParseError( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("ERROR: %s, line %d: %s\n", se_parsename, se_lines, string);
}

void SE_ParseWarning( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("WARNING: %s, line %d: %s\n", se_parsename, se_lines, string);
}

static char *SE_SkipWhitespace( char *data, qboolean *hasNewLines ) {
	int c;

	while( (c = *data) <= ' ') {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' ) {
			se_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

char *SE_ParseExt( char **data_p, qboolean allowLineBreaks )
{
	char *in, *out;

	se_token[0] = 0;

	if( !*data_p ) return se_token;

	in = *data_p;
	out = se_token;

	if( *in && *in <= '/' ) // skip lot of conditions if *in is regular char
	{
		// ignore while whitespace or newline
		while( *in && *in <= ' ' ) {
			if( *in++ == '\n') {
				se_lines++;
				if( !allowLineBreaks ) {
					*data_p = in;
					return se_token;
				}
			}
		}

		// skip comments
		while( *in == '/' ) {
			in++;
			if( *in == '/' ) {
				in++;
				while( *in && *in != '\n' ) in++;  // ignore until newline
				if( *in ) in++;
			}
			else if( *in == '*' ) {
				in++;
				while( *in && ( *in != '*' || in[1] != '/' ) ) in++;  // ignore until comment close
				if( *in ) in += 2;
			}
			else {
				*out++ = '/';
				break;
			}
			while( *in && *in <= ' ' ) {
				if( *in++ == '\n') {
					se_lines++;
					if( !allowLineBreaks ) {
						*data_p = in;
						return se_token;
					}
				}
			}
		}

		// handle quoted strings
		if( *in == '"' ) {
			in++;
			while( *in && *in != '"' ) {
				if( (out-se_token) >= BIG_INFO_STRING-2 ) {
					SE_ParseWarning( "Token exceeded %d chars, truncated.", BIG_INFO_STRING-2 );
					break;
				}
				*out++ = *in++;
			}
			if( *in ) in++;
			*out = '\0';
			*data_p = in;
			return se_token;
		}
	}

	// parse a regular word
	while( *in > ' ' ) {
		if( (out-se_token) >= BIG_INFO_STRING-1 ) {
			SE_ParseWarning( "Token exceeded %d chars, truncated.", BIG_INFO_STRING-2 );
			break;
		}
		*out++ = *in++;
	}
	*out = '\0';
	*data_p = ( *in ? in : NULL );	// next text point or NULL if end of text reached
	return se_token;
}

void SE_SkipRestOfLine ( char **data ) {
	char	*p;
	int		c;

	p = *data;
	while ( (c = *p++) != 0 ) {
		if ( c == '\n' ) {
			se_lines++;
			break;
		}
	}

	*data = p;
}

static	cvar_t		*se_debug;
static	cvar_t		*se_language;

typedef unsigned int uInt;

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

/* Ensures that newlines "\n" are converted to real '\n' (from g_spawn.c) */
char *SE_NewString( const char *string ) {
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = (char *) SE_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	
	return newb;
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

	SE_BeginParseSession( language );

	do {
		token = SE_Parse( &text_p );
		if ( !strcmp( "ENDMARKER", token ) ) {
			break;
		}
		if ( !strcmp( "VERSION", token ) ) {
			//token = COM_Parse( &text_p );
			SE_SkipRestOfLine( &text_p );
			continue;
		}
		if ( !strcmp( "CONFIG", token ) ) {
			//token = COM_Parse( &text_p );
			SE_SkipRestOfLine( &text_p );
			continue;
		}
		if ( !strcmp( "FILENOTES", token ) ) {
			//token = COM_Parse( &text_p );
			SE_SkipRestOfLine( &text_p );
			continue;
		}
		if ( !strcmp( "REFERENCE", token ) ) {
			token = SE_ParseExt( &text_p, qfalse );
			Q_strncpyz( reference, va("%s_%s", title, token), MAX_QPATH );
			token = SE_Parse( &text_p );
			if ( !strcmp( "NOTES", token ) ) {
				SE_SkipRestOfLine( &text_p );
				token = SE_Parse( &text_p ); // look for LANG_ENGLISH
				if ( !strcmp( "LANG_ENGLISH", token ) ) {
					token = SE_ParseExt( &text_p, qfalse );
					Q_strncpyz( translated, token, MAX_TRANS_STRING );
				}
			}
			else if ( !strcmp( "LANG_ENGLISH", token ) ) {
				token = SE_ParseExt( &text_p, qfalse );
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
			str->translated = SE_NewString( translated );

			strings[numStrings] = *str;

			numStrings++;
			continue;
		}
	} while ( token );

	SE_BeginParseSession( "" );

	FS_FreeFile( langfile.v );
}

// Compare is expected in FILENAME_REFERENCE format
// above stores FILENAME_(each reference) into each individual .reference
int SE_GetStringBuffer( const char *compare, char *buffer, int bufferSize ) {
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

char *SE_GetString( const char *compare ) {
	static char text[MAX_STRING_CHARS] = { 0 };

	stringEd_t *str;

	str = FindString( compare );

	if( str && str->translated && *str->translated ) {
		Q_strncpyz( text, str->translated, sizeof(text) );
		return text;
	}
	else
		return "";
}

void SE_Shutdown( void ) {
}

void SE_Init( void ) {
	char    **fileList;
	int numFiles, i;
	char	title[MAX_QPATH];

	se_debug = Cvar_Get( "se_debug", "0", CVAR_TEMP );
	se_language = Cvar_Get( "se_language", "english", CVAR_ARCHIVE|CVAR_NORESTART );

	fileList = FS_ListFiles( "strings/english", ".str", &numFiles );

	if( numFiles > 128 )
		numFiles = 128;

	for ( i = 0; i < numFiles; i++ ) {
		COM_StripExtension( fileList[i], title, MAX_QPATH );
		Q_strupr( title );
		SE_Load( title, va( "strings/english/%s", fileList[i] ) );
	}
}
