ROCKSDB_PATH=/home/wuxiaofei-xy/temp-deploy-auto/badaserver/dataserver/deps/essdb/c_src/libssdb/deps/rocksdb
g++ -std=c++11 -o vtscan_get_all_keys -I$ROCKSDB_PATH/include/ -L$ROCKSDB_PATH vtscan_get_all_keys.cc -lrocksdb -lpthread -lz -lbz2 -lrt -lsnappy
