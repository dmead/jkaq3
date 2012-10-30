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
#include "client.h"

/*

key up events are sent even if in console mode

*/

field_t	historyEditLines[COMMAND_HISTORY];

int			nextHistoryLine;		// the last line in the history buffer, not masked
int			historyLine;	// the line being displayed from history buffer
							// will be <= nextHistoryLine

field_t		g_consoleField;
field_t		chatField;
qboolean	chat_team;

int			chat_playerNum;

qboolean	key_overstrikeMode;

int				anykeydown;
qkey_t		keys[MAX_KEYS];

typedef struct {
	char	*name;
	int		keynum;
} keyname_t;

// A_UNDEFINED_7 -> COMMAND
// A_UNDEFINED_8 -> F13
// A_UNDEFINED_9 -> F14
// A_UNDEFINED_10 -> F15
// A_UNDEFINED_11 -> KP_SLASH
// A_UNDEFINED_12 -> KP_STAR
// A_UNDEFINED_13 -> WINDOWS
// A_UNDEFINED_14 -> COMPOSE
// A_UNDEFINED_15 -> MODE
// A_UNDEFINED_16 -> HELP
// A_UNDEFINED_17 -> SYSREQ
// A_UNDEFINED_18 -> BREAK
// A_UNDEFINED_19 -> MENU
// A_UNDEFINED_20 -> POWER
// A_UNDEFINED_21 -> UNDO

