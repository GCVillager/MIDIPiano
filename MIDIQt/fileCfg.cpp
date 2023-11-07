#include "fileCfg.h"

bool isNum(const std::string& str)//判断一个字符串是不是全是数字
{
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] < '0' || str[i]>'9')
		{
			return false;
		}
	}
	return true;
}

//判断一个字符串是不是数字且在闭区间[a,b]内
bool isLegalNum(const std::string& str, int inf, int sup)
{
	if (!isNum(str))
	{
		return false;
	}
	int num = atoi(str.c_str());
	if (num<inf || num>sup)
	{
		return false;
	}
	return true;
}


std::vector<std::string> readCfg(const std::string& path)//从指定路径读取配置文件
{
	std::vector<std::string>ret;
	std::ifstream fin(path);
	if (!fin.is_open())
	{
		return ret;
	}
	//如果未能成功打开就算了
	while (true)
	{
		std::string line;
		getline(fin, line);
		if (fin.eof())//读到eof说明文件结束
		{
			break;
		}
		ret.push_back(line);
	}
	fin.close();
	return ret;
}

std::string strip(const std::string& str)
{//去除字符串头尾的空白符
	std::string ret="";
	int frontIndex=0,backIndex = str.size()-1;
	while (str[frontIndex] == ' ' || str[frontIndex] == '\n' || str[frontIndex] == '\t')
	{
		frontIndex++;
	}
	while (str[backIndex] == ' ' || str[backIndex] == '\n' || str[backIndex] == '\t')
	{
		backIndex--;
	}
	for (int i = frontIndex; i <= backIndex; i++)
	{
		ret += str[i];
	}
	return ret;
}

//解析字符串是否是合法的配置格式，并按等号分隔
std::vector<std::pair<std::string, std::string>> parseCfg(std::vector<std::string> cfgLines)
{
	std::vector<std::pair<std::string, std::string>> ret;
	for (int i = 0; i < cfgLines.size();i++)
	{
		std::string& line = cfgLines[i];
		int equalCnt = 0;//记录等号的数量
		int equalIndex = 0;//记录等号的位置
		for (int j = 0; j < line.size(); j++)
		{
			if (line[j] == '=')
			{
				equalCnt++;
				equalIndex = j;
			}
		}
		if (equalCnt != 1)//等号数量必须为1，否则不合法
		{
			continue;
		}
		std::string str1 = strip(line.substr(0, equalIndex));
		//相当于python str1[0:equalIndex].strip()
		std::string str2 = strip(line.substr(equalIndex+1, line.size()- equalIndex));
		//相当于python str2[equalIndex:].strip()
		ret.push_back({str1,str2});
	}
	return ret;
}

void writeCfg(const std::string& path)//将当前设置写入cfg
{
	std::ofstream fout{ path };
	if (!fout.is_open())
	{
		return;
	}
	//打不开就算了
	fout << "scale=" << note::getScale() << std::endl;
	fout << "volume=" << volume::getVolume100(channelPool::PLAY) << std::endl;
	fout << "delay=" << play::getDelay(channelPool::PLAY) << std::endl;
	fout << "instrument=" << instrument::getIns() << std::endl;
	fout << "replay_volume=" << volume::getVolume100(channelPool::REPLAY) << std::endl;
	fout.close();
}

//读取、解析cfg文件，并设置核心状态
void applyCfg(HMIDIOUT hMidi,const std::string& path)
{
	std::vector<std::pair<std::string, std::string>> parsedCfg = parseCfg(readCfg(path));
	//从readCfg()读，经parseCfg()解析
	for (std::pair<std::string, std::string> line : parsedCfg)
	{
		if (line.first == "scale")
		{
			if (isLegalNum(line.second, 0, 23))
			{
				note::setScale(atoi(line.second.c_str()));
			}
		}
		if (line.first == "volume")
		{
			if (isLegalNum(line.second, 0, 100))
			{
				volume::setVolume(channelPool::PLAY,atoi(line.second.c_str()));
			}
		}
		if (line.first == "delay")
		{
			if (isLegalNum(line.second, 0, 5000))
			{
				play::setDelay(channelPool::PLAY,atoi(line.second.c_str()));
			}
		}
		if (line.first == "instrument")
		{
			if (isLegalNum(line.second, 0, 127))
			{
				instrument::setIns(hMidi, atoi(line.second.c_str()));
			}
		}
		if (line.first == "replay_volume")
		{
			if (isLegalNum(line.second, 0, 100))
			{
				volume::setVolume(channelPool::REPLAY, atoi(line.second.c_str()));
			}
		}
	}
}