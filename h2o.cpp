// CONSOLE
#include <stdp>
#include <txtp>
#include <cmath>

#pragma comment( lib, "user32")

#define SET_TSTART   0
#define SET_TEND     1
#define SET_SHOTML   2
#define SET_H2OGOAL  3
#define SET_CONSUMED 4

#define TIME(h, m, s) (((int)h << 16) | ((int)m << 8) | (int)s)

UINT_PTR timer;
FLASHWINFO flash_inf;
HANDLE mutex_lock;

char h_start = 5;
char m_start = 0;
char h_end = 20;
char m_end = 0;
int shot_ml = 80;
int h2o_goal = 2000;

int start_mins, end_mins, end_mins_grace; // Total minutes in start/end times

bool hybernation;
bool end_of_time;
bool dclick_timeout;

int h2o_consumed;
int shots_need;
int int_min, int_sec; // Interval

int min, sec; // Current Time

int decTimes(int start, int end); // OUT: raw seconds IN: TIME MACRO
void recalcInterval();
void flash(bool on_off);
void prnt(const char* t);
void pi(long long int i);
void pshots();
void pbar();
void scurt(); // Set Cursor in the position to pring time
void savest(); // Save State
void readst(); // Read State
void hideCursor();
void __declspec(nothrow) tproc(HWND p1, UINT p2, UINT_PTR p3, DWORD p4);
DWORD __declspec(nothrow) inputThread(LPVOID lp);
DWORD __declspec(nothrow) dclickTmThread(LPVOID lp);

int wmain()
{ 	
	mutex_lock = CreateMutex(NULL, FALSE, NULL);
	
	COORD buf_s;
	buf_s.X = 79;
	buf_s.Y = 20;
	SMALL_RECT wnd_pos;
	wnd_pos.Top = 0;
	wnd_pos.Left = 0;
	wnd_pos.Bottom = 19;
	wnd_pos.Right = 78;
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleWindowInfo(oh, TRUE, &wnd_pos);
	SetConsoleScreenBufferSize(oh, buf_s);
	
	SetConsoleTitle(L"H20");
	hideCursor();
	
	HWND wnd = GetConsoleWindow();
	
	flash_inf.cbSize = sizeof(FLASHWINFO);
	flash_inf.hwnd = wnd;
	flash_inf.uCount = 0;
	flash_inf.dwTimeout = 500;
	
	readst();
	recalcInterval();
	min = int_min;
	sec = int_sec;
	shots_need = 1;
	
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	int cur_mins = lt.wHour * 60 + lt.wMinute;
	start_mins = h_start * 60 + m_start;
	end_mins = h_end * 60 + m_end;
	end_mins_grace = end_mins + 10;
	
	if(cur_mins >= end_mins || cur_mins < start_mins)
	{
		hybernation = true;
		end_of_time = true;
	}
	else
	{
		flash(1);
	}
	
	timer = SetTimer(NULL, NULL, 1000, tproc);
	
	pbar();
	
	CreateThread(NULL, 0, inputThread, NULL, 0, NULL);
	
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) != 0)
	{
		DispatchMessage(&msg);
	}
	
	KillTimer(NULL, timer);
	return 0;
}

int decTimes(int start, int end)
{
	char h, m, s;
	char hs = char(start >> 16);
	char ms = char((start & 0xFF00) >> 8);
	char ss = char(start & 0xFF);
	char he = char(end >> 16);
	char me = char((end & 0xFF00) >> 8);
	char se = char(end & 0xFF);
	
	if(he <= hs)
	{
		h = char(24 - hs + he);
	}
	else
	{
		h = char(he - hs);
	}
	m = char(me - ms);
	s = char(se - ss);
	
	if(s < 0)
	{
		--m;
		s = char(60 + s);
	}
	
	if(m < 0)
	{
		--h;
		m += 60;
	}
	else if(h == 24)
	{
		h = 0;
	}
	
	return h * 3600 + m * 60 + s;
}

