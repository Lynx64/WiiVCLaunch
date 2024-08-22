#pragma once

#include <mocha/mocha.h>

MochaUtilsStatus MountWrapper(const char *mount, const char *dev, const char *mountTo);
MochaUtilsStatus initMocha();