// names not in this list can either be lowercase ascii, or '0xnn' hex sequences
keyname_t keynames[] =
{
	{"TAB", A_TAB},
	{"ENTER", A_ENTER},
	{"ESCAPE", A_ESCAPE},
	{"SPACE", A_SPACE},
	{"BACKSPACE", A_BACKSPACE},
	{"UPARROW", A_CURSOR_UP},
	{"DOWNARROW", A_CURSOR_DOWN},
	{"LEFTARROW", A_CURSOR_LEFT},
	{"RIGHTARROW", A_CURSOR_RIGHT},

	{"ALT", A_ALT},
	{"CTRL", A_CTRL},
	{"SHIFT", A_SHIFT},

	{"COMMAND", A_UNDEFINED_7},

	{"CAPSLOCK", A_CAPSLOCK},

	{"F1", A_F1},
	{"F2", A_F2},
	{"F3", A_F3},
	{"F4", A_F4},
	{"F5", A_F5},
	{"F6", A_F6},
	{"F7", A_F7},
	{"F8", A_F8},
	{"F9", A_F9},
	{"F10", A_F10},
	{"F11", A_F11},
	{"F12", A_F12},
	{"F13", A_UNDEFINED_8},
	{"F14", A_UNDEFINED_9},
	{"F15", A_UNDEFINED_10},

	{"INS", A_INSERT},
	{"DEL", A_DELETE},
	{"PGDN", A_PAGE_DOWN},
	{"PGUP", A_PAGE_UP},
	{"HOME", A_HOME},
	{"END", A_END},

	{"MOUSE1", A_MOUSE1},
	{"MOUSE2", A_MOUSE2},
	{"MOUSE3", A_MOUSE3},
	{"MOUSE4", A_MOUSE4},
	{"MOUSE5", A_MOUSE5},

	{"MWHEELUP",	A_MWHEELUP },
	{"MWHEELDOWN",	A_MWHEELDOWN },

	{"JOY0", A_JOY0},
	{"JOY1", A_JOY1},
	{"JOY2", A_JOY2},
	{"JOY3", A_JOY3},
	{"JOY4", A_JOY4},
	{"JOY5", A_JOY5},
	{"JOY6", A_JOY6},
	{"JOY7", A_JOY7},
	{"JOY8", A_JOY8},
	{"JOY9", A_JOY9},
	{"JOY10", A_JOY10},
	{"JOY11", A_JOY11},
	{"JOY12", A_JOY12},
	{"JOY13", A_JOY13},
	{"JOY14", A_JOY14},
	{"JOY15", A_JOY15},
	{"JOY16", A_JOY16},
	{"JOY17", A_JOY17},
	{"JOY18", A_JOY18},
	{"JOY19", A_JOY19},
	{"JOY20", A_JOY20},
	{"JOY21", A_JOY21},
	{"JOY22", A_JOY22},
	{"JOY23", A_JOY23},
	{"JOY24", A_JOY24},
	{"JOY25", A_JOY25},
	{"JOY26", A_JOY26},
	{"JOY27", A_JOY27},
	{"JOY28", A_JOY28},
	{"JOY29", A_JOY29},
	{"JOY30", A_JOY30},
	{"JOY31", A_JOY31},

	{"AUX0", A_AUX0},
	{"AUX1", A_AUX1},
	{"AUX2", A_AUX2},
	{"AUX3", A_AUX3},
	{"AUX4", A_AUX4},
	{"AUX5", A_AUX5},
	{"AUX6", A_AUX6},
	{"AUX7", A_AUX7},
	{"AUX8", A_AUX8},
	{"AUX9", A_AUX9},
	{"AUX10", A_AUX10},
	{"AUX11", A_AUX11},
	{"AUX12", A_AUX12},
	{"AUX13", A_AUX13},
	{"AUX14", A_AUX14},
	{"AUX15", A_AUX15},
	{"AUX16", A_AUX16},
	{"AUX17", A_AUX17},
	{"AUX18", A_AUX18},
	{"AUX19", A_AUX19},
	{"AUX20", A_AUX20},
	{"AUX21", A_AUX21},
	{"AUX22", A_AUX22},
	{"AUX23", A_AUX23},
	{"AUX24", A_AUX24},
	{"AUX25", A_AUX25},
	{"AUX26", A_AUX26},
	{"AUX27", A_AUX27},
	{"AUX28", A_AUX28},
	{"AUX29", A_AUX29},
	{"AUX30", A_AUX30},
	{"AUX31", A_AUX31},

	{"KP_HOME",			A_KP_7 },
	{"KP_UPARROW",		A_KP_8 },
	{"KP_PGUP",			A_KP_9 },
	{"KP_LEFTARROW",	A_KP_4 },
	{"KP_5",			A_KP_5 },
	{"KP_RIGHTARROW",	A_KP_6 },
	{"KP_END",			A_KP_1 },
	{"KP_DOWNARROW",	A_KP_2 },
	{"KP_PGDN",			A_KP_3 },
	{"KP_ENTER",		A_KP_ENTER },
	{"KP_INS",			A_KP_0 },
	{"KP_DEL",			A_KP_PERIOD },
	{"KP_SLASH",		A_UNDEFINED_11 },
	{"KP_MINUS",		A_KP_MINUS },
	{"KP_PLUS",			A_KP_PLUS },
	{"KP_NUMLOCK",		A_NUMLOCK },
	{"KP_STAR",			A_UNDEFINED_12 },
	//{"KP_EQUALS",		A_KP_EQUALS },

	{"PAUSE", A_PAUSE},
	
	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

#if 0
	{"WORLD_0", K_WORLD_0},
	{"WORLD_1", K_WORLD_1},
	{"WORLD_2", K_WORLD_2},
	{"WORLD_3", K_WORLD_3},
	{"WORLD_4", K_WORLD_4},
	{"WORLD_5", K_WORLD_5},
	{"WORLD_6", K_WORLD_6},
	{"WORLD_7", K_WORLD_7},
	{"WORLD_8", K_WORLD_8},
	{"WORLD_9", K_WORLD_9},
	{"WORLD_10", K_WORLD_10},
	{"WORLD_11", K_WORLD_11},
	{"WORLD_12", K_WORLD_12},
	{"WORLD_13", K_WORLD_13},
	{"WORLD_14", K_WORLD_14},
	{"WORLD_15", K_WORLD_15},
	{"WORLD_16", K_WORLD_16},
	{"WORLD_17", K_WORLD_17},
	{"WORLD_18", K_WORLD_18},
	{"WORLD_19", K_WORLD_19},
	{"WORLD_20", K_WORLD_20},
	{"WORLD_21", K_WORLD_21},
	{"WORLD_22", K_WORLD_22},
	{"WORLD_23", K_WORLD_23},
	{"WORLD_24", K_WORLD_24},
	{"WORLD_25", K_WORLD_25},
	{"WORLD_26", K_WORLD_26},
	{"WORLD_27", K_WORLD_27},
	{"WORLD_28", K_WORLD_28},
	{"WORLD_29", K_WORLD_29},
	{"WORLD_30", K_WORLD_30},
	{"WORLD_31", K_WORLD_31},
	{"WORLD_32", K_WORLD_32},
	{"WORLD_33", K_WORLD_33},
	{"WORLD_34", K_WORLD_34},
	{"WORLD_35", K_WORLD_35},
	{"WORLD_36", K_WORLD_36},
	{"WORLD_37", K_WORLD_37},
	{"WORLD_38", K_WORLD_38},
	{"WORLD_39", K_WORLD_39},
	{"WORLD_40", K_WORLD_40},
	{"WORLD_41", K_WORLD_41},
	{"WORLD_42", K_WORLD_42},
	{"WORLD_43", K_WORLD_43},
	{"WORLD_44", K_WORLD_44},
	{"WORLD_45", K_WORLD_45},
	{"WORLD_46", K_WORLD_46},
	{"WORLD_47", K_WORLD_47},
	{"WORLD_48", K_WORLD_48},
	{"WORLD_49", K_WORLD_49},
	{"WORLD_50", K_WORLD_50},
	{"WORLD_51", K_WORLD_51},
	{"WORLD_52", K_WORLD_52},
	{"WORLD_53", K_WORLD_53},
	{"WORLD_54", K_WORLD_54},
	{"WORLD_55", K_WORLD_55},
	{"WORLD_56", K_WORLD_56},
	{"WORLD_57", K_WORLD_57},
	{"WORLD_58", K_WORLD_58},
	{"WORLD_59", K_WORLD_59},
	{"WORLD_60", K_WORLD_60},
	{"WORLD_61", K_WORLD_61},
	{"WORLD_62", K_WORLD_62},
	{"WORLD_63", K_WORLD_63},
	{"WORLD_64", K_WORLD_64},
	{"WORLD_65", K_WORLD_65},
	{"WORLD_66", K_WORLD_66},
	{"WORLD_67", K_WORLD_67},
	{"WORLD_68", K_WORLD_68},
	{"WORLD_69", K_WORLD_69},
	{"WORLD_70", K_WORLD_70},
	{"WORLD_71", K_WORLD_71},
	{"WORLD_72", K_WORLD_72},
	{"WORLD_73", K_WORLD_73},
	{"WORLD_74", K_WORLD_74},
	{"WORLD_75", K_WORLD_75},
	{"WORLD_76", K_WORLD_76},
	{"WORLD_77", K_WORLD_77},
	{"WORLD_78", K_WORLD_78},
	{"WORLD_79", K_WORLD_79},
	{"WORLD_80", K_WORLD_80},
	{"WORLD_81", K_WORLD_81},
	{"WORLD_82", K_WORLD_82},
	{"WORLD_83", K_WORLD_83},
	{"WORLD_84", K_WORLD_84},
	{"WORLD_85", K_WORLD_85},
	{"WORLD_86", K_WORLD_86},
	{"WORLD_87", K_WORLD_87},
	{"WORLD_88", K_WORLD_88},
	{"WORLD_89", K_WORLD_89},
	{"WORLD_90", K_WORLD_90},
	{"WORLD_91", K_WORLD_91},
	{"WORLD_92", K_WORLD_92},
	{"WORLD_93", K_WORLD_93},
	{"WORLD_94", K_WORLD_94},
	{"WORLD_95", K_WORLD_95},
#endif

	{"WINDOWS", A_UNDEFINED_13},
	{"COMPOSE", A_UNDEFINED_14},
	{"MODE", A_UNDEFINED_15},
	{"HELP", A_UNDEFINED_16},
	{"PRINT", A_PRINTSCREEN},
	{"SYSREQ", A_UNDEFINED_17},
	{"SCROLLOCK", A_SCROLLLOCK },
	{"BREAK", A_UNDEFINED_18},
	{"MENU", A_UNDEFINED_19},
	{"POWER", A_UNDEFINED_20},
	{"EURO", A_EURO},
	{"UNDO", A_UNDEFINED_21},

	{NULL,0}
};

