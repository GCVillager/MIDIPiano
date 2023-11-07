#pragma once

#include <fstream>
#include <vector>
#include <string>

#include "midi.h"

bool isNum(const std::string& str);
bool isLegalNum(const std::string& str, int inf, int sup);

std::vector<std::string> readCfg(const std::string& path);
std::string strip(const std::string& str);
std::vector<std::pair<std::string, std::string>> parseCfg(std::vector<std::string> cfgLines);
void applyCfg(HMIDIOUT hMidi, const std::string& path);
void writeCfg(const std::string& path);