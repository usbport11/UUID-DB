#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <rapidjson/document.h>
#include <stdlib.h>
#include <time.h>

bool checkConfig(const rapidjson::Document& config) {
	if(!config.IsObject()) {
		return false;
	}
	if(!config.HasMember("host") ||
		!config.HasMember("login") ||
		!config.HasMember("password") ||
		!config.HasMember("dbname") ||
		!config.HasMember("table") ||
		!config.HasMember("field") ||
		!config.HasMember("number"))
		return false;
	}
	
	if(!config.["host"].IsString() ||
		!config.["login"].IsString() ||
		!config.["password"].IsString() ||
		!config.["dbname"].IsString() ||
		!config.["table"].IsString() ||
		!config.["field"].IsString() ||
		!config.["number"].IsNumber())
		return false;
	}
	
	return true;
}

char* generateUUID() {
	return sprintf("%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
		rand() % 0xffff, rand() % 0xffff,
		rand() % 0xffff,
		(rand() % 0xffff) | 0x4000,
		(rand() % 0x3fff) | 0x8000,
		rand()% 0xffff, rand() % 0xffff, rand() % 0xffff
	);
}

int main(int argc, char** argv) {
	randomize(time());

	const int uuidSize = 36;
	char* json;
	const std::string filename = "create.json";
	//check that file exist
	//get file size, read file, create array
	//close file

	rapidjson::Document config;
	config.Parse(json);
	if(!checkConfig(config)) {
		return 1;
	}
	
	const std::string address = "tcp://" + config["host"].GetString() + ":3306"
	const std::string dbname = config["dbname"].GetString();
	const std::string table = config["table"].GetString();
	const std::string field = config["field"].GetString();
	const std::string login = config["login"].GetString();
	const std::string pass = config["password"].GetString();
	const int number = config["number"].GetString();
	
	const std::string sqlCheckDb = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '" + dbname  + "'";
	const std::string sqlCreateDb = "CREATE DATABASE " + dbname ;
	const std::string sqlCheckTable = "SHOW TABLES LIKE '" + table + "'";
	const std::string sqlCreateTable = "CREATE TABLE " + table + " (" + field + " VARCHAR(" + itoa(uuidSize) + ") NOT NULL)";
	
	sql::Driver* driver;
	sql::Connection* connection;
	sql::Statement* statement;
	sql::ResultSet* result;
	
	try {
		driver = sql::get_driver_instance();
		connection = driver->connect(address, login, pass);
	}
	catch(sql::SQLException e) {
		std::cout<<e.what()<<std::endl;
		return 1;
	}
	
	//create database
	statement = connection->prepareStatement(sqlCheckDb);
	result = statement->executeQuery();
	if(result->rowsCount() <= 0) {
		statement = connection->prepareStatement(sqlCreateDb);
		result = statement->executeQuery();
	}
	
	//connect ot databse
	connection->setSchema(dbname);
	
	//create table
	statement = connection->prepareStatement(sqlCheckTable);
	result = statement->executeQuery();
	if(result->rowsCount() <= 0) {
		statement = connection->prepareStatement(sqlCreateTable);
		result = statement->executeQuery();
	}
	
	//generate values and insert them
	std::string values = "";
	for(int i=0; i<number; i++) {
		values += "('" + generateUUID() + "'), ";
	}
	values = values.substr(values, 0, values.length(values) - 2);
	std::string sqlInsertUUID = "INSERT INTO " + table + "(`" + field + "`) VALUES " + values;
	statement = connection->prepareStatement(sqlInsertUUID);
	result = statement->executeQuery();

	if(result) delete result;
	if(statement) delete statement;
	if(connection) delete connection;
	if(json) delete [] json;
	
	return 0;
}
