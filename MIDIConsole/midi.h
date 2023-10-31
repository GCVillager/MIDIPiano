
#ifndef MIDI_H
#define MIDI_H

#include <map>
#include <queue>
#include <string>
#include <atomic>

#include <mmeapi.h>
#include <Windows.h>
#pragma comment(lib,"winmm.lib")

using namespace std;


class channelPool
{
	static atomic_int occupyChannel[26];
public:
	static int usingChannel();//当前整个电子琴使用的通道
	static int getChannel(int key);
	static int releaseChannel(int key);
	static int playingNumber(int key);
};

class note
{
	enum noteEnum;
	enum scale;

	static map<int, int> keyMap;
	static bool isInitialed;
	static int curScale;
	static int scale[24][21];
public:
	static void initScale();
	static void setScale(int scale);
	static int getNote(int key);
};

class volume
{
	static int curVolume;
public:
	static void setVolume(int volume);
	static int getVolume100();
	static int getVolume128();
};

class instrument
{
	static int curIns;
public:
	static void setIns(HMIDIOUT hMidi,int insNum);
	static void setIns(HMIDIOUT hMidi);
	static int getIns();
};

class play
{
	static int delay;
	static void delayPlay(HMIDIOUT hMidi, int value,int key);
public:
	static void playNote(HMIDIOUT hMidi, int key, bool status);
	inline static void setDelay(int time);
	inline static int getDelay();
};




#endif // !MIDI_H

