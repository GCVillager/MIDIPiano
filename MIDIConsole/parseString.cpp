#include "parseString.h"

vector<string> split(string str)
{
	vector<string> ret;
	ret.push_back("");
	bool lastIsSpace = true;//防止出现多个空格字符串的情况
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')
		{
			if (lastIsSpace)
				continue;
			else
			{
				lastIsSpace = true;
				ret.push_back("");
			}
		}
		else
		{
			lastIsSpace = false;
			ret[ret.size() - 1] += str[i];
		}
		
	}
	return ret;
}

string lower(string str)
{
	string ret;
	for (int i = 0; i < str.size(); i++)
	{
		if (isupper(str[i]))
			ret += (str[i] - 32);
		else
			ret += str[i];
	}
	return ret;
}

bool isNum(string str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] < '0' || str[i]>'9')
			return false;
	}
	return true;
}