/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Description : DQM module, which fits many PXD histograms and writes    *
 *               out fit parameters in new histograms                     *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck, Peter Kodys                               *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "dqm/analysis/modules/DQMHistAnalysisPXDER.h"

#include <framework/core/HistoModule.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <boost/format.hpp>

#include <TClass.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TF1.h>

#include <memory>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDER)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDERModule::DQMHistAnalysisPXDERModule() : DQMHistAnalysisModule()
{
  //Set module properties
  setDescription("PXD DQM analysis module for Express Reco ");
  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("refHisto.root"));

  // NO parallel processing
}


DQMHistAnalysisPXDERModule::~DQMHistAnalysisPXDERModule()
{
}

void DQMHistAnalysisPXDERModule::initialize()
{
  m_refFile = NULL;
  if (m_refFileName != "") {
    m_refFile = new TFile(m_refFileName.data());
  }

  gROOT->cd(); // this seems to be important, or strange things happen
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

  c_nPXDSensors = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() <= c_lastPXDLayer) {  // PXD
        c_nPXDSensors += geo.getLadders(layer).size() * geo.getSensors(ladder).size();
      }
      break;
    }
  }

//   m_hitMapCounts = "DQMER_PXD_PixelHitmapCounts";
//   m_hitMapClCounts = "DQMER_PXD_ClusterHitmapCounts";

  for (int i = 0; i < c_nPXDSensors; i++) {
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    //----------------------------------------------------------------
    // Number of fired pixels per frame
    //----------------------------------------------------------------
    m_fired.emplace_back(str(format("DQMER_PXD_%1%_Fired") % sensorDescr));
    m_ref_fired.emplace_back("ref/" + m_fired.back());
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    m_clusters.emplace_back(str(format("DQMER_PXD_%1%_Clusters") % sensorDescr));
    m_ref_clusters.emplace_back("ref/" + m_clusters.back());
    //----------------------------------------------------------------
    // Start row distribution
    //----------------------------------------------------------------
    m_startRow.emplace_back(str(format("DQMER_PXD_%1%_StartRow") % sensorDescr));
    m_ref_startRow.emplace_back("ref/" + m_startRow.back());
    //----------------------------------------------------------------
    // Cluster seed charge by distance from the start row
    //----------------------------------------------------------------
    m_chargStartRow.emplace_back(str(format("DQMER_PXD_%1%_AverageSeedByStartRow") % sensorDescr));
    m_ref_chargStartRow.emplace_back("ref/" + m_chargStartRow.back());


    m_startRowCount.emplace_back(str(format("DQMER_PXD_%1%_SeedCountsByStartRow") % sensorDescr));
    m_ref_startRowCount.emplace_back("ref/" + m_startRowCount.back());
    //----------------------------------------------------------------
    // Cluster Charge
    //----------------------------------------------------------------
    m_clusterCharge.emplace_back(str(format("DQMER_PXD_%1%_ClusterCharge") % sensorDescr));
    m_ref_clusterCharge.emplace_back("ref/" + m_clusterCharge.back());
    //----------------------------------------------------------------
    // Pixel Signal
    //----------------------------------------------------------------
    m_pixelSignal.emplace_back(str(format("DQMER_PXD_%1%_PixelSignal") % sensorDescr));
    m_ref_pixelSignal.emplace_back("ref/" + m_pixelSignal.back());
    //----------------------------------------------------------------
    // Cluster Size in U
    //----------------------------------------------------------------
    m_clusterSizeU.emplace_back(str(format("DQMER_PXD_%1%_ClusterSizeU") % sensorDescr));
    m_ref_clusterSizeU.emplace_back("ref/" + m_clusterSizeU.back());
    //----------------------------------------------------------------
    // Cluster Size in V
    //----------------------------------------------------------------
    m_clusterSizeV.emplace_back(str(format("DQMER_PXD_%1%_ClusterSizeV") % sensorDescr));
    m_ref_clusterSizeV.emplace_back("ref/" + m_clusterSizeV.back());
    //----------------------------------------------------------------
    // Cluster Size in U+V
    //----------------------------------------------------------------
    m_clusterSizeUV.emplace_back(str(format("DQMER_PXD_%1%_ClusterSizeUV") % sensorDescr));
    m_ref_clusterSizeUV.emplace_back("ref/" + m_clusterSizeUV.back());
  }
//   m_fHitMapCountsFlag = NULL;
//   m_fHitMapClCountsFlag = NULL;
//   m_hitMapCounts = NULL;
//   m_hitMapClCounts = NULL;

