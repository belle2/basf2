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
  setDescription("Masking of pixels and strips in PXD, VXD and tels");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("Clusters", m_storeTelClustersName, "Name of the telescopes cluster collection",
           std::string("TelClusters")); // always be explicit about this, can cause trouble
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("mask"));
  m_MaskDirectoryPath = std::string("");
  addParam("MaskDirectoryPath", m_MaskDirectoryPath, "Path to place of mask files", m_MaskDirectoryPath);
  m_nEventsProcess = -1;
  addParam("nEventsProcess", m_nEventsProcess, "Number of events to process", m_nEventsProcess);
  m_AppendMaskFile = 1;
  addParam("AppendMaskFile", m_AppendMaskFile, "Set option for append of existing file or recreate new list", m_AppendMaskFile);
  m_SVDStrongMasking = 0;     /**< Use strong SVD masking with time shape */
  addParam("SVDStrongMasking", m_SVDStrongMasking, "1: strong SVD masking with time shape; 2: correlation masking",
           m_SVDStrongMasking);
  m_MaskingStep = 0;
  addParam("MaskingStep", m_MaskingStep, "0: prepare SVD time shape; 1: apply SVD time shape and create masking", m_MaskingStep);
  if (m_MaskingStep != 0) m_MaskingStep = 1;
  m_PXDCutSeedL = 0;
  m_PXDCutSeedH = 100000;
  addParam("PXDCutSeedL", m_PXDCutSeedL, "PXD: seed cut lower border", m_PXDCutSeedL);
  addParam("PXDCutSeedH", m_PXDCutSeedH, "PXD: seed cut higher border", m_PXDCutSeedH);

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

  for (int i = 0; i < c_nTBPlanes; i++) {
    int iPlane;
    string name;
    string title;
    int nPixelsU;
    int nPixelsV;
    int nChargeRange;
    if (i < c_lastPXDPlane) {                                    // PXD
      iPlane = indexToPlanePXD(i);
      name = str(format("h2PXD%1%HitmapUV") % iPlane);
      title = str(format("PXD Hitmap in U x V, plane %1%") % iPlane);
      nPixelsU = getInfoPXD(i).getUCells();
      nPixelsV = getInfoPXD(i).getVCells();
    } else if ((i >= c_lastPXDPlane) && (i < c_lastSVDPlane)) {  // SVD
      iPlane = indexToPlaneSVD(i);
      name = str(format("h2SVD%1%HitmapUV") % iPlane);
      title = str(format("SVD Hitmap in U x V, plane %1%") % iPlane);
      nPixelsU = getInfoSVD(i).getUCells();
      nPixelsV = getInfoSVD(i).getVCells();
    } else {                                                     // Tel
      iPlane = indexToPlaneTel(i - 6);
      name = str(format("h2Tel%1%HitmapUV") % iPlane);
      title = str(format("Tel Hitmap in U x V, plane %1%") % iPlane);
      nPixelsU = getInfoTel(i - 6).getUCells();
      nPixelsV = getInfoTel(i - 6).getVCells();
    }
    //----------------------------------------------------------------
    // Hitmaps: Number of clusters by coordinate : Hitmap[PlaneNo]
    //----------------------------------------------------------------
    // Hitmaps in UV
    m_hitMapUV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
    m_hitMapUV[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapUV[i]->GetYaxis()->SetTitle("v position [pitch units]");
    m_hitMapUV[i]->GetZaxis()->SetTitle("hits");
    //----------------------------------------------------------------
    // Digitmaps: Number of digits by coordinate : DigitMap[PlaneNo]
    //----------------------------------------------------------------
    // Digitmaps in UV
    if (i < c_lastPXDPlane) {                                    // PXD
      name = str(format("h2PXD%1%DigitHitmapUV") % iPlane);
      title = str(format("PXD Digit Hitmap in U x V, plane %1%") % iPlane);
    } else if ((i >= c_lastPXDPlane) && (i < c_lastSVDPlane)) {  // SVD
      name = str(format("h2SVD%1%DigitHitmapUV") % iPlane);
      title = str(format("SVD Digit Hitmap in U x V, plane %1%") % iPlane);
    } else {                                                     // Tel
      name = str(format("h2Tel%1%DigitHitmapUV") % iPlane);
      title = str(format("Tel Digit Hitmap in U x V, plane %1%") % iPlane);
    }
    m_digitMapUV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
    m_digitMapUV[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_digitMapUV[i]->GetYaxis()->SetTitle("v position [pitch units]");
    m_digitMapUV[i]->GetZaxis()->SetTitle("digits");
    //----------------------------------------------------------------
    // Charge of digits : hDigitCharge[PlaneNo]
    //----------------------------------------------------------------
    // digit charge by plane
    nChargeRange = 500;
    if (i < c_lastPXDPlane) {                                    // PXD
      name = str(format("hPXD%1%DigitCharge") % iPlane);
      title = str(format("PXD digit charge, plane %1%") % iPlane);
    } else if ((i >= c_lastPXDPlane) && (i < c_lastSVDPlane)) {  // SVD
      name = str(format("hSVD%1%DigitCharge") % iPlane);
      title = str(format("SVD digit charge, plane %1%") % iPlane);
    } else {                                                     // Tel
      nChargeRange = 10;
      name = str(format("hTel%1%DigitCharge") % iPlane);
      title = str(format("Tel digit charge, plane %1%") % iPlane);
    }
    m_digitCharge[i] = new TH1F(name.c_str(), title.c_str(), nChargeRange, 0, nChargeRange);
    m_digitCharge[i]->GetXaxis()->SetTitle("pixel charge [ADU]");
    m_digitCharge[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of clusters : hClusterCharge[PlaneNo]
    //----------------------------------------------------------------
    // cluster charge by plane
    if (i < c_lastPXDPlane) {                                    // PXD
      name = str(format("hPXD%1%ClusterCharge") % iPlane);
      title = str(format("PXD Cluster charge, plane %1%") % iPlane);
    } else if ((i >= c_lastPXDPlane) && (i < c_lastSVDPlane)) {  // SVD
      name = str(format("hSVD%1%ClusterCharge") % iPlane);
      title = str(format("SVD Cluster charge, plane %1%") % iPlane);
    } else {                                                     // Tel
      name = str(format("hTel%1%ClusterCharge") % iPlane);
      title = str(format("Tel Cluster charge, plane %1%") % iPlane);
    }
    m_clusterCharge[i] = new TH1F(name.c_str(), title.c_str(), nChargeRange, 0, nChargeRange);
    m_clusterCharge[i]->GetXaxis()->SetTitle("pixel charge [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster seed charge distribution : hSeed[PlaneNo]
    //----------------------------------------------------------------
    // seed by plane
    if (i < c_lastPXDPlane) {                                    // PXD
      name = str(format("hPXD%1%Seed") % iPlane);
      title = str(format("PXD Seed, plane %1%") % iPlane);
    } else if ((i >= c_lastPXDPlane) && (i < c_lastSVDPlane)) {  // SVD
      name = str(format("hSVD%1%Seed") % iPlane);
      title = str(format("SVD Seed, plane %1%") % iPlane);
    } else {                                                     // Tel
      name = str(format("hTel%1%Seed") % iPlane);
      title = str(format("Tel Seed, plane %1%") % iPlane);
    }
    m_seed[i] = new TH1F(name.c_str(), title.c_str(), nChargeRange, 0, nChargeRange);
    m_seed[i]->GetXaxis()->SetTitle("seed charge [ADU]");
    m_seed[i]->GetYaxis()->SetTitle("count");
  }
  for (int i1 = 0; i1 < c_nTBPlanes - 1; i1++) {
    int i2 = i1 + 1;
    int nPixelsU;
    int nPixelsV;
    float nPixelsExtenzion = 20;
    int nPixelsUExtend;
    int nPixelsVExtend;
    float PixelsUStart;
    float PixelsUEnd;
    float PixelsVStart;
    float PixelsVEnd;
    string AxisU = str(format("y position plane%1% [cm]") % i1);
    string AxisV = str(format("y position plane%1% [cm]") % i2);
    string name = str(format("hCorrNeighboreYu_%1%_%2%_yy") % i1 % i2);
    string title = str(format("Corelation %1% vs %2%, axis y x y") % i1 % i2);

    if (i1 < 3) {        // Tel 0,1,2
      nPixelsU = getInfoTel(i1).getUCells();
      PixelsUEnd = getInfoTel(i1).getUSize();
    } else if (i1 < 5) { // PXD 1,2
      nPixelsU = getInfoPXD(i1 - 3).getUCells();
      PixelsUEnd = getInfoPXD(i1 - 3).getUSize();
    } else if (i1 < 9) { // SVD 3,4,5,6
      nPixelsU = getInfoSVD(i1 - 3).getUCells();
      PixelsUEnd = getInfoSVD(i1 - 3).getUSize();
    } else {             // Tel 3,4,5
      nPixelsU = getInfoTel(i1 - 6).getUCells();
      PixelsUEnd = getInfoTel(i1 - 6).getUSize();
    }
    if (i2 < 3) {        // Tel 0,1,2
      nPixelsV = getInfoTel(i2).getUCells();
      PixelsVEnd = getInfoTel(i2).getUSize();
    } else if (i2 < 5) { // PXD 1,2
      nPixelsV = getInfoPXD(i2 - 3).getUCells();
      PixelsVEnd = getInfoPXD(i2 - 3).getUSize();
    } else if (i2 < 9) { // SVD 3,4,5,6
      nPixelsV = getInfoSVD(i2 - 3).getUCells();
      PixelsVEnd = getInfoSVD(i2 - 3).getUSize();
    } else {             // Tel 3,4,5
      nPixelsV = getInfoTel(i2 - 6).getUCells();
      PixelsVEnd = getInfoTel(i2 - 6).getUSize();
    }

    nPixelsUExtend = nPixelsU + 2 * nPixelsExtenzion;
    PixelsUStart = PixelsUEnd * (-0.5 - nPixelsExtenzion / nPixelsU);
    PixelsUEnd = - PixelsUStart;
    nPixelsVExtend = nPixelsV + 2 * nPixelsExtenzion;
    PixelsVStart = PixelsVEnd * (-0.5 - nPixelsExtenzion / nPixelsV);
    PixelsVEnd = - PixelsVStart;

    m_CorrNeighboreYu[i1] = new TH2F(name.c_str(), title.c_str(), nPixelsUExtend, PixelsUStart, PixelsUEnd, nPixelsVExtend,
                                     PixelsVStart, PixelsVEnd);
    m_CorrNeighboreYu[i1]->GetXaxis()->SetTitle(AxisU.data());
    m_CorrNeighboreYu[i1]->GetYaxis()->SetTitle(AxisV.data());
    m_CorrNeighboreYu[i1]->GetZaxis()->SetTitle("count");

    AxisU = str(format("z position plane%1% [cm]") % i1);
    AxisV = str(format("z position plane%1% [cm]") % i2);
    name = str(format("hCorrNeighboreZv_%1%_%2%_zz") % i1 % i2);
    title = str(format("Corelation %1% vs %2%, axis z x z") % i1 % i2);

    if (i1 < 3) {        // Tel 0,1,2
      nPixelsU = getInfoTel(i1).getVCells();
      PixelsUEnd = getInfoTel(i1).getVSize();
    } else if (i1 < 5) { // PXD 1,2
      nPixelsU = getInfoPXD(i1 - 3).getVCells();
      PixelsUEnd = getInfoPXD(i1 - 3).getVSize();
    } else if (i1 < 9) { // SVD 3,4,5,6
      nPixelsU = getInfoSVD(i1 - 3).getVCells();
      PixelsUEnd = getInfoSVD(i1 - 3).getVSize();
    } else {             // Tel 3,4,5
      nPixelsU = getInfoTel(i1 - 6).getVCells();
      PixelsUEnd = getInfoTel(i1 - 6).getVSize();
    }
    if (i2 < 3) {        // Tel 0,1,2
      nPixelsV = getInfoTel(i2).getVCells();
      PixelsVEnd = getInfoTel(i2).getVSize();
    } else if (i2 < 5) { // PXD 1,2
      nPixelsV = getInfoPXD(i2 - 3).getVCells();
      PixelsVEnd = getInfoPXD(i2 - 3).getVSize();
    } else if (i2 < 9) { // SVD 3,4,5,6
      nPixelsV = getInfoSVD(i2 - 3).getVCells();
      PixelsVEnd = getInfoSVD(i2 - 3).getVSize();
    } else {             // Tel 3,4,5
      nPixelsV = getInfoTel(i2 - 6).getVCells();
      PixelsVEnd = getInfoTel(i2 - 6).getVSize();
    }

    nPixelsUExtend = nPixelsU + 2 * nPixelsExtenzion;
    PixelsUStart = PixelsUEnd * (-0.5 - nPixelsExtenzion / nPixelsU);
    PixelsUEnd = - PixelsUStart;
    nPixelsVExtend = nPixelsV + 2 * nPixelsExtenzion;
    PixelsVStart = PixelsVEnd * (-0.5 - nPixelsExtenzion / nPixelsV);
    PixelsVEnd = - PixelsVStart;

    m_CorrNeighboreZv[i1] = new TH2F(name.c_str(), title.c_str(), nPixelsUExtend, PixelsUStart, PixelsUEnd, nPixelsVExtend,
                                     PixelsVStart, PixelsVEnd);
    m_CorrNeighboreZv[i1]->GetXaxis()->SetTitle(AxisU.data());
    m_CorrNeighboreZv[i1]->GetYaxis()->SetTitle(AxisV.data());
    m_CorrNeighboreZv[i1]->GetZaxis()->SetTitle("count");

  }
  //----------------------------------------------------------------
  // Special for SVD:
  // digit charge, count, timestamp distribution : hSVDCharge/Count/Time[SVDNo]
  //----------------------------------------------------------------
  // charge/count vs time by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    string name;
    string title;
    int iPlane = indexToPlaneSVD(i + c_lastPXDPlane);
    int nPixelsU = getInfoSVD(i + c_lastPXDPlane).getUCells();
    int nPixelsV = getInfoSVD(i + c_lastPXDPlane).getVCells();
    int nTimeStamps = 6;

    name = str(format("h2SVD%1%HitMapU") % iPlane);
    title = str(format("SVD Digit Hitmap vs. timestamp in U, plane %1%") % iPlane);
    m_SVDHitMapU[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nTimeStamps, 0, nTimeStamps);
    m_SVDHitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_SVDHitMapU[i]->GetYaxis()->SetTitle("timestamp [time tick]");
    m_SVDHitMapU[i]->GetZaxis()->SetTitle("counts");

    name = str(format("h2SVD%1%AvrChargeU") % iPlane);
    title = str(format("SVD digit average charge vs. timestamp in U, plane %1%") % iPlane);
    m_SVDAvrChargeU[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nTimeStamps, 0, nTimeStamps);
    m_SVDAvrChargeU[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_SVDAvrChargeU[i]->GetYaxis()->SetTitle("timestamp [time tick]");
    m_SVDAvrChargeU[i]->GetZaxis()->SetTitle("charge [ADU]");

    name = str(format("h2SVD%1%ClusterTimeU") % iPlane);
    title = str(format("SVD cluster vs. timestamp in U, plane %1%") % iPlane);
    m_SVDClusterTimeU[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, 100, -3 * 30, (nTimeStamps - 3) * 30);
    m_SVDClusterTimeU[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_SVDClusterTimeU[i]->GetYaxis()->SetTitle("timestamp [ns]");
    m_SVDClusterTimeU[i]->GetZaxis()->SetTitle("charge [ADU]");

    name = str(format("h1SVD%1%ClusterTimeMapU") % iPlane);
    title = str(format("SVD cluster time in U, plane %1%") % iPlane);
    m_SVDClusterTimeMapU[i] = new TH1F(name.c_str(), title.c_str(), 100, -3 * 30, (nTimeStamps - 3) * 30);
    m_SVDClusterTimeMapU[i]->GetXaxis()->SetTitle("u timestamp [ns]");
    m_SVDClusterTimeMapU[i]->GetYaxis()->SetTitle("counts");


    name = str(format("h2SVD%1%HitMapV") % iPlane);
    title = str(format("SVD Digit Hitmap vs. timestamp in V, plane %1%") % iPlane);
    m_SVDHitMapV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsV, 0, nPixelsV, nTimeStamps, 0, nTimeStamps);
    m_SVDHitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_SVDHitMapV[i]->GetYaxis()->SetTitle("timestamp [time tick]");
    m_SVDHitMapV[i]->GetZaxis()->SetTitle("counts");

    name = str(format("h2SVD%1%AvrChargeV") % iPlane);
    title = str(format("SVD digit average charge vs. timestamp in V, plane %1%") % iPlane);
    m_SVDAvrChargeV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsV, 0, nPixelsV, nTimeStamps, 0, nTimeStamps);
    m_SVDAvrChargeV[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_SVDAvrChargeV[i]->GetYaxis()->SetTitle("timestamp [time tick]");
    m_SVDAvrChargeV[i]->GetZaxis()->SetTitle("charge [ADU]");

    name = str(format("h2SVD%1%ClusterTimeV") % iPlane);
    title = str(format("SVD cluster vs. timestamp in V, plane %1%") % iPlane);
    m_SVDClusterTimeV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsV, 0, nPixelsV, 100, -3 * 30, (nTimeStamps - 3) * 30);
    m_SVDClusterTimeV[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_SVDClusterTimeV[i]->GetYaxis()->SetTitle("timestamp [ns]");
    m_SVDClusterTimeV[i]->GetZaxis()->SetTitle("charge [ADU]");

    name = str(format("h1SVD%1%ClusterTimeMapV") % iPlane);
    title = str(format("SVD cluster time in V, plane %1%") % iPlane);
    m_SVDClusterTimeMapV[i] = new TH1F(name.c_str(), title.c_str(), 100, -3 * 30, (nTimeStamps - 3) * 30);
    m_SVDClusterTimeMapV[i]->GetXaxis()->SetTitle("v timestamp [ns]");
    m_SVDClusterTimeMapV[i]->GetYaxis()->SetTitle("counts");


  }
  // cd back to root directory
  oldDir->cd();
}


void MaskingModule::initialize()
{
  //Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);
  StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);
  StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
  RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits);
  RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits);

  // EventMetaData
  StoreObjPtr<EventMetaData> storeEventMetaData;
  //StoreObjPtr<EventMetaData> storeEventMetaData;
  storeEventMetaData.required();

  // Set maximum of procesed events
  if (m_nEventsProcess < 0) m_nEventsProcess = std::numeric_limits<long>::max();
  m_nEventsProcessFraction = 1.0;
  m_nRealEventsProcess = 0;

  //Store names to speed up creation later
  m_storePXDClustersName = storePXDClusters.getName();
  m_storePXDDigitsName = storePXDDigits.getName();
  m_relPXDClusterDigitName = relPXDClusterDigits.getName();

  m_storeSVDClustersName = storeSVDClusters.getName();
  m_storeSVDDigitsName = storeSVDDigits.getName();
  m_relSVDClusterDigitName = relSVDClusterDigits.getName();

  m_storeTelClustersName = storeTelClusters.getName();
  m_storeTelDigitsName = storeTelDigits.getName();
  m_relTelClusterDigitName = relTelClusterDigits.getName();

  m_storeFramesName = storeFrames.getName();

  defineHisto();
}

void MaskingModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_SVDHitMapU[i]->Reset();
    m_SVDHitMapV[i]->Reset();
    m_SVDAvrChargeU[i]->Reset();
    m_SVDAvrChargeV[i]->Reset();
    m_SVDClusterTimeU[i]->Reset();
    m_SVDClusterTimeV[i]->Reset();
    m_SVDClusterTimeMapU[i]->Reset();
    m_SVDClusterTimeMapV[i]->Reset();
  }
  for (int i = 0; i < c_nTBPlanes; i++) {
    m_digitMapUV[i]->Reset();
    m_digitCharge[i]->Reset();
    m_hitMapUV[i]->Reset();
    m_clusterCharge[i]->Reset();
    m_seed[i]->Reset();
  }
  for (int i = 0; i < c_nTBPlanes - 1; i++) {
    m_CorrNeighboreYu[i]->Reset();
    m_CorrNeighboreZv[i]->Reset();
  }
  for (int i = 0; i < c_nTBPlanes; i++) {
    m_nEventsPlane[i] = 0;
    m_StatEverageOccupancy[2 * i + 0] = 0;
    m_StatEverageOccupancy[2 * i + 1] = 0;
  }
  m_StatAllEvents = 0;
  m_StatSelEvents = 0;
}


void MaskingModule::event()
{
  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);

  const StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits, m_relSVDClusterDigitName);
  RelationIndex < SVDDigit, SVDCluster > relSVDDigitCluster(DataStore::relationName(
                                                              DataStore::arrayName<SVDDigit>(""),
                                                              DataStore::arrayName<SVDCluster>(""))
                                                           );

  const StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);
  const StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);
  const RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits, m_relTelClusterDigitName);

  const StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  StoreObjPtr<EventMetaData> storeEventMetaData;

  if (abs(storeEventMetaData->getEvent()) > m_nEventsProcess) {
    storeEventMetaData->setEndOfData();
  }
  m_StatAllEvents++;
  //if (CallFullTrackFilter(&storePXDClusters, &storeSVDClusters, &storeTelClusters) == 0) return;
  m_StatSelEvents++;

  m_nRealEventsProcess++;
  // **************** START: masking with correlation between neighbore planes **********************
  int* isMaskedPXD;
  int* isMaskedSVD;
  int* isMaskedTel;
  isMaskedPXD = new int[storePXDClusters.getEntries()];
  isMaskedSVD = new int[storeSVDClusters.getEntries()];
  isMaskedTel = new int[storeTelClusters.getEntries()];

  for (int i = 0; i < storePXDClusters.getEntries(); i++) {
    isMaskedPXD[i] = 0;
  }
  for (int i = 0; i < storeSVDClusters.getEntries(); i++) {
    isMaskedSVD[i] = 0;
  }
  for (int i = 0; i < storeTelClusters.getEntries(); i++) {
    isMaskedTel[i] = 0;
  }

  if (m_SVDStrongMasking == 2) {
    int iPlane1 = -1;
    int iPlane1s = -1;
    for (int i1 = 0; i1 < storeTelClusters.getEntries() + storeSVDClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
      PXDCluster clusterTel1;
      PXDCluster clusterPXD1;
      SVDCluster clusterSVD1;
      int iOrdPXD = i1;
      int iOrdSVD = i1 - storePXDClusters.getEntries();
      //int iOrdTel = i1 - storePXDClusters.getEntries() - storeTelClusters.getEntries();
      if (i1 < storePXDClusters.getEntries()) {                                        // PXD clusters:
        const PXDCluster& clusterPXD = *storePXDClusters[i1];
        if (CallCorrelation(&storePXDClusters, &storeSVDClusters, &storeTelClusters, &clusterPXD) == 1) {
          //printf("CallCor PXD\n");
        }
        clusterPXD1 = clusterPXD;
        iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
        iPlane1 = c_nTelPlanes / 2 - 1 + iPlane1;
      } else if (i1 < storePXDClusters.getEntries() + storeSVDClusters.getEntries()) { // SVD clusters:
        const SVDCluster& clusterSVD = *storeSVDClusters[i1 - storePXDClusters.getEntries()];
        if (CallCorrelation(&storePXDClusters, &storeSVDClusters, &storeTelClusters, &clusterSVD) == 1) {
          //printf("CallCor SVD\n");
        }
        clusterSVD1 = clusterSVD;
        if (fabs(clusterSVD1.getClsTime() - (int)clusterSVD1.getClsTime()) < 0.000001) continue;
        iPlane1 = clusterSVD1.getSensorID().getLayerNumber();
        iPlane1 = c_nTelPlanes / 2 - 1 + iPlane1;
      } else {                                                                         // Tel clusters:
        const PXDCluster& clusterTel = *storeTelClusters[i1 - storePXDClusters.getEntries() - storeSVDClusters.getEntries()];
        if (CallCorrelation(&storePXDClusters, &storeSVDClusters, &storeTelClusters, &clusterTel) == 1) {
          //printf("CallCor Tel\n");
        }
        clusterTel1 = clusterTel;
        iPlane1 = clusterTel1.getSensorID().getLayerNumber();
        iPlane1s = clusterTel1.getSensorID().getSensorNumber();
        iPlane1 = iPlane1s - 1;
        if (iPlane1 > 2) iPlane1 += c_lastSVDPlane;
      }
      if (iPlane1 < c_nTelPlanes / 2 - 1) {                             // 0,1: Tel 0-1,1-2
        int iOrd = 0;
        for (const PXDCluster& clusterTel2 : storeTelClusters) {
          iOrd++;
          int iPlane2s = clusterTel2.getSensorID().getSensorNumber();
          int iPlane2 = iPlane2s - 1;
          if (iPlane2 > 2) iPlane2 += c_lastSVDPlane;
          if (iPlane2 == iPlane1 + 1) {
            CallCorrelationPXDPXD(&clusterTel1, &clusterTel2, iPlane1, iPlane2);
          }
        }
      } else if (iPlane1 == c_nTelPlanes / 2 - 1) {                     // 2:   Tel 2 - PXD 2
        int iOrd = 0;
        for (const PXDCluster& clusterPXD2 : storePXDClusters) {
          iOrd++;
          int iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
          CallCorrelationPXDPXD(&clusterTel1, &clusterPXD2, iPlane1, iPlane2);
        }
      } else if (iPlane1 == c_nTelPlanes / 2) {                         // 3:   skip, no PXD 1
      } else if (iPlane1 == c_nTelPlanes / 2 + c_lastPXDPlane - 1) {    // 4:   PXD 2 - SVD 3
        int iOrd = 0;
        for (const SVDCluster& clusterSVD2 : storeSVDClusters) {
          iOrd++;
          int iPlane2 = clusterSVD2.getSensorID().getLayerNumber();
          iPlane2 += c_nTelPlanes / 2 - 1;
          int imask = 0;
          if (iPlane2 == iPlane1 + 1) {
            imask = CallCorrelationPXDSVD(&clusterPXD1, &clusterSVD2, iPlane1, iPlane2);
            if (imask == 1) {   // some parameter of pixel/strip is out of ROI, so mask both from correlation
              isMaskedSVD[iOrd - 1] = 1;
              isMaskedPXD[iOrdPXD] = 1;
            } else {            // good for next processing, fill histos
              m_clusterCharge[iPlane1]->Fill(clusterPXD1.getCharge());
              m_seed[iPlane1]->Fill(clusterPXD1.getSeedCharge());
              TVector3 rLocal1(clusterPXD1.getU(), clusterPXD1.getV(), 0);
              TVector3 rGlobal1 = getInfoPXD(iPlane1 - c_nTelPlanes / 2).pointToGlobal(rLocal1);
              m_hitMapUV[iPlane1]->Fill(
                (float)getInfoPXD(iPlane1 - 3).getUCellID(clusterPXD1.getU()),
                (float)getInfoPXD(iPlane1 - 3).getVCellID(clusterPXD1.getV()));
              if (clusterSVD2.isUCluster()) {  //  axis U
                TVector3 rLocal2(clusterSVD2.getPosition(), 0, 0);
                TVector3 rGlobal2 = getInfoSVD(iPlane2 - c_nTelPlanes / 2).pointToGlobal(rLocal2);
                m_CorrNeighboreYu[iPlane1]->Fill(rGlobal1.Y(), rGlobal2.Y());
              } else {                         //  axis V
                TVector3 rLocal2(0, clusterSVD2.getPosition(), 0);
                TVector3 rGlobal2 = getInfoSVD(iPlane2 - c_nTelPlanes / 2).pointToGlobal(rLocal2);
                m_CorrNeighboreZv[iPlane1]->Fill(rGlobal1.Z(), rGlobal2.Z());
              }
            }
          }
        }
      } else if (iPlane1 < c_nTelPlanes / 2 + c_lastSVDPlane - 1) {     // 5,6,7: SVD 3-4,4-5,5-6
        int iOrd = 0;
        for (const SVDCluster& clusterSVD2 : storeSVDClusters) {
          iOrd++;
          if (fabs(clusterSVD2.getClsTime() - (int)clusterSVD2.getClsTime()) < 0.000001) continue;
          if (clusterSVD1.isUCluster() != clusterSVD2.isUCluster()) continue;  // different axes
          int iPlane2 = clusterSVD2.getSensorID().getLayerNumber();
          iPlane2 += c_nTelPlanes / 2 - 1;
          int imask = 0;
          if (iPlane2 == iPlane1 + 1) {  // with following plane
            imask = CallCorrelationSVDSVD(&clusterSVD1, &clusterSVD2, iPlane1, iPlane2);
            if (imask == 1) {   // some parameter of pixel/strip is out of ROI, so mask both from correlation
              isMaskedSVD[iOrd - 1] = 1;
              isMaskedSVD[iOrdSVD] = 1;
            } else {            // good for next processing, fill histos
              // / *
              m_clusterCharge[iPlane1]->Fill(clusterSVD1.getCharge());
              m_seed[iPlane1]->Fill(clusterSVD1.getSeedCharge());
              if (clusterSVD1.isUCluster()) {  //  axis U
                TVector3 rLocal1(clusterSVD1.getPosition(), 0, 0);
                TVector3 rGlobal1 = getInfoSVD(iPlane1 - c_nTelPlanes / 2).pointToGlobal(rLocal1);
                TVector3 rLocal2(clusterSVD2.getPosition(), 0, 0);
                TVector3 rGlobal2 = getInfoSVD(iPlane2 - c_nTelPlanes / 2).pointToGlobal(rLocal2);
                m_CorrNeighboreYu[iPlane1]->Fill(rGlobal1.Y(), rGlobal2.Y());

                m_SVDClusterTimeU[iPlane1 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                  (float)getInfoSVD(iPlane1 - 3).getUCellID(clusterSVD1.getPosition()),
                  (float)clusterSVD1.getClsTime());
                m_SVDClusterTimeMapU[iPlane1 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                  (float)clusterSVD1.getClsTime());
                m_hitMapUV[iPlane1]->Fill(
                  (float)getInfoSVD(iPlane1 - 3).getUCellID(clusterSVD1.getPosition()),
                  0.0);
                if (iPlane2 == c_nTelPlanes / 2 + c_lastSVDPlane - 1) {  // add last SVD plane:
                  m_SVDClusterTimeU[iPlane2 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                    (float)getInfoSVD(iPlane2 - 3).getUCellID(clusterSVD2.getPosition()),
                    (float)clusterSVD2.getClsTime());
                  m_SVDClusterTimeMapU[iPlane2 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                    (float)clusterSVD2.getClsTime());
                  m_hitMapUV[iPlane2]->Fill(
                    (float)getInfoSVD(iPlane2 - 3).getUCellID(clusterSVD2.getPosition()),
                    0.0);
                }
              } else {                         //  axis V
                TVector3 rLocal1(0, clusterSVD1.getPosition(), 0);
                TVector3 rGlobal1 = getInfoSVD(iPlane1 - c_nTelPlanes / 2).pointToGlobal(rLocal1);
                TVector3 rLocal2(0, clusterSVD2.getPosition(), 0);
                TVector3 rGlobal2 = getInfoSVD(iPlane2 - c_nTelPlanes / 2).pointToGlobal(rLocal2);
                m_CorrNeighboreZv[iPlane1]->Fill(rGlobal1.Z(), rGlobal2.Z());

                m_SVDClusterTimeV[iPlane1 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                  (float)getInfoSVD(iPlane1 - 3).getVCellID(clusterSVD1.getPosition()),
                  (float)clusterSVD1.getClsTime());
                m_SVDClusterTimeMapV[iPlane1 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                  (float)clusterSVD1.getClsTime());
                m_hitMapUV[iPlane1]->Fill(
                  0.0, (float)getInfoSVD(iPlane1 - 3).getVCellID(clusterSVD1.getPosition()));
                if (iPlane2 == c_nTelPlanes / 2 + c_lastSVDPlane - 1) {  // add last SVD plane:
                  m_SVDClusterTimeV[iPlane2 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                    (float)getInfoSVD(iPlane2 - 3).getVCellID(clusterSVD2.getPosition()),
                    (float)clusterSVD2.getClsTime());
                  m_SVDClusterTimeMapV[iPlane2 - c_lastPXDPlane - c_nTelPlanes / 2]->Fill(
                    (float)clusterSVD2.getClsTime());
                  m_hitMapUV[iPlane2]->Fill(
                    0.0, (float)getInfoSVD(iPlane2 - 3).getUCellID(clusterSVD2.getPosition()));
                }
              }
              // * /
            }
          }
        }
      } else if (iPlane1 == c_nTelPlanes / 2 + c_lastSVDPlane - 1) {    // 8:   SVD 6 - Tel 3
        int iOrd = 0;
        for (const PXDCluster& clusterTel2 : storeTelClusters) {
          iOrd++;
          int iPlane2s = clusterTel2.getSensorID().getSensorNumber();
          int iPlane2 = iPlane2s - 1;
          if (iPlane2 > 2) iPlane2 += c_lastSVDPlane;
          if (iPlane2 == iPlane1 + 1)
            CallCorrelationPXDSVD(&clusterTel2, &clusterSVD1, iPlane2, iPlane1);
        }
      } else if (iPlane1 < c_lastTBPlane - 1) {                         // 9,10: Tel 3-4,4-5
        int iOrd = 0;
        for (const PXDCluster& clusterTel2 : storeTelClusters) {
          iOrd++;
          int iPlane2s = clusterTel2.getSensorID().getSensorNumber();
          int iPlane2 = iPlane2s - 1;
          if (iPlane2 > 2) iPlane2 += c_lastSVDPlane;
          if (iPlane2 == iPlane1 + 1) {
            if (CallCorrelationPXDPXD(&clusterTel1, &clusterTel2, iPlane1, iPlane2)) {
//              TVector3 rLocal1(cluster1.getPosition(), 0, 0);
//              TVector3 rGlobal1 = getInfoSVD(index1).pointToGlobal(rLocal1);
//              TVector3 rLocal2(cluster2.getPosition(), 0, 0);
//              TVector3 rGlobal2 = getInfoSVD(index1).pointToGlobal(rLocal2);
//              m_CorrNeighboreYu[iPlane1]->Fill(2,3);
            }
          }
        }
      }
    }
  }
  // **************** END: masking with correlation between neighbore planes ************************
  // **************** PXD masking histograms **********************
  // If there are no PXD pixels, leave PXD histograms
  if (storePXDDigits && storePXDDigits.getEntries()) {
    // Digits: hitmap, charge
    int iShow = 0;
    int iDigCount = 0;
    for (const PXDDigit& digit : storePXDDigits) {
      iDigCount++;
      int iPlane = digit.getSensorID().getLayerNumber();
      if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
      int index = planeToIndexPXD(iPlane);
      // START: look for corresponding cluster:
      //printf("----> look for corresponding cluster: PXDdig %i\n", iDigCount);
//      int idOfDigit = digit.getArrayIndex();
      int idOfDigit = iDigCount - 1;
      int idOfCluster = -1;
      //printf("kuk1\n");
//      for (auto rel : relPXDClusterDigits) {
      for (int index = 0; index < relPXDClusterDigits.getEntries(); index++) {
//printf("kuk2 %i %i %i %i\n",iDigCount,idOfDigit,relPXDClusterDigits[index].getFromIndex(),relPXDClusterDigits[index].getToIndex());
        int iii = relPXDClusterDigits[index].getToIndex();
        if (iii == idOfDigit) {
//        if (rel.getToIndex() == idOfDigit) {
//          printf("kuk3\n");
//          idOfCluster = rel.getFromIndex();
          idOfCluster = relPXDClusterDigits[index].getFromIndex();
//          printf("     ---->%i %i %i\n",iDigCount,relPXDClusterDigits[index].getFromIndex(),relPXDClusterDigits[index].getToIndex());
          break;
        }
      }
//      printf("----> kuk1 %i %i\n", iPlane, index);
      if (idOfCluster != -1) printf("                ---->Dig %i Cls %i DigSig %3.0f (%i) ClChar %3.0f (%5.2f)\n", iDigCount, idOfCluster,
                                      digit.getCharge(), digit.getUCellID(), storePXDClusters[idOfCluster]->getCharge(), storePXDClusters[idOfCluster]->getU());
      //digit.getArrayIndex());
      // END: look for corresponding cluster:
      m_nEventsPlane[index]++;
//      printf("----> kuk1 %i %s\n", index, m_digitCharge[index]->GetTitle());
      m_digitCharge[index]->Fill(digit.getCharge());
      m_digitMapUV[index]->Fill(
        digit.getUCellID(),
        digit.getVCellID()
      );
    }
    // Clusters: hitmaps, charge, seed
    for (const PXDCluster& cluster : storePXDClusters) {
      int iPlane = cluster.getSensorID().getLayerNumber();
      if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
      int index = planeToIndexPXD(iPlane);
      if (getInfoPXD(index).getUCellID(cluster.getU()) < -1) iShow = 1;
      else iShow = 0;
      if (iShow == 22) {
        printf("--us--------> %f\n", (float)cluster.getUSize());
        printf("--vs--------> %f\n", (float)cluster.getVSize());
        printf("--u---------> %f\n", cluster.getU());
        printf("--v---------> %f\n", cluster.getV());
        printf("----u-------> %i\n", getInfoPXD(index).getUCellID(cluster.getU()));
        printf("----v-------> %i\n", getInfoPXD(index).getVCellID(cluster.getV()));
        //string sTelDigits = str(format("TelDigits"));
        //const std::string sss = "TelClusters";
//  StoreArray<TelDigit> cluster_digits = cluster.getRelationsTo(sss);
//  StoreArray<TelDigit> cluster_digits = cluster.getRelationsTo(storeTelDigits.getName());
        RelationVector<TelDigit> relatedDigits = cluster.getRelationsTo<TelDigit>("TelDigits");
        int nDigits = relatedDigits.size();
        printf("Digits: clus charge: %i, pocet: %i clusters: %i :", (int)cluster.getCharge(), nDigits, storeTelClusters.getEntries());
//for (TelDigit & digit : cluster.getRelationsTo<TelDigit>("TelClustersToTelDigits")) {
        for (TelDigit& digit : relatedDigits) {
//for (TelDigit & digit : cluster.getRelationsTo<TelDigit>("TelDigits")) {
          printf(" %i %i %i<", (int)digit.getCharge(), (int)digit.getUCellID(), (int)digit.getVCellID());
        }
        printf("\n");
//  for (const TelDigit & digit : cluster_digits) {
//    printf(" %i %i<",digit.getUCellID(),digit.getVCellID());
//  }
        //for digit in cluster_digits:
      }
      if ((cluster.getSeedCharge() >= m_PXDCutSeedL) && (cluster.getSeedCharge() <= m_PXDCutSeedH))
        m_hitMapUV[index]->Fill(
          getInfoPXD(index).getUCellID(cluster.getU()),
          getInfoPXD(index).getVCellID(cluster.getV())
        );
      m_clusterCharge[index]->Fill(cluster.getCharge());
      m_seed[index]->Fill(cluster.getSeedCharge());
    }

  }
  // ************ End PXD masking histograms **********************
  // **************** SVD masking histograms **********************
  // If there are no SVD pixels, leave SVD histograms
  int IsCluster = 0;
  int iiMax = 2;
  if (storeSVDDigits && storeSVDDigits.getEntries()) {
    // Digits: hitmap, charge
    int iStoreStrip[6];
    int iStoreCharge[6];
    int iUStrip[6];
    int indexx = 100;
    for (const SVDDigit& digit : storeSVDDigits) {


//      SVDCluster RelClusters = relSVDDigitCluster.getElementsFrom(*storeSVDClusters);
//      RelationVector<SVDCluster> relatedClusters = digit.getRelationsFrom<SVDCluster>("SVDClusters");
//      for (SVDCluster & cluster : relatedClusters) {  // go over clusters
//        printf("---------> kuk relatedClusters\n");
//      }




      int iPlane = digit.getSensorID().getLayerNumber();
      if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
      int index = planeToIndexSVD(iPlane);
      m_nEventsPlane[index]++;
      if (indexx > (int)digit.getIndex()) {  // reset buffer in new strip
        for (int ii = 0; ii < 6; ++ii) {
          iStoreStrip[ii] = 0;
          iStoreCharge[ii] = 0;
          iUStrip[ii] = 0;
        }
      }
      indexx = (int)digit.getIndex();
      if ((indexx < 0) || (indexx > 5)) indexx = 0;
      iStoreStrip[indexx] = (int)digit.getCellID();
      iStoreCharge[indexx] = (int)digit.getCharge();
      iUStrip[indexx] = (int)digit.isUStrip();
      IsCluster = 0;
      if (indexx == 5) { // eval cluster existence:
        if ((iStoreCharge[0] < iStoreCharge[1]) &&
            (iStoreCharge[1] < iStoreCharge[2]) &&
            (iStoreCharge[2] > iStoreCharge[3]) &&
            (iStoreCharge[3] > iStoreCharge[4]) &&
            (iStoreCharge[4] > iStoreCharge[5]) &&
            (iUStrip[0] == iUStrip[1]) &&
            (iUStrip[1] == iUStrip[2]) &&
            (iUStrip[2] == iUStrip[3]) &&
            (iUStrip[3] == iUStrip[4]) &&
            (iUStrip[4] == iUStrip[5]) &&
            (iStoreStrip[0] == iStoreStrip[1]) &&
            (iStoreStrip[1] == iStoreStrip[2]) &&
            (iStoreStrip[2] == iStoreStrip[3]) &&
            (iStoreStrip[3] == iStoreStrip[4]) &&
            (iStoreStrip[4] == iStoreStrip[5])
           )
          IsCluster = 1;
      }
      if (IsCluster) {  // only for good candidates for seed
        for (int ii = 0; ii < 6; ++ii) {
          if (iStoreCharge[ii] > 0) {
            if (digit.isUStrip()) {   // u strip
              m_SVDHitMapU[index - c_lastPXDPlane]->Fill(iStoreStrip[ii], ii);
              m_SVDAvrChargeU[index - c_lastPXDPlane]->Fill(iStoreStrip[ii], ii, iStoreCharge[ii]);
              if (ii == iiMax) m_digitMapUV[index]->Fill((float)iStoreStrip[ii], 0.0);
              if (ii == iiMax) m_digitCharge[index]->Fill(iStoreCharge[ii]);
            } else {                  // v strip
              m_SVDHitMapV[index - c_lastPXDPlane]->Fill(iStoreStrip[ii], ii);
              m_SVDAvrChargeV[index - c_lastPXDPlane]->Fill(iStoreStrip[ii], ii, iStoreCharge[ii]);
              if (ii == iiMax) m_digitMapUV[index]->Fill(0.0, (float)iStoreStrip[ii]);
              if (ii == iiMax) m_digitCharge[index]->Fill(iStoreCharge[ii]);
            }
          }
        }
      }
    }
    // Clusters: hitmaps, charge, seed
    int iOrd = 0;
    for (const SVDCluster& cluster : storeSVDClusters) {
//      printf("---------> kuk SVDClusterIndex %i %i\n",cluster.getArrayIndex(), iOrd);
      if (fabs(cluster.getClsTime() - (int)cluster.getClsTime()) < 0.000001) continue;
      if (isMaskedSVD[iOrd]) continue;
//      printf("---------> kuk SVDClusterIndex %i\n",cluster.getArrayIndex());
      iOrd++;
      int iPlane = cluster.getSensorID().getLayerNumber();
      if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
      //int index = planeToIndexSVD(iPlane);
      if (m_SVDStrongMasking == 1) if (!CallSVDFilter(&cluster)) continue;
      /*
            if (cluster.isUCluster()) {
              m_SVDClusterTimeU[index - c_lastPXDPlane]->Fill(
                (float)getInfoSVD(index).getUCellID(cluster.getPosition()),
                (float)cluster.getClsTime()
              );
              printf("-------------> kuk! u\n");
              m_SVDClusterTimeMapU[index - c_lastPXDPlane]->Fill(
                (float)cluster.getClsTime()
              );
              m_hitMapUV[index]->Fill(
                (float)getInfoSVD(index).getUCellID(cluster.getPosition()),
                0.0
              );
            } else {
              m_SVDClusterTimeV[index - c_lastPXDPlane]->Fill(
                (float)getInfoSVD(index).getVCellID(cluster.getPosition()),
                (float)cluster.getClsTime()
              );
              printf("-------------> kuk! v\n");
              m_SVDClusterTimeMapV[index - c_lastPXDPlane]->Fill(
                (float)cluster.getClsTime()
              );
              m_hitMapUV[index]->Fill(
                0.0,
                (float)getInfoSVD(index).getVCellID(cluster.getPosition())
              );
            }
            m_clusterCharge[index]->Fill(cluster.getCharge());
            m_seed[index]->Fill(cluster.getSeedCharge());
      */
    }
  }
  // ************ End SVD masking histograms **********************
  // **************** Tel masking histograms **********************
  // Clusters: hitmaps, charge, seed
  int iShow = 0;
  for (const PXDCluster& cluster : storeTelClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    int iPlanes = cluster.getSensorID().getSensorNumber();
//printf("%i ", iPlanes);
    if (iPlane != 7) continue;
    if ((iPlanes < 1) || (iPlanes > 6)) continue;
//      if ((iPlane < c_firstTelPlane) || (iPlane > c_lastTelPlane)) continue;
    int index = planeToIndexTel(iPlanes) + 6;
    if (getInfoTel(index - 6).getUCellID(cluster.getU()) < -1) iShow = 1;
    else iShow = 0;
    if (iShow == 22) {
      printf("--us--------> %f\n", (float)cluster.getUSize());
      printf("--vs--------> %f\n", (float)cluster.getVSize());
      printf("--u---------> %f\n", cluster.getU());
      printf("--v---------> %f\n", cluster.getV());
      printf("----u-------> %i\n", getInfoTel(index - 6).getUCellID(cluster.getU()));
      printf("----v-------> %i\n", getInfoTel(index - 6).getVCellID(cluster.getV()));
      //string sTelDigits = str(format("TelDigits"));
      //const std::string sss = "TelClusters";
//  StoreArray<TelDigit> cluster_digits = cluster.getRelationsTo(sss);
//  StoreArray<TelDigit> cluster_digits = cluster.getRelationsTo(storeTelDigits.getName());
      RelationVector<TelDigit> relatedDigits = cluster.getRelationsTo<TelDigit>("TelDigits");
      int nDigits = relatedDigits.size();
      printf("Digits: clus charge: %i, pocet: %i clusters: %i :", (int)cluster.getCharge(), nDigits, storeTelClusters.getEntries());
//for (TelDigit & digit : cluster.getRelationsTo<TelDigit>("TelClustersToTelDigits")) {
      for (TelDigit& digit : relatedDigits) {
//for (TelDigit & digit : cluster.getRelationsTo<TelDigit>("TelDigits")) {
        printf(" %i %i %i<", (int)digit.getCharge(), (int)digit.getUCellID(), (int)digit.getVCellID());
      }
      printf("\n");
//  for (const TelDigit & digit : cluster_digits) {
//    printf(" %i %i<",digit.getUCellID(),digit.getVCellID());
//  }
      //for digit in cluster_digits:
    }
    m_hitMapUV[index]->Fill(
      getInfoTel(index - 6).getUCellID(cluster.getU()),
      getInfoTel(index - 6).getVCellID(cluster.getV())
    );
    m_clusterCharge[index]->Fill(cluster.getCharge());
    m_seed[index]->Fill(cluster.getSeedCharge());
  }
  // If there are no Tel pixels, leave Tel histograms
  if (storeTelDigits && storeTelDigits.getEntries()) {
    // Digits: hitmap, charge
    for (const TelDigit& digit : storeTelDigits) {
      int iPlane = digit.getSensorID().getSensorNumber();
      if ((iPlane < c_firstTelPlane) || (iPlane > c_lastTelPlane)) continue;
      int index = planeToIndexTel(iPlane) + 6;
      m_nEventsPlane[index]++;
      m_digitCharge[index]->Fill(digit.getCharge());
//if (iShow) printf("> %i ",digit.getUCellID());
//if (iShow) printf(" %i<",digit.getVCellID());
      m_digitMapUV[index]->Fill(
        digit.getUCellID(),
        digit.getVCellID()
      );
    }
    /*
    */
  }
  // ************ End Tel masking histograms **********************
  delete isMaskedPXD;
  delete isMaskedSVD;
  delete isMaskedTel;
}

