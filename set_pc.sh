#!/bin/bash
###this script is to bind threads to difference cpus
threads=`ps -eLf | grep "16415.*/server" | grep -v "grep\|tmux" | awk '{print $4}'`
i=0
for thread in $threads
do
  #echo $thread
  taskset -pc $i $thread
  #taskset -pc $thread
  let ++i
done
