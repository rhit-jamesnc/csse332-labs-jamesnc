#!/bin/bash

# run_benchmark.sh v1.0
#
# Create a directory for a new lab based on the template.
#
# Author: Mohammad Noureddine <mohammad.noureddine4@gmail.com>
# Late modified: 2025-11-19
#

VERSION_NUMBER="1.0"
VERSION_DATE="September 2025"

# config variables

# color variables
RED='\033[1;31m'    # red
YELLOW='\033[1;33m' # yellow
GREEN='\033[1;32m'  # green
RESET='\033[0m'     # no color

# Print a warning for the user.
#
# @param First argument is the warning message to print to the user.
#
print_warning() {
  echo -n -e "$YELLOW"
  echo -e "[WARNING] $1"
  echo -n -e "$RESET"
}

# Print an error message for the user
#
# @param First argument is the error message to print to the user.
#
print_error() {
  echo -n -e "$RED"
  echo -e "[ERROR] $1"
  echo -n -e "$RESET"
}

# Print a log for the user.
#
# @param First argument is the log message to print to the user.
#
print_log() {
  echo -n -e "$GREEN"
  echo -e "$1"
  echo -n -e "$RESET"
}
print_logn() {
  echo -n -e "${GREEN}$1${RESET}"
}

# Print the usage description for this program, usually program exits after
# this.
#
print_usage() {
  echo -e "usage: $0 [-h] [-V] -b binary -o filename"
  echo -e ''
  echo -e 'Run the benchmark for a given reading task in lab 02'
  echo -e ''
  echo -e 'options:'
  echo -e '-b BINARY    The path to the binary to execute'
  echo -e '-o FILENAME  The basename of the output file to generate'
  echo -e '-h           Show this help message and exit.'
  echo -e '-V           Print version number and exit.'
}

# Print the version of this program, usually program exits after this.
print_version() {
  echo -e "$0 version $VERSION_NUMBER of $VERSION_DATE"
}

# parse command line arguments
while getopts ":hVb:o:" arg; do
  case $arg in
    b)
      binary_path=${OPTARG}
      ;;
    o)
      outfile=${OPTARG}
      ;;
    V)
      print_version
      exit 0
      ;;
    h | *)
      print_usage
      exit 0
      ;;
  esac
done
shift $((OPTIND-1))

if ! command -v gnuplot &> /dev/null ; then
  print_error "You do not have gnuplot installed"
  echo -e '\t Please install it using the following command:'
  echo -e '\t sudo apt update && sudo apt install gnuplot'
  echo -e ''
  echo -e '\t If `sudo apt update` fails, please run it as follows:'
  echo -e '\t  sudo apt update --fix-missing && sudo apt install gnuplot'
  exit 1
fi

if ! command -v bc &> /dev/null ; then
  print_error "You do not have bc installed"
  echo -e '\t Please install it using the following command:'
  echo -e '\t sudo apt update && sudo apt install bc'
  echo -e ''
  echo -e '\t If `sudo apt update` fails, please run it as follows:'
  echo -e '\t  sudo apt update --fix-missing && sudo apt install bc'
  exit 1
fi

if ! command -v strace &> /dev/null ; then
  print_error "You do not have strace installed"
  echo -e '\t Please install it using the following command:'
  echo -e '\t sudo apt update && sudo apt install strace'
  echo -e ''
  echo -e '\t If `sudo apt update` fails, please run it as follows:'
  echo -e '\t  sudo apt update --fix-missing && sudo apt install strace'
  exit 1
fi

if [[ -z $binary_path ]]; then
  print_error "Please pass a binary file path using -b."
  print_usage
  exit 99
fi

if [[ ! -x $binary_path ]]; then
  print_error "${binary_path} is not an executable file."
  exit 99
fi

if [[ -z $outfile ]]; then
  print_warning "No output file name passed, with use output as default."
  outfile="output.jpg"
fi

BENCH_NUMS=$(seq 1024 1024 $((1024*40)))
num_bench=$(echo "${BENCH_NUMS}" | wc -l)
# BENCH_NUMS=$(seq 1024 1024 $((1024*4096)))

spinner=('/' '-' '\' '|')
spinner_idx=0

echo -e "Running $binary_path on each input number: "

count=0
rm -f dataout.txt
rm -f timedata.txt
for num in ${BENCH_NUMS}; do
  tmp_file="/tmp/${num}.mydata"
  # if (( count % 10 == 0 )); then
    spinner_idx=$(( (spinner_idx + 1) % 4 ))
  # fi
  printf "\r[%s]  Running benchmark $((count + 1))/$num_bench: $num" "${spinner[$spinner_idx]}"
  count=$((count+1))

  if ! strace -c --trace=read -o "${tmp_file}" "${binary_path}" "${num}" &> /dev/null; then
    print_error "Could not run strace on input ${num}, please double check your paths and binary."
    rm -f /tmp/*.mydata
    exit 99
  fi

  # grab the number of read calls from the strace output
  read_calls=$(awk '/read/ {print $4; exit}' "$tmp_file" 2>/dev/null || echo "0")
  echo "${num} ${read_calls}" >> dataout.txt

  # generate timing
  # total_time=$(perf stat -r 50 "${binary_path}" "${num}" 2>&1 | awk '/seconds time elapsed/ {print $1; exit}')
  # total_time=$(echo "${total_time} * 1000" | bc -l)
  total_time="0"
  for(( i = 0; i < 50; i++ ));
  do
    ${binary_path} "${num}" > /dev/null 2> "${tmp_file}"
    exec_time=$(awk '/seconds time elapsed/ {print $1; exit}' < "$tmp_file")
    total_time=$(echo "$total_time + $exec_time" | bc -l)
  done
  total_time=$(echo "${total_time} / 50 * 1000" | bc -l)
  echo "${num} ${total_time}" >> timedata.txt
done
printf "\r%*s" 80 ""
# printf "\e[1A\033[0K Ok."
print_logn "\e[1A\033[0K Ok." && echo ''
echo -e '\t Benchmarks complete, results saved to dataout.txt'

echo -n -e 'Generating plot: '
gnuplot <<EOF
set terminal jpeg size 1000,400 font "Arial,10"
set output "$outfile"

set xlabel "Input Size"
# Left y-axis for read calls
set ylabel "Number of read system calls" textcolor rgb "red"
set ytics nomirror textcolor rgb "red"

# Right y-axis for execution time
set y2label "Time (ms)" textcolor rgb "blue"
set y2tics nomirror textcolor rgb "blue"

set xtics 1024 rotate by -45
set grid

set key top right

plot "dataout.txt" using 1:2 with linespoints title "# of read calls" linewidth 3 pointtype 7 pointsize 0.5 linecolor rgb "red" axes x1y1, \
     "timedata.txt" using 1:2 with linespoints title "Execution Time" linewidth 3 pointtype 7 pointsize 0.5 linecolor rgb "blue" axes x1y2
EOF
print_logn 'Ok.' && echo ''

echo -e -n 'Cleaning up: '
rm -f /tmp/*.mydata
print_logn 'Ok.' && echo ''

