#pragma once

#pragma warning(disable : 4251 4305 4244 4005)

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>
#include <memory>

#include "UIlib.h"
using namespace DuiLib;

#include "CMoveControl.h"
#include "CWnd2048.h"
