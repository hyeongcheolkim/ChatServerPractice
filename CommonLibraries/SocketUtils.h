#pragma once
class SocketUtils
{
private:
	static LPFN_CONNECTEX connectEx;
	static LPFN_DISCONNECTEX disconnectEx;
	static LPFN_ACCEPTEX acceptEx;
public:
	static LPFN_CONNECTEX getConnectEx();
	static LPFN_DISCONNECTEX getDisconnectEx();
	static LPFN_ACCEPTEX getAcceptEx();

	SOCKET createSocket();
	
};

LPFN_CONNECTEX SocketUtils::connectEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::disconnectEx = nullptr;
LPFN_ACCEPTEX SocketUtils::acceptEx = nullptr;