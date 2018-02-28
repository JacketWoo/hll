#!/bin/bash
pid=`ps -ef | grep efan_front | grep -v "grep" | awk '{print $2;}'`

time=60
echo $#
if [ $# -eq 1  ]; then
	time=$1
fi
echo "perf record -e cpu-clock -p ${pid} -g -o efan.perf.data -- sleep ${time}"
perf record -e cpu-clock -p ${pid} -g -o efan.perf.data -- sleep ${time}
echo "perf script -i efan.perf.data > efan.perf"
perf script -i efan.perf.data > efan.perf
echo "/home/wuxiaofei-xy/flame_graph/FlameGraph/stackcollapse-perf.pl efan.perf > efan.folded"
/home/wuxiaofei-xy/flame_graph/FlameGraph/stackcollapse-perf.pl efan.perf > efan.folded
echo "/home/wuxiaofei-xy/flame_graph/FlameGraph/stackcollapse.pl efan.perf > efan.folded"
/home/wuxiaofei-xy/flame_graph/FlameGraph/flamegraph.pl efan.folded > efan.svg
echo "finished"
