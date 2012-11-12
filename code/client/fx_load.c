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
#include "fx_local.h"

#define FILE_HASH_SIZE		1024
static	fxFile_t*		hashTable[FILE_HASH_SIZE];

fxFile_t parsedfile;
int fx_numFiles;
int fx_filesCapacity;
fxFile_t *fx_fileHandles;

void FX_InitFileMemory( int initialSize )
{
	Com_Memset(&parsedfile, 0, sizeof(parsedfile));
	Com_Memset(hashTable, 0, sizeof(hashTable));
	fx_fileHandles = (fxFile_t *)malloc (sizeof (fxFile_t) * initialSize);
	fx_numFiles = 0;
	fx_filesCapacity = initialSize;
}

static fxFile_t *FX_AllocFile(void)
{
    fxFile_t *file = NULL;
	if ( fx_numFiles >= fx_filesCapacity )
	{
		int newSize = (int)(fx_filesCapacity * 1.5f);
		fxFile_t *newFileHandles = (fxFile_t *)realloc (fx_fileHandles, sizeof (fxFile_t) * newSize);
		if ( newFileHandles == NULL )
		{
			Com_Error (ERR_FATAL, "Failed to allocate memory for additional FX files.\n");
			return NULL;
		}

		fx_fileHandles = newFileHandles;
		fx_filesCapacity = newSize;
	}

	file = &fx_fileHandles[fx_numFiles];
	fx_numFiles++;

	return file;
}

