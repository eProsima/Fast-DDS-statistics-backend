#!/bin/bash

# How to launch from a colcon workspace:
# bash ./src/fastdds_statistics_backend/test/performance/memory_usage/run_memory_usage_test.sh --fastdds ./build/fastrtps/examples/cpp/dds/BasicConfigurationExample/BasicConfigurationExample --backend ./build/fastdds_statistics_backend/examples/cpp/HelloWorldExample/HelloWorldExample

############################################################
# Script Arguments
############################################################

TEST_EXECUTABLE="$(pwd)/test/performance/memory_usage/run_memory_usage_test.sh"
FASTDDS_EXAMPLE_EXECUTABLE="$(pwd)/BasicConfigurationExample/BasicConfigurationExample"
BACKEND_EXAMPLE_EXECUTABLE="$(pwd)/HelloWorldExample/HelloWorldExample"

RESULT_PAHT='./result'

ENTITIES_LOOP_ITERATIONS=20
ENTITIES_LOOP_ELAPSED=60
ENTITIES_IN_LOOP=5
SLEEP_RESIDUAL_TIME=3

PUBLICATION_RATE=10

DEBUG=0

############################################################
# Functions needed in script
############################################################

# TODO comment
execute_memory_usage_test () {
    # Get arguments
    local FILE_NAME=${1}
    local RESET=${2}
    local DUMP=${3}

    if [[ "${RESET}" == 1 ]];
    then
        if [[ "${DUMP}" == 1 ]];
        then
            echo "--- Running test with reset and dump"
            /bin/bash "${TEST_EXECUTABLE}" \
                --fastdds "${FASTDDS_EXAMPLE_EXECUTABLE}" \
                --backend "${BACKEND_EXAMPLE_EXECUTABLE}" \
                --result-path "${RESULT_PAHT}" \
                --result-file "${FILE_NAME}" \
                --loop-iterations "${ENTITIES_LOOP_ITERATIONS}" \
                --loop-elapsed "${ENTITIES_LOOP_ELAPSED}" \
                --loop-entities "${ENTITIES_IN_LOOP}" \
                --publication-rate "${PUBLICATION_RATE}" \
                --reset \
                --dump
        else
            echo "--- Running test with reset and no dump"
            /bin/bash "${TEST_EXECUTABLE}" \
                --fastdds "${FASTDDS_EXAMPLE_EXECUTABLE}" \
                --backend "${BACKEND_EXAMPLE_EXECUTABLE}" \
                --result-path "${RESULT_PAHT}" \
                --result-file "${FILE_NAME}" \
                --loop-iterations "${ENTITIES_LOOP_ITERATIONS}" \
                --loop-elapsed "${ENTITIES_LOOP_ELAPSED}" \
                --loop-entities "${ENTITIES_IN_LOOP}" \
                --publication-rate "${PUBLICATION_RATE}" \
                --reset
        fi
    else
        if [[ "${DUMP}" == 1 ]];
        then
            echo "--- Running test with no reset and dump"
            /bin/bash "${TEST_EXECUTABLE}" \
                --fastdds "${FASTDDS_EXAMPLE_EXECUTABLE}" \
                --backend "${BACKEND_EXAMPLE_EXECUTABLE}" \
                --result-path "${RESULT_PAHT}" \
                --result-file "${FILE_NAME}" \
                --loop-iterations "${ENTITIES_LOOP_ITERATIONS}" \
                --loop-elapsed "${ENTITIES_LOOP_ELAPSED}" \
                --loop-entities "${ENTITIES_IN_LOOP}" \
                --publication-rate "${PUBLICATION_RATE}" \
                --dump
        else
            echo "--- Running test with no reset and no dump"
            /bin/bash "${TEST_EXECUTABLE}" \
                --fastdds "${FASTDDS_EXAMPLE_EXECUTABLE}" \
                --backend "${BACKEND_EXAMPLE_EXECUTABLE}" \
                --result-path "${RESULT_PAHT}" \
                --result-file "${FILE_NAME}" \
                --loop-iterations "${ENTITIES_LOOP_ITERATIONS}" \
                --loop-elapsed "${ENTITIES_LOOP_ELAPSED}" \
                --loop-entities "${ENTITIES_IN_LOOP}" \
                --publication-rate "${PUBLICATION_RATE}"
        fi
    fi


}

############################################################
# Parse arguments
############################################################

POSITIONAL=()
while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
        --test)
        TEST_EXECUTABLE="$2"
        shift # past argument
        shift # past value
        ;;
        --fastdds)
        FASTDDS_EXAMPLE_EXECUTABLE="$2"
        shift # past argument
        shift # past value
        ;;
        --backend)
        BACKEND_EXAMPLE_EXECUTABLE="$2"
        shift # past argument
        shift # past value
        ;;

        --result-path)
        RESULT_PAHT="$2"
        shift # past argument
        shift # past value
        ;;

        --loop-iterations)
        ENTITIES_LOOP_ITERATIONS="$2"
        shift # past argument
        shift # past value
        ;;
        --loop-elapsed)
        ENTITIES_LOOP_ELAPSED="$2"
        shift # past argument
        shift # past value
        ;;
        --loop-entities)
        ENTITIES_IN_LOOP="$2"
        shift # past argument
        shift # past value
        ;;

        --publication-rate)
        PUBLICATION_RATE="$2"
        shift # past argument
        shift # past value
        ;;

        --debug)
        DEBUG=1
        shift # past argument
        ;;

        *)    # unknown option
        POSITIONAL+=("$1") # save it in an array for later
        shift # past argument
        ;;
    esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

read -ra ARGS <<< "${EXPERIMENT_ARGS}"


############################################################
# Prepare result path
############################################################
echo
echo "---------------------------------------------------------------------------"
echo " Starting execution "
echo "---------------------------------------------------------------------------"
echo

execute_memory_usage_test "backend_memory_usage.csv" "0" "0"
execute_memory_usage_test "backend_memory_usage_reset.csv" "1" "0"
execute_memory_usage_test "backend_memory_usage_dump.csv" "0" "1"
execute_memory_usage_test "backend_memory_usage_reset_dump.csv" "1" "1"

############################################################
# Finish execution
############################################################
echo
echo "---------------------------------------------------------------------------"
echo "Finishing execution"
echo "---------------------------------------------------------------------------"
echo
