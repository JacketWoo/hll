#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "rocksdb/iterator.h"

using namespace rocksdb;

class Decoder{
private:
  const char *p;
  int size;
  Decoder(){}
public:
  Decoder(const char *p, int size){
    this->p = p;
    this->size = size;
  }
  int skip(int n){
    if(size < n){
      return -1;
    }
    p += n;
    size -= n;
    return n;
  }
  int read_int64(int64_t *ret){
    if(size < sizeof(int64_t)){
      return -1;
    }
    if(ret){
      *ret = *(int64_t *)p;
    }
    p += sizeof(int64_t);
    size -= sizeof(int64_t);
    return sizeof(int64_t);
  }
  int read_uint64(uint64_t *ret){
    if(size < sizeof(uint64_t)){
      return -1;
    }
    if(ret){
      *ret = *(uint64_t *)p;
    }
    p += sizeof(uint64_t);
    size -= sizeof(uint64_t);
    return sizeof(uint64_t);
  }
  int read_data(std::string *ret){
    int n = size;
    if(ret){
      ret->assign(p, size);
    }
    p += size;
    size = 0;
    return n;
  }
  int read_8_data(std::string *ret=NULL){
    if(size < 1){
      return -1;
    }
    int len = (uint8_t)p[0];
    p += 1;
    size -= 1;
    if(size < len){
      return -1;
    }
    if(ret){
      ret->assign(p, len);
    }
    p += len;
    size -= len;
    return 1 + len;
  }
};

int decode_hash_key(const Slice &slice, std::string *name, std::string *key){
  Decoder decoder(slice.data(), slice.size());
  if(decoder.skip(1) == -1){
    return -1;
  }
  if(decoder.read_8_data(name) == -1){
    return -1;
  }
  if(decoder.skip(1) == -1){
    return -1;
  }
  if(decoder.read_data(key) == -1){
    return -1;
  }
  return 0;
}

int decode_hsize_key(const Slice &slice, std::string *name){
  Decoder decoder(slice.data(), slice.size());
  if(decoder.skip(1) == -1){
    return -1;
  }
  if(decoder.read_data(name) == -1){
    return -1;
  }
  return 0;
}

void get_src_dest_db_path(int32_t partition_num, std::string *src_db_path, std::string *dst_file_path) {
  //this is for 1024 total partitions and is for vtscan
  assert(partition_num >= 0 && partition_num < 1024);
  *dst_file_path = "/data/vtscan/";
  if (access(dst_file_path->c_str(), F_OK) != 0) {
    if (mkdir(dst_file_path->c_str(), 0755) != 0) {
      fprintf(stderr, "mkdir error , dir: %s\n", dst_file_path->c_str());
      exit(-1);
    }
  }
  if (partition_num < 256) {
    *src_db_path = "/data1/bada/data/11/vtscan/" + std::to_string(partition_num) + "/";
  } else if (partition_num < 512) {
    *src_db_path = "/data2/bada/data/12/vtscan/" + std::to_string(partition_num) + "/";
  } else if (partition_num < 768) {
    *src_db_path = "/data3/bada/data/13/vtscan/" + std::to_string(partition_num) + "/";
  } else {
    *src_db_path = "/data4/bada/data/14/vtscan/" + std::to_string(partition_num) + "/";
  }
  *dst_file_path = *dst_file_path + std::to_string(partition_num);
}




std::string kDBPath;
std::string key_file_path;
int main(int argc, char* argv[]) {
//  assert(argc == 2);
  if (argc != 2) {
    fprintf(stderr, "Wrong usage\n");
    fprintf(stderr, "Usage:\n"
                    "      ./vtscan_get_all_keys partition_num\n");
    return -1;
  }
  int32_t partition_num = atoi(argv[1]);
  fprintf(stderr, "partition %d starts\n", partition_num);
  time_t start_s = time(NULL);
  get_src_dest_db_path(partition_num, &kDBPath, &key_file_path);

  DB* db;
  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB for only read
  Status s = DB::OpenForReadOnly(options, kDBPath, &db);
  assert(s.ok());
  // open key store file
  int fd = open(key_file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC);
  if (fd == -1) {
    fprintf(stderr, "open key store file error!\n");
    return -1;
  }

  ReadOptions read_options;
  read_options.fill_cache = false;
  Iterator *iter = db->NewIterator(read_options);
  iter->Seek("H");
  Slice rocksdb_key;
  std::string name, pre_name;
  int ret;
  while (iter->Valid()) {
    rocksdb_key = iter->key();
    if (rocksdb_key.size() == 0 || rocksdb_key.data()[0] != 'H') {
      break;
    }
    ret = decode_hsize_key(rocksdb_key, &name);
    assert(ret == 0);
    if (name == pre_name) {
      iter->Next();  
      continue;
    }
    write(fd, name.data(), name.size());
    write(fd, "\n", 1);
    pre_name = name;
    iter->Next();
  }
  delete iter;
  delete db;
  close(fd);
  fprintf(stderr, "partition %d finishes, time elapsed: %lds\n", partition_num, time(NULL)-start_s);
  return 0;
}