void FX_CleanupFileMemory(void)
{
	free (fx_fileHandles);
	fx_fileHandles = NULL;
	fx_numFiles = 0;
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

fxFile_t *FX_FindEffect( const char *name )
{
	int			hash;
	fxFile_t	*fx;

	if ( name[0] == 0 ) {
		return NULL;
	}

	hash = generateHashValue(name, FILE_HASH_SIZE);

	//
	// see if the fx is already loaded
	//
	for (fx = hashTable[hash]; fx; fx = fx->next) {
		if ( !Q_stricmp( fx->name, name ) ) {
			// match found
			return fx;
		}
	}

	return NULL;
}

fxFile_t *FX_GetByHandle( fxHandle_t hFX ) {
	if ( hFX < 0 ) {
		Com_Printf( S_COLOR_YELLOW, "FX_GetByHandle: out of range hFX '%d'\n", hFX );
		return NULL;
	}
	if ( hFX >= fx_numFiles ) {
		Com_Printf( S_COLOR_YELLOW, "FX_GetByHandle: out of range hFX '%d'\n", hFX );
		return NULL;
	}
	return &fx_fileHandles[hFX];
}

fxHandle_t FX_RegisterEffect( const char *name )
{
	char fileName[MAX_QPATH];
	char *p;
	fxFile_t *fx;
	int hash;

	if( !name ) {
		Com_Error( ERR_FATAL, "RE_RegisterEffect: NULL" );
	}

	if( !name[0] ) {
		return 0;
	}

	if( name[0] == '\n' || name[0] == '\r' || name[0] == '\t' ) {
		Com_Error( ERR_FATAL, "RE_RegisterEffect: called with empty name" );
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Error( ERR_FATAL, "Effect name %s exceeds MAX_QPATH", name );
	}

	if ( Q_stricmpn( name, EFFECTS_FOLDER, 8 ) != 0 ) {
		Com_sprintf( fileName, sizeof( fileName ), EFFECTS_FOLDER "%s", name );
	}
	else {
		Q_strncpyz( fileName, name, sizeof( fileName ) );
	}

	// Hacks to fix env//waterfall_mist.efx
	p = strstr( fileName, "//" );
	while( p != NULL ) {
		strcpy (p, p + 1);
		p = strstr(p, "//");
	}

	COM_DefaultExtension(fileName, sizeof(fileName), ".efx");

	fx = FX_FindEffect( fileName );

	if( fx )
		return fx - fx_fileHandles;

	// No? Parse the effect file.
	if(!FX_ParseEffectFile(fileName)) {
		Com_Printf( S_COLOR_YELLOW "WARNING: Failed to parse effect file (%s)\n", fileName);
		return 0;
	}

	fx = FX_AllocFile();
	FX_FilePrecache();
	*fx = parsedfile;

	hash = generateHashValue(fileName, FILE_HASH_SIZE);
	fx->next = hashTable[hash];
	hashTable[hash] = fx;

	return fx - fx_fileHandles;
}

//
//		Functions that parse a specific type of field in the .efx file
//

qboolean FX_Parse_Field_FXInt(fxInt_t *field, char **buf)
{
	char *token = COM_ParseExt(buf, qfalse);
	if(!token || !token[0] )
	{
		return qfalse;
	}
	(*field)[0] = atoi(token);
	(*field)[1] = atoi(token);
	token = COM_ParseExt(buf, qfalse);
	if(!token || !token[0])
	{
		return qtrue;
	}
	(*field)[1] = atoi(token);
	return qtrue;
}

qboolean FX_Parse_Field_FXFloat(fxFloat_t *field, char **buf)
{
	char *token = COM_ParseExt(buf, qfalse);
	if(!token || !token[0])
	{
		return qfalse;
	}
	(*field)[0] = atof(token);
	(*field)[1] = atof(token);
	token = COM_ParseExt(buf, qfalse);
	if(!token || !token[0])
	{
		return qtrue;
	}
	(*field)[1] = atof(token);
	return qtrue;
}

qboolean FX_Parse_Field_FXVec3(fxVec3_t *field, char **buf)
{
	int i;
	char *token = COM_ParseExt(buf, qfalse);
	if(!token || !token[0])
	{
		return qfalse;
	}
	(*field)[0][0] = atof(token);
	for(i = 1; i < 3; i++)
	{
		token = COM_ParseExt(buf, qfalse);
		if(!token || !token[0])
		{
			return qfalse;
		}
		(*field)[0][i] = atof(token);
	}
	VectorCopy((*field)[0], (*field)[1]);
	for(i = 0; i < 3; i++)
	{
		token = COM_ParseExt(buf, qfalse);
		if(!token || !token[0])
		{
			return qtrue;
		}
		(*field)[1][i] = atof(token);
	}
	return qtrue;
}

// The "first" functions will ignore the first check
qboolean FX_Parse_Field_Flags(int *field, char **buf)
{
	char *token = COM_ParseExt(buf, qfalse);
	if(!token || !token[0])
	{
		return qfalse;
	}

	do
	{
		if(!Q_stricmp(token, "depthHack"))
		{
			*field |= (1 << FXFLAG_DEPTHHACK);
		}
		else if(!Q_stricmp(token, "useModel"))
		{
			*field |= (1 << FXFLAG_USEMODEL);
		}
		else if(!Q_stricmp(token, "useBBox"))
		{
			*field |= (1 << FXFLAG_USEBBOX);
		}
		else if(!Q_stricmp(token, "usePhysics"))
		{
			*field |= (1 << FXFLAG_USEPHYSICS);
		}
		else if(!Q_stricmp(token, "expensivePhysics"))
		{
			*field |= (1 << FXFLAG_EXPENSIVEPHYSICS);
		}
		else if(!Q_stricmp(token, "ghoul2Collision"))
		{
			*field |= (1 << FXFLAG_GHOUL2COLLISION);
		}
		else if(!Q_stricmp(token, "ghoul2Decals"))
		{
			*field |= (1 << FXFLAG_GHOUL2DECALS);
		}
		else if(!Q_stricmp(token, "impactKills"))
		{
			*field |= (1 << FXFLAG_IMPACTKILLS);
		}
		else if(!Q_stricmp(token, "impactFx"))
		{
			*field |= (1 << FXFLAG_IMPACTFX);
		}
		else if(!Q_stricmp(token, "deathFx"))
		{
			*field |= (1 << FXFLAG_DEATHFX);
		}
		else if(!Q_stricmp(token, "useAlpha"))
		{
			*field |= (1 << FXFLAG_USEALPHA);
		}
		else if(!Q_stricmp(token, "emitFx"))
		{
			*field |= (1 << FXFLAG_EMITFX);
		}
		else if(!Q_stricmp(token, "relative"))
		{
			*field |= (1 << FXFLAG_RELATIVE);
		}
		else if(!Q_stricmp(token, "setShaderTime"))
		{
			*field |= (1 << FXFLAG_SETSHADERTIME);
		}
		else if(!Q_stricmp(token, "paperPhysics") || !Q_stricmp(token, "localizedFlash") || !Q_stricmp(token, "playerView"))
		{
			*field |= (1 << FXFLAG_PAPERPHYSICS);
		}
	} while((token = COM_ParseExt(buf, qfalse)) && token && token[0]);
	return qtrue;
}

qboolean FX_Parse_Field_Spawnflags(int *field, char **buf)
{
	char *token = COM_ParseExt(buf, qfalse);
	if(!token || !token[0])
	{
		return qfalse;
	}

	do
	{
		if(!Q_stricmp(token, "cheapOrgCalc"))
		{
			*field |= (1 << FXSFLAG_CHEAPORIGINCALC);
		}
		else if(!Q_stricmp(token, "org2FromTrace"))
		{
			*field |= (1 << FXSFLAG_ORG2FROMTRACE);
		}
		else if(!Q_stricmp(token, "traceImpactFx"))
		{
			*field |= (1 << FXSFLAG_TRACEIMPACTFX);
		}
		else if(!Q_stricmp(token, "org2IsOffset"))
		{
			*field |= (1 << FXSFLAG_ORG2ISOFFSET);
		}
		else if(!Q_stricmp(token, "cheapOrg2Calc"))
		{
			*field |= (1 << FXSFLAG_CHEAPORIGIN2CALC);
		}
		else if(!Q_stricmp(token, "absoluteVelocity"))
		{
			*field |= (1 << FXSFLAG_ABSOLUTEVEL);
		}
		else if(!Q_stricmp(token, "absoluteAcceleration"))
		{
			*field |= (1 << FXSFLAG_ABSOLUTEACCEL);
		}
		else if(!Q_stricmp(token, "orgOnSphere"))
		{
			*field |= (1 << FXSFLAG_ORGONSPHERE);
		}
		else if(!Q_stricmp(token, "orgOnCylinder"))
		{
			*field |= (1 << FXSFLAG_ORGONCYLINDER);
		}
		else if(!Q_stricmp(token, "axisOnSphere"))
		{
			*field |= (1 << FXSFLAG_AXISFROMSPHERE);
		}
		else if(!Q_stricmp(token, "randRotAroundFwd"))
		{
			*field |= (1 << FXSFLAG_RANDROTAROUNDFWD);
		}
		else if(!Q_stricmp(token, "evenDistribution"))
		{
			*field |= (1 << FXSFLAG_EVENDISTRIBUTION);
		}
		else if(!Q_stricmp(token, "rgbComponentInterpolation"))
		{
			*field |= (1 << FXSFLAG_RGBCOMPONENTINTERPOLATION);
		}
		else if(!Q_stricmp(token, "affectedByWind"))
		{
			*field |= (1 << FXSFLAG_AFFECTEDBYWIND);
		}
	} while((token = COM_ParseExt(buf, qfalse)) && token && token[0]);
	return qtrue;
}

qboolean FX_Parse_Field_Shaderlist(fxShaderList_t *field, char **buf)
{
	char *token;
	qboolean endBracketFound = qfalse;
	token = COM_Parse(buf);
	if(!token || !token[0])
	{
		Com_Printf( S_COLOR_YELLOW "WARNING: Missing '[' in effect file.\n");
		return qfalse;
	}
	do
	{
		if(!Q_stricmp(token, "]"))
		{
			endBracketFound = qtrue;
			break;
		}
		else if(!Q_stricmp(token, "["))
		{
			continue;
		}
		if(strlen(token) >= MAX_QPATH)
		{
			Com_Error(ERR_FATAL, "FX_Parse_Field_Shaderlist: MAX_QPATH exceeded (%s)", token);
			return qfalse;
		}
		{

			field->fields = (char (*)[MAX_QPATH])realloc(field->fields, (MAX_QPATH*(field->numFields+2)));
			field->fieldHandles = (qhandle_t *)realloc(field->fieldHandles, sizeof(qhandle_t)*(field->numFields+2));
			strcpy(field->fields[field->numFields++], token);
		}
	} while((token = COM_Parse(buf)) && token && token[0]);
	return qtrue;
}

void FX_Parse_Field_TimelapseFlags(fxTimeLapse_t *field, char **buf)
{
	char *token;
	token = COM_Parse(buf);
	do
	{
		if(!token || !token[0])
		{
			return;
		}
		else if(!Q_stricmp(token, "}"))
		{
			return;
		}
		else if(!Q_stricmp(token, "constant"))
		{
			field->flags |= FXTLF_CONSTANT;
			continue;
		}
		else if(!Q_stricmp(token, "linear"))
		{
			field->flags |= ( 1 << FXTLF_LINEAR);
			continue;
		}
		else if(!Q_stricmp(token, "nonlinear"))
		{
			field->flags |= ( 1 << FXTLF_NONLINEAR );
			continue;
		}
		else if(!Q_stricmp(token, "wave"))
		{
			field->flags |= ( 1 << FXTLF_WAVE );
			continue;
		}
		else if(!Q_stricmp(token, "clamp"))
		{
			field->flags |= ( 1 << FXTLF_CLAMP);
			continue;
		}
		else if(!Q_stricmp(token, "random"))
		{
			field->flags |= ( 1 << FXTLF_RANDOM);
			continue;
		}
	} while((token = COM_ParseExt(buf, qfalse)) && token && token[0]);
}

qboolean FX_Parse_Field_Timelapse(fxTimeLapse_t *field, char **buf)
{
	char *token = COM_Parse(buf);
	qboolean endBracketFound = qfalse;
	if(!field)
	{
		// Simply keep parsing until you find a }
		do
		{
			token = COM_Parse(buf);
		} while(token && token[0] && token [0] != '}');
		return qtrue;
	}
	if(!token || !token[0])
	{
		Com_Printf( S_COLOR_YELLOW "WARNING: Missing '{' in effect file timelapse %s\n", parsedfile.name);
		return qfalse;
	}
	do
	{
		if(!Q_stricmp(token, "}"))
		{
			endBracketFound = qtrue;
			break;
		}
		if(!Q_stricmp(token, "start"))
		{
			switch(field->timelapseType)
			{
				case FXTLT_INT:
					if(!FX_Parse_Field_FXInt(&field->start.si, buf))
					{
						return qfalse;
					}
					break;
				default:
				case FXTLT_FLOAT:
					if(!FX_Parse_Field_FXFloat(&field->start.sf, buf))
					{
						return qfalse;
					}
					break;
				case FXTLT_VECTOR:
					if(!FX_Parse_Field_FXVec3(&field->start.sv, buf))
					{
						return qfalse;
					}
					break;
			}
		}
		else if(!Q_stricmp(token, "end"))
		{
			switch(field->timelapseType)
			{
				case FXTLT_INT:
					if(!FX_Parse_Field_FXInt(&field->end.ei, buf))
					{
						return qfalse;
					}
					break;
				default:
				case FXTLT_FLOAT:
					if(!FX_Parse_Field_FXFloat(&field->end.ef, buf))
					{
						return qfalse;
					}
					break;
				case FXTLT_VECTOR:
					if(!FX_Parse_Field_FXVec3(&field->end.ev, buf))
					{
						return qfalse;
					}
					break;
			}
		}
		else if(!Q_stricmp(token, "parm"))
		{
			if(!FX_Parse_Field_FXInt(&field->parameter, buf))
			{
				Com_Printf( S_COLOR_YELLOW "WARNING: FX \"parm\" field with no value(s): %s\n", parsedfile.name);
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "flags"))
		{
			FX_Parse_Field_TimelapseFlags(field, buf);
		}
		
	} while((token = COM_Parse(buf)) && token && token[0]);
	return qtrue;
}

//
//		Functions which parse a specific segment
//

qboolean FX_ParseCameraShake(char **buf)
{
	// Skip the leading {
	FXCameraShakeSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		// Set all defaults here
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_CAMERASHAKE;
		segData = (FXCameraShakeSegment_t *)malloc(sizeof(FXCameraShakeSegment_t));
		Com_Memset(segData, 0, sizeof(FXCameraShakeSegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->radius, 10);
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if (!Q_stricmp(token, "bounce"))
			{
				SkipRestOfLine(buf);
				continue;			// Unknown;
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "intensity"))
			{
				if(!FX_Parse_Field_FXInt(&segData->intensity, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"intensity\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXInt(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else 
			{
				Com_Printf( S_COLOR_YELLOW "WARNING: Unknown token '%s' in efx file %s\n", token, parsedfile.name);
				continue;
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXCameraShakeSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseCylinder(char **buf)
{
	FXCylinderSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		// Defaults
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_CYLINDER;
		segData = (FXCylinderSegment_t *)malloc(sizeof(FXCylinderSegment_t));
		Com_Memset(segData, 0, sizeof(FXCylinderSegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		FX_Copy(segData->size.start.sf, 1.0f);
		FX_Copy(segData->size2.start.sf, 1.0f);
		FX_Copy(segData->length.start.sf, 1.0f);
		FX_Copy(segData->alpha.start.sf, 1.0f);
		FXV_Copy(segData->rgb.start.sv, 1.0f);
		// Set up timelapses
		segData->alpha.timelapseType = FXTLT_FLOAT;
		segData->length.timelapseType = FXTLT_FLOAT;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->size2.timelapseType = FXTLT_FLOAT;
		segData->shader.fields = shaderFix;
		segData->shader.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags") || !Q_stricmp(token, "flag"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags") || !Q_stricmp(token, "spawnflag"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->height, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"height\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->alpha, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->shader, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXCylinderSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseDecal(char **buf)
{
	FXDecalSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_DECAL;
		segData = (FXDecalSegment_t *)malloc(sizeof(FXDecalSegment_t));
		Com_Memset(segData, 0, sizeof(FXDecalSegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		FX_Copy(segData->size.start.sf, 1.0f);
		FX_Copy(segData->alpha.start.sf, 1.0f);
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->alpha.timelapseType = FXTLT_FLOAT;
		segData->shader.fields = shaderFix;
		segData->shader.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->alpha, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->shader, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->height, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"height\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rotation"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->rotation, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rotation\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXDecalSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}
qboolean FX_ParseElectricity(char **buf)
{
	FXElectricitySegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_ELECTRICITY;
		segData = (FXElectricitySegment_t *)malloc(sizeof(FXElectricitySegment_t));
		Com_Memset(segData, 0, sizeof(FXElectricitySegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->alpha.timelapseType = FXTLT_FLOAT;
		segData->shader.fields = shaderFix;
		segData->shader.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin2"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin2, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin2\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->alpha, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->shader, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->height, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"height\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXElectricitySegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}
qboolean FX_ParseEmitter(char **buf)
{
	FXEmitterSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_EMITTER;
		segData = (FXEmitterSegment_t *)malloc(sizeof(FXEmitterSegment_t));
		Com_Memset(segData, 0, sizeof(FXEmitterSegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		segData->size.timelapseType = FXTLT_FLOAT;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->height, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"height\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXEmitterSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseFlash(char **buf)
{
	FXFlashSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_FLASH;
		segData = (FXFlashSegment_t *)malloc(sizeof(FXFlashSegment_t));
		Com_Memset(segData, 0, sizeof(FXFlashSegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->alpha.timelapseType = FXTLT_FLOAT;
		segData->shader.fields = shaderFix;
		segData->shader.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->alpha, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->shader, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXFlashSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseFXRunner(char **buf)
{
	//FXCameraShakeSegment_t seg;
	return qtrue;
}

qboolean FX_ParseLight(char **buf)
{
	FXLightSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_LIGHT;
		segData = (FXLightSegment_t *)malloc(sizeof(FXLightSegment_t));
		Com_Memset(segData, 0, sizeof(FXLightSegment_t));
		FX_Copy(segData->life, 50);
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->size.start.sf[0] = segData->size.start.sf[1] = 1;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->rgb.start.sv[0][0] = segData->rgb.start.sv[0][1] = segData->rgb.start.sv[0][2] = \
			segData->rgb.start.sv[1][0] = segData->rgb.start.sv[1][1] = segData->rgb.start.sv[1][2] = 1.0f;		// White color default for lights
		parsedfile.segments[parsedfile.numSegments].flags = 0;
		parsedfile.segments[parsedfile.numSegments].spawnflags = 0;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(NULL, buf))
				{
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXLightSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseLine(char **buf)
{
	FXLineSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_LINE;
		segData = (FXLineSegment_t *)malloc(sizeof(FXLineSegment_t));
		Com_Memset(segData, 0, sizeof(FXLineSegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->alpha.timelapseType = FXTLT_FLOAT;
		segData->shader.fields = shaderFix;
		segData->shader.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin2"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin2, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin2\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->alpha, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->shader, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->height, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"height\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXLineSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseParticle(char **buf, qboolean oriented)
{
	FXParticleSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		vec3_t white = {1.0f, 1.0f, 1.0f};
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = (oriented) ? EFXS_ORIENTEDPARTICLE : EFXS_PARTICLE;
		segData = (FXParticleSegment_t *)malloc(sizeof(FXParticleSegment_t));
		Com_Memset(segData, 0, sizeof(FXParticleSegment_t));
		// TODO: put this all in a huge function
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->alpha.timelapseType = FXTLT_FLOAT;
		FX_Copy(segData->alpha.start.sf, 1.0f);
		FX_Copy(segData->alpha.end.ef, 1.0f);
		FXV_Copy(segData->rgb.start.sv, 1.0f);
		FXV_Copy(segData->rgb.end.ev, 1.0f);
		FX_Copy(segData->size.start.sf, 1.0f);
		FX_Copy(segData->size.end.ef, 1.0f);
		segData->shader.fields = shaderFix;
		segData->shader.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->alpha, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->shader, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->height, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"height\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rotation"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->rotation, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rotation\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rotationDelta"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->rotationDelta, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rotationDelta\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		Com_Printf("^3WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXParticleSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseSound(char **buf)
{
	FXSoundSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_SOUND;
		segData = (FXSoundSegment_t *)malloc(sizeof(FXSoundSegment_t));
		Com_Memset(segData, 0, sizeof(FXSoundSegment_t));
		FX_Copy(segData->count, 1);
		segData->sound.fields = shaderFix;
		segData->sound.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "sound", 5))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->sound, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"sound\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXSoundSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseTail(char **buf)
{
	FXTailSegment_t *segData;
	qboolean foundLastBracket = qfalse;
	char *token = COM_Parse(buf);
	if(token[0] && token)
	{
		char (*shaderFix)[MAX_QPATH] = (char (*)[MAX_QPATH])malloc(MAX_QPATH);
		qhandle_t *shaderHandleFix = (qhandle_t *)malloc(sizeof(qhandle_t));
		parsedfile.segments[parsedfile.numSegments].segmentType = EFXS_TAIL;
		segData = (FXTailSegment_t *)malloc(sizeof(FXTailSegment_t));
		Com_Memset(segData, 0, sizeof(FXTailSegment_t));
		FX_Copy(segData->life, 50);
		FX_Copy(segData->count, 1);
		segData->size.timelapseType = FXTLT_FLOAT;
		segData->length.timelapseType = FXTLT_FLOAT;
		segData->rgb.timelapseType = FXTLT_VECTOR;
		segData->alpha.timelapseType = FXTLT_FLOAT;
		segData->shader.fields = shaderFix;
		segData->shader.fieldHandles = shaderHandleFix;
		while(1)
		{
			token = COM_Parse(buf);
			if(!token || !token[0])
			{
				break;
			}
			if(token[0] == '}')
			{
				foundLastBracket = qtrue;
				break;
			}
			if(!Q_stricmp(token, "name"))
			{
				SkipRestOfLine(buf);
				continue;			// Editor only - skip this field
			} else if(!Q_stricmp(token, "life"))
			{
				if(!FX_Parse_Field_FXInt(&segData->life, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"life\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!FX_Parse_Field_FXInt(&segData->delay, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"delay\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!FX_Parse_Field_FXInt(&segData->count, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"count\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!FX_Parse_Field_FXInt(&segData->cullrange, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!FX_Parse_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"flags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!FX_Parse_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!FX_Parse_Field_FXVec3(&segData->origin, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"origin\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->alpha, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "length"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->length, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!FX_Parse_Field_Shaderlist(&segData->shader, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->rgb, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!FX_Parse_Field_Timelapse(&segData->size, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"size\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->radius, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"radius\" in %s\n", parsedfile.name);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!FX_Parse_Field_FXFloat(&segData->height, buf))
				{
					Com_Printf( S_COLOR_YELLOW "WARNING: Missing value for \"height\" in %s\n", parsedfile.name);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		Com_Printf( S_COLOR_YELLOW "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXTailSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean FX_ParseEffectFile(const char *fileName)
{
	// Open the effect file and parse it
	char *buffer_p;
	union {
		char *c;
		void *v;
	} buffer;
	int len = FS_ReadFile(fileName, &buffer.v);

	if(len <= 0)
	{
		return qfalse;
	}
	if(buffer.c[0] <= 0)
	{
		return qfalse;
	}
	// OK, now that we've performed basic checks on the file, let's start to parse it
	Com_Memset(&parsedfile, 0, sizeof(parsedfile));
	Q_strncpyz(parsedfile.name, fileName, sizeof(parsedfile.name));
	parsedfile.segments = (FXSegment_t *)malloc(sizeof(FXSegment_t));
	Com_Memset(parsedfile.segments, 0, sizeof(FXSegment_t));

	buffer_p = buffer.c;
	while(1)
	{
		char *token = COM_ParseExt( &buffer_p, qtrue );
		if( !token || !token[0] )
		{
			break;
		}

		if(!Q_stricmp(token, "CameraShake"))
		{
			if(!FX_ParseCameraShake(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Cylinder"))
		{
			if(!FX_ParseCylinder(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Decal"))
		{
			if(!FX_ParseDecal(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Electricity"))
		{
			if(!FX_ParseElectricity(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Emitter"))
		{
			if(!FX_ParseEmitter(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Flash"))
		{
			if(!FX_ParseFlash(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "FXRunner"))
		{
			if(!FX_ParseFXRunner(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Light"))
		{
			if(!FX_ParseLight(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Line"))
		{
			if(!FX_ParseLine(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "OrientedParticle"))
		{
			if(!FX_ParseParticle(&buffer_p, qtrue))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Particle"))
		{
			if(!FX_ParseParticle(&buffer_p, qfalse))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Sound"))
		{
			if(!FX_ParseSound(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Tail"))
		{
			if(!FX_ParseTail(&buffer_p))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "repeatDelay"))
		{
			SkipRestOfLine(&buffer_p);
		}
	}
	return qtrue;
}

void FX_FilePrecache(void)
{
	// Runs the second pass of the file handling.
	// This is used to register any sort of shaders, sounds, models, or other effects
	int i;
	int k;
	for(i = 0; i < parsedfile.numSegments; i++)
	{
		switch(parsedfile.segments[i].segmentType)
		{
			// Precaching these is a huge mess, but unfortunately there's no non-messy way of doing it, I'm afraid --eez
			case EFXS_CYLINDER:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXCylinderSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXCylinderSegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXCylinderSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_DECAL:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXDecalSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXDecalSegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXDecalSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_ELECTRICITY:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXElectricitySegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXElectricitySegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXElectricitySegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_FLASH:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXFlashSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXFlashSegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXFlashSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_LINE:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXLineSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXLineSegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXLineSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_ORIENTEDPARTICLE:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXOrientedParticleSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXOrientedParticleSegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXOrientedParticleSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_PARTICLE:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXParticleSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXParticleSegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXParticleSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_TAIL:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXTailSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXTailSegment->shader.fieldHandles[k] = 
							re.RegisterShader(parsedfile.segments[i].SegmentData.FXTailSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_SOUND:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXSoundSegment->sound.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXSoundSegment->sound.fieldHandles[k] = 
							S_RegisterSound(parsedfile.segments[i].SegmentData.FXSoundSegment->sound.fields[k], qfalse);
					}
				}
				break;
			default:
				// No valid segment type? NO PRECACHE 4 U
				break;
		}
	}
}
