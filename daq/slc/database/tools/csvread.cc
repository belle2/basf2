#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: %s <csvfile>", argv[0]);
    return 1;
  }
  std::string csvfilename = argv[1];
  std::ifstream fin(csvfilename.c_str());
  std::string line;
  getline(fin, line);
  line = StringUtil::replace(line, " ", "");
  std::vector<std::string> name_v = StringUtil::split(line, ',');
  while (fin && getline(fin, line)) {
    line = StringUtil::replace(line, " ", "");
    line = StringUtil::replace(line, "\n", "");
    line = StringUtil::replace(line, "\t", "");
    line = StringUtil::replace(line, "\r", "");
    std::vector<std::string> value_v = StringUtil::split(line, ',');
    std::ofstream fout((value_v[0] + ".conf").c_str());
    std::string nodename = StringUtil::toupper(value_v[0]);
    fout << "#" << std::endl;
    fout << "# DB object (confname = " << nodename << "@RC:default) " << std::endl;
    fout << "#" << std::endl;
    fout << "" << std::endl;
    fout << "nodename : " << nodename << std::endl;
    fout << "config   : RC:default" << std::endl;
    fout << "" << std::endl;

    fout << "#" << std::endl;
    fout << "#" << std::endl;
    fout << "#" << std::endl;
    fout << "" << std::endl;
    fout << "# COPPER hardware/firmware configuration" << std::endl;
    bool used[4] = {false, false, false, false};
    for (size_t i = 0; i < value_v.size(); i++) {
      std::string name = name_v[i];
      std::string value = value_v[i];
      if (value.size() > 0) {
        fout << StringUtil::form("%-12s : ", name.c_str()) << value << std::endl;
        if (name == "copperid") {
          name = "hostname";
          fout << StringUtil::form("%-12s : ", name.c_str())  << value << std::endl;
        }
        if (StringUtil::find(name, "ttrx")) {
          name = StringUtil::replace(name, ".id", ".firm");
          fout << StringUtil::form("%-12s : ", name.c_str()) << "/home/usr/b2daq/run/ttrx/tt5r022.bit" << std::endl;
        }
        if (StringUtil::find(name, "hslb")) {
          name = StringUtil::replace(name, ".id", ".firm");
          fout << StringUtil::form("%-12s : ", name.c_str()) << "/home/usr/b2daq/run/hslb/hslb035_rck.bit" << std::endl;
          name = StringUtil::replace(name, "hslb[", "");
          int id = atoi(name.c_str());
          used[id] = true;
        }
      }
    }
    fout << "" << std::endl;
    fout << "# Active HSLB-FINNESS slots" << std::endl;
    fout << "hslb[0].used : " << (used[0] ? "true" : "false") << std::endl;
    fout << "hslb[1].used : " << (used[1] ? "true" : "false") << std::endl;
    fout << "hslb[2].used : " << (used[2] ? "true" : "false") << std::endl;
    fout << "hslb[3].used : " << (used[3] ? "true" : "false") << std::endl;
    fout << "" << std::endl;
    fout << "# Readout software" << std::endl;
    fout << "basf2.script : daq/rawdata/examples/RecvSendCOPPER.py " << std::endl;
    fout << "" << std::endl;
    fout << "# FEE configuration (firmware and parameters etc.)" << std::endl;
    if (used[0]) fout << "fee[0]       : object()" << std::endl;
    if (used[1]) fout << "fee[1]       : object()" << std::endl;
    if (used[2]) fout << "fee[2]       : object()" << std::endl;
    if (used[3]) fout << "fee[3]       : object()" << std::endl;
    fout << "" << std::endl;
    fout << "#" << std::endl;
    fout << "#" << std::endl;
    fout << "#" << std::endl;
  }
  return 0;
}
