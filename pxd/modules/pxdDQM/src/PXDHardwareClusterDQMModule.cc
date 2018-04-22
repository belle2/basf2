/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDHardwareClusterDQMModule.h>

#include "TDirectory.h"
#include "TObject.h"
#include "TNtuple.h"
#include "TFile.h"
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDHardwareClusterDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDHardwareClusterDQMModule::PXDHardwareClusterDQMModule() : HistoModule() , m_storeHardClusterArray() , m_storeRawCluster()
{
  //Set module properties
  setDescription("Monitor PXD hardware cluster");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdhardcluster"));
}

void PXDHardwareClusterDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  hHardClusterPerHalfLadder = new TH1F("hHardClusterPerHalfLadder", "Pxd Hardware Clusters per half ladder;Number of Clusters;", 200,
                                       0, 200);
  hHardClusterPacketSize = new TH1F("hHardClusterPacketSize", "Pxd Hardware Cluster Packetsize;short words per packet", 100, 0, 100);
  hHardClusterHitsCount = new TH1F("hHardClusterHitsCount", "Pxd Hardware Cluster Pixels count;Pixels per cluster", 100, 0, 100);
  hHardClusterHitMapAll  = new TH2F("hHardClusterHitMapAll",
                                    "Pxd Hardware Cluster Hit Map Overview;row;column", 768, 0, 768, 255, 0, 255);
  hHardClusterPixelAdc  = new TH1F("hHardClusterPixelAdc",
                                   "Pxd Hardware Pixel adc Overview;adc;nr pixels", 255, 0, 255);
  hHardClusterAdc  = new TH1F("hHardClusterAdc",
                              "Pxd Hardware Cluster adc Overview;adc;nr clusters", 5000, 0, 5000);
  hHardClusterSeedAdc  = new TH1F("hHardClusterSeedAdc",
                                  "Pxd Hardware seed adc Overview;adc;nr seed pixels", 255, 0, 255);

  oldDir->cd();
}

void PXDHardwareClusterDQMModule::initialize()
{
  REG_HISTOGRAM
  m_storeHardClusterArray.isRequired();
  m_storeRawCluster.isRequired();

  StoreArray<PXDHardwareCluster> storeHardwareClusters(m_storeHardClusterArray);
  StoreArray<PXDRawCluster> storeRawClusters(m_storeRawCluster);
}

void PXDHardwareClusterDQMModule::beginRun()
{
// Just to make sure, reset all the histograms.
  hHardClusterPerHalfLadder->Reset();
  hHardClusterPacketSize->Reset();
  hHardClusterHitsCount->Reset();
  hHardClusterHitMapAll->Reset();
  hHardClusterPixelAdc->Reset();
  hHardClusterAdc->Reset();
  hHardClusterSeedAdc->Reset();
}

void PXDHardwareClusterDQMModule::event()
{
  int cluster_packets = 0;
  for (auto& it : m_storeRawCluster) {
    cluster_packets++;

    hHardClusterPacketSize->Fill(it.getLength());
  }
  hHardClusterPerHalfLadder->Fill(cluster_packets);
  for (auto& it : m_storeHardClusterArray) {
    hHardClusterHitsCount->Fill(it.getNrPixels());

    for (unsigned int i = 0 ; i < it.getNrPixels() ; i++) {
      hHardClusterHitMapAll->Fill(it.getPixelU(i), it.getPixelV(i), it.getPixelQ(i));
      hHardClusterHitMapAll->SetOption("colz");
      hHardClusterPixelAdc->Fill(it.getPixelQ(i));
    }

    hHardClusterAdc->Fill(it.getClusterCharge());
    hHardClusterSeedAdc->Fill(it.getSeedCharge());

  }
}

