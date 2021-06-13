# glowmarkt
Some API samples for Glowmarkt MQTT

This package needs the following pre-requisites:

libmysql-dev (or MariaDB equivalent)

https://github.com/json-c/json-c

libcurl-dev

libmosquitto

cmake


Installation
------------

```
git clone https://github.com/DougieLawson/glowmarkt
cd glowmarkt 
mkdir build	
cd build
cmake ..
make
mysql -h 192.168.3.14 -u MySQLUser -p  < gas_usage.sql
```


