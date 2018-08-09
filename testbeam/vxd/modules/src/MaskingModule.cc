#include "testbeam/vxd/modules/MaskingModule.h"
#include <framework/datastore/RelationIndex.h>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Masking)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MaskingModule::MaskingModule() : HistoModule()
{
  //Set module properties
  setDescription("Masking of pixels and strips in PXD, VXD and Tels");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("mask"));
  m_nEventsProcess = -1;
  addParam("nEventsProcess", m_nEventsProcess, "Number of events to process", m_nEventsProcess);
  m_AppendMaskFile = 1;
  addParam("AppendMaskFile", m_AppendMaskFile, "Set option for append of existing file or recreate new list", m_AppendMaskFile);
  m_PXDCutSeedL = 0;
  m_PXDCutSeedH = 100000;
  addParam("PXDCutSeedL", m_PXDCutSeedL, "PXD: seed cut lower border", m_PXDCutSeedL);
  addParam("PXDCutSeedH", m_PXDCutSeedH, "PXD: seed cut higher border", m_PXDCutSeedH);
  addParam("CutSVDCharge", m_CutSVDCharge, "Cut to show on plot signal over this border", m_CutSVDCharge);

  addParam("PXDCut", m_PXDCut, "Cut for masking of PXD pixel - prreset for 10 kEvents", m_PXDCut);
  addParam("SVDuCut", m_SVDuCut, "Cut for masking of SVD U strip - prreset for 10 kEvents", m_SVDuCut);
  addParam("SVDvCut", m_SVDvCut, "Cut for masking of SVD V strip - prreset for 10 kEvents", m_SVDvCut);
  addParam("TelCut", m_TelCut, "Cut for masking of PXD pixel - prreset for 10 kEvents", m_TelCut);

  addParam("PXDMaskFileBasicName", m_PXDMaskFileBasicName, "Name of file with list of masked channels",
           std::string("PXD_MaskFiredBasic.xml"));
  addParam("SVDMaskFileBasicName", m_SVDMaskFileBasicName, "Name of file with list of masked channels",
           std::string("SVD_MaskFiredBasic.xml"));
  addParam("TelMaskFileBasicName", m_TelMaskFileBasicName, "Name of file with list of masked channels",
           std::string("Tel_MaskFiredBasic.xml"));
  addParam("PXDMaskFileRunName", m_PXDMaskFileRunName, "Name of file with list of masked channels",
           std::string("PXD_MaskFired_RunXXX.xml"));
  addParam("SVDMaskFileRunName", m_SVDMaskFileRunName, "Name of file with list of masked channels",
           std::string("SVD_MaskFired_RunXXX.xml"));
  addParam("TelMaskFileRunName", m_TelMaskFileRunName, "Name of file with list of masked channels",
           std::string("Tel_MaskFired_RunXXX.xml"));
}


