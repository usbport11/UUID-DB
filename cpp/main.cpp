#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>

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

bool checkCreateConfig(const rapidjson::Document& config) {
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
		std::cout << "Some parameter has wrong type" << std::endl;
		return false;
	}

	return true;
}

bool checkCopyConfig(const rapidjson::Document& config) {
	if (!config.IsObject()) {
		return false;
	}
	if (!config.HasMember("host") ||
		!config.HasMember("username") ||
		!config.HasMember("password") ||
		!config.HasMember("source") ||
		!config.HasMember("destination")) {
		std::cout << "Some parameter skiped" << std::endl;
		return false;
	}
	if (!config["source"].HasMember("dbname") ||
		!config["source"].HasMember("table") ||
		!config["source"].HasMember("field") ||
		!config["destination"].HasMember("dbname") ||
		!config["destination"].HasMember("table") ||
		!config["destination"].HasMember("field")) {
		std::cout << "Some parameter skiped" << std::endl;
			return false;
	}
	if (!config["host"].IsString() ||
		!config["username"].IsString() ||
		!config["password"].IsString() ||
		!config["source"]["dbname"].IsString() ||
		!config["source"]["table"].IsString() ||
		!config["source"]["field"].IsString() ||
		!config["destination"]["dbname"].IsString() ||
		!config["destination"]["table"].IsString() ||
		!config["destination"]["field"].IsString()) {
		std::cout << "Some parameter has wrong type" << std::endl;
		return false;
	}

	return true;
}

bool create(std::string path) {
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

	path += "\\" + filename;
	json = getFileData(path);
	std::cout << path << std::endl;
	if (!json) {
		return false;
	}

	rapidjson::Document config;
	config.Parse(json);
	if (!checkCreateConfig(config)) {
		if (json) delete[] json;
		return false;
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

	char buffer[10] = { 0 };
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
		return false;
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
			return false;
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
			return false;
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
		return false;
	}
	std::cout << "UUIDs generated and added to table" << std::endl;

	if (pstmt) delete pstmt;
	if (connection) delete connection;

	return true;
}

bool copy(std::string path) {
	UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
	UUIDv4::UUID uuid;

	sql::Driver* driver = NULL;
	sql::Connection* connection = NULL;
	sql::Statement* stmt = NULL;
	sql::PreparedStatement* pstmt = NULL;
	sql::ResultSet* result = NULL;

	const int uuidSize = 36;
	char* json;
	const std::string filename = "copy.json";

	path += "\\" + filename;
	json = getFileData(path);
	std::cout << path << std::endl;
	if (!json) {
		return false;
	}

	rapidjson::Document config;
	config.Parse(json);
	if (!checkCopyConfig(config)) {
		if (json) delete[] json;
		return false;
	}
	if (json) delete[] json;

	std::string address = "tcp://";
	address += config["host"].GetString();
	address += ":3306";
	const std::string login = config["username"].GetString();
	const std::string pass = config["password"].GetString();
	const std::string dbnameSource = config["source"]["dbname"].GetString();
	const std::string tableSource = config["source"]["table"].GetString();
	const std::string fieldSource = config["source"]["field"].GetString();
	const std::string dbnameDestination = config["destination"]["dbname"].GetString();
	const std::string tableDestination = config["destination"]["table"].GetString();
	const std::string fieldDestination = config["destination"]["field"].GetString();

	char buffer[10] = { 0 };
	_itoa_s(uuidSize, buffer, _countof(buffer), 10);
	const std::string sqlCheckSourceDb = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '" + dbnameSource + "'";
	const std::string sqlCheckSourceTable = "SHOW TABLES LIKE '" + tableSource + "'";
	const std::string sqlCheckDestinationDb = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '" + dbnameDestination + "'";
	const std::string sqlCreateDestinationDb = "CREATE DATABASE " + dbnameDestination;
	const std::string sqlCheckDestinationTable = "SHOW TABLES LIKE '" + tableDestination + "'";
	const std::string sqlCreateDestinationTable = "CREATE TABLE " + tableDestination + " (id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY, " + fieldDestination + " VARCHAR(" + buffer +  ") NOT NULL)";
	const std::string sqlGetSourceData = "SELECT " + fieldSource + " FROM " + dbnameSource + "." + tableSource;

	try {
		driver = get_driver_instance();
		connection = driver->connect(address, login, pass);
	}
	catch (sql::SQLException e) {
		std::cout << e.what() << std::endl;
		return false;
	}

	//check source db
	stmt = connection->createStatement();
	result = stmt->executeQuery(sqlCheckSourceDb);
	if (result->rowsCount() <= 0) {
		std::cout << "Source database not exist" << std::endl;
		return false;
	}

	//connect ot source databse
	connection->setSchema(dbnameSource);

	//check source table
	stmt = connection->createStatement();
	result = stmt->executeQuery(sqlCheckSourceTable);
	if (result->rowsCount() <= 0) {
		std::cout << "Source table not exist" << std::endl;
		return false;
	}

	//check destination db
	stmt = connection->createStatement();
	result = stmt->executeQuery(sqlCheckDestinationDb);
	if (result->rowsCount() <= 0) {
		std::cout << "Destination database not exist" << std::endl;
		try {
			pstmt = connection->prepareStatement(sqlCreateDestinationDb);
			pstmt->execute();
		}
		catch (sql::SQLException e) {
			std::cout << e.what() << std::endl;
			return false;
		}
		std::cout << "Destination database created" << std::endl;
	}

	//connect ot destination databse
	connection->setSchema(dbnameDestination);

	//check destination table
	stmt = connection->createStatement();
	result = stmt->executeQuery(sqlCheckDestinationTable);
	if (result->rowsCount() <= 0) {
		std::cout << "Destination table not exist" << std::endl;
		try {
			pstmt = connection->prepareStatement(sqlCreateDestinationTable);
			pstmt->execute();
		}
		catch (sql::SQLException e) {
			std::cout << e.what() << std::endl;
			return false;
		}
		std::cout << "Destination table created" << std::endl;
	}

	//get and prepare data
	std::string values = "";
	pstmt = connection->prepareStatement(sqlGetSourceData);
	result = pstmt->executeQuery();
	while (result->next()) {
		values += "('" + result->getString(fieldSource) + "'), "; //error
	}
	if (result) delete result;
	values = values.substr(0, values.length() - 2);
	std::string sqlInsertUUID = "INSERT INTO " + tableDestination + "(`" + fieldDestination + "`) VALUES " + values;

	try {
		pstmt = connection->prepareStatement(sqlInsertUUID);
		pstmt->executeQuery();
	}
	catch (sql::SQLException e) {
		std::cout << e.what() << std::endl;
		return false;
	}
	std::cout << "UUIDs copied to destination table" << std::endl;

	if (pstmt) delete pstmt;
	if (connection) delete connection;

	return true;
}