void MaskingModule::endRun()
{
  StoreObjPtr<EventMetaData> storeEventMetaData;
  long unsigned runNo = storeEventMetaData->getRun();
  m_nEventsProcessFraction = (double)m_nRealEventsProcess / m_nEventsProcess;
  // Maskin border for all sensors at 10 000 events!:
  float PXDCut = 22.0;
  float TelCut = 12.0;
//  float SVDCutU[] = {60.0, 60.0, 60.0, 60.0};    // for clusters - old setting
//  float SVDCutV[] = {250.0, 360.0, 400.0, 360.0};// for clusters - old setting
//  float SVDCutUOut[] = {30.0, 30.0, 30.0, 30.0}; // for clusters - old setting
//  float SVDCutVOut[] = {40.0, 40.0, 40.0, 40.0}; // for clusters - old setting
//  float SVDMaskRegUMi[] = {0, 0, 0, 0};          // for clusters - old setting
//  float SVDMaskRegUMa[] = {500, 500, 500, 500};  // for clusters - old setting
//  float SVDMaskRegVMi[] = {250, 150, 150, 150};  // for clusters - old setting
//  float SVDMaskRegVMa[] = {490, 370, 370, 400};  // for clusters - old setting

  float SVDCutU[] = {60.0, 60.0, 60.0, 40.0};
  float SVDCutV[] = {160.0, 360.0, 360.0, 360.0};
  float SVDCutUOut[] = {20.0, 20.0, 20.0, 10.0};
  float SVDCutVOut[] = {15.0, 20.0, 20.0, 20.0};
  float SVDMaskRegUMi[] = {0, 0, 0, 0};
  float SVDMaskRegUMa[] = {500, 500, 500, 500};
  float SVDMaskRegVMi[] = {300, 20, 200, 200};
  float SVDMaskRegVMa[] = {500, 350, 350, 350};

  // correction for unmerged events and different No. of proces events:
  PXDCut *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;
  for (int i = 0; i < c_nSVDPlanes; ++i) {
    SVDCutU[i] *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;
    SVDCutV[i] *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;
    if (runNo == 470) {  //OK
      SVDCutU[i] *= 1.999;
      SVDCutV[i] *= 1.1;
    } else if (runNo == 473) { //OK
      SVDCutU[i] *= 1.999;
      SVDCutV[i] *= 1.2;
    } else if (runNo == 479) { //OK
      SVDCutU[i] *= 0.999;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 509) { //OK
      SVDCutU[i] *= 0.999;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 510) { //OK
      SVDCutU[i] *= 0.999;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 511) { //OK
      SVDCutU[i] *= 0.999;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 517) { //OK
      SVDCutU[i] *= 1.5;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 561) { //OK
      SVDCutU[i] *= 2.5;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 562) { //OK
      SVDCutU[i] *= 2.5;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 577) { //OK
      SVDCutU[i] *= 2.9;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 607) {  //OK
      SVDCutU[i] *= 2.999;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 636) { //OK
      SVDCutU[i] *= 3.0;
      SVDCutV[i] *= 0.999;
    } else if (runNo == 642) { //OK
      SVDCutU[i] *= 3.0;
      SVDCutV[i] *= 0.999;
    } else {
      SVDCutU[i] *= 3.0;
      SVDCutV[i] *= 1.5;
      SVDCutUOut[i] *= 3.0;
      SVDCutVOut[i] *= 3.0;
    }
  }
  TelCut *= m_nEventsProcessFraction * m_nEventsProcess / 10000.0;

  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_SVDAvrChargeU[i]->Divide(m_SVDHitMapU[i]);
    m_SVDAvrChargeV[i]->Divide(m_SVDHitMapV[i]);
  }


  FILE* MaskList;
  B2INFO("Start to create masking");
  printf("Start to create masking from %i events (fraction: %6.3f)\n", (int)m_nRealEventsProcess, m_nEventsProcessFraction);
  for (int i = 0; i < c_nPXDPlanes; i++) {
    if (m_nEventsPlane[i]) {
      int nMasked = 0;
      int iPlane = indexToPlanePXD(i);
      int iLayer = getInfoPXD(i).getID().getLayerNumber();
      int iLadder = getInfoPXD(i).getID().getLadderNumber();
      int iSensor = getInfoPXD(i).getID().getSensorNumber();
      string FileName = str(format("%3%PXD%1%_MaskListRun%2%.xml") % iPlane % runNo % m_MaskDirectoryPath);
      std::string m_ignoredPixelsListName = str(format("../../../..%1%") % FileName);
//      ifstream ifile(m_ignoredPixelsListName.data());
//      if (ifile) {
//        std::string m_ignoredPixelsListName = str(format("%1%") % FileName);
      std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
          m_ignoredPixelsListName));
