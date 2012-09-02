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

#include "tr_local.h"
#include "../qcommon/qcommon.h"

qhandle_t RE_RegisterFont( const char *fontName ) {
	qhandle_t	hFont;
	font_t		*font;
	dfontdat_t	*fontData;
	union {
		byte *b;
		void *v;
	} buffer;
	int len;
	int i;
	char name[MAX_QPATH], nametga[MAX_QPATH];

	if( !fontName ) {
		ri.Printf( PRINT_ALL, "RE_RegisterFont: called with empty name\n" );
		return 0;
	}

	if ( strlen( fontName ) >= MAX_QPATH ) {
		ri.Error( ERR_FATAL, "Font name exceeds MAX_QPATH\n" );
		return 0;
	}

	Com_sprintf( name, sizeof( name ), "fonts/%s.fontdat", fontName );
	Com_sprintf( nametga, sizeof( nametga ), "fonts/%s.tga", fontName );

	len = ri.FS_ReadFile( name, NULL );

	// catch the fonts/fonts/reallybigfont case or other mod author failures early
	if (len < 0) {
		return 0;
	}

	// see if the font is already loaded
	for ( hFont = 1; hFont < tr.numFonts ; hFont++ ) {
		font = tr.fonts[hFont];
		if ( !Q_stricmp( font->name, name ) ) {
			if( font->imageHandle == 0 ) {
				return 0;		// default font (none)
			}
			return hFont;
		}
	}

	// allocate a new font
	if ( tr.numFonts == MAX_FONTS ) {
		ri.Printf( PRINT_WARNING, "WARNING: RE_RegisterFont( '%s' ) MAX_FONTS hit\n", name );
		return 0;
	}
	tr.numFonts++;
	font = ri.Hunk_Alloc( sizeof( font_t ), h_low );
	tr.fonts[hFont] = font;
	Q_strncpyz( font->name, name, sizeof( font->name ) );
	font->imageHandle = RE_RegisterShaderNoMip( nametga );

	// make sure the render thread is stopped
	R_SyncRenderThread();

	// load and parse the font file
	len = ri.FS_ReadFile( name, &buffer.v );

	if (!buffer.b || len < 0) {
		return 0;
	}

	if( len != sizeof(dfontdat_t) ) {
		ri.Error( ERR_DROP, "RE_RegisterFont: fontdat too short (%s)", name );
	}

	fontData = ri.Hunk_Alloc( sizeof( dfontdat_t ), h_low );
	fontData = (dfontdat_t *)buffer.b;

	for(i = 0; i < GLYPH_COUNT; i++) {
		fontData->mGlyphs[i].width = LittleShort(fontData->mGlyphs[i].width);
		fontData->mGlyphs[i].height = LittleShort(fontData->mGlyphs[i].height);
		fontData->mGlyphs[i].horizAdvance = LittleShort(fontData->mGlyphs[i].horizAdvance);
		fontData->mGlyphs[i].horizOffset = LittleShort(fontData->mGlyphs[i].horizOffset);
		fontData->mGlyphs[i].baseline = LittleLong(fontData->mGlyphs[i].baseline);
		fontData->mGlyphs[i].s = LittleFloat(fontData->mGlyphs[i].s);
		fontData->mGlyphs[i].t = LittleFloat(fontData->mGlyphs[i].t);
		fontData->mGlyphs[i].s2 = LittleFloat(fontData->mGlyphs[i].s2);
		fontData->mGlyphs[i].t2 = LittleFloat(fontData->mGlyphs[i].t2);
	}
	fontData->mPointSize = LittleShort(fontData->mPointSize);
	fontData->mHeight = LittleShort(fontData->mHeight);
	fontData->mAscender = LittleShort(fontData->mAscender);
	fontData->mDescender = LittleShort(fontData->mDescender);
	fontData->mKoreanHack = LittleShort(fontData->mKoreanHack);

    // Hack to fix the numbers apparently.
    if( !fontData->mHeight ) {
        float pointSize = fontData->mPointSize;
        float a = pointSize * 0.1f + 2.5f;

        fontData->mHeight = (short)pointSize;
        fontData->mAscender = (short)(pointSize - a);
        fontData->mDescender = (short)(pointSize - fontData->mAscender);
		ri.Printf( PRINT_DEVELOPER, "RE_RegisterFont( '%s' ) font contains empty height. estimating... to %hi\n", name, fontData->mHeight);
    }

	Com_Memcpy( &font->fontData, fontData, sizeof( dfontdat_t ) );

	ri.FS_FreeFile( buffer.v );

	if ( font->imageHandle == 0 ) {
		return 0;		// use default font
	}

	return hFont;
}