/*
=============================================================================

EDIT FIELDS

=============================================================================
*/

/*
===================
Field_Draw

Handles horizontal scrolling and cursor blinking
x, y, and width are in pixels
===================
*/
void Field_VariableSizeDraw( field_t *edit, int x, int y, int width, int size, qboolean showCursor,
		qboolean noColorEscape ) {
	int		len;
	int		drawLen;
	int		prestep;
	int		cursorChar;
	char	str[MAX_STRING_CHARS];
	int		i;

	drawLen = edit->widthInChars - 1; // - 1 so there is always a space for the cursor
	len = strlen( edit->buffer );

	// guarantee that cursor will be visible
	if ( len <= drawLen ) {
		prestep = 0;
	} else {
		if ( edit->scroll + drawLen > len ) {
			edit->scroll = len - drawLen;
			if ( edit->scroll < 0 ) {
				edit->scroll = 0;
			}
		}
		prestep = edit->scroll;
	}

	if ( prestep + drawLen > len ) {
		drawLen = len - prestep;
	}

	// extract <drawLen> characters from the field at <prestep>
	if ( drawLen >= MAX_STRING_CHARS ) {
		Com_Error( ERR_DROP, "drawLen >= MAX_STRING_CHARS" );
	}

	Com_Memcpy( str, edit->buffer + prestep, drawLen );
	str[ drawLen ] = 0;

	// draw it
	if ( size == SMALLCHAR_WIDTH ) {
		float	color[4];

		color[0] = color[1] = color[2] = color[3] = 1.0;
		SCR_DrawSmallStringExt( x, y, str, color, qfalse, noColorEscape );
	} else {
		// draw big string with drop shadow
		SCR_DrawBigString( x, y, str, 1.0, noColorEscape );
	}

	// draw the cursor
	if ( showCursor ) {
		if ( (int)( cls.realtime >> 8 ) & 1 ) {
			return;		// off blink
		}

		if ( key_overstrikeMode ) {
			cursorChar = 11;
		} else {
			cursorChar = 10;
		}

		i = drawLen - strlen( str );

		if ( size == SMALLCHAR_WIDTH ) {
			SCR_DrawSmallChar( x + ( edit->cursor - prestep - i ) * size, y, cursorChar );
		} else {
			str[0] = cursorChar;
			str[1] = 0;
			SCR_DrawBigString( x + ( edit->cursor - prestep - i ) * size, y, str, 1.0, qfalse );
		}
	}
}

void Field_Draw( field_t *edit, int x, int y, int width, qboolean showCursor, qboolean noColorEscape ) 
{
	Field_VariableSizeDraw( edit, x, y, width, SMALLCHAR_WIDTH, showCursor, noColorEscape );
}

void Field_BigDraw( field_t *edit, int x, int y, int width, qboolean showCursor, qboolean noColorEscape ) 
{
	Field_VariableSizeDraw( edit, x, y, width, BIGCHAR_WIDTH, showCursor, noColorEscape );
}

/*
================
Field_Paste
================
*/
void Field_Paste( field_t *edit ) {
	char	*cbd;
	int		pasteLen, i;

	cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		return;
	}

	// send as if typed, so insert / overstrike works properly
	pasteLen = strlen( cbd );
	for ( i = 0 ; i < pasteLen ; i++ ) {
		Field_CharEvent( edit, cbd[i] );
	}

	Z_Free( cbd );
}

