#!/bin/bash
# Written by Furey.
# With additions from Tony and Alander.

# Set the port number.
port=5000

if [ "$1" != "" ]
then
	port="$1"
fi

# Set limits.
if [ -e shutdown.txt ]
then
	rm -f shutdown.txt
fi

while [ 1 ]
do
    index=1000
    
    while [ 1 ]
    do
    	if [ -e ../files/logs/$index.log ]
    	then
    		let "index=($index+1)"
    	else
    		break;
    	fi
    done

    nice -n 19 ../src/rom $port &> ../files/logs/$index.log 
    
    if [ -e ../files/gamefiles/shutdown.txt ]
    then
    	rm -f ../files/gamefiles/shutdown.txt
    	exit 0
    fi
    
    sleep 3
done
