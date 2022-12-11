#pragma once
#include <string>
#include "../inc/sha256.h"


namespace sql 
{
	class Connection;
	class Statement;
	class PreparedStatement;
	class ResultSet;
	namespace mysql 
	{
		class MySQL_Driver;
	} // mysql
	
} // sql



class Database
{
public:
	Database();
	~Database();

	bool uniqueUsername(const std::string& username);
	bool usernameExists(const std::string& username);
	bool correctPassword(const std::string& password);
	void storeUserLoginInfo(const std::string& username, const std::string& email, const std::string& password);

private:
	sql::mysql::MySQL_Driver* m_driver;
	sql::Connection* m_con;
	sql::PreparedStatement* m_prepStmt;
	sql::Statement* m_stmt;
	sql::ResultSet* m_res;
	SHA256 m_sha256;
};

