/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**************************************************************************
 * Automated CDCTRG dead-channel payload writer (reads JSON config).
 * Usage (inside basf2 build, after scons):
 *   trg-cdc-deadch-writedb cdcdead_config.json
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/LogSystem.h>
#include <trg/cdc/dbobjects/CDCTriggerDeadch.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace Belle2;

#define ONLINE 0
//#define ONLINE 1

struct DeadchConfig {
  int run[4];
  std::vector<int> mgr;
};

bool get_wire_mgr(int mask_sl[], int mask_layer[], int mask_ch[], int mgr)
{
  int mgr_sl = mgr / 1000;
  int mgr_i  = (mgr - mgr_sl * 1000) / 10;
  int mgr_u  = mgr % 10;
  //MGR0 0-4 :L3-7   lch0-159
  //MGR1 0-4 :L8-12  lch0-159
  //MGR2 0-5 :L14-18 lch0-191
  //MGR3 0-6 :L20-24 lch0-223
  //MGR4 0-7 :L26-30 lch0-255
  //MGR5 0-8 :L32-36 lch0-287
  //MGR6 0-9 :L38-42 lch0-319
  //MGR7 0-10:L44-48 lch0-351
  //MGR8 0-11:L50-54 lch0-383
  //160*8 + 160*6 + 192*6 + 224*6 + 256*6 + 288*6 + 320*6 + 352*6 + 384*6 = 14336 = nSenseWires

  for (int i = 0; i < 32; i++) {
    mask_sl[i] = mgr_sl;
    mask_ch[i] = mgr_i * 32 + i;
    mask_layer[i] = 0;
    if (mgr_u == 1) mask_layer[i] += 3;
    if (mgr_sl == 0) mask_layer[i] += 2;
  }
  for (int i = 0; i < 32; i++) {
    mask_sl[i + 32] = mgr_sl;
    mask_ch[i + 32] = mgr_i * 32 + i;
    mask_layer[i + 32] = 1;
    if (mgr_u == 1) mask_layer[i + 32] += 3;
    if (mgr_sl == 0) mask_layer[i + 32] += 2;
  }
  for (int i = 0; i < 32; i++) {
    mask_sl[i + 64] = mgr_sl;
    mask_ch[i + 64] = mgr_i * 32 + i;
    mask_layer[i + 64] = 2;
    if (mgr_u == 1) mask_layer[i + 64] += 3;
    if (mgr_sl == 0) mask_layer[i + 64] += 2;
  }
  return true;
}

static std::string trim(const std::string& s)
{
  size_t b = 0;
  while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) b++;
  size_t e = s.size();
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) e--;
  return s.substr(b, e - b);
}

static std::vector<int> parseIntList(const std::string& line)
{
  std::vector<int> out;
  size_t lb = line.find('[');
  size_t rb = line.find(']');
  if (lb == std::string::npos || rb == std::string::npos || rb <= lb) return out;
  std::string inner = line.substr(lb + 1, rb - lb - 1);
  std::stringstream ss(inner);
  std::string tok;
  while (std::getline(ss, tok, ',')) {
    tok = trim(tok);
    if (!tok.empty()) out.push_back(std::stoi(tok));
  }
  return out;
}

static bool loadConfigs(const std::string& path, std::vector<DeadchConfig>& configs)
{
  std::ifstream ifs(path);
  if (!ifs) {
    std::cerr << "Cannot open config file: " << path << std::endl;
    return false;
  }

  std::string line;
  bool inConfigs = false;
  DeadchConfig current{};
  bool haveCurrent = false;

  while (std::getline(ifs, line)) {
    line = trim(line);
    if (line.find("\"configs\"") != std::string::npos) {
      inConfigs = true;
      continue;
    }
    if (!inConfigs) continue;

    if (line.find('}') != std::string::npos && haveCurrent) {
      configs.push_back(current);
      current = DeadchConfig{};
      haveCurrent = false;
    }
    if (line.find("\"run\"") != std::string::npos) {
      std::vector<int> run = parseIntList(line);
      if (run.size() != 4) {
        std::cerr << "Bad run array (need 4 values): " << line << std::endl;
        return false;
      }
      for (int i = 0; i < 4; i++) current.run[i] = run[i];
      haveCurrent = true;
    }
    if (line.find("\"mgr\"") != std::string::npos) {
      current.mgr = parseIntList(line);
    }
  }

  if (haveCurrent) configs.push_back(current);

  if (configs.empty()) {
    std::cerr << "No configs found in " << path << std::endl;
    return false;
  }
  return true;
}

void setdeadchFromConfigs(const std::vector<DeadchConfig>& configs)
{
  const static int MAX_N_LAYERS = 8;
  const int N_config = configs.size();

  auto badch_map = new bool[N_config][9][8][384]; //sl layer ch
  for (int i = 0; i < N_config; i++) {
    for (unsigned int j = 0; j < c_nSuperLayers; j++) {
      for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
        for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
          badch_map[i][j][k][l] = true;
        }
      }
    }
  }

  // mask L54 for all runs
  for (int i = 0; i < N_config; i++) {
    for (unsigned int j = 8; j < 9; j++) {
      for (unsigned int k = 4; k < 5; k++) {
        for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
          badch_map[i][j][k][l] = false;
        }
      }
    }
  }

  // mask merger (ONLINE==0 only; ONLINE==1 uses config 0 with L54 mask only, no merger)
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      for (unsigned int j = 0; j < configs[i].mgr.size(); j++) {
        int mgr_sl[96];
        int mgr_layer[96];
        int mgr_ch[96];
        get_wire_mgr(mgr_sl, mgr_layer, mgr_ch, configs[i].mgr[j]);
        for (int k = 0; k < 96; k++) {
          badch_map[i][mgr_sl[k]][mgr_layer[k]][mgr_ch[k]] = false;
        }
      }
    }
  }

  DBImportObjPtr<CDCTriggerDeadch> db_dead;
  db_dead.construct();

  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      const int* r = configs[i].run;
      std::cout << "import config " << i << " iov=(" << r[0] << "," << r[1]
                << "," << r[2] << "," << r[3] << ") n_mgr=" << configs[i].mgr.size() << std::endl;
      IntervalOfValidity iov(r[0], r[1], r[2], r[3]);
      for (unsigned int j = 0; j < c_nSuperLayers; j++) {
        for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
          for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
            db_dead->setdeadch(j, k, l, badch_map[i][j][k][l]);
          }
        }
      }
      db_dead.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = 0; i < 1; i++) { //no merger dead channel for run-independent MC. L54 is masked.
      IntervalOfValidity iov(0, 0, -1, -1);
      for (unsigned int j = 0; j < c_nSuperLayers; j++) {
        for (unsigned int k = 0; k < MAX_N_LAYERS; k++) {
          for (unsigned int l = 0; l < c_maxNDriftCells; l++) {
            db_dead->setdeadch(j, k, l, badch_map[i][j][k][l]);
          }
        }
      }
      db_dead.import(iov);
    }
  }

  delete[] badch_map;
}

int main(int argc, char* argv[])
{
  std::string configPath = "cdcdead_config.json";
  if (argc >= 2) configPath = argv[1];

  std::vector<DeadchConfig> configs;
  if (!loadConfigs(configPath, configs)) return 1;

  std::cout << "Loaded " << configs.size() << " config(s) from " << configPath << std::endl;
  setdeadchFromConfigs(configs);
  std::cout << "Done. localdb written in current working directory." << std::endl;
  return 0;
}
