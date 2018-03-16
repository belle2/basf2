#include "testbeam/vxd/modules/QualityCheckModule.h"
#include <framework/datastore/RelationIndex.h>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(QualityCheck)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

QualityCheckModule::QualityCheckModule() : HistoModule()
{
  //Set module properties
  setDescription("QualityCheck of pixels and strips in PXD, VXD and Tels");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("QualityCheck"));
  m_nEventsProcess = -1;
  addParam("nEventsProcess", m_nEventsProcess, "Number of events to process", m_nEventsProcess);
  m_PXDCutSeedL = 0;
  m_PXDCutSeedH = 100000;
  addParam("PXDCutSeedL", m_PXDCutSeedL, "PXD: seed cut lower border", m_PXDCutSeedL);
  addParam("PXDCutSeedH", m_PXDCutSeedH, "PXD: seed cut higher border", m_PXDCutSeedH);
  addParam("CutSVDCharge", m_CutSVDCharge, "Cut to show on plot signal over this border", m_CutSVDCharge);

  addParam("TelRunNo", m_TelRunNo, "Telescope Run No", m_TelRunNo);
  addParam("RunNo", m_RunNo, "Run No",  std::string("-1"));


  addParam("SummaryFileRunName", m_SummaryFileRunName, "Name of summary file",
           std::string("QualityCheckSummary_RunXXX.txt"));

  m_StartTime = 1461929600592085;
  addParam("StartTime", m_sStartTime, "TLU timestamp od start from FTSW", std::string("1461929600592085"));
  m_EndTime = 1461929609592099;
  addParam("EndTime", m_sEndTime, "TLU timestamp od end from FTSW", std::string("1461929609592099"));

}