//      }
//      for (auto pixel : m_ignoredPixelsList->getIgnoredPixels(VxdID(iLayer, iLadder, iSensor))) {
//        unsigned int u = (unsigned int)(pixel.first);
//        unsigned int v = (unsigned int)(pixel.second);
//      }

      MaskList = fopen(FileName.data(), "w");
      fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      fprintf(MaskList, "<Meta>\n");
      fprintf(MaskList, "    <Date>22.01.2014</Date>\n");
      fprintf(MaskList, "    <Description short=\"Ignore pixel list for PXD plane(s) in 2014 VXD beam test\">\n");
      fprintf(MaskList, "        Crude initial list of bad pixels\n");
      fprintf(MaskList, "    </Description>\n");
      fprintf(MaskList, "    <Author>Peter Kvasnicka</Author>\n");
      fprintf(MaskList, "</Meta>\n");
      fprintf(MaskList, "<PXD>\n");
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
      for (int i1 = 0; i1 < m_digitMapUV[i]->GetNbinsX(); ++i1) {
        for (int i2 = 0; i2 < m_digitMapUV[i]->GetNbinsY(); ++i2) {
          int ExistMask = 0;
          if (m_AppendMaskFile) {
            if (!m_ignoredPixelsList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
              ExistMask = 1;
            }
          }
          if (ExistMask || (m_digitMapUV[i]->GetBinContent(i1 + 1, i2 + 1) > PXDCut)) {
            fprintf(MaskList, "        <pixels uStart = \"%04i\" vStart = \"%04i\"></pixels>\n", i1, i2);
            nMasked++;
          }
        }
      }
      fprintf(MaskList, "\n");
      fprintf(MaskList, "      </sensor>\n");
      fprintf(MaskList, "    </ladder>\n");
      fprintf(MaskList, "  </layer>\n");
      fprintf(MaskList, "</PXD>\n");
      fclose(MaskList);
      B2INFO("PXD(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMasked << " pixels in: " <<
             m_ignoredPixelsListName.data());
      printf("PXD(%i,%i,%i) masked %i pixels in: %s\n", iLayer, iLadder, iSensor, nMasked, m_ignoredPixelsListName.data());
    }
  }

  for (int i = 0; i < c_nSVDPlanes; i++) {
    if (m_nEventsPlane[i + 2]) {
      int nMaskedU = 0;
      int nMaskedV = 0;
      int iPlane = indexToPlaneSVD(i + 2);
      int iLayer = getInfoSVD(i + 2).getID().getLayerNumber();
      int iLadder = getInfoSVD(i + 2).getID().getLadderNumber();
      int iSensor = getInfoSVD(i + 2).getID().getSensorNumber();
      string FileName = str(format("%3%SVD%1%_MaskListRun%2%.xml") % iPlane % runNo % m_MaskDirectoryPath);
      std::string m_ignoredPixelsListName = str(format("../../../..%1%") % FileName);
//      std::string m_ignoredPixelsListName = str(format("%1%") % FileName);
      std::unique_ptr<SVDIgnoredStripsMap> m_ignoredPixelsList = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(
          m_ignoredPixelsListName));
      MaskList = fopen(FileName.data(), "w");
      fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      fprintf(MaskList, "<Meta>\n");
      fprintf(MaskList, "    <Date>22.01.2014</Date>\n");
      fprintf(MaskList, "    <Description short=\"Ignore strip list for SVD planes in 2014 VXD beam test\">\n");
      fprintf(MaskList, "        Crude initial list of bad strips\n");
      fprintf(MaskList, "    </Description>\n");
      fprintf(MaskList, "    <Author>Peter Kvasnicka</Author>\n");
      fprintf(MaskList, "</Meta>\n");
      fprintf(MaskList, "<SVD>\n");
      fprintf(MaskList, "    <layer n=\"%i\">\n", iLayer);
      fprintf(MaskList, "        <ladder n=\"%i\">\n", iLadder);
      fprintf(MaskList, "            <sensor n=\"%i\">\n", iSensor);
      fprintf(MaskList, "                <side side=\"v\">\n");
      fprintf(MaskList, "                    <stripsFromTo fromStrip = \"620\" toStrip = \"767\"></stripsFromTo>\n");
      fprintf(MaskList, "                    <!-- Individual strips can be masked, too -->\n");
//      for (int i1 = 0; i1 < m_hitMapUV[i+2]->GetNbinsX(); ++i1) {
      for (int i1 = 0; i1 < m_digitMapUV[i + 2]->GetNbinsX(); ++i1) {
        int ExistMask = 0;
        if (m_AppendMaskFile == 22) {
          if (!m_ignoredPixelsList->stripOK(VxdID(iLayer, iLadder, iSensor, 1), (unsigned short) i1)) {
            ExistMask = 1;
          }
        }
        float SVDCut = SVDCutUOut[i];
        if ((i1 > SVDMaskRegUMi[i]) && (i1 < SVDMaskRegUMa[i]))
          SVDCut = SVDCutU[i];
        //if (ExistMask || (m_hitMapUV[i+2]->GetBinContent(i1+1, 1) > SVDCut)) {
        if (ExistMask || (m_digitMapUV[i + 2]->GetBinContent(i1 + 1, 1) > SVDCut)) {
          fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i1);
          nMaskedU++;
        }
      }
      fprintf(MaskList, "                </side>\n");
      fprintf(MaskList, "                <side side=\"u\">\n");
