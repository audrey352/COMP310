#!/bin/bash

SHELL=../src/mysh

PASS=0
FAIL=0

while read testfile; do
    resultfile="${testfile%.txt}_result.txt"
    resultfile2="${testfile%.txt}_result2.txt"
    outfile="${testfile%.txt}.out"

    echo "Running test: $testfile"

    if [[ ! -f "$testfile" ]]; then
        echo " Missing test file: $testfile"
        ((FAIL++))
        continue
    fi

    # ---------- AGING tests: allow 2 result files ----------
    if [[ "$testfile" == *AGING* ]]; then
        # at least one result file must exist
        if [[ ! -f "$resultfile" && ! -f "$resultfile2" ]]; then
            echo " Missing both result files for AGING test: $resultfile / $resultfile2"
            ((FAIL++))
            continue
        fi

        $SHELL < "$testfile" > "$outfile"

        pass=false
        # check first result
        if [[ -f "$resultfile" ]]; then
            if diff -u "$resultfile" "$outfile" > /dev/null; then
                pass=true
            fi
        fi
        # check second result
        if [[ "$pass" == false && -f "$resultfile2" ]]; then
            if diff -u "$resultfile2" "$outfile" > /dev/null; then
                pass=true
            fi
        fi

        if [[ "$pass" == true ]]; then
            echo " PASS (matches one of the result files)"
            ((PASS++))
        else
            echo " FAIL (output matches neither result file)"
            echo " Diff with result.txt:"
            [[ -f "$resultfile" ]] && diff -u "$resultfile" "$outfile"
            echo " Diff with result2.txt:"
            [[ -f "$resultfile2" ]] && diff -u "$resultfile2" "$outfile"
            ((FAIL++))
        fi

        echo
        continue
    fi

    # ---------- MT tests: order does not matter ----------
    if [[ "$testfile" == *MT* ]]; then
        $SHELL < "$testfile" > "$outfile"
        sort "$outfile" > "${outfile}.sorted"
        sort "$resultfile" > "${resultfile}.sorted"

        if diff -u "${resultfile}.sorted" "${outfile}.sorted" > /dev/null; then
            echo " PASS (unordered match)"
            ((PASS++))
        else
            echo " FAIL (unordered mismatch)"
            echo " Diff (sorted):"
            diff -u "${resultfile}.sorted" "${outfile}.sorted"
            ((FAIL++))
        fi

        rm -f "${outfile}.sorted" "${resultfile}.sorted"
        echo
        continue
    fi

    # ---------- normal tests ----------
    $SHELL < "$testfile" > "$outfile"
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