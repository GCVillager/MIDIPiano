#include "replay.h"

bool replay::timerStarted = false;//
bool replay::isRecording = false;
std::atomic_bool replay::isPlaying = false;
std::atomic_int replay::playingRound = 0;
int replay::delay = 0;
std::chrono::system_clock::time_point replay::startTime;
std::stringstream replay::readList;
std::stringstream replay::writeList;

void replay::setRecordStatus(bool status)//设置是否记录，true为记录
{
	isRecording = status;
	if (status == false)
	{
		timerStarted = false;//保证下一次开始时变量状态都正确
	}
}
void replay::setRecordDelay(int delay)//记录时设置松开延迟
{
	if (isRecording)
	{
		writeList << "delay " << delay << std::endl;
	}
	
}
int replay::getReplayDelay()//获取播放到的文件此时的松开延迟
{
	return delay;
}
void replay::recordCmd(int cmd,int key,int status)//记录当前的键盘操作
{
	if (!isRecording)
	{
		return;//如果没开启记录就不记录了，丢弃
	}
	const int shieldChannel = 0x0000FFF0;//屏蔽通道和音量用的常量
	if (!timerStarted)//这一块可以确保第一个音符从时间0开始
	{
		timerStarted = true;
		startTime = std::chrono::system_clock::now();
		int curIns = instrument::getIns();
		writeList << 0 << ' ' << (curIns << 8|0XC0 ) << ' ' << 0 << ' ' << 2<< std::endl;
		//先记录一下此时乐器，否则信息丢失
	}
	auto now= std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
	//算出毫秒单位的时间，和第一个音相比的偏移
	writeList << duration.count() << ' ' << (cmd&shieldChannel) <<' '<<key<<' '<<status<< std::endl;
	//离开始时刻的时间、播放的音（已去除通道和音量）、按下的键（帮助执行延迟停止用的）、开始还是关闭 
}
void replay::writeFile(const std::string& file)//将记录的操作一次性写入文件并清空缓存
{
	std::ofstream fout(file);
	if (!fout.is_open())
	{
		return;
	}
	fout << writeList.str();
	fout.close();
	clearWriteBuff();//清空缓存
}
void replay::readFile(const std::string& file)//从文件读取记录，准备被播放
{
	readList.str("");
	std::ifstream fin(file);
	if (!fin.is_open())
	{
		return;
	}
	readList << fin.rdbuf();//直接读取全部
	fin.close();
}
void replay::stopReplay(HMIDIOUT hMidi)//停止播放记录
{
	isPlaying = false;
	int channel = channelPool::usingChannel(channelPool::REPLAY);
	midiOutShortMsg(hMidi, 0X7BB0 | channel);//关闭整个通道
}

//开始播放记录（也可能是重新开始）
//由于使用了Sleep，调用这个函数一定要开线程
void replay::startReplay(HMIDIOUT hMidi)
{
	std::stringstream readListCopy;//复制一份。因为可能会重复播放，读完了就没了
	readListCopy << readList.str();
	playingRound++;
	int curPlayRound = playingRound;//这个是为了防止一种bug写的保障“装置”
	isPlaying = true;
	std::string dataHead;
	int curTime = 0;//记录当前播放的时间。从一条记录的时间跳向下一条的时间
	while (readListCopy>>dataHead && isPlaying && playingRound==curPlayRound)
	{//没读完，而且是在播放（如果被停止了就会在这里停下）
		if (dataHead == "delay")
		{
			int newDelay;
			readListCopy >> newDelay;
			delay = newDelay;//先写入，等待后续读
		}
		else
		{
			int time = atoi(dataHead.c_str());
			Sleep(time - curTime);//跳向下一个时间节点
			play::setDelay(channelPool::REPLAY, delay);
			//在这里读是为了防止更改对上一个音符的长度造成影响
			curTime = time;
			int note, key;
			int status;
			int channel = channelPool::usingChannel(channelPool::REPLAY);
			int volume = volume::getVolume128(channelPool::REPLAY);
			readListCopy >> note >> key >> status;
			switch (status)
			{
			case cmdStatus::BEGIN:
				note = volume << 16 | note | channel;
				midiOutShortMsg(hMidi, note);//播放声音
				break;
			case cmdStatus::END:
				std::thread(play::delayPlay, hMidi, channelPool::REPLAY, note | channel, key).detach();
				//开线程延迟结束
				break;
			case cmdStatus::INS:
				midiOutShortMsg(hMidi, note | channel);//修改乐器
				break;
			}
		}
	}
}
void replay::clearWriteBuff()//清空记录缓冲区
{
	writeList.str("");
}

//记录当前的键盘操作
//程序写成屎山了，不得不写前置声明了，这是连接屎与屎的桥梁
void recordCmd(int cmd, int key, int status)
{
	replay::recordCmd(cmd, key, status);
}