//      for (int i2 = 0; i2 < m_hitMapUV[i+2]->GetNbinsY(); ++i2) {
      for (int i2 = 0; i2 < m_digitMapUV[i + 2]->GetNbinsY(); ++i2) {
        int ExistMask = 0;
        if (m_AppendMaskFile == 22) {
          if (!m_ignoredPixelsList->stripOK(VxdID(iLayer, iLadder, iSensor, 0), (unsigned short) i2)) {
            ExistMask = 1;
          }
        }
        float SVDCut = SVDCutVOut[i];
        if ((i2 > SVDMaskRegVMi[i]) && (i2 < SVDMaskRegVMa[i]))
          SVDCut = SVDCutV[i];
        //if (ExistMask || (m_hitMapUV[i+2]->GetBinContent(1, i2+1) > SVDCut)) {
        if (ExistMask || (m_digitMapUV[i + 2]->GetBinContent(1, i2 + 1) > SVDCut)) {

          fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i2);
          nMaskedV++;
        }
      }
      fprintf(MaskList, "                </side>\n");
      fprintf(MaskList, "            </sensor>\n");
      fprintf(MaskList, "        </ladder>\n");
      fprintf(MaskList, "    </layer>\n");
      fprintf(MaskList, "</SVD>\n");
      fclose(MaskList);
      B2INFO("SVD(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMaskedU << " U strips in: " <<
             m_ignoredPixelsListName.data());
      B2INFO("SVD(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMaskedV << " V strips in: " <<
             m_ignoredPixelsListName.data());
      printf("SVD(%i,%i,%i) masked %i U strips in: %s\n", iLayer, iLadder, iSensor, nMaskedU, m_ignoredPixelsListName.data());
      printf("SVD(%i,%i,%i) masked %i V strips in: %s\n", iLayer, iLadder, iSensor, nMaskedV, m_ignoredPixelsListName.data());
    }
  }

  for (int i = 0; i < c_nTelPlanes; i++) {
    if (m_nEventsPlane[i + 6]) {
      int nMasked = 0;
      int iPlane = i + 1;
      int iLayer = getInfoTel(i).getID().getLayerNumber();
      int iLadder = getInfoTel(i).getID().getLadderNumber();
      int iSensor = getInfoTel(i).getID().getSensorNumber();
      string FileName = str(format("%3%Tel%1%_MaskListRun%2%.xml") % iPlane % runNo % m_MaskDirectoryPath);
      std::string m_ignoredPixelsListName = str(format("../../../..%1%") % FileName);
//      std::string m_ignoredPixelsListName = str(format("%1%") % FileName);
      std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
          m_ignoredPixelsListName));
      MaskList = fopen(FileName.data(), "w");
      fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      fprintf(MaskList, "<Meta>\n");
      fprintf(MaskList, "    <Date>22.01.2014</Date>\n");
      fprintf(MaskList, "    <Description short=\"Ignore pixel list for PXD plane(s) in 2014 VXD beam test\">\n");
      fprintf(MaskList, "        Crude initial list of bad pixels\n");
      fprintf(MaskList, "    </Description>\n");
      fprintf(MaskList, "    <Author>Peter Kvasnicka</Author>\n");
      fprintf(MaskList, "</Meta>\n");
      fprintf(MaskList, "<PXD>\n");
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

      for (int i1 = 0; i1 < m_digitMapUV[i + 6]->GetNbinsX(); ++i1) {
        for (int i2 = 0; i2 < m_digitMapUV[i + 6]->GetNbinsY(); ++i2) {
          int ExistMask = 0;
          if (m_AppendMaskFile) {
            if (!m_ignoredPixelsList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
              ExistMask = 1;
            }
          }
          if (ExistMask || (m_digitMapUV[i + 6]->GetBinContent(i1 + 1, i2 + 1) > TelCut)) {
            fprintf(MaskList, "        <pixels uStart = \"%04i\" vStart = \"%04i\"></pixels>\n", i1, i2);
            nMasked++;
          }
        }
      }
      fprintf(MaskList, "\n");
      fprintf(MaskList, "      </sensor>\n");
      fprintf(MaskList, "    </ladder>\n");
      fprintf(MaskList, "  </layer>\n");
      fprintf(MaskList, "</PXD>\n");
      fclose(MaskList);
      B2INFO("Tel(" << iLayer << "," << iLadder << "," << iSensor << ") masked " << nMasked << " pixels in: " <<
             m_ignoredPixelsListName.data());
      printf("Tel(%i,%i,%i) masked %i pixels in: %s\n", iLayer, iLadder, iSensor, nMasked, m_ignoredPixelsListName.data());
    }
  }
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

