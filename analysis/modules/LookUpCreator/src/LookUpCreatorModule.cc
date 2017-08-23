/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/LookUpCreator/LookUpCreatorModule.h>
#include <framework/core/ModuleParam.templateDetails.h>

// framework aux
#include <framework/logging/Logger.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(LookUpCreator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  LookUpCreatorModule::LookUpCreatorModule() : Module()

  {
    setDescription("Creates test lookup table");
    addParam("tableIDNotSpec", m_tableIDNotSpec, "Bin:weight info map without specific bin-numbering scheme");
    addParam("tableIDSpec", m_tableIDSpec, "Bin:weight info map with specific bin-numbering scheme");
    addParam("outOfRangeWeight", m_outOfRangeWeight, "Weight info for out-of-range partiles");
  }

  void LookUpCreatorModule::initialize()
  {
    LookupTable table = LookupTable();
    if (!m_tableIDNotSpec.empty() and m_tableIDSpec.empty()) {
      for (auto entry : m_tableIDNotSpec) {
        table.addEntry(entry.first, entry.second);
      }
    } else if (!m_tableIDSpec.empty() and m_tableIDNotSpec.empty()) {
      for (auto entry : m_tableIDSpec) {
        table.addEntry(entry.first.first, entry.first.second, entry.second);
      }
    } else {
      B2ERROR("Please define one table: with OR without specific bin IDs");
    }

    if (!m_outOfRangeWeight.empty()) {
      table.defineOutOfRangeWeight(m_outOfRangeWeight);
    }



    //std::string x_name = "x name";
    //std::string y_name = "y name";
    //BinLimits x_lim1 = std::pair<double, double>(0, 1);
    //BinLimits y_lim1 = std::pair<double, double>(0, 10);
    //BinLimits x_lim2 = std::pair<double, double>(1, 2);
    //BinLimits y_lim2 = std::pair<double, double>(2, 20);
    //NDBin bin1;
    //bin1.insert(std::pair<std::string, BinLimits>(x_name, x_lim1));
    //bin1.insert(std::pair<std::string, BinLimits>(y_name, y_lim1));
    //NDBin bin2;
    //bin2.insert(std::pair<std::string, BinLimits>(x_name, x_lim2));
    //bin2.insert(std::pair<std::string, BinLimits>(y_name, y_lim1));
    //NDBin bin3;
    //bin3.insert(std::pair<std::string, BinLimits>(x_name, x_lim2));
    //bin3.insert(std::pair<std::string, BinLimits>(y_name, y_lim2));
    //NDBin bin4;
    //bin4.insert(std::pair<std::string, BinLimits>(x_name, x_lim1));
    //bin4.insert(std::pair<std::string, BinLimits>(y_name, y_lim2));
    //WeightInfo info1;
    //info1.insert(std::pair<std::string, double>("bin_id", 1));
    //WeightInfo info2;
    //info2.insert(std::pair<std::string, double>("bin_id", 2));
    //WeightInfo info3;
    //info3.insert(std::pair<std::string, double>("bin_id", 3));
    //WeightInfo info4;
    //info4.insert(std::pair<std::string, double>("bin_id", 4));
    //table.addEntry(info1, bin1);
    //table.addEntry(info2, bin2);
    //table.addEntry(info3, bin3);
    //table.addEntry(info4, bin4);
    B2INFO("Printing Lookup table");
    table.printLookupTable();
  }

} // end Belle2 namespace

