#pragma once
#ifndef _FONT_H
#define _FONT_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "picture.h"
#include <math.h>

#define USHORT_L(buf) ((buf[0]<<8)|(buf[1]))

//------------------ Data Type ---------------------//
typedef unsigned char	BYTE;
typedef char			CHAR;
typedef unsigned short	USHORT;
typedef short           SHORT;
typedef unsigned long   ULONG;
typedef long            LONG;

//------------------ Data Struct-------------------//
typedef struct
{
	float verson;
	USHORT numtable;
	USHORT searchRange;
	USHORT entrySelector;
	USHORT rangeShift;
}TTF_TabDir;

typedef struct
{
	CHAR Tags[4];
	ULONG checkSum;
	ULONG offset;
	ULONG length;
}TTF_Table;


// maxp - Maximum Profile
typedef struct
{
	float Table_version_number;
	USHORT numGlyphs;             // The number of glyphs in the font
	USHORT maxPoints;             // Maximum points in a non-composite glyph
	USHORT maxContours;           // Maximum contours in a non-composite glyph
	USHORT maxCompositePoints;    // Maximum points in a composite glyph
	USHORT maxCompositeContours;  // Maximum Contours in a composite glyph
	USHORT maxZones;              // 1 if instructions do not not use the twilight zone (Z0), or 2 if instructions do use Z0;should be set to 2 in most cases.
	USHORT maxTwilightPoints;     // Maximum points used in Z0
	USHORT maxStorage;            // Number of Storage Area locations.
	USHORT maxFunctionDefs;       // Number of FDEFs.
	USHORT maxInstructionDefs;    // Number of IDEFs.
	USHORT maxStackElements;      // Maximum stack depth.
	USHORT maxSizeOfInstructions; // Maximum byte count for glyph instructions
	USHORT maxComponentElements;  // Maximum number of components referenced at "top level" for any composite glyph
	USHORT maxComponentDepth;     // Maximum levels of recursion; 1 for simple components.
}TTF_MAXP_TABLE;

typedef struct
{
	float Table_version_number;
	float font_Version;
	ULONG checkSumAdjustment;  //set it to 0,sum the entire font as ULONG,then store 0xB1B0AFBA - sum
	ULONG magicNumber;		// set to 0x5F0F3CF5
	USHORT flags;           // Bit 0 - baseline for font at y = 0
	// Bit 1 - left sidebearing at x = 0
	// Bit 2 - instructions may depend on point size
	// Bit 3 - force ppem to integer values for all internal scaler math;may use fractional ppem sizes if this bit is clear;
	// Bit 4 - instructions may alter advabce width (the advance widths might not scale linearly);
	// Note: ALL other bits must be zeros.
	USHORT unitsPerEm;
	time_t time_creat;
	time_t time_modified;
	SHORT xMin;            // For all glyph bounding boxes;
	SHORT yMin;            // For all glyph bounding boxes;
	SHORT xMax;            // For all glyph bounding boxes;
	SHORT yMax;            // For all glyph bounding boxes;
	USHORT macStyle;       // Bit 0 bold(if set to 1);Bit 1 italic(if set to 1); Bits 2-15 reserved (set to 0);
	USHORT lowestRecPPEM;  // Smallest readable size in pixels;
	SHORT  fontDirectionHint; // 0 Fully mixed directional glyphs;
	// 1 only strongly left to right;
	// 2 Like 1 but also contains neutrals
	// -1 Only strongly right to left;
	// -2 Like -1 but also contains neutrals
	SHORT indexToLocFormat;   // 0 for short offset, 1 for long.
	SHORT glyphDataFonmat;    // 0 for current format.
}TTF_HEAD_TABLE;

int show_char(_Suyu_BMP* img, int x, int y, int font_height, char* filename, wchar_t w_c);
//void show_string(_Suyu_BMP* img, int fontHeight, int x, int y, wchar_t* str);
void show_string(_Suyu_BMP* img, char* filename, int fontHeight, int x, int y, wchar_t* str);


#endif
