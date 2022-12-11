#pragma once

#include <asio.hpp>
#include <unordered_map>
#include <functional>

#include "../../NetworkingCommon/netMessage.h"
#include "../../NetworkingCommon/threadSafeQueue.h"

class Connection
{
public:
	typedef asio::ip::tcp::socket tcpSocket;
	Connection(ThreadSafeQueue& serverMessageQueue, tcpSocket clientSocket, asio::io_context& m_asioContext);
	void sendMessage(const Message& message);
	void setUsername(const std::string& username);
	const std::string& getUsername();

private:
	void sendMessageHeader();
	void sendMessageType();
	void sendMessageBody();

	void readMessageHeader();
	void readMessageType();
	void readMessageDestination();
	void readMessageBody();
	void pushMessageToServerQueue();
	//static void readLoginMessage(Message& tempMessage, tcpSocket& clientSocket);
	//static void readRegistrationMessage(Message& tempMessage, tcpSocket& clientSocket);
	//static void readTextMessage(Message& tempMessage, tcpSocket& clientSocket);

private:
	asio::io_context& m_asioContext;
	asio::ip::tcp::socket m_clientSocket;
	ThreadSafeQueue m_outgoingMessageQueue; // messages from server to client
	ThreadSafeQueue& m_incomingMessageQueue; // messages from clients to server
	Message m_tempMessage;
	std::string m_clientUsername;
	//static std::unordered_map<MessageType, std::function<void(Message&, tcpSocket& clientSocket)>> readMessageBody;

};

