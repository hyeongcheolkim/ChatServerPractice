#pragma once

#ifdef _DEBUG
#pragma comment(lib, "CommonLibrariesBuildOutput\\Debug\\CommonLibraries.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "CommonLibrariesBuildOutput\\Release\\CommonLibraries.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobuf.lib")
#endif

#include "common.h"