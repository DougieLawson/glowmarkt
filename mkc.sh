#!/bin/bash

#
#	Glow userid. Use: -uXXX or --username XXX 
#	Glow password. Use:  -pXXX or --password XXX 
#	Glow device ID. Use: -dXXX or --device XXX

#	Mosquitto broker. Use: -bXX.XX.XX.XX or --broker XX.XX.XX.XX or --broker raspberrypi.local

#	MariaDB hostname. Use: -hXX.XX.XX.XX or --sqlhost XX.XX.XX.XX or -sqlhost raspberrpi.local
#	MariaDB username. Use: -sXXX or --sqluser XXX
#	MariaDB DB name. Use: -tXXX or --sqldbase XXX
#	MariaDB password. Use: -qXXX or --sqlpwd XXX


 /home/pi/glowmarkt/mkCfg --user "example@gmail.com" --password  "mysecret" --device "CAFEBEEFDEAD" \
	 --broker 192.168.3.14 --sqlhost 192.168.3.14 --sqluser mysqluser --sqlpwd mysqlsecret --sqldbase EV
