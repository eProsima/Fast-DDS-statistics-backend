#!/usr/bin/env bash

set -e

idl_files=(
    './test/dds/communication/Communication.idl'
    './include/fastdds_statistics_backend/topic_types/monitorservice_types.idl'
    './include/fastdds_statistics_backend/topic_types/types.idl'
    './examples/cpp/HelloWorldExample/HelloWorld.idl'
)

red='\E[1;31m'
yellow='\E[1;33m'
textreset='\E[1;0m'

current_dir=$(git rev-parse --show-toplevel)

if [[ ! "$(pwd -P)" -ef "$current_dir" ]]; then
    echo -e "${red}This script must be executed in the repository root directory.${textreset}"
    exit 1
fi

if [[ -z "$(which fastddsgen)" ]]; then
    echo "Cannot find fastddsgen. Please, include it in PATH environment variable"
    exit 1
fi

ret_value=0

for idl_file in "${idl_files[@]}"; do
    echo -e "Processing ${yellow}$idl_file${textreset}"

    fastddsgen "$idl_file" -replace

    if [[ $? != 0 ]]; then
        ret_value=1
    fi
done

exit $ret_value
