#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "picture.h"

#pragma pack (1)

typedef  struct
{
	unsigned int biSize;
	long  biWidth;
	long  biHeight;
	unsigned short  biPlanes;
	unsigned short  biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	long  biXPelsPerMeter;
	long  biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
}BITMAPINFOHEADER1;

typedef  struct
{
	unsigned short  bfType;
	unsigned int  bfSize;
	unsigned short  bfReserved1;
	unsigned short  bfReserved2;
	unsigned int  bfOffBits;
}BITMAPFILEHEADER1;

typedef  struct
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
}RGBQUAD1;

typedef struct
{
	BITMAPFILEHEADER1 bmfHeader;
	BITMAPINFOHEADER1 bmiHeader;
	unsigned int RGB_MASK[3];

}BITMAPINFO1;

#pragma pack (pop)

typedef RGBQUAD1* LPRGBQUAD1;

#define BI_RGB	 		0  
#define BI_RLE8 		1  
#define BI_RLE4 		2  
#define BI_BITFIELDS 	3   

void Init_Suyu_BMP(_Suyu_BMP* img, int width, int height)
{
	if (height < 0 || width < 0)
	{
		img->sflag = _S_PERR;
	}

	img->_Hight = height;
	img->_Width = width;
	
	img->_BMP = (unsigned int*)malloc(sizeof(unsigned int) * height * width);
	if (img->_BMP)
	{
		img->sflag = _S_OK;
	}
	else
	{
		img->sflag = _S_MERR;
	}
}

void free_Suyu_BMP(_Suyu_BMP* BMP)
{
	if (BMP->_BMP)
		free(BMP->_BMP);
	BMP->sflag = _S_MERR;
	BMP->_Hight = BMP->_Width = 0;
}

void BMP_draw_point(_Suyu_BMP* img, int x, int y, unsigned int color)
{
	if ((x > img->_Width) || (x < 0)) return;
	if ((y > img->_Hight) || (y < 0)) return;
	*(img->_BMP + x + y * img->_Width) = (unsigned int)color;
}

unsigned int BMP_GetColor(_Suyu_BMP img, int x, int y)
{
	return img._BMP[x + y * img._Width];
}

int BMP_Decode(_Suyu_BMP* img, char* filename)
{
	FILE* fp;
	int buffsize = 2048;
	unsigned char* databuf,* bmpbuff;
	long count;
	unsigned char rgb = 0, color_byte;
	int img_width, img_height;
	int rowlen = 0, countpixel = 0;
	int y = 0, x = 0;
	unsigned char color_r, color_g, color_b;
	int real_x = 0, real_y = 0;
	int last_x = 0, last_y = 0;
	float inc_x = 0.0, inc_y = 0.0;
	int i_x, i_y;
	BITMAPINFO1* pbmp;

	// 解码
	fopen_s(&fp, filename, "rb");
	if (!fp)
	{
		img->sflag = _S_PERR;
		return -1;
	}

	databuf = (unsigned char*)malloc(buffsize * sizeof(unsigned char));
	fread_s(databuf, buffsize, sizeof(unsigned char), buffsize, fp);
	pbmp = (BITMAPINFO1*)databuf;
	count = pbmp->bmfHeader.bfOffBits;
	color_byte = pbmp->bmiHeader.biBitCount / 8;
	img_width = pbmp->bmiHeader.biWidth;
	img_height = pbmp->bmiHeader.biHeight;

	inc_x = 1; // (float)(img->_Width) / (float)(img_width);
	inc_y = 1; // (float)(img->_Hight) / (float)(img_height);
	if ((img_width * color_byte) % 4 == 0) 
		rowlen = img_width * color_byte;
	else
		rowlen = ((img_width * color_byte) / 4 + 1) * 4;

	real_x = 0;
	real_y = img->_Hight - 1;
	y = img_height - 1;
	countpixel = 0;
	bmpbuff = databuf;
	while (1)
	{
		while (count < buffsize)
		{
			if (color_byte == 3)
			{
				switch (rgb)
				{
				case 0: color_b = (unsigned char)bmpbuff[count]; break;
				case 1: color_g = (unsigned char)bmpbuff[count]; break;
				case 2: color_r = (unsigned char)bmpbuff[count]; break;
				}
			}
			rgb++;
			count++;
			if (rgb == color_byte)
			{
				if (x <= img_width)
				{
					last_x = real_x;
					real_x = (int)(x * inc_x);
					for (i_y = last_y; i_y >= real_y; i_y--)
					{
						for (i_x = last_x; i_x <= real_x; i_x++)
						{
							BMP_draw_point(img, i_x, i_y, color_r | (color_g << 8) | (color_b << 16));
						}
					}
					x++;
					color_r = 0;
					color_g = 0; 
					color_b = 0;
					rgb = 0;
				}
			}
			countpixel++;
			if (countpixel >= rowlen)
			{
				if (y == 0 && (x == img_width)) break;
				y--;
				last_y = real_y;
				real_y = (int)(y * inc_y);
				countpixel = 0;
				x = 0;
				color_r = 0;
				color_g = 0;
				color_b = 0;
				rgb = 0;
			}
		}
		fread(databuf, 1, buffsize, fp);
		if (y == 0 && (x == img_width)) break;
		count = 0;
	}
	return 0;
}

