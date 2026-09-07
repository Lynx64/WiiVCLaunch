#pragma once

#include <coreinit/dynload.h>

#include <string_view>
#include <vector>

uint32_t findMem(uint32_t start, uint32_t size, const char *originalVal, size_t originalValSize);
bool getRplInfo(std::vector<OSDynLoad_NotifyData> &rpls);
bool findRpl(const std::string_view &name, OSDynLoad_NotifyData &foundRpl);
