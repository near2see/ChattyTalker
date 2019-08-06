#include "Client.h"

BlockUdpClient::BlockUdpClient(SOCKADDR_IN serv_addr)
{
	clnt_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (clnt_socket_ == INVALID_SOCKET)
	{
		ErrorHandling("BlockUdpClient : Invalid clnt_sock_ ", &clnt_socket_);
	}

	serv_addr_ = serv_addr;

	if (bind(clnt_socket_, (SOCKADDR*)& clnt_addr_, sizeof(clnt_addr_)) == SOCKET_ERROR)
	{
		ErrorHandling("Client: binding socket failed", &clnt_socket_);
	}

	SOCKADDR_IN addr;
	int addrlen = sizeof(addr);
	getsockname(clnt_socket_, (SOCKADDR*)& addr, &addrlen);
	printf("BlockUdpClient : %s\n", toString(addr).c_str());
}

void BlockUdpClient::Chat()
{
	printf("BlockUdpClient::Chat() Start\n");
	bool on_chat = true;
	std::thread recv([&]() { Recvfrom(on_chat); });

	char message[MAX_MESSAGE_SIZE];

	while (true)
	{
		memset(message, 0, MAX_MESSAGE_SIZE);
		std::cin.getline(message, MAX_MESSAGE_SIZE);

		if (strlen(message) < 1)
			continue;

		//Leave ó�� �ʿ�

		if (strcmp(message, "/exit") == 0)
		{
			on_chat = false;
			ChatPacket chat_packet(LEAVE);
			//The transmission is not guaranteed. because it's UDP
			sendto(clnt_socket_, (char*)&chat_packet, sizeof(chat_packet), 0, (SOCKADDR*)& serv_addr_, sizeof(serv_addr_));
			closesocket(clnt_socket_);
			recv.join();
			return;
		}

		MessagePacket msg_packet(message);

		sendto(clnt_socket_, (char*)&msg_packet, sizeof(msg_packet), 0, (SOCKADDR*) &serv_addr_, sizeof(serv_addr_));
	}
}

void BlockUdpClient::Recvfrom(bool& on_chat)
{
	while (on_chat)
	{
		char buf[MAX_PACKET_SIZE];
		memset(buf, 0, MAX_PACKET_SIZE);
		SOCKADDR_IN addr;
		int addrlen = sizeof(addr);

		int r = recvfrom(clnt_socket_, buf, MAX_PACKET_SIZE, 0, (SOCKADDR*)& addr, &addrlen);

		if (r < 0)
		{
			return;
		}

		PacketType packet_type;
		if (GetPacketType(buf, packet_type) && packet_type == MESSAGE)
		{
			MessagePacket* msg_packet = (MessagePacket*)buf;
			printf("%s\n", msg_packet->get_message());
		}
	}
}