// Create flag histograms:
//   DirPXDFlags->cd();
  m_fFiredFlag = new TH1I("DQMER_PXD_FiredFlag", "DQM ER PXD Fired Flag",
                          c_nPXDSensors, 0, c_nPXDSensors);
  m_fFiredFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fFiredFlag->GetYaxis()->SetTitle("flag");
  m_fClustersFlag = new TH1I("DQMER_PXD_ClustersFlag", "DQM ER PXD Clusters Flag",
                             c_nPXDSensors, 0, c_nPXDSensors);
  m_fClustersFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClustersFlag->GetYaxis()->SetTitle("flag");
  m_fStartRowFlag = new TH1I("DQMER_PXD_StartRowFlag", "DQM ER PXD Start Row Flag",
                             c_nPXDSensors, 0, c_nPXDSensors);
  m_fStartRowFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStartRowFlag->GetYaxis()->SetTitle("flag");
  m_fChargStartRowFlag = new TH1I("DQMER_PXD_ChargStartRowFlag", "DQM ER PXD Charg Start Row Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fChargStartRowFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fChargStartRowFlag->GetYaxis()->SetTitle("flag");
  m_fStartRowCountFlag = new TH1I("DQMER_PXD_StartRowCountFlag", "DQM ER PXD Row Count Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fStartRowCountFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStartRowCountFlag->GetYaxis()->SetTitle("flag");
//   m_fHitMapCountsFlag = new TH1I("DQMER_PXD_PixelHitmapCountsFlag", "DQM ER PXD Pixel Hitmaps Counts Flag",
//                                  c_nPXDSensors, 0, c_nPXDSensors);
//   m_fHitMapCountsFlag->GetXaxis()->SetTitle("Sensor ID");
//   m_fHitMapCountsFlag->GetYaxis()->SetTitle("flag");
//   m_fHitMapClCountsFlag = new TH1I("DQMER_PXD_ClusterHitmapCountsFlag", "DQM ER PXD Cluster Hitmaps Counts Flag",
//                                    c_nPXDSensors, 0, c_nPXDSensors);
//   m_fHitMapClCountsFlag->GetXaxis()->SetTitle("Sensor ID");
//   m_fHitMapClCountsFlag->GetYaxis()->SetTitle("flag");
  m_fClusterChargeFlag = new TH1I("DQMER_PXD_ClusterChargeFlag", "DQM ER PXD Cluster Charge Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterChargeFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterChargeFlag->GetYaxis()->SetTitle("flag");
  m_fPixelSignalFlag = new TH1I("DQMER_PXD_PixelSignalFlag", "DQM ER PXD Pixel Signal Flag",
                                c_nPXDSensors, 0, c_nPXDSensors);
  m_fPixelSignalFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fPixelSignalFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeUFlag = new TH1I("DQMER_PXD_ClasterSizeUFlag", "DQM ER PXD Cluster Size U Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeUFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeVFlag = new TH1I("DQMER_PXD_ClasterSizeVFlag", "DQM ER PXD Cluster Size V Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeVFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeUVFlag = new TH1I("DQMER_PXD_ClasterSizeUVFlag", "DQM ER PXD Cluster Size UV Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeUVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeUVFlag->GetYaxis()->SetTitle("flag");

  for (int i = 0; i < c_nPXDSensors; i++) {
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, iLayer, iLadder, iSensor);
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
//     m_hitMapCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
//     m_hitMapClCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fFiredFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClustersFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStartRowFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fChargStartRowFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStartRowCountFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
//     m_fHitMapCountsFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
//     m_fHitMapClCountsFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterChargeFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fPixelSignalFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeUVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }
//   m_oldDir->cd();
}

void DQMHistAnalysisPXDERModule::beginRun()
{
  // Just to make sure, reset all the histograms.
//   if (m_fHitMapCountsFlag != NULL) m_fHitMapCountsFlag->Reset();
//   if (m_fHitMapClCountsFlag != NULL) m_fHitMapClCountsFlag->Reset();
  if (m_fFiredFlag != NULL) m_fFiredFlag->Reset();
  if (m_fClustersFlag != NULL) m_fClustersFlag->Reset();
  if (m_fStartRowFlag != NULL) m_fStartRowFlag->Reset();
  if (m_fChargStartRowFlag != NULL) m_fChargStartRowFlag->Reset();
  if (m_fStartRowCountFlag != NULL) m_fStartRowCountFlag->Reset();
  if (m_fClusterChargeFlag != NULL) m_fClusterChargeFlag->Reset();
  if (m_fPixelSignalFlag != NULL) m_fPixelSignalFlag->Reset();
  if (m_fClusterSizeUFlag != NULL) m_fClusterSizeUFlag->Reset();
  if (m_fClusterSizeVFlag != NULL) m_fClusterSizeVFlag->Reset();
  if (m_fClusterSizeUVFlag != NULL) m_fClusterSizeUVFlag->Reset();

//   if (m_hitMapCounts != NULL) m_hitMapCounts->Reset();
//   if (m_hitMapClCounts != NULL) m_hitMapClCounts->Reset();

}


void DQMHistAnalysisPXDERModule::event()
{

  // Dont sum up!
//   if (m_fHitMapCountsFlag != NULL) m_fHitMapCountsFlag->Reset();
//   if (m_fHitMapClCountsFlag != NULL) m_fHitMapClCountsFlag->Reset();
  if (m_fFiredFlag != NULL) m_fFiredFlag->Reset();
  if (m_fClustersFlag != NULL) m_fClustersFlag->Reset();
  if (m_fStartRowFlag != NULL) m_fStartRowFlag->Reset();
  if (m_fChargStartRowFlag != NULL) m_fChargStartRowFlag->Reset();
  if (m_fStartRowCountFlag != NULL) m_fStartRowCountFlag->Reset();
  if (m_fClusterChargeFlag != NULL) m_fClusterChargeFlag->Reset();
  if (m_fPixelSignalFlag != NULL) m_fPixelSignalFlag->Reset();
  if (m_fClusterSizeUFlag != NULL) m_fClusterSizeUFlag->Reset();
  if (m_fClusterSizeVFlag != NULL) m_fClusterSizeVFlag->Reset();
  if (m_fClusterSizeUVFlag != NULL) m_fClusterSizeUVFlag->Reset();

  // Compare histograms with reference histograms and create flags:
  for (int i = 0; i < c_nPXDSensors; i++) {
    double pars[2];
    pars[0] = 0.01;// Probabilty value error?
    pars[1] = 0.05;// Probabilty value warning?

    double m_NoOfEvents = 1., m_NoOfEventsRef = 1.; // workaround

//     SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
//             m_hitMapCounts, r_hitMapCounts, m_fHitMapCountsFlag);
//     SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
//             m_hitMapClCounts, r_hitMapClCounts, m_fHitMapClCountsFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_fired.at(i), m_ref_fired.at(i), m_fFiredFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusters.at(i), m_ref_clusters.at(i), m_fClustersFlag);
    SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_startRow.at(i), m_ref_startRow.at(i), m_fStartRowFlag);
    SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_chargStartRow.at(i), m_ref_chargStartRow.at(i), m_fChargStartRowFlag);
    SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_startRowCount.at(i), m_ref_startRowCount.at(i), m_fStartRowCountFlag);
    SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterCharge.at(i), m_ref_clusterCharge.at(i), m_fClusterChargeFlag);
    SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_pixelSignal.at(i), m_ref_pixelSignal.at(i), m_fPixelSignalFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterSizeU.at(i), m_ref_clusterSizeU.at(i), m_fClusterSizeUFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterSizeV.at(i), m_ref_clusterSizeV.at(i), m_fClusterSizeVFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterSizeUV.at(i), m_ref_clusterSizeUV.at(i), m_fClusterSizeUVFlag);

  }
}

