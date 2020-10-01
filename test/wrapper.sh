#! /usr/bin/env bash

set -o errexit
set -o nounset


function log_info()
{
  log_ "info" "$@"
}

function log_error()
{
  log_ "error" "$@"
}

function log_()
{
  local type=$1
  local message=$2
  local date_str=; date_str=$(date +'%Y-%m-%d %H:%M:%S')
  echo "[${type}] [${date_str}] ${message}"
}

function main()
{
  log_info "Stage #1 Compiling..."
  if ! (cd ../src && make) ; then
    log_error "Failed to compile."
    return 1
  fi


  log_info "Stage #2 Test generating..."
  if ! python3 generator.py . ; then
    log_error "Failed to generate tests."
    return 1
  fi

  log_info "Stage #3 Checking..."
  for test_file in $( ls *.txt ) ; do
    if ! ../src/lab2 < ${test_file} ; then
      local ret=$(echo $?)
      if (( $ret==2 )) ; then
        log_error "Division by zero"
        return 2
      fi
      if (( $ret==1 )) ; then
        log_error "Failed to run test"
        return 1
      fi
    fi
  done
}

main $@
