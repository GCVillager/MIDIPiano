
#ifndef MIDI_H
#define MIDI_H

#include <map>
#include <queue>
#include <string>

#include <mmeapi.h>
#include <Windows.h>
#pragma comment(lib,"winmm.lib")

using namespace std;


class channelPool
{
	static bool isInitialed;
	static queue<int> pool;
public:
	static void poolInit();
	static int getChannel();
	static void releaseChannel(int channel);
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
	static int getNode(int key);
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
	static map<int, int> occupyChannel;//value1=°´¼ü,value2=Í¨µÀ
	static void delayPlay(HMIDIOUT hMidi, int value,int channel);
public:
	static void playNote(HMIDIOUT hMidi, int key, bool status);
	inline static void setDelay(int time);
	inline static int getDelay();
};




#endif // !MIDI_H