void DQMHistAnalysisPXDERModule::endRun()
{
}

void DQMHistAnalysisPXDERModule::terminate()
{
}

int DQMHistAnalysisPXDERModule::getSensorIndex(const int Layer, const int Ladder, const int Sensor) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > c_lastPXDLayer) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          return tempcounter;
        }
        tempcounter++;
      }
    }
  }
  return tempcounter;
}


void DQMHistAnalysisPXDERModule::getIDsFromIndex(const int Index, int& Layer, int& Ladder, int& Sensor) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > c_lastPXDLayer) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          Layer = layer.getLayerNumber();
          Ladder = ladder.getLadderNumber();
          Sensor = sensor.getSensorNumber();
          return;
        }
        tempcounter++;
      }
    }
  }
}

int DQMHistAnalysisPXDERModule::SetFlag(int Type, int bin, double* pars, double ratio, std::string name_hist,
                                        std::string name_refhist, TH1I* flaghist)
{
  int iret = 0;
  float WarningLevel = 6.0;
  float ErrorLevel = 10.0;

  TH1* hist, *refhist;

  hist = GetHisto(name_hist);
  if (!hist) return -1;
  refhist = GetHisto(name_refhist);
  if (!refhist) return -1;

  // What happens if they are TH1I, TH1D and not TH1F

  auto temp = std::unique_ptr<TH1F>(new TH1F("temp", "temp", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(),
                                             hist->GetXaxis()->GetXmax()));
  double NEvents = 0;
  double flagInt = 0;
  double flagrInt = 0;
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    double val = hist->GetBinContent(j + 1);
    NEvents += val;
    val = val / ratio;
    temp->SetBinContent(j + 1, val);
    flagInt += temp->GetBinContent(j + 1);
    flagrInt += refhist->GetBinContent(j + 1);
  }
  if (NEvents < 100) {  // not enough information for comparition
    iret = -1;
    flaghist->SetBinContent(bin + 1, -1);
    return iret;
  }
  double flag  = temp->GetMean();
  double flagErr  = temp->GetMeanError();
  double flagRMS  = temp->GetRMS();
  double flagRMSErr  = temp->GetRMSError();
  double flagr  = refhist->GetMean();
  double flagrErr  = refhist->GetMeanError();
  double flagrRMS  = refhist->GetRMS();
  double flagrRMSErr  = refhist->GetRMSError();
  TString strDebugInfo = Form("Conditions for Flag--->\n   source %f %f+-%f %f+-%f\n  referen %f %f+-%f %f+-%f\n",
                              flagInt, flag, flagErr, flagRMS, flagRMSErr,
                              flagrInt, flagr, flagrErr, flagrRMS, flagrRMSErr
                             );
  B2DEBUG(130, strDebugInfo.Data());
  if (Type == 1) {  // counts+mean+RMS use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 2) { // counts use
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 3) { // mean use
    if (fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 4) { // RMS use
    if (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 5) { // counts+mean use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 9) { // bin content use
    flagInt = temp->GetBinContent(bin + 1);
    flagrInt = refhist->GetBinContent(bin + 1);
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 10) {
    float flag2  = refhist->Chi2Test(temp);
    flaghist->SetBinContent(bin + 1, 0);
    if (flag2 > pars[1])
      flaghist->SetBinContent(bin + 1, 2);
    if (flag2 > pars[0])
      flaghist->SetBinContent(bin + 1, 1);
    iret = 1;
  } else if (Type == 100) {
    flaghist->SetBinContent(bin + 1, 0);
    iret = 1;
  } else {
    flaghist->SetBinContent(bin + 1, -3);
    iret = -1;
  }
  strDebugInfo = Form("SetFlag---> %f, type %i\n", flaghist->GetBinContent(bin + 1), Type);
  B2DEBUG(130, strDebugInfo.Data());
  return iret;
}

// int DQMHistAnalysisPXDERModule::SetFlag(int Type, int bin, double* pars, double ratio, std::string name_hist, std::string name_refhist, TH1I* flaghist)
// {
//
//   TH1F* histF = new TH1F("histF", "histF", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
//   TH1F* refhistF = new TH1F("refhistF", "refhistF", refhist->GetNbinsX(), refhist->GetXaxis()->GetXmin(),
//                             refhist->GetXaxis()->GetXmax());
//   for (int j = 0; j < hist->GetNbinsX(); j++) {
//     histF->SetBinContent(j + 1, hist->GetBinContent(j + 1));
//     refhistF->SetBinContent(j + 1, refhist->GetBinContent(j + 1));
//   }
//   int ret = SetFlag(Type, bin, pars, ratio, histF, refhistF, flaghist);
//   delete histF;
//   delete refhistF;
//   return ret;
// }

TH1* DQMHistAnalysisPXDERModule::GetHisto(TString histoname)
{
  TH1* hh1;
  hh1 = findHist(histoname.Data());
  if (hh1 == NULL) {
    B2INFO("Histo " << histoname << " not in memfile");
    // the following code sux ... is there no root function for that?


    // first search reference root file ... if ther is one
    if (m_refFile && m_refFile->IsOpen()) {
      TDirectory* d = m_refFile;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      B2INFO(myl);
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2INFO("Cd Dir " << tok);
            d = e;
          } else {
            B2INFO("cd failed " << tok);
          }
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2INFO("Histo " << histoname << " found in ref file");
          hh1 = (TH1*)obj;
        } else {
          B2INFO("Histo " << histoname << " found in ref file but wrong type");
        }
      } else {
        // seems find will only find objects, not keys, thus get the object on first access
        TIter next(d->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)next())) {
          TObject* obj2 = key->ReadObj() ;
          if (obj2->InheritsFrom("TH1")) {
            if (obj2->GetName() == tok) {
              hh1 = (TH1*)obj2;
              B2INFO("Histo " << histoname << " found as key -> readobj");
              break;
            }
          }
        }
        if (hh1 == NULL) B2INFO("Histo " << histoname << " NOT found in ref file " << tok);
      }
    }

    if (hh1 == NULL) {
      B2INFO("Histo " << histoname << " not in memfile or ref file");
      // the following code sux ... is there no root function for that?

      TDirectory* d = gROOT;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2INFO("Cd Dir " << tok);
            d = e;
          } else B2INFO("cd failed " << tok);
          d->cd();
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2INFO("Histo " << histoname << " found in mem");
          hh1 = (TH1*)obj;
        }
      } else {
        B2INFO("Histo " << histoname << " NOT found in mem");
      }
    }
  }

  if (hh1 == NULL) {
    B2INFO("Histo " << histoname << " not found");
  }

  return hh1;
}
