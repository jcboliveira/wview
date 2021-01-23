# wview
Unix weather application.
This repository is a change to the original wview. In this release all stations types are dropped except Davis vantage pro+. 
Changes:
1- Add 5 second realtime JSON parameter list the location is
/var/lib/wview/img/ramdisk/json/realtimeparameterlist.txt

2 - Add new parameteres related to Vantage pro+ second loop like apparent temperature. You can find the new parameters JSON in the txtx folder.

This fork is intended for personal weather sites constructions, it doesn't use the original wview site. you can check the work at: https://meteo.ipp.pt

Instalation: First install the original wview and radlib then this version. You can use the Configure Raspbian to configure for any debian flavor.
Aditional packages that you need after the original debian wview release:
libcurl4-openssl-dev libgd-dev libsqlite3-dev libudev-dev libusb-1.0-0-dev libssl-dev
