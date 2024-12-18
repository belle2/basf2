/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclDQMConnectedRegions/eclDQMConnectedRegions.h>

/* Basf2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLCalDigit.h>

/* ROOT headers. */
#include <TDirectory.h>

/* C++ headers. */
#include <string>

//NAMESPACE(S)
using namespace Belle2;

REG_MODULE(ECLDQMConnectedRegions);

ECLDQMConnectedRegionsModule::ECLDQMConnectedRegionsModule()
  : HistoModule()
{
  //Set module properties.
  setDescription("ECL Data Quality Monitor to monitor ECL Connected Regions");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify parallel processing.

  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", std::string("ECL"));
}

void ECLDQMConnectedRegionsModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.
  TDirectory* dirDAQ = dynamic_cast<TDirectory*>(oldDir->Get(m_histogramDirectoryName.c_str()));
  if (!dirDAQ) dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  m_largestCRCrystalsNum = new TH1F("Crystals_distribution_in_largest_cr", "Crystals distribution in largest ECLConnectedRegions",
                                    1000, 0,
                                    2000);
  m_largestCRLocalMaxNum = new TH1F("LocalMaximum_distribution_in_largest_cr", "LocalMaximum distribution in ECLConnectedRegion",
                                    1000, 0,
                                    2000);

  //cd into parent directory.
  oldDir->cd();
}

void ECLDQMConnectedRegionsModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager.
}

void ECLDQMConnectedRegionsModule::beginRun()
{
  m_largestCRLocalMaxNum->Reset();
  m_largestCRCrystalsNum->Reset();
}

void ECLDQMConnectedRegionsModule::event()
{
  StoreArray<ECLConnectedRegion> ecl_connected_regions;
  size_t max_crystals = 0;
  size_t localmax_in_largest_cr = 0;
  for (auto& region : ecl_connected_regions) {
    auto cr_crystals_num = region.getRelationsWith<ECLCalDigit>().size();
    auto cr_localmax_num = region.getRelationsWith<ECLLocalMaximum>().size();
    if (cr_crystals_num > max_crystals) {
      max_crystals = cr_crystals_num;
      localmax_in_largest_cr = cr_localmax_num;
    }
  }
  m_largestCRCrystalsNum->Fill(max_crystals);
  m_largestCRLocalMaxNum->Fill(localmax_in_largest_cr);
}
