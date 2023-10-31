#include <iostream>
#include <thread>

#include <Windows.h>
#include <mmeapi.h>

#include "midi.h"

//channelPool start
//当前按键正在发的声音段数，使用atomic确保是原子操作
atomic_int channelPool::occupyChannel[26] = { false };
inline int channelPool::usingChannel()//当前整个电子琴使用的通道
{
	return 0xB;//这里写0xB是为了方便修改，下面同理
}
int channelPool::getChannel(int key)
{
	occupyChannel[key - 'A']++;
	return usingChannel();
}
inline int channelPool::releaseChannel(int key)
{
	occupyChannel[key - 'A']--;
	return usingChannel();
}
inline int channelPool::playingNumber(int key)
{
	return occupyChannel[key - 'A'];
}
//channelPool end


//note start
enum note::noteEnum//音高和midi数字对应表，中央C=C4=60
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
enum note::scale//各大小调的枚举
{
	MJ_C, MJ_CS, MJ_D, MJ_DS, MJ_E, MJ_F,
	MJ_FS, MJ_G, MJ_GS, MJ_A, MJ_AS, MJ_B,
	MN_A, MN_AS, MN_B, MN_C, MN_CS, MN_D,
	MN_DS, MN_E, MN_F, MN_FS, MN_G, MN_GS
};
//映射键盘，下面是低音上面是高音
map<int, int> note::keyMap = {
	{'Q',14},{'W',15},{'E',16},{'R',17},{'T',18},{'Y',19},{'U',20},
	{'A',7}, {'S',8}, {'D',9}, {'F',10},{'G',11},{'H',12},{'J',13},
	{'Z',0}, {'X',1}, {'C',2}, {'V',3}, {'B',4}, {'N',5}, {'M',6} };

bool note::isInitialed = false;//记录大小调表是否已经被生成
int note::curScale = MJ_G;
int note::scale[24][21];

void note::initScale()
{
	//根据大调音阶和小调音阶的规律生成对应表
	scale[0][0] = C3;//中央C是C4=60
	scale[12][0] = A2;//A小调的
	for (int i = 0; i < 12; i++)
	{
		const int stepsMajor[] = { 2,2,1,2,2,2,1 };//大调，全全半全全全半
		const int stepsMinor[] = { 2,1,2,2,1,2,2 };//小调，全半全全半全全
		if (i != 0)
		{
			scale[i][0] = scale[i - 1][0] + 1;//根据上一个大调生成下一个大调
			scale[i + 12][0] = scale[i + 12 - 1][0] + 1;//小调同理
		}
		for (int j = 1; j < 21; j++)
		{
			scale[i][j] = scale[i][j - 1] + stepsMajor[(j - 1) % 7];//根据上一个音符生成下一个音符
			scale[i + 12][j] = scale[i + 12][j - 1] + stepsMinor[(j - 1) % 7];
		}
	}
}
void note::setScale(int scale)
{
	if (!isInitialed)
		isInitialed = true, initScale();//使用前确保已经生成大小调表
	curScale = scale;
}
int note::getNote(int key)
{
	if (!isInitialed)
		isInitialed = true, initScale();
	int keyNum = keyMap[key];//映射得到这个键在大小调表里对应的索引，从而得出音符值
	return scale[curScale][keyNum];
}
//note end


//volume start
int volume::curVolume = 100;//这个音量值是百分比制的
void volume::setVolume(int volume)
{
	curVolume = volume;
}
int volume::getVolume100()//返回百分比制的音量
{
	return curVolume;
}
int volume::getVolume128()//返回最大值为127（7F）的音量值，给接口用的
{
	return int(round(curVolume * 127.0 / 100));
}
//volume end


//instrument start
int instrument::curIns = 1;
void instrument::setIns(HMIDIOUT hMidi, int insNum)
{
	int channel = channelPool::usingChannel();
	midiOutShortMsg(hMidi, insNum << 8 | 0xC0 | channel);//将默认通道的乐器改成指定乐器
}
void instrument::setIns(HMIDIOUT hMidi)
{
	setIns(hMidi, curIns);//这个接口其实本来不使用，只是初始化用的
}
int instrument::getIns()
{
	return curIns;
}
//instrument end


//play start
int play::delay = 1500;
void play::delayPlay(HMIDIOUT hMidi, int value, int key)//模拟钢琴踏板的延迟停止
{
	Sleep(delay);
	if (channelPool::playingNumber(key) <= 1)
	{
		midiOutShortMsg(hMidi, value);
		cout << key << ' ' << channelPool::playingNumber(key) << endl;
	}
	channelPool::releaseChannel(key);
}

void play::playNote(HMIDIOUT hMidi, int key, bool status)//status:true=打开,false=关闭
{
	if (status == false)//停止
	{
		int pitch = note::getNote(key);
		int channel = channelPool::usingChannel();
		thread(delayPlay, hMidi, 0 << 16 | pitch << 8 | 0x90 | channel, key).detach();
		//所谓停止其实是把这个音的音量改成0，开多线程以实现延迟停止功能
		return;
	}
	else//开始
	{
		int realVolume = volume::getVolume128();
		int pitch = note::getNote(key);
		int channel = channelPool::getChannel(key);
		midiOutShortMsg(hMidi, realVolume << 16 | pitch << 8 | 0x90 | channel);
		return;
	}
}

inline void play::setDelay(int time)//设置松开后延迟停止的等待时长
{
	delay = time;
}

inline int play::getDelay()
{
	return delay;
}
//play end