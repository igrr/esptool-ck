#!/bin/bash
#
#
repeat=$1
cmd="${@:2}"

if [[ -z "$repeat" || -z "$cmd" ]]; then
	echo "Usage: test_error_rate.sh <repeat_count> command [ args ... ]"
	echo "For example, to test uploading a binary to nodemcu board 200 times, run:"
	echo "./test_failure_rate.sh 200 ./esptool -cp /dev/tty.SLAB_USBtoUART -cb 921600 -cd nodemcu -cf some_test_file.bin"
	exit 1
fi

success=0
fail=0

for i in $(seq 1 $repeat); do
	echo "Run $i"
	if $cmd; then
		success=$(($success+1))
	else
		fail=$(($fail+1))
	fi
done

echo "Success: $success"
echo "Fail: $fail"
