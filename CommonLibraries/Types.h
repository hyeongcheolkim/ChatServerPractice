#pragma once

using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

enum class ChatType : int
{
	CHAT_FROM_CLIENT = 1,
	ECHO_FROM_SERVER = 2,
	BOARDCAST_FROM_SERVER = 3,
	NONE = -1,
};

class CHAT_OVERLAPPED : public OVERLAPPED
{
public:
	ChatType chatType = ChatType::NONE;
};

const int32 BUFSIZE = 10000;
struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
};
