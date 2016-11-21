#!/bin/bash 

LOG_FILE="./log.raw.txt"

sudo hciconfig hci0 up
gatttool -b 02:80:E1:00:00:06 --char-write-req -a 0x0026 -n 03 || exit 1 
echo "Begin read LOG"
rm $LOG_FILE
for i in {1..10090}; do 
   TMP=$(echo -ne "$i:\t")
   echo -ne $TMP
   echo -ne $TMP >> $LOG_FILE
   TMP=$(gatttool -b 02:80:E1:00:00:06 --char-read -a 0x0024)
   if [ ! "$TMP" ]; then
      sudo hciconfig hci0 down
      sleep 1
      sudo hciconfig hci0 up
      sleep 1
      gatttool -b 02:80:E1:00:00:06 --char-write-req -a 0x0026 -n 03
   fi
   echo $TMP 
   echo $TMP >> $LOG_FILE; 
done

