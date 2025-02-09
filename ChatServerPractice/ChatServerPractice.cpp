#include "stdafx.h"
#include "Chat.pb.h"

set<Session*> sessionManager;
vector<thread> threads;

void IocpProcess(HANDLE iocpHandle)
{
	while (true)
	{
		DWORD bytesTransferred = 0;
		Session* session = nullptr;
		CHAT_OVERLAPPED* chatOverlapped = nullptr;

		BOOL status = ::GetQueuedCompletionStatus(
			iocpHandle,
			&bytesTransferred,
			(ULONG_PTR*)&session,
			(LPOVERLAPPED*)&chatOverlapped,
			INFINITE
		);

		if (status == FALSE || bytesTransferred == 0)
		{
			sessionManager.erase(session);
			cout << "Client disconnect"
				 << "(session poolsize " << sessionManager.size() + 1 << "->" << sessionManager.size() << ')' << '\n';
			continue;
		}

		const auto& chatType = chatOverlapped->chatType;
		Protocol::Chat chat;
		chat.ParseFromArray(session->recvBuffer, BUFSIZE);

		if(chatType == ChatType::CHAT_FROM_CLIENT)
		{
			cout << "[thread-id-" 
				 << this_thread::get_id << "]"
				 << "recv Data(content="
				 << chat.content() << ','
				 << "chatType=" << static_cast<int>(chat.desiredaction()) << ','
				 << "session pool size=" << sessionManager.size() << ')'
				 << '\n';
			WSABUF wsaBuf;
			wsaBuf.buf = session->recvBuffer;
			wsaBuf.len = BUFSIZE;
			DWORD recvLen = 0;
			DWORD flags = 0;
			::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, chatOverlapped, NULL);


			CHAT_OVERLAPPED chatOverlappedToSend = {};
			WSABUF wsaBufToSend;
			Protocol::Chat chatToSend;
			char sendBufferToSend[BUFSIZE];
			
			wsaBufToSend.buf = sendBufferToSend;
			wsaBufToSend.len = BUFSIZE;
			DWORD sendLen = 0;
			DWORD sendFlags = 0;
			if (chat.desiredaction() == Protocol::DesiredAction::ECHO)
			{
				chatOverlappedToSend.chatType = ChatType::ECHO_FROM_SERVER;
				chatToSend.set_content("(echo)" + chat.content());
				chatToSend.SerializeToArray(sendBufferToSend, BUFSIZE);

				::WSASend(session->socket, &wsaBufToSend, 1, &sendLen, sendFlags, &chatOverlappedToSend, nullptr);
			}
			if (chat.desiredaction() == Protocol::DesiredAction::BROADCAST)
			{
				chatOverlappedToSend.chatType = ChatType::BOARDCAST_FROM_SERVER;
				chatToSend.set_content("(broadcast)" + chat.content());
				chatToSend.SerializeToArray(sendBufferToSend, BUFSIZE);

				for(const auto& s : sessionManager)
					::WSASend(s->socket, &wsaBufToSend, 1, &sendLen, sendFlags, &chatOverlappedToSend, nullptr);
			}
		}

		if (chatType == ChatType::ECHO_FROM_SERVER){}
		if (chatType == ChatType::BOARDCAST_FROM_SERVER){}
	}
}

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(8080);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	HANDLE iocpHandle = ::CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		0,
		0
	);

	for (int32 i = 0; i < 5; i++)
	{
		thread t([=]() { ::IocpProcess(iocpHandle); });
		threads.push_back(std::move(t));
	}
	
	cout << "server start" << '\n';
	while (true)
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);

		SOCKET clientSocket = ::accept(
			listenSocket,
			(SOCKADDR*)&clientAddr,
			&addrLen
		);

		if (clientSocket == INVALID_SOCKET)
			return 0;

		Session* session = new Session();
		session->socket = clientSocket;
		sessionManager.insert(session);

		cout << "Client Connected"
			 << "(session poolsize " << sessionManager.size()-1 << "->" << sessionManager.size() << ')' << '\n';

		::CreateIoCompletionPort(
			(HANDLE)clientSocket,
			iocpHandle,
			(ULONG_PTR)session,
			0
		);

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;
		CHAT_OVERLAPPED chatOverlapped = {};
		chatOverlapped.chatType = ChatType::CHAT_FROM_CLIENT;
		DWORD recvLen = 0;
		DWORD flags = 0;

		::WSARecv(
			clientSocket,
			&wsaBuf,
			1,
			&recvLen,
			&flags,
			&chatOverlapped,
			NULL
		);
	}

	for (auto& t : threads)
		t.join();

	::WSACleanup();
}