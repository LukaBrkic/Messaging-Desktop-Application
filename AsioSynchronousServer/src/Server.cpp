#include "../inc/Server.h"

#include <iostream>
#include <algorithm>
#include <iterator>

#include "../inc/sha256.h"
#include "../inc/Util.h"




Server::Server(unsigned short port) : 
    m_port(port),
    m_asioAcceptor(m_asioIOContext, std::move(asio::ip::tcp::endpoint(asio::ip::address::from_string("0.0.0.0"), port)))
{}

void Server::start()
{
    startAccept();
    m_threadContext = std::thread([this]() { m_asioIOContext.run(); });
    update();
}


void Server::startAccept()
{
    m_asioAcceptor.async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket socket) {
        std::cout << "CLIENT CONNECTED" << std::endl;
        if (ec)
            std::cout << ec.message() << std::endl;
        else
        {
            m_unnamedConnections.emplace_back(std::make_unique<Connection>(m_messageQueue, std::move(socket), m_asioIOContext));
        }
        startAccept();
        });
}


void Server::update()
{
    while (true)
    {
        if (!m_messageQueue.empty())
        {
            processMessage(m_messageQueue.popFront());
        }
    }
}

void Server::processMessage(const Message& message)
{
    if (message.messageType == MessageType::Login)
        validateLogin(message);
    else if (message.messageType == MessageType::Registration)
        validateRegistration(message);
    else if (message.messageType == MessageType::TextMessage)
        sendTextMessage(message);
    else if (message.messageType == MessageType::FriendRequest)
        checkIfFriendExists(message);
    // add destination user
    //else if (message.messageType == MessageType::TextMessage)
    //    message.sourceConnection->sendMessage(message);
}

void Server::validateLogin(const Message& message)
{
    std::string username, password;
    Util::extractUserInfo(message.messageContent, username, password);
    if (m_database.usernameExists(username) && m_database.correctPassword(password))
    {
        // smislit nesto bolje
        message.sourceConnection->setUsername(username);
        message.sourceConnection->sendMessage(Message("Success" + username, MessageType::Login));
        m_connectionsByUsername[username] = message.sourceConnection;
    }
    else 
        message.sourceConnection->sendMessage(Message("Fail for user " + username, MessageType::Login));
}


void Server::validateRegistration(const Message& message)
{
    std::string username, email, pass, repeatPass;
    Util::extractUserInfo(message.messageContent, username, email, pass, repeatPass);
    std::vector<uint8_t> registrationErrors;
    if (m_database.usernameExists(username))
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::UsernameNotUnique));
    if (username.length() < USERNAME_MIN_LENGTH)
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::UsernameTooShort));
    else if (username.length() > USERNAME_MAX_LENGTH)
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::UsernameTooLong));
    if (!std::all_of(username.begin(), username.end(), [](const char& ch) {return ch != ';'; }))
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::UsernameContainsInvalidCharacters));

    if (!std::all_of(email.begin(), email.end(), [](const char& ch) {return ch != ';'; }))
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::EmailContainsInvalidCharacters));

    if (pass.length() < PASSWORD_MIN_LENGTH)
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::PasswordTooShort));
    if (!std::all_of(pass.begin(), pass.end(), [](const char& ch) {return ch != ';'; }))
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::PasswordContainsInvalidCharacters));
    if(pass != repeatPass)
        registrationErrors.emplace_back(static_cast<uint8_t>(RegistrationValidationError::PasswordsNotTheSame));

    Message m(registrationErrors, MessageType::Registration);
    message.sourceConnection->sendMessage(m);

    if (registrationErrors.empty())
        storeUserLoginInfo(username, email, pass);
}

void Server::sendTextMessage(const Message& message)
{
    std::string destinationUser, textMessage;
    Util::extractUsernameAndMessage(message.messageContent, destinationUser, textMessage);
    if (m_connectionsByUsername.find(destinationUser) != m_connectionsByUsername.end()) // check if destination is logged in
    {
        // TODO: dodat check if source is logged in

        std::string sourceUser = message.sourceConnection->getUsername();
        Message newMessage(std::string(sourceUser + ';' + textMessage));
        m_connectionsByUsername[destinationUser]->sendMessage(newMessage);
    }
}

void Server::checkIfFriendExists(const Message& message)
{
    std::string username;
    Util::extractUsername(message.messageContent, username);
    if (m_database.usernameExists(username))
    {
        message.sourceConnection->sendMessage(Message("True", MessageType::FriendRequest));
    }
    else
    {
        message.sourceConnection->sendMessage(Message("False", MessageType::FriendRequest));
    }

}


void Server::storeUserLoginInfo(const  std::string& username, const  std::string& email, std::string& pass)
{
    m_database.storeUserLoginInfo(username, email, pass);
}