MaskingModule::~MaskingModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void MaskingModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlanePXD(i);
    for (int iS = 1; iS < c_MaxSensorsInPXDPlane; iS++) {
      m_PXDHitMapUV[iS * c_nPXDPlanes + i] = NULL;
      m_PXDMaskUV[iS * c_nPXDPlanes + i] = NULL;
      m_PXDSignal[iS * c_nPXDPlanes + i] = NULL;
      if (iS >= getSensorsInLayer(indexToPlanePXD(i))) continue;
      int nStripsU = getInfoPXD(i, iS + 1).getUCells();
      int nStripsV = getInfoPXD(i, iS + 1).getVCells();
      string name = str(format("hPXD_L%1%_S%2%_HitMap") % iPlane % (iS + 1));
      string title = str(format("TB2016 PXD layer %2%, sensor %1%, hit map") % (iS + 1) % iPlane);
      m_PXDHitMapUV[iS * c_nPXDPlanes + i] = new TH2F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU, nStripsV, 0, nStripsV);
      m_PXDHitMapUV[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("u [pitch]");
      m_PXDHitMapUV[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("v [pitch]");
      m_PXDHitMapUV[iS * c_nPXDPlanes + i]->GetZaxis()->SetTitle("count");

      name = str(format("hPXD_L%1%_S%2%_Mask") % iPlane % (iS + 1));
      title = str(format("TB2016 PXD layer %2%, sensor %1%, masked") % (iS + 1) % iPlane);
      m_PXDMaskUV[iS * c_nPXDPlanes + i] = new TH2F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU, nStripsV, 0, nStripsV);
      m_PXDMaskUV[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("u [pitch]");
      m_PXDMaskUV[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("v [pitch]");
      m_PXDMaskUV[iS * c_nPXDPlanes + i]->GetZaxis()->SetTitle("count");

      name = str(format("hPXD_L%1%_S%2%_Signal") % iPlane % (iS + 1));
      title = str(format("TB2016 PXD layer %2%, sensor %1%, signal") % (iS + 1) % iPlane);
      m_PXDSignal[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 100, 0, 100);
      m_PXDSignal[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("signal [ADU]");
      m_PXDSignal[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");

    }
  }

  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlaneSVD(i);
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      m_SVDHitMapU[iS * c_nSVDPlanes + i] = NULL;
      m_SVDHitMapV[iS * c_nSVDPlanes + i] = NULL;
      m_SVDMaskU[iS * c_nSVDPlanes + i] = NULL;
      m_SVDMaskV[iS * c_nSVDPlanes + i] = NULL;
      m_SVDSignalU[iS * c_nSVDPlanes + i] = NULL;
      m_SVDSignalV[iS * c_nSVDPlanes + i] = NULL;
      if (iS >= getSensorsInLayer(indexToPlaneSVD(i))) continue;
      int nStripsU = getInfoSVD(i, iS + 1).getUCells();
      int nStripsV = getInfoSVD(i, iS + 1).getVCells();
      string name = str(format("hSVD_L%1%_S%2%_HitMapU") % iPlane % (iS + 1));
      string title = str(format("TB2016 SVD layer %2%, sensor %1%, hit map in u") % (iS + 1) % iPlane);
      m_SVDHitMapU[iS * c_nSVDPlanes + i] = new TH2F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU, 6, 0, 6);
      m_SVDHitMapU[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("u [pitch]");
      m_SVDHitMapU[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("timestamp");
      m_SVDHitMapU[iS * c_nSVDPlanes + i]->GetZaxis()->SetTitle("count");
      name = str(format("hSVD_L%1%_S%2%_HitMapV") % iPlane % (iS + 1));
      title = str(format("TB2016 SVD layer %2%, sensor %1%, hit map in v") % (iS + 1) % iPlane);
      m_SVDHitMapV[iS * c_nSVDPlanes + i] = new TH2F(name.c_str(), title.c_str(), nStripsV, 0, nStripsV, 6, 0, 6);
      m_SVDHitMapV[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("v [pitch]");
      m_SVDHitMapV[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("timestamp");
      m_SVDHitMapV[iS * c_nSVDPlanes + i]->GetZaxis()->SetTitle("count");
      name = str(format("hSVD_L%1%_S%2%_MaskU") % iPlane % (iS + 1));
      title = str(format("TB2016 SVD layer %2%, sensor %1%, masked in u") % (iS + 1) % iPlane);
      m_SVDMaskU[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU);
      m_SVDMaskU[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("u [pitch]");
      m_SVDMaskU[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      name = str(format("hSVD_L%1%_S%2%_MaskV") % iPlane % (iS + 1));
      title = str(format("TB2016 SVD layer %2%, sensor %1%, masked in v") % (iS + 1) % iPlane);
      m_SVDMaskV[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), nStripsV, 0, nStripsV);
      m_SVDMaskV[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("v [pitch]");
      m_SVDMaskV[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      name = str(format("hSVD_L%1%_S%2%_SignalU") % iPlane % (iS + 1));
      title = str(format("TB2016 SVD layer %2%, sensor %1%, signal U") % (iS + 1) % iPlane);
      m_SVDSignalU[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 100, 0, 100);
      m_SVDSignalU[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("signal [ADU]");
      m_SVDSignalU[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      name = str(format("hSVD_L%1%_S%2%_SignalV") % iPlane % (iS + 1));
      title = str(format("TB2016 SVD layer %2%, sensor %1%, signal V") % (iS + 1) % iPlane);
      m_SVDSignalV[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 100, 0, 100);
      m_SVDSignalV[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("signal [ADU]");
      m_SVDSignalV[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
  }

  for (int i = 0; i < c_nTelPlanes; i++) {
    m_TelHitMapUV[i] = NULL;
    m_TelMaskUV[i] = NULL;
    int iPlane = indexToPlaneTel(i);
    int nStripsU = getInfoTel(i).getUCells();
    int nStripsV = getInfoTel(i).getVCells();
    string name = str(format("hTel_L%1%_S%2%_HitMap") % iPlane % 1);
    string title = str(format("TB2016 Tel layer %2%, sensor %1%, hit map") % 1 % iPlane);
    m_TelHitMapUV[i] = new TH2F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU, nStripsV, 0, nStripsV);
    m_TelHitMapUV[i]->GetXaxis()->SetTitle("u [pitch]");
    m_TelHitMapUV[i]->GetYaxis()->SetTitle("v [pitch]");
    m_TelHitMapUV[i]->GetZaxis()->SetTitle("count");
    name = str(format("hTel_L%1%_S%2%_Mask") % iPlane % 1);
    title = str(format("TB2016 Tel layer %2%, sensor %1%, masked") % 1 % iPlane);
    m_TelMaskUV[i] = new TH2F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU, nStripsV, 0, nStripsV);
    m_TelMaskUV[i]->GetXaxis()->SetTitle("u [pitch]");
    m_TelMaskUV[i]->GetYaxis()->SetTitle("v [pitch]");
    m_TelMaskUV[i]->GetZaxis()->SetTitle("count");
  }

  // cd back to root directory
  oldDir->cd();
}


void MaskingModule::initialize()
{
  //Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
//  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
//  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
//  StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);
  StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);

//  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
//  RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits);
//  RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits);

  // EventMetaData
  StoreObjPtr<EventMetaData> storeEventMetaData;
  //StoreObjPtr<EventMetaData> storeEventMetaData;
  storeEventMetaData.isRequired();

  //const StoreObjPtr<FileMetaData> storeFileMetaData(m_storeFileMetaDataName, DataStore::c_Persistent);
  //unsigned int EventLow = storeFileMetaData->getEventLow();
  //const StoreObjPtr<FileMetaData> storeFileMetaData(m_storeFileMetaDataName, DataStore::c_Persistent);
  //int Experiment = storeEventMetaData->getExperiment();
  //int RunNo = storeEventMetaData->getRun();
//  InputController::setNextEntry(Experiment, RunNo, EventLow + 14000 - 1090);   // jump to some event
  //InputController::setNextEntry(Experiment, RunNo, EventLow + 1120 + 0);   // jump to some event


  // Set maximum of procesed events
  if (m_nEventsProcess < 0) m_nEventsProcess = std::numeric_limits<long>::max();
  m_nEventsProcessFraction = 1.0;
  m_nRealEventsProcess = 0;
  for (int i = 0; i < c_nTBPlanes; i++) m_nEventsPlane[i] = 0;

  //Store names to speed up creation later
//  m_storePXDClustersName = storePXDClusters.getName();
  m_storePXDDigitsName = storePXDDigits.getName();
//  m_relPXDClusterDigitName = relPXDClusterDigits.getName();

//  m_storeSVDClustersName = storeSVDClusters.getName();
  m_storeSVDDigitsName = storeSVDDigits.getName();
//  m_relSVDClusterDigitName = relSVDClusterDigits.getName();

//  m_storeTelClustersName = storeTelClusters.getName();
  m_storeTelDigitsName = storeTelDigits.getName();
//  m_relTelClusterDigitName = relTelClusterDigits.getName();



  defineHisto();
}

void MaskingModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nPXDPlanes; i++) {
    for (int iS = 1; iS < c_MaxSensorsInPXDPlane; iS++) {  // TODO should be from 0, but sinsor 1 is missing on TB2016
      if (iS >= getSensorsInLayer(indexToPlanePXD(i))) continue;
      m_PXDHitMapUV[iS * c_nPXDPlanes + i]->Reset();
      m_PXDMaskUV[iS * c_nPXDPlanes + i]->Reset();
      m_PXDSignal[iS * c_nPXDPlanes + i]->Reset();
    }
  }

  for (int i = 0; i < c_nSVDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      if (iS >= getSensorsInLayer(indexToPlaneSVD(i))) continue;
      m_SVDHitMapU[iS * c_nSVDPlanes + i]->Reset();
      m_SVDHitMapV[iS * c_nSVDPlanes + i]->Reset();
      m_SVDMaskU[iS * c_nSVDPlanes + i]->Reset();
      m_SVDMaskV[iS * c_nSVDPlanes + i]->Reset();
      m_SVDSignalU[iS * c_nSVDPlanes + i]->Reset();
      m_SVDSignalV[iS * c_nSVDPlanes + i]->Reset();
    }
  }

  for (int i = 0; i < c_nTelPlanes; i++) {
    m_TelHitMapUV[i]->Reset();
    m_TelMaskUV[i]->Reset();
  }

}


void MaskingModule::event()
{
  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
//  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
//  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);

  const StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
//  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
//  const RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits, m_relSVDClusterDigitName);
//  RelationIndex < SVDDigit, SVDCluster > relSVDDigitCluster(DataStore::relationName(
//                                                              DataStore::arrayName<SVDDigit>(""),
//                                                              DataStore::arrayName<SVDCluster>(""))
//                                                           );

  const StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);
//  const StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);
//  const RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits, m_relTelClusterDigitName);



  StoreObjPtr<EventMetaData> storeEventMetaData;


  // printf("---->%i %i\n",storeEventMetaData->getEvent(),(unsigned int)m_nEventsProcess);
  if (m_nRealEventsProcess > (unsigned int)m_nEventsProcess) {
    //if (storeEventMetaData->getEvent() > (unsigned int)m_nEventsProcess) {
    //printf("------------------------>%i %i\n",storeEventMetaData->getEvent(),(unsigned int)m_nEventsProcess);
    storeEventMetaData->setEndOfData();
  }
  m_nRealEventsProcess++;

  // **************** PXD masking histograms **********************
  // If there are no PXD pixels, leave PXD histograms
  if (storePXDDigits && storePXDDigits.getEntries()) {
    for (const PXDDigit& digit : storePXDDigits) {
      int iPlane = digit.getSensorID().getLayerNumber();
      if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
      int index = planeToIndexPXD(iPlane);
      int Sensor = digit.getSensorID().getSensorNumber();
      m_nEventsPlane[index]++;
      m_PXDSignal[(Sensor - 1) * c_nPXDPlanes + index]->Fill(digit.getCharge());
      if ((digit.getCharge() > m_PXDCutSeedL) && (digit.getCharge() < m_PXDCutSeedH)) {
        m_PXDHitMapUV[(Sensor - 1) * c_nPXDPlanes + index]->Fill(digit.getUCellID(), digit.getVCellID());
      }
    }
  }

  // **************** SVD masking histograms **********************
  // If there are no SVD pixels, leave SVD histograms
  if (storeSVDDigits && storeSVDDigits.getEntries()) {
    for (const SVDDigit& digit : storeSVDDigits) {
      int iPlane = digit.getSensorID().getLayerNumber();
      if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
      int index = planeToIndexSVD(iPlane);
      int Sensor = digit.getSensorID().getSensorNumber();
      m_nEventsPlane[index + 2]++;
      if (digit.isUStrip()) {
        m_SVDSignalU[(Sensor - 1) * c_nSVDPlanes + index]->Fill(digit.getCharge());
      } else {
        m_SVDSignalV[(Sensor - 1) * c_nSVDPlanes + index]->Fill(digit.getCharge());
      }
      if (digit.getCharge() > m_CutSVDCharge) {
        if (digit.isUStrip()) {
          m_SVDHitMapU[(Sensor - 1) * c_nSVDPlanes + index]->Fill(digit.getCellID(), digit.getIndex());
        } else {
          m_SVDHitMapV[(Sensor - 1) * c_nSVDPlanes + index]->Fill(digit.getCellID(), digit.getIndex());
        }
      }
    }
  }

  // **************** Tel masking histograms **********************
  // If there are no Tel pixels, leave Tel histograms
  if (storeTelDigits && storeTelDigits.getEntries()) {
    for (const TelDigit& digit : storeTelDigits) {
      int iPlane = digit.getSensorID().getSensorNumber();  // Specific for telescopes!
      if ((iPlane < c_firstTelPlane) || (iPlane > c_lastTelPlane)) continue;
      int index = planeToIndexTel(iPlane);
      m_nEventsPlane[index + 6]++;
      m_TelHitMapUV[index]->Fill(digit.getUCellID(), digit.getVCellID());
    }
  }
  // ************ End Tel masking histograms **********************
}

void MaskingModule::endRun()
{

  if (m_nRealEventsProcess < 1000) {
    printf("Not enough data: %li < 1000, terminate without masking file create.\n", m_nRealEventsProcess);
    return;
  }

  m_nEventsProcessFraction = (double)m_nRealEventsProcess / m_nEventsProcess;

  // Maskin border for all sensors at 10 000 events!:
  float PXDCut = m_PXDCut;
  float TelCut = m_TelCut;

  float SVDCutU[] = {500.0, 500.0, 500.0, 500.0};
  float SVDCutV[] = {1000.0, 1000.0, 1000.0, 1000.0};
  float SVDCutUOut[] = {500.0, 500.0, 500.0, 500.0};
  float SVDCutVOut[] = {1000.0, 1000.0, 1000.0, 1000.0};
  float SVDMaskRegUMi[] = {0, 0, 0, 0};
  float SVDMaskRegUMa[] = {1000, 1000, 1000, 1000};
  float SVDMaskRegVMi[] = {0, 0, 0, 0};
  float SVDMaskRegVMa[] = {1000, 1000, 1000, 1000};

  for (int i = 0; i < 4; i++) {
    SVDCutU[i] = m_SVDuCut;
    SVDCutV[i] = m_SVDvCut;
    SVDCutUOut[i] = SVDCutU[i];
    SVDCutVOut[i] = SVDCutV[i];
  }

  // correction for unmerged events and different No. of proces events:
  PXDCut *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;
  for (int i = 0; i < c_nSVDPlanes; ++i) {
    SVDCutU[i] *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;
    SVDCutV[i] *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;
  }
  TelCut *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;

  FILE* MaskList;
  B2INFO("Start to create masking");
  printf("Start to create masking from %i events (fraction: %6.3f)\n", (int)m_nRealEventsProcess, m_nEventsProcessFraction);

  std::string FileName = str(format("%1%") % m_PXDMaskFileBasicName);
  std::string m_ignoredPixelsListName = str(format("%1%") % FileName);
  std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsBasicList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
        m_ignoredPixelsListName));
  FileName = str(format("%1%") % m_PXDMaskFileRunName);
  m_ignoredPixelsListName = str(format("%1%") % FileName);
  std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
      m_ignoredPixelsListName));
  MaskList = fopen(FileName.data(), "w");
  fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(MaskList, "<Meta>\n");
  fprintf(MaskList, "    <Date>20.04.2016</Date>\n");
  fprintf(MaskList, "    <Description short=\"Ignore strip list for PXD planes in 2016 VXD beam test\">\n");
  fprintf(MaskList, "        Crude initial list of bad pixels\n");
  fprintf(MaskList, "    </Description>\n");
  fprintf(MaskList, "    <Author>Peter Kodys</Author>\n");
  fprintf(MaskList, "</Meta>\n");
  fprintf(MaskList, "<PXD>\n");
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iLayer = getInfoPXD(i, 2).getID().getLayerNumber();
    int iLadder = getInfoPXD(i, 2).getID().getLadderNumber();
    if (m_nEventsPlane[i]) {
      fprintf(MaskList, "  <layer n=\"%i\">\n", iLayer);
      fprintf(MaskList, "    <ladder n=\"%i\">\n", iLadder);
      for (int j = 1; j < c_MaxSensorsInPXDPlane; j++) {
        if (j >= getSensorsInLayer(indexToPlanePXD(i))) continue;
        int nMasked = 0;
        int iSensor = getInfoPXD(i, j + 1).getID().getSensorNumber();
        fprintf(MaskList, "      <sensor n=\"%i\">\n", iSensor);
        fprintf(MaskList, "        <!-- Masking rectangular parts of the sensor -->\n");
        fprintf(MaskList, "        <!--pixels uStart = \"070\" uEnd = \"110\" vStart = \"0\" vEnd = \"500\"></pixels-->\n");
        fprintf(MaskList, "\n");
        fprintf(MaskList, "        <!-- Individual pixels can be masked, too -->\n");
        fprintf(MaskList, "        <!--pixels uStart = \"130\" vStart = \"500\"></pixels-->\n");
        fprintf(MaskList, "\n");
        fprintf(MaskList, "        <!-- ROW is V  /  COLUMN is U -->\n");
        fprintf(MaskList, "\n");
        fprintf(MaskList, "        <!-- Individual rows and columns can be masked, too -->\n");
        fprintf(MaskList, "        <!--pixels vStart = \"500\"></pixels-->\n");
        fprintf(MaskList, "        <!--pixels uStart = \"120\"></pixels-->\n");
        fprintf(MaskList, "\n");
        fprintf(MaskList, "        <!-- Ranges of rows and columns can be masked, too -->\n");
        fprintf(MaskList, "        <!--pixels vStart = \"100\" vEnd = \"120\"></pixels-->\n");
        fprintf(MaskList, "        <!--pixels uStart = \"120\" uEnd = \"202\"></pixels-->\n");
        fprintf(MaskList, "\n");
        for (int i1 = 0; i1 < m_PXDMaskUV[j * c_nPXDPlanes + i]->GetNbinsX(); ++i1) {
          for (int i2 = 0; i2 < m_PXDMaskUV[j * c_nPXDPlanes + i]->GetNbinsY(); ++i2) {
            int ExistMask = 0;
            if (m_AppendMaskFile) {
              if (!m_ignoredPixelsBasicList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
                ExistMask += 1;
              }
              if (!m_ignoredPixelsList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
                ExistMask += 1;
              }
            }
            if (ExistMask || (m_PXDHitMapUV[j * c_nPXDPlanes + i]->GetBinContent(i1 + 1, i2 + 1) > PXDCut)) {
              fprintf(MaskList, "        <pixels uStart = \"%04i\" vStart = \"%04i\"></pixels>\n", i1, i2);
              m_PXDMaskUV[j * c_nPXDPlanes + i]->SetBinContent(i1 + 1, i2 + 1, 1 + ExistMask);
              nMasked++;
            }
          }
        }
        fprintf(MaskList, "\n");
        fprintf(MaskList, "      </sensor>\n");
        B2INFO("PXD(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMasked << " pixels in: " <<
               m_ignoredPixelsListName.data());
        printf("PXD(%i,%i,%i) masked %i pixels in: %s\n", iLayer, iLadder, iSensor, nMasked, m_ignoredPixelsListName.data());
      }
      fprintf(MaskList, "    </ladder>\n");
      fprintf(MaskList, "  </layer>\n");
    }
  }
  fprintf(MaskList, "</PXD>\n");
  fclose(MaskList);

  FileName = str(format("%1%") % m_SVDMaskFileBasicName);
  m_ignoredPixelsListName = str(format("%1%") % FileName);
  std::unique_ptr<SVDIgnoredStripsMap> m_ignoredStripsBasicList = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(
        m_ignoredPixelsListName));
  string FileName2 = str(format("%1%") % m_SVDMaskFileRunName);
  string m_ignoredPixelsListName2 = str(format("%1%") % FileName2);
  std::unique_ptr<SVDIgnoredStripsMap> m_ignoredStripsList = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(
      m_ignoredPixelsListName2));
  MaskList = fopen(FileName2.data(), "w");
  fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(MaskList, "<Meta>\n");
  fprintf(MaskList, "    <Date>20.04.2016</Date>\n");
  fprintf(MaskList, "    <Description short=\"Ignore strip list for SVD planes in 2016 VXD beam test\">\n");
  fprintf(MaskList, "        Crude initial list of bad strips\n");
  fprintf(MaskList, "    </Description>\n");
  fprintf(MaskList, "    <Author>Peter Kodys</Author>\n");
  fprintf(MaskList, "</Meta>\n");
  fprintf(MaskList, "<SVD>\n");
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iLayer = getInfoSVD(i, 2).getID().getLayerNumber();
    if (m_nEventsPlane[i + 2]) {
      int iLadder = getInfoSVD(i, 2).getID().getLadderNumber();
      fprintf(MaskList, "    <layer n=\"%i\">\n", iLayer);
      fprintf(MaskList, "        <ladder n=\"%i\">\n", iLadder);
      for (int j = 0; j < c_MaxSensorsInSVDPlane; j++) {
        if (j >= getSensorsInLayer(indexToPlaneSVD(i))) continue;
        int nMaskedU = 0;
        int nMaskedV = 0;
        int iSensor = getInfoSVD(i, j + 1).getID().getSensorNumber();
        fprintf(MaskList, "            <sensor n=\"%i\">\n", iSensor);
        fprintf(MaskList, "                <side side=\"u\">\n");
        fprintf(MaskList, "                    <!-- stripsFromTo fromStrip = \"620\" toStrip = \"767\"></stripsFromTo-->\n");
        fprintf(MaskList, "                    <!-- Individual strips can be masked, too -->\n");
        for (int i1 = 0; i1 < m_SVDMaskU[j * c_nSVDPlanes + i]->GetNbinsX(); ++i1) {
          int ExistMask = 0;
          if (m_AppendMaskFile) {
            if (!m_ignoredStripsBasicList->stripOK(VxdID(iLayer, iLadder, iSensor, 1), (unsigned short) i1)) {
              ExistMask += 1;
            }
            if (!m_ignoredStripsList->stripOK(VxdID(iLayer, iLadder, iSensor, 1), (unsigned short) i1)) {
              ExistMask += 1;
            }
          }
          float SVDCut = SVDCutUOut[i];
          if ((i1 > SVDMaskRegUMi[i]) && (i1 < SVDMaskRegUMa[i]))
            SVDCut = SVDCutU[i];
          int sTS = 0;
          for (int iTS = 0; iTS < 6; iTS++) {  // look for occupancy on timestamps
            if (m_SVDHitMapU[j * c_nSVDPlanes + i]->GetBinContent(i1 + 1, iTS + 1) > SVDCut) {
              sTS++;
            }
          }

          if (ExistMask || (sTS > 1)) {  // in al least two timestamps over cut for masking...
            fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i1);
            m_SVDMaskU[j * c_nSVDPlanes + i]->SetBinContent(i1 + 1, 1 + ExistMask);
            nMaskedU++;
          }
        }
        fprintf(MaskList, "                </side>\n");
        fprintf(MaskList, "                <side side=\"v\">\n");
        for (int i2 = 0; i2 < m_SVDMaskV[j * c_nSVDPlanes + i]->GetNbinsX(); ++i2) {
          int ExistMask = 0;
          if (m_AppendMaskFile) {
            if (!m_ignoredStripsBasicList->stripOK(VxdID(iLayer, iLadder, iSensor, 0), (unsigned short) i2)) {
              ExistMask += 1;
            }
            if (!m_ignoredStripsList->stripOK(VxdID(iLayer, iLadder, iSensor, 0), (unsigned short) i2)) {
              ExistMask += 1;
            }
          }
          float SVDCut = SVDCutVOut[i];
          if ((i2 > SVDMaskRegVMi[i]) && (i2 < SVDMaskRegVMa[i]))
            SVDCut = SVDCutV[i];
          int sTS = 0;
          for (int iTS = 0; iTS < 6; iTS++) {  // look for occupancy on timestamps
            if (m_SVDHitMapV[j * c_nSVDPlanes + i]->GetBinContent(i2 + 1, iTS + 1) > SVDCut) sTS++;
          }

          if (ExistMask || (sTS > 1)) {  // in al least two timestamps over cut for masking...
            fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i2);
            m_SVDMaskV[j * c_nSVDPlanes + i]->SetBinContent(i2 + 1, 1 + ExistMask);
            nMaskedV++;
          }
        }
        fprintf(MaskList, "                </side>\n");
        fprintf(MaskList, "            </sensor>\n");
        B2INFO("SVD(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMaskedU << " U strips in: " <<
               m_ignoredPixelsListName.data());
        B2INFO("SVD(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMaskedV << " V strips in: " <<
               m_ignoredPixelsListName.data());
        printf("SVD(%i,%i,%i) masked %i U strips in: %s\n", iLayer, iLadder, iSensor, nMaskedU, m_ignoredPixelsListName.data());
        printf("SVD(%i,%i,%i) masked %i V strips in: %s\n", iLayer, iLadder, iSensor, nMaskedV, m_ignoredPixelsListName.data());
      }
      fprintf(MaskList, "        </ladder>\n");
      fprintf(MaskList, "    </layer>\n");
    }
  }
  fprintf(MaskList, "</SVD>\n");
  fclose(MaskList);

  FileName = str(format("%1%") % m_TelMaskFileBasicName);
  m_ignoredPixelsListName = str(format("%1%") % FileName);
  std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredTelPixelsBasicList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
        m_ignoredPixelsListName));
  FileName = str(format("%1%") % m_TelMaskFileRunName);
  m_ignoredPixelsListName = str(format("%1%") % FileName);
  std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredTelPixelsList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
        m_ignoredPixelsListName));
  MaskList = fopen(FileName.data(), "w");
  fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(MaskList, "<Meta>\n");
  fprintf(MaskList, "    <Date>20.04.2016</Date>\n");
  fprintf(MaskList, "    <Description short=\"Ignore strip list for Tel planes in 2016 VXD beam test\">\n");
  fprintf(MaskList, "        Crude initial list of bad pixels\n");
  fprintf(MaskList, "    </Description>\n");
  fprintf(MaskList, "    <Author>Peter Kodys</Author>\n");
  fprintf(MaskList, "</Meta>\n");
  fprintf(MaskList, "<PXD>\n");
  for (int i = 0; i < c_nTelPlanes; i++) {
    if (m_nEventsPlane[i + 6]) {
      int nMasked = 0;
      int iLayer = getInfoTel(i).getID().getLayerNumber();
      int iLadder = getInfoTel(i).getID().getLadderNumber();
      int iSensor = getInfoTel(i).getID().getSensorNumber();
      fprintf(MaskList, "  <layer n=\"%i\">\n", iLayer);
      fprintf(MaskList, "    <ladder n=\"%i\">\n", iLadder);
      fprintf(MaskList, "      <sensor n=\"%i\">\n", iSensor);
      fprintf(MaskList, "        <!-- Masking rectangular parts of the sensor -->\n");
      fprintf(MaskList, "        <!--pixels uStart = \"070\" uEnd = \"110\" vStart = \"0\" vEnd = \"500\"></pixels-->\n");
      fprintf(MaskList, "\n");
      fprintf(MaskList, "        <!-- Individual pixels can be masked, too -->\n");
      fprintf(MaskList, "        <!--pixels uStart = \"130\" vStart = \"500\"></pixels-->\n");
      fprintf(MaskList, "\n");
      fprintf(MaskList, "        <!-- ROW is V  /  COLUMN is U -->\n");
      fprintf(MaskList, "\n");
      fprintf(MaskList, "        <!-- Individual rows and columns can be masked, too -->\n");
      fprintf(MaskList, "        <!--pixels vStart = \"500\"></pixels-->\n");
      fprintf(MaskList, "        <!--pixels uStart = \"120\"></pixels-->\n");
      fprintf(MaskList, "\n");
      fprintf(MaskList, "        <!-- Ranges of rows and columns can be masked, too -->\n");
      fprintf(MaskList, "        <!--pixels vStart = \"100\" vEnd = \"120\"></pixels-->\n");
      fprintf(MaskList, "        <!--pixels uStart = \"120\" uEnd = \"202\"></pixels-->\n");
      fprintf(MaskList, "\n");

      for (int i1 = 0; i1 < m_TelMaskUV[i]->GetNbinsX(); ++i1) {
        for (int i2 = 0; i2 < m_TelMaskUV[i]->GetNbinsY(); ++i2) {
          int ExistMask = 0;
          if (m_AppendMaskFile) {
            if (!m_ignoredTelPixelsBasicList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
              ExistMask += 1;
            }
            if (!m_ignoredTelPixelsList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
              ExistMask += 1;
            }
          }
          if (ExistMask || (m_TelHitMapUV[i]->GetBinContent(i1 + 1, i2 + 1) > TelCut)) {
            fprintf(MaskList, "        <pixels uStart = \"%04i\" vStart = \"%04i\"></pixels>\n", i1, i2);
            m_TelMaskUV[i]->SetBinContent(i1 + 1, i2 + 1, 1 + ExistMask);
            nMasked++;
          }
        }
      }
      fprintf(MaskList, "\n");
      fprintf(MaskList, "      </sensor>\n");
      fprintf(MaskList, "    </ladder>\n");
      fprintf(MaskList, "  </layer>\n");
      B2INFO("Tel(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMasked << " pixels in: " <<
             m_ignoredPixelsListName.data());
      printf("Tel(%i,%i,%i) masked %i pixels in: %s\n", iLayer, iLadder, iSensor, nMasked, m_ignoredPixelsListName.data());
    }
  }
  fprintf(MaskList, "</PXD>\n");
  fclose(MaskList);

  printf("Statistics: all events: %i, use: %i (%4.2f)\n", m_StatAllEvents, m_StatSelEvents, (float)m_StatSelEvents / m_StatAllEvents);
  for (int i = 0; i < c_nTBPlanes; i++) {
    if (m_StatSelEvents) {
      m_StatEverageOccupancy[i * 2 + 0] /= (float)m_StatSelEvents;
      m_StatEverageOccupancy[i * 2 + 1] /= (float)m_StatSelEvents;
    }
    printf("     Plane %i, avrg occupanci: u: %4.2f v: %4.2f\n", i, m_StatEverageOccupancy[i * 2 + 0],
           m_StatEverageOccupancy[i * 2 + 1]);
  }
}


