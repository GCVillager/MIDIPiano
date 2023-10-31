#include <iostream>
#include <thread>

#include <Windows.h>
#include <mmeapi.h>

#include "midi.h"

//channelPool start
//��ǰ�������ڷ�������������ʹ��atomicȷ����ԭ�Ӳ���
atomic_int channelPool::occupyChannel[26] = { false };
inline int channelPool::usingChannel()//��ǰ����������ʹ�õ�ͨ��
{
	return 0xB;//����д0xB��Ϊ�˷����޸ģ�����ͬ��
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
enum note::noteEnum//���ߺ�midi���ֶ�Ӧ������C=C4=60
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
enum note::scale//����С����ö��
{
	MJ_C, MJ_CS, MJ_D, MJ_DS, MJ_E, MJ_F,
	MJ_FS, MJ_G, MJ_GS, MJ_A, MJ_AS, MJ_B,
	MN_A, MN_AS, MN_B, MN_C, MN_CS, MN_D,
	MN_DS, MN_E, MN_F, MN_FS, MN_G, MN_GS
};
//ӳ����̣������ǵ��������Ǹ���
map<int, int> note::keyMap = {
	{'Q',14},{'W',15},{'E',16},{'R',17},{'T',18},{'Y',19},{'U',20},
	{'A',7}, {'S',8}, {'D',9}, {'F',10},{'G',11},{'H',12},{'J',13},
	{'Z',0}, {'X',1}, {'C',2}, {'V',3}, {'B',4}, {'N',5}, {'M',6} };

bool note::isInitialed = false;//��¼��С�����Ƿ��Ѿ�������
int note::curScale = MJ_G;
int note::scale[24][21];

void note::initScale()
{
	//���ݴ�����׺�С�����׵Ĺ������ɶ�Ӧ��
	scale[0][0] = C3;//����C��C4=60
	scale[12][0] = A2;//AС����
	for (int i = 0; i < 12; i++)
	{
		const int stepsMajor[] = { 2,2,1,2,2,2,1 };//�����ȫȫ��ȫȫȫ��
		const int stepsMinor[] = { 2,1,2,2,1,2,2 };//С����ȫ��ȫȫ��ȫȫ
		if (i != 0)
		{
			scale[i][0] = scale[i - 1][0] + 1;//������һ�����������һ�����
			scale[i + 12][0] = scale[i + 12 - 1][0] + 1;//С��ͬ��
		}
		for (int j = 1; j < 21; j++)
		{
			scale[i][j] = scale[i][j - 1] + stepsMajor[(j - 1) % 7];//������һ������������һ������
			scale[i + 12][j] = scale[i + 12][j - 1] + stepsMinor[(j - 1) % 7];
		}
	}
}
void note::setScale(int scale)
{
	if (!isInitialed)
		isInitialed = true, initScale();//ʹ��ǰȷ���Ѿ����ɴ�С����
	curScale = scale;
}
int note::getNote(int key)
{
	if (!isInitialed)
		isInitialed = true, initScale();
	int keyNum = keyMap[key];//ӳ��õ�������ڴ�С�������Ӧ���������Ӷ��ó�����ֵ
	return scale[curScale][keyNum];
}
//note end


//volume start
int volume::curVolume = 100;//�������ֵ�ǰٷֱ��Ƶ�
void volume::setVolume(int volume)
{
	curVolume = volume;
}
int volume::getVolume100()//���ذٷֱ��Ƶ�����
{
	return curVolume;
}
int volume::getVolume128()//�������ֵΪ127��7F��������ֵ�����ӿ��õ�
{
	return int(round(curVolume * 127.0 / 100));
}
//volume end


//instrument start
int instrument::curIns = 1;
void instrument::setIns(HMIDIOUT hMidi, int insNum)
{
	int channel = channelPool::usingChannel();
	midiOutShortMsg(hMidi, insNum << 8 | 0xC0 | channel);//��Ĭ��ͨ���������ĳ�ָ������
}
void instrument::setIns(HMIDIOUT hMidi)
{
	setIns(hMidi, curIns);//����ӿ���ʵ������ʹ�ã�ֻ�ǳ�ʼ���õ�
}
int instrument::getIns()
{
	return curIns;
}
//instrument end


//play start
int play::delay = 1500;
void play::delayPlay(HMIDIOUT hMidi, int value, int key)//ģ�����̤����ӳ�ֹͣ
{
	Sleep(delay);
	if (channelPool::playingNumber(key) <= 1)
	{
		midiOutShortMsg(hMidi, value);
		cout << key << ' ' << channelPool::playingNumber(key) << endl;
	}
	channelPool::releaseChannel(key);
}

void play::playNote(HMIDIOUT hMidi, int key, bool status)//status:true=��,false=�ر�
{
	if (status == false)//ֹͣ
	{
		int pitch = note::getNote(key);
		int channel = channelPool::usingChannel();
		thread(delayPlay, hMidi, 0 << 16 | pitch << 8 | 0x90 | channel, key).detach();
		//��νֹͣ��ʵ�ǰ�������������ĳ�0�������߳���ʵ���ӳ�ֹͣ����
		return;
	}
	else//��ʼ
	{
		int realVolume = volume::getVolume128();
		int pitch = note::getNote(key);
		int channel = channelPool::getChannel(key);
		midiOutShortMsg(hMidi, realVolume << 16 | pitch << 8 | 0x90 | channel);
		return;
	}
}

inline void play::setDelay(int time)//�����ɿ����ӳ�ֹͣ�ĵȴ�ʱ��
{
	delay = time;
}

inline int play::getDelay()
{
	return delay;
}
//play end