#include "tr_fx.h"

FXFile_t parsedfile;
int FX_numFXFiles;
FXFile_t *FX_fxHandles;
qboolean justEndedField = qfalse;

//
//		Functions that parse a specific type of field in the .efx file
//

qboolean CFxParser_Field_FXInt(fxInt_t *field, char **buf)
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

qboolean CFxParser_Field_FXFloat(fxFloat_t *field, char **buf)
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

qboolean CFxParser_Field_FXVec3(fxVec3_t *field, char **buf)
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
qboolean CFxParser_Field_Flags(int *field, char **buf)
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

qboolean CFxParser_Field_Spawnflags(int *field, char **buf)
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

qboolean CFxParser_Field_Shaderlist(fxShaderList_t *field, char **buf)
{
	char *token;
	qboolean endBracketFound = qfalse;
	token = COM_Parse(buf);
	if(!token || !token[0])
	{
		ri.Printf( PRINT_WARNING, "WARNING: Missing '[' in effect file.\n");
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
		if(strlen(token) > MAX_QPATH)
		{
			ri.Error(ERR_FATAL, "CFxParser_Field_Shaderlist: MAX_QPATH exceeded (%s)", token);
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

void CFxParser_Field_TimelapseFlags(fxTimeLapse_t *field, char **buf)
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

qboolean CFxParser_Field_Timelapse(fxTimeLapse_t *field, char **buf)
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
		ri.Printf(PRINT_WARNING, "WARNING: Missing '{' in effect file timelapse %s\n", parsedfile.filename);
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
					if(!CFxParser_Field_FXInt(&field->start.si, buf))
					{
						return qfalse;
					}
					break;
				default:
				case FXTLT_FLOAT:
					if(!CFxParser_Field_FXFloat(&field->start.sf, buf))
					{
						return qfalse;
					}
					break;
				case FXTLT_VECTOR:
					if(!CFxParser_Field_FXVec3(&field->start.sv, buf))
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
					if(!CFxParser_Field_FXInt(&field->end.ei, buf))
					{
						return qfalse;
					}
					break;
				default:
				case FXTLT_FLOAT:
					if(!CFxParser_Field_FXFloat(&field->end.ef, buf))
					{
						return qfalse;
					}
					break;
				case FXTLT_VECTOR:
					if(!CFxParser_Field_FXVec3(&field->end.ev, buf))
					{
						return qfalse;
					}
					break;
			}
		}
		else if(!Q_stricmp(token, "parm"))
		{
			if(!CFxParser_Field_FXInt(&field->parameter, buf))
			{
				ri.Printf(PRINT_WARNING, "WARNING: FX \"parm\" field with no value(s): %s\n", parsedfile.filename);
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "flags"))
		{
			CFxParser_Field_TimelapseFlags(field, buf);
		}
		
	} while((token = COM_Parse(buf)) && token && token[0]);
	justEndedField = qtrue;
	return qtrue;
}

//
//		Functions which parse a specific segment
//

