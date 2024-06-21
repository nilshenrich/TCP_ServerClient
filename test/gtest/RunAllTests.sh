#!/bin/bash

# Get directory of this script
currentDir=$(dirname $(readlink -f $0))

# Path to report file
reportFile="${currentDir}/results.json"

# List of protocols to test
PROTOCOLS="tcp tls"

# List of modes to test
MODES="continuous fragmentation general"

# List of certificate types to test
CERTS="ec rsa"

# Text colors to be used
col_fail='\033[0;31m' # red
col_pass='\033[0;32m' # green
col_auto='\033[0m' # neutral color (system default)

# Run all tests in all combinations
finalResult=""
executionConter=0
for p_protocol in $PROTOCOLS
do
    for p_mode in $MODES
    do
        for p_certType in $CERTS
        do
            # Run tests
            executionConter=$((executionConter+1))
            echo "Running tests with protocol: $p_protocol, mode: $p_mode, cert type: $p_certType"
            ./RunFilteredTests.sh $p_protocol $p_mode $p_certType

            # Get number of failed tests and total number of tests and append to final results
            numTestsTotal=$(jq -s '.[0].tests' $reportFile)
            numTestsFailed=$(jq -s '.[0].failures' $reportFile)
            finalResult+=$(printf %4s "[$executionConter]") # Add execution counter as ID
            if [ $numTestsFailed -eq 0 ]; then # Text color if based on passed or failed
                finalResult+="${col_pass}"
            else
                finalResult+="${col_fail}"
            fi
            finalResult+=$(printf %13s "$numTestsFailed/$numTestsTotal") # Add number of failed tests and total
            finalResult+=${col_auto} # Continue with default text color
            finalResult+=$(printf %9s "$p_protocol") # Add protocol
            finalResult+=$(printf %14s "$p_mode") # Add mode
            finalResult+=$(printf %9s "$p_certType") # Add cert type
            finalResult+="\n"

            # Rename report file to include protocol, mode and cert type
            mv $reportFile "${reportFile%.*}_${p_protocol}_${p_mode}_${p_certType}.${reportFile##*.}"
        done
    done
done

# Print final results
echo "Final results:"
echo "  ID Failed/Total Protocol          Mode CertType"
echo -e "$finalResult"
