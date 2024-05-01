#!/bin/bash

# Exit if any command fails
set -e

# Get directory of this script
currentDir=$(dirname $(readlink -f $0))

run_gtest_tests() {
    tests=$1
    output=$2
    ./test --gtest_filter=$tests --gtest_output=json:$output
}

# Number of overall failed tests
failed_tests=0

run() {
    # Parameters
    json_file="$currentDir/results_$1.json"

    # Test case scopes
    tests_continuous="Continuous_*"
    tests_fragmentation="Fragmentation_*"
    tests_general="General_*"

    # Create empty JSON file if not existing
    dateTime=`date +%G-%m-%dT%X%Z`
    cd $currentDir/build/
    touch $json_file
    jq -n "{\"tests\":0,\"failures\":0,\"disabled\":0,\"errors\":0,\"timestamp\":\"$dateTime\",\"time\":\"0s\",\"name\":\"AllTests\",\"testsuites\":[]}" > $json_file

    # Run test cases of scope and store to JSON file
    temp_file_continuous="$currentDir/results_temp_continuous.json"
    run_gtest_tests $tests_continuous $temp_file_continuous
    numTestsTotal=`jq -s '.[0].tests + .[1].tests' $json_file $temp_file_continuous`
    numTestsFailed=`jq -s '.[0].failures + .[1].failures' $json_file $temp_file_continuous`
    numTestsDisabled=`jq -s '.[0].disabled + .[1].disabled' $json_file $temp_file_continuous`
    numTestsError=`jq -s '.[0].errors + .[1].errors' $json_file $temp_file_continuous`
    execTimeTotal=`jq -s '(.[0].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000) + (.[1].time | capture("(?<id>[[:digit:].]+)").id | tonumber * 10000 | round / 10000)' $json_file $temp_file_continuous`
    allTestsuites=`jq -s '.[0].testsuites + .[1].testsuites' $json_file $temp_file_continuous`
    jq -sn "{\"tests\":$numTestsTotal,\"failures\":$numTestsFailed,\"disabled\":$numTestsDisabled,\"errors\":$numTestsError,\"timestamp\":\"$dateTime\",\"time\":\"${execTimeTotal}s\",\"name\":\"AllTests\",\"testsuites\":$allTestsuites}" > $json_file
    rm $temp_file_continuous

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

    # Get failed tests and add to global failed tests
    failed_tests_this=$(jq '.failures | length' $json_file)
    failed_tests=$(($failed_tests + $failed_tests_this))
}

echo "================================================================================"
echo "Run all tests using RSA keys"
echo "================================================================================"
cd $currentDir/..
./TlsCreateCertFiles_rsa.sh
run "rsa"

echo "================================================================================"
echo "Run all tests using EC keys"
echo "================================================================================"
cd $currentDir/..
./TlsCreateCertFiles_ec.sh
run "ec"

# Get number of failed test cases and return
echo "Number of failed tests overall: $failed_tests"
exit $failed_tests
