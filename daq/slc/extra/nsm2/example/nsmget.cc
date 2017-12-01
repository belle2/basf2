//
// nsmget.cc
//
// original author: Thorsten Roeder (MPI/Munich)
// modifications:   Mikihiko Nakao (KEK)
//
// 20140920 initial version named as nsmget.cpp
// 20140922 file/function/type names and style modified by MN
// 20150809 struct name taken from NSM sys memory (>= 1.9.44)
// 20150811 readmem version


#include "../nsmget/nsmget.h"

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "belle2nsm.h"

//#define MODE_MONITORING

/*
void
nsmparse_print(const NSMparse *parse)
{
  while (parse) {
    std::cout << "parse type: " << parse->type
              << ", name: " << parse->name
              << ", size: " << parse->size
              << ", offset: " << parse->offset
              << ", bytes: " << parse->bytes
              << std::endl;
    parse = parse->next;
  };
}
*/

template <typename T>
void printValue(T a)
{
  std::cout << "value: ";
  std::cout << (T)a;
  std::cout << std::endl;
}

void printValue(void* raw_memory, const nsmget_entry_t& entry)
{
  if (! entry.parse_entry) {
    std::cerr << "Internal error. Parse entry was NULL." << std::endl;
    std::cerr << "Value may not exist." << std::endl;
    return;
  }

  char* ptr = (char*)raw_memory + entry.offset;

  switch (entry.type) {
    case INT32:  printValue(*(int32_t*)ptr); break;
    case INT64:  printValue(*(int64_t*)ptr); break;
    case INT16:  printValue(*(int16_t*)ptr); break;
    case CHAR:   printValue(*(int8_t*)ptr); break;
    case UINT64: printValue(*(uint64_t*)ptr); break;
    case UINT32: printValue(*(uint32_t*)ptr); break;
    case UINT16: printValue(*(uint16_t*)ptr); break;
    case UCHAR:  printValue(*(uint8_t*)ptr); break;
    case FLOAT:  printValue(*(float*)ptr); break;
    case DOUBLE: printValue(*(double*)ptr); break;
    default:
      std::cerr << "Error: unknown NSM variable type "
                << entry.type
                << " for proper conversion."
                << std::endl;
  }
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage: "
              << argv[0]
              << " <data_name>:<variable_name>"
              << std::endl;
    std::cout << "  e.g. "
              << argv[0]
              << " \"CLIENT:nodes[3]:evt_number\""
              << std::endl;
    return EXIT_FAILURE;
  }

  std::string variable_path(argv[1]);
  std::string data_name;
  std::string variable_name;

  if (nsmget_path(variable_path, data_name, variable_name)) {
    std::cerr << "Failed to parse variable path" << std::endl;
    return EXIT_FAILURE;
  }

  if (! b2nsm_init(0)) {
    std::cerr << "Failed to initialize NSM2: "
              << b2nsm_strerror() << std::endl;
    return EXIT_FAILURE;
  }

  char fmtbuf[256];
  fmtbuf[0] = 0;
  int siz = b2nsm_statmem(data_name.c_str(), fmtbuf, sizeof(fmtbuf));
  if (siz < 0) {
    std::cerr << "nsmget: "
              << b2nsm_strerror() << std::endl;
    return EXIT_FAILURE;
  } else if (siz == 0) {
    std::cerr << "nsmget: data is not available" << std::endl;
    return EXIT_FAILURE;
  }
  std::string struct_name(fmtbuf);
  char* data_buf = new char [siz];

  int ret = b2nsm_readmem(data_buf,
                          data_name.c_str(),
                          struct_name.c_str(),
                          -1);

  if (ret < 0) {
    std::cerr << "Failed to read NSM2 data " << data_name << std::endl;
    return EXIT_FAILURE;
  }

  NSMparse* parse = nsmget_file(struct_name);
  if (! parse) {
    std::cerr << "Failed to parse file for format: "
              << struct_name
              << std::endl;
    return EXIT_FAILURE;
  }

  std::size_t offset = 0;
  nsmget_map_t mapping;
  if (nsmget_map(parse, "", offset, mapping)) {
    std::cerr << "Failed to create lookup map." << std::endl;
    return EXIT_FAILURE;
  }

#ifdef MODE_MONITORING
  while (1) {
    printValue(data_buf, mapping[variable_name]);
    sleep(1);
  }
#else
  printValue(data_buf, mapping[variable_name]);
#endif

  return EXIT_SUCCESS;
}
