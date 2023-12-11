#!/bin/bash

set -eu
set -o pipefail

top_dir=$(dirname ${0})
day=$(basename $(dirname ${0}))

input_file=${top_dir}/../inputs/${day}.txt

cmd=${top_dir}/../build/bin/${day}

${cmd} ${input_file} >& /dev/null

exit 0