QualityCheckModule::~QualityCheckModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void QualityCheckModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  //printf("h---------------> Time: %lu %lu %s\n", m_StartTime, m_EndTime, m_SummaryFileRunName.c_str());
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();


  float events = m_EndEvent - m_StartEvent;
  int nEvPacks = int(events / m_EventUnit);
  float Time = (float)(m_EndTime - m_StartTime) / 1000000.0;  // in seconds
  int nTimePacks = int(Time / m_TimeUnit);
  //printf("---> Event: %i %f %f, Time: %i %f %f\n", nEvPacks, events, EventUnit, nTimePacks, Time, TimeUnit);
  float CorrRange = 10.0; // in cm
  int iCorrRange = CorrRange * 20;  // 1 bin = 0.5 mm

  m_TriggerRate = NULL;
  m_TriggerRateTime = NULL;
  m_TriggerRateTime2 = NULL;
  m_CorrelationPXDSVD_Y = NULL;
  m_CorrelationPXDSVD_Z = NULL;
  string name = str(format("TriggerRate"));
  string title = str(format("TB2016 trigger rate on events"));
  m_TriggerRate = new TH1F(name.c_str(), title.c_str(), nEvPacks, 0, events);
  m_TriggerRate->GetXaxis()->SetTitle("# event");
  m_TriggerRate->GetYaxis()->SetTitle("trigger rate");
  name = str(format("TriggerRateTime"));
  title = str(format("TB2016 trigger rate on time"));
  m_TriggerRateTime = new TH1F(name.c_str(), title.c_str(), nTimePacks, 0, Time);
  m_TriggerRateTime->GetXaxis()->SetTitle("time [sec]");
  m_TriggerRateTime->GetYaxis()->SetTitle("trigger rate");
  name = str(format("TriggerRateTimeCorrels"));
  title = str(format("TB2016 trigger rate on time for correlations"));
  m_TriggerRateTime2 = new TH1F(name.c_str(), title.c_str(), nTimePacks, 0, Time);
  m_TriggerRateTime2->GetXaxis()->SetTitle("time [sec]");
  m_TriggerRateTime2->GetYaxis()->SetTitle("trigger rate");
  name = str(format("CoorelationsPXDSVD_Y"));
  title = str(format("TB2016 correlations between PXD and SVD for axis Y"));
  m_CorrelationPXDSVD_Y = new TH1F(name.c_str(), title.c_str(), iCorrRange, -CorrRange / 2.0, CorrRange / 2.0);
  m_CorrelationPXDSVD_Y->GetXaxis()->SetTitle("differnece [cm]");
  m_CorrelationPXDSVD_Y->GetYaxis()->SetTitle("counts");
  name = str(format("CoorelationsPXDSVD_Z"));
  title = str(format("TB2016 correlations between PXD and SVD for axis Z"));
  m_CorrelationPXDSVD_Z = new TH1F(name.c_str(), title.c_str(), iCorrRange, -CorrRange / 2.0, CorrRange / 2.0);
  m_CorrelationPXDSVD_Z->GetXaxis()->SetTitle("differnece [cm]");
  m_CorrelationPXDSVD_Z->GetYaxis()->SetTitle("counts");
  for (int i = 0; i < c_nTBPlanes; i++) {
    m_PlaneOccupancy[i] = NULL;
    m_PlaneOccupancyTime[i] = NULL;
    int iPlane = 0;
    string name;
    string title;
    if (i < c_nVXDPlanes) {
      iPlane = indexToPlaneVXD(i);
      name = str(format("VXD_L%1%_Occupancy") % iPlane);
      title = str(format("TB2016 VXD layer %1%, occupancy") % iPlane);
    } else {
      iPlane = indexToPlaneTel(i - c_nVXDPlanes);
      name = str(format("Tel_L%1%_Occupancy") % iPlane);
      title = str(format("TB2016 Tel layer %1%, occupancy") % iPlane);
    }
    m_PlaneOccupancy[i] = new TH1F(name.c_str(), title.c_str(), nEvPacks, 0, events);
    m_PlaneOccupancy[i]->GetXaxis()->SetTitle("# event");
    m_PlaneOccupancy[i]->GetYaxis()->SetTitle("hits");

    if (i < c_nVXDPlanes) {
      iPlane = indexToPlaneVXD(i);
      name = str(format("VXD_L%1%_OccupancyTime") % iPlane);
      title = str(format("TB2016 VXD layer %1%, occupancy in time") % iPlane);
    } else {
      iPlane = indexToPlaneTel(i - c_nVXDPlanes);
      name = str(format("Tel_L%1%_OccupancyTime") % iPlane);
      title = str(format("TB2016 Tel layer %1%, occupancy in time") % iPlane);
    }
    m_PlaneOccupancyTime[i] = new TH1F(name.c_str(), title.c_str(), nTimePacks, 0, Time);
    m_PlaneOccupancyTime[i]->GetXaxis()->SetTitle("time [sec]");
    m_PlaneOccupancyTime[i]->GetYaxis()->SetTitle("hits");

  }

  // cd back to root directory
  oldDir->cd();
}


