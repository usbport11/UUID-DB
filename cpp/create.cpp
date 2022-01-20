#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
//#include <jdbc\mysql_driver.h>
#include <jdbc\mysql_connection.h>
#include <jdbc\cppconn\resultset.h>
#include <jdbc\cppconn\driver.h>
#include <jdbc\cppconn\exception.h>
#include <jdbc\cppconn\prepared_statement.h>
#include "rapidjson\document.h"
#include "uuid_v4.h"

char* getFileData(const std::string& fileName) {
	char* fileContent = NULL;

	if (fileName.empty()) {
		std::cout << "File name is NULL" << std::endl;
		return false;
	}
	std::ifstream file(fileName.c_str(), std::ios::binary);
	if (!file.good()) {
		std::cout << "File not exists" << std::endl;
		return false;
	}
	if (file.peek() == std::ifstream::traits_type::eof()) {
		std::cout << "File is empty" << std::endl;
		return false;
	}

	int fileSize = file.tellg();
	file.seekg(0, std::ios::end);
	fileSize = (int)file.tellg() - fileSize;

	fileContent = new char[fileSize + 1];
	memset(fileContent, 0, fileSize + 1);
	file.seekg(0, std::ios::beg);
	if (!file.read(fileContent, fileSize)) {
		std::cout << "Failed to read file" << std::endl;
		return false;
	}
	file.close();

	return fileContent;
}

bool checkConfig(const rapidjson::Document& config) {
	if (!config.IsObject()) {
		return false;
	}
	if (!config.HasMember("host") ||
		!config.HasMember("username") ||
		!config.HasMember("password") ||
		!config.HasMember("dbname") ||
		!config.HasMember("table") ||
		!config.HasMember("field") ||
		!config.HasMember("number")) {
		std::cout << "Some parameter skiped" << std::endl;
		return false;
	}

if (!config["host"].IsString() ||
	!config["username"].IsString() ||
	!config["password"].IsString() ||
	!config["dbname"].IsString() ||
	!config["table"].IsString() ||
	!config["field"].IsString() ||
	!config["number"].IsNumber()) {
		std::cout << "Some parameter wrong has type" << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char** argv) {
 	UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
	UUIDv4::UUID uuid;

	sql::Driver* driver = NULL;
	sql::Connection* connection = NULL;
	sql::Statement* stmt = NULL;
	sql::PreparedStatement* pstmt = NULL;
	sql::ResultSet* result = NULL;

	const int uuidSize = 36;
	char* json;
	const std::string filename = "create.json";

	std::string path = argv[0];
	std::cout<<path<<std::endl;
	int last = path.find_last_of("\\");
	path = path.substr(0, last);
	path += "\\" + filename;

	json = getFileData(path);
	std::cout << path << std::endl;
	if(!json) {
		return 1;
	}

	rapidjson::Document config;
	config.Parse(json);
	if(!checkConfig(config)) {
		return 1;
	}
	if (json) delete[] json;

	std::string address = "tcp://"; 
	address += config["host"].GetString();
	address += ":3306";
	const std::string dbname = config["dbname"].GetString();
	const std::string table = config["table"].GetString();
	const std::string field = config["field"].GetString();
	const std::string login = config["username"].GetString();
	const std::string pass = config["password"].GetString();
	const int number = config["number"].GetInt();

	char buffer[10] = {0};
	_itoa_s(uuidSize, buffer, _countof(buffer), 10);
	const std::string sqlCheckDb = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '" + dbname + "'";
	const std::string sqlCreateDb = "CREATE DATABASE " + dbname;
	const std::string sqlCheckTable = "SHOW TABLES LIKE '" + table + "'";
	const std::string sqlCreateTable = "CREATE TABLE " + table + " (id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY, " + field + " VARCHAR(" + buffer + ") NOT NULL)";

	try {
		driver = get_driver_instance();
		connection = driver->connect(address, login, pass);
	}
	catch (sql::SQLException e) {
		std::cout << e.what() << std::endl;
		return 1;
	}

	//create database
	stmt = connection->createStatement();
	result = stmt->executeQuery(sqlCheckDb);
	if (result->rowsCount() <= 0) {
		std::cout << "Databse not exist" << std::endl;
		try {
			pstmt = connection->prepareStatement(sqlCreateDb);
			pstmt->execute();
		}
		catch (sql::SQLException e) {
			std::cout << e.what() << std::endl;
			return 1;
		}
		std::cout << "Database created" << std::endl;
	}

	//connect ot databse
	connection->setSchema(dbname);

	//create table
	stmt = connection->createStatement();
	result = stmt->executeQuery(sqlCheckTable);
	if (result->rowsCount() <= 0) {
		std::cout << "Table not exist" << std::endl;
		try {
			pstmt = connection->prepareStatement(sqlCreateTable);
			pstmt->execute();
		}
		catch (sql::SQLException e) {
			std::cout << e.what() << std::endl;
			return 1;
		}
		std::cout << "Table created" << std::endl;
	}

	//generate values and insert them
	std::string values = "";
	for (int i = 0; i < number; i++) {
		uuid = uuidGenerator.getUUID();
		values += "('" + uuid.str() + "'), ";
	}
	values = values.substr(0, values.length() - 2);
	std::string sqlInsertUUID = "INSERT INTO " + table + "(`" + field + "`) VALUES " + values;

	try {
		pstmt = connection->prepareStatement(sqlInsertUUID);
		pstmt->executeQuery();
	}
	catch (sql::SQLException e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	std::cout << "UUIDs generated and added to table" << std::endl;

	if (pstmt) delete pstmt;
	if (connection) delete connection;

	std::cout << "Bye!" << std::endl;

	return 0;
}