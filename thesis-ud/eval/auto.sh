#!/bin/bash
./run2res.sh run-test.txt > res-test.txt
./trec_eval run-test.txt res-test.txt
