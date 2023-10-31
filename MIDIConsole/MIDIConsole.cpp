#include <iostream>
#include <cstring>
#include <cstdlib>

#include <Windows.h>
#include <mmeapi.h>

#include "parseString.h"
#include "midi.h"
#include "interface.h"
using namespace std;

void fixWindowShape()
{
	HWND hWnd = GetConsoleWindow(); //获得cmd窗口句柄
	RECT rc;
	GetWindowRect(hWnd, &rc); //获得cmd窗口对应矩形

	//改变cmd窗口风格
	SetWindowLongPtr(hWnd,
		GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX);
	SetWindowPos(hWnd,
		NULL,
		rc.left,rc.top,
		rc.right - rc.left, rc.bottom - rc.top,
		NULL);
}

void setCursorState(bool status)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle, &CursorInfo);//获取控制台光标信息
	CursorInfo.bVisible = status; //隐藏控制台光标
	SetConsoleCursorInfo(handle, &CursorInfo);//设置控制台光标状态
}

int main()
{
	fixWindowShape();
	setCursorState(false);
	HMIDIOUT hMidi;
	midiOutOpen(&hMidi, 0, 0, 0, 0);
	instrument::setIns(hMidi);
	bool keyDown[2][256];
	memset(keyDown, 0, sizeof(keyDown));//清零
	int round = 0;//20ms作为一轮，round值只为0或1，规定1-round就是之前那一轮(反复覆盖）
	while (true)
	{
		round = 1 - round;
		for (int i = 0; i < 256; i++)
		{
			if (GetAsyncKeyState(i) & 0x8000)//当前状态在最高位中
				keyDown[round][i] = true;
			else
				keyDown[round][i] = false;
		}
		for (int i = 'A'; i <= 'Z'; i++)
		{
			if (i == 'I' or i == 'K' or i == 'O' or i == 'L' or i == 'P')
				continue;
			if (keyDown[round][i] && !keyDown[1 - round][i])//刚被按下
				play::playNote(hMidi, i, true);
			if (!keyDown[round][i] && keyDown[1 - round][i])//刚被松开
				play::playNote(hMidi, i, false);
		}
		if (keyDown[round][VK_ADD] || keyDown[round][VK_OEM_PLUS])//主键盘区的加按键，控制音量用
		{
			//可单按，可长按
			int curVolume = volume::getVolume100();
			if (curVolume <= 98)
				volume::setVolume(curVolume + 2);
		}
		if (keyDown[round][VK_SUBTRACT] || keyDown[round][VK_OEM_MINUS])//主键盘区的减，同上
		{
			int curVolume = volume::getVolume100();
			if (curVolume >= 2)
				volume::setVolume(curVolume -= 2);
		}
		if (!keyDown[round][VK_OEM_2] && keyDown[1 - round][VK_OEM_2])
		{
			setCursorState(true);
			string cmd;
			getline(cin, cmd);
			auto parsedCmd = split(cmd);
			if (lower(parsedCmd[0]) == "/vol" or lower(parsedCmd[0]) == "/volume")
			{
				if (parsedCmd.size() != 2)
				{
					//clearLine();
					cout << "语法错误：参数应该为 1 个，实际接收到" << parsedCmd.size() - 1 << "个"<<endl;
				}
				if (isNum(parsedCmd[1]))
					volume::setVolume(atoi(parsedCmd[1].c_str()));
			}
		}
		Sleep(20);//可以认为是键盘读取tick间隔，减少IO开销
	}
	return 0;
}
