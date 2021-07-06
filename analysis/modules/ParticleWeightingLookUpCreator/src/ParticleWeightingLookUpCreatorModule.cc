/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleWeightingLookUpCreator/ParticleWeightingLookUpCreatorModule.h>

// framework aux
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/database/DBImportObjPtr.h>
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
    addParam("experimentLow", m_experimentLow, "Interval of validity, ex.low");
    addParam("experimentHigh", m_experimentHigh, "Interval of validity, ex.high");
    addParam("runLow", m_runLow, "Interval of validity, run low");
    addParam("runHigh", m_runHigh, "Interval of validity, run high");
  }

  /**
   * Some massaging of python input is needed
   */
  NDBin ParticleWeightingLookUpCreatorModule::NDBinTupleToNDBin(const NDBinTuple& bin_tuple)
  {
    NDBin binning;
    for (auto bin_1d : bin_tuple) {
      std::string axis_name = bin_1d.first;
      double min_val = std::get<0>(bin_1d.second);
      double max_val = std::get<1>(bin_1d.second);
      lims = new ParticleWeightingBinLimits(min_val, max_val);
      binning.insert(std::make_pair(axis_name, lims));
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
    importer.import(Belle2::IntervalOfValidity(m_experimentLow, m_runLow, m_experimentHigh, m_runHigh));

  }

  void ParticleWeightingLookUpCreatorModule::terminate()
  {
    if (lims != nullptr) {
      delete lims;
    }
  }

} // end Belle2 namespace

