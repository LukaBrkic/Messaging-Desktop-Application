#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <deque>
#include <thread>

#include <asio.hpp>

#include "../inc/Database.h"
#include "../../NetworkingCommon/netMessage.h"
#include "../../NetworkingCommon/threadSafeQueue.h"
#include "Connection.h"



class Server {
public:
    Server(unsigned short port);
    void update();

    void processMessage(const Message& message);
    void validateLogin(const Message& message);
    void validateRegistration(const Message& message);
    void sendTextMessage(const Message& message);
    void checkIfFriendExists(const Message& message);

    void start();
    void storeUserLoginInfo(const  std::string& username, const  std::string& email, std::string& pass);

    void startAccept();



private:
    Database m_database;
    ThreadSafeQueue m_messageQueue;
    std::vector<std::unique_ptr<Connection>> m_unnamedConnections;
    std::vector<std::unique_ptr<Connection>> m_loggedInUsers; // dodat kasnije
    std::unordered_map<std::string, Connection*> m_connectionsByUsername;
    unsigned short m_port;
    asio::io_context m_asioIOContext;
    asio::ip::tcp::acceptor m_asioAcceptor;
    asio::error_code m_asioErrorCode;
    std::thread m_threadContext;
};