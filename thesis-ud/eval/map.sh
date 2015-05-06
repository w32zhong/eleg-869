#!/bin/bash
while read l
do
	echo $l | \
	awk '{
	if ($4 >= 3)
		print $0
	else
		print $1, $2, $3, 0
	}'
done < "$1"
