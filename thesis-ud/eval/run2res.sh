#!/bin/bash
while read l
do
	echo $l | awk '{ print $1, "xxx", $3, $2, -$2, "my_run"}'
done < "$1"
