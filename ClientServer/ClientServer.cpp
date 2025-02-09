#include "stdafx.h"
#include "Chat.pb.h"

vector<thread> threads;
mutex mtx;
condition_variable cv;
int32 messageProcessedDone = 2;

void IocpProcess(HANDLE iocpHandle)
{
	while (true)
	{
		DWORD bytesTransferred = 0;
		Session* session = nullptr;
		CHAT_OVERLAPPED* chatOverlapped = nullptr;

		BOOL ret = ::GetQueuedCompletionStatus
		(
			iocpHandle,
			&bytesTransferred,
			(ULONG_PTR*)&session, 
			(LPOVERLAPPED*)&chatOverlapped,
			INFINITE
		);

		if (ret == FALSE || bytesTransferred == 0)
			continue;

		const auto& chatType = chatOverlapped->chatType;
		Protocol::Chat chat;
		chat.ParseFromArray(session->recvBuffer, BUFSIZE);

		if (chatType == ChatType::CHAT_FROM_CLIENT)
		{
			cout << "[thread-id-" << this_thread::get_id << " | CHAT_FROM_CLIENT]"
				 << "message send\n(\n\tcontent="
				 << chat.content()
				 << ",\n\tchatType=" << static_cast<int>(chat.desiredaction()) << "\n)"
				 << '\n';
		}
		if (chatType != ChatType::CHAT_FROM_CLIENT)
		{
			cout << "[thread-id-" << this_thread::get_id << " | FROM_SERVER]"
				 << "message receive\n(\n\tcontent="
				 << chat.content()
				 << ",\n\tchatType=" << static_cast<int>(chat.desiredaction()) << "\n)"
				 << '\n';
		}

		//로그 순서 동기화를 위한 부분
		--messageProcessedDone;
		cv.notify_one();
	}
}

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		return 0;

	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(8080);

	while (true)
	{
		if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			break;
		}
	}

	Session* session = new Session();
	session->socket = clientSocket;
	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*Key*/(ULONG_PTR)session, 0);

	for (int32 i = 0; i < 5; i++)
	{
		thread t([=]() { ::IocpProcess(iocpHandle); });
		threads.push_back(std::move(t));
	}

	thread recvThread([=]()
		{
			while (true)
			{
				WSABUF wsaBuf;
				wsaBuf.buf = session->recvBuffer;
				wsaBuf.len = BUFSIZE;

				CHAT_OVERLAPPED chatOverlapped{};

				DWORD recvLen = 0;
				DWORD flags = 0;
				::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &chatOverlapped, NULL);
			}
		}
	);

	cout << "Connected to Server!" << '\n';

	char sendBuffer[BUFSIZE];
	CHAT_OVERLAPPED overlapped{};
	overlapped.chatType = ChatType::CHAT_FROM_CLIENT;
	Protocol::Chat chat;

	string chatType, content;
	while (true)
	{
		cout << "채팅타입을 입력하세요(echo[0], broadcast[1] 중 택)\n";
		getline(cin, chatType);
		if (chatType != "echo" && chatType != "broadcast" && chatType != "0" && chatType != "1")
		{
			cout << '\n' << "echo 또는 boardcast 둘중 하나만 선택 가능합니다\n" << '\n';
			continue;
		}
		cout << "채팅 메세지를 입력하세요:\n";
		getline(cin, content);

		if (chatType == "echo" || chatType == "0")
			chat.set_desiredaction(Protocol::DesiredAction::ECHO);
		if (chatType == "broadcast" || chatType == "1")
			chat.set_desiredaction(Protocol::DesiredAction::BROADCAST);
		chat.set_content(content);
		chat.SerializeToArray(session->recvBuffer, BUFSIZE);

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;
		DWORD sendLen = 0;
		DWORD flags = 0;
		::WSASend(
			clientSocket,
			&wsaBuf,
			1,
			&sendLen,
			flags,
			&overlapped,
			nullptr
		);

		//로그출력순서 꼬이지 않게하는 락
		//send, recv 두번 탈동안 sleep
		unique_lock<mutex> lock(mtx);
		cv.wait(lock, [] {return messageProcessedDone == 0; });
		messageProcessedDone = 2;
	}

	recvThread.join();
	for (auto& t : threads)
		t.join();
	::closesocket(clientSocket);
	::WSACleanup();
}