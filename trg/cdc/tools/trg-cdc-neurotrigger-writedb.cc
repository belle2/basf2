/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
#include <iostream>
#include <fstream>

class InputParser {
public:
  InputParser(const int& argc, char** argv)
  {
    for (int i = 1; i < argc; ++i)
      this->tokens.push_back(std::string(argv[i]));
  }
  const std::string& getCmdOption(const std::string& option) const
  {
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
      return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
  }
  bool cmdOptionExists(const std::string& option) const
  {
    return std::find(this->tokens.begin(), this->tokens.end(), option)
           != this->tokens.end();
  }
private:
  std::vector <std::string> tokens;
};

using namespace Belle2;

int main(int argc, char** argv)
{
  int iovc = 0;
  int nniov_exp_start;
  int nniov_exp_end;
  int nniov_run_start;
  int nniov_run_end;
  std::string configfilename = "";
  std::string nnname;
  std::string nnpath;
  std::string nnnote;
  std::string fwname;
  std::string fwnote;
  bool ppbool;
  std::string ppnote;


  InputParser input(argc, argv);
  if (input.cmdOptionExists("-h")) {
    std::cout << "A small tool to create ConDB payloads for the Neurotrigger." << std::endl;
    std::cout << "Usage: \% trg-cdc-neurotrigger-writedb -f example.conf" << std::endl;
  }
  const std::string& filename = input.getCmdOption("-f");
  if (!filename.empty()) {
    configfilename = filename;
  }

  // Creating payload object:
  DBImportObjPtr<CDCTriggerNeuroConfig> nc;
  nc.construct();

  std::ifstream confile;
  try {
    confile.open(configfilename, std::ifstream::in);
  } catch (int e) {
    std::cout << "ERROR! While opening file: " << configfilename << "    Error code: " << e << std::endl;
    exit(EXIT_FAILURE);
  }
  std::string line_all;
  if (!confile.is_open()) {
    std::cout << "ERROR! While opening file: " << configfilename << std::endl;
    exit(EXIT_FAILURE);
  }
  while (std::getline(confile, line_all)) {  // remove comments
    std::size_t hashtag = line_all.find('#');
    std::string line = line_all.substr(0, hashtag);
    std::string par;
    std::string key;
    if (line.length() < 3) {
      continue;
      // check, if line wasnt a pure comment line
    }
    if (line.find('=') == std::string::npos) {
      continue;
    }
    par = line.substr(0, line.find('='));
    par.erase(std::remove(par.begin(), par.end(), ' '), par.end()); // remove whitespaces in whole string

    if (par == "nniov_run_start") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nniov_run_start = std::stoi(key);
      iovc++;
    }
    if (par == "nniov_run_end") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nniov_run_end = std::stoi(key);
      iovc++;
    }
    if (par == "nniov_exp_start") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nniov_exp_start = std::stoi(key);
      iovc++;
    }
    if (par == "nniov_exp_end") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nniov_exp_end = std::stoi(key);
      iovc++;
    }
    if (par == "nnname") {
      // may look confusing at a first glance, but cuts exactly the content between the first two occurrences of doublequotes
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nnname = key;
      //set neural network filename:
      nc->setNNName(nnname);
    }
    if (par == "nnnote") {
      // may look confusing at a first glance, but cuts exactly the content between the first two occurrences of doublequotes
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nnnote = key;
      //add notes for expert networks
      nc->setNNNotes(nnnote);
    }
    if (par == "nnpath") {
      // may look confusing at a first glance, but cuts exactly the content between the first two occurrences of doublequotes
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nnpath = key;
      // loading MLPs:
      nc->loadMLPs(nnpath, "MLPs");
    }
    if (par == "fwname") {
      // may look confusing at a first glance, but cuts exactly the content between the first two occurrences of doublequotes
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      fwname = key;
      //set firmware version id:
      nc->setNNTFirmwareVersionID(fwname);
    }
    if (par == "fwnote") {
      // may look confusing at a first glance, but cuts exactly the content between the first two occurrences of doublequotes
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      fwnote = key;
      // add comment about firmware:
      nc->setNNTFirmwareComment(fwnote);

    }
    if (par == "ppnote") {
      // may look confusing at a first glance, but cuts exactly the content between the first two occurrences of doublequotes
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      ppnote = key;
      //add preprocessing notes:
      nc->setPPNotes(ppnote);
    }
    if (par == "ppbool") {
      if (line.find("alse") != std::string::npos) { //dirty case insensitive
        ppbool = false;
      } else if (line.find("rue") != std::string::npos) {
        ppbool = true;
      } else {
        std::cout << "ERROR!: Wrong key argument for parameter ppbool:" << line << std::endl;
      }
      //define to use the ETF:
      nc->setUseETF(ppbool);
    }
    if (par == "addb2formatline") {
      // split key in data fields:
      std::stringstream ss;
      ss << line.substr((line.find('(') + 1), (line.find(')') - 1 - line.find('(')));
      std::string uid;
      std::string startstr;
      std::string endstr;
      std::string offsetstr;
      std::string description;
      std::getline(ss, uid, ',');
      std::getline(ss, startstr, ',');
      std::getline(ss, endstr, ',');
      std::getline(ss, offsetstr, ',');
      std::getline(ss, description, '\n');
      nc->addB2FormatLine(std::stoi(startstr), std::stoi(endstr), std::stoi(offsetstr), uid.substr((uid.find('"') + 1), (uid.find('"',
                          uid.find('"') + 1) - 1 - uid.find('"'))), description.substr((description.find('"') + 1), (description.find('"',
                              description.find('"') + 1) - 1 - description.find('"'))));

    }
  }
  if (iovc > 3) { // >3 means all 4 iov numbers are there
    // set interval of validity:
    IntervalOfValidity iov(nniov_exp_start, nniov_run_start, nniov_exp_end, nniov_run_end);
    nc.import(iov);
  }
  return 0;
}
