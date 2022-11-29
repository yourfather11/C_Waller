#include "font.h"

typedef struct
{
	long x;
	long y;
}POINT;

float Scale_entry;

ULONG bigendianlong(BYTE* buf)
{
	return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

float Fixedtofloat(BYTE* buf)
{
	return (buf[0] << 8) + buf[1] + ((buf[2] << 8) + buf[3]) / (2 << 16);
}

void draw_fontglyf(_Suyu_BMP* img, USHORT numberContours, USHORT* endPtrsContours, BYTE* flag, POINT* points);

void GetTTF_TableDir(FILE* fp, TTF_TabDir* tabdir)
{
	BYTE *buff;
	buff = (BYTE*)malloc(sizeof(TTF_TabDir));
	if (!buff)
		return;
	fseek(fp, 0, SEEK_SET);
	fread(buff, sizeof(TTF_TabDir), 1, fp);
	tabdir->verson = Fixedtofloat(buff);
	tabdir->numtable = (buff[4] << 8) + buff[5];
	tabdir->searchRange = (buff[6] << 8) + buff[7];
	tabdir->entrySelector = (buff[8] << 8) + buff[9];
	tabdir->rangeShift = (buff[10] << 8) + buff[11];
}

TTF_Table* GetTTF_Table(FILE* fp, TTF_TabDir tabdir)
{
	TTF_Table* tab = (TTF_Table*)malloc(sizeof(TTF_Table) * tabdir.numtable);
	int i = 0;
	BYTE buf[16];
	fseek(fp, 12, SEEK_SET);
	for (i = 0; i < tabdir.numtable; i++)
	{
		fread(buf, sizeof(TTF_Table), 1, fp);
		tab[i].Tags[0] = buf[0]; tab[i].Tags[1] = buf[1]; tab[i].Tags[2] = buf[2]; tab[i].Tags[3] = buf[3];
		tab[i].checkSum = bigendianlong(buf + 4);
		tab[i].offset = bigendianlong(buf + 8);
		tab[i].length = bigendianlong(buf + 12);
	}
	return tab;
}

ULONG CalcTableChecksum(ULONG table, ULONG length, FILE* fp)
{
	BYTE buf[4];
	ULONG sum = 0;
	ULONG Endptr = table + ((length + 3) & ~3) / sizeof(ULONG);
	fseek(fp, table, SEEK_SET);
	while (table < Endptr)
	{
		table++;
		fread(buf, 4, 1, fp);
		sum += bigendianlong(buf);
	}
	return sum;
}

void GetTTF_Head_Table(FILE* fp, TTF_Table head, TTF_HEAD_TABLE* htable)
{
	BYTE* buf;
	fseek(fp, head.offset, SEEK_SET);
	buf = (BYTE*)malloc(sizeof(BYTE) * head.length);
	fread(buf, head.length, 1, fp);
	htable->Table_version_number = Fixedtofloat(buf);
	htable->font_Version = Fixedtofloat(buf + 4);
	htable->checkSumAdjustment = bigendianlong(buf + 8);
	htable->magicNumber = bigendianlong(buf + 12);
	htable->flags = (buf[16] << 8) | buf[17];
	htable->unitsPerEm = (buf[18] << 8) | buf[19];
	htable->time_creat = (bigendianlong(buf + 20) << 32) | (bigendianlong(buf + 24));
	htable->time_modified = (bigendianlong(buf + 28) << 32) | (bigendianlong(buf + 32));
	htable->xMin = (buf[36] << 8) | buf[37];
	htable->yMin = (buf[38] << 8) | buf[39];
	htable->xMax = (buf[40] << 8) | buf[41];
	htable->yMax = (buf[42] << 8) | buf[43];
	htable->macStyle = (buf[44] << 8) | buf[45];
	htable->lowestRecPPEM = (buf[46] << 8) | buf[47];
	htable->fontDirectionHint = (buf[48] << 8) | buf[49];
	htable->indexToLocFormat = (buf[50] << 8) | buf[51];
	htable->glyphDataFonmat = (buf[52] << 8) | buf[53];
}

void GetTTF_maxp_Table(FILE* fp, TTF_Table maxp, TTF_MAXP_TABLE* mtable)
{
	BYTE* buf;
	fseek(fp, maxp.offset, SEEK_SET);
	buf = (BYTE*)malloc(sizeof(BYTE) * maxp.length);
	if (buf == NULL)return;
	fread(buf, maxp.length, 1, fp);
	mtable->Table_version_number = Fixedtofloat(buf);
	mtable->numGlyphs = (buf[4] << 8) | buf[5];
	mtable->maxPoints = (buf[6] << 8) | buf[7];
	mtable->maxContours = (buf[8] << 8) | buf[9];
	mtable->maxCompositePoints = (buf[10] << 8) | buf[11];
	mtable->maxCompositeContours = (buf[12] << 8) | buf[13];
	mtable->maxZones = (buf[14] << 8) | buf[15];
	mtable->maxTwilightPoints = (buf[16] << 8) | buf[17];
	mtable->maxStorage = (buf[18] << 8) | buf[19];
	mtable->maxFunctionDefs = (buf[20] << 8) | buf[21];
	mtable->maxInstructionDefs = (buf[22] << 8) | buf[23];
	mtable->maxStackElements = (buf[24] << 8) | buf[25];
	mtable->maxSizeOfInstructions = (buf[26] << 8) | buf[27];
	mtable->maxComponentElements = (buf[28] << 8) | buf[29];
	mtable->maxComponentDepth = (buf[30] << 8) | buf[31];
}

ULONG GetTTF_Cmap_Table(FILE* fp, TTF_Table cmap, int index)
{
	BYTE* buf;
	USHORT number_encodetable;
	USHORT Platform_ID;
	USHORT encode_ID;
	ULONG offset;
	USHORT format;
	USHORT i = 0, ptr = 0, j = 0;
	USHORT* endCount, * startCount, * idDelta, * idRangeOffset, * glyphIdArray;
	fseek(fp, cmap.offset, SEEK_SET);
	buf = (BYTE*)malloc(sizeof(BYTE) * cmap.length);
	fread(buf, cmap.length, 1, fp);
	number_encodetable = (buf[2] << 8) | buf[3];
	ptr = 4;
	for (i = 0; i < number_encodetable; i++)
	{
		Platform_ID = USHORT_L((buf + ptr)); ptr += 2;
		encode_ID = USHORT_L((buf + ptr)); ptr += 2;
		offset = bigendianlong(buf + ptr);
		ptr += 4;
		if (Platform_ID != 3 || encode_ID != 1) continue;
		ptr = offset;
		format = USHORT_L((buf + ptr)); ptr += 2;
		if (format != 4) return -1;
		USHORT length = USHORT_L((buf + ptr)); ptr += 2;
		USHORT version = USHORT_L((buf + ptr)); ptr += 2;
		USHORT segCount = USHORT_L((buf + ptr)) / 2; ptr += 2;
		USHORT searchRange = USHORT_L((buf + ptr)); ptr += 2;
		USHORT entrySeletor = USHORT_L((buf + ptr)); ptr += 2;
		USHORT rangeShift = USHORT_L((buf + ptr)); ptr += 2;
		// endCount[segCount]
		endCount = (USHORT*)malloc(sizeof(USHORT) * segCount);
		for (j = 0; j < segCount; j++)
		{
			endCount[j] = USHORT_L((buf + ptr)); ptr += 2;
		}

		USHORT reservePad = USHORT_L((buf + ptr)); ptr += 2;
		// startCount[segCount]
		startCount = (USHORT*)malloc(sizeof(USHORT) * segCount);
		for (j = 0; j < segCount; j++)
		{
			startCount[j] = USHORT_L((buf + ptr)); ptr += 2;
		}
		// idDelta[segCount]
		idDelta = (USHORT*)malloc(sizeof(USHORT) * segCount);
		for (j = 0; j < segCount; j++)
		{
			idDelta[j] = USHORT_L((buf + ptr)); ptr += 2;
		}
		// idRangeOffset[segCount]
		idRangeOffset = (USHORT*)malloc(sizeof(USHORT) * segCount);
		USHORT tmp_pos = ptr;
		for (j = 0; j < segCount; j++)
		{
			idRangeOffset[j] = USHORT_L((buf + ptr)); ptr += 2;
		}

		// glyphIdArry[]
		USHORT Offset;
		for (j = 0; j < segCount; j++)
		{
			if (idRangeOffset[j] == 0)
			{
				if (index >= startCount[j] && index < endCount[j])
				{
					return index + (SHORT)idDelta[j];
				}
			}
			else
			{
				if (index >= startCount[j] && index < endCount[j])
				{
					Offset = cmap.offset + tmp_pos + j * 2 + 2 * (index - startCount[j]) + idRangeOffset[j];
					fseek(fp, Offset, SEEK_SET);
					fread(buf, 2, 1, fp);
					return USHORT_L((buf));
				}
			}
		}
	}
	return -1;
}

int show_char(_Suyu_BMP* img, int x, int y, int font_height, char* filename, wchar_t w_c)
{
	FILE* ttf_fp;
	TTF_TabDir ttf_tdir;
	TTF_Table* tab;
	unsigned char buff[256];
	TTF_HEAD_TABLE htable;
	TTF_MAXP_TABLE mtable;
	USHORT advance_Width;
	SHORT lsb;
	USHORT* endPtsContours;
	unsigned char* flag;
	POINT* points;
	BYTE* instruction;
	USHORT instructionLength;
	ULONG len, read_len, i, head_index = 0, maxp_index = 0, cmap_index = 0, loca_index = 0, hmtx_index = 0, glyp_index = 0;
	fopen_s(&ttf_fp, filename, "rb");
	if (ttf_fp == NULL)
	{
		return -1;
	}
	GetTTF_TableDir(ttf_fp, &ttf_tdir);
	tab = GetTTF_Table(ttf_fp, ttf_tdir);
	for (i = 0; i < ttf_tdir.numtable; i++)
	{
		if (tab[i].Tags[0] == 'g' && tab[i].Tags[1] == 'l' && tab[i].Tags[2] == 'y' && tab[i].Tags[3] == 'f') glyp_index = i;
		if (tab[i].Tags[0] == 'h' && tab[i].Tags[1] == 'm' && tab[i].Tags[2] == 't' && tab[i].Tags[3] == 'x') hmtx_index = i;
		if (tab[i].Tags[0] == 'h' && tab[i].Tags[1] == 'e' && tab[i].Tags[2] == 'a' && tab[i].Tags[3] == 'd') head_index = i;
		if (tab[i].Tags[0] == 'm' && tab[i].Tags[1] == 'a' && tab[i].Tags[2] == 'x' && tab[i].Tags[3] == 'p') maxp_index = i;
		if (tab[i].Tags[0] == 'c' && tab[i].Tags[1] == 'm' && tab[i].Tags[2] == 'a' && tab[i].Tags[3] == 'p') cmap_index = i;
		if (tab[i].Tags[0] == 'l' && tab[i].Tags[1] == 'o' && tab[i].Tags[2] == 'c' && tab[i].Tags[3] == 'a') loca_index = i;
	}
	GetTTF_Head_Table(ttf_fp, tab[head_index], &htable);
	GetTTF_maxp_Table(ttf_fp, tab[maxp_index], &mtable);
	ULONG loca_offset = GetTTF_Cmap_Table(ttf_fp, tab[cmap_index], w_c);
	fseek(ttf_fp, tab[hmtx_index].offset + loca_offset * 4, SEEK_SET);
	fread(buff, 1, 4, ttf_fp);
	advance_Width = USHORT_L(buff);
	lsb = USHORT_L((buff + 2));
	if (htable.indexToLocFormat)
	{
		fseek(ttf_fp, tab[loca_index].offset + loca_offset * 4, SEEK_SET);
		fread(buff, 1, 4, ttf_fp);
		loca_offset = bigendianlong(buff);
		loca_offset += tab[glyp_index].offset;
		fseek(ttf_fp, loca_offset, SEEK_SET);
		fread(buff, 2, 5, ttf_fp);
		SHORT numberOfContours = USHORT_L(buff);
		SHORT xMin_glyph = USHORT_L((buff + 2));
		SHORT yMin_glyph = USHORT_L((buff + 4));
		SHORT xMax_glyph = USHORT_L((buff + 6));
		SHORT yMax_glyph = USHORT_L((buff + 8));
		endPtsContours = (USHORT*)malloc(sizeof(USHORT) * numberOfContours);
		if (numberOfContours <= 0)
			return -1;
		for (i = 0; i < numberOfContours; i++)
		{
			fread(buff, 2, 1, ttf_fp);
			endPtsContours[i] = USHORT_L(buff);
		}
		int pot_number = endPtsContours[numberOfContours - 1] + 1;
		fread(buff, 2, 1, ttf_fp);
		instructionLength = USHORT_L(buff);
		fread(buff, 1, instructionLength, ttf_fp);
		BYTE repeat = 0;
		flag = (BYTE*)malloc(pot_number);
		for (i = 0; i <= endPtsContours[numberOfContours - 1]; i++)
		{
			if (repeat == 0)
			{
				fread(&flag[i], 1, 1, ttf_fp);
				if (flag[i] & 0x8)
				{
					fread(&repeat, 1, 1, ttf_fp);
				}
			}
			else
			{
				flag[i] = flag[i - 1];
				repeat--;
			}
		}
		float scale = (htable.xMax - htable.xMin) > (htable.yMax - htable.yMin) ? ((htable.xMax - htable.xMin) / (font_height * 1.0)) : \
			((htable.yMax - htable.yMin) / (font_height * 1.0));
		points = (POINT*)malloc(sizeof(POINT) * pot_number);
		for (i = 0; i <= endPtsContours[numberOfContours - 1]; i++)
		{
			if (flag[i] & 0x2)  //X-Short
			{
				fread(buff, sizeof(BYTE), 1, ttf_fp);
				if (flag[i] & 0x10)  //����
				{
					if (i == 0)
					{
						if (xMin_glyph > 0)
						{
							points[i].x = (BYTE)buff[0];
						}
						else
						{
							points[i].x = (BYTE)buff[0] - xMin_glyph;
						}
					}
					else points[i].x = (BYTE)buff[0] + points[i - 1].x;
				}
				else
				{
					if (i == 0)
					{
						if (xMin_glyph > 0)
						{
							points[i].x = (BYTE)buff[0] * (-1);
						}
						else
						{
							points[i].x = (BYTE)buff[0] * (-1) - xMin_glyph;
						}
					}
					else points[i].x = (BYTE)buff[0] * (-1) + points[i - 1].x;
				}
			}
			else  // long-Short
			{
				if (i == 0)
				{
					fread(buff, sizeof(SHORT), 1, ttf_fp);
					if (xMin_glyph > 0)
					{
						points[i].x = (SHORT)USHORT_L(buff);
					}
					else
					{
						points[i].x = (SHORT)USHORT_L(buff) - xMin_glyph;
					}
				}
				else if (flag[i] & 0x10)  //�Ƿ���ͬ
				{
					points[i].x = points[i - 1].x;
				}
				else
				{
					fread(buff, sizeof(SHORT), 1, ttf_fp);
					points[i].x = (SHORT)USHORT_L(buff) + points[i - 1].x;
				}
			}
		}
		for (i = 0; i <= endPtsContours[numberOfContours - 1]; i++)
		{
			if (flag[i] & 0x4)  //X-Short
			{
				fread(buff, sizeof(BYTE), 1, ttf_fp);
				if (flag[i] & 0x20)
				{
					if (i == 0)
					{
						if (yMin_glyph > 0)
						{
							points[i].y = (BYTE)buff[0];
						}
						else
						{
							points[i].y = (BYTE)buff[0] - yMin_glyph;
						}
						
					}
					else points[i].y = (BYTE)buff[0] + points[i - 1].y;
				}
				else
				{
					if (i == 0)
					{
						if (yMin_glyph > 0)
						{
							points[i].y = (BYTE)buff[0] * (-1);
						}
						else
						{
							points[i].y = (BYTE)buff[0] * (-1) - yMin_glyph;
						}
					}
					else points[i].y = (BYTE)buff[0] * (-1) + points[i - 1].y;
				}
			}
			else  // long-Short
			{
				if (i == 0)
				{
					fread(buff, sizeof(SHORT), 1, ttf_fp);
					//points[i].y = (SHORT)USHORT_L(buff) - yMin_glyph;
					if (yMin_glyph > 0)
					{
						points[i].y = (SHORT)USHORT_L(buff);
					}
					else
					{
						points[i].y = (SHORT)USHORT_L(buff) - yMin_glyph;
					}
				}
				else if (flag[i] & 0x20)
				{
					points[i].y = points[i - 1].y;
				}
				else
				{
					fread(buff, sizeof(SHORT), 1, ttf_fp);
					points[i].y = (SHORT)USHORT_L(buff) + points[i - 1].y;
				}
			}
		}

		for (i = 0; i <= endPtsContours[numberOfContours - 1]; i++)
		{
			points[i].y /= scale;
			points[i].x /= scale;
		}

		_Suyu_BMP Bmp;
		Init_Suyu_BMP(&Bmp, font_height, font_height);
		for (int i = 0; i < Bmp._Hight * Bmp._Width; i++)
		{
			Bmp._BMP[i] = 0;
		}
		draw_fontglyf(&Bmp, numberOfContours, endPtsContours, flag, points);
		for (int i = 0; i < Bmp._Hight; i++)
		{
			for (int j = 0; j < Bmp._Width; j++)
			{
				if (BMP_GetColor(Bmp, j, i))
				{
					unsigned char r = (img->_BMP[(y + i) * img->_Width + x + j] & 0x0000FF);
					unsigned char g = (img->_BMP[(y + i) * img->_Width + x + j] & 0x00FF00) >> 8;
					unsigned char b = (img->_BMP[(y + i) * img->_Width + x + j] & 0xFF0000) >> 16;
					if (r < (255 - 18))
						r += 18;
					else
						r = 255;
					if (g < (255 - 18))
						g += 18;
					else
						g = 255;
					if (b < (255 - 18))
						b += 18;
					else
						b = 255;
					BMP_draw_point(img, x + j, y + i, r | (g << 8) | (b << 16));
				}
			}
		}
		free_Suyu_BMP(&Bmp);
		free(flag);
		free(endPtsContours);
		free(points);
	}
}

void show_string(_Suyu_BMP* img, char* filename, int fontHeight, int x, int y, wchar_t* str)
{
	while (*str)
	{
		show_char(img, x, y, fontHeight, filename, *str);
		if (*str <= 127)
			x += fontHeight / 2;
		else
			x += fontHeight;
		str++;
	}
}


void drawBezierCurve(_Suyu_BMP *img,POINT p0, POINT p1, POINT p2, unsigned int color)
{
	float delat = 0.001;
	float i, u;

	for (i = 0.0; i <= 1.0; i += delat)
	{
		u = 1 - i;
		BMP_draw_point(img, i * i * p0.x / 4 + i * u * p1.x / 2 + u * u * p2.x / 4, i * i * p0.y / 4 + i * u * p1.y / 2 + u * u * p2.y / 4, color);
	}
}

ULONG Amn(ULONG m, ULONG n) 
{
	ULONG res = 1, i;
	for (i = 0; i < m; i++)
	{
		res *= (n - i);
	}
	return res;
}

ULONG factorial(ULONG m)
{

	if (m == 1) return 1;
	else return m * factorial(m - 1);
}

ULONG Cmn(ULONG m, ULONG n)
{
	if (n < 1) return 1;
	if (m == 0) m = n;
	return Amn(m, n) / factorial(m);
}

unsigned long font_fill_len = 0;
POINT font_points[65535 * 10];

void join(double x, double y)
{
	font_points[font_fill_len].x = x;
	font_points[font_fill_len].y = y;
	font_fill_len++;
}

void drawBezierN(_Suyu_BMP *img,POINT* ps, int n, unsigned int color)
{
	float delat = 0.0001; //step
	float i, u;
	double x, y, temp;
	int num;
	if (n < 1) return;
	for (num = 0; num <= n; num++)
	{
		//setfillcolor(0x007F7FFF);
		//setcolor(0x007F7FFF);
		//fillcircle(ps[num].x / Scale_entry, ps[num].y / Scale_entry, 1);
	}
	for (i = 0.0; i <= 1.0; i += delat)
	{
		u = 1 - i;
		x = 0;
		for (num = 0; num <= n; num++)
		{
			temp = 1;
			temp *= Cmn(num, n);
			temp *= ps[num].x / Scale_entry;
			temp *= pow(i, num);
			temp *= pow(u, (n - num));
			x += temp;
		}
		y = 0;
		for (num = 0; num <= n; num++)
		{
			temp = 1;
			temp *= Cmn(num, n);
			temp *= ps[num].y / Scale_entry;
			temp *= pow(i, num);
			temp *= pow(u, (n - num));
			y += temp;
		}
		BMP_draw_point(img, x, y, color);
		//join(x, y);
	}
}

void draw_Polygons(_Suyu_BMP *img, unsigned int color)
{
	struct toppoint             
	{
		int ymin;               
		int ymax;               
		int xmin;               
		char flag;              
		float slope;            
	};
	int y;
	int num = 0, min, max, i, j;
	struct toppoint* ET = (struct toppoint*)malloc(font_fill_len * sizeof(struct toppoint));  
	int* xstack = (int*)malloc((font_fill_len + 1) * sizeof(int));  

	for (num = 0, min = font_points[0].y; num < font_fill_len; num++)
	{
		if (min > font_points[num].y) min = font_points[num].y;
	}

	for (num = 0, max = font_points[0].y; num < font_fill_len; num++)
	{
		if (max < font_points[num].y) max = font_points[num].y;
	}

	for (num = 0; num < font_fill_len; num++)
	{
		ET[num].flag = 0; 
		ET[num].ymin = (font_points[num].y > font_points[(num + 1) % font_fill_len].y ? font_points[(num + 1) % font_fill_len].y : font_points[num].y);  
		ET[num].ymax = (font_points[num].y < font_points[(num + 1) % font_fill_len].y ? font_points[(num + 1) % font_fill_len].y : font_points[num].y);  
		ET[num].xmin = (font_points[num].y > font_points[(num + 1) % font_fill_len].y ? font_points[(num + 1) % font_fill_len].x : font_points[num].x);
		ET[num].slope = (((float)(font_points[(num + 1) % font_fill_len].y - font_points[num].y) == 0) ? 0 : (float)(font_points[(num + 1) % font_fill_len].x - font_points[num].x) / (float)(font_points[(num + 1) % font_fill_len].y - font_points[num].y));

	}

	//if (getpixel(font_points[0].x, font_points[0].y) == RGB(0, 0, 0)) setlinecolor(RGB(255, 255, 255));
	//else setlinecolor(RGB(0, 0, 0));
	//setlinecolor(color);
	/*for (i = 0; i < font_fill_len; i++)
	{
		putpixel(font_points[i].x, font_points[i].y, RGB(255, 0, 0));
	}*/

	for (y = min; y <= max; y++)
	{
		i = 1;
		for (num = 0; num < font_fill_len; num++)   
		{
			if (ET[num].flag == 0 && y > ET[num].ymin) 
			{
				if (ET[num].ymax < y)  
				{
					ET[num].flag = 1; 
				}
				else
				{
					xstack[i++] = ET[num].xmin + ET[num].slope * (y - ET[num].ymin);
				}
			}
		}
		xstack[0] = i - 1;
		for (i = 1; i < xstack[0]; i++)
		{
			for (j = 1; j < xstack[0] + 1 - i; j++)
			{
				if (xstack[j] > xstack[j + 1])
				{
					num = xstack[j];
					xstack[j] = xstack[j + 1];
					xstack[j + 1] = num;
				}
			}
		}
		for (num = 1; num <= xstack[0]; num += 2)
		{
			if (BMP_GetColor(*img,(xstack[num] + xstack[num + 1]) / 2, img->_Hight - y) != 0x00000000)
			{
				draw_line_BMP(img, xstack[num], img->_Hight - y, xstack[num + 1], img->_Hight - y, 0x00000000);
			}
			else
			{
				draw_line_BMP(img, xstack[num], img->_Hight - y, xstack[num + 1], img->_Hight - y, color);
			}
		}
	}
	font_fill_len = 0;

}

void memcpy_suyu(void* dst, void* src, size_t len)
{
	BYTE* tdst = (BYTE*)dst;
	BYTE* tsrc = (BYTE*)src;
	while (len)
	{
		*(tdst++) = *(tsrc++);
		len--;
	}
}

void draw_fontglyf(_Suyu_BMP *img, USHORT numberContours, USHORT* endPtrsContours, BYTE* flag, POINT* points)
{
	int i = 0, j = 0, k = 0, len = 0;
	BYTE* temp_flag = flag;
	POINT* temp_point = points, * point_temp;
	k = endPtrsContours[0];
	for (i = 0; i < numberContours; i++)
	{
		font_fill_len = 0;
		for (j = 0; j <= k;)
		{
			len = 0;
			do
			{
				len++;
			} while (((temp_flag[j + len] & 0x01) == 0) && (j + len) <= k); 
			if ((j + len) > k && ((temp_flag[j + len] & 0x01) == 0))   
			{
				len--;
				point_temp = (POINT*)malloc(sizeof(POINT) * (len + 2));
				memcpy_suyu(point_temp, &temp_point[j], sizeof(POINT) * (len + 1));
				point_temp[len + 1] = temp_point[0];
				len++;  
				//drawBezierN(point_temp, len, RGB(255, 0, 0));
				memcpy_suyu(font_points + font_fill_len, point_temp, sizeof(POINT) * (len));
				font_fill_len += len;
				free(point_temp);
			}
			else if ((j + len) > k && ((temp_flag[j + len] & 0x01) == 1))
			{
				//len--;
				point_temp = (POINT*)malloc(sizeof(POINT) * (len + 1));
				memcpy_suyu(point_temp, &temp_point[j], sizeof(POINT) * (len));
				point_temp[len] = temp_point[0];
				memcpy_suyu(font_points + font_fill_len, point_temp, sizeof(POINT) * (len));
				font_fill_len += len;
				//drawBezierN(point_temp, len, RGB(255, 255, 255));
				free(point_temp);
			}
			else
			{

				//memcpy(font_points, &temp_point[j], sizeof(POINT) * (len));
				//drawBezierN(&temp_point[j], len, RGB(255, 255, 255));
				memcpy_suyu(font_points + font_fill_len, &temp_point[j], sizeof(POINT) * (len));
				font_fill_len += len;
			}
			j += len;
		}

		draw_Polygons(img, 0x00FFFFFF);
		/*if((temp_flag[0]&0x40) == 0x40)
			draw_Polygons(RGB(0, 0, 0));
		else draw_Polygons(RGB(255, 255, 255));*/
		temp_flag = flag + endPtrsContours[i] + 1;
		temp_point = points + endPtrsContours[i] + 1;
		if (i < numberContours - 1) k = endPtrsContours[i + 1] - endPtrsContours[i] - 1;
	}
}
