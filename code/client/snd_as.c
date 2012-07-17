/*
===========================================================================
Copyright (C) 2011 Jeremy 'Ensiform' Davis
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

#include "client.h"
#include "snd_local.h"

/* COM_Parse stuff but follows the sound/sound.txt formatting guidelines */
static	char	as_token[MAX_TOKEN_CHARS];
static	char	as_parsename[MAX_TOKEN_CHARS];
static	int		as_lines;

char *AS_ParseExt( char **data_p, qboolean allowLineBreaks );

void AS_BeginParseSession( const char *name )
{
	as_lines = 0;
	Com_sprintf(as_parsename, sizeof(as_parsename), "%s", name);
}

int AS_GetCurrentParseLine( void )
{
	return as_lines;
}

char *AS_Parse( char **data_p )
{
	return AS_ParseExt( data_p, qtrue );
}

void AS_ParseError( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("ERROR: %s, line %d: %s\n", as_parsename, as_lines, string);
}

void AS_ParseWarning( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("WARNING: %s, line %d: %s\n", as_parsename, as_lines, string);
}

/*
===============
AS_ParseString
===============
*/
qboolean AS_ParseString( char **data, const char **s ) 
{
//	*s = COM_ParseExt( data, qtrue );
	*s = AS_ParseExt( data, qfalse );
	if ( s[0] == 0 ) 
	{
		Com_Printf("unexpected EOF\n");
		return qtrue;
	}
	return qfalse;
}

/*
===============
AS_ParseInt
===============
*/
qboolean AS_ParseInt( char **data, int *i ) 
{
	const char	*token;

	token = AS_ParseExt( data, qfalse );
	if ( token[0] == 0 ) 
	{
		Com_Printf( "unexpected EOF\n" );
		return qtrue;
	}

	*i = atoi( token );
	return qfalse;
}

/*
===============
AS_ParseFloat
===============
*/
qboolean AS_ParseFloat( char **data, float *f ) 
{
	const char	*token;

	token = AS_ParseExt( data, qfalse );
	if ( token[0] == 0 ) 
	{
		Com_Printf( "unexpected EOF\n" );
		return qtrue;
	}

	*f = atof( token );
	return qfalse;
}

/*
===============
AS_ParseVec4
===============
*/
qboolean AS_ParseVec4( char **buffer, vec4_t *c) 
{
	int i;
	float f;

	for (i = 0; i < 4; i++) 
	{
		if (AS_ParseFloat(buffer, &f)) 
		{
			return qtrue;
		}
		(*c)[i] = f;
	}
	return qfalse;
}

/*
==============
AS_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
static char *AS_SkipWhitespace( char *data, qboolean *hasNewLines ) {
	int c;

	while( (c = *data) <= ' ') {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' ) {
			as_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

int AS_Compress( char *data_p ) {
	char *in, *out;
	int c;
	qboolean newline = qfalse, whitespace = qfalse;

	in = out = data_p;
	if (in) {
		while ((c = *in) != 0) {
			// skip semicolon comments
			if ( c == ';' ) {
				while (*in && *in != '\n') {
					in++;
				}
			// record when we hit a newline
			} else if ( c == '\n' || c == '\r' ) {
				newline = qtrue;
				in++;
				// record when we hit whitespace
			} else if ( c == ' ' || c == '\t') {
				whitespace = qtrue;
				in++;
				// an actual token
			} else {
				// if we have a pending newline, emit it (and it counts as whitespace)
				if (newline) {
					*out++ = '\n';
					newline = qfalse;
					whitespace = qfalse;
				} if (whitespace) {
					*out++ = ' ';
					whitespace = qfalse;
				}

				// copy quoted strings unmolested
				if (c == '"') {
					*out++ = c;
					in++;
					while (1) {
						c = *in;
						if (c && c != '"') {
							*out++ = c;
							in++;
						} else {
							break;
						}
					}
					if (c == '"') {
						*out++ = c;
						in++;
					}
				} else {
					*out = c;
					out++;
					in++;
				}
			}
		}

		*out = 0;
	}
	return out - data_p;
}

char *AS_ParseExt( char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	char *data;

	data = *data_p;
	len = 0;
	as_token[0] = 0;

	// make sure incoming data is valid
	if ( !data )
	{
		*data_p = NULL;
		return as_token;
	}

	while ( 1 )
	{
		// skip whitespace
		data = AS_SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return as_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return as_token;
		}

		c = *data;

		// skip semicolon comments
		if ( c == ';' )
		{
			data++;
			while (*data && *data != '\n') {
				data++;
			}
		}
		else
		{
			break;
		}
	}

	// handle quoted strings
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				as_token[len] = 0;
				*data_p = ( char * ) data;
				return as_token;
			}
			if (len < MAX_TOKEN_CHARS - 1)
			{
				as_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS - 1)
		{
			as_token[len] = c;
			len++;
		}
		data++;
		c = *data;
		if ( c == '\n' )
			as_lines++;
	} while (c>32);

	as_token[len] = 0;

	*data_p = ( char * ) data;
	return as_token;
}

/*
==================
COM_MatchToken
==================
*/
void AS_MatchToken( char **buf_p, char *match ) {
	char	*token;

	token = AS_Parse( buf_p );
	if ( strcmp( token, match ) ) {
		Com_Error( ERR_DROP, "MatchToken: %s != %s", token, match );
	}
}