void MaskingModule::terminate()
{
}

int MaskingModule::CallSVDFilter(const SVDCluster* cluster)
{
// ************* START: Filter for right SVD shape in time **************************************
  RelationVector<SVDDigit> relatedStrips = cluster->getRelationsTo<SVDDigit>("SVDDigits");
  if (abs((int)relatedStrips.size()) == 0) return 0;  // no strips in cluster
  if (((float)cluster->getClsTime() > -50) && ((float)cluster->getClsTime() < 0)) return 1;
  return 0;




  int IsCluster = 0;
  int iStoreStrip[6];
  int iStoreCharge[6];
  int indexx = 100;
  for (SVDDigit& digit : relatedStrips) {   // go over strips
    if (indexx > (int)digit.getIndex()) {  // reset buffer in new strip
      for (int ii = 0; ii < 6; ++ii) {
        iStoreStrip[ii] = 0;
        iStoreCharge[ii] = 0;
      }
    }
    indexx = (int)digit.getIndex();
    if ((indexx < 0) || (indexx > 5)) indexx = 0;
    iStoreStrip[indexx] = (int)digit.getCellID();
    iStoreCharge[indexx] = (int)digit.getCharge();
    if (indexx == 5) { // eval cluster existence: only for full strip response in time
      if ((iStoreCharge[0] < iStoreCharge[1]) &&
          (iStoreCharge[1] < iStoreCharge[2]) &&
          (iStoreCharge[2] > iStoreCharge[3]) &&
          (iStoreCharge[3] > iStoreCharge[4]) &&
          (iStoreCharge[4] > iStoreCharge[5]) &&
          (iStoreStrip[0] == iStoreStrip[1]) &&
          (iStoreStrip[1] == iStoreStrip[2]) &&
          (iStoreStrip[2] == iStoreStrip[3]) &&
          (iStoreStrip[3] == iStoreStrip[4]) &&
          (iStoreStrip[4] == iStoreStrip[5])
         )
        IsCluster++;  // to count how many strips fired in all timestamps and right shape
    }
  }
  if (IsCluster == 0) return 0;  // al least one strip must firing in last timestamp and right time shape
  return 1;
// ************* END: Filter for right SVD shape in time ****************************************
}