void QualityCheckModule::initialize()
{
  //Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
//  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
//  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<TelCluster> storeTelClusters(m_storeTelClustersName);
//  StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);


//  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
//  RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits);
//  RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits);

  // FTSW is necessary, without it just fail.
  StoreArray<RawFTSW> storeFTSW("");
  storeFTSW.isRequired();
  m_storeRawFTSWsName = storeFTSW.getName();
  m_TLUStartTagFromFTSW = static_cast<unsigned short>(storeFTSW[0]->Get15bitTLUTag(0));
  m_TLUStartTagFromFTSWCor = m_TLUStartTagFromFTSW;

  // EventMetaData
  StoreObjPtr<EventMetaData> storeEventMetaData;
//  storeEventMetaData.isRequired();

  // EventMetaData
  //m_storeFileMetaDataName = storeFileMetaData.getName();
  const StoreObjPtr<FileMetaData> storeFileMetaData(m_storeFileMetaDataName, DataStore::c_Persistent);
  int Experiment = storeEventMetaData->getExperiment();
  int RunNo = storeEventMetaData->getRun();
//  storeFileMetaData.isRequired();
  m_ProcessEvents = storeFileMetaData->getNEvents();
  int RunLow = storeFileMetaData->getRunLow();
  int RunHigh = storeFileMetaData->getRunHigh();
  unsigned int EventLow = storeFileMetaData->getEventLow();
  unsigned int EventHigh = storeFileMetaData->getEventHigh();
  string Date = storeFileMetaData->getDate();
  printf("--------> Exp%i, RunNo%i, Events%lu RunLow%i RunHigh%i TLUTag: %i Events: %u - %u = %u\n", Experiment, RunNo,
         m_ProcessEvents, RunLow, RunHigh, m_TLUStartTagFromFTSW, EventHigh, EventLow, EventHigh - EventLow);
  m_StartEvent = EventLow;
  m_EndEvent = EventHigh;
  if (m_StartEvent > m_EndEvent) m_StartEvent = 0;
  m_StartTime = std::stoul(m_sStartTime.c_str());
  m_EndTime = std::stoul(m_sEndTime.c_str());
  printf("--------> Time: from %lu to %lu\n", m_StartTime, m_EndTime);

  //timeval* time = new timeval;
  //storeFTSW[0]->GetTTTimeVal(0, time);
  //m_currentTimeStampFromFTSW = time->tv_sec * 1000000 + time->tv_usec;
  //int iii = InputController::getCurrentEntry();
//  m_StartTime = m_currentTimeStampFromFTSW;
//  m_EndTime = m_StartTime + 300.5 * 1000000.0;
//  InputController::setNextEntry(Experiment, RunNo, EventHigh);
//  InputController::setNextEntry(Experiment, RunNo, EventLow + 14000 - 1090);   // jump to some event
//  iii = InputController::getCurrentEntry();
//  printf("Jsem v event(%i)\n", iii);

  // Set maximum of procesed events
  if (m_nEventsProcess < 0) m_nEventsProcess = std::numeric_limits<long>::max();
  m_nEventsProcessFraction = 1.0;
  m_nRealEventsProcess = 0;
  for (int i = 0; i < c_nTBPlanes; i++) m_nEventsPlane[i] = 0;

  //Store names to speed up creation later
  m_storePXDClustersName = storePXDClusters.getName();
//  m_storePXDDigitsName = storePXDDigits.getName();
//  m_relPXDClusterDigitName = relPXDClusterDigits.getName();

  m_storeSVDClustersName = storeSVDClusters.getName();
//  m_storeSVDDigitsName = storeSVDDigits.getName();
//  m_relSVDClusterDigitName = relSVDClusterDigits.getName();

  m_storeTelClustersName = storeTelClusters.getName();
//  m_storeTelDigitsName = storeTelDigits.getName();
//  m_relTelClusterDigitName = relTelClusterDigits.getName();



  //StoreObjPtr<TelEventInfo> storeTelEventInfo;

//  defineHisto();
}

void QualityCheckModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  printf("Jsem v beginRun\n");

  m_TriggerRate->Reset();
  m_TriggerRateTime->Reset();
  m_TriggerRateTime2->Reset();
  m_CorrelationPXDSVD_Y->Reset();
  m_CorrelationPXDSVD_Z->Reset();
  for (int i = 0; i < c_nTBPlanes; i++) {
    m_PlaneOccupancy[i]->Reset();
    m_PlaneOccupancyTime[i]->Reset();
  }

  m_nVXDDataEvents = 0;
  m_nTelDataEvents = 0;
  m_nMapHits = 0;
  m_nBOREvents = 0;
  m_nEOREvents = 0;
  m_nNoTrigEvents = 0;
}


