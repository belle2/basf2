/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include "trg/klm/modules/klmtrigger/ntuples_full.h"


template <int N = 0>
auto & get_IO_csv_handle()
{
  struct data {
    int event_nr = 0;
    std::string dump_path;
    std::map<std::string, std::optional<std::ofstream>> files;
    bool do_dump = false;
  };
  static data m_data{};
  return m_data;

}



template <typename T>
void save(std::string filename, T&& content)
{
  if (content.empty()) {
    return;
  }
  auto& o_file = get_IO_csv_handle().files[filename];

  if (!o_file) {
    std::ofstream ofs;
    // Open the file
    ofs.open(filename);

    // Check if the file is successfully opened
    if (!ofs.is_open()) {
      throw std::runtime_error("unable to open file: " + filename);
    }
    o_file = std::move(ofs);

    *o_file << "gevent_number";
    const auto& row = content[0];

    nt::ntuple_for_each(row,
    [&](auto & ele) {
      *o_file   << "," << ele.get_name();
    }
                       );

    *o_file << '\n';

  }



  for (const auto& row : content) {
    *o_file <<  get_IO_csv_handle().event_nr;
    nt::ntuple_for_each(row,
    [&](auto & ele) {
      *o_file << ',' << ele.v;
    }
                       );

    *o_file << '\n';
  }

}


template <typename T>
void save_internal(std::string source_file, int lineno,  std::string variable_name, T&& content)
{
  if (!get_IO_csv_handle().do_dump) {
    return;
  }
  auto itterA = source_file.find_last_of("\\");
  auto itterB = source_file.find_last_of("/");
  itterB = itterB < source_file.size() ? itterB : 0;
  itterA = itterA < source_file.size() ? itterA : 0;
  auto itter = std::max(itterA, itterB);

  auto file = source_file.substr(itter + 1);

  file = get_IO_csv_handle().dump_path  + file + "_" + std::to_string(lineno) + "_" + variable_name + ".csv";
  //std::cout << source_file << "  " << variable_name << " " << file << "\n";
  save(file, content);
}

#ifdef __DO_CSV_PRINTOUT__
#define __CSV__WRITE_W_lINE__(x)  save_internal(__FILE__, __LINE__, #x, x)
#define __CSV__WRITE__(x) save_internal(__FILE__, 0, #x, x)
#else

// do nothing
#define __CSV__WRITE_W_lINE__(x)  do {} while (false)
#define __CSV__WRITE__(x) do {} while (false)

#endif
