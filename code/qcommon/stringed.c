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

typedef struct stringRef_s {
	char *key;
	char *value;

	struct stringRef_s* next;
} stringRef_t;

#define HASH_SIZE		2048
static	stringRef_t*	hashTable[HASH_SIZE];

stringRef_t current;
size_t numStrings;
size_t capacity;
stringRef_t *strings;

void SE_InitLang( void/*const char *lang*/ );
void SE_Init( void/*int initialSize*/ )
{
	Com_Memset(&current, 0, sizeof(current));
	Com_Memset(hashTable, 0, sizeof(hashTable));
	strings = (stringRef_t *)malloc (sizeof (stringRef_t) * 4096/*initialSize*/);
	numStrings = 0;
	capacity = 4096/*initialSize*/;
	SE_InitLang();
}

static stringRef_t *Alloc( void )
{
	stringRef_t *str = NULL;
	if ( numStrings >= capacity )
	{
		int newSize = (int)(capacity * 1.5f);
		stringRef_t *newStrings = (stringRef_t *)realloc (strings, sizeof (stringRef_t) * newSize);
		if ( newStrings == NULL )
		{
			Com_Error (ERR_FATAL, "Failed to allocate memory for additional string references.\n");
			return NULL;
		}

		strings = newStrings;
		capacity = newSize;
	}

	str = &strings[numStrings];
	numStrings++;

	return str;
}

void SE_Shutdown( void )
{
	int i;

	for(i = 0; i < numStrings; i++)
	{
		free(strings[i].key);
		free(strings[i].value);
	}

	free(strings);
	strings = NULL;
	numStrings = 0;
}

/*
================
return a hash value for the filename
================
*/
#ifdef __GNUCC__
  #warning TODO: check if long is ok here 
#endif
static long generateHashValue(const char *fname, const int size)
{
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		if (letter == PATH_SEP) letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (size-1);
	return hash;
}

stringRef_t *FindString( const char *key )
{
	int			hash;
	stringRef_t	*str;

	if ( key[0] == 0 ) {
		return NULL;
	}

	hash = generateHashValue(key, HASH_SIZE);

	//
	// see if the string is already loaded
	//
	for (str = hashTable[hash]; str; str = str->next) {
		if ( !Q_stricmp( str->key, key ) ) {
			// match found
			return str;
		}
	}

	return NULL;
}

// replaces all occurances of "\n" with '\n'
char *SE_NewLines(char *str) {
	char *d, *c, *s;

	if(!*str)
		return str;
	s = str;
	while(*str) {
		if(*str == '\\') {
			d = str;
			c = str;
			*str++;
			if(*str && *str == 'n') {
				*d = '\n';
				while(*++str) {
					*++d = *str;
				}
				*++d = '\0';
				str = c;
				continue;
			}
		}
		*str++;
	}
	return s;
}

void SE_Load( const char *title, const char *language ) {
	union {
		char	*c;
		void	*v;
	} langfile;
	char *text_p;
	int hash;

	FS_ReadFile( language, &langfile.v );
	if ( !langfile.c ) {
		Com_Printf( S_COLOR_YELLOW "SE_Load(): Couldn't load \"%s\"!\n", language );
		return;
	}

	text_p = langfile.c;

	SE_BeginParseSession( language );

	while(1)
	{
		char *token = SE_Parse( &text_p );
		if( !token || !token[0] )
		{
			break;
		}

		if(!strcmp(token, "ENDMARKER"))
		{
			break;
		}
		else if(!strcmp(token, "VERSION"))
		{
			SE_SkipRestOfLine(&text_p);
		}
		else if(!strcmp(token, "CONFIG"))
		{
			SE_SkipRestOfLine(&text_p);
		}
		else if(!strcmp(token, "FILENOTES"))
		{
			SE_SkipRestOfLine(&text_p);
		}
		else if(!strcmp(token, "REFERENCE"))
		{
			char *reftok = SE_ParseExt(&text_p, qfalse);
			if(reftok && reftok[0])
			{
				if ( strlen( reftok ) >= MAX_QPATH ) {
					Com_Error( ERR_FATAL, "String reference %s exceeds MAX_QPATH", reftok );
				}
				Com_Memset(&current, 0, sizeof(current));
				current.key = (char *)malloc(strlen(title)+1+strlen(reftok)+1);
				strcpy(current.key, title);
				strcat(current.key, "_");
				strcat(current.key, reftok);

				while(1)
				{
					char *tok = SE_Parse(&text_p);
					if(!tok || !tok[0])
					{
						break;
					}
					if(!strcmp(tok, "ENDMARKER"))
					{
						break;
					}
					else if(!strcmp(tok, "NOTES"))
					{
						SE_SkipRestOfLine(&text_p);
					}
					else if(!Q_strncmp(tok, "LANG_", 5))
					{
						char *valtok = SE_ParseExt(&text_p, qfalse);
						if(valtok && valtok[0])
						{
							char *temp;
							if ( strlen( valtok ) >= BIG_INFO_STRING ) {
								Com_Error( ERR_FATAL, "String value in ref %s exceeds BIG_INFO_STRING", reftok );
							}
							temp = SE_NewLines(valtok);
							current.value = (char *)malloc(strlen(temp)+1);
							strcpy(current.value, temp);
						}
						break;
					}
				}

				if(current.value && current.value[0])
				{
					stringRef_t *str = Alloc();
					*str = current;
					hash = generateHashValue(current.key, HASH_SIZE);
					str->next = hashTable[hash];
					hashTable[hash] = str;
				}
			}
		}
	}

	SE_BeginParseSession( "" );

	FS_FreeFile( langfile.v );
}

void SE_InitLang( void/*const char *lang*/ )
{
	char    **fileList;
	int numFiles, i;
	char	title[MAX_QPATH];

	fileList = FS_ListFiles( "strings/english", ".str", &numFiles );

	if( numFiles > 128 )
		numFiles = 128;

	for ( i = 0; i < numFiles; i++ ) {
		COM_StripExtension( fileList[i], title, MAX_QPATH );
		SE_Load( title, va( "strings/english/%s", fileList[i] ) );
	}

	FS_FreeFileList(fileList);
}

// Compare is expected in FILENAME_REFERENCE format
// above stores FILENAME_(each reference) into each individual .reference
int SE_GetStringBuffer( const char *compare, char *buffer, int bufferSize ) {
	stringRef_t *str;

	if( !buffer )
		return 0;

	str = FindString( compare );

	if( str && str->value && str->value[0] ) {
		Q_strncpyz( buffer, str->value, bufferSize );
		return 1;
	}
	else
		*buffer = '\0';

	return 0;
}

char *SE_GetString( const char *compare ) {
	static char text[2][MAX_STRING_CHARS] = { 0 };
	static int index = 0;
	stringRef_t *str;

	str = FindString( compare );

	index ^= 1;

	if( str && str->value && str->value[0] ) {
		Q_strncpyz( text[index], str->value, sizeof(text[0]) );
		return text[index];
	}
	else
		return "";
}
