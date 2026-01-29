#!/bin/bash

SHELL=../project/src/mysh

PASS=0
FAIL=0

while read testfile; do
	resultfile="${testfile%.txt}_result.txt"
	outfile="${testfile%.txt}.out"

	echo "Running test: $testfile"
	
	if [[ ! -f "$testfile" ]]; then
		echo " Missing test file: $testfile"
		((FAIL++))
		continue
	fi

	if [[ ! -f "$resultfile" ]]; then
		echo " Missing result file: $resultfile"
		((FAIL++))
		continue
	fi

	$SHELL < "$testfile"  > "$outfile"

	if diff -u "$resultfile" "$outfile" > /dev/null; then
		echo " PASS"
		((PASS++))
	else
		echo " FAIL"
		echo " Diff:"
		diff -u "$resultfile" "$outfile"
		((FAIL++))
	fi

	echo

done < test_list.txt
echo "Summary:"
echo " Passed: $PASS"
echo " Failed: $FAIL"