void BMP_Encode(char* filename, _Suyu_BMP* img)
{
	FILE* fp;
	unsigned int bmpheadsize;
	unsigned int i;
	unsigned char* buf;
	BITMAPINFO1 hbmp;

	unsigned char res = 0;
	unsigned char* databuff;
	unsigned int pixcnt;
	unsigned int bi4width;
	unsigned int tx, ty;
	unsigned int buffsize = (img->_Width / 1024 + 1) * 1024;
	databuff = (unsigned char*)malloc(buffsize * 3);

	if (databuff == NULL)
	{
		return;
	}

	bmpheadsize = sizeof(hbmp);
	buf = (unsigned char*)&hbmp;
	memset(buf, 0, bmpheadsize);

	hbmp.bmiHeader.biSize = sizeof(BITMAPINFOHEADER1);
	hbmp.bmiHeader.biWidth = img->_Width;
	hbmp.bmiHeader.biHeight = img->_Hight;
	hbmp.bmiHeader.biPlanes = 1;
	hbmp.bmiHeader.biBitCount = 24;
	hbmp.bmiHeader.biCompression = BI_RGB;
	hbmp.bmiHeader.biSizeImage = hbmp.bmiHeader.biHeight * hbmp.bmiHeader.biWidth * hbmp.bmiHeader.biBitCount / 8;

	hbmp.bmfHeader.bfType = (((unsigned short)'M' << 8) + 'B');
	hbmp.bmfHeader.bfSize = bmpheadsize + hbmp.bmiHeader.biSizeImage;
	hbmp.bmfHeader.bfOffBits = bmpheadsize;

	hbmp.RGB_MASK[0] = 0x00FF0000;  //��ɫ����;
	hbmp.RGB_MASK[1] = 0x0000FF00;  //��ɫ���룻
	hbmp.RGB_MASK[2] = 0x000000FF;  //��ɫ���룻

	fopen_s(&fp,(char*)filename, "wb+");
	if (fp == NULL)
	{
		free(databuff);
		return;
	}

	if ((hbmp.bmiHeader.biWidth * 3) % 4)
	{
		bi4width = ((hbmp.bmiHeader.biWidth * 3) / 4 + 1) * 4;
	}
	else bi4width = hbmp.bmiHeader.biWidth * 3;

	fwrite((unsigned char*)&hbmp, bmpheadsize, 1, fp);

	for (ty = img->_Hight - 1; hbmp.bmiHeader.biHeight; ty--)
	{
		pixcnt = 0;
		for (tx = 0; pixcnt != (bi4width / 3);)
		{
			if (pixcnt < hbmp.bmiHeader.biWidth)
			{
				databuff[pixcnt * 3 + 2] = (unsigned char)(img->_BMP[tx + ty * img->_Width] & 0x000000FF);
				databuff[pixcnt * 3 + 1] = (unsigned char)((img->_BMP[tx + ty * img->_Width] & 0x0000FF00) >> 8);
				databuff[pixcnt * 3] = (unsigned char)((img->_BMP[tx + ty * img->_Width] & 0x00FF0000) >> 16);
			}
			else
			{
				databuff[pixcnt * 3] = 0xFF;
				databuff[pixcnt * 3 + 1] = 0XFF;
				databuff[pixcnt * 3 + 2] = 0XFF;
			}
			pixcnt++;
			tx++;
		}
		hbmp.bmiHeader.biHeight--;
		fwrite((unsigned char*)databuff, bi4width, 1, fp);
	}

	fclose(fp);
	free(databuff);

}

void draw_line_BMP(_Suyu_BMP *img, int x1, int y1, int x2, int y2, unsigned int color)
{
	unsigned short t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	unsigned char color_r, color_g, color_b;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1;
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	if (delta_x > 0)incx = 1;
	else if (delta_x == 0)incx = 0;
	else { incx = -1; delta_x = -delta_x; }
	if (delta_y > 0)incy = 1;
	else if (delta_y == 0)incy = 0;
	else { incy = -1; delta_y = -delta_y; }
	if (delta_x > delta_y)distance = delta_x;
	else distance = delta_y;
	for (t = 0; t <= distance + 1; t++)
	{
		//color_r = (u8)(BMP[uRow + uCol * Screen_Width] & 0x000000FF);
		//color_r += (u8)(color & 0x000000FF);
		//if (color_r <= (u8)(color & 0x000000FF)) color_r = 0xFF;
		//color_g = (u8)((BMP[uRow + uCol * Screen_Width] & 0x0000FF00) >> 8);
		//color_g += (u8)((color & 0x0000FF00) >> 8);
		//if (color_g <= (u8)((color & 0x0000FF00) >> 8)) color_g = 0xFF;
		//color_b = (u8)((BMP[uRow + uCol * Screen_Width] & 0x00FF0000) >> 16);
		//color_b += (u8)((color & 0x00FF0000) >> 16);
		//if (color_b <= (u8)((color & 0x00FF0000) >> 16)) color_b = 0xFF;
		BMP_draw_point(img, uRow, uCol, /*color_r | (color_g << 8) | (color_b << 16)*/color);
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}