/*
=================
Field_KeyDownEvent

Performs the basic line editing functions for the console,
in-game talk, and menu fields

Key events are used for non-printable characters, others are gotten from char events.
=================
*/
void Field_KeyDownEvent( field_t *edit, int key ) {
	int		len;

	// shift-insert is paste
	if ( ( ( key == A_INSERT ) || ( key == A_KP_0 ) ) && keys[A_SHIFT].down ) {
		Field_Paste( edit );
		return;
	}

	key = tolower( key );
	len = strlen( edit->buffer );

	switch ( key ) {
		case A_DELETE:
			if ( edit->cursor < len ) {
				memmove( edit->buffer + edit->cursor, 
					edit->buffer + edit->cursor + 1, len - edit->cursor );
			}
			break;

		case A_CURSOR_RIGHT:
			if ( edit->cursor < len ) {
				edit->cursor++;
			}
			break;

		case A_CURSOR_LEFT:
			if ( edit->cursor > 0 ) {
				edit->cursor--;
			}
			break;

		case A_HOME:
			edit->cursor = 0;
			break;

		case A_END:
			edit->cursor = len;
			break;

		case A_INSERT:
			key_overstrikeMode = !key_overstrikeMode;
			break;

		default:
			break;
	}

	// Change scroll if cursor is no longer visible
	if ( edit->cursor < edit->scroll ) {
		edit->scroll = edit->cursor;
	} else if ( edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= len ) {
		edit->scroll = edit->cursor - edit->widthInChars + 1;
	}
}

/*
==================
Field_CharEvent
==================
*/
void Field_CharEvent( field_t *edit, int ch ) {
	int		len;

	// Someone is trying to enter in an alt code
	if( keys[A_ALT].down && isdigit(ch) ) {
		return;
	}

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		Field_Paste( edit );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		Field_Clear( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			memmove( edit->buffer + edit->cursor - 1, 
				edit->buffer + edit->cursor, len + 1 - edit->cursor );
			edit->cursor--;
			if ( edit->cursor < edit->scroll )
			{
				edit->scroll--;
			}
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		edit->cursor = len;
		edit->scroll = edit->cursor - edit->widthInChars;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < 32 ) {
		return;
	}

	if ( key_overstrikeMode ) {	
		// - 2 to leave room for the leading slash and trailing \0
		if ( edit->cursor == MAX_EDIT_LINE - 2 )
			return;
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
	} else {	// insert mode
		// - 2 to leave room for the leading slash and trailing \0
		if ( len == MAX_EDIT_LINE - 2 ) {
			return; // all full
		}
		memmove( edit->buffer + edit->cursor + 1, 
			edit->buffer + edit->cursor, len + 1 - edit->cursor );
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
	}

	if ( edit->cursor >= edit->widthInChars ) {
		edit->scroll++;
	}

	if ( edit->cursor == len + 1) {
		edit->buffer[edit->cursor] = 0;
	}
}

/*
=============================================================================

CONSOLE LINE EDITING

==============================================================================
*/

/*
====================
Console_Key

Handles history and console scrollback
====================
*/
extern cvar_t *con_allowChat;
void Console_Key (int key) {
	// ctrl-L clears screen
	if ( key == 'l' && keys[A_CTRL].down ) {
		Cbuf_AddText ("clear\n");
		return;
	}

	// enter finishes the line
	if ( key == A_ENTER || key == A_KP_ENTER ) {
		if( !con_allowChat->integer ) {
			/* JKA: If you want to make qui gon work, make a new version of Field_CompleteCommand that doesn't care about arguments beyond argv[0] */
			//Field_AutoComplete(&g_consoleField);

			Com_Printf( "]%s\n", g_consoleField.buffer );

			if( !g_consoleField.buffer[0] ) {
				return; // empty lines just scroll the console without adding to history
			}
			// leading slash is an explicit command
			else if( g_consoleField.buffer[0] == '\\' || g_consoleField.buffer[0] == '/' ) {
				Cbuf_AddText( g_consoleField.buffer + 1 ); // valid command
				Cbuf_AddText( "\n" );
			}
			else {
				Cbuf_AddText( g_consoleField.buffer ); // valid command
				Cbuf_AddText( "\n" );
			}
		}
		else {
			// if not in the game explicitly prepend a slash if needed
			if ( clc.state != CA_ACTIVE &&
					g_consoleField.buffer[0] &&
					g_consoleField.buffer[0] != '\\' &&
					g_consoleField.buffer[0] != '/' ) {
				char	temp[MAX_EDIT_LINE-1];

				Q_strncpyz( temp, g_consoleField.buffer, sizeof( temp ) );
				Com_sprintf( g_consoleField.buffer, sizeof( g_consoleField.buffer ), "\\%s", temp );
				g_consoleField.cursor++;
			}

			Com_Printf ( "]%s\n", g_consoleField.buffer );

			// leading slash is an explicit command
			if ( g_consoleField.buffer[0] == '\\' || g_consoleField.buffer[0] == '/' ) {
				Cbuf_AddText( g_consoleField.buffer+1 );	// valid command
				Cbuf_AddText ("\n");
			} else {
				// other text will be chat messages
				if ( !g_consoleField.buffer[0] ) {
					return;	// empty lines just scroll the console without adding to history
				} else {
					Cbuf_AddText ("cmd say ");
					Cbuf_AddText( g_consoleField.buffer );
					Cbuf_AddText ("\n");
				}
			}
		}

		// copy line to history buffer
		historyEditLines[nextHistoryLine % COMMAND_HISTORY] = g_consoleField;
		nextHistoryLine++;
		historyLine = nextHistoryLine;

		Field_Clear( &g_consoleField );

		g_consoleField.widthInChars = g_console_field_width;

		CL_SaveConsoleHistory( );

		if ( clc.state == CA_DISCONNECTED ) {
			SCR_UpdateScreen ();	// force an update, because the command
		}							// may take some time
		return;
	}

	// command completion

	if (key == A_TAB) {
		Field_AutoComplete(&g_consoleField);
		return;
	}

	// command history (ctrl-p ctrl-n for unix style)

	if ( (key == A_MWHEELUP && keys[A_SHIFT].down) || ( key == A_CURSOR_UP ) || ( key == A_KP_8 ) ||
		 ( ( tolower(key) == 'p' ) && keys[A_CTRL].down ) ) {
		if ( nextHistoryLine - historyLine < COMMAND_HISTORY 
			&& historyLine > 0 ) {
			historyLine--;
		}
		g_consoleField = historyEditLines[ historyLine % COMMAND_HISTORY ];
		return;
	}

	if ( (key == A_MWHEELDOWN && keys[A_SHIFT].down) || ( key == A_CURSOR_DOWN ) || ( key == A_KP_2 ) ||
		 ( ( tolower(key) == 'n' ) && keys[A_CTRL].down ) ) {
		historyLine++;
		if (historyLine >= nextHistoryLine) {
			historyLine = nextHistoryLine;
			Field_Clear( &g_consoleField );
			g_consoleField.widthInChars = g_console_field_width;
			return;
		}
		g_consoleField = historyEditLines[ historyLine % COMMAND_HISTORY ];
		return;
	}

	// console scrolling
	if ( key == A_PAGE_UP ) {
		Con_PageUp();
		return;
	}

	if ( key == A_PAGE_DOWN ) {
		Con_PageDown();
		return;
	}

	if ( key == A_MWHEELUP ) {	//----(SA)	added some mousewheel functionality to the console
		Con_PageUp();
		if(keys[A_CTRL].down) {	// hold <ctrl> to accelerate scrolling
			Con_PageUp();
			Con_PageUp();
		}
		return;
	}

	if ( key == A_MWHEELDOWN ) {	//----(SA)	added some mousewheel functionality to the console
		Con_PageDown();
		if(keys[A_CTRL].down) {	// hold <ctrl> to accelerate scrolling
			Con_PageDown();
			Con_PageDown();
		}
		return;
	}

	// ctrl-home = top of console
	if ( key == A_HOME && keys[A_CTRL].down ) {
		Con_Top();
		return;
	}

	// ctrl-end = bottom of console
	if ( key == A_END && keys[A_CTRL].down ) {
		Con_Bottom();
		return;
	}

	// pass to the normal editline routine
	Field_KeyDownEvent( &g_consoleField, key );
}

