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
// tr_font.c
// 
//
// The font system uses FreeType 2.x to render TrueType fonts for use within the game.
// As of this writing ( Nov, 2000 ) Team Arena uses these fonts for all of the ui and 
// about 90% of the cgame presentation. A few areas of the CGAME were left uses the old 
// fonts since the code is shared with standard Q3A.
//
// If you include this font rendering code in a commercial product you MUST include the
// following somewhere with your product, see www.freetype.org for specifics or changes.
// The Freetype code also uses some hinting techniques that MIGHT infringe on patents 
// held by apple so be aware of that also.
//
// As of Q3A 1.25+ and Team Arena, we are shipping the game with the font rendering code
// disabled. This removes any potential patent issues and it keeps us from having to 
// distribute an actual TrueTrype font which is 1. expensive to do and 2. seems to require
// an act of god to accomplish. 
//
// What we did was pre-render the fonts using FreeType ( which is why we leave the FreeType
// credit in the credits ) and then saved off the glyph data and then hand touched up the 
// font bitmaps so they scale a bit better in GL.
//
// There are limitations in the way fonts are saved and reloaded in that it is based on 
// point size and not name. So if you pre-render Helvetica in 18 point and Impact in 18 point
// you will end up with a single 18 point data file and image set. Typically you will want to 
// choose 3 sizes to best approximate the scaling you will be doing in the ui scripting system
// 
// In the UI Scripting code, a scale of 1.0 is equal to a 48 point font. In Team Arena, we
// use three or four scales, most of them exactly equaling the specific rendered size. We 
// rendered three sizes in Team Arena, 12, 16, and 20. 
//
// To generate new font data you need to go through the following steps.
// 1. delete the fontImage_x_xx.tga files and fontImage_xx.dat files from the fonts path.
// 2. in a ui script, specificy a font, smallFont, and bigFont keyword with font name and 
//    point size. the original TrueType fonts must exist in fonts at this point.
// 3. run the game, you should see things normally.
// 4. Exit the game and there will be three dat files and at least three tga files. The 
//    tga's are in 256x256 pages so if it takes three images to render a 24 point font you 
//    will end up with fontImage_0_24.tga through fontImage_2_24.tga
// 5. You will need to flip the tga's in Photoshop as the tga output code writes them upside
//    down.
// 6. In future runs of the game, the system looks for these images and data files when a s
//    specific point sized font is rendered and loads them for use. 
// 7. Because of the original beta nature of the FreeType code you will probably want to hand
//    touch the font bitmaps.
// 
// Currently a define in the project turns on or off the FreeType code which is currently 
// defined out. To pre-render new fonts you need enable the define ( BUILD_FREETYPE ) and 
// uncheck the exclude from build check box in the FreeType2 area of the Renderer project. 


#include "tr_local.h"
#include "../qcommon/qcommon.h"

static int fdOffset;
static byte *fdFile;

typedef union {
	byte  fred[4];
	short ffred;
} poor_s;

short readShort( void ) {
	poor_s me;
#if defined Q3_BIG_ENDIAN
	me.fred[0] = fdFile[fdOffset + 3];
	me.fred[1] = fdFile[fdOffset + 2];
#elif defined Q3_LITTLE_ENDIAN
	me.fred[0] = fdFile[fdOffset + 0];
	me.fred[1] = fdFile[fdOffset + 1];
#endif
	fdOffset += 2;
	return me.ffred;
}
int readInt( void ) {
	int i = fdFile[fdOffset] + ( fdFile[fdOffset + 1] << 8 ) + ( fdFile[fdOffset + 2] << 16 ) + ( fdFile[fdOffset + 3] << 24 );
	fdOffset += 4;
	return i;
}
typedef union {
	byte  fred[4];
	float ffred;
} poor;

float readFloat( void ) {
	poor me;
#if defined Q3_BIG_ENDIAN
	me.fred[0] = fdFile[fdOffset + 3];
	me.fred[1] = fdFile[fdOffset + 2];
	me.fred[2] = fdFile[fdOffset + 1];
	me.fred[3] = fdFile[fdOffset + 0];
#elif defined Q3_LITTLE_ENDIAN
	me.fred[0] = fdFile[fdOffset + 0];
	me.fred[1] = fdFile[fdOffset + 1];
	me.fred[2] = fdFile[fdOffset + 2];
	me.fred[3] = fdFile[fdOffset + 3];
#endif
	fdOffset += 4;
	return me.ffred;
}

/* This has to be done better because ocr_a is loaded as smallFont at ingame.menu for cgame */