void QualityCheckModule::event()
{
//  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
//  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
//  const StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const StoreArray<TelCluster> storeTelClusters(m_storeTelClustersName);

//  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
//  const RelationArray relClusterDigits(storeSVDClusters, storeDigits, m_relClusterDigitName);
//  const RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits, m_relTelClusterDigitName);

//  StoreObjPtr<EventMetaData> emd;
//  unsigned int m_evt = emd->getEvent();
//  insignet int m_evt = emd->g
//  emd.getEvent().

  // Tel basic histograms:
  // Fired strips
  vector< set<int> > uStripsSenpt(c_nTelPlanes); // sets to eliminate multiple samples per strip

  //StoreObjPtr<TelEventInfo> storeTelEventInfo;
  StoreArray<RawFTSW> storeFTSW(m_storeRawFTSWsName);

  //storeTelEventInfo.assign(new TelEventInfo(m_currentTLUTagFromFTSW, m_currentTLUTagFromFTSW, m_currentTimeStampFromFTSW));
  m_currentTLUTagFromFTSW = static_cast<unsigned short>(storeFTSW[0]->Get15bitTLUTag(0));
  StoreObjPtr<EventMetaData> storeEventMetaData;
  //int EventNo2 = static_cast<unsigned short>(storeFTSW[0]->GetNumEvents());

  m_nEventsProcess++;
  m_nNoTrigEvents++;

  timeval* time = new timeval;
  storeFTSW[0]->GetTTTimeVal(0, time);
  m_currentTimeStampFromFTSW = time->tv_sec * 1000000 + time->tv_usec;
  int FromStartTLUTagFromFTSW = (int)((long)m_currentTLUTagFromFTSW - m_TLUStartTagFromFTSWCor);
  if (m_nVXDDataEvents > 50) {  // first 50 events could be negative from counting from history before current file storing
    if ((FromStartTLUTagFromFTSW < 0) && (FromStartTLUTagFromFTSW > (32768 / 2))) m_TLUStartTagFromFTSWCor += 32768;
    while (FromStartTLUTagFromFTSW < 0) {
      FromStartTLUTagFromFTSW += 32768;  // correction for TLU counter reset after every 32 768 (2^15)
    }
  }
  float Time = (float)((long)(m_currentTimeStampFromFTSW - m_StartTime));
  printf("---> Event %li TLUEvent %i LocalTLUEvent %i Timestamp[us] %li LocalTime[ms] %5.3f \n",
         m_nVXDDataEvents, (int)m_currentTLUTagFromFTSW, FromStartTLUTagFromFTSW, m_currentTimeStampFromFTSW, Time / 1000.0);


  int IsGoodEventY = 0;
  int IsGoodEventZ = 0;
  int PlaneOccupancy[c_nTBPlanes];
  for (int i = 0; i < c_nTBPlanes; i++) {
    PlaneOccupancy[i] = 0;
  }
  m_nVXDDataEvents++;
  B2DEBUG(25, "Finished VXD Event();");

  float CutDQMCorrelTime = 70;  // ns
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storePXDClusters.getEntries() + storeTelClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fTime1 = 0.0;
    float fPosSPU1 = 0.0;
    float fPosSPV1 = 0.0;
    int iIsPXD1 = 0;
    int iIsTel1 = 0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storePXDClusters.getEntries()) {  // PXD clusters:
      const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 < c_firstPXDPlane) || (iPlane1 > c_lastPXDPlane)) continue;
      index1 = planeToIndexVXD(iPlane1);
      //float fCharge1 = clusterPXD1.getCharge();
      //if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
      PlaneOccupancy[index1]++;
      VxdID sensorID1 = clusterPXD1.getSensorID();
      auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
      TVector3 rLocal1(clusterPXD1.getU(), clusterPXD1.getV(), 0);
      TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
      iIsPXD1 = 1;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosSPU1 = rGlobal1.Y();
      fPosSPV1 = rGlobal1.Z();
    } else if ((i1 >= storePXDClusters.getEntries())
               && (i1 < (storePXDClusters.getEntries() + storeSVDClusters.getEntries()))) {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storePXDClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 < c_firstSVDPlane) || (iPlane1 > c_lastSVDPlane)) continue;
      index1 = planeToIndexVXD(iPlane1);
      PlaneOccupancy[index1]++;
      //float fCharge1 = cluster1.getCharge();
      fTime1 = cluster1.getClsTime();
      VxdID sensorID1 = cluster1.getSensorID();
      auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
      if (cluster1.isUCluster()) {
        //if (fCharge1 < CutDQMCorrelSigU[index1]) continue;
        TVector3 rLocal1(cluster1.getPosition(), 0 , 0);
        TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
        iIsU1 = 1;
        fPosSPU1 = rGlobal1.Y();
      } else {
        //if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
        TVector3 rLocal1(0, cluster1.getPosition(), 0);
        TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
        iIsV1 = 1;
        fPosSPV1 = rGlobal1.Z();
      }
    } else {                                  // Tel clusters:
      const TelCluster& clusterTel1 = *storeTelClusters[ i1 - (storePXDClusters.getEntries() + storeSVDClusters.getEntries())];
      iPlane1 = clusterTel1.getSensorID().getSensorNumber();
      if ((iPlane1 < c_firstTelPlane) || (iPlane1 > c_lastTelPlane)) continue;
      index1 = c_nVXDPlanes + iPlane1 - c_firstTelPlane;
      PlaneOccupancy[index1]++;
      VxdID sensorID1 = clusterTel1.getSensorID();
      auto info = dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID1));
      TVector3 rLocal1(clusterTel1.getU(), clusterTel1.getV(), 0);
      TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
      iIsTel1 = 1;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosSPU1 = rGlobal1.Y();
      fPosSPV1 = rGlobal1.Z();
    }
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storePXDClusters.getEntries() + storeTelClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fTime2 = 0.0;
      float fPosSPU2 = 0.0;
      float fPosSPV2 = 0.0;
      int iIsPXD2 = 0;
      int iIsTel2 = 0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storePXDClusters.getEntries()) {  // PXD clusters:
        const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        index2 = planeToIndexVXD(iPlane2);
        //float fCharge2 = clusterPXD2.getCharge();
        //if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
        VxdID sensorID2 = clusterPXD2.getSensorID();
        auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
        TVector3 rLocal2(clusterPXD2.getU(), clusterPXD2.getV(), 0);
        TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
        iIsPXD2 = 1;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosSPU2 = rGlobal2.Y();
        fPosSPV2 = rGlobal2.Z();
      } else if ((i2 >= storePXDClusters.getEntries())
                 && (i2 < (storePXDClusters.getEntries() + storeSVDClusters.getEntries()))) {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storePXDClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstSVDPlane) || (iPlane2 > c_lastSVDPlane)) continue;
        index2 = planeToIndexVXD(iPlane2);
        //float fCharge2 = cluster2.getCharge();
        fTime2 = cluster2.getClsTime();
        VxdID sensorID2 = cluster2.getSensorID();
        auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
        if (cluster2.isUCluster()) {
          //if (fCharge2 < CutDQMCorrelSigU[index2]) continue;
          TVector3 rLocal2(cluster2.getPosition(), 0 , 0);
          TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
          iIsU2 = 1;
          fPosSPU2 = rGlobal2.Y();
        } else {
          //if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
          TVector3 rLocal2(0, cluster2.getPosition(), 0);
          TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
          iIsV2 = 1;
          fPosSPV2 = rGlobal2.Z();
        }
      } else {                                  // Tel clusters:
        const TelCluster& clusterTel2 = *storeTelClusters[ i2 - (storePXDClusters.getEntries() + storeSVDClusters.getEntries())];
        iPlane2 = clusterTel2.getSensorID().getSensorNumber();
        if ((iPlane2 < c_firstTelPlane) || (iPlane2 > c_lastTelPlane)) continue;
        index2 = c_nVXDPlanes + iPlane2 - c_firstTelPlane;
        VxdID sensorID2 = clusterTel2.getSensorID();
        auto info = dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID2));
        TVector3 rLocal2(clusterTel2.getU(), clusterTel2.getV(), 0);
        TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
        iIsTel2 = 1;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosSPU2 = rGlobal2.Y();
        fPosSPV2 = rGlobal2.Z();
      }
      if ((iIsPXD1 == 0) && (iIsPXD2 == 0) && (iIsTel1 == 0) && (iIsTel2 == 0)) // only for SVD - filter:
        if ((fabs(fTime1 - fTime2)) > CutDQMCorrelTime) continue;
      if ((index1 == 1) && (index2 == 2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_CorrelationPXDSVD_Y->Fill(fPosSPU1 - fPosSPU2);
        if (fabs(fPosSPU1 - fPosSPU2) < 0.5) IsGoodEventY = 1;
      } else if ((index1 == 2) && (index2 == 1) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_CorrelationPXDSVD_Z->Fill(fPosSPV2 - fPosSPV1);
        if (fabs(fPosSPV2 - fPosSPV1) < 0.5) IsGoodEventZ = 1;
      }
    }
  }
  if (IsGoodEventY && IsGoodEventZ) {
    m_nRealEventsProcess++;
    m_TriggerRate->Fill(m_nVXDDataEvents);
    m_TriggerRateTime2->Fill(Time / 1000000.0);
    for (int i = 0; i < c_nTBPlanes; i++) {
      //m_PlaneOccupancyTime[i]->Fill(Time / 1000000.0, PlaneOccupancy[i]);
      m_PlaneOccupancy[i]->Fill(m_nVXDDataEvents, PlaneOccupancy[i]);
    }
  }
  m_TriggerRateTime->Fill(Time / 1000000.0);
  for (int i = 0; i < c_nTBPlanes; i++) {
    m_PlaneOccupancyTime[i]->Fill(Time / 1000000.0, PlaneOccupancy[i]);
    //m_PlaneOccupancy[i]->Fill(m_nVXDDataEvents, PlaneOccupancy[i]);
  }

}

