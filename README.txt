Generate, copy, compare uuid tables using php.
In cpp folder c++ analog.

Each script has own config file:
create.json
copy.json
compare.json

Usage (run from php bin directory):

1) create new table with and fill it by generated UUIDs
php e:\xampp\htdocs\uuid\create.php

2) copy from one table to another (select data from source table and insert into destination table)
php e:\xampp\htdocs\uuid.local\copy.php

3) compare between tables (print out unique items)
php e:\xampp\htdocs\uuid.local\compare.php
