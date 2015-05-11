#!/bin/bash
while read l
do
	docID=`echo $l | awk '{ print $3 }'`
	oldScore=`echo $l | awk '{ print $4 }'`
	cat "$2" | grep "$docID" > /dev/null
	if [ ${PIPESTATUS[1]} -eq 0 ] 
	then
		newScore=`cat "$2" | grep "$docID" | awk '{print $4}'`
		if [ $oldScore == $newScore ] 
		then
			echo "$l"
		else
			echo "$oldScore != $newScore" >> correct.log
			fmt='{print $1,$2,$3,"'
			fmt+=${newScore}
			fmt+='"}'
			#echo $fmt
			echo "$l" | awk "${fmt}" 
		fi
	else
		echo "$l"
	fi
done < "$1"
