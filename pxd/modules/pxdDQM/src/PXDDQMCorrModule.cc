/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "pxd/modules/pxdDQM/PXDDQMCorrModule.h"

#include <framework/core/HistoModule.h>

#include <vector>
#include <set>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMCorr)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMCorrModule::PXDDQMCorrModule() : HistoModule()
{
  //Set module properties
  setDescription("PXD DQM Correlation module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxd"));
}


PXDDQMCorrModule::~PXDDQMCorrModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMCorrModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  //----------------------------------------------------------------

  int nPixelsU1 = 1;//getInfo(0).getUCells();// we need an actual sensor ID to use that
  int nPixelsV1 = 3;//getInfo(0).getVCells();
  int nPixelsU2 = 1;//getInfo(1).getUCells();
  int nPixelsV2 = 3;//getInfo(1).getVCells();
  m_CorrelationU = new TH2F("CorrelationU", "Correlation of U;U1/cm;U2/cm", 25, -nPixelsU1, nPixelsU1, 25, nPixelsU2, nPixelsU2);
  m_CorrelationV = new TH2F("CorrelationV", "Correlation of V;V1/cm;V2/cm", 50, -nPixelsV1, nPixelsV1, 50, nPixelsV2, nPixelsV2);
  m_DeltaU = new TH1F("DeltaU", "Correlation of U2-U1;Udiff/cm", 100, -nPixelsU1, nPixelsU2);
  m_DeltaV = new TH1F("DeltaV", "Correlation of V2-V1;Vdiff/cm", 200, -nPixelsV1, nPixelsV2);

  // cd back to root directory
  oldDir->cd();
}


void PXDDQMCorrModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  m_storeClusters.isRequired(m_storeClustersName);
}

void PXDDQMCorrModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  m_CorrelationU->Reset();
  m_CorrelationV->Reset();
  m_DeltaU->Reset();
  m_DeltaV->Reset();
}


void PXDDQMCorrModule::event()
{

  // If there are no clusters, leave
  if (!m_storeClusters || !m_storeClusters.getEntries()) return;

  for (const PXDCluster& cluster1 : m_storeClusters) {
    int iPlane1 = cluster1.getSensorID().getLayerNumber();
    if ((iPlane1 < c_firstPXDPlane) || (iPlane1 > c_lastPXDPlane)) continue;
    int index1 = planeToIndex(iPlane1);
    if (index1 == 0) {
      for (const PXDCluster& cluster2 : m_storeClusters) {
        int iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        int index2 = planeToIndex(iPlane2);
        if (index2 == 1) {
          m_CorrelationU->Fill(cluster1.getU(), cluster2.getU());
          m_CorrelationV->Fill(cluster1.getV(), cluster2.getV());
          m_DeltaU->Fill(cluster2.getU() - cluster1.getU());
          m_DeltaV->Fill(cluster2.getV() - cluster1.getV());
        }
      }
    }
  }
}


void PXDDQMCorrModule::endRun()
{
}


void PXDDQMCorrModule::terminate()
{
}
