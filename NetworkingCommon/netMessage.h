#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include <set>

#define USERNAME_MAX_LENGTH 255
#define USERNAME_MIN_LENGTH 5
#define PASSWORD_MIN_LENGTH 5

class Connection;

enum class RegistrationValidationError {
	UsernameNotUnique,
	UsernameTooShort,
	UsernameTooLong,
	UsernameContainsInvalidCharacters,
	EmailContainsInvalidCharacters,
	PasswordTooShort,
	PasswordContainsInvalidCharacters,
	PasswordsNotTheSame
};

enum class MessageType {
	Registration,
	Login,
	TextMessage,
	FriendRequest
};


struct Message {
	Message() : messageSize(0){}
	Message(uint16_t l_messageSize) :
		messageSize(l_messageSize),
		messageContent(l_messageSize)
		{}

	Message(const std::string& messageToSend, MessageType messageType = MessageType::TextMessage) : 
		messageContent(messageToSend.begin(), messageToSend.end()),
		messageType(messageType),
		messageSize(messageContent.size()){}

	Message(const std::string& messageToSend, const std::string& destination, MessageType messageType = MessageType::TextMessage) :
		messageContent(messageToSend.begin(), messageToSend.end()),
		messageType(messageType),
		messageSize(messageContent.size()) {}

	Message(const std::vector<uint8_t>& messageContent, MessageType messageType = MessageType::TextMessage) :
		messageSize(messageContent.size()),
		messageType(messageType),
		messageContent(messageContent) {}
	
	std::vector<uint8_t> messageContent;
	uint16_t messageSize;
	MessageType messageType;
	Connection* sourceConnection;
};
