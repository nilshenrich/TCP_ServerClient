#!/bin/bash

# Get directory of this script
currentDir=$(dirname $(readlink -f $0))

run_gtest_tests() {
    tests=$1
    output=$2
    ./networktester --gtest_filter=$tests --gtest_output=json:$output
}

# Test case scopes
tests_forwarding="Forwarding_*"
tests_fragmentation="Fragmentation_*"
tests_general="General_*"

# Create empty JSON file if not existing
dateTime=`date +%G-%m-%dT%X%Z`
cd $currentDir/build/
json_file="$currentDir/results.json"
touch $json_file
jq -n "{\"tests\":0,\"failures\":0,\"disabled\":0,\"errors\":0,\"timestamp\":\"$dateTime\",\"time\":\"0s\",\"name\":\"AllTests\",\"testsuites\":[]}" > $json_file

# Run test cases of scope and store to JSON file
temp_file_forwarding="$currentDir/results_temp_forwarding.json"
run_gtest_tests $tests_forwarding $temp_file_forwarding
numTestsTotal=`jq -s '.[0].tests + .[1].tests' $json_file $temp_file_forwarding`
numTestsFailed=`jq -s '.[0].failures + .[1].failures' $json_file $temp_file_forwarding`
numTestsDisabled=`jq -s '.[0].disabled + .[1].disabled' $json_file $temp_file_forwarding`
numTestsError=`jq -s '.[0].errors + .[1].errors' $json_file $temp_file_forwarding`
execTimeTotal=`jq -s '(.[0].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000) + (.[1].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000)' $json_file $temp_file_forwarding`
allTestsuites=`jq -s '.[0].testsuites + .[1].testsuites' $json_file $temp_file_forwarding`
jq -sn "{\"tests\":$numTestsTotal,\"failures\":$numTestsFailed,\"disabled\":$numTestsDisabled,\"errors\":$numTestsError,\"timestamp\":\"$dateTime\",\"time\":\"${execTimeTotal}s\",\"name\":\"AllTests\",\"testsuites\":$allTestsuites}" > $json_file
rm $temp_file_forwarding

temp_file_fragmentation="$currentDir/results_temp_fragmentation.json"
run_gtest_tests $tests_fragmentation $temp_file_fragmentation
numTestsTotal=`jq -s '.[0].tests + .[1].tests' $json_file $temp_file_fragmentation`
numTestsFailed=`jq -s '.[0].failures + .[1].failures' $json_file $temp_file_fragmentation`
numTestsDisabled=`jq -s '.[0].disabled + .[1].disabled' $json_file $temp_file_fragmentation`
numTestsError=`jq -s '.[0].errors + .[1].errors' $json_file $temp_file_fragmentation`
execTimeTotal=`jq -s '(.[0].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000) + (.[1].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000)' $json_file $temp_file_fragmentation`
allTestsuites=`jq -s '.[0].testsuites + .[1].testsuites' $json_file $temp_file_fragmentation`
jq -sn "{\"tests\":$numTestsTotal,\"failures\":$numTestsFailed,\"disabled\":$numTestsDisabled,\"errors\":$numTestsError,\"timestamp\":\"$dateTime\",\"time\":\"${execTimeTotal}s\",\"name\":\"AllTests\",\"testsuites\":$allTestsuites}" > $json_file
rm $temp_file_fragmentation

temp_file_general="$currentDir/results_temp_general.json"
run_gtest_tests $tests_general $temp_file_general
numTestsTotal=`jq -s '.[0].tests + .[1].tests' $json_file $temp_file_general`
numTestsFailed=`jq -s '.[0].failures + .[1].failures' $json_file $temp_file_general`
numTestsDisabled=`jq -s '.[0].disabled + .[1].disabled' $json_file $temp_file_general`
numTestsError=`jq -s '.[0].errors + .[1].errors' $json_file $temp_file_general`
execTimeTotal=`jq -s '(.[0].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000) + (.[1].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000)' $json_file $temp_file_general`
allTestsuites=`jq -s '.[0].testsuites + .[1].testsuites' $json_file $temp_file_general`
jq -sn "{\"tests\":$numTestsTotal,\"failures\":$numTestsFailed,\"disabled\":$numTestsDisabled,\"errors\":$numTestsError,\"timestamp\":\"$dateTime\",\"time\":\"${execTimeTotal}s\",\"name\":\"AllTests\",\"testsuites\":$allTestsuites}" > $json_file
rm $temp_file_general

# Get number of failed test cases and return
failed_tests=$(jq '.failures | length' $json_file)
echo "Number of failed tests: $failed_tests"
exit $failed_tests
