#include "stdafx.h"
#include "ConcurrentSet.h"

class ChatServer
{
public:
	static void broadcastMessage(const string& msg, SOCKET sender);
	static void echoMessage(const string& msg, SOCKET sender);
private:
	ConcurrentSet<SOCKET> sockets;
};