//============================================================================

/*
================
Message_Key

In game talk message
================
*/
void Message_Key( int key ) {
	char	buffer[MAX_STRING_CHARS];

	if (key == A_ESCAPE) {
		Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_MESSAGE );
		Field_Clear( &chatField );
		return;
	}

	if ( key == A_ENTER || key == A_KP_ENTER )
	{
		if ( chatField.buffer[0] && clc.state == CA_ACTIVE ) {
			if ( chat_playerNum != -1 )
				Com_sprintf( buffer, sizeof( buffer ), "tell %i \"%s\"\n", chat_playerNum, chatField.buffer );
			else if ( chat_team )
				Com_sprintf( buffer, sizeof( buffer ), "say_team \"%s\"\n", chatField.buffer );
			else
				Com_sprintf( buffer, sizeof( buffer ), "say \"%s\"\n", chatField.buffer );

			CL_AddReliableCommand(buffer, qfalse);
		}
		Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_MESSAGE );
		Field_Clear( &chatField );
		return;
	}

	Field_KeyDownEvent( &chatField, key );
}

//============================================================================

qboolean Key_GetOverstrikeMode( void ) {
	return key_overstrikeMode;
}

void Key_SetOverstrikeMode( qboolean state ) {
	key_overstrikeMode = state;
}

/*
===================
Key_IsDown
===================
*/
qboolean Key_IsDown( int keynum ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return qfalse;
	}

	return keys[keynum].down;
}

/*
===================
Key_StringToKeynum

Returns a key number to be used to index keys[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.

0x11 will be interpreted as raw hex, which will allow new controlers

to be configured even if they don't have defined names.
===================
*/
int Key_StringToKeynum( char *str ) {
	keyname_t	*kn;
	
	if ( !str || !str[0] ) {
		return -1;
	}
	if ( !str[1] ) {
		// Always lowercase
		Q_strlwr( str );
		return str[0];
	}

	// check for hex code
	if ( strlen( str ) == 4 ) {
		int n = Com_HexStrToInt( str );

		if ( n >= 0 ) {
			return n;
		}
	}

	// scan for a text match
	for ( kn=keynames ; kn->name ; kn++ ) {
		if ( !Q_stricmp( str,kn->name ) )
			return kn->keynum;
	}

	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, a K_* name, or a 0x11 hex string) for the
given keynum.
===================
*/
char *Key_KeynumToString( int keynum ) {
	keyname_t	*kn;	
	static	char	tinystr[5];
	int			i, j;
	char		*stringed;

	if ( keynum == -1 ) {
		return "<KEY NOT FOUND>";
	}

	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return "<OUT OF RANGE>";
	}

	// check for printable ascii (don't use quote)
	if ( keynum > 32 && keynum < 127 && keynum != '"' && keynum != ';' ) {
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	// check for a key string
	for ( kn=keynames ; kn->name ; kn++ ) {
		if (keynum == kn->keynum) {
			stringed = SE_GetString(va("KEYNAMES_KEYNAME_%s",kn->name));
			if( stringed && *stringed )
				return stringed;
			else
				return kn->name;
		}
	}

	// make a hex string
	i = keynum >> 4;
	j = keynum & 15;

	tinystr[0] = '0';
	tinystr[1] = 'x';
	tinystr[2] = i > 9 ? i - 10 + 'a' : i + '0';
	tinystr[3] = j > 9 ? j - 10 + 'a' : j + '0';
	tinystr[4] = 0;

	return tinystr;
}

