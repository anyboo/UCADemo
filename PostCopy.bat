@echo off
echo "===== Start Postbuild Process ===== "

set CopyTargetFile=%1
set OutputDir="../../BIN/DEMO/%2"

:Copy Demo folder
echo "Copy to" %OutputDir%
copy %CopyTargetFile% %OutputDir%

:Copy UMF FOlder if exist folder
if NOT "%UDPREL%"=="" copy %CopyTargetFile% "%UDPREL%"\VCA

echo "===== End Postbuild Process ===== "