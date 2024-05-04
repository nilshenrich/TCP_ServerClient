#!/bin/bash

# Get arguments
p_protocol=$1  # "tcp" or "tls"
p_mode=$2      # "continuous", "fragmentation" or "general"
p_certType=$3  # "ec" or "rsa"

# Check arguments
if [ -z "$p_protocol" ] || [ -z "$p_mode" ] || [ -z "$p_certType" ]; then
    echo "Usage: $0 <tcp|tls> <continuous|fragmentation|general> <ec|rsa>"
    exit -1
fi
if [ "$p_protocol" != "tcp" ] && [ "$p_protocol" != "tls" ]; then
    echo "Protocol must be 'tcp' or 'tls'"
    exit -1
fi
if [ "$p_mode" != "continuous" ] && [ "$p_mode" != "fragmentation" ] && [ "$p_mode" != "general" ]; then
    echo "Mode must be 'continuous', 'fragmentation' or 'general'"
    exit -1
fi
if [ "$p_certType" != "ec" ] && [ "$p_certType" != "rsa" ]; then
    echo "Cert type must be 'ec' or 'rsa'"
    exit -1
fi

# Get directory of this script
currentDir=$(dirname $(readlink -f $0))

# Test case scope
test_filter="${p_mode^}_${p_protocol^}*"

# Path to certificate creation script
certScript="TlsCreateCertFiles_$p_certType.sh"

# Path to report file
reportFile="${currentDir}/results.json"

# Create certificates
cd ${currentDir}/../
./$certScript

# Run tests with filter applied
cd ${currentDir}/build/
./test --gtest_filter=$test_filter --gtest_output=json:$reportFile

# Read report file and get number of failed tests
numTestsTotal=$(jq -s '.[0].tests' $reportFile)
numTestsFailed=$(jq -s '.[0].failures' $reportFile)

# Check if report file exists. If not, something went wrong
if [ ! -f "$reportFile" ]; then
    echo "ERROR: Report file does not exist: $reportFile"
    exit -1
fi

# Print number of failed tests and exit with this number
echo "$numTestsFailed tests failed of $numTestsTotal"
exit $numTestsFailed
