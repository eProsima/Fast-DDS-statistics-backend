#!/bin/bash

############################################################
# Script Arguments
############################################################

WORKSPACE_DIR=$(pwd)
FASTDDS_EXAMPLE_EXECUTABLE="${WORKSPACE_DIR}/BasicConfigurationExample/BasicConfigurationExample"

RESULT_PAHT='./result'
RESULT_FILE="fastdds_memory_usage.csv"
LIBRARIES_TO_MEASURE="libfastcdr libfastrtps stack heap anon"

STATISTIC_TOPICS="HISTORY_LATENCY_TOPIC;NETWORK_LATENCY_TOPIC;PUBLICATION_THROUGHPUT_TOPIC;SUBSCRIPTION_THROUGHPUT_TOPIC;RTPS_SENT_TOPIC;RTPS_LOST_TOPIC;HEARTBEAT_COUNT_TOPIC;ACKNACK_COUNT_TOPIC;NACKFRAG_COUNT_TOPIC;GAP_COUNT_TOPIC;DATA_COUNT_TOPIC;RESENT_DATAS_TOPIC;SAMPLE_DATAS_TOPIC;PDP_PACKETS_TOPIC;EDP_PACKETS_TOPIC;DISCOVERY_TOPIC;PHYSICAL_DATA_TOPIC"
SLEEP_TIME_ELAPSE=30
SLEEP_RESIDUAL_TIME=2

PUBLICATION_RATE=10

DEBUG=0

############################################################
# Functions needed in script
############################################################

# pmap result:
# column 1: Initial memory address
# column 2: Virtual memory
# column 3: RSS

# TODO comment
get_memory_usage_from_library () {
    # Get arguments
    PID_=${1}
    LIBRARY_NAME_=${2}

    RESULT_=0

    # Get lines of pmap to check if they exist, if not while call fails
    PMAP_RESULT_="$(pmap ${PID_} | grep ${LIBRARY_NAME_})"

    # If there are lines, analyze them
    if [ ! -z "$PMAP_RESULT_" ];
    then

        # For each line, get the memory value and add it to RESULT
        while read -r line; do

            LINE_=${line}

            # Column 1 ->
            NEW_VALUE_WITH_K_="$(echo -n "${LINE_}" | awk '{print $3}')"

            # This is only needed without --extended to remove K
            # NEW_VALUE_="$(echo -n "${NEW_VALUE_WITH_K_::-1}")"
            NEW_VALUE_=${NEW_VALUE_WITH_K_}

            RESULT_=$((${RESULT_}+${NEW_VALUE_}))

        done <<< "$(pmap ${PID_} --extended | grep ${LIBRARY_NAME_})"

    fi

    # Return the final result (0 if no library in pmap)
    echo -n "${RESULT_}"
}

# TODO comment
print_memory_usage () {
    # Get arguments
    PID_=${1}
    LIBRARY_NAMES_=${2}
    RESULT_FILE_=${3}

    echo "Storing in ${RESULT_FILE_} memory usage from process ${PID_}"

    # Print time
    CURRENT_TIME_="$(date +%s%3N)"
    echo -n "${CURRENT_TIME_}" >> ${RESULT_FILE_}
    echo -n ";" >> ${RESULT_FILE_}

    # Print library memory usage for each library
    for library in ${LIBRARY_NAMES_};
    do
        get_memory_usage_from_library ${PID_} ${library} >> ${RESULT_FILE_}
        echo -n ";" >> ${RESULT_FILE_}
    done

    # Print total
    # TOTAL_MEMORY_USAGE_="$(pmap ${PID_} | tail -n 1 | awk '/[0-9]/K{print $2}')"
    TOTAL_MEMORY_USAGE_="$(pmap ${PID_} --extended | tail -n 1 | awk '{print $4}')"
    # If there are lines, analyze them
    if [ ! -z "$PMAP_RESULT_" ];
    then
        # This is only needed without --extended to remove K
        # echo -n "${TOTAL_MEMORY_USAGE_::-1}" >> ${RESULT_FILE_}
        echo -n "${TOTAL_MEMORY_USAGE_}" >> ${RESULT_FILE_}
    else
        echo -n "0" >> ${RESULT_FILE_}
    fi

    echo "" >> ${RESULT_FILE_}
}


############################################################
# Parse arguments
############################################################

POSITIONAL=()
while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
        --workspace)
        WORKSPACE_DIR="$2"
        shift # past argument
        shift # past value
        ;;
        --fastdds)
        FASTDDS_EXAMPLE_EXECUTABLE="$2"
        shift # past argument
        shift # past value
        ;;

        --result-path)
        RESULT_PAHT="$2"
        shift # past argument
        shift # past value
        ;;
        --result-file)
        RESULT_FILE="$2"
        shift # past argument
        shift # past value
        ;;
        --measurament-libraries)
        LIBRARIES_TO_MEASURE="$2"
        shift # past argument
        shift # past value
        ;;

        --statistics)
        STATISTIC_TOPICS="$2"
        shift # past argument
        shift # past value
        ;;
        --residual-time)
        SLEEP_RESIDUAL_TIME="$2"
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
# Initialize values
############################################################
echo
echo "---------------------------------------------------------------------------"
echo "Starting execution"
echo "---------------------------------------------------------------------------"
echo

RESULT_FILE_PATH=${RESULT_PAHT}/${RESULT_FILE}


############################################################
# Prepare result path and file
############################################################
echo
echo "---------------------------------------------------------------------------"
echo "Initializing result file ${RESULT_FILE_PATH}"
echo "---------------------------------------------------------------------------"
echo

mkdir -p ${RESULT_PAHT}
# Remove file if exists
rm -f ${RESULT_FILE_PATH}
# Initialize file with header. First time
echo -n "time(ms);" > ${RESULT_FILE_PATH}
# Then each of the libraries to measure
for lib in ${LIBRARIES_TO_MEASURE};
do
    echo -n "${lib};" >> ${RESULT_FILE_PATH}
done
# Finally total
echo "total(KB)" >> ${RESULT_FILE_PATH}


############################################################
# Execute backend example
############################################################
echo
echo "---------------------------------------------------------------------------"
echo "Executing Fast DDS exaple ${FASTDDS_EXAMPLE_EXECUTABLE}"
echo "---------------------------------------------------------------------------"
echo

if [ $DEBUG -eq 1 ];
then
    FASTDDS_STATISTICS="${STATISTIC_TOPICS}" ${FASTDDS_EXAMPLE_EXECUTABLE} publisher --interval ${PUBLICATION_RATE} --topic "TestTopic${i}" &
    EXAMPLE_PID=$!
else
    FASTDDS_STATISTICS="${STATISTIC_TOPICS}" ${FASTDDS_EXAMPLE_EXECUTABLE} publisher --interval ${PUBLICATION_RATE} --topic "TestTopic${i}" > /dev/null 2>&1 &
    EXAMPLE_PID=$!
fi

sleep ${SLEEP_RESIDUAL_TIME}

print_memory_usage "${EXAMPLE_PID}" "${LIBRARIES_TO_MEASURE}" "${RESULT_FILE_PATH}"

sleep ${SLEEP_TIME_ELAPSE}

print_memory_usage "${EXAMPLE_PID}" "${LIBRARIES_TO_MEASURE}" "${RESULT_FILE_PATH}"

############################################################
# Finish execution
############################################################
echo
echo "---------------------------------------------------------------------------"
echo "Finishing execution"
echo "---------------------------------------------------------------------------"
echo

# Kill backend example process
kill ${EXAMPLE_PID}