void recalcInterval()
{	
	if(end_of_time)
	{
		return;
	}
	
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	
	int itot = decTimes(TIME(h_start, m_start, 0), TIME(h_end, m_end, 0));
	int ielasp = decTimes(TIME(h_start, m_start, 0), TIME(lt.wHour, lt.wMinute, lt.wSecond));
	
	int interval = (int)round(float(itot-ielasp)/floor(float(h2o_goal - h2o_consumed)/(float)shot_ml - 1.0f));
	interval = interval <= 0 ? 1 : interval;
	int_min = interval/60;
	int_sec = interval % 60;

/* 	INTERVAL RECALC DISPLAY
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD xy;
	static short a = 4;
	static short b = 27;
	xy.X = b;
	xy.Y = a++;
	if(a >= 18)
	{
		a = 4;
		b += 6;
		xy.X = b;
		xy.Y = a;
	}
	SetConsoleCursorPosition(oh, xy);
	pi(int_min);
	prnt(":");
	pi(int_sec);
	prnt("    "); */
}

void flash(bool on_off)
{
	if(hybernation)
	{
		return;
	}
	
	if(on_off)
	{
		flash_inf.dwFlags = FLASHW_TRAY | FLASHW_TIMER;
		FlashWindowEx(&flash_inf);
	}
	else
	{
		flash_inf.dwFlags = 0;
		FlashWindowEx(&flash_inf);
	}
}

void prnt(const char* t)
{
	long unsigned int wrt = 0;
	static HANDLE stdo = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(stdo, t, (DWORD)strlen(t), &wrt, NULL);
}

void pi(long long int i)
{
	p|i;
}

void pshots()
{
	COORD shotz;
	shotz.X = 29;
	shotz.Y = 2;
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(oh, shotz);
	pi(shots_need);
	prnt("  ");
}

void pbar()
{
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD comp;
	comp.X = 0;
	comp.Y = 0;
	
	SetConsoleCursorPosition(oh, comp);
	float pctg = (float)h2o_consumed/(float)h2o_goal * 60.0f;
	
	p|"▐";
	for(int j = 0; j < 60; ++j)
	{							
		if(j < (int)pctg)
		{
			p|BD|"█"|D;
		}
		else
		{
			p|"░";
		}
	}
	p|"▌ ";
	
	p|(int)(pctg/60.0f * 100.0f)|"% "|h2o_consumed|'/'|h2o_goal;
	prnt("      ");
	scurt();
	if(hybernation)
	{
		p|"zZ:zZ";
	}
	else
	{
		p|SP(2, '0')|int_min|DP|':'|SP(2, '0')|int_sec|DP;
	}
	pshots();
}

void scurt()
{
	static COORD xy = { 27 , 1 };
	//static COORD clear = { 25 , 1 };
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleCursorPosition(oh, clear);
	//p|"  ";
	SetConsoleCursorPosition(oh, xy);
}