int MaskingModule::CallCorrelationPXDPXD(const PXDCluster* clusterPXD1, const PXDCluster* clusterPXD2, int iPlane1, int iPlane2)
{
// ************* START: Filter for correlation between neighbore Tel and Tel ******************
//                                                               Tel and PXD ******************
//                                                               PXD and PXD ******************
//  printf("Correlation PXD-PXD, or PXD-Tel or Tel-Tel %i %i %i\n",iPlane1,(int)clusterPXD1->getCharge(),(int)clusterPXD2->getCharge());
//                m_CorrNeighboreYu[iPlane1]->Fill(2,3);
  TVector3 rLocal1(clusterPXD1->getU(), clusterPXD1->getV(), 0);
  TVector3 rGlobal1;
  if ((iPlane1 >= 3) && (iPlane1 <= 4)) {  // PXD 0,1
    rGlobal1 = getInfoPXD(iPlane1 - 3).pointToGlobal(rLocal1);
  } else if ((iPlane1 < 3)) {                // Tel 0,1,2
    rGlobal1 = getInfoTel(iPlane1).pointToGlobal(rLocal1);
  } else if ((iPlane1 > 8)) {                // Tel 3,4,5
    rGlobal1 = getInfoTel(iPlane1 - 6).pointToGlobal(rLocal1);
  }
  TVector3 rLocal2(clusterPXD2->getU(), clusterPXD2->getV(), 0);
  TVector3 rGlobal2;
  if ((iPlane2 >= 3) && (iPlane2 <= 4)) {  // PXD 0,1
    rGlobal2 = getInfoPXD(iPlane2 - 3).pointToGlobal(rLocal2);
  } else if ((iPlane2 < 3)) {                // Tel 0,1,2
    rGlobal2 = getInfoTel(iPlane2).pointToGlobal(rLocal2);
  } else if ((iPlane2 > 8)) {                // Tel 3,4,5
    rGlobal2 = getInfoTel(iPlane2 - 6).pointToGlobal(rLocal2);
  }
  if ((fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.7) && (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.7)) {
    return 0;
  }
  return 1;
// ************* END: Filter for correlation between neighbore Tel and Tel ********************
}