bool compare(std::string path) {
	sql::Driver* driver = NULL;
	sql::Connection* connection = NULL;
	sql::Statement* stmt = NULL;
	sql::PreparedStatement* pstmt = NULL;
	sql::ResultSet* result = NULL;

	const int uuidSize = 36;
	char* json;
	const std::string filename = "compare.json";

	path += "\\" + filename;
	json = getFileData(path);
	std::cout << path << std::endl;
	if (!json) {
		return false;
	}

	rapidjson::Document config;
	config.Parse(json);
	if (!checkCopyConfig(config)) {
		if (json) delete[] json;
		return false;
	}
	if (json) delete[] json;

	std::string address = "tcp://";
	address += config["host"].GetString();
	address += ":3306";
	const std::string login = config["username"].GetString();
	const std::string pass = config["password"].GetString();
	const std::string dbnameSource = config["source"]["dbname"].GetString();
	const std::string tableSource = config["source"]["table"].GetString();
	const std::string fieldSource = config["source"]["field"].GetString();
	const std::string dbnameDestination = config["destination"]["dbname"].GetString();
	const std::string tableDestination = config["destination"]["table"].GetString();
	const std::string fieldDestination = config["destination"]["field"].GetString();

	const std::string sqlGetSourceData = "SELECT " + fieldSource + " FROM " + dbnameSource + "." + tableSource;
	const std::string sqlGetDestinationData = "SELECT " + fieldDestination + " FROM " + dbnameDestination + "." + tableDestination;

	try {
		driver = get_driver_instance();
		connection = driver->connect(address, login, pass);
	}
	catch (sql::SQLException e) {
		std::cout << e.what() << std::endl;
		return false;
	}

	std::vector<std::string> sourceArray;
	std::vector<std::string> destinationArray;
	std::vector<std::string> diff;

	pstmt = connection->prepareStatement(sqlGetSourceData);
	result = pstmt->executeQuery();
	while (result->next()) {
		sourceArray.push_back(result->getString(fieldSource));
	}
	if (result) delete result;

	pstmt = connection->prepareStatement(sqlGetDestinationData);
	result = pstmt->executeQuery();
	while (result->next()) {
		destinationArray.push_back(result->getString(fieldDestination));
	}
	if (result) delete result;

	std::set_symmetric_difference(std::begin(sourceArray), std::end(sourceArray),
		std::begin(destinationArray), std::end(destinationArray),
		std::back_inserter(diff));

	for (auto& s : diff)
		std::cout << s << std::endl;

	sourceArray.clear();
	destinationArray.clear();

	if (pstmt) delete pstmt;
	if (connection) delete connection;

	return true;
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		std::cout << "Too few parameters" << std::endl;
		return 1;
	}
	if (argc > 2) {
		std::cout << "Too many parameters" << std::endl;
		return 1;
	}

	std::string path = std::experimental::filesystem::current_path().string();
	std::string parameter = argv[1];
	std::transform(parameter.begin(), parameter.end(), parameter.begin(), ::tolower);

	if (!parameter.compare("create")) {
		create(path);
		return 0;
	}
	if (!parameter.compare("copy")) {
		copy(path);
		return 0;
	}
	if (!parameter.compare("compare")) {
		compare(path);
		return 0;
	}

	std::cout << "Unknown parameter" << std::endl;

	return 0;
}