void savest()
{
	// Create or open File or Device =================================================================
	HANDLE fh = CreateFile(
		L"h2o.db",					//   [I]  Name of the file or device to create/open
		GENERIC_WRITE,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_READ,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_ALWAYS,				//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,	//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================

	if(fh == INVALID_HANDLE_VALUE)
	{
		return;
	}
	
	txt data;
	data += i2t((int)h_start) + ':' + i2t(m_start) + '\n';
	data += i2t((int)h_end) + ':' + i2t(m_end) + '\n';
	data += i2t(shot_ml) + '\n';
	data += i2t(h2o_goal) + '\n';
	data += i2t(h2o_consumed) + '\n';
	
	DWORD bw;
	WriteFile(fh, data, (DWORD)~data, &bw, NULL);
	CloseHandle(fh);
}

void readst()
{
	// Create or open File or Device =================================================================
	HANDLE fh = CreateFile(
		L"h2o.db",					//   [I]  Name of the file or device to create/open
		GENERIC_READ,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_READ,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_EXISTING,				//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,	//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================

	if(fh == INVALID_HANDLE_VALUE)
	{
		return;
	}
	
	txt buff = 0x100;
	ReadFile(fh, buff, (DWORD)!buff, *buff, NULL);
	
	int setting = 0;
	txt it;
	for(ui64 i = 0; i < ~buff; ++i)
	{
		if(buff[i] == ':')
		{
			if(setting == SET_TSTART)
			{
				h_start = (char)t2i(it);
			}
			else
			{
				h_end = (char)t2i(it);
				//scurt();
				//p|"#########|"|it|"|t2i|"|t2i(it);
			}
			
			it = 0;
			continue;
		}
		
		if(buff[i] == '\n')
		{
			if(setting == SET_TSTART)
			{
				m_start = (char)t2i(it);
			}
			else if(setting == SET_TEND)
			{
				m_end = (char)t2i(it);
			}
			else if(setting == SET_SHOTML)
			{
				shot_ml = (int)t2i(it);
			}
			else if(setting == SET_H2OGOAL)
			{
				h2o_goal = (int)t2i(it);
			}
			else if(setting == SET_CONSUMED)
			{
				h2o_consumed = (int)t2i(it);
				//scurt();
				//p|"#########|"|it|"|t2i|"|t2i(it);
			}
			
			it = 0;
			++setting;
			continue;
		}
		
		it += buff[i];
	}

	CloseHandle(fh);
}

void hideCursor()
{
	p|DC;
}

#pragma warning( suppress : 4100 )
void __declspec(nothrow) tproc(HWND p1, UINT p2, UINT_PTR p3, DWORD p4)
{
	hideCursor();
	
	SetTimer(NULL, timer, 1000, tproc);
	
	// CRITICAL SECTION
	WaitForSingleObject(mutex_lock, INFINITE);
	--sec;
	if(sec < 0)
	{
		sec = 59;
		--min;
	}
	ReleaseMutex(mutex_lock);
	// CRITICAL SECTION
	
	static SYSTEMTIME lt;
	GetLocalTime(&lt);
	int cur_mins = lt.wHour * 60 + lt.wMinute;
	
	if(lt.wHour == h_end && lt.wMinute == m_end)
	{
		end_of_time = true;
	}
	else if(cur_mins >= end_mins_grace)
	{
		flash(0);
		hybernation = true;
		end_of_time = true;
	}
	else if(end_of_time && cur_mins >= start_mins)
	{
		hybernation = false;
		end_of_time = false;
		flash(1);
		recalcInterval();
		// CRITICAL SECTION
		WaitForSingleObject(mutex_lock, INFINITE);
		min = int_min;
		sec = int_sec;
		shots_need = 1;
		h2o_consumed = 0;
		ReleaseMutex(mutex_lock);
		// CRITICAL SECTION
		pbar();
		return;
	}
	
	txtp tp;
	if(hybernation)
	{
		static int cnt;
		
		switch(cnt)
		{
			case 0:
				tp|"     ";
				break;
			case 1:
				tp|"z";
				break;
			case 2:
				tp|"zZ";
				break;
			case 3:
				tp|"zZ:";
				break;
			case 4:
				tp|"zZ:z";
				break;
			case 5:
				tp|"zZ:zZ";
				break;
			default:
				break;
		}
		
		cnt = ++cnt >= 6 ? 0 : cnt;
	}
	else
	{
		tp|SP(2, '0')|min|DP|':'|SP(2, '0')|sec|DP;
	}
	
	// CRITICAL SECTION
	WaitForSingleObject(mutex_lock, INFINITE);
	
	scurt();
	p|!tp;
	
	ReleaseMutex(mutex_lock);
	// CRITICAL SECTION
	
	if(!min && !sec)
	{
		if(h2o_consumed + shots_need * shot_ml < h2o_goal && shots_need <= 0)
		{
			// CRITICAL SECTION
			WaitForSingleObject(mutex_lock, INFINITE);
			++shots_need;
			ReleaseMutex(mutex_lock);
			// CRITICAL SECTION
		}
		
		// CRITICAL SECTION
		WaitForSingleObject(mutex_lock, INFINITE);
		min = int_min;
		sec = int_sec;
		ReleaseMutex(mutex_lock);
		// CRITICAL SECTION
		pshots();
		
		if(shots_need > 0)
		{
			flash(1);
		}
	}
	
	if(shots_need > 0)
	{
		int old_int_min = int_min;
		int old_int_sec = int_sec;
		recalcInterval();
		
		// CRITICAL SECTION
		WaitForSingleObject(mutex_lock, INFINITE);
		min -= old_int_min - int_min;
		sec -= old_int_sec - int_sec;
		if(sec < 0)
		{
			sec = 60 + sec;
			--min;
		}
		else if(sec > 59)
		{
			sec -= 60;
			++min;
		}
		ReleaseMutex(mutex_lock);
		// CRITICAL SECTION
	}
	
	hideCursor();
}

#pragma warning( suppress : 4100 )
DWORD __declspec(nothrow) inputThread(LPVOID lp)
{
	static HANDLE ih = GetStdHandle(STD_INPUT_HANDLE);
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	HWND wnd = GetConsoleWindow();
	COORD comp;
	comp.X = 0;
	comp.Y = 0;

    SetConsoleMode(ih, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);
	
	INPUT_RECORD ir[128];
	DWORD nread;
	while(1)
	{
		ReadConsoleInput(ih, ir, 128, &nread);
		for(DWORD i = 0; i < nread; ++i)
		{
			if(ir[i].EventType == MOUSE_EVENT)
			{
				const MOUSE_EVENT_RECORD* m = &ir[i].Event.MouseEvent;
				if(m->dwButtonState == 0x1 && !dclick_timeout)
				{
					// MULTI-CLICK PROTECTION
					dclick_timeout = true;
					CreateThread(NULL, 0, dclickTmThread, NULL, 0, NULL);					
					
					if(h2o_consumed < h2o_goal)
					{
						// CRITICAL SECTION
						WaitForSingleObject(mutex_lock, INFINITE);
						--shots_need;
						h2o_consumed += shot_ml;
						ReleaseMutex(mutex_lock);
						// CRITICAL SECTION
						
						if(shots_need < 0)
						{
							recalcInterval();
							// CRITICAL SECTION
							WaitForSingleObject(mutex_lock, INFINITE);
							min = int_min;
							sec = int_sec;
							shots_need = 0;
							scurt();
							p|SP(2, '0')|min|DP|':'|SP(2, '0')|sec|DP;
							ReleaseMutex(mutex_lock);
							// CRITICAL SECTION
						}
						
						if(h2o_consumed >= h2o_goal)
						{
							h2o_consumed = h2o_goal;
						}
						
						if(shots_need <= 0)
						{
							flash(0);
						}
						
						float pctg = (float)h2o_consumed/(float)h2o_goal * 60.0f;
						
						// CRITICAL SECTION
						WaitForSingleObject(mutex_lock, INFINITE);
						
						SetConsoleCursorPosition(oh, comp);
						p|"▐";
						for(int j = 0; j < 60; ++j)
						{							
							if(j < (int)pctg)
							{
								p|BD|"█"|D;
							}
							else
							{
								p|"░";
							}
						}
						p|"▌ ";
						
						p|(int)(pctg/60.0f * 100.0f)|"% "|h2o_consumed|'/'|h2o_goal;
						
						pshots();
						
						ReleaseMutex(mutex_lock);
						// CRITICAL SECTION
						
						savest();
					}
				}
				else if(m->dwButtonState == 0x2)
				{
					hideCursor();
					ShowWindow(wnd, SW_MINIMIZE);
					hideCursor();
				}
			}
		}
	}
	
	p|EC;
	return 0;
}

#pragma warning( suppress : 4100 )
DWORD __declspec(nothrow) dclickTmThread(LPVOID lp)
{
	Sleep(500);
	dclick_timeout = false;
	return 0;
}



// IMPROVEMENTS DONE:
// UNICODE PROGRESS BAR
// TEST IF NOT MISSING 1 SECOND BETWEEEN PERIODZ OR MB OVERCOUNTIN 1 SEC
// MB RECALCULATE EVERY 1 MIN PERIOD IF FAILED TO COSUME
// IMPORTANT! RECAL INTERVAL ALSO WHEN CONSUMIN MORE H2O (I.E. -1/-2 ETC)
// SAVE STATE TO FILE RESTORE ON STARTUP
// OUT OF COMPUTA HYBERNATONE FAIL TO STOP HYBERNATION MODE



/* TODO NOT IMPORTANT Top half block ASCII CODE 223
 ▄▄ ▄▄  ▄▄ ▄▄
S12:33 E20:00
 ▀▀ ▀▀  ▀▀ ▀▀
Bottom half block ASCII CODE 220 ---> Change Into Full Block When hovered/pressed change to half thick full */
// REPOSITION WINDOW TO FIT TIGHTLY TO THE TASKBAR BY GETTIN IT'S WIDTH AND MOVING WINDOW!
// WHEN LAUNCHING FROM THE NPP IDE MSVC_BUILD.CMD WINDOW SIZE IS PADDED WITH 2 ADDITIONAL CELLS



// NOT POSSIBLE:
// TEST ICON AFTER FELOAD IF STILL REKT -> FIX -> ALL WINDOWS CONSOLE SLAVE WINDOWS ARE DPI UNAWARE!