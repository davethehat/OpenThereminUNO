#!/bin/bash

case `uname` in
"Darwin")
arduino=/Applications/Arduino.app/Contents/MacOS/JavaApplicationStub
;;
*)
arduino=arduino
;;
esac


preferences=${PWD}/preferences.txt
sketch=${PWD}/Open_Theremin_UNO.ino
cmd=${1}
shift



case ${cmd} in
'verify')
${arduino} --preferences-file ${preferences} --verify --verbose ${*} ${sketch}
;;
'upload')
${arduino} --preferences-file ${preferences} --upload ${*} ${sketch}
;;
*)
echo "Unknown command: ${cmd}"
;;
esac