void QualityCheckModule::endRun()
{
  StoreObjPtr<EventMetaData> storeEventMetaData;
  //long unsigned runNo = storeEventMetaData->getRun();
  //m_nEventsProcessFraction = (float)m_nRealEventsProcess / m_nNoTrigEvents;
  m_nEventsProcessFraction = m_CorrelationPXDSVD_Z->GetKurtosis(); // direction not affected by magnetic field
  //double m_nEventsProcessFractionY = m_CorrelationPXDSVD_Y->GetKurtosis();

  B2INFO("Start to create summary list");

  float TrgAvrg = 0.0;
  float TrgTimeAvrg = 0.0;
  int IsPlane[c_nTBPlanes];
  int IsPlOK[c_nTBPlanes];
  int IsPlTOK[c_nTBPlanes];
  float PlOccup[c_nTBPlanes];
  for (int i = 0; i < c_nTBPlanes; i++) {
    IsPlane[i] = 1;
    IsPlOK[i] = 1;
    IsPlTOK[i] = 1;
    PlOccup[i] = 0.0;
  }

  double avrg = 0;
  for (int ib = 1; ib < m_TriggerRate->GetNbinsX() - 1; ib++) {
    avrg += m_TriggerRate->GetBinContent(ib + 1);
  }
  avrg /= (m_TriggerRate->GetNbinsX() - 2);
  TrgAvrg = avrg;
  TrgAvrg = TrgAvrg / m_EventUnit * 1000.0;  // per 1000 events

  avrg = 0;
  for (int ib = 0; ib < m_TriggerRateTime->GetNbinsX(); ib++) {
    avrg += m_TriggerRateTime->GetBinContent(ib + 1);
  }
  avrg /= m_TriggerRateTime->GetNbinsX();
  TrgTimeAvrg = avrg;
  TrgTimeAvrg = TrgTimeAvrg / m_TimeUnit;



  for (int i = 0; i < c_nTBPlanes; i++) {
    m_PlaneOccupancy[i]->Divide(m_TriggerRate);
    m_PlaneOccupancyTime[i]->Divide(m_TriggerRateTime);
  }
  for (int i = 0; i < c_nTBPlanes; i++) {
    double avrg = 0;
    double Errr = 0;
    for (int ib = 0; ib < m_PlaneOccupancy[i]->GetNbinsX(); ib++) {
      avrg += m_PlaneOccupancy[i]->GetBinContent(ib + 1);
      Errr += m_PlaneOccupancy[i]->GetBinError(ib + 1);
    }
    if (avrg == 0) {
      IsPlane[i] = 0;
      IsPlOK[i] = 0;
      IsPlTOK[i] = 0;
      PlOccup[i] = 0.0;
    } else {
      avrg /= m_PlaneOccupancy[i]->GetNbinsX();
      PlOccup[i] = avrg;
      Errr /= m_PlaneOccupancy[i]->GetNbinsX();
      for (int ib = 1; ib < m_PlaneOccupancy[i]->GetNbinsX() - 1; ib++) {
        if (fabs(avrg - m_PlaneOccupancy[i]->GetBinContent(ib + 1)) > (3.0 * Errr)) IsPlOK[i] = 0;
      }
    }

    avrg = 0;
    Errr = 0;
    for (int ib = 0; ib < m_PlaneOccupancyTime[i]->GetNbinsX(); ib++) {
      avrg += m_PlaneOccupancyTime[i]->GetBinContent(ib + 1);
      Errr += m_PlaneOccupancyTime[i]->GetBinError(ib + 1);
    }
    if (avrg != 0) {
      avrg /= m_PlaneOccupancyTime[i]->GetNbinsX();
      Errr /= m_PlaneOccupancyTime[i]->GetNbinsX();
      for (int ib = 1; ib < m_PlaneOccupancyTime[i]->GetNbinsX() - 1; ib++) {
        if (fabs(avrg - m_PlaneOccupancyTime[i]->GetBinContent(ib + 1)) > (3.0 * Errr)) IsPlTOK[i] = 0;
      }
    }

  }

  //printf("-------->m_TelRunNo %i\n", m_TelRunNo);
  //if (m_TelRunNo != -1) {
  //}

  std::string FileName = str(format("%1%") % m_SummaryFileRunName);
  FILE* SummaryFile;
  if ((SummaryFile = fopen(FileName.data(), "r")) == NULL) {
    //fclose(SummaryFile);
    // create header of file:
    if ((SummaryFile = fopen(FileName.data(), "w")) == NULL) {
    }
    fprintf(SummaryFile, "  RunNo Events   EvFrom     EvTo T[sec]            DateTime         TimeFrom    AvEv    AvTi PXDSVD Tel ");
    for (int i = 0; i < c_nTBPlanes; i++) {
      if (i < c_nVXDPlanes) {
        fprintf(SummaryFile, "V%i OKE OKT  Occ ", i + 1);
      } else {
        fprintf(SummaryFile, "T%i OKE OKT  Occ ", i - c_nVXDPlanes + 1);
      }
    }
    fprintf(SummaryFile, "\n");
    fclose(SummaryFile);
  }
  if ((SummaryFile = fopen(FileName.data(), "a")) == NULL) {
    printf("ERROR: Unable to open sumary file %s for append\n", FileName.data());
    //fclose(SummaryFile);
  }
  time_t rawtime = (int)(m_StartTime / 1000000);
  char tcc[200];
  strftime(tcc, 200, "%Y/%m/%d,%H:%M:%S", localtime(&rawtime));
  fprintf(SummaryFile, "%7s %6i %8i %8i %6i %s %lu %7.2f %7.2f %6.2f %3i  ",
          m_RunNo.c_str(), (int)(m_EndEvent - m_StartEvent), (int)m_StartEvent, (int)m_EndEvent, (int)((m_EndTime - m_StartTime) / 1000000),
          tcc, m_StartTime, TrgAvrg,  TrgTimeAvrg, m_nEventsProcessFraction, m_TelRunNo
         );
  for (int i = 0; i < c_nTBPlanes; i++) {
    fprintf(SummaryFile, "%i   %i   %i %4.1f  ", IsPlane[i], IsPlOK[i], IsPlTOK[i], PlOccup[i]);
  }
  fprintf(SummaryFile, "\n");
  fclose(SummaryFile);

}


void QualityCheckModule::terminate()
{
}

