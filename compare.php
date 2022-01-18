<?php
$path = dirname(__FILE__);
$configName = "compare.json";
$source = file_get_contents($path . "\\" . $configName);
if(!$source) {
	die();
}


$config = json_decode($source);

$conn = new mysqli($config->{"host"}, $config->{"username"}, $config->{"password"});
if($conn->connect_error) {
	die("Connection failed: " . $conn->connect_error);
}

$sqlGetSourceData = "SELECT " . $config->{"source"}->{"field"} . " FROM " . $config->{"source"}->{"dbname"} . "." . $config->{"source"}->{"table"};
$sqlGetDestinationData = "SELECT " . $config->{"destination"}->{"field"} . " FROM " . $config->{"destination"}->{"dbname"} . "." . $config->{"destination"}->{"table"};
$arraySource = array();
$arrayDestination = array();

$result = $conn->query($sqlGetSourceData);
if($result) {
	if($result->num_rows <= 0) {
		die("No records in source table");
	}
	while ($row = $result->fetch_array(MYSQLI_ASSOC)) {
		array_push($arraySource, $row[$config->{"source"}->{"field"}]);
	}
}
else {
	die("Error select data: " . $conn->error . "\n");
}

$result = $conn->query($sqlGetDestinationData);
if($result) {
	if($result->num_rows <= 0) {
		die("No records in destination table");
	}
	while ($row = $result->fetch_array(MYSQLI_ASSOC)) {
		array_push($arrayDestination, $row[$config->{"destination"}->{"field"}]);
	}
}
else {
	die("Error select data: " . $conn->error . "\n");
}

$conn->close();

$diff = array_diff($arraySource, $arrayDestination);

echo "Diff between " . $config->{"source"}->{"dbname"} . "." . $config->{"source"}->{"table"} . " and " . $config->{"destination"}->{"dbname"} . "." . $config->{"destination"}->{"table"} . ":\n";
print_r($diff);
?>