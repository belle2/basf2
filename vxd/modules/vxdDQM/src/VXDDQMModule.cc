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

#include "vxd/modules/vxdDQM/VXDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <TMath.h>

//#include <set>
#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDDQMModule::VXDDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("VXD DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("UseDigits", m_UseDigits,
           "flag <0,1> for using digits only, no cluster information will be required, default = 0 ", m_UseDigits);
  addParam("Reduce1DCorrelHistos", m_Reduce1DCorrelHistos,
           "flag <0,1> for removing of 1D correlation plots from output, default = 0 ", m_Reduce1DCorrelHistos);
  addParam("Reduce2DCorrelHistos", m_Reduce2DCorrelHistos,
           "flag <0,1> for removing of 2D correlation plots from output, default = 0 ", m_Reduce2DCorrelHistos);
  addParam("Only23LayersHistos", m_Only23LayersHistos,
           "flag <0,1> for to keep only correlation plots between layer 2 and 3 (between PXD and SVD), default = 0 ", m_Only23LayersHistos);
  addParam("SaveOtherHistos", m_SaveOtherHistos,
           "flag <0,1> for creation of correlation plots for non-neighboar layers, default = 0 ", m_SaveOtherHistos);
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

}


VXDDQMModule::~VXDDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void VXDDQMModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;

  if (m_CorrelationGranulation > 1.0) m_CorrelationGranulation = 1.0;  //  set maximum of gramularity to 1 degree
  if (m_CorrelationGranulation < 0.02) m_CorrelationGranulation = 0.02;  //  set minimum of gramularity to 0.02 degree

  // if binning go over h_MaxBins it decrease preset of range
  int h_MaxBins = 2000;       //maximal size of histogram binning:

  // basic constants presets:
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  c_nVXDLayers = geo.getLayers().size();
  c_firstVXDLayer = 1;  // counting start from 1...
  c_lastVXDLayer = c_nVXDLayers;
  c_nPXDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size();
  c_firstPXDLayer = c_firstVXDLayer;
  c_lastPXDLayer = c_nPXDLayers;
  c_nSVDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::SVD).size();
  c_firstSVDLayer = c_nPXDLayers + c_firstPXDLayer;
  c_lastSVDLayer = c_firstSVDLayer + c_nSVDLayers;

  c_MaxLaddersInPXDLayer = 0;
  c_MaxLaddersInSVDLayer = 0;
  c_MaxSensorsInPXDLayer = 0;
  c_MaxSensorsInSVDLayer = 0;

  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() <= c_lastPXDLayer) {  // PXD
        if (c_MaxLaddersInPXDLayer < geo.getLadders(layer).size())
          c_MaxLaddersInPXDLayer = geo.getLadders(layer).size();
        if (c_MaxSensorsInPXDLayer < geo.getSensors(ladder).size())
          c_MaxSensorsInPXDLayer = geo.getSensors(ladder).size();
      } else { // SVD
        if (c_MaxLaddersInSVDLayer < geo.getLadders(layer).size())
          c_MaxLaddersInSVDLayer = geo.getLadders(layer).size();
        if (c_MaxSensorsInSVDLayer < geo.getSensors(ladder).size())
          c_MaxSensorsInSVDLayer = geo.getSensors(ladder).size();
      }
      break;
    }
  }

  // 1D histograms in global coordinates - save a size but not so clear:
  TDirectory* DirVXDCorrels1D_23 = NULL;
  TDirectory* DirVXDCorrels1DNeigh = NULL;
  TDirectory* DirVXDHitmaps1D = NULL;
  TDirectory* DirVXDCorrels1D = NULL;
  if (!m_Reduce1DCorrelHistos) {
    DirVXDCorrels1D_23 = oldDir->mkdir("VXD_1D_Correlations_Layers_23");
    if (!m_Only23LayersHistos) {
      DirVXDCorrels1DNeigh = oldDir->mkdir("VXD_1D_Correlations_Other_Neighboar");
      DirVXDHitmaps1D = oldDir->mkdir("VXD_1D_Hitmaps");
      if (m_SaveOtherHistos) {
        DirVXDCorrels1D = oldDir->mkdir("VXD_1D_Correlations_Others");
      }
    }
  }

  // 2D histograms in global coordinates:
  TDirectory* DirVXDCorrels_23 = NULL;
  TDirectory* DirVXDCorrelsNeigh = NULL;
  TDirectory* DirVXDHitmaps = NULL;
  TDirectory* DirVXDCorrels = NULL;
  if (!m_Reduce2DCorrelHistos) {
    DirVXDCorrels_23 = oldDir->mkdir("VXD_Correlations_Layers_23");
    if (!m_Only23LayersHistos) {
      DirVXDCorrelsNeigh = oldDir->mkdir("VXD_Correlations_Other_Neighboar");
      DirVXDHitmaps = oldDir->mkdir("VXD_Hitmaps");
      if (m_SaveOtherHistos) {
        DirVXDCorrels = oldDir->mkdir("VXD_Correlations_Others");
      }
    }
  }

  m_correlationsSP = (TH2F**) new TH2F*[c_nVXDLayers * c_nVXDLayers];
  m_correlationsSP1DPhi = (TH1F**) new TH1F*[c_nVXDLayers * c_nVXDLayers];
  m_correlationsSP1DTheta = (TH1F**) new TH1F*[c_nVXDLayers * c_nVXDLayers];


  for (int i = 0; i < c_nVXDLayers; i++) {
    for (int j = 0; j < c_nVXDLayers; j++) {
      m_correlationsSP[c_nVXDLayers * j + i] = NULL;
      m_correlationsSP1DPhi[c_nVXDLayers * j + i] = NULL;
      m_correlationsSP1DTheta[c_nVXDLayers * j + i] = NULL;
    }
  }

  string AxisTitPhi = str(format("Phi position [degree]"));  // 0..360 degree, from x+ to y+...
  string AxisTitTheta = str(format("Theta position [degree]"));  // 0..180 degree, z+ to z-
  if (m_UseDigits) {
    AxisTitPhi = str(format("From digits: %1%") % AxisTitPhi);
    AxisTitTheta = str(format("From digits: %1%") % AxisTitTheta);
  }
  for (VxdID layer1 : geo.getLayers()) {
    int iLayer1 = layer1.getLayerNumber();
    int i = iLayer1 - c_firstVXDLayer;
    float vSize1, uSize1;
    int nStripsU1, nStripsV1;
    uSize1 = 360.0;  // degree
    vSize1 = 180.0;  // degree

    nStripsU1 = uSize1 / m_CorrelationGranulation;
    nStripsV1 = vSize1 / m_CorrelationGranulation;

    if (nStripsU1 > h_MaxBins) {
      uSize1 = h_MaxBins * m_CorrelationGranulation;
      nStripsU1 = uSize1 / m_CorrelationGranulation;
    }
    if (nStripsV1 > h_MaxBins) {
      vSize1 = h_MaxBins * m_CorrelationGranulation;
      nStripsV1 = vSize1 / m_CorrelationGranulation;
    }

    for (VxdID layer2 : geo.getLayers()) {
      int iLayer2 = layer2.getLayerNumber();
      int j = iLayer2 - c_firstVXDLayer;
      float vSize2, uSize2;
      int nStripsU2, nStripsV2;
      uSize2 = 360.0;  // degree
      vSize2 = 180.0;  // degree

      nStripsU2 = uSize2 / m_CorrelationGranulation;
      nStripsV2 = vSize2 / m_CorrelationGranulation;

      if (nStripsU2 > h_MaxBins) {
        uSize2 = h_MaxBins * m_CorrelationGranulation;
        nStripsU2 = uSize2 / m_CorrelationGranulation;
      }
      if (nStripsV2 > h_MaxBins) {
        vSize2 = h_MaxBins * m_CorrelationGranulation;
        nStripsV2 = vSize2 / m_CorrelationGranulation;
      }

      if (i == j) {  // hit maps
        if ((!m_Reduce2DCorrelHistos) && (!m_Only23LayersHistos)) {
          DirVXDHitmaps->cd();
          string nameSP = str(format("VXD_Hitmap_Ang_Occ_L%1%") % iLayer2);
          string titleSP = str(format("VXD Hitmap Angular Occupancy, layer %1%") % iLayer2);
          m_correlationsSP[c_nVXDLayers * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(),
                                                            nStripsU2, -uSize2 / 2.0, uSize2 / 2.0, nStripsV2, 0.0, vSize2);
          m_correlationsSP[c_nVXDLayers * j + i]->GetXaxis()->SetTitle(AxisTitPhi.c_str());
          m_correlationsSP[c_nVXDLayers * j + i]->GetYaxis()->SetTitle(AxisTitTheta.c_str());
          m_correlationsSP[c_nVXDLayers * j + i]->GetZaxis()->SetTitle("hits");
        }

        if ((!m_Reduce1DCorrelHistos) && (!m_Only23LayersHistos)) {
          DirVXDHitmaps1D->cd();
          string nameSP = str(format("VXD_Hitmap_Phi_L%1%") % iLayer2);
          string titleSP = str(format("VXD Hitmap in Phi, layer %1%") % iLayer2);
          m_correlationsSP1DPhi[c_nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                                 nStripsU2, -uSize2 / 2.0, uSize2 / 2.0);
          m_correlationsSP1DPhi[c_nVXDLayers * j + i]->GetXaxis()->SetTitle(AxisTitPhi.c_str());
          m_correlationsSP1DPhi[c_nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");
          nameSP = str(format("VXD_Hitmap_Theta_L%1%") % iLayer2);
          titleSP = str(format("VXD Hitmap in Theta, layer %1%") % iLayer2);
          m_correlationsSP1DTheta[c_nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                                   nStripsV2, 0.0, vSize2);
          m_correlationsSP1DTheta[c_nVXDLayers * j + i]->GetXaxis()->SetTitle(AxisTitTheta.c_str());
          m_correlationsSP1DTheta[c_nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");
        }
      } else if (i < j) { // correlations for Phi
        if ((!m_SaveOtherHistos) && (abs(i - j) > 1)) continue;
        if (!m_Reduce2DCorrelHistos) {
          if (abs(i - j) > 1) {
            DirVXDCorrels->cd();
          } else {
            DirVXDCorrelsNeigh->cd();
          }
          if (((i == (c_lastPXDLayer - c_firstVXDLayer)) &&
               (j == (c_firstSVDLayer - c_firstVXDLayer))) ||
              ((i == (c_firstSVDLayer - c_firstVXDLayer)) &&
               (j == (c_lastPXDLayer - c_firstVXDLayer)))) {
            DirVXDCorrels_23->cd();
          }
          string nameSP = str(format("VXD_Correlations_Phi_L%1%_L%2%") % iLayer1 % iLayer2);
          string titleSP = str(format("VXD Correlations in Phi, layers %1% %2%") % iLayer1 % iLayer2);
          m_correlationsSP[c_nVXDLayers * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(),
                                                            nStripsU1, -0.5 * uSize1, 0.5 * uSize1,
                                                            nStripsU2, -0.5 * uSize2, 0.5 * uSize2);
          string axisxtitle = str(format("%1%, layer %2%") % AxisTitPhi.c_str() % iLayer1);
          string axisytitle = str(format("%1%, layer %2%") % AxisTitPhi.c_str() % iLayer2);
          m_correlationsSP[c_nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
          m_correlationsSP[c_nVXDLayers * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
          m_correlationsSP[c_nVXDLayers * j + i]->GetZaxis()->SetTitle("hits");
        }
        if (!m_Reduce1DCorrelHistos) {
          if (abs(i - j) > 1) {
            DirVXDCorrels1D->cd();
          } else {
            DirVXDCorrels1DNeigh->cd();
          }
          if (((i == (c_lastPXDLayer - c_firstVXDLayer)) &&
               (j == (c_firstSVDLayer - c_firstVXDLayer))) ||
              ((i == (c_firstSVDLayer - c_firstVXDLayer)) &&
               (j == (c_lastPXDLayer - c_firstVXDLayer)))) {
            DirVXDCorrels1D_23->cd();
          }
          string nameSP = str(format("VXD_1D_Correlations_Phi_L%1%_L%2%") % iLayer1 % iLayer2);
          string titleSP = str(format("VXD 1D Correlations in Phi, layer %1% - %2%") % iLayer1 % iLayer2);
          m_correlationsSP1DPhi[c_nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                                 nStripsU1 * 2, -1.0 * uSize1, 1.0 * uSize1);
          string axisxtitle = str(format("%1%, layer %2% - %3%") % AxisTitPhi.c_str() % iLayer1 % iLayer2);
          m_correlationsSP1DPhi[c_nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
          m_correlationsSP1DPhi[c_nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");

        }
      } else {       // correlations for Theta
        if ((!m_SaveOtherHistos) && (abs(i - j) > 1)) continue;
        if (!m_Reduce2DCorrelHistos) {
          if (abs(i - j) > 1) {
            DirVXDCorrels->cd();
          } else {
            DirVXDCorrelsNeigh->cd();
          }
          if (((i == (c_lastPXDLayer - c_firstVXDLayer)) &&
               (j == (c_firstSVDLayer - c_firstVXDLayer))) ||
              ((i == (c_firstSVDLayer - c_firstVXDLayer)) &&
               (j == (c_lastPXDLayer - c_firstVXDLayer)))) {
            DirVXDCorrels_23->cd();
          }
          string nameSP = str(format("VXD_Correlations_Theta_L%1%_L%2%") % iLayer1 % iLayer2);
          string titleSP = str(format("VXD Correlations in Theta, layers %1% %2%") % iLayer1 % iLayer2);
          m_correlationsSP[c_nVXDLayers * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(),
                                                            nStripsV2, 0.0, vSize2,
                                                            nStripsV1, 0.0, vSize1);
          string axisxtitle = str(format("%1%, layer %2%") % AxisTitTheta.c_str() % iLayer1);
          string axisytitle = str(format("%1%, layer %2%") % AxisTitTheta.c_str() % iLayer2);
          m_correlationsSP[c_nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
          m_correlationsSP[c_nVXDLayers * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
          m_correlationsSP[c_nVXDLayers * j + i]->GetZaxis()->SetTitle("hits");
        }
        if (!m_Reduce1DCorrelHistos) {
          if (abs(i - j) > 1) {
            DirVXDCorrels1D->cd();
          } else {
            DirVXDCorrels1DNeigh->cd();
          }
          if (((i == (c_lastPXDLayer - c_firstVXDLayer)) &&
               (j == (c_firstSVDLayer - c_firstVXDLayer))) ||
              ((i == (c_firstSVDLayer - c_firstVXDLayer)) &&
               (j == (c_lastPXDLayer - c_firstVXDLayer)))) {
            DirVXDCorrels1D_23->cd();
          }
          string nameSP = str(format("VXD_1D_Correlations_Theta_L%1%_L%2%") % iLayer1 % iLayer2);
          string titleSP = str(format("VXD 1D Correlations in Theta, layer %1% - %2%") % iLayer1 % iLayer2);
          m_correlationsSP1DTheta[c_nVXDLayers * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(),
                                                                   nStripsV1 * 2, -1.0 * vSize1, 1.0 * vSize1);
          string axisxtitle = str(format("%1%, layer %2% - %3%") % AxisTitTheta.c_str() % iLayer1 % iLayer2);
          m_correlationsSP1DTheta[c_nVXDLayers * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
          m_correlationsSP1DTheta[c_nVXDLayers * j + i]->GetYaxis()->SetTitle("hits");
        }
      }
    }
  }

  oldDir->cd();
}


void VXDDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
  RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits);
  m_storePXDClustersName = storePXDClusters.getName();
  m_relPXDClusterDigitName = relPXDClusterDigits.getName();
  m_storeSVDClustersName = storeSVDClusters.getName();
  m_relSVDClusterDigitName = relSVDClusterDigits.getName();

  //Store names to speed up creation later
  m_storePXDDigitsName = storePXDDigits.getName();
  m_storeSVDDigitsName = storeSVDDigits.getName();
}

void VXDDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nVXDLayers; i++) {
    for (int j = 0; j < c_nVXDLayers; j++) {
      if (m_correlationsSP[c_nVXDLayers * j + i] != NULL) m_correlationsSP[c_nVXDLayers * j + i]->Reset();
      if (m_correlationsSP1DPhi[c_nVXDLayers * j + i] != NULL) m_correlationsSP1DPhi[c_nVXDLayers * j + i]->Reset();
      if (m_correlationsSP1DTheta[c_nVXDLayers * j + i] != NULL) m_correlationsSP1DTheta[c_nVXDLayers * j + i]->Reset();
    }
  }
}


void VXDDQMModule::event()
{

  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);

  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);

  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits, m_relSVDClusterDigitName);

  // Correlations for space point coordinates
  int MaxHits = 0;
  if (m_UseDigits) {
    MaxHits = storeSVDDigits.getEntries() + storePXDDigits.getEntries();
  } else {
    MaxHits = storeSVDClusters.getEntries() + storePXDClusters.getEntries();
  }
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
        if ((iLayer1 < c_firstPXDLayer) || (iLayer1 > c_lastPXDLayer)) continue;
        index1 = iLayer1 - c_firstVXDLayer;
        float fCharge1 = digitPXD1.getCharge();
        if (fCharge1 < m_CutCorrelationSigPXD) continue;
        VxdID sensorID1 = digitPXD1.getSensorID();
        auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
        TVector3 rLocal1(digitPXD1.getUCellPosition(), digitPXD1.getVCellPosition(), 0);
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
        const SVDDigit& digitSVD1 = *storeSVDDigits[i1 - storePXDDigits.getEntries()];
        iLayer1 = digitSVD1.getSensorID().getLayerNumber();
        if ((iLayer1 < c_firstSVDLayer) || (iLayer1 > c_lastSVDLayer)) continue;
        index1 = iLayer1 - c_firstVXDLayer;
        float fCharge1 = digitSVD1.getCharge();
        fTime1 = digitSVD1.getTime();
        VxdID sensorID1 = digitSVD1.getSensorID();
        auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
        if (digitSVD1.isUStrip()) {
          if (fCharge1 < m_CutCorrelationSigUSVD) continue;
          TVector3 rLocal1(digitSVD1.getCellPosition(), 0 , 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsU1 = 1;
          fPosSPU1 = ral1.Phi() / TMath::Pi() * 180;
        } else {
          if (fCharge1 < m_CutCorrelationSigVSVD) continue;
          TVector3 rLocal1(0, digitSVD1.getCellPosition(), 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsV1 = 1;
          fPosSPV1 = ral1.Theta() / TMath::Pi() * 180;
        }
      }
    } else {  // CLUSTERS:
      if (i1 < storePXDClusters.getEntries()) {  // PXD digits/clusters:
        const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
        iLayer1 = clusterPXD1.getSensorID().getLayerNumber();
        if ((iLayer1 < c_firstPXDLayer) || (iLayer1 > c_lastPXDLayer)) continue;
        index1 = iLayer1 - c_firstVXDLayer;
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
        if ((iLayer1 < c_firstSVDLayer) || (iLayer1 > c_lastSVDLayer)) continue;
        index1 = iLayer1 - c_firstVXDLayer;
        float fCharge1 = clusterSVD1.getCharge();
        fTime1 = clusterSVD1.getClsTime();
        VxdID sensorID1 = clusterSVD1.getSensorID();
        auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
        if (clusterSVD1.isUCluster()) {
          if (fCharge1 < m_CutCorrelationSigUSVD) continue;
          TVector3 rLocal1(clusterSVD1.getPosition(), 0 , 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsU1 = 1;
          fPosSPU1 = ral1.Phi() / TMath::Pi() * 180;
        } else {
          if (fCharge1 < m_CutCorrelationSigVSVD) continue;
          TVector3 rLocal1(0, clusterSVD1.getPosition(), 0);
          TVector3 ral1 = info.pointToGlobal(rLocal1);
          iIsV1 = 1;
          fPosSPV1 = ral1.Theta() / TMath::Pi() * 180;
        }
      }
    }
    // hit maps for PXD:
    if ((iIsU1 == 1) && (iIsV1 == 1)) {
      if (m_correlationsSP[c_nVXDLayers * index1 + index1] != NULL)
        m_correlationsSP[c_nVXDLayers * index1 + index1]->Fill(fPosSPU1, fPosSPV1);
      if (m_correlationsSP1DPhi[c_nVXDLayers * index1 + index1] != NULL)
        if (!m_Reduce1DCorrelHistos)
          m_correlationsSP1DPhi[c_nVXDLayers * index1 + index1]->Fill(fPosSPU1);
      if (m_correlationsSP1DTheta[c_nVXDLayers * index1 + index1] != NULL)
        if (!m_Reduce1DCorrelHistos)
          m_correlationsSP1DTheta[c_nVXDLayers * index1 + index1]->Fill(fPosSPV1);
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
          if ((iLayer2 < c_firstPXDLayer) || (iLayer2 > c_lastPXDLayer)) continue;
          index2 = iLayer2 - c_firstVXDLayer;
          float fCharge2 = digitPXD2.getCharge();
          if (fCharge2 < m_CutCorrelationSigPXD) continue;
          VxdID sensorID2 = digitPXD2.getSensorID();
          auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
          TVector3 rLocal2(digitPXD2.getUCellPosition(), digitPXD2.getVCellPosition(), 0);
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
          const SVDDigit& digitSVD2 = *storeSVDDigits[i2 - storePXDDigits.getEntries()];
          iLayer2 = digitSVD2.getSensorID().getLayerNumber();
          if ((iLayer2 < c_firstSVDLayer) || (iLayer2 > c_lastSVDLayer)) continue;
          index2 = iLayer2 - c_firstVXDLayer;
          float fCharge2 = digitSVD2.getCharge();
          fTime2 = digitSVD2.getTime();
          VxdID sensorID2 = digitSVD2.getSensorID();
          auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
          if (digitSVD2.isUStrip()) {
            if (fCharge2 < m_CutCorrelationSigUSVD) continue;
            TVector3 rLocal2(digitSVD2.getCellPosition(), 0 , 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsU2 = 1;
            fPosSPU2 = ral2.Phi() / TMath::Pi() * 180;
          } else {
            if (fCharge2 < m_CutCorrelationSigVSVD) continue;
            TVector3 rLocal2(0, digitSVD2.getCellPosition(), 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsV2 = 1;
            fPosSPV2 = ral2.Theta() / TMath::Pi() * 180;
          }
        }
      } else { // CLUSTERS:
        if (i2 < storePXDClusters.getEntries()) {  // PXD digits/clusters:
          const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
          iLayer2 = clusterPXD2.getSensorID().getLayerNumber();
          if ((iLayer2 < c_firstPXDLayer) || (iLayer2 > c_lastPXDLayer)) continue;
          index2 = iLayer2 - c_firstVXDLayer;
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
          if ((iLayer2 < c_firstSVDLayer) || (iLayer2 > c_lastSVDLayer)) continue;
          index2 = iLayer2 - c_firstVXDLayer;
          float fCharge2 = clusterSVD2.getCharge();
          fTime2 = clusterSVD2.getClsTime();
          VxdID sensorID2 = clusterSVD2.getSensorID();
          auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
          if (clusterSVD2.isUCluster()) {
            if (fCharge2 < m_CutCorrelationSigUSVD) continue;
            TVector3 rLocal2(clusterSVD2.getPosition(), 0 , 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsU2 = 1;
            fPosSPU2 = ral2.Phi() / TMath::Pi() * 180;
          } else {
            if (fCharge2 < m_CutCorrelationSigVSVD) continue;
            TVector3 rLocal2(0, clusterSVD2.getPosition(), 0);
            TVector3 ral2 = info.pointToGlobal(rLocal2);
            iIsV2 = 1;
            fPosSPV2 = ral2.Theta() / TMath::Pi() * 180;
          }
        }
      }
      if ((iIsPXD1 == 0) && (iIsPXD2 == 0))
        if ((fabs(fTime1 - fTime2)) > m_CutCorrelationTimeSVD) continue;
      if (!m_SaveOtherHistos)
        if (abs(index1 - index2) > 1) continue;  // only neighboars
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1) && (iIsPXD1 == 0) && (iIsPXD2 == 0)) {
        // hit maps for SVD:
        if (m_correlationsSP[c_nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP[c_nVXDLayers * index2 + index1]->Fill(fPosSPU1, fPosSPV2);
        if (m_correlationsSP1DPhi[c_nVXDLayers * index2 + index1] != NULL)
          if (!m_Reduce1DCorrelHistos)
            m_correlationsSP1DPhi[c_nVXDLayers * index2 + index1]->Fill(fPosSPU1);
        if (m_correlationsSP1DTheta[c_nVXDLayers * index2 + index1] != NULL)
          if (!m_Reduce1DCorrelHistos)
            m_correlationsSP1DTheta[c_nVXDLayers * index2 + index1]->Fill(fPosSPV2);
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        if (m_correlationsSP[c_nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP[c_nVXDLayers * index2 + index1]->Fill(fPosSPU1, fPosSPU2);
        if (m_correlationsSP1DPhi[c_nVXDLayers * index2 + index1] != NULL)
          if (!m_Reduce1DCorrelHistos)
            m_correlationsSP1DPhi[c_nVXDLayers * index2 + index1]->Fill(fPosSPU2 - fPosSPU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        if (m_correlationsSP[c_nVXDLayers * index2 + index1] != NULL)
          m_correlationsSP[c_nVXDLayers * index2 + index1]->Fill(fPosSPV2, fPosSPV1);
        if (m_correlationsSP1DTheta[c_nVXDLayers * index2 + index1] != NULL)
          if (!m_Reduce1DCorrelHistos)
            m_correlationsSP1DTheta[c_nVXDLayers * index2 + index1]->Fill(fPosSPV2 - fPosSPV1);
      }
    }
  }

}


void VXDDQMModule::endRun()
{
}


void VXDDQMModule::terminate()
{
}
