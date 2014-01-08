#include "testbeam/vxd/modules/DQM/SVDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <set>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TVector3.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMModule::SVDDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
}


SVDDQMModule::~SVDDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMModule::defineHisto()
{
  // Fired strip counts U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hFiredU%1%") % iPlane);
    string title = str(format("Fired pixels in U, plane %1%") % iPlane);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# of fired u strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
  }
  // Fired strip counts V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hFiredV%1%") % iPlane);
    string title = str(format("Fired pixels in V, plane %1%") % iPlane);
    m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# of fired v strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
  }
  // Number of clusters U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hClustersU%1%") % iPlane);
    string title = str(format("Number of clusters in U, plane %1%") % iPlane);
    m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
  }
  // Number of clusters V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hClustersV%1%") % iPlane);
    string title = str(format("Number of clusters in V, plane %1%") % iPlane);
    m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersV[i]->GetXaxis()->SetTitle("# of v clusters");
    m_clustersV[i]->GetYaxis()->SetTitle("count");
  }
  // Hitmaps in U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hHitmapU%1%") % iPlane);
    string title = str(format("Hitmap in U, plane %1%") % iPlane);
    float length = getInfo(i).getLength();
    int nStrips = getInfo(i).getUCells();
    m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nStrips, -0.5 * length, 0.5 * length);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
  }
  // Hitmaps in V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hHitmapV%1%") % iPlane);
    string title = str(format("Hitmap in V, plane %1%") % iPlane);
    float width = getInfo(i).getWidth();
    int nStrips = getInfo(i).getVCells();
    m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nStrips, -0.5 * width, 0.5 * width);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
  }
}


void SVDDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  //storeClusters.required();
  //storeDigits.required();

  RelationArray relClusterDigits(storeClusters, storeDigits);

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relClusterDigitName = relClusterDigits.getName();
}

void SVDDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_firedU[i]->Reset();
    m_firedV[i]->Reset();
    m_clustersU[i]->Reset();
    m_clustersV[i]->Reset();
    m_hitMapU[i]->Reset();
    m_hitMapV[i]->Reset();
  }
}


void SVDDQMModule::event()
{
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDCluster> storeClusters(m_storeClustersName);

  const RelationArray relClusterDigits(storeClusters, storeDigits, m_relClusterDigitName);

  // Fired strips
  vector< set<int> > uStrips(c_nSVDPlanes); // sets to eliminate multiple samples per strip
  vector< set<int> > vStrips(c_nSVDPlanes);
  for (const SVDDigit & digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    if (digit.isUStrip())
      uStrips.at(index).insert(digit.getCellID());
    else
      vStrips.at(index).insert(digit.getCellID());
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_firedU[i]->Fill(0.1667 * uStrips[i].size()); // FIXME:Account for 6 samples
    m_firedV[i]->Fill(0.1667 * vStrips[i].size());
  }

  // Cluster counts - same as strips
  int countsU[c_nSVDPlanes];
  int countsV[c_nSVDPlanes];
  for (int i = 0; i < c_nSVDPlanes; i++) {
    countsU[i] = 0; countsV[i] = 0;
  }
  for (const SVDCluster & cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    if (cluster.isUCluster())
      countsU[index]++;
    else
      countsV[index]++;
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_clustersU[i]->Fill(countsU[i]);
    m_clustersV[i]->Fill(countsV[i]);
  }

  // Hitmaps
  for (const SVDCluster & cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    if (cluster.isUCluster())
      m_hitMapU[index]->Fill(cluster.getPosition());
    else
      m_hitMapV[index]->Fill(cluster.getPosition());
  }

}


void SVDDQMModule::endRun()
{
}


void SVDDQMModule::terminate()
{
}
