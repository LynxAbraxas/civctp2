mkdir tservt
cd tservt
set OLDINC=%INCLUDE%
set INCLUDE=%INCLUDE%;..\..\..\h;..\..\..\demo\utils;..\..\dp 
cl /W3 /Zi ..\tservt.c  ..\tserv.c ..\tserva.c ..\tca.c ..\tcapw.c ..\crypttab.c ..\..\..\demo\utils\mywcs.c ..\..\3rdparty\d3des\d3des.c ..\..\3rdparty\md5\md5c.c ..\..\..\demo\utils\rawwin.c ..\..\..\win\lib\dp2d.lib /MDd version.lib advapi32.lib user32.lib /Fetservt.exe
set INCLUDE=%OLDINC%
cd ..

