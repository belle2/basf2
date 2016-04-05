#include "pxd/modules/pxdDQM/PXDDQMCorrModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <pxd/dataobjects/PXDCluster.h>

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
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();
  //----------------------------------------------------------------

  int nPixelsU1 = 1;//getInfo(0).getUCells();
  int nPixelsV1 = 3;//getInfo(0).getVCells();
  int nPixelsU2 = 1;//getInfo(1).getUCells();
  int nPixelsV2 = 3;//getInfo(1).getVCells();
  m_CorrelationU = new TH2F("CorrelationU", "Correlation of U;U1/pitch unit;U2/pitch unit", 50, 0, nPixelsU1, 50, 0, nPixelsU2);
  m_CorrelationV = new TH2F("CorrelationV", "Correlation of V;V1/pitch unit;V2/pitch unit", 50, 0, nPixelsV1, 50, 0, nPixelsV2);

  // cd back to root directory
  oldDir->cd();
}


void PXDDQMCorrModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
}

void PXDDQMCorrModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  m_CorrelationU->Reset();
  m_CorrelationV->Reset();
}


void PXDDQMCorrModule::event()
{
  const StoreArray<PXDCluster> storeClusters(m_storeClustersName);

  // If there are no clusters, leave
  if (!storeClusters || !storeClusters.getEntries()) return;

  for (const PXDCluster& cluster1 : storeClusters) {
    int iPlane1 = cluster1.getSensorID().getLayerNumber();
    if ((iPlane1 < c_firstPXDPlane) || (iPlane1 > c_lastPXDPlane)) continue;
    int index1 = planeToIndex(iPlane1);
    if (index1 == 0) {
      for (const PXDCluster& cluster2 : storeClusters) {
        int iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        int index2 = planeToIndex(iPlane2);
        if (index2 == 1) {
          m_CorrelationU->Fill(cluster1.getU(), cluster2.getU());
          m_CorrelationV->Fill(cluster1.getV(), cluster2.getV());
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