qboolean CFxParser_ParseCameraShake(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "intensity"))
			{
				if(!CFxParser_Field_FXInt(&segData->intensity, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"intensity\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXInt(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else 
			{
				ri.Printf(PRINT_WARNING, "WARNING: Unknown token '%s' in efx file %s\n", token, parsedfile.filename);
				continue;
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXCameraShakeSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseCylinder(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags") || !Q_stricmp(token, "flag"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags") || !Q_stricmp(token, "spawnflag"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXFloat(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!CFxParser_Field_FXFloat(&segData->height, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"height\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(&segData->alpha, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!CFxParser_Field_Shaderlist(&segData->shader, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXCylinderSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseDecal(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(&segData->alpha, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!CFxParser_Field_Shaderlist(&segData->shader, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXFloat(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!CFxParser_Field_FXFloat(&segData->height, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"height\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rotation"))
			{
				if(!CFxParser_Field_FXFloat(&segData->rotation, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rotation\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXDecalSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}
qboolean CFxParser_ParseElectricity(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin2"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin2, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin2\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(&segData->alpha, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!CFxParser_Field_Shaderlist(&segData->shader, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXFloat(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!CFxParser_Field_FXFloat(&segData->height, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"height\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXElectricitySegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}
qboolean CFxParser_ParseEmitter(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXFloat(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!CFxParser_Field_FXFloat(&segData->height, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"height\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXEmitterSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseFlash(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(&segData->alpha, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!CFxParser_Field_Shaderlist(&segData->shader, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXFlashSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseFXRunner(char **buf)
{
	//FXCameraShakeSegment_t seg;
	return qtrue;
}

qboolean CFxParser_ParseLight(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(NULL, buf))
				{
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXLightSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseLine(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin2"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin2, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin2\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(&segData->alpha, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!CFxParser_Field_Shaderlist(&segData->shader, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXFloat(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!CFxParser_Field_FXFloat(&segData->height, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"height\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXLineSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseParticle(char **buf, qboolean oriented)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(&segData->alpha, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!CFxParser_Field_Shaderlist(&segData->shader, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXFloat(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!CFxParser_Field_FXFloat(&segData->height, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"height\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rotation"))
			{
				if(!CFxParser_Field_FXFloat(&segData->rotation, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rotation\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rotationDelta"))
			{
				if(!CFxParser_Field_FXFloat(&segData->rotationDelta, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rotationDelta\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		Com_Printf("^3WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXParticleSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseSound(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXFloat(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "sound", 5))
			{
				if(!CFxParser_Field_Shaderlist(&segData->sound, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"sound\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXSoundSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

qboolean CFxParser_ParseTail(char **buf)
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
				if(!CFxParser_Field_FXInt(&segData->life, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"life\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "delay"))
			{
				if(!CFxParser_Field_FXInt(&segData->delay, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"delay\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "count"))
			{
				if(!CFxParser_Field_FXInt(&segData->count, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"count\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "cullrange"))
			{
				if(!CFxParser_Field_FXInt(&segData->cullrange, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"cullrange\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "flags"))
			{
				if(!CFxParser_Field_Flags(&parsedfile.segments[parsedfile.numSegments].flags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"flags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "spawnflags"))
			{
				if(!CFxParser_Field_Spawnflags(&parsedfile.segments[parsedfile.numSegments].spawnflags, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"spawnflags\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "origin"))
			{
				if(!CFxParser_Field_FXVec3(&segData->origin, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"origin\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "alpha"))
			{
				if(!CFxParser_Field_Timelapse(&segData->alpha, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "length"))
			{
				if(!CFxParser_Field_Timelapse(&segData->length, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"alpha\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmpn(token, "shader", 6))
			{
				if(!CFxParser_Field_Shaderlist(&segData->shader, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Could not parse value \"shaders\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "rgb"))
			{
				if(!CFxParser_Field_Timelapse(&segData->rgb, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"rgb\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "size"))
			{
				if(!CFxParser_Field_Timelapse(&segData->size, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"size\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "radius"))
			{
				if(!CFxParser_Field_FXFloat(&segData->radius, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"radius\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			} else if(!Q_stricmp(token, "height"))
			{
				if(!CFxParser_Field_FXFloat(&segData->height, buf))
				{
					ri.Printf(PRINT_WARNING, "WARNING: Missing value for \"height\" in %s\n", parsedfile.filename);
					return qfalse;
				}
			}
		}
	}
	if(!foundLastBracket)
	{
		free(segData);
		ri.Printf(PRINT_WARNING, "WARNING: Ending bracket not found on .efx file. BAD THINGS WILL HAPPEN!\n");
		return qfalse;
	}
	// Add the segment to the effect file
	parsedfile.segments[parsedfile.numSegments].SegmentData.FXTailSegment = segData;
	parsedfile.numSegments++;
	parsedfile.segments = (FXSegment_t *)realloc(parsedfile.segments, sizeof(FXSegment_t)*(parsedfile.numSegments+1));
	return qtrue;
}

//
//		Main effect parser
//

qboolean CFxParser_ParseEffect(char *fileName)
{
	// Open the effect file and parse it
	char *buffer;
	int len = ri.FS_ReadFile(fileName, (void **)&buffer);

	if(len <= 0)
	{
		return qfalse;
	}
	if(buffer[0] <= 0)
	{
		return qfalse;
	}
	// OK, now that we've performed basic checks on the file, let's start to parse it
	Com_Memset(&parsedfile, 0, sizeof(parsedfile));
	strcpy(parsedfile.filename, fileName);
	parsedfile.segments = (FXSegment_t *)malloc(sizeof(FXSegment_t));
	Com_Memset(parsedfile.segments, 0, sizeof(FXSegment_t));

	while(1)
	{
		char *token = COM_ParseExt( &buffer, qtrue );
		if( !token[0] || !token )
		{
			break;
		}

		if(!Q_stricmp(token, "CameraShake"))
		{
			if(!CFxParser_ParseCameraShake(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Cylinder"))
		{
			if(!CFxParser_ParseCylinder(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Decal"))
		{
			if(!CFxParser_ParseDecal(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Electricity"))
		{
			if(!CFxParser_ParseElectricity(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Emitter"))
		{
			if(!CFxParser_ParseEmitter(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Flash"))
		{
			if(!CFxParser_ParseFlash(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "FXRunner"))
		{
			if(!CFxParser_ParseFXRunner(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Light"))
		{
			if(!CFxParser_ParseLight(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Line"))
		{
			if(!CFxParser_ParseLine(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "OrientedParticle"))
		{
			if(!CFxParser_ParseParticle(&buffer, qtrue))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Particle"))
		{
			if(!CFxParser_ParseParticle(&buffer, qfalse))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Sound"))
		{
			if(!CFxParser_ParseSound(&buffer))
			{
				return qfalse;
			}
		}
		else if(!Q_stricmp(token, "Tail"))
		{
			if(!CFxParser_ParseTail(&buffer))
			{
				return qfalse;
			}
		}
	}
	return qtrue;
}

//
//		Effect register function (usually called via trap calls)
//

void CFxScheduler_RunSecondPass(void)
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
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXCylinderSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_DECAL:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXDecalSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXDecalSegment->shader.fieldHandles[k] = 
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXDecalSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_ELECTRICITY:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXElectricitySegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXElectricitySegment->shader.fieldHandles[k] = 
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXElectricitySegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_FLASH:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXFlashSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXFlashSegment->shader.fieldHandles[k] = 
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXFlashSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_LINE:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXLineSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXLineSegment->shader.fieldHandles[k] = 
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXLineSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_ORIENTEDPARTICLE:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXOrientedParticleSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXOrientedParticleSegment->shader.fieldHandles[k] = 
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXOrientedParticleSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_PARTICLE:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXParticleSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXParticleSegment->shader.fieldHandles[k] = 
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXParticleSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_TAIL:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXTailSegment->shader.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXTailSegment->shader.fieldHandles[k] = 
							RE_RegisterShader(parsedfile.segments[i].SegmentData.FXTailSegment->shader.fields[k]);
					}
				}
				break;
			case EFXS_SOUND:
				{
					for(k = 0; k < parsedfile.segments[i].SegmentData.FXSoundSegment->sound.numFields; k++)
					{
						parsedfile.segments[i].SegmentData.FXSoundSegment->sound.fieldHandles[k] = 
							ri.RegisterSound(parsedfile.segments[i].SegmentData.FXSoundSegment->sound.fields[k], qfalse);
							//RE_RegisterShader(parsedfile.segments[i].SegmentData.FXSoundSegment->sound.fields[k]);
					}
				}
				break;
			default:
				// No valid segment type? NO PRECACHE 4 U
				break;
		}
	}
}

int CFxScheduler_RegisterEffect(const char *path)
{
	int i;
	char finalPath[MAX_QPATH];
	if(path[0] <= 0 || path[0] == '\n' || path[0] == '\t' || path[0] == '\r')
	{
		return 0;	// Not valid
	}
	if(strlen(path) >= MAX_QPATH)
	{
		ri.Error(ERR_FATAL, "%s exceeds MAX_QPATH", path);
	}
	if(Q_stricmpn(path, EFFECTS_FOLDER, 8) != 0)
	{
		Com_sprintf(finalPath, sizeof(finalPath), EFFECTS_FOLDER "%s", path);
	}
	else
	{
		Q_strncpyz(finalPath, path, sizeof(finalPath));
	}

	COM_DefaultExtension(finalPath, sizeof(finalPath), ".efx");

	// Loop through the FX files and see if we've got ourselves a repeat file
	for(i = 0; i < FX_numFXFiles; i++)
	{
		if(!Q_stricmp(FX_fxHandles[i].filename, finalPath))
		{
			// The filenames match -- return this handle
			return i;
		}
	}

	// No? Parse the effect file.
	if(!CFxParser_ParseEffect(finalPath))
	{
		ri.Printf(PRINT_WARNING, "WARNING: Failed to parse effect file (%s)\n", path);
		return 0;
	}
	else
	{
		// Allocate a little bit of mem and assign the file
		FX_fxHandles = (FXFile_t *)realloc(FX_fxHandles, sizeof(FXFile_t) * (FX_numFXFiles+1));
		CFxScheduler_RunSecondPass();
		FX_fxHandles[FX_numFXFiles] = parsedfile;
		FX_numFXFiles++;
		return FX_numFXFiles-1;
	}
}

//
//		Init and cleanup of the effects system
//

void CFxScheduler_FreeShaderField(fxShaderList_t *field)
{
	if(!field->fields)
	{
		return;
	}
	free(field->fields);
	if(!field->fieldHandles)
	{
		free(field->fieldHandles);
	}
}

void CFxScheduler_Init(void)
{
	FX_numFXFiles = 0;
	FX_fxHandles = (FXFile_t*)malloc(sizeof(FXFile_t));
	CFxScheduler_InitScheduler();
	//CFxScheduler_RegisterEffect("temp/shake");
}

void CFxScheduler_Cleanup(void)
{
	int i;
	int j;
	for(i = 0; i < FX_numFXFiles; i++)
	{
		for(j = 0; j < FX_fxHandles[i].numSegments; j++)
		{
			switch(FX_fxHandles[i].segments[j].segmentType)
			{
				case EFXS_CAMERASHAKE:
					free(FX_fxHandles[i].segments[j].SegmentData.FXCameraShakeSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXCameraShakeSegment = NULL;
					break;
				case EFXS_CYLINDER:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXCylinderSegment->shader);
					FX_fxHandles[i].segments[j].SegmentData.FXCylinderSegment = NULL;
					break;
				case EFXS_DECAL:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXDecalSegment->shader);
					free(FX_fxHandles[i].segments[j].SegmentData.FXDecalSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXDecalSegment = NULL;
					break;
				case EFXS_ELECTRICITY:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXElectricitySegment->shader);
					free(FX_fxHandles[i].segments[j].SegmentData.FXElectricitySegment);
					FX_fxHandles[i].segments[j].SegmentData.FXElectricitySegment = NULL;
					break;
				case EFXS_EMITTER:
					free(FX_fxHandles[i].segments[j].SegmentData.FXEmitterSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXEmitterSegment = NULL;
					break;
				case EFXS_FLASH:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXFlashSegment->shader);
					free(FX_fxHandles[i].segments[j].SegmentData.FXFlashSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXFlashSegment = NULL;
					break;
				case EFXS_FXRUNNER:
					//free(FX_fxHandles[i].segments[j].SegmentData.FXCameraShakeSegment);
					break;
				case EFXS_LIGHT:
					free(FX_fxHandles[i].segments[j].SegmentData.FXLightSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXLightSegment = NULL;
					break;
				case EFXS_LINE:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXLineSegment->shader);
					free(FX_fxHandles[i].segments[j].SegmentData.FXLineSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXLineSegment = NULL;
					break;
				case EFXS_ORIENTEDPARTICLE:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXOrientedParticleSegment->shader);
					free(FX_fxHandles[i].segments[j].SegmentData.FXOrientedParticleSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXOrientedParticleSegment = NULL;
					break;
				case EFXS_PARTICLE:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXParticleSegment->shader);
					free(FX_fxHandles[i].segments[j].SegmentData.FXParticleSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXParticleSegment = NULL;
					break;
				case EFXS_SOUND:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXSoundSegment->sound);
					free(FX_fxHandles[i].segments[j].SegmentData.FXSoundSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXSoundSegment = NULL;
					break;
				case EFXS_TAIL:
					CFxScheduler_FreeShaderField(&FX_fxHandles[i].segments[j].SegmentData.FXTailSegment->shader);
					free(FX_fxHandles[i].segments[j].SegmentData.FXTailSegment);
					FX_fxHandles[i].segments[j].SegmentData.FXTailSegment = NULL;
					break;
			}
		}

		free(FX_fxHandles[i].segments);
	}
	free(FX_fxHandles);
	CFxScheduler_FreeScheduler();
}