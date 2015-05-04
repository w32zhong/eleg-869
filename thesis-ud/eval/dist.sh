#!/bin/bash
for i in {1..18}
do
	echo -n "$i "
	cat "$1" | grep "query-$i-" | awk '$4=="0" { print $0 }' | wc -l | xargs echo -n ' & '
	cat "$1" | grep "query-$i-" | awk '$4=="1" { print $0 }' | wc -l | xargs echo -n ' & '
	cat "$1" | grep "query-$i-" | awk '$4=="2" { print $0 }' | wc -l | xargs echo -n ' & '
	cat "$1" | grep "query-$i-" | awk '$4=="3" { print $0 }' | wc -l | xargs echo -n ' & ' 
	cat "$1" | grep "query-$i-" | awk '$4=="4" { print $0 }' | wc -l | xargs echo -n ' & '
	cat "$1" | grep "query-$i-" | wc -l | xargs echo -n ' & '
	echo '\\'
done