#define MAX_FONTS 4
static dfontdat_t registeredFont[MAX_FONTS];
static char registeredFontNames[MAX_FONTS][64];
static qhandle_t registeredFontHandles[MAX_FONTS];
static int registeredFontOffsets[MAX_FONTS] = { 2, 1, 3, 4 };
qboolean small_font_hack = qfalse; // qfalse for UI - aurabesh, qtrue for CGAME/ingame - ocr_a

dfontdat_t *FontFromHandle( int offset ) {
	if( offset == 1 ) {
		return &registeredFont[FONT_MEDIUM-1];
	} else if( offset == 2 ) {
		return &registeredFont[FONT_SMALL-1];
	} else if( offset == 3 ) {
		return &registeredFont[FONT_LARGE-1];
	} else if( offset == 4 ) {
		return &registeredFont[FONT_SMALL2-1];
	} else {
		return &registeredFont[FONT_MEDIUM-1];
	}
}
qhandle_t ShaderFromHandle( int offset ) {
	if( offset == 1 ) {
		return registeredFontHandles[FONT_MEDIUM-1];
	} else if( offset == 2 ) {
		return registeredFontHandles[FONT_SMALL-1];
	} else if( offset == 3 ) {
		return registeredFontHandles[FONT_LARGE-1];
	} else if( offset == 4 ) {
		return registeredFontHandles[FONT_SMALL2-1];
	} else {
		return registeredFontHandles[FONT_MEDIUM-1];
	}
}
void R_InitFont( const int index, const char *fontName ) {
	dfontdat_t *font;
	void *faceData;
	int i, len;
	char name[64], nametga[64];
	qhandle_t hnd;
	short dummy;

	if( !fontName ) {
		ri.Printf( PRINT_ALL, "RE_InitFont: called with empty name\n" );
		return;
	}

	if( index < 0 || index > MAX_FONTS ) {
		ri.Printf( PRINT_ALL, "RE_InitFont: called with bad index: %i\n", index );
		return;
	}

	// make sure the render thread is stopped
	R_SyncRenderThread();

	Com_sprintf( name, sizeof( name ), "fonts/%s.fontdat", fontName );

	len = ri.FS_ReadFile( name, NULL );

	if( len == sizeof( dfontdat_t ) ) {
		ri.FS_ReadFile( name, &faceData );
		font = (dfontdat_t *)ri.Hunk_AllocateTempMemory( len );
		fdOffset = 0;
		fdFile = (byte *)faceData;
		//file->Seek( 896, SEEK_SET );
		for( i = 0; i < 32; i++ ) {
			font->mGlyphs[i].width        = 0;
			font->mGlyphs[i].height       = 0;
			font->mGlyphs[i].horizAdvance = 0;
			font->mGlyphs[i].horizOffset  = 0;
			font->mGlyphs[i].baseline     = 0;
			font->mGlyphs[i].s            = 0;
			font->mGlyphs[i].t            = 0;
			font->mGlyphs[i].s2           = 0;
			font->mGlyphs[i].t2           = 0;
			readShort();
			readShort();
			readShort();
			readShort();
			readInt();
			readFloat();
			readFloat();
			readFloat();
			readFloat();
		}

		for( i = 32; i < GLYPH_COUNT; i++ ) {
			font->mGlyphs[i].width        = readShort();
			font->mGlyphs[i].height       = readShort();
			font->mGlyphs[i].horizAdvance = readShort();
			font->mGlyphs[i].horizOffset  = readShort();
			font->mGlyphs[i].baseline     = readInt();
			font->mGlyphs[i].s            = readFloat();
			font->mGlyphs[i].t            = readFloat();
			font->mGlyphs[i].s2           = readFloat();
			font->mGlyphs[i].t2           = readFloat();
		}

		/* Looks like something missing from the spec (maybe :s) */
		dummy             = readShort();
		font->mPointSize  = readShort();
		font->mHeight     = readShort();
		font->mAscender   = readShort();
		font->mDescender  = readShort();
		font->mKoreanHack = readShort();
	}

	Com_Memcpy( &registeredFont[index-1], font, sizeof( dfontdat_t ) );

	ri.Hunk_FreeTempMemory( font );

	Q_strncpyz( registeredFontNames[index-1], name, sizeof( registeredFontNames[0] ) );
	Com_sprintf( nametga, sizeof( nametga ), "fonts/%s.tga", fontName );
	hnd = RE_RegisterShaderNoMip( nametga );
	registeredFontHandles[index-1] = hnd;
}
void R_InitFonts( void ) {
	/* Basejka stuffs:
	register font returns 1 for medium
	register font returns 2 for small
	register font returns 3 for big
	register font returns 4 for small2
	*/

	R_InitFont( FONT_MEDIUM, "ergoec" );
	R_InitFont( FONT_SMALL, "aurabesh" );
	R_InitFont( FONT_LARGE, "anewhope" );
	R_InitFont( FONT_SMALL2, "arialnb" );

}
qhandle_t RE_RegisterFont( const char *fontName ) {
	int i;
	char name[64];

	if( !small_font_hack && !Q_stricmp( fontName, "ocr_a" ) ) {
		R_InitFont( FONT_SMALL, "ocr_a" );
		small_font_hack = qtrue;
	}

	if( small_font_hack && !Q_stricmp( fontName, "aurabesh" ) ) {
		R_InitFont( FONT_SMALL, "aurabesh" );
		small_font_hack = qfalse;
	}

	Com_sprintf( name, sizeof( name ), "fonts/%s.fontdat", fontName );
	for( i = 0; i < MAX_FONTS; i++ ) {
		if( Q_stricmp( name, registeredFontNames[i] ) == 0 ) {
			return registeredFontOffsets[i];
			//return registeredFontHandles[i];
		}
	}
	return 0;
}
int RE_Font_StrLenPixels( const char *text, const int iFontIndex, const float scale ) {
	dfontdat_t *font = FontFromHandle( iFontIndex );
	glyphInfo_t *glyph;
	const char *s = text;
	int count, len;
	float out;

	if( !font )
		return 0;

	if( scale <= 0 )
		return 0;

	if( !text )
		return 0;

	out = 0;

	if( text ) {
		len = strlen( text );

		count = 0;
		while( s && *s && count < len ) {
			if( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			else {
				glyph = &font->mGlyphs[(unsigned char)*s];
				out  += glyph->width;
				s++;
				count++;
			}
		}
	}

	return out * scale;
}
int RE_Font_StrLenChars( const char *text ) {
	return strlen( text );
}
int RE_Font_HeightPixels( const int iFontIndex, const float scale ) {
	dfontdat_t *font = FontFromHandle( iFontIndex );

	if( !font )
		return 0;
	
	return font->mHeight * scale;
}
void Font_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	float xscale;
	float yscale;

	// scale for screen sizes
	xscale = glConfig.vidWidth / 640.0;
	yscale = glConfig.vidHeight / 480.0;
	if( x )
		*x *= xscale;

	if( y )
		*y *= yscale;

	if( w )
		*w *= xscale;

	if( h )
		*h *= yscale;
}
void RE_Font_PaintChar( float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader ) {
	float w, h;
	w = width * scale;
	h = height * scale;
	Font_AdjustFrom640( &x, &y, &w, &h );
	RE_StretchPic( x, y, w, h, s, t, s2, t2, hShader );
}
void RE_Font_DrawString( int ox, int oy, const char *text, const float *rgba, const int setIndex, int iCharLimit, const float scale ) {
	qhandle_t fontIndex = setIndex;
	qhandle_t shader;
	dfontdat_t *font;
	glyphInfo_t *glyph;
	int len, count;
	vec4_t newColor;

	// Fuckers :(
	if( fontIndex & STYLE_DROPSHADOW )
		fontIndex -= STYLE_DROPSHADOW;

	fontIndex &= ~STYLE_BLINK;
	font = FontFromHandle( fontIndex );
	shader = ShaderFromHandle( fontIndex );

	if( !font || scale <= 0 )
		return;

	if( text ) {
		const char *s = text;
		RE_SetColor( rgba );
		memcpy( &newColor[0], &rgba[0], sizeof( vec4_t ) );
		len = strlen( text );
		if ( iCharLimit > 0 && len > iCharLimit ) {
			len = iCharLimit;
		}

		count = 0;
		while( s && *s && count < len ) {
			glyph = &font->mGlyphs[(unsigned char)*s];
			if( Q_IsColorString( s ) ) {
				memcpy( newColor, g_color_table[ColorIndex( *( s + 1 ) )], sizeof( newColor ) );
				newColor[3] = rgba[3];
				RE_SetColor( newColor );
				s += 2;
				continue;
			}
			else {
				float yadj = /*scale **/ glyph->baseline;
				RE_Font_PaintChar( ox + glyph->horizOffset, (oy - yadj)+(font->mAscender), glyph->width, glyph->height, scale, glyph->s, glyph->t, glyph->s2, glyph->t2, shader );
				ox += ( glyph->horizAdvance * scale );
				s++;
				count++;
			}
		}
		RE_SetColor( NULL );
	}
}
