/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclDQMClusters/eclDQMClusters.h>

/* Basf2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>

/* ROOT headers. */
#include <TDirectory.h>

/* C++ headers. */
#include <stdexcept>

//NAMESPACE(S)
using namespace Belle2;

REG_MODULE(ECLDQMClusters);

ECLDQMClustersModule::ECLDQMClustersModule()
  : HistoModule()
{
  //Set module properties.
  setDescription("ECL Data Quality Monitor to monitor ECL clusters");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify parallel processing.

  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", std::string("ECL"));

  addParam("badCrystalsThreshold", m_BadCrystalsThreshold,
           "Crystals number threshold above this number assume that cluster is bad", 400);
}

ECLDQMClustersModule::~ECLDQMClustersModule()
{
}


void ECLDQMClustersModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.
  TDirectory* dirDAQ = dynamic_cast<TDirectory*>(oldDir->Get(m_histogramDirectoryName.c_str()));
  if (!dirDAQ) dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  m_CrystalsInClustersHistogram = new TProfile("avg_crystals_in_cluster", "Average crystals number in ECLCluster", 1, 0, 1);
  m_BadClustersHistogram = new TProfile("bad_clusters", "Number of bad ECLClusters", 1, 0, 1);
  m_ClustersCrystalsNumHistogram = new TH1F("Crystals_distribution_in_clusters", "Crystals distribution in ECLClusters", 1000, 0,
                                            2000);

  //cd into parent directory.
  oldDir->cd();
}

void ECLDQMClustersModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager.
}

void ECLDQMClustersModule::beginRun()
{
  m_CrystalsInClustersHistogram->Reset();
  m_BadClustersHistogram->Reset();
  m_ClustersCrystalsNumHistogram->Reset();
}

void ECLDQMClustersModule::event()
{
  StoreArray<ECLCluster> ecl_clusters;
  for (int i = 0; i < ecl_clusters.getEntries(); i++) {
    auto crystalsNum = ecl_clusters[i]->getNumberOfCrystals();
    m_CrystalsInClustersHistogram->Fill(0., crystalsNum);
    m_BadClustersHistogram->Fill(0., (Double_t)(crystalsNum > m_BadCrystalsThreshold));
    m_ClustersCrystalsNumHistogram->Fill(crystalsNum);
  }
}

void ECLDQMClustersModule::endRun()
{
}


void ECLDQMClustersModule::terminate()
{
}
