<?php
$uuidLength = 36;
$path = dirname(__FILE__);
$configName = "copy.json";
$source = file_get_contents($path . "\\" . $configName);
$config = json_decode($source);

$conn = new mysqli($config->{"host"}, $config->{"username"}, $config->{"password"});
if($conn->connect_error) {
	die("Connection failed: " . $conn->connect_error);
}

$sqlCheckSourceDb = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '" . $config->{"source"}->{"dbname"} . "'";
$sqlCheckDestinationDb = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '" . $config->{"destination"}->{"dbname"} . "'";
$sqlCreateDestinationDb = "CREATE DATABASE " . $config->{"destination"}->{"dbname"} ;
$sqlCheckDestinationTable = "SHOW TABLES LIKE '" . $config->{"destination"}->{"table"} . "'";
$sqlCreateDestinationTable = "CREATE TABLE " . $config->{"destination"}->{"table"} . " (" . $config->{"destination"}->{"field"} . " VARCHAR(" . strval($uuidLength) . ") NOT NULL)";
$sqlGetSourceData = "SELECT " . $config->{"source"}->{"field"} . " FROM " . $config->{"source"}->{"dbname"} . "." . $config->{"source"}->{"table"};

$result = $conn->query($sqlCheckSourceDb);
if($result) {
	if($result->num_rows <= 0) {
		die("Source database not exist");
	}
}
else {
	echo "Error while query: " . $conn->error . "\n";
}

$result = $conn->query($sqlCheckDestinationDb);
if($result) {
	if($result->num_rows <= 0) {
		if($conn->query($sqlCreateDestinationDb)) {
			echo "Database " . $config->{"destination"}->{"dbname"} . " created successfully\n";
		}
		else {
			die("Error creating database: " . $conn->error . "\n");
		}
	}
}
else {
	die("Error while query: " . $conn->error . "\n");
}

$conn->select_db($config->{"destination"}->{"dbname"});

$result = $conn->query($sqlCheckDestinationTable);
if($result) {
	if($result->num_rows <= 0) {
		if($conn->query($sqlCreateDestinationTable)) {
			echo "Table " . $config->{"destination"}->{"table"} . " created successfully\n";
		}
		else {
			die("Error creating table: " . $conn->error . "\n");
		}
	}
}

$values = "";
$result = $conn->query($sqlGetSourceData);
if($result) {
	if($result->num_rows <= 0) {
		die("No records in source table");
	}
	while ($row = $result->fetch_array(MYSQLI_ASSOC)) {
		$values .= "('" . $row[$config->{"source"}->{"field"}] . "'), ";
	}
	$values = substr($values, 0, strlen($values) - 2);
}

$sqlInsertUUID = "INSERT INTO " . $config->{"destination"}->{"table"} . "(`" . $config->{"destination"}->{"field"} . "`) VALUES " . $values;

$result = $conn->query($sqlInsertUUID);
if(!$result) {
	echo "Error insert data: " . $conn->error . "\n";
}

$conn->close();
?>