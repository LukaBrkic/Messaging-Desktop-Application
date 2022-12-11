#include "../inc/Database.h"

#include <mysql/jdbc.h>
#include <iostream>

const std::string DATABASE_NAME = "USERS";
const std::string USER_LOGIN_INFO_TABLE_NAME = "USER_LOGIN_INFO";

using namespace std;

Database::Database() :
	m_driver(nullptr),
	m_con(nullptr),
	m_stmt(nullptr),
	m_prepStmt(nullptr),
	m_res(nullptr)
{
	try {
		m_driver = sql::mysql::get_mysql_driver_instance();
		m_con = m_driver->connect("tcp://127.0.0.1:3306", "root", "OIVHHop08@@");
		m_stmt = m_con->createStatement();
		m_stmt->execute("USE " + DATABASE_NAME);
	}
	catch (const sql::SQLException& e)
	{
		std::cout << e.what() << std::endl;
	}
	//try {
	//	m_prepStmt = m_con->prepareStatement("INSERT INTO a(id, label) VALUES (?, ?)");

	//}
	//catch (const sql::SQLException& e)
	//{
	//	std::cout << e.what() << std::endl;
	//}
	//m_prepStmt->setInt(1, 1);
	//m_prepStmt->setString(2, "a");
	//m_prepStmt->execute();

	//m_prepStmt->setInt(1, 2);
	//m_prepStmt->setString(2, "b");
	//m_prepStmt->execute();

	//m_prepStmt = m_con->prepareStatement("SELECT id, label FROM a WHERE id=?");
	//m_prepStmt->setInt(1, 1);
	//m_res = m_prepStmt->executeQuery();
	//while (m_res->next()) {
	//	// You can use either numeric offsets...
	//	cout << "id = " << m_res->getInt(1); // getInt(1) returns the first column
	//	// ... or column names for accessing results.
	//	// The latter is recommended.
	//	cout << ", label = '" << m_res->getString("label") << "'" << endl;
	//}

}

bool Database::usernameExists(const std::string& username)
{
	try {
		m_prepStmt = m_con->prepareStatement("SELECT username FROM " + USER_LOGIN_INFO_TABLE_NAME + " WHERE username = ? ");
		m_prepStmt->setString(1, username);
		m_res = m_prepStmt->executeQuery();
	}
	catch (const sql::SQLException& e) {
		std::cout << e.what() << std::endl;
	}
	if (m_res->next()) // at least 1 row
		return true;
	return false;
}

bool Database::correctPassword(const std::string& password)
{
	const std::string hash = m_sha256(password);
	try {
		m_prepStmt = m_con->prepareStatement("SELECT password FROM " + USER_LOGIN_INFO_TABLE_NAME + " WHERE password = ? ");
		m_prepStmt->setString(1, hash);
		m_res = m_prepStmt->executeQuery();
	}
	catch (const sql::SQLException& e) {
		std::cout << e.what() << std::endl;
	}
	if (m_res->next()) // at least 1 row
		return true;
	return false;
}


void Database::storeUserLoginInfo(const std::string& username, const std::string& email, const std::string& password)
{
	const std::string hash = m_sha256(password);
	try {
		m_prepStmt = m_con->prepareStatement("INSERT INTO " + USER_LOGIN_INFO_TABLE_NAME + " (username, password, email) VALUES(?, ?, ?)");
		m_prepStmt->setString(1, username);
		m_prepStmt->setString(2, hash);
		m_prepStmt->setString(3, password);
		m_res = m_prepStmt->executeQuery();
	}
	catch (const sql::SQLException& e) {
		std::cout << e.what() << std::endl;
	}
}

Database::~Database() {
	delete m_stmt;
	delete m_con;
	delete m_prepStmt;
	delete m_res;
}