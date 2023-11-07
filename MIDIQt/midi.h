#pragma once

#include <map>
#include <queue>
#include <string>
#include <atomic>
#include <thread>

#include <Windows.h>
#pragma comment(lib,"winmm.lib")

namespace cmdStatus
{
	enum cmdStatus//0是停止播放，1是开始播放，2是切换通道。主要是给记录功能用的
	{
		END, BEGIN, INS
	};
}


void recordCmd(int cmd, int key, int status);
//不得已写的前置声明，为了做记录用的

class channelPool
{
	static std::atomic_int occupyChannel[2][26];
public:
	enum usage
	{
		PLAY,REPLAY
	};
	static int usingChannel(int usage);//当前整个电子琴使用的通道
	static int getChannel(int usage,int key);
	static int releaseChannel(int usage,int key);
	static int playingNumber(int usage,int key);
};

class note
{
	enum noteEnum;

	static std::map<int, int> keyMap;
	static bool isInitialed;
	static int curScale;
	static int scale[24][21];
	enum noteEnum//音高和midi数字对应表，中央C=C4=60
	{
		A0 = 21, A0S, B0,
		C1, C1S, D1, D1S, E1, F1, F1S, G1, G1S, A1, A1S, B1,
		C2, C2S, D2, D2S, E2, F2, F2S, G2, G2S, A2, A2S, B2,
		C3, C3S, D3, D3S, E3, F3, F3S, G3, G3S, A3, A3S, B3,
		C4, C4S, D4, D4S, E4, F4, F4S, G4, G4S, A4, A4S, B4,
		C5, C5S, D5, D5S, E5, F5, F5S, G5, G5S, A5, A5S, B5,
		C6, C6S, D6, D6S, E6, F6, F6S, G6, G6S, A6, A6S, B6,
		C7, C7S, D7, D7S, E7, F7, F7S, G7, G7S, A7, A7S, B7,
		C8, C8S, D8, D8S, E8, F8, F8S, G8, G8S, A8, A8S, B8,
		C9, C9S, D9, D9S, E9, F9, F9S, G9, G9S, A9, A9S, B9,
		C10, C10S, D10, D10S, E10, F10, F10S, G10
	};
	static void initScale();
public:
	enum scale//各大小调的枚举
	{
		MJ_C, MJ_CS, MJ_D, MJ_DS, MJ_E, MJ_F,
		MJ_FS, MJ_G, MJ_GS, MJ_A, MJ_AS, MJ_B,
		MN_A, MN_AS, MN_B, MN_C, MN_CS, MN_D,
		MN_DS, MN_E, MN_F, MN_FS, MN_G, MN_GS
	};
	static void setScale(int scale);
	static int getScale();
	static int getNote(int key);
};

class volume
{
	static int curVolume[2];
public:
	static void setVolume(int usage,int volume);
	static int getVolume100(int usage);
	static int getVolume128(int usage);
};

class instrument
{
	static int curIns;
public:
	static void setIns(HMIDIOUT hMidi, int insNum);
	static int getIns();
};

class play
{
	static int delay[2];
public:
	static void delayPlay(HMIDIOUT hMidi, int usage, int value, int key);
	static void playNote(HMIDIOUT hMidi, int usage, int key, bool status);
	static void setDelay(int usage,int time);
	static int getDelay(int usage);
};
