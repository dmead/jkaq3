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

#if 0

#include "../botlib/botlib.h"

extern	botlib_export_t	*botlib_export;

#define MAX_STRING_CATEGORIES 100

void SE_Load( const char *language ) {
	int handle;
	//og::Lexer lex(og::LEXER_NO_BOM_WARNING);
	//og::String config, notes;

	//if( !lex.LoadFile( language ) )
	//	return;

	try {
		lex.ExpectToken( "VERSION" );
		int version = lex.ReadInt();
		lex.ExpectToken( "CONFIG" );
		config = lex.ReadString();
		lex.ExpectToken( "NOTES" );
		notes = lex.ReadString();

		const og::Token *token;

		const char *p;

		while ( (token = lex.ReadToken()) != NULL ) {
			p = token->GetString();
			if ( !p || !*p )
				continue;

		}

		lex.ExpectToken( "ENDMARKER" );
	}
	catch( og::LexerError err ) {
		og::String errStr;
		err.ToString( errStr );
		og::User::Error( og::ERR_LEXER_FAILURE, errStr.c_str(), language );
	}
}

void SE_Init( void ) {
	if( og::FileList * files = og::FS->GetFileList( "strings/english", ".str", og::LF_DEFAULT | og::LF_REMOVE_DIR ) ) {
		for( int i = 0; i < files->Num(); i++ ) {
			SE_Load( files->GetName( i ) );
		}
	}
}

#endif
