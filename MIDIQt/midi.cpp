#include <Windows.h>
#include <mmeapi.h>

#include "midi.h"

//channelPool start
//当前按键正在发的声音段数，使用atomic确保是原子操作
std::atomic_int channelPool::occupyChannel[2][26] = { 0 };
int channelPool::usingChannel(int usage)//当前整个电子琴使用的通道
{//写这个函数是为了方便后期修改
	if (usage == PLAY)
		return 0x1;
	else if (usage == REPLAY)
		return 0x2;
	return 0x0;
}
int channelPool::getChannel(int usage,int key)
{
	occupyChannel[usage][key - 'A']++;
	return usingChannel(usage);
}
int channelPool::releaseChannel(int usage,int key)
{
	occupyChannel[usage][key - 'A']--;
	return usingChannel(usage);
}
int channelPool::playingNumber(int usage,int key)
{
	return occupyChannel[usage][key - 'A'];
}
//channelPool end


//note start

//映射键盘，下面是低音上面是高音
std::map<int, int> note::keyMap = {
	{'Q',14},{'W',15},{'E',16},{'R',17},{'T',18},{'Y',19},{'U',20},
	{'A',7}, {'S',8}, {'D',9}, {'F',10},{'G',11},{'H',12},{'J',13},
	{'Z',0}, {'X',1}, {'C',2}, {'V',3}, {'B',4}, {'N',5}, {'M',6} };

bool note::isInitialed = false;//记录大小调表是否已经被生成
int note::curScale = MJ_C;
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
	{
		isInitialed = true;
		initScale();//使用前确保已经生成大小调表
	}
	curScale = scale;
}
int note::getScale()
{
	return curScale;
}
int note::getNote(int key)
{
	if (!isInitialed)
	{
		isInitialed = true;
		initScale();
	}
	int keyNum = keyMap[key];//映射得到这个键在大小调表里对应的索引，从而得出音符值
	return scale[curScale][keyNum];
}
//note end


//volume start
int volume::curVolume[2] = { 100,100 };;//这个音量值是百分比制的
void volume::setVolume(int usage,int volume)
{
	curVolume[usage] = volume;
}
int volume::getVolume128(int usage)//返回最大值为127（7F）的音量值，给接口用的
{
	return int(round(curVolume[usage] * 127.0 / 100));
}
int volume::getVolume100(int usage)//返回最大值为100的音量，主要是给人看的
{
	return curVolume[usage];
}
//volume end


//instrument start
int instrument::curIns = 0;
void instrument::setIns(HMIDIOUT hMidi, int insNum)
{
	curIns = insNum;
	int channel = channelPool::usingChannel(channelPool::PLAY);
	int info = insNum << 8 | 0xC0 | channel;
	recordCmd(info, 0, cmdStatus::INS);
	midiOutShortMsg(hMidi, info);//将默认通道的乐器改成指定乐器
}
int instrument::getIns()
{
	return curIns;
}
//instrument end


//play start
int play::delay[2] = {0,0};
void play::delayPlay(HMIDIOUT hMidi,int usage, int value, int key)//模拟钢琴踏板的延迟停止
{
	Sleep(delay[usage]);//等一段时间再考虑停止
	//为1则说明只有它自己在发声，应停止
	//否则说明其他音在发，如果停止会影响其他音，故不停止
	if (channelPool::playingNumber(usage,key) <= 1)
	{
		midiOutShortMsg(hMidi, value);
	}
	channelPool::releaseChannel(usage,key);//声明已完成
}
int play::getDelay(int usage)
{
	return delay[usage];
}

void play::playNote(HMIDIOUT hMidi, int usage, int key, bool status)//status:true=打开,false=关闭
{
	if (status == false)//停止
	{
		int pitch = note::getNote(key);
		int channel = channelPool::usingChannel(usage);
		int info = pitch << 8 | 0x90 | channel;
		recordCmd(info, key, status);
		std::thread(delayPlay, hMidi, usage,info, key).detach();
		//相当于 0<<16 | pitch << 8 | 0x90 | channel
		//所谓停止其实是把这个音的音量改成0，另外开多线程以实现延迟停止功能
		return;
	}
	else//开始
	{
		int realVolume = volume::getVolume128(usage);
		int pitch = note::getNote(key);
		int channel = channelPool::getChannel(usage,key);
		int info = realVolume << 16 | pitch << 8 | 0x90 | channel;
		recordCmd(info, key, status);
		midiOutShortMsg(hMidi, info);
		return;
	}
}

void play::setDelay(int usage,int time)//设置松开后延迟停止的等待时长
{
	delay[usage] = time;
}

//play end