int MaskingModule::CallCorrelationPXDSVD(const PXDCluster* clusterPXD, const SVDCluster* clusterSVD, int iPlane1, int iPlane2)
{
// ************* START: Filter for correlation between neighbore Tel and SVD ******************
//                                                               PXD and SVD ******************
  //printf("Correlation PXD-SVD, or Tel-SVD %i %i %i\n",iPlane1,(int)clusterPXD->getCharge(),(int)clusterSVD->getCharge());
  TVector3 rLocal1(clusterPXD->getU(), clusterPXD->getV(), 0);
  TVector3 rGlobal1;
  if ((iPlane1 >= 3) && (iPlane1 <= 4)) {  // PXD 0,1
    rGlobal1 = getInfoPXD(iPlane1 - 3).pointToGlobal(rLocal1);
  } else if ((iPlane1 < 3)) {                // Tel 0,1,2
    rGlobal1 = getInfoTel(iPlane1).pointToGlobal(rLocal1);
  } else if ((iPlane1 > 8)) {                // Tel 3,4,5
    rGlobal1 = getInfoTel(iPlane1 - 6).pointToGlobal(rLocal1);
  }
  if (clusterSVD->isUCluster()) {
    TVector3 rLocal2(clusterSVD->getPosition(), 0, 0);
    TVector3 rGlobal2 = getInfoSVD(iPlane2 - 3).pointToGlobal(rLocal2);
    if (fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.7) {
      return 0;
    }
  } else {
    TVector3 rLocal2(0, clusterSVD->getPosition(), 0);
    TVector3 rGlobal2 = getInfoSVD(iPlane2 - 3).pointToGlobal(rLocal2);
    if (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.7) {
      return 0;
    }
  }
  return 1;
// ************* END: Filter for correlation between neighbore Tel and SVD ********************
}

int MaskingModule::CallCorrelationSVDSVD(const SVDCluster* clusterSVD1, const SVDCluster* clusterSVD2, int iPlane1, int iPlane2)
{
// ************* START: Filter for correlation between neighbore SVD and SVD ******************
  //printf("Correlation SVD-SVD %i %i %i\n",iPlane1,(int)clusterSVD1->getCharge(),(int)clusterSVD2->getCharge());
  if (clusterSVD1->isUCluster() != clusterSVD2->isUCluster()) return 1;
  if (clusterSVD1->isUCluster()) {
    TVector3 rLocal1(clusterSVD1->getPosition(), 0, 0);
    TVector3 rGlobal1 = getInfoSVD(iPlane1 - 3).pointToGlobal(rLocal1);
    TVector3 rLocal2(clusterSVD2->getPosition(), 0, 0);
    TVector3 rGlobal2 = getInfoSVD(iPlane2 - 3).pointToGlobal(rLocal2);
    if (fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.3) {
      return 0;
    }
  } else {
    TVector3 rLocal1(0, clusterSVD1->getPosition(), 0);
    TVector3 rGlobal1 = getInfoSVD(iPlane1 - 3).pointToGlobal(rLocal1);
    TVector3 rLocal2(0, clusterSVD2->getPosition(), 0);
    TVector3 rGlobal2 = getInfoSVD(iPlane2 - 3).pointToGlobal(rLocal2);
    if (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.3) {
      return 0;
    }
  }
  return 1;
// ************* END: Filter for correlation between neighbore SVD and SVD ********************
}

int MaskingModule::CallFullTrackFilter(const StoreArray<PXDCluster>* storePXDClusters,
                                       const StoreArray<SVDCluster>* storeSVDClusters, const StoreArray<PXDCluster>* storeTelClusters)
{

  int IsOKPXD = 0;
  int IsOKSVD[8];
  int IsOKTel[3];
  for (int i = 0; i < 8; ++i) IsOKSVD[i] = 0;
  for (int i = 0; i < 3; ++i) IsOKTel[i] = 0;
  float StatEverageOccupancy[c_nTBPlanes * 2];
  for (int i = 0; i < c_nTBPlanes; i++) {
    StatEverageOccupancy[i * 2 + 0] = 0;
    StatEverageOccupancy[i * 2 + 1] = 0;
  }
  for (const PXDCluster& clusterPXD : *storePXDClusters) {             // PXD *********************
    int iPlane = clusterPXD.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int index = planeToIndexPXD(iPlane);
    //float y = getInfoPXD(index).getUCellID(clusterPXD.getU());
    //float z = getInfoPXD(index).getVCellID(clusterPXD.getV());
    TVector3 rLocal(clusterPXD.getU(), clusterPXD.getV(), 0);
    TVector3 rGlobal = getInfoPXD(index).pointToGlobal(rLocal);
    if ((rGlobal.Y() > -0.45) && (rGlobal.Y() < 0.15)) {
//      printf("Tz%f ",rGlobal.Y());
      IsOKPXD += 1;
      StatEverageOccupancy[(iPlane + 2) * 2 + 0]++;
      StatEverageOccupancy[(iPlane + 2) * 2 + 1]++;
    }
  }
  for (const SVDCluster& clusterSVD : *storeSVDClusters) {             // SVD *********************
    int iPlane = clusterSVD.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    // Skip if ubnormal value for ClsTime:
    if (fabs(clusterSVD.getClsTime() - (int)clusterSVD.getClsTime()) < 0.000001) continue;
    if (clusterSVD.isUCluster()) {         // must exist
      IsOKSVD[iPlane - 3] += 1;
      StatEverageOccupancy[(iPlane + 2) * 2 + 0]++;
//printf("u%6.2f ",clusterSVD.getClsTime());
    } else {
      IsOKSVD[4 + iPlane - 3] += 1;
      StatEverageOccupancy[(iPlane + 2) * 2 + 1]++;
//printf("v%6.2f ",clusterSVD.getClsTime());
    }
    //int index = planeToIndexSVD(iPlane);
    //float y = getInfoPXD(index).getUCellID(clusterPXD.getU());
    //float z = getInfoPXD(index).getVCellID(clusterPXD.getV());
    //TVector3 rLocal(clusterPXD.getU(), clusterPXD.getV(), 0);
    //TVector3 rGlobal = getInfoPXD(index).pointToGlobal(rLocal);
    //if ((rGlobal.Y() > -0.45) && (rGlobal.Y() < 0.15)) {
//      printf("Tz%f ",rGlobal.Y());
    //  IsOKSVD[] += 1;
    //}
  }
  for (const PXDCluster& clusterTel : *storeTelClusters) {             // Tel *********************
    int iPlane = clusterTel.getSensorID().getSensorNumber();
    if (iPlane > 3) {
      StatEverageOccupancy[(iPlane + 5) * 2 + 0]++;
      StatEverageOccupancy[(iPlane + 5) * 2 + 1]++;
      continue; // not interesting
    }
    if (iPlane < 3) {         // must exist
      StatEverageOccupancy[(iPlane - 1) * 2 + 0]++;
      StatEverageOccupancy[(iPlane - 1) * 2 + 1]++;
      IsOKTel[iPlane - 1] += 1;
    }
    if (iPlane == 3) {        // must be in range
      int index = planeToIndexTel(iPlane);
      //float y = getInfoTel(index).getUCellID(clusterTel.getU());
      //    float z = getInfoTel(index).getVCellID(clusterTel.getV());
      TVector3 rLocal(clusterTel.getU(), clusterTel.getV(), 0);
      TVector3 rGlobal = getInfoTel(index).pointToGlobal(rLocal);
      if ((rGlobal.Z() > -0.5) && (rGlobal.Z() < 0.5)) {
        //      printf("Tz%f ",rGlobal.Z());
        IsOKTel[iPlane - 1] += 1;
        StatEverageOccupancy[(iPlane - 1) * 2 + 0]++;
        StatEverageOccupancy[(iPlane - 1) * 2 + 1]++;
      }
    }
  }

// cut for No. of clusters per event:
  int iMaxClasters = 999;
  if (IsOKPXD > iMaxClasters) {
    IsOKPXD = 0;
    int ii = 4;
    StatEverageOccupancy[ii * 2 + 0] = 0;
    StatEverageOccupancy[ii * 2 + 1] = 0;
  }
  for (int i = 0; i < 8; ++i) if (IsOKSVD[i] > iMaxClasters) {
      IsOKSVD[i] = 0;
      int ii = (int)(i / 4) + 5;
      StatEverageOccupancy[ii * 2 + 0] = 0;
      StatEverageOccupancy[ii * 2 + 1] = 0;
    }
  for (int i = 0; i < 3; ++i) if (IsOKTel[i] > iMaxClasters) {
      IsOKTel[i] = 0;
      int ii = i;
      StatEverageOccupancy[ii * 2 + 0] = 0;
      StatEverageOccupancy[ii * 2 + 1] = 0;
    }

  for (int i = 0; i < 8; ++i) IsOKSVD[0] *= IsOKSVD[i];
  for (int i = 0; i < 3; ++i) IsOKTel[0] *= IsOKTel[i];
  //printf("--------> %i %i %i\n",IsOKPXD, IsOKSVD[0], IsOKTel[0]);
  if (IsOKPXD * IsOKSVD[0] * IsOKTel[0]) {
    for (int i = 0; i < c_nTBPlanes; i++) {
      m_StatEverageOccupancy[i * 2 + 0] += StatEverageOccupancy[i * 2 + 0];
      m_StatEverageOccupancy[i * 2 + 1] += StatEverageOccupancy[i * 2 + 1];
    }
    return 1;
  }
  return 0;
}

