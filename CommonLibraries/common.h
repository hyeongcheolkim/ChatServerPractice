#pragma once

#include <string>
#include <iostream>
#include <winsock2.h>
#include <set>
#include <unordered_set>
#include <vector>
#include <thread>
#include "Types.h"
#include "test.h"
#include <mutex>
#include <functional>
using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "CommonLibrariesBuildOutput\\Debug\\CommonLibraries.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "CommonLibrariesBuildOutput\\Release\\CommonLibraries.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")

#endif

#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "common.h"