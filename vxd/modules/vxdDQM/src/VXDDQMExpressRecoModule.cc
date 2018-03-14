/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "vxd/modules/vxdDQM/VXDDQMExpressRecoModule.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>

#include <TMath.h>

#include <boost/format.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDDQMExpressReco)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDDQMExpressRecoModule::VXDDQMExpressRecoModule() : HistoModule()
{
  //Set module properties
  setDescription("VXD DQM module for Express Reco "
                 "Recommended Number of events for monitor is 40 kEvents or more to fill all histograms "
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("UseDigits", m_UseDigits,
           "flag <0,1> for using digits only, no cluster information will be required, default = 0 ", m_UseDigits);
  addParam("CorrelationGranulation", m_CorrelationGranulation,
           "Set granulation of histogram plots, default is 1 degree, min = 0.02 degree, max = 1 degree ", m_CorrelationGranulation);
  addParam("SwapPXD", m_SwapPXD, "flag <0,1> very special case for swap of phi-theta coordinates, default = 0 ", m_SwapPXD);
  addParam("CutCorrelationSigPXD", m_CutCorrelationSigPXD,
           "Cut threshold of PXD signal for accepting to correlations, default = 0 ADU ", m_CutCorrelationSigPXD);
  addParam("CutCorrelationSigUSVD", m_CutCorrelationSigUSVD,
           "Cut threshold of SVD signal for accepting to correlations in u, default = 0 ADU ", m_CutCorrelationSigUSVD);
  addParam("CutCorrelationSigVSVD", m_CutCorrelationSigVSVD,
           "Cut threshold of SVD signal for accepting to correlations in v, default = 0 ADU ", m_CutCorrelationSigVSVD);
  addParam("CutCorrelationTimeSVD", m_CutCorrelationTimeSVD,
           "Cut threshold of SVD time window for accepting to correlations, default = 70 ns ", m_CutCorrelationTimeSVD);

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("VXDExpReco"));
}


