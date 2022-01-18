<?php
require_once("genetateUUID.php");

$uuidLength = 36;
$path = dirname(__FILE__);
$configName = "create.json";
if(!file_exists($path . "\\" . $configName)) {
	die("Config file not found\n");
}
$source = file_get_contents($path . "\\" . $configName);
$config = json_decode($source);
if(!$config) {
	die("Config file error\n");
}

$conn = new mysqli($config->{"host"}, $config->{"username"}, $config->{"password"});
if($conn->connect_error) {
	die("Connection failed: " . $conn->connect_error);
}

$sqlCheckDb = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '" . $config->{"dbname"} . "'";
$sqlCreateDb = "CREATE DATABASE " . $config->{"dbname"} ;
$sqlCheckTable = "SHOW TABLES LIKE '" . $config->{"table"} . "'";
$sqlCreateTable = "CREATE TABLE " . $config->{"table"} . " (" . $config->{"field"} . " VARCHAR(" . strval($uuidLength) . ") NOT NULL)";

$result = $conn->query($sqlCheckDb);
if($result) {
	if($result->num_rows <= 0) {
		if($conn->query($sqlCreateDb)) {
			echo "Database " . $config->{"dbname"} . " created successfully\n";
		}
		else {
			echo "Error creating database: " . $conn->error . "\n";
		}
	}
}
else {
	echo "Error while query: " . $conn->error . "\n";
}

$conn->select_db($config->{"dbname"});

$result = $conn->query($sqlCheckTable);
if($result) {
	if($result->num_rows <= 0) {
		if($conn->query($sqlCreateTable)) {
			echo "Table " . $config->{"table"} . " created successfully\n";
		}
		else {
			echo "Error creating table: " . $conn->error . "\n";
		}
	}
}

$number = $config->{"number"};
$values = "";
for($i=0; $i<$number; $i++) {
	$values .= "('" . generateUUID() . "'), ";
}
$values = substr($values, 0, strlen($values) - 2);

$sqlInsertUUID = "INSERT INTO " . $config->{"table"} . "(`" . $config->{"field"} . "`) VALUES " . $values;
$result = $conn->query($sqlInsertUUID);
if(!$result) {
	echo "Error insert data: " . $conn->error . "\n";
}

$conn->close();
?>
