#include "../inc/Connection.h"
#include <iostream>

typedef asio::ip::tcp::socket tcpSocket;

Connection::Connection(ThreadSafeQueue& serverMessageQueue, tcpSocket clientSocket, asio::io_context& asioContext) :
    m_clientSocket(std::move(clientSocket)),
    m_incomingMessageQueue(serverMessageQueue),
    m_asioContext(asioContext)
{
    readMessageHeader();
}


void Connection::readMessageHeader()
{
    asio::async_read(m_clientSocket, asio::buffer(&m_tempMessage.messageSize, sizeof(m_tempMessage.messageSize)), 
        [this](const asio::error_code& ec, size_t a) {
            std::cout << "Received message, size: " << m_tempMessage.messageSize << std::endl;
            if (ec)
                std::cout << ec.message() << std::endl;
            else if (m_tempMessage.messageSize > 0)
            {
                m_tempMessage.messageContent.resize(m_tempMessage.messageSize);
                readMessageType();
            }
        }
    );
}

void Connection::readMessageType()
{
    asio::async_read(m_clientSocket, asio::buffer(&m_tempMessage.messageType, sizeof(m_tempMessage.messageType)),
        [this](const asio::error_code& ec, size_t a) {
            if (ec)
                std::cout << ec.message() << std::endl;
            else {
                readMessageBody();
            }
        }
    );
}

void Connection::readMessageBody()
{
    asio::async_read(m_clientSocket, asio::buffer(m_tempMessage.messageContent.data(), m_tempMessage.messageSize),
        [this](asio::error_code ec, size_t a) {
            if (ec)
                std::cout << ec.message() << std::endl;
            else
            {
                std::cout << "Read message body" << std::endl;
                for (auto x : m_tempMessage.messageContent)
                    std::cout << x;
                std::cout << "Read message body 2" << std::endl;
            }
            pushMessageToServerQueue();
            readMessageHeader();
        }
    );
}

void Connection::sendMessage(const Message& message)
{
    asio::post(m_asioContext, 
        [this, message]() 
        {
            bool hasMessage = !m_outgoingMessageQueue.empty();
            m_outgoingMessageQueue.insertBack(message);
            if (!hasMessage) {
                sendMessageHeader();
            }
        }
    );
}


void Connection::sendMessageHeader()
{
    asio::async_write(m_clientSocket, asio::buffer(&m_outgoingMessageQueue.front().messageSize, sizeof(m_outgoingMessageQueue.front().messageSize)), 
        [this](const asio::error_code& ec, size_t a) {
            std::cout << "header bytes sent: " << a << std::endl;
            if (ec)
                std::cout << ec.message() << std::endl;
            sendMessageType();
        });
}

void Connection::sendMessageType()
{
    asio::async_write(m_clientSocket, asio::buffer(&m_outgoingMessageQueue.front().messageType, sizeof(m_outgoingMessageQueue.front().messageType)),
        [this](const asio::error_code& ec, size_t a) {
            std::cout << "sent Type: " << static_cast<int>(m_outgoingMessageQueue.front().messageType) << std::endl;
            if (ec)
                std::cout << ec.message() << std::endl;
            else
                sendMessageBody();
        }
    );
}

void Connection::sendMessageBody()
{
    asio::async_write(m_clientSocket, asio::buffer(m_outgoingMessageQueue.front().messageContent.data(), m_outgoingMessageQueue.front().messageSize),
        [this](const asio::error_code& ec, size_t a) {
            std::cout << "sent message body" << std::endl;
            if (ec)
                std::cout << ec.message() << std::endl;
            else
            {
                std::cout << std::endl;
                for (const auto& x : m_outgoingMessageQueue.front().messageContent)
                    std::cout << x;
                std::cout << std::endl;
            }

            m_outgoingMessageQueue.popFront();
        }
    );
}

void Connection::pushMessageToServerQueue()
{
    std::cout << "uslo pushMessageToServerQueue" << std::endl;
    m_tempMessage.sourceConnection = this;
    m_incomingMessageQueue.insertBack(m_tempMessage);
    std::cout << "izaslo pushMessageToServerQueue" << std::endl;

}

void Connection::setUsername(const std::string& username)
{
    m_clientUsername = username;
}

const std::string& Connection::getUsername()
{
    return m_clientUsername;
}


//void Connection::readLoginMessage(Message& tempMessage, asio::ip::tcp::socket& clientSocket)
//{
//    asio::async_read(clientSocket, asio::buffer(&tempMessage.messageContent, tempMessage.messageSize),
//        [](const asio::error_code& ec, size_t a) {
//            
//        }
//    );
//}
//
//void Connection::readRegistrationMessage(Message& tempMessage, asio::ip::tcp::socket& clientSocket)
//{
//
//}
//
//void Connection::readTextMessage(Message& tempMessage, asio::ip::tcp::socket& clientSocket)
//{
//
//}

