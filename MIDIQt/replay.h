#pragma once

#include <vector>
#include<fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <atomic>
#include <thread>

#include "midi.h"

class replay
{
	static std::atomic_int playingRound;
	static std::atomic_bool isPlaying;
	static std::chrono::system_clock::time_point startTime;
	static bool timerStarted;
	static int delay;
	static bool isRecording;
	static std::stringstream readList;
	static std::stringstream writeList;
public:
	static void setRecordStatus(bool status);
	static void setRecordDelay(int delay);
	static int getReplayDelay();
	static void recordCmd(int cmd,int key,int status);
	static void writeFile(const std::string& file);
	static void readFile(const std::string& file);
	static void stopReplay(HMIDIOUT hMidi);
	static void startReplay(HMIDIOUT hMidi);
	static void clearWriteBuff();
};

void recordCmd(int cmd, int key, int status);