/*
===============
R_InitFonts
===============
*/
void	R_InitFonts( void ) {
	font_t		*font;

	tr.numFonts = 1;

	// make the default font be blank
	font = tr.fonts[0] = ri.Hunk_Alloc( sizeof( font_t ), h_low );
	Q_strncpyz( font->name, "<default font>", sizeof( font->name )  );
	font->imageHandle = 0;
	Com_Memset( &font->fontData, 0, sizeof( dfontdat_t ) );
}

/*
===============
R_GetFontByHandle
===============
*/
font_t	*R_GetFontByHandle( qhandle_t hFont ) {
	if ( hFont < 1 || hFont >= tr.numFonts ) {
		return tr.fonts[0];
	}
	return tr.fonts[ hFont ];
}

/*
===============
R_FontList_f
===============
*/
void	R_FontList_f( void ) {
	int			i;
	font_t		*font;

	ri.Printf (PRINT_ALL, "------------------\n");

	for ( i = 0 ; i < tr.numFonts ; i++ ) {
		font = tr.fonts[i];

		if(!font)
			break;

		ri.Printf( PRINT_ALL, "%3i:%s  ps:%hi h:%hi a:%hi d:%hi k:%hi\n", i, font->name,
			font->fontData.mPointSize, font->fontData.mHeight, font->fontData.mAscender,
			font->fontData.mDescender, font->fontData.mKoreanHack);
	}
	ri.Printf (PRINT_ALL, "------------------\n");
}

int RE_Font_StrLenPixels( const char *text, const int iFontIndex, const float scale ) {
	font_t *font = R_GetFontByHandle( iFontIndex );
	glyphInfo_t *glyph;
	const char *s = text;
	int count, len;
	float out;

	if( !font || font->imageHandle == 0 )
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
				glyph = &font->fontData.mGlyphs[(unsigned char)*s];
				out  += glyph->horizAdvance;
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
	font_t *font = R_GetFontByHandle( iFontIndex );

	if( !font || font->imageHandle == 0 )
		return 0;
	
	return font->fontData.mHeight * scale;
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

static vec4_t dropShadow = {0.2f, 0.2f, 0.2f, 1};
void RE_Font_DrawString( int ox, int oy, const char *text, const float *rgba, const int setIndex, int iCharLimit, const float scale ) {
	qhandle_t fontIndex = setIndex;
	qhandle_t shader;
	font_t *font;
	glyphInfo_t *glyph;
	int len, count;
	vec4_t newColor;
	float fox = (float)ox;
	float foy = (float)oy;

	fontIndex &= ~STYLE_DROPSHADOW;
	fontIndex &= ~STYLE_BLINK;
	font = R_GetFontByHandle( fontIndex );
	shader = font->imageHandle;

	if( !font || shader == 0 || scale <= 0 )
		return;

	if( text ) {
		const char *s = text;
        float yoffset = 0;

		//if ((setIndex & STYLE_BLINK) && ((cls.realtime/BLINK_DIVISOR) & 1))
		//	return;

		RE_SetColor( rgba );
		memcpy( &newColor[0], &rgba[0], sizeof( vec4_t ) );
		len = strlen( text );
		if ( iCharLimit > 0 && len > iCharLimit ) {
			len = iCharLimit;
		}

		count = 0;
        yoffset = oy + (int)(0.5f + scale * (font->fontData.mHeight - (font->fontData.mDescender * 0.5f)));
		while( s && *s && count < len ) {
			glyph = &font->fontData.mGlyphs[(unsigned char)*s];
			if( Q_IsColorString( s ) ) {
				memcpy( newColor, ColorForIndex(ColorIndex( *( s + 1 ) )), sizeof( newColor ) );
				newColor[3] = rgba[3];
				RE_SetColor( newColor );
				s += 2;
				continue;
			}
			else {
				/*   ----------------------- ascender = highest point above baseline
				 *     F o n t
				 *   ----------------------- baseline = line all chars rest on
				 *   ----------------------- descender = lowest point below baseline
				 */

                float yadj = scale * glyph->baseline;

				if( setIndex & STYLE_DROPSHADOW ) {
					dropShadow[3] = newColor[3];
					RE_SetColor( dropShadow );
					RE_Font_PaintChar( (fox + ( glyph->horizOffset * scale ))+1, ((yoffset - yadj))+1, glyph->width, glyph->height, scale, glyph->s, glyph->t, glyph->s2, glyph->t2, shader );
					RE_SetColor( newColor );
					dropShadow[3] = 1.0;
				}

				RE_Font_PaintChar( fox + ( glyph->horizOffset * scale ), (yoffset - yadj), glyph->width, glyph->height, scale, glyph->s, glyph->t, glyph->s2, glyph->t2, shader );
				fox += ( glyph->horizAdvance * scale );
				s++;
				count++;
			}
		}
		RE_SetColor( NULL );
	}
}