/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding( int keynum, const char *binding ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return;
	}

	// free old bindings
	if ( keys[ keynum ].binding ) {
		Z_Free( keys[ keynum ].binding );
	}
		
	// allocate memory for new binding
	keys[keynum].binding = CopyString( binding );

	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	cvar_modifiedFlags |= CVAR_ARCHIVE;
}

/*
===================
Key_GetBinding
===================
*/
char *Key_GetBinding( int keynum ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return "";
	}

	return keys[ keynum ].binding;
}

/* 
===================
Key_GetKey
===================
*/
int Key_GetKey( const char *binding ) {
	int i;

	if (binding) {
		for (i=0 ; i < MAX_KEYS ; i++) {
			if (keys[i].binding && Q_stricmp(binding, keys[i].binding) == 0) {
				return i;
			}
		}
	}
	return -1;
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f( void ) {
	int		b;

	if (Cmd_Argc() != 2) {
		Com_Printf ("unbind <key> : remove commands from a key\n");
		return;
	}
	
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1) {
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, "");
}

/*
===================
Key_Unbindall_f
===================
*/
void Key_Unbindall_f( void ) {
	int		i;
	
	for (i=0 ; i < MAX_KEYS; i++)
		if (keys[i].binding)
			Key_SetBinding (i, "");
}

/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f( void ) {
	int			i, c, b;
	char		cmd[MAX_STRING_CHARS];
	
	c = Cmd_Argc();

	if (c < 2) {
		Com_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1) {
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2) {
		if (keys[b].binding)
			Com_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keys[b].binding );
		else
			Com_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}
	
	// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++) {
		strcat (cmd, Cmd_Argv(i));
		if (i != (c-1))
			strcat (cmd, " ");
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings( fileHandle_t f ) {
	int		i;

	FS_Printf (f, "unbindall\n" );

	for (i=0 ; i<MAX_KEYS ; i++) {
		if (keys[i].binding && keys[i].binding[0] ) {
			FS_Printf (f, "bind %s \"%s\"\n", Key_KeynumToString(i), keys[i].binding);

		}

	}
}


/*
============
Key_Bindlist_f

============
*/
void Key_Bindlist_f( void ) {
	int		i;

	for ( i = 0 ; i < MAX_KEYS ; i++ ) {
		if ( keys[i].binding && keys[i].binding[0] ) {
			Com_Printf( "%s \"%s\"\n", Key_KeynumToString(i), keys[i].binding );
		}
	}
}

/*
============
Key_KeynameCompletion
============
*/
void Key_KeynameCompletion( void(*callback)(const char *s) ) {
	int		i;

	for( i = 0; keynames[ i ].name != NULL; i++ )
		callback( keynames[ i ].name );
}

/*
====================
Key_CompleteUnbind
====================
*/
static void Key_CompleteUnbind( char *args, int argNum )
{
	if( argNum == 2 )
	{
		// Skip "unbind "
		char *p = Com_SkipTokens( args, 1, " " );

		if( p > args )
			Field_CompleteKeyname( );
	}
}

/*
====================
Key_CompleteBind
====================
*/
static void Key_CompleteBind( char *args, int argNum )
{
	char *p;

	if( argNum == 2 )
	{
		// Skip "bind "
		p = Com_SkipTokens( args, 1, " " );

		if( p > args )
			Field_CompleteKeyname( );
	}
	else if( argNum >= 3 )
	{
		// Skip "bind <key> "
		p = Com_SkipTokens( args, 2, " " );

		if( p > args )
			Field_CompleteCommand( p, qtrue, qtrue );
	}
}

/*
===================
CL_InitKeyCommands
===================
*/
void CL_InitKeyCommands( void ) {
	// register our functions
	Cmd_AddCommand ("bind",Key_Bind_f);
	Cmd_SetCommandCompletionFunc( "bind", Key_CompleteBind );
	Cmd_AddCommand ("unbind",Key_Unbind_f);
	Cmd_SetCommandCompletionFunc( "unbind", Key_CompleteUnbind );
	Cmd_AddCommand ("unbindall",Key_Unbindall_f);
	Cmd_AddCommand ("bindlist",Key_Bindlist_f);
}

/*
===================
CL_ParseBinding

Execute the commands in the bind string
===================
*/
void CL_ParseBinding( int key, qboolean down, unsigned time )
{
	char buf[ MAX_STRING_CHARS ], *p = buf, *end;

	if( !keys[key].binding || !keys[key].binding[0] )
		return;
	Q_strncpyz( buf, keys[key].binding, sizeof( buf ) );

	while( 1 )
	{
		while( isspace( *p ) )
			p++;
		end = strchr( p, ';' );
		if( end )
			*end = '\0';
		if( *p == '+' )
		{
			// button commands add keynum and time as parameters
			// so that multiple sources can be discriminated and
			// subframe corrected
			char cmd[1024];
			Com_sprintf( cmd, sizeof( cmd ), "%c%s %d %d\n",
				( down ) ? '+' : '-', p + 1, key, time );
			Cbuf_AddText( cmd );
		}
		else if( down )
		{
			// normal commands only execute on key press
			Cbuf_AddText( p );
			Cbuf_AddText( "\n" );
		}
		if( !end )
			break;
		p = end + 1;
	}
}

