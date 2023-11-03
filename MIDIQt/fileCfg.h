#pragma once

#include <fstream>
#include <vector>
#include <string>

#include "midi.h"
using namespace std;

bool isNum(const string& str);
bool isLegalNum(const string& str, int inf, int sup);

vector<string> readCfg(const string& path);
string strip(const string& str);
vector<pair<string, string>> parseCfg(vector<string> cfgLines);
void applyCfg(HMIDIOUT hMidi, const string& path);
void writeCfg(const string& path);