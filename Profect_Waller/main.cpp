#include "main.h"

char bmp_path1[MAX_PATH];
char bmp_path2[MAX_PATH];
char user_name[128];

Dev_info dev_info;


// 获取屏幕
// GetSystemMetrics(SM_CXFULLSCREEN)
// GetSystenMetrics(SM_CXFULLSCREEN)
// 设置壁纸
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
void get_device_info(Dev_info& inf);

int main(int argc, char** argv)
{
	time_t now_time;
	now_time = time(NULL);
	tm now_tm;
	localtime_s(&now_tm, &now_time);
	int old_day = now_tm.tm_mday;
	get_device_info(dev_info);
	vMainUpdate();

	int time_file = 0;
	long modify_time = 0;
	long tmp_time = 0;
	FILE* fp;
	struct stat buf;
	fopen_s(&fp, "C:\\Users\\唐苏龙\\Downloads\\config.txt", "r");
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
			char str[MAX_PATH];
			sprintf_s(str, MAX_PATH, "C:\\Users\\%s\\Downloads\\config.txt", user_name);
			fopen_s(&fp, str, "r");
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

void get_device_info(Dev_info &inf)
{
	char tmp[MAX_PATH];
	DWORD usernamelength = MAX_PATH;
	HWND hwd = GetDesktopWindow();
	HDC hdc = GetDC(hwd);

	//inf.screen_height = GetSystemMetrics(SM_CYSCREEN);
	//inf.screen_width = GetSystemMetrics(SM_CXSCREEN);
	inf.screen_height = GetDeviceCaps(hdc, DESKTOPVERTRES);
	inf.screen_width = GetDeviceCaps(hdc, DESKTOPHORZRES);
	inf.user_dir = (char*)malloc(sizeof(char) * MAX_PATH);
	GetUserNameA(tmp, &usernamelength);
	strcpy_s(inf.user_dir,MAX_PATH, "C:/Users/");
	strcat_s(inf.user_dir,MAX_PATH - strlen(inf.user_dir), tmp);
}



void vMainSpecialDay(_Suyu_BMP* bmp);
void vMainCountDownDay(_Suyu_BMP* bmp);

void vMainUpdate(void)
{
	sprintf_s(bmp_path1,MAX_PATH, "%s/Downloads/2.bmp", dev_info.user_dir);
	sprintf_s(bmp_path2,MAX_PATH, "%s/Downloads/3.bmp", dev_info.user_dir);
	_Suyu_BMP suyu_img;
	Init_Suyu_BMP(&suyu_img, dev_info.screen_width, dev_info.screen_height);
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
	char str[MAX_PATH];
	sprintf_s(str, MAX_PATH, "%s/Downloads/config.txt", dev_info.user_dir);
	fopen_s(&fp, str, "r");
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

		show_string(bmp, (char*)"C:\\Windows\\Fonts\\simkai.ttf", font_height, dev_info.screen_width - xlen, y, mstr);

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
	char str[MAX_PATH];
	sprintf_s(str, MAX_PATH, "%s/Downloads/config.txt", dev_info.user_dir);
	fopen_s(&fp, str, "r");
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
