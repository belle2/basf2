/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleWeightingLookUpCreator/ParticleWeightingLookUpCreatorModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/database/DBImportObjPtr.h>


// framework aux
#include <framework/logging/Logger.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleWeightingLookUpCreator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleWeightingLookUpCreatorModule::ParticleWeightingLookUpCreatorModule() : Module()

  {
    setDescription("Creates test LookUp table");
    listOfNoIdEntries empty_noid_list;
    addParam("tableIDNotSpec", m_tableIDNotSpec, "Bin:weight info map without specific bin-numbering scheme", empty_noid_list);
    listOfSpecificIDEntries empty_specificid_list;
    addParam("tableIDSpec", m_tableIDSpec, "Bin:weight info map with specific bin-numbering scheme", empty_specificid_list);
    addParam("tableName", m_tableName, "Name of the LookUp table");
    addParam("outOfRangeWeight", m_outOfRangeWeight, "Weight info for out-of-range partiles");
  }

  // Sorry
  NDBin ParticleWeightingLookUpCreatorModule::NDBinTupleToNDBin(NDBinTuple bin_tuple)
  {
    NDBin binning;
    for (auto bin_1d : bin_tuple) {
      std::string axis_name = bin_1d.first;
      double min_val = std::get<0>(bin_1d.second);
      double max_val = std::get<1>(bin_1d.second);
      BinLimits lims = std::pair<double, double>(min_val, max_val);
      binning.insert(std::pair<std::string, BinLimits>(axis_name, lims));
    }
    return binning;
  }

  void ParticleWeightingLookUpCreatorModule::initialize()
  {
    ParticleWeightingLookUpTable table = ParticleWeightingLookUpTable();

    if (!m_outOfRangeWeight.empty()) {
      table.defineOutOfRangeWeight(m_outOfRangeWeight);
    }

    if (!m_tableIDNotSpec.empty() and m_tableIDSpec.empty()) {
      for (auto entry : m_tableIDNotSpec) {
        WeightInfo info = std::get<0>(entry);
        NDBin bin = NDBinTupleToNDBin(std::get<1>(entry));
        table.addEntry(info, bin);
      }
    } else if (!m_tableIDSpec.empty() and m_tableIDNotSpec.empty()) {
      for (auto entry : m_tableIDSpec) {
        noIdEntry noid = std::get<0>(entry);
        double bin_id = std::get<1>(entry);
        WeightInfo info = std::get<0>(noid);
        NDBin bin = NDBinTupleToNDBin(std::get<1>(noid));
        table.addEntry(info, bin, bin_id);
      }
    } else {
      B2ERROR("Please define one table: with OR without specific bin IDs");
    }

    if (!m_outOfRangeWeight.empty()) {
      table.defineOutOfRangeWeight(m_outOfRangeWeight);
    }
    B2INFO("Printing LookUp table");
    table.printParticleWeightingLookUpTable();

    Belle2::DBImportObjPtr<Belle2::ParticleWeightingLookUpTable> importer{m_tableName};
    importer.construct(table);
    importer.import(Belle2::IntervalOfValidity(0, 0, 3, 14));

  }

} // end Belle2 namespace

