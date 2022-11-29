#pragma once
#ifndef  _PICTURE_H
#define  _PICTURE_H

typedef enum
{
	_S_OK = 0,
	_S_MERR,
	_S_PERR,
}_S_Flag;

typedef struct
{
	unsigned int* _BMP;
	int _Width;
	int _Hight;
	_S_Flag sflag;
}_Suyu_BMP;

void Init_Suyu_BMP(_Suyu_BMP* img, int width, int height);
void free_Suyu_BMP(_Suyu_BMP* BMP);
void BMP_draw_point(_Suyu_BMP* img, int x, int y, unsigned int color);
void BMP_Encode(char* filename, _Suyu_BMP* img);
int BMP_Decode(_Suyu_BMP* img, char* filename);
unsigned int BMP_GetColor(_Suyu_BMP img, int x, int y);
void draw_line_BMP(_Suyu_BMP* img, int x1, int y1, int x2, int y2, unsigned int color);

#endif