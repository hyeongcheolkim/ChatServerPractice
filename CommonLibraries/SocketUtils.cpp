#include "pch.h"
#include "SocketUtils.h"

LPFN_CONNECTEX SocketUtils::getConnectEx()
{
	return nullptr;
}

LPFN_DISCONNECTEX SocketUtils::getDisconnectEx()
{
	if (SocketUtils::connectEx != nullptr)
		return SocketUtils::disconnectEx;
}

LPFN_ACCEPTEX SocketUtils::getAcceptEx()
{
	if (SocketUtils::connectEx != nullptr)
		return SocketUtils::acceptEx;
}

SOCKET SocketUtils::createSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}