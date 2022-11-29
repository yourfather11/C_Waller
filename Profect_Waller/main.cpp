#include <stdio.h>
#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "picture.h"
#include "font.h"
#include <time.h>

#define SCREEN_WIDTH	3840
#define SCREEN_HEIGHT	2160

char bmp_path1[] = "C:\\Users\\15477\\Downloads\\1.BMP";
char bmp_path2[] = "C:\\Users\\15477\\Downloads\\2.BMP";

// BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinlni);

// 计算从 0001-1-1 起的天数
int countdays(int y, int m, int d)
{
	if (m < 3)	y--, m += 12;
	return 365 * y + (y >> 2) - y / 100 + y / 400 + (153 * m - 457) / 5 + d - 306;
}

/** 新建文件error.txt
*** 将错误的信息打印在error.txt
***
**/
/* 相识纪念日 */
int nyear = 2022;
int nmonth = 5;
int ndate = 3;

void vMainUpdate(void);

int main(int argc, char** argv)
{
	time_t now_time;
	now_time = time(NULL);
	tm now_tm;
	localtime_s(&now_tm, &now_time);
	int old_day = now_tm.tm_mday;
	vMainUpdate();

	int time_file = 0;
	long modify_time = 0;
	long tmp_time = 0;
	FILE* fp;
	struct stat buf;
	fopen_s(&fp, "C:\\Users\\15477\\Downloads\\config.txt", "r");
	if (fp != NULL)
	{
		int fd = _fileno(fp);
		fstat(fd, &buf);
		modify_time = buf.st_mtime;
		fclose(fp);
	}
	tmp_time = modify_time;
	while (1)
	{
		Sleep(1000);
		now_time = time(NULL);  // 获取当前时间 判断是否要更新
		// 判断是不是新的一天 如果是的话进行更新
		localtime_s(&now_tm, &now_time);
		if (old_day != now_tm.tm_mday)
		{
			// 更新壁纸
			vMainUpdate();
		}

		/* 5分钟判断一次文件是否更新 */
		time_file++;
		
		if (time_file > 300)
		{
			time_file = 0;
			fopen_s(&fp, "C:\\Users\\15477\\Downloads\\config.txt", "r");
			if (fp != NULL)
			{
				int fd = _fileno(fp);
				fstat(fd, &buf);
				tmp_time = buf.st_mtime;
				fclose(fp);
			}
		}
		if (tmp_time != modify_time)
		{
			// 更新壁纸
			vMainUpdate();
		}
		// 文件更新了
	}

	return 0;
}

void vMainSpecialDay(_Suyu_BMP* bmp);
void vMainCountDownDay(_Suyu_BMP* bmp);

void vMainUpdate(void)
{
	_Suyu_BMP suyu_img;
	Init_Suyu_BMP(&suyu_img, SCREEN_WIDTH, SCREEN_HEIGHT);
	BMP_Decode(&suyu_img, bmp_path1);
	
	vMainSpecialDay(&suyu_img);
	vMainCountDownDay(&suyu_img);
	// 编码
	BMP_Encode(bmp_path2, &suyu_img);
	free_Suyu_BMP(&suyu_img);
	SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, bmp_path2, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);  // 切换壁纸
}

void vMainSpecialDay(_Suyu_BMP* bmp)
{
	FILE* fp;
	char tmp_buf[512];
	wchar_t target[256];
	wchar_t mstr[256];
	char section[] = "[Special Day]";

	fopen_s(&fp, "C:\\Users\\15477\\Downloads\\config.txt", "r");
	int x = 0;
	int y = 0;
	/* 文件不能打开 */
	if (fp == NULL)
	{
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fgets(tmp_buf, 512, fp);
	while (!feof(fp))
	{
		if (strncmp(tmp_buf, section, sizeof(section) - 1) == 0)
		{
			break;
		}
		fgets(tmp_buf, 512, fp);
	}

	time_t now_time = time(NULL);
	tm now_tm;
	localtime_s(&now_tm, &now_time);

	int tyear, tmonth, tday;

	fgets(tmp_buf, 512, fp);
	while (!feof(fp) && (tmp_buf[0] != '\n' && tmp_buf[0] != ' '))
	{
		sscanf_s(tmp_buf, "%d-%d-%d ", &tyear, &tmonth, &tday);
		int length = strlen(tmp_buf);
		tmp_buf[length - 1] = 0;
		int cdays = countdays(now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday) - countdays(tyear, tmonth, tday);
		char* index = strstr(tmp_buf, "#");
		index++;
		int size = MultiByteToWideChar(CP_UTF8, 0, index, -1, NULL, 0);
		MultiByteToWideChar(CP_UTF8, 0, index, -1, target, size);

		wsprintf(mstr, target, cdays);

		int xlen = 0;
		int font_height = 100;
		int i = 0;
		while (*(mstr + i))
		{
			if (*(mstr + i) < 128)
			{
				xlen += font_height / 2;
			}
			else
			{
				xlen += font_height;
			}
			i++;
		}

		show_string(bmp, (char*)"C:\\Windows\\Fonts\\simkai.ttf", font_height, SCREEN_WIDTH - xlen, y, mstr);

		y += font_height;

		fgets(tmp_buf, 512, fp);
	}

	fclose(fp);
}

void vMainCountDownDay(_Suyu_BMP* bmp)
{
	FILE* fp;
	char tmp_buf[512];
	wchar_t target[256];
	wchar_t mstr[256];
	char section[] = "[Coundown]";

	fopen_s(&fp, "C:\\Users\\15477\\Downloads\\config.txt", "r");
	int x = 0;
	int y = 0;
	/* 文件不能打开 */
	if (fp == NULL)
	{
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fgets(tmp_buf, 512, fp);
	while (!feof(fp))
	{
		if (strncmp(tmp_buf, section, sizeof(section) - 1) == 0)
		{
			break;
		}
		fgets(tmp_buf, 512, fp);
	}

	time_t now_time = time(NULL);
	tm now_tm;
	localtime_s(&now_tm, &now_time);

	int tyear, tmonth, tday;

	fgets(tmp_buf, 512, fp);
	while (!feof(fp) && (tmp_buf[0] != '\n' && tmp_buf[0] != ' '))
	{
		sscanf_s(tmp_buf, "%d-%d-%d ", &tyear, &tmonth, &tday);
		int length = strlen(tmp_buf);
		tmp_buf[length - 1] = 0;
		int cdays = countdays(tyear, tmonth, tday) - countdays(now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
		char* index = strstr(tmp_buf, "#");
		index++;
		int size = MultiByteToWideChar(CP_UTF8, 0, index, -1, NULL, 0);
		MultiByteToWideChar(CP_UTF8, 0, index, -1, target, size);

		wsprintf(mstr, target, cdays);

		int xlen = 0;
		int font_height = 100;
		int i = 0;
		while (*(mstr + i))
		{
			if (*(mstr + i) < 128)
			{
				xlen += font_height / 2;
			}
			else
			{
				xlen += font_height;
			}
			i++;
		}

		show_string(bmp, (char*)"C:\\Windows\\Fonts\\simkai.ttf", font_height, 0, y, mstr);

		y += font_height;

		fgets(tmp_buf, 512, fp);
	}

	fclose(fp);
}