int MaskingModule::CallCorrelation(const StoreArray<PXDCluster>* storePXDClusters,
                                   const StoreArray<SVDCluster>* storeSVDClusters,
                                   const StoreArray<PXDCluster>* storeTelClusters,
                                   const PXDCluster* clusterPXD)
{
// **** return 1: if OK cluster, 0: bad cluster without correlation ****
  int isOK = 0;

  int isPXD = 1;
  int iPlane = clusterPXD->getSensorID().getLayerNumber();
  TVector3 rLocal1(clusterPXD->getU(), clusterPXD->getV(), 0);
  TVector3 rGlobal1;
  if (iPlane == 7) {
    isPXD = 0;
    iPlane = clusterPXD->getSensorID().getSensorNumber();
  }
  int indexTB;
  if (isPXD) {
    //printf("CallCorrelation PXD%i ind%i pln%i ", iPlane, planeVXDToTBIndex(iPlane), indexToTBPlane(planeVXDToTBIndex(iPlane)));
    indexTB = planeVXDToTBIndex(iPlane);
    rGlobal1 = getInfoPXD(planeToIndexPXD(iPlane)).pointToGlobal(rLocal1);
  } else {
    //printf("CallCorrelation Tel%i ind%i pln%i ", iPlane, planeTelToTBIndex(iPlane), indexToTBPlane(planeTelToTBIndex(iPlane)));
    indexTB = planeTelToTBIndex(iPlane);
    rGlobal1 = getInfoTel(planeToIndexTel(iPlane)).pointToGlobal(rLocal1);
  }
  int indexTBprev = indexTB - 1;
  if (indexTBprev == c_MissingPlaneIndex) indexTBprev--;
  int indexTBnext = indexTB + 1;
  if (indexTBnext == c_MissingPlaneIndex) indexTBnext++;
  for (int i = 0; i < 2; ++i) {
    indexTB = indexTBprev;
    if (i) indexTB = indexTBnext;
    if ((indexTB < 0) && (indexTB >= c_nTBPlanes)) continue;
    // selection of set of cluster:
    if (indexTB == planeVXDToTBIndex(2)) {  // PXD:
      for (const PXDCluster& clusterPXD2 : *storePXDClusters) {
        int iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int index2 = planeVXDToTBIndex(iPlane2);
        if (indexTB != index2) continue;
        TVector3 rLocal2(clusterPXD2.getU(), clusterPXD2.getV(), 0);
        TVector3 rGlobal2 = getInfoPXD(planeToIndexPXD(iPlane2)).pointToGlobal(rLocal2);
        if ((fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.5) && (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.5)) isOK = 1;
        if (isOK) break;
      }
    } else if ((indexTB >= planeVXDToTBIndex(c_firstSVDPlane)) && (indexTB <= planeVXDToTBIndex(c_lastSVDPlane))) {  // SVD:
      for (const SVDCluster& clusterSVD2 : *storeSVDClusters) {
        int iPlane2 = clusterSVD2.getSensorID().getLayerNumber();
        int index2 = planeVXDToTBIndex(iPlane2);
        if (indexTB != index2) continue;
        if (fabs(clusterSVD2.getClsTime() - (int)clusterSVD2.getClsTime()) < 0.000001) continue;
        if (clusterSVD2.isUCluster()) {
          TVector3 rLocal2(clusterSVD2.getPosition(), 0, 0);
          TVector3 rGlobal2 = getInfoSVD(planeToIndexSVD(iPlane2)).pointToGlobal(rLocal2);
          if (fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.5) isOK = 1;
        } else {
          TVector3 rLocal2(0, clusterSVD2.getPosition(), 0);
          TVector3 rGlobal2 = getInfoSVD(planeToIndexSVD(iPlane2)).pointToGlobal(rLocal2);
          if (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.5) isOK = 1;
        }
        if (isOK) break;
      }
    } else {                                // Tel:
      for (const PXDCluster& clusterTel2 : *storeTelClusters) {
        int iPlane2 = clusterTel2.getSensorID().getSensorNumber();
        int index2 = planeTelToTBIndex(iPlane2);
        if (indexTB != index2) continue;
        TVector3 rLocal2(clusterTel2.getU(), clusterTel2.getV(), 0);
        TVector3 rGlobal2 = getInfoTel(planeToIndexTel(iPlane2)).pointToGlobal(rLocal2);
        if ((fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.5) && (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.5)) isOK = 1;
        if (isOK) break;
      }
    }
    if (isOK) break;
  }
  return isOK;
}

int MaskingModule::CallCorrelation(const StoreArray<PXDCluster>* storePXDClusters,
                                   const StoreArray<SVDCluster>* storeSVDClusters,
                                   const StoreArray<PXDCluster>* storeTelClusters,
                                   const SVDCluster* clusterSVD)
{
// **** return 1: if OK cluster, 0: bad cluster without correlation ****
  int isOK = 0;

  int iPlane = clusterSVD->getSensorID().getLayerNumber();
  //printf("CallCorrelation SVD%i ind%i pln%i ", iPlane, planeVXDToTBIndex(iPlane), indexToTBPlane(planeVXDToTBIndex(iPlane)));
  TVector3 rLocal1;
  if (clusterSVD->isUCluster()) {
    TVector3 rLocal(clusterSVD->getPosition(), 0, 0);
    rLocal1 = rLocal;
  } else {
    TVector3 rLocal(0, clusterSVD->getPosition(), 0);
    rLocal1 = rLocal;
  }
  TVector3 rGlobal1 = getInfoSVD(planeToIndexSVD(iPlane)).pointToGlobal(rLocal1);
  int indexTB = planeVXDToTBIndex(iPlane);
  int indexTBprev = indexTB - 1;
  if (indexTBprev == c_MissingPlaneIndex) indexTBprev--;
  int indexTBnext = indexTB + 1;
  if (indexTBnext == c_MissingPlaneIndex) indexTBnext++;
  for (int i = 0; i < 2; ++i) {
    indexTB = indexTBprev;
    if (i) indexTB = indexTBnext;
    if ((indexTB < 0) && (indexTB >= c_nTBPlanes)) continue;
    if (indexTB == planeVXDToTBIndex(2)) {  // PXD:
      for (const PXDCluster& clusterPXD2 : *storePXDClusters) {
        int iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int index2 = planeVXDToTBIndex(iPlane2);
        if (indexTB != index2) continue;
        TVector3 rLocal2(clusterPXD2.getU(), clusterPXD2.getV(), 0);
        TVector3 rGlobal2 = getInfoPXD(planeToIndexPXD(iPlane2)).pointToGlobal(rLocal2);
        if (clusterSVD->isUCluster()) {
          if (fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.5) isOK = 1;
        } else {
          if (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.5) isOK = 1;
        }
        if (isOK) break;
      }
    } else if ((indexTB >= planeVXDToTBIndex(c_firstSVDPlane)) && (indexTB <= planeVXDToTBIndex(c_lastSVDPlane))) {  // SVD:
      for (const SVDCluster& clusterSVD2 : *storeSVDClusters) {
        int iPlane2 = clusterSVD2.getSensorID().getLayerNumber();
        int index2 = planeVXDToTBIndex(iPlane2);
        if (indexTB != index2) continue;
        if (clusterSVD->isUCluster() != clusterSVD2.isUCluster()) continue;
        if (fabs(clusterSVD2.getClsTime() - (int)clusterSVD2.getClsTime()) < 0.000001) continue;
        if (clusterSVD2.isUCluster()) {
          TVector3 rLocal2(clusterSVD2.getPosition(), 0, 0);
          TVector3 rGlobal2 = getInfoSVD(planeToIndexSVD(iPlane2)).pointToGlobal(rLocal2);
          if (fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.5) isOK = 1;
        } else {
          TVector3 rLocal2(0, clusterSVD2.getPosition(), 0);
          TVector3 rGlobal2 = getInfoSVD(planeToIndexSVD(iPlane2)).pointToGlobal(rLocal2);
          if (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.5) isOK = 1;
        }
        if (isOK) break;
      }
    } else {                                // Tel:
      for (const PXDCluster& clusterTel2 : *storeTelClusters) {
        int iPlane2 = clusterTel2.getSensorID().getSensorNumber();
        int index2 = planeTelToTBIndex(iPlane2);
        if (indexTB != index2) continue;
        TVector3 rLocal2(clusterTel2.getU(), clusterTel2.getV(), 0);
        TVector3 rGlobal2 = getInfoTel(planeToIndexTel(iPlane2)).pointToGlobal(rLocal2);
        if (clusterSVD->isUCluster()) {
          if (fabs(rGlobal1.Y() - rGlobal2.Y()) < 0.5) isOK = 1;
        } else {
          if (fabs(rGlobal1.Z() - rGlobal2.Z()) < 0.5) isOK = 1;
        }
        if (isOK) break;
      }
    }
    if (isOK) break;
  }

  return isOK;
}