/*
=================
SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
void AS_SkipBracedSection (char **program) {
	char			*token;
	int				depth;

	depth = 0;
	do {
		token = AS_ParseExt( program, qtrue );
		if( token[1] == 0 ) {
			if( token[0] == '{' ) {
				depth++;
			}
			else if( token[0] == '}' ) {
				depth--;
			}
		}
	} while( depth && *program );
}

/*
=================
SkipRestOfLine
=================
*/
void AS_SkipRestOfLine ( char **data ) {
	char	*p;
	int		c;

	p = *data;
	while ( (c = *p++) != 0 ) {
		if ( c == '\n' ) {
			as_lines++;
			break;
		}
	}

	*data = p;
}

void AS_UpdateSet( const char *name, vec3_t origin ) {
}

#define SOUNDSET_FILE "sound/sound.txt"

void AS_ParseSets( void ) {
	union {
		char	*c;
		void	*v;
	} as_file;
	char *text_p, *token;

	FS_ReadFile( SOUNDSET_FILE, &as_file.v );

	if ( !as_file.c ) {
		Com_Printf( S_COLOR_RED "ERROR: Couldn't load ambient sound sets from \"%s\"\n", SOUNDSET_FILE );
		return;
	}

	text_p = as_file.c;

	do {
		token = AS_Parse( &text_p );
		if ( !token || !token[0] ) {
			break;
		}
		if ( !strcmp( "type", token ) ) {
			token = AS_Parse( &text_p );
			if( Q_stricmp( token, "ambientset" ) ) {
				Com_Printf( S_COLOR_RED "AS_ParseHeader: Set type \"%s\" is not a valid set type!\n", token );
				FS_FreeFile( as_file.v );
				return;
			}
			continue;
		}
		if ( !strcmp( "amsdir", token ) ) {
			//token = AS_Parse( &text_p );
			AS_SkipRestOfLine( &text_p );
			continue;
		}
		if ( !strcmp( "outdir", token ) ) {
			//token = AS_Parse( &text_p );
			AS_SkipRestOfLine( &text_p );
			continue;
		}
		if ( !strcmp( "basedir", token ) ) {
			//token = AS_Parse( &text_p );
			AS_SkipRestOfLine( &text_p );
			continue;
		}
		//generalSet localSet bmodelSet
	} while ( token );

	FS_FreeFile( as_file.v );
}

void AS_PrecacheEntry( const char *name ) {
	if( !Q_stricmp(name, "#clear" ) ) {
		// clear list of precached 
	}
	else {
		// check for dupes, append at end
	}
}

int AS_AddLocalSet( const char *name, vec3_t listener_origin, vec3_t origin, int entID, int time ) {
	return 0;
}

sfxHandle_t AS_GetBModelSound( const char *name, int stage ) {
	return 0;
}