/*
===================
CL_KeyDownEvent

Called by CL_KeyEvent to handle a keypress
===================
*/
void CL_KeyDownEvent( int key, unsigned time )
{
	keys[key].down = qtrue;
	keys[key].repeats++;
	if( keys[key].repeats == 1 && key != A_SCROLLLOCK && key != A_NUMLOCK && key != A_CAPSLOCK )
		anykeydown++;

	if( keys[A_ALT].down && key == A_ENTER )
	{
		Cvar_SetValue( "r_fullscreen",
			!Cvar_VariableIntegerValue( "r_fullscreen" ) );
		return;
	}

	// console key is hardcoded, so the user can never unbind it
	if( key == A_CONSOLE || ( keys[A_SHIFT].down && key == A_ESCAPE ) )
	{
		Con_ToggleConsole_f ();
		Key_ClearStates ();
		return;
	}

	// keys can still be used for bound actions
	if ( ( key < 128 || key == A_MOUSE1 ) &&
		( clc.demoplaying || clc.state == CA_CINEMATIC ) && Key_GetCatcher( ) == 0 ) {

		if (Cvar_VariableValue ("com_cameraMode") == 0) {
			Cvar_Set ("nextdemo","");
			key = A_ESCAPE;
		}
	}

	// escape is always handled special
	if ( key == A_ESCAPE ) {
		if ( Key_GetCatcher( ) & KEYCATCH_MESSAGE ) {
			// clear message mode
			Message_Key( key );
			return;
		}

		// escape always gets out of CGAME stuff
		if (Key_GetCatcher( ) & KEYCATCH_CGAME) {
			Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_CGAME );
			VM_Call (cgvm, CG_EVENT_HANDLING, CGAME_EVENT_NONE);
			return;
		}

		if ( !( Key_GetCatcher( ) & KEYCATCH_UI ) ) {
			if ( clc.state == CA_ACTIVE && !clc.demoplaying ) {
				VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_INGAME );
			}
			else if ( clc.state != CA_DISCONNECTED ) {
				CL_Disconnect_f();
				S_StopAllSounds();
				VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
			}
			return;
		}

		VM_Call( uivm, UI_KEY_EVENT, key, qtrue );
		return;
	}

	// distribute the key down event to the apropriate handler
	if ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) {
		Console_Key( key );
	} else if ( Key_GetCatcher( ) & KEYCATCH_UI ) {
		if ( uivm ) {
			VM_Call( uivm, UI_KEY_EVENT, key, qtrue );
		} 
	} else if ( Key_GetCatcher( ) & KEYCATCH_CGAME ) {
		if ( cgvm ) {
			VM_Call( cgvm, CG_KEY_EVENT, key, qtrue );
		} 
	} else if ( Key_GetCatcher( ) & KEYCATCH_MESSAGE ) {
		Message_Key( key );
	} else if ( clc.state == CA_DISCONNECTED ) {
		Console_Key( key );
	} else {
		// send the bound action
		CL_ParseBinding( key, qtrue, time );
	}
	return;
}

/*
===================
CL_KeyUpEvent

Called by CL_KeyEvent to handle a keyrelease
===================
*/
void CL_KeyUpEvent( int key, unsigned time )
{
	keys[key].repeats = 0;
	keys[key].down = qfalse;
	if (key != A_SCROLLLOCK && key != A_NUMLOCK && key != A_CAPSLOCK)
		anykeydown--;

	if (anykeydown < 0) {
		anykeydown = 0;
	}

	// don't process key-up events for the console key
	if ( key == A_CONSOLE || ( key == A_ESCAPE && keys[A_SHIFT].down ) )
		return;

	//
	// key up events only perform actions if the game key binding is
	// a button command (leading + sign).  These will be processed even in
	// console mode and menu mode, to keep the character from continuing
	// an action started before a mode switch.
	//
	if( clc.state != CA_DISCONNECTED )
		CL_ParseBinding( key, qfalse, time );

	if ( Key_GetCatcher( ) & KEYCATCH_UI && uivm ) {
		VM_Call( uivm, UI_KEY_EVENT, key, qfalse );
	} else if ( Key_GetCatcher( ) & KEYCATCH_CGAME && cgvm ) {
		VM_Call( cgvm, CG_KEY_EVENT, key, qfalse );
	}
}

/*
===================
CL_KeyEvent

Called by the system for both key up and key down events
===================
*/
void CL_KeyEvent (int key, qboolean down, unsigned time) {
	if( down )
		CL_KeyDownEvent( key, time );
	else
		CL_KeyUpEvent( key, time );
}

/*
===================
CL_CharEvent

Normal keyboard characters, already shifted / capslocked / etc
===================
*/
void CL_CharEvent( int key ) {
	// delete is not a printable character and is
	// otherwise handled by Field_KeyDownEvent
	if ( key == 127 ) {
		return;
	}

	// distribute the key down event to the apropriate handler
	if ( Key_GetCatcher( ) & KEYCATCH_CONSOLE )
	{
		Field_CharEvent( &g_consoleField, key );
	}
	else if ( Key_GetCatcher( ) & KEYCATCH_UI )
	{
		VM_Call( uivm, UI_KEY_EVENT, key | K_CHAR_FLAG, qtrue );
	}
	else if ( Key_GetCatcher( ) & KEYCATCH_MESSAGE ) 
	{
		Field_CharEvent( &chatField, key );
	}
	else if ( clc.state == CA_DISCONNECTED )
	{
		Field_CharEvent( &g_consoleField, key );
	}
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	anykeydown = 0;

	for ( i=0 ; i < MAX_KEYS ; i++ ) {
		if (i == A_SCROLLLOCK || i == A_NUMLOCK || i == A_CAPSLOCK)
			continue;

		if ( keys[i].down ) {
			CL_KeyEvent( i, qfalse, 0 );

		}
		keys[i].down = 0;
		keys[i].repeats = 0;
	}
}

