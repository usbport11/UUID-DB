Visual Studio 2019 CE

Build: 
  MySQL
  Download mysql-installer-community-8.0.28.0.msi from official MySQL site.
  Linker - Input - Property - Additional Dependencies = C:\Program Files (x86)\MySQL\Connector C++ 8.0\lib\vs14\mysqlcppconn.lib
  Linker - Main - Additional library directory = C:\Program Files (x86)\MySQL\Connector C++ 8.0\lib
  C++ - Main - Additional include directory = C:\Program Files (x86)\MySQL\Connector C++ 8.0\include
  C++ - Preprocessor - Definitions = CPPCONN_PUBLIC_FUNC=
  Put near exe dll files: libcrypto-1_1.dll, libssl-1_1.dll, mysqlcppconn-9-vs14.dll

  Json
  Use RapidJSON

  UUID
  Use UUID_v4

Usage:
  In main folder must be config files in json format.
  Create: uuid_db.exe create
  Copy: uuid_db.exe copy
  Compare: uuid_db.exe compare