VXDDQMExpressRecoModule::~VXDDQMExpressRecoModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void VXDDQMExpressRecoModule::defineHisto()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) {
    B2WARNING("Missing geometry for VXD, check steering file.");
    return;
  }
  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD.");
  }
  if (gTools->getNumberOfSVDLayers() == 0) {
    B2WARNING("Missing geometry for SVD.");
  }

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  if (m_CorrelationGranulation > 5.0) m_CorrelationGranulation = 5.0;  //  set maximum of gramularity to 1 degree
  if (m_CorrelationGranulation < 0.02) m_CorrelationGranulation = 0.02;  //  set minimum of gramularity to 0.02 degree

  // if binning go over h_MaxBins it decrease preset of range
  int h_MaxBins = 2000;       //maximal size of histogram binning:

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  // basic constants presets:
  int nVXDLayers = gTools->getNumberOfLayers();

  // Create basic histograms:
  m_correlationsSP = (TH2F**) new TH2F*[nVXDLayers * nVXDLayers];
  m_correlationsSP1DPhi = (TH1F**) new TH1F*[nVXDLayers * nVXDLayers];
  m_correlationsSP1DTheta = (TH1F**) new TH1F*[nVXDLayers * nVXDLayers];


  for (int i = 0; i < nVXDLayers; i++) {
    for (int j = 0; j < nVXDLayers; j++) {
      m_correlationsSP[nVXDLayers * j + i] = NULL;
      m_correlationsSP1DPhi[nVXDLayers * j + i] = NULL;
      m_correlationsSP1DTheta[nVXDLayers * j + i] = NULL;
    }
  }

  string Diru = str(format("Phi"));
  string Dirv = str(format("Theta"));
  string Unit = str(format("degree"));
  string AxisTitPhi = str(format("%1% position [%2%]") % Diru.c_str() % Unit.c_str());  // 0..360 degree, from x+ to y+...
  string AxisTitTheta = str(format("%1% position [%2%]") % Dirv.c_str() % Unit.c_str());  // 0..180 degree, z+ to z-
  if (m_UseDigits) {
    AxisTitPhi = str(format("From digits: %1%") % AxisTitPhi);
    AxisTitTheta = str(format("From digits: %1%") % AxisTitTheta);
  }
  for (VxdID layer1 : geo.getLayers()) {
    int i = layer1.getLayerNumber() - gTools->getFirstLayer();
    float uSize1s, uSize1e;
    float vSize1s, vSize1e;
    int nStripsU1, nStripsV1;
    uSize1s = -180.0;  // degree
    uSize1e = 180.0;   // degree
    vSize1s = 0.0;     // degree
    vSize1e = 180.0;   // degree
    nStripsU1 = (uSize1e - uSize1s) / m_CorrelationGranulation;
    if (nStripsU1 > h_MaxBins) nStripsU1 = h_MaxBins;
    nStripsV1 = nStripsU1;
    for (VxdID layer2 : geo.getLayers()) {
      int j = layer2.getLayerNumber() - gTools->getFirstLayer();
      float uSize2s, uSize2e;
      float vSize2s, vSize2e;
      int nStripsU2, nStripsV2;
      uSize2s = -180.0;  // degree
      uSize2e = 180.0;   // degree
      vSize2s = 0.0;     // degree
      vSize2e = 180.0;   // degree
      nStripsU2 = nStripsU1;
      nStripsV2 = nStripsV1;

      if (i == j) {  // hit maps
        string nameSP = str(format("DQMER_VXD_Hitmap_L%1%") % layer2.getLayerNumber());
        string titleSP = str(format("DQM ER VXD Hitmap, layer %1%") % layer2.getLayerNumber());
        m_correlationsSP[nVXDLayers * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(),
                                                        nStripsU2, uSize2s, uSize2e,
                                                        nStripsV2, vSize2s, vSize2e);
        m_correlationsSP[nVXDLayers * j + i]->GetXaxis()->SetTitle(AxisTitPhi.c_str());
        m_correlationsSP[nVXDLayers * j + i]->GetYaxis()->SetTitle(AxisTitTheta.c_str());
        m_correlationsSP[nVXDLayers * j + i]->GetZaxis()->SetTitle("hits");

        nameSP = str(format("DQMER_VXD_Hitmap_%1%_L%2%") % Diru.c_str() % layer2.getLayerNumber());
        titleSP = str(format("DQM ER VXD Hitmap in %1%, layer %2%") % Diru.c_str() % layer2.getLayerNumber());
        m_correlationsSP1DPhi[nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                             nStripsU2, uSize2s, uSize2e);
        m_correlationsSP1DPhi[nVXDLayers * j + i]->GetXaxis()->SetTitle(AxisTitPhi.c_str());
        m_correlationsSP1DPhi[nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");

        nameSP = str(format("DQMER_VXD_Hitmap_%1%_L%2%") % Dirv.c_str() % layer2.getLayerNumber());
        titleSP = str(format("DQM ER VXD Hitmap in %1%, layer %2%") % Dirv.c_str() % layer2.getLayerNumber());
        m_correlationsSP1DTheta[nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                               nStripsV2, vSize2s, vSize2e);
        m_correlationsSP1DTheta[nVXDLayers * j + i]->GetXaxis()->SetTitle(AxisTitTheta.c_str());
        m_correlationsSP1DTheta[nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");

      } else if (i < j) { // correlations for Phi
        string nameSP = str(format("DQMER_VXD_Correlations_%1%_L%2%_L%3%") % Diru.c_str() % layer1.getLayerNumber() %
                            layer2.getLayerNumber());
        string titleSP = str(format("DQM ER VXD Correlations in %1%, layers %2% %3%") % Diru.c_str() % layer1.getLayerNumber() %
                             layer2.getLayerNumber());
        m_correlationsSP[nVXDLayers * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(),
                                                        nStripsU1, uSize1s, uSize1e,
                                                        nStripsU2, uSize2s, uSize2e);
        string axisxtitle = str(format("%1%, layer %2%") % AxisTitPhi.c_str() % layer1.getLayerNumber());
        string axisytitle = str(format("%1%, layer %2%") % AxisTitPhi.c_str() % layer2.getLayerNumber());
        m_correlationsSP[nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsSP[nVXDLayers * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsSP[nVXDLayers * j + i]->GetZaxis()->SetTitle("hits");

        nameSP = str(format("DQMER_VXD_1D_Correlations_%1%_L%2%_L%3%") % Diru.c_str() % layer1.getLayerNumber() % layer2.getLayerNumber());
        titleSP = str(format("DQM ER VXD 1D Correlations in %1%, layers %2% %3%") % Diru.c_str() % layer1.getLayerNumber() %
                      layer2.getLayerNumber());
        m_correlationsSP1DPhi[nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                             nStripsU1, uSize1s, uSize1e);
        axisxtitle = str(format("%1%, layer %2% - %3%") % AxisTitPhi.c_str() % layer1.getLayerNumber() % layer2.getLayerNumber());
        m_correlationsSP1DPhi[nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsSP1DPhi[nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");
      } else {       // correlations for Theta
        string nameSP = str(format("DQMER_VXD_Correlations_%1%_L%2%_L%3%") % Dirv.c_str() % layer1.getLayerNumber() %
                            layer2.getLayerNumber());
        string titleSP = str(format("DQM ER VXD Correlations in %1%, layers %2% %3%") % Dirv.c_str() % layer1.getLayerNumber() %
                             layer2.getLayerNumber());
        m_correlationsSP[nVXDLayers * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(),
                                                        nStripsV1, vSize1s, vSize1e,
                                                        nStripsV2, vSize2s, vSize2e);
        string axisxtitle = str(format("%1%, layer %2%") % AxisTitTheta.c_str() % layer1.getLayerNumber());
        string axisytitle = str(format("%1%, layer %2%") % AxisTitTheta.c_str() % layer2.getLayerNumber());
        m_correlationsSP[nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsSP[nVXDLayers * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsSP[nVXDLayers * j + i]->GetZaxis()->SetTitle("hits");

        nameSP = str(format("DQMER_VXD_1D_Correlations_%1%_L%2%_L%3%") % Dirv.c_str() % layer1.getLayerNumber() % layer2.getLayerNumber());
        titleSP = str(format("DQM ER VXD 1D Correlations in %1%, layers %2% %3%") % Dirv.c_str() % layer1.getLayerNumber() %
                      layer2.getLayerNumber());
        m_correlationsSP1DTheta[nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                               nStripsV1, -vSize1e, vSize1e);
        axisxtitle = str(format("%1%, layer %2% - %3%") % AxisTitTheta.c_str() % layer1.getLayerNumber() % layer2.getLayerNumber());
        m_correlationsSP1DTheta[nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsSP1DTheta[nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");
      }
    }
  }

  oldDir->cd();
}


void VXDDQMExpressRecoModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() != 0) {
    //Register collections
    StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
    StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
    StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
    StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
    RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
    RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDShaperDigits);
    m_storePXDClustersName = storePXDClusters.getName();
    m_relPXDClusterDigitName = relPXDClusterDigits.getName();
    m_storeSVDClustersName = storeSVDClusters.getName();
    m_relSVDClusterDigitName = relSVDClusterDigits.getName();

    storePXDDigits.isRequired();
    storeSVDShaperDigits.isRequired();

    //Store names to speed up creation later
    m_storePXDDigitsName = storePXDDigits.getName();
    m_storeSVDShaperDigitsName = storeSVDShaperDigits.getName();
  }
}

void VXDDQMExpressRecoModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) return;

  // Just to make sure, reset all the histograms.
  int nVXDLayers = gTools->getNumberOfLayers();
  for (int i = 0; i < nVXDLayers; i++) {
    for (int j = 0; j < nVXDLayers; j++) {
      if (m_correlationsSP[nVXDLayers * j + i] != NULL) m_correlationsSP[nVXDLayers * j + i]->Reset();
      if (m_correlationsSP1DPhi[nVXDLayers * j + i] != NULL) m_correlationsSP1DPhi[nVXDLayers * j + i]->Reset();
      if (m_correlationsSP1DTheta[nVXDLayers * j + i] != NULL) m_correlationsSP1DTheta[nVXDLayers * j + i]->Reset();
    }
  }
}


void VXDDQMExpressRecoModule::event()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) return;

  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);

  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);

  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDShaperDigits, m_relSVDClusterDigitName);

  // If there are no digits, leave
  if (!storePXDDigits && !storeSVDShaperDigits) return;

  // basic constants presets:
  int nVXDLayers = gTools->getNumberOfLayers();
  int firstVXDLayer = gTools->getFirstLayer();
  int firstPXDLayer = gTools->getFirstPXDLayer();
  int lastPXDLayer = gTools->getLastPXDLayer();
  int firstSVDLayer = gTools->getFirstSVDLayer();
  int lastSVDLayer = gTools->getLastSVDLayer();

  int MaxHits = 0;
  if (m_UseDigits) {
    MaxHits = storeSVDShaperDigits.getEntries() + storePXDDigits.getEntries();
  } else {
    MaxHits = storeSVDClusters.getEntries() + storePXDClusters.getEntries();
  }

  // If there are no hits, leave
  if (MaxHits == 0) return;

  for (int i1 = 0; i1 < MaxHits; i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fTime1 = 0.0;
    float fPosSPU1 = 0.0;
    float fPosSPV1 = 0.0;
    int iIsPXD1 = 0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iLayer1 = 0;
    if (m_UseDigits) {  // DIGITS:
      if (i1 < storePXDDigits.getEntries()) {  // PXD digits/clusters:
        const PXDDigit& digitPXD1 = *storePXDDigits[i1];
        iLayer1 = digitPXD1.getSensorID().getLayerNumber();
        if ((iLayer1 < firstPXDLayer) || (iLayer1 > lastPXDLayer)) continue;
        index1 = iLayer1 - firstVXDLayer;
        float fCharge1 = digitPXD1.getCharge();
        if (fCharge1 < m_CutCorrelationSigPXD) continue;
        VxdID sensorID1 = digitPXD1.getSensorID();
        auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
        TVector3 rLocal1(info.getUCellPosition(digitPXD1.getUCellID()), info.getVCellPosition(digitPXD1.getVCellID()), 0);
        TVector3 ral1 = info.pointToGlobal(rLocal1);
        iIsPXD1 = 1;
        iIsU1 = 1;
        iIsV1 = 1;
        fPosSPU1 = ral1.Phi() / TMath::Pi() * 180;
        fPosSPV1 = ral1.Theta() / TMath::Pi() * 180;
        if (m_SwapPXD) {
          fPosSPV1 = ral1.Phi() / TMath::Pi() * 180;
          fPosSPU1 = ral1.Theta() / TMath::Pi() * 180;
        }
      } else {                                  // SVD digits/clusters:
        const SVDShaperDigit& digitSVD1 = *storeSVDShaperDigits[i1 - storePXDDigits.getEntries()];
        iLayer1 = digitSVD1.getSensorID().getLayerNumber();
        if ((iLayer1 < firstSVDLayer) || (iLayer1 > lastSVDLayer)) continue;
        index1 = iLayer1 - firstVXDLayer;
        fTime1 = digitSVD1.getFADCTime();
        VxdID sensorID1 = digitSVD1.getSensorID();
        auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
        SVDShaperDigit::APVFloatSamples samples = digitSVD1.getSamples();
        if (digitSVD1.isUStrip()) {
          int iCont = 0;
          for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
            float fCharge1 = samples[i];
            if (fCharge1 > m_CutCorrelationSigUSVD) iCont = 1;
          }
          if (iCont == 0) continue;
          float possi = info.getUCellPosition(digitSVD1.getCellID());
          TVector3 rLocal1(possi, 0 , 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsU1 = 1;
          fPosSPU1 = ral1.Phi() / TMath::Pi() * 180;
        } else {
          int iCont = 0;
          for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
            float fCharge1 = samples[i];
            if (fCharge1 > m_CutCorrelationSigVSVD) iCont = 1;
          }
          if (iCont == 0) continue;

          //float possi = digitSVD1.getCellPosition();  //   is not work anymore
          float possi = info.getVCellPosition(digitSVD1.getCellID());

          TVector3 rLocal1(0, possi, 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsV1 = 1;
          fPosSPV1 = ral1.Theta() / TMath::Pi() * 180;
        }
      }
    } else {  // CLUSTERS:
      if (i1 < storePXDClusters.getEntries()) {  // PXD digits/clusters:
        const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
        iLayer1 = clusterPXD1.getSensorID().getLayerNumber();
        if ((iLayer1 < firstPXDLayer) || (iLayer1 > lastPXDLayer)) continue;
        index1 = iLayer1 - firstVXDLayer;
        float fCharge1 = clusterPXD1.getCharge();
        if (fCharge1 < m_CutCorrelationSigPXD) continue;
        VxdID sensorID1 = clusterPXD1.getSensorID();
        auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
        TVector3 rLocal1(clusterPXD1.getU(), clusterPXD1.getV(), 0);
        TVector3 ral1 = info.pointToGlobal(rLocal1);
        iIsPXD1 = 1;
        iIsU1 = 1;
        iIsV1 = 1;
        fPosSPU1 = ral1.Phi() / TMath::Pi() * 180;
        fPosSPV1 = ral1.Theta() / TMath::Pi() * 180;
        if (m_SwapPXD) {
          fPosSPV1 = ral1.Phi() / TMath::Pi() * 180;
          fPosSPU1 = ral1.Theta() / TMath::Pi() * 180;
        }
      } else {                                  // SVD digits/clusters:
        const SVDCluster& clusterSVD1 = *storeSVDClusters[i1 - storePXDClusters.getEntries()];
        iLayer1 = clusterSVD1.getSensorID().getLayerNumber();
        if ((iLayer1 < firstSVDLayer) || (iLayer1 > lastSVDLayer)) continue;
        index1 = iLayer1 - firstVXDLayer;
        float fCharge1 = clusterSVD1.getCharge();
        fTime1 = clusterSVD1.getClsTime();
        VxdID sensorID1 = clusterSVD1.getSensorID();
        auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
        if (clusterSVD1.isUCluster()) {
          if (fCharge1 < m_CutCorrelationSigUSVD * 200) continue;  // in electrons
          TVector3 rLocal1(clusterSVD1.getPosition(), 0 , 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsU1 = 1;
          fPosSPU1 = ral1.Phi() / TMath::Pi() * 180;
        } else {
          if (fCharge1 < m_CutCorrelationSigVSVD * 200) continue;  // in electrons
          TVector3 rLocal1(0, clusterSVD1.getPosition(), 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsV1 = 1;
          fPosSPV1 = ral1.Theta() / TMath::Pi() * 180;
        }
      }
    }
    // hit maps for PXD:
    if ((iIsU1 == 1) && (iIsV1 == 1)) {
      if (m_correlationsSP[nVXDLayers * index1 + index1] != NULL)
        m_correlationsSP[nVXDLayers * index1 + index1]->Fill(fPosSPU1, fPosSPV1);
      if (m_correlationsSP1DPhi[nVXDLayers * index1 + index1] != NULL)
        m_correlationsSP1DPhi[nVXDLayers * index1 + index1]->Fill(fPosSPU1);
      if (m_correlationsSP1DTheta[nVXDLayers * index1 + index1] != NULL)
        m_correlationsSP1DTheta[nVXDLayers * index1 + index1]->Fill(fPosSPV1);
    }
    for (int i2 = 0; i2 < MaxHits; i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fTime2 = 0.0;
      float fPosSPU2 = 0.0;
      float fPosSPV2 = 0.0;
      int iIsPXD2 = 0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iLayer2 = 0;
      if (m_UseDigits) {  // DIGITS:
        if (i2 < storePXDDigits.getEntries()) {  // PXD digits/clusters:
          const PXDDigit& digitPXD2 = *storePXDDigits[i2];
          iLayer2 = digitPXD2.getSensorID().getLayerNumber();
          if ((iLayer2 < firstPXDLayer) || (iLayer2 > lastPXDLayer)) continue;
          index2 = iLayer2 - firstVXDLayer;
          float fCharge2 = digitPXD2.getCharge();
          if (fCharge2 < m_CutCorrelationSigPXD) continue;
          VxdID sensorID2 = digitPXD2.getSensorID();
          auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
          TVector3 rLocal2(info.getUCellPosition(digitPXD2.getUCellID()), info.getVCellPosition(digitPXD2.getVCellID()), 0);
          TVector3 ral2 = info.pointToGlobal(rLocal2);
          iIsPXD2 = 1;
          iIsU2 = 1;
          iIsV2 = 1;
          fPosSPU2 = ral2.Phi() / TMath::Pi() * 180;
          fPosSPV2 = ral2.Theta() / TMath::Pi() * 180;
          if (m_SwapPXD) {
            fPosSPV2 = ral2.Phi() / TMath::Pi() * 180;
            fPosSPU2 = ral2.Theta() / TMath::Pi() * 180;
          }
        } else {                                  // SVD digits/clusters:
          const SVDShaperDigit& digitSVD2 = *storeSVDShaperDigits[i2 - storePXDDigits.getEntries()];
          iLayer2 = digitSVD2.getSensorID().getLayerNumber();
          if ((iLayer2 < firstSVDLayer) || (iLayer2 > lastSVDLayer)) continue;
          index2 = iLayer2 - firstVXDLayer;
          fTime2 = digitSVD2.getFADCTime();
          VxdID sensorID2 = digitSVD2.getSensorID();
          auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID2));

          SVDShaperDigit::APVFloatSamples samples = digitSVD2.getSamples();
          if (digitSVD2.isUStrip()) {
            int iCont = 0;
            for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
              float fCharge2 = samples[i];
              if (fCharge2 > m_CutCorrelationSigUSVD) iCont = 1;
            }
            if (iCont == 0) continue;
            float possi = info.getUCellPosition(digitSVD2.getCellID());
            TVector3 rLocal2(possi, 0 , 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsU2 = 1;
            fPosSPU2 = ral2.Phi() / TMath::Pi() * 180;
          } else {
            int iCont = 0;
            for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
              float fCharge2 = samples[i];
              if (fCharge2 > m_CutCorrelationSigVSVD) iCont = 1;
            }
            if (iCont == 0) continue;
            float possi = info.getVCellPosition(digitSVD2.getCellID());
            TVector3 rLocal2(0, possi, 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsV2 = 1;
            fPosSPV2 = ral2.Theta() / TMath::Pi() * 180;
          }
        }
      } else { // CLUSTERS:
        if (i2 < storePXDClusters.getEntries()) {  // PXD digits/clusters:
          const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
          iLayer2 = clusterPXD2.getSensorID().getLayerNumber();
          if ((iLayer2 < firstPXDLayer) || (iLayer2 > lastPXDLayer)) continue;
          index2 = iLayer2 - firstVXDLayer;
          float fCharge2 = clusterPXD2.getCharge();
          if (fCharge2 < m_CutCorrelationSigPXD) continue;
          VxdID sensorID2 = clusterPXD2.getSensorID();
          auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
          TVector3 rLocal2(clusterPXD2.getU(), clusterPXD2.getV(), 0);
          TVector3 ral2 = info.pointToGlobal(rLocal2);
          iIsPXD2 = 1;
          iIsU2 = 1;
          iIsV2 = 1;
          fPosSPU2 = ral2.Phi() / TMath::Pi() * 180;
          fPosSPV2 = ral2.Theta() / TMath::Pi() * 180;
          if (m_SwapPXD) {
            fPosSPV2 = ral2.Phi() / TMath::Pi() * 180;
            fPosSPU2 = ral2.Theta() / TMath::Pi() * 180;
          }
        } else {                                  // SVD digits/clusters:
          const SVDCluster& clusterSVD2 = *storeSVDClusters[i2 - storePXDClusters.getEntries()];
          iLayer2 = clusterSVD2.getSensorID().getLayerNumber();
          if ((iLayer2 < firstSVDLayer) || (iLayer2 > lastSVDLayer)) continue;
          index2 = iLayer2 - firstVXDLayer;
          float fCharge2 = clusterSVD2.getCharge();
          fTime2 = clusterSVD2.getClsTime();
          VxdID sensorID2 = clusterSVD2.getSensorID();
          auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
          if (clusterSVD2.isUCluster()) {
            if (fCharge2 < m_CutCorrelationSigUSVD * 200) continue;  // in electrons
            TVector3 rLocal2(clusterSVD2.getPosition(), 0 , 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsU2 = 1;
            fPosSPU2 = ral2.Phi() / TMath::Pi() * 180;
          } else {
            if (fCharge2 < m_CutCorrelationSigVSVD * 200) continue;  // in electrons
            TVector3 rLocal2(0, clusterSVD2.getPosition(), 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsV2 = 1;
            fPosSPV2 = ral2.Theta() / TMath::Pi() * 180;
          }
        }
      }
      if ((iIsPXD1 == 0) && (iIsPXD2 == 0))
        if ((fabs(fTime1 - fTime2)) > m_CutCorrelationTimeSVD) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1) && (iIsPXD1 == 0) && (iIsPXD2 == 0)) {
        // hit maps for SVD:
        if (m_correlationsSP[nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP[nVXDLayers * index2 + index1]->Fill(fPosSPU1, fPosSPV2);
        if (m_correlationsSP1DPhi[nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP1DPhi[nVXDLayers * index2 + index1]->Fill(fPosSPU1);
        if (m_correlationsSP1DTheta[nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP1DTheta[nVXDLayers * index2 + index1]->Fill(fPosSPV2);
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        if (m_correlationsSP[nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP[nVXDLayers * index2 + index1]->Fill(fPosSPU1, fPosSPU2);
        if (m_correlationsSP1DPhi[nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP1DPhi[nVXDLayers * index2 + index1]->Fill(fPosSPU2 - fPosSPU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        if (m_correlationsSP[nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP[nVXDLayers * index2 + index1]->Fill(fPosSPV2, fPosSPV1);
        if (m_correlationsSP1DTheta[nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP1DTheta[nVXDLayers * index2 + index1]->Fill(fPosSPV2 - fPosSPV1);
      }
    }
  }

}
