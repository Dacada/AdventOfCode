#!/bin/sh
set -e

RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m'

for year in $(ls -dv ????)
do
    for file in $(ls -v $year/*.c)
    do
	day=$(basename ${file%.*})

	if [[ $day == "intcode" ]]
	then
	    continue
	fi

	for part in {1..2}
	do
	    output=$(./aoc --plain-output $year $day $part)

	    time=$(echo $output | cut -d '|' -f 1 | cut -d ',' -f 1)
	    status_code=$(echo $output | cut -d  '|' -f 1 | cut -d  ',' -f 9)

	    result=$(echo $output | cut -d '|' -f 2)
	    expected=$(jq -r .'"'$year'"'.'"'$day'"'.'"'$part'"' solutions.json)

	    if [[ $status_code != "0" ]]
	    then
		status=${RED}"FAILED"${NC}
	    elif [[ $expected != "null" && $expected != $result ]]
	    then
		status=${RED}"WRONG"${NC}
	    elif (( $(echo "$time > 1" | bc -l) ))
	    then
		status=${YELLOW}"SLOW"${NC}
	    else
		status=${GREEN}"OK"${NC}
	    fi

	    echo -e "[$status] $year Day $day Part $part (${time}s)"
	done
    done
done
