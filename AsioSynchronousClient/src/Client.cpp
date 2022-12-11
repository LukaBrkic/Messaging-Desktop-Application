#include "../inc/Client.h"
#include "../inc/ClientApp.h"
#include <chrono>
using namespace std::chrono_literals;



Client::Client(const std::string& serverIP, unsigned short serverPort, ClientApp& clientApp) :
    m_serverIP(serverIP),
    m_serverPort(serverPort),
    m_clientApp(clientApp)
{
}

void Client::start()
{
    m_started = true;
    try {
        m_connection = std::make_unique<Connection>(m_incomingQueue, std::move(asio::ip::tcp::socket{ m_asioIOContext }), m_asioIOContext, *this);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    asio::ip::tcp::resolver resolver(m_asioIOContext);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(m_serverIP, std::to_string(m_serverPort));
    
    m_connection->connectToServer(endpoints);

    m_asioExecutionCompletionThread = std::thread([&] {m_asioIOContext.run(); });
    update();
}

void Client::sendRegistrationMessage(std::string&& username, 
                                     std::string&& email, 
                                     std::string&& password, 
                                     std::string&& repeatPassword)
{
    std::string message = username + ";";
    message += password + ";";
    message +=  repeatPassword + ";";
    message += email + ";";
    m_connection->sendMessage(Message(message, MessageType::Registration));
}

void Client::sendLoginMessage(std::string&& username, std::string&& password)
{
    std::string message = username + ";";
    message +=  password + ";";
    m_connection->sendMessage(Message(message, MessageType::Login));
}


void Client::update()
{
    m_updateThread = std::thread
    {
        [&]()
        {
            while (true)
            {
                    if (!m_incomingQueue.empty())
                    {
                        processMessage(m_incomingQueue.popFront());
                    }
            }
        }
    };
}

void Client::processMessage(const Message& message)
{
    if (message.messageType == MessageType::Registration)
        processRegistrationMessage(message);
    else if (message.messageType == MessageType::Login)
        processLoginMessage(message);
    else if (message.messageType == MessageType::TextMessage || message.messageType == MessageType::FriendRequest)
        processTextMessage(message);
}

void Client::processRegistrationMessage(const Message& message)
{
    if (message.messageContent.empty())
        m_clientApp.SetState(1); // go to login success screen;
}

void Client::processLoginMessage(const Message& message)
{
    if (message.messageContent[0] == 'S')
        m_clientApp.SetState(2); // go to main screen;
}

void Client::processTextMessage(const Message& message)
{
    m_clientApp.pushMessage(message);
}

// to do
void Client::sendTextMessage(std::string&& destinationUser, std::string&& message)
{
    std::string messageToSend = destinationUser + ';';
    std::cout << "message to send: " << messageToSend << std::endl;
    messageToSend += message;
    Message m(messageToSend, MessageType::TextMessage);
    m_connection->sendMessage(m);
}

bool Client::connectedToServer()
{
    return m_connection->connectedToServer();
}

bool Client::started()
{
    return m_started;
}

void Client::checkIfFriendExists(std::string&& username)
{
    m_connection->sendMessage(Message(username, MessageType::FriendRequest));
}