static int keyCatchers = 0;

/*
====================
Key_GetCatcher
====================
*/
int Key_GetCatcher( void ) {
	return keyCatchers;
}

/*
====================
Key_SetCatcher
====================
*/
void Key_SetCatcher( int catcher ) {
	// If the catcher state is changing, clear all key states
	if( catcher != keyCatchers )
		Key_ClearStates( );

	keyCatchers = catcher;
}

// This must not exceed MAX_CMD_LINE
#define			MAX_CONSOLE_SAVE_BUFFER	1024
#define			CONSOLE_HISTORY_FILE    "jamphistory"
static char	consoleSaveBuffer[ MAX_CONSOLE_SAVE_BUFFER ];
static int	consoleSaveBufferSize = 0;

/*
================
CL_LoadConsoleHistory

Load the console history from cl_consoleHistory
================
*/
void CL_LoadConsoleHistory( void )
{
	char					*token, *text_p;
	int						i, numChars, numLines = 0;
	fileHandle_t	f;

	consoleSaveBufferSize = FS_FOpenFileRead( CONSOLE_HISTORY_FILE, &f, qfalse );
	if( !f )
	{
		Com_Printf( "Couldn't read %s.\n", CONSOLE_HISTORY_FILE );
		return;
	}

	if( consoleSaveBufferSize <= MAX_CONSOLE_SAVE_BUFFER &&
			FS_Read( consoleSaveBuffer, consoleSaveBufferSize, f ) == consoleSaveBufferSize )
	{
		text_p = consoleSaveBuffer;

		for( i = COMMAND_HISTORY - 1; i >= 0; i-- )
		{
			if( !*( token = COM_Parse( &text_p ) ) )
				break;

			historyEditLines[ i ].cursor = atoi( token );

			if( !*( token = COM_Parse( &text_p ) ) )
				break;

			historyEditLines[ i ].scroll = atoi( token );

			if( !*( token = COM_Parse( &text_p ) ) )
				break;

			numChars = atoi( token );
			text_p++;
			if( numChars > ( strlen( consoleSaveBuffer ) -	( text_p - consoleSaveBuffer ) ) )
			{
				Com_DPrintf( S_COLOR_YELLOW "WARNING: probable corrupt history\n" );
				break;
			}
			Com_Memcpy( historyEditLines[ i ].buffer,
					text_p, numChars );
			historyEditLines[ i ].buffer[ numChars ] = '\0';
			text_p += numChars;

			numLines++;
		}

		memmove( &historyEditLines[ 0 ], &historyEditLines[ i + 1 ],
				numLines * sizeof( field_t ) );
		for( i = numLines; i < COMMAND_HISTORY; i++ )
			Field_Clear( &historyEditLines[ i ] );

		historyLine = nextHistoryLine = numLines;
	}
	else
		Com_Printf( "Couldn't read %s.\n", CONSOLE_HISTORY_FILE );

	FS_FCloseFile( f );
}

/*
================
CL_SaveConsoleHistory

Save the console history into the cvar cl_consoleHistory
so that it persists across invocations of q3
================
*/
void CL_SaveConsoleHistory( void )
{
	int						i;
	int						lineLength, saveBufferLength, additionalLength;
	fileHandle_t	f;

	consoleSaveBuffer[ 0 ] = '\0';

	i = ( nextHistoryLine - 1 ) % COMMAND_HISTORY;
	do
	{
		if( historyEditLines[ i ].buffer[ 0 ] )
		{
			lineLength = strlen( historyEditLines[ i ].buffer );
			saveBufferLength = strlen( consoleSaveBuffer );

			//ICK
			additionalLength = lineLength + strlen( "999 999 999  " );

			if( saveBufferLength + additionalLength < MAX_CONSOLE_SAVE_BUFFER )
			{
				Q_strcat( consoleSaveBuffer, MAX_CONSOLE_SAVE_BUFFER,
						va( "%d %d %d %s ",
						historyEditLines[ i ].cursor,
						historyEditLines[ i ].scroll,
						lineLength,
						historyEditLines[ i ].buffer ) );
			}
			else
				break;
		}
		i = ( i - 1 + COMMAND_HISTORY ) % COMMAND_HISTORY;
	}
	while( i != ( nextHistoryLine - 1 ) % COMMAND_HISTORY );

	consoleSaveBufferSize = strlen( consoleSaveBuffer );

	f = FS_FOpenFileWrite( CONSOLE_HISTORY_FILE );
	if( !f )
	{
		Com_Printf( "Couldn't write %s.\n", CONSOLE_HISTORY_FILE );
		return;
	}

	if( FS_Write( consoleSaveBuffer, consoleSaveBufferSize, f ) < consoleSaveBufferSize )
		Com_Printf( "Couldn't write %s.\n", CONSOLE_HISTORY_FILE );

	FS_FCloseFile( f );
}
