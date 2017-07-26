//
// libnsmget.cc
//
// original author: Thorsten Roeder (MPI/Munich)
// modifications:   Mikihiko Nakao (KEK)
//
// 20140920 ---- initial version named as b2nsmaccesslib.cpp
// 20140922 1941 file/function/type names and style modified by MN
//

static const char* nsmget_version   = "nsmget 1.9.41";

#include "nsmget.h"

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>

#include <pwd.h>
#include <sys/types.h>

NSMparse*
nsmget_file(const std::string& struct_name)
{
  int new_revision = -1;
  char fmt[256] = {0};
  const std::string include_file = struct_name + ".h";
  return nsmlib_parsefile(struct_name.c_str(),
                          -1,
                          include_file.c_str(),
                          fmt,
                          &new_revision);
}

int
nsmget_path(const std::string& variable_path,
            std::string& data_name,
            std::string& variable_name)
{
  variable_name = variable_path;

  std::size_t found1st = variable_name.find(":");
  if (found1st == std::string::npos) {
    std::cerr << "Error: malformed variable name" << std::endl;
    return 1;
  }

  data_name = variable_name.substr(0, found1st);

  variable_name.erase(0, found1st + 1);

  return 0;
}

int
nsmget_path(const std::string& variable_path,
            std::string& data_name,
            std::string& struct_name,
            std::string& variable_name)
{
  variable_name = variable_path;

  std::size_t found1st = variable_name.find(":");
  if (found1st == std::string::npos) {
    std::cerr << "Error: malformed variable name" << std::endl;
    return 1;
  }

  std::size_t found2nd = variable_name.find(":", found1st + 1);
  if (found2nd == std::string::npos) {
    std::cerr << "Error: malformed variable name" << std::endl;
    return 2;
  }

  data_name = variable_name.substr(0, found1st);
  struct_name = variable_name.substr(found1st + 1, found2nd - found1st - 1);

  variable_name.erase(0, found2nd + 1);

  return 0;
}

nsmget_native_t
nsmget_type(const char c)
{
  switch (c) {
    case 'l': return INT64;
    case 'i': return INT32;
    case 's': return INT16;
    case 'c': return CHAR;
    case 'L': return UINT64;
    case 'I': return UINT32;
    case 'S': return UINT16;
    case 'C': return UCHAR;
    case 'd': return DOUBLE;
    case 'f': return FLOAT;
    default:
      std::cerr << "Error: no known conversion of NSM type: " << c << std::endl;
      // fatal error
      break;
  }
  return CHAR;
}

const NSMparse*
nsmget_map(const NSMparse* start,
           const std::string prefix,
           std::size_t& current_offset,
           nsmget_map_t& mapping)
{
  while (start) {
    if (start->type == ')') {
      return start;

    } else if (start->type == '(') {
      // start of a nested struct
      nsmget_map_t sub_mapping;
      std::size_t sub_offset = 0;
      const NSMparse* sub_end = nsmget_map(start->next,
                                           std::string(),
                                           sub_offset,
                                           sub_mapping);

      if (sub_end->size != -1) {
        for (int i = 0; i < sub_end->size; i++) {
          nsmget_map_t::iterator it = sub_mapping.begin();
          while (it != sub_mapping.end()) {
            nsmget_entry_t entry;

            entry.offset = current_offset + it->second.offset;
            entry.parse_entry = it->second.parse_entry;
            entry.type = nsmget_type(it->second.parse_entry->type);

            std::stringstream ss;
            if (!prefix.empty()) {
              ss << prefix;
              ss << ":";
            }
            ss << sub_end->name;
            ss << "[";
            ss << i;
            ss << "]";
            ss << ":";
            ss << it->first;
            mapping[ss.str()] = entry;

            ++it;
          }

          current_offset += sub_offset;
        }
      } else {
        nsmget_map_t::iterator it = sub_mapping.begin();

        while (it != sub_mapping.end()) {
          nsmget_entry_t entry;

          entry.offset = it->second.offset;
          entry.parse_entry = it->second.parse_entry;
          entry.type = nsmget_type(it->second.parse_entry->type);

          std::stringstream ss;
          if (!prefix.empty()) {
            ss << prefix;
            ss << ":";
          }
          ss << it->first;
          mapping[ss.str()] = entry;

          ++it;
        }

        current_offset += sub_offset;
      }

      start = sub_end;
    } else {
      if (start->size > 0) {
        // array of scalars
        for (int i = 0; i < start->size; i++) {
          nsmget_entry_t entry;

          entry.offset = current_offset;
          entry.parse_entry = start;
          entry.type = nsmget_type(start->type);

          current_offset += start->bytes;

          std::stringstream ss;
          if (!prefix.empty()) {
            ss << prefix;
            ss << ":";
          }
          ss << start->name;
          ss << "[";
          ss << i;
          ss << "]";
          mapping[ss.str()] = entry;
        }
      } else {
        nsmget_entry_t entry;

        entry.offset = current_offset;
        entry.parse_entry = start;
        entry.type = nsmget_type(start->type);

        current_offset += start->bytes;

        std::stringstream ss;
        if (!prefix.empty()) {
          ss << prefix;
          ss << ":";
        }
        ss << start->name;
        mapping[ss.str()] = entry;
      }
    }

    start = start->next;
  }

  return NULL;
}
