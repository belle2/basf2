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

#include "vxd/modules/vxdMask/vxdDigitMaskingModule.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <pxd/dataobjects/PXDDigit.h>

#include <pxd/online/PXDIgnoredPixelsMap.h>
#include <svd/online/SVDIgnoredStripsMap.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <TMath.h>

#include <boost/format.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(vxdDigitMasking)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

vxdDigitMaskingModule::vxdDigitMaskingModule() : HistoModule()
{
  //Set module properties
  setDescription("vxdDigitMasking of pixels and strips in PXD and SVD base on their fireing");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("maskVXD"));
  m_nEventsProcess = -1;
  addParam("nEventsProcess", m_nEventsProcess, "Number of events to process", m_nEventsProcess);
  m_AppendMaskFile = 1;
  addParam("AppendMaskFile", m_AppendMaskFile, "Set option for append of existing file or recreate new list", m_AppendMaskFile);

  addParam("PXDChargeCut", m_PXDChargeCut, "Cut for Charge of PXD pixel", m_PXDChargeCut);
  addParam("SVDuChargeCut", m_SVDuChargeCut, "Cut for Charge of SVD U strip", m_SVDuChargeCut);
  addParam("SVDvChargeCut", m_SVDvChargeCut, "Cut for Charge of SVD V strip", m_SVDvChargeCut);

  addParam("PXDCut", m_PXDCut, "Cut for masking of PXD pixel - preset for 1 kEvent", m_PXDCut);
  addParam("SVDuCut", m_SVDuCut, "Cut for masking of SVD U strip - preset for 1 kEvent", m_SVDuCut);
  addParam("SVDvCut", m_SVDvCut, "Cut for masking of SVD V strip - preset for 1 kEvent", m_SVDvCut);

  addParam("PXDMaskFileBasicName", m_PXDMaskFileBasicName, "Name of file with list of masked channels",
           std::string("PXD_MaskFiredBasic.xml"));
  addParam("SVDMaskFileBasicName", m_SVDMaskFileBasicName, "Name of file with list of masked channels",
           std::string("SVD_MaskFiredBasic.xml"));
  addParam("PXDMaskFileRunName", m_PXDMaskFileRunName, "Name of file with list of masked channels",
           std::string("PXD_MaskFired_RunXXX.xml"));
  addParam("SVDMaskFileRunName", m_SVDMaskFileRunName, "Name of file with list of masked channels",
           std::string("SVD_MaskFired_RunXXX.xml"));
}


vxdDigitMaskingModule::~vxdDigitMaskingModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void vxdDigitMaskingModule::defineHisto()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) {
    B2WARNING("Missing geometry for VXD, check steering file.");
    return;
  }
  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD-masking is skiped.");
  }
  if (gTools->getNumberOfSVDLayers() == 0) {
    B2WARNING("Missing geometry for SVD, SVD-masking is skiped.");
  }

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  int nPXDSensors = gTools->getNumberOfPXDSensors();
  if (gTools->getNumberOfPXDLayers() != 0) {
    m_PXDHitMapUV = new TH2F*[nPXDSensors];
    m_PXDMaskUV = new TH2F*[nPXDSensors];

    for (int i = 0; i < nPXDSensors; i++) {
      VxdID id = gTools->getSensorIDFromPXDIndex(i);
      int iLayer = id.getLayerNumber();
      int iLadder = id.getLadderNumber();
      int iSensor = id.getSensorNumber();
      VxdID sensorID(iLayer, iLadder, iSensor);
      PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);

      int nPixelsU = SensorInfo.getUCells();
      int nPixelsV = SensorInfo.getVCells();
      //----------------------------------------------------------------
      // Hitmaps: Number of pixels by coordinate
      //----------------------------------------------------------------
      string name = str(format("PXD_%1%_PixelHitmap") % sensorDescr);
      string title = str(format("PXD Sensor %1% Pixel Hitmap") % sensorDescr);
      m_PXDHitMapUV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
      m_PXDHitMapUV[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_PXDHitMapUV[i]->GetYaxis()->SetTitle("v position [pitch units]");
      m_PXDHitMapUV[i]->GetZaxis()->SetTitle("hits");

      name = str(format("PXD_%1%_PixelMaskmap") % sensorDescr);
      title = str(format("PXD Sensor %1% Pixel Maskmap") % sensorDescr);
      m_PXDMaskUV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
      m_PXDMaskUV[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_PXDMaskUV[i]->GetYaxis()->SetTitle("v position [pitch units]");
      m_PXDMaskUV[i]->GetZaxis()->SetTitle("mask");
    }
  }
  int nSVDSensors = gTools->getNumberOfSVDSensors();
  if (gTools->getNumberOfPXDLayers() != 0) {
    m_SVDHitMapU = new TH1F*[nSVDSensors];
    m_SVDHitMapV = new TH1F*[nSVDSensors];
    m_SVDHitMapU2 = new TH1F*[nSVDSensors];
    m_SVDHitMapV2 = new TH1F*[nSVDSensors];
    m_SVDMaskU = new TH1F*[nSVDSensors];
    m_SVDMaskV = new TH1F*[nSVDSensors];
    m_SVDMaskU2 = new TH1F*[nSVDSensors];
    m_SVDMaskV2 = new TH1F*[nSVDSensors];

    for (int i = 0; i < nSVDSensors; i++) {
      VxdID id = gTools->getSensorIDFromSVDIndex(i);
      int iLayer = id.getLayerNumber();
      int iLadder = id.getLadderNumber();
      int iSensor = id.getSensorNumber();
      VxdID sensorID(iLayer, iLadder, iSensor);
      SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);

      int nStripsU = SensorInfo.getUCells();
      int nStripsV = SensorInfo.getVCells();
      //----------------------------------------------------------------
      // Hitmaps: Number of strips by coordinate
      //----------------------------------------------------------------
      string name = str(format("SVD_%1%_StripHitmapU") % sensorDescr);
      string title = str(format("SVD Sensor %1% Strip Hitmap in U") % sensorDescr);
      m_SVDHitMapU[i] = new TH1F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU);
      m_SVDHitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_SVDHitMapU[i]->GetYaxis()->SetTitle("hits");
      name = str(format("SVD_%1%_StripHitmapV") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap in V") % sensorDescr);
      m_SVDHitMapV[i] = new TH1F(name.c_str(), title.c_str(), nStripsV, 0, nStripsV);
      m_SVDHitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_SVDHitMapV[i]->GetYaxis()->SetTitle("hits");

      name = str(format("SVD_%1%_StripMaskmapU") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Maskmap in U") % sensorDescr);
      m_SVDMaskU[i] = new TH1F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU);
      m_SVDMaskU[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_SVDMaskU[i]->GetYaxis()->SetTitle("mask");
      name = str(format("SVD_%1%_StripMaskmapV") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Maskmap in V") % sensorDescr);
      m_SVDMaskV[i] = new TH1F(name.c_str(), title.c_str(), nStripsV, 0, nStripsV);
      m_SVDMaskV[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_SVDMaskV[i]->GetYaxis()->SetTitle("mask");

      name = str(format("SVD_%1%_StripHitmapU2") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap 2 samples in U") % sensorDescr);
      m_SVDHitMapU2[i] = new TH1F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU);
      m_SVDHitMapU2[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_SVDHitMapU2[i]->GetYaxis()->SetTitle("hits");
      name = str(format("SVD_%1%_StripHitmapV2") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap 2 samples in V") % sensorDescr);
      m_SVDHitMapV2[i] = new TH1F(name.c_str(), title.c_str(), nStripsV, 0, nStripsV);
      m_SVDHitMapV2[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_SVDHitMapV2[i]->GetYaxis()->SetTitle("hits");

      name = str(format("SVD_%1%_StripMaskmapU2") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Maskmap 2 samples in U") % sensorDescr);
      m_SVDMaskU2[i] = new TH1F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU);
      m_SVDMaskU2[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_SVDMaskU2[i]->GetYaxis()->SetTitle("mask");
      name = str(format("SVD_%1%_StripMaskmapV2") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Maskmap 2 samples in V") % sensorDescr);
      m_SVDMaskV2[i] = new TH1F(name.c_str(), title.c_str(), nStripsV, 0, nStripsV);
      m_SVDMaskV2[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_SVDMaskV2[i]->GetYaxis()->SetTitle("mask");

    }
  }
  m_nEventsPlane = new long[nPXDSensors + nSVDSensors];
  // cd back to root directory
  oldDir->cd();
}


void vxdDigitMaskingModule::initialize()
{
  //Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() != 0) {
    //Register collections
    StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
    //Store names to speed up creation later
    m_storePXDDigitsName = storePXDDigits.getName();
  }
  if (gTools->getNumberOfSVDLayers() != 0) {
    //Register collections
    StoreArray<SVDShaperDigit> storeSVDDigits(m_storeSVDShaperDigitsName);
    //Store names to speed up creation later
    m_storeSVDShaperDigitsName = storeSVDDigits.getName();
  }
}

void vxdDigitMaskingModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int nPXDSensors = gTools->getNumberOfPXDSensors();
  int nSVDSensors = gTools->getNumberOfSVDSensors();
  m_nRealEventsProcess = 0;
  for (int i = 0; i < (nPXDSensors + nSVDSensors); i++) {
    m_nEventsPlane[i] = 0;
  }
}


void vxdDigitMaskingModule::event()
{
  m_nRealEventsProcess++;
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int nPXDSensors = gTools->getNumberOfPXDSensors();
  if (gTools->getNumberOfPXDLayers() != 0) {
    const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
    // If there are no digits, leave
    if (storePXDDigits && storePXDDigits.getEntries()) {
      int firstPXDLayer = gTools->getFirstPXDLayer();
      int lastPXDLayer = gTools->getLastPXDLayer();

      for (const PXDDigit& digit : storePXDDigits) {
        if (digit.getCharge() < m_PXDChargeCut) continue;
        int iLayer = digit.getSensorID().getLayerNumber();
        if ((iLayer < firstPXDLayer) || (iLayer > lastPXDLayer)) continue;
        int iLadder = digit.getSensorID().getLadderNumber();
        int iSensor = digit.getSensorID().getSensorNumber();
        VxdID sensorID(iLayer, iLadder, iSensor);
        int index = gTools->getPXDSensorIndex(sensorID);
        if (m_PXDHitMapUV[index] != NULL) m_PXDHitMapUV[index]->Fill(digit.getUCellID(), digit.getVCellID());
        m_nEventsPlane[index]++;
      }
    }
  }
  if (gTools->getNumberOfSVDLayers() != 0) {
    const StoreArray<SVDShaperDigit> storeSVDDigits(m_storeSVDShaperDigitsName);
    // If there are no digits, leave
    if (storeSVDDigits && storeSVDDigits.getEntries()) {
      int firstSVDLayer = gTools->getFirstSVDLayer();
      int lastSVDLayer = gTools->getLastSVDLayer();

      for (const SVDShaperDigit& digit : storeSVDDigits) {
        int iLayer = digit.getSensorID().getLayerNumber();
        if ((iLayer < firstSVDLayer) || (iLayer > lastSVDLayer)) continue;
        int iLadder = digit.getSensorID().getLadderNumber();
        int iSensor = digit.getSensorID().getSensorNumber();
        VxdID sensorID(iLayer, iLadder, iSensor);
        int index = gTools->getSVDSensorIndex(sensorID);
        SVDShaperDigit::APVFloatSamples samples = digit.getSamples();

        if (digit.isUStrip()) {
          int iCont = 0;
          for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
            float fCharge1 = samples[i];
            if (fCharge1 > m_SVDuChargeCut)
              iCont++;
          }
          if (iCont == 1) {
            if (m_SVDHitMapU[index] != NULL) m_SVDHitMapU[index]->Fill(digit.getCellID());
            m_nEventsPlane[nPXDSensors + index]++;
          }
          if (iCont > 1)
            if (m_SVDHitMapU2[index] != NULL) m_SVDHitMapU2[index]->Fill(digit.getCellID());
        } else {
          int iCont = 0;
          for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
            float fCharge1 = samples[i];
            if (fCharge1 > m_SVDvChargeCut)
              iCont++;
          }
          if (iCont == 1) {
            if (m_SVDHitMapV[index] != NULL) m_SVDHitMapV[index]->Fill(digit.getCellID());
            m_nEventsPlane[nPXDSensors + index]++;
          }
          if (iCont > 1)
            if (m_SVDHitMapV2[index] != NULL) m_SVDHitMapV2[index]->Fill(digit.getCellID());
        }
      }
    }
  }
}

void vxdDigitMaskingModule::endRun()
{

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  if (m_nRealEventsProcess < 500) {
    TString message = Form("Not enough data: %li < 500, terminate without masking file create.", m_nRealEventsProcess);
    B2WARNING(message.Data());
    return;
  }
  m_nEventsProcessFraction = (double)m_nRealEventsProcess / m_nEventsProcess;

  // Maskin border for all sensors at 1000 events!:
  float PXDCut = m_PXDCut;

  float SVDUCut = m_SVDuCut;
  float SVDVCut = m_SVDvCut;

  // correction for unmerged events and different No. of proces events:
  PXDCut *= m_nEventsProcessFraction * m_nEventsProcess / 1000.0;
  SVDUCut *= m_nEventsProcessFraction * m_nEventsProcess / 1000.0;
  SVDVCut *= m_nEventsProcessFraction * m_nEventsProcess / 1000.0;

  FILE* MaskList;

  int nPXDSensors = gTools->getNumberOfPXDSensors();
  int nSVDSensors = gTools->getNumberOfSVDSensors();
  if (gTools->getNumberOfPXDLayers() != 0) {
    TString message = Form("Start to create masking from %i events (fraction: %6.3f)", (int)m_nRealEventsProcess,
                           m_nEventsProcessFraction);
    B2INFO(message.Data());

    std::string FileName = str(format("%1%") % m_PXDMaskFileBasicName);
    std::string ignoredPixelsListName = str(format("%1%") % FileName);
    std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsBasicList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
          ignoredPixelsListName));
    FileName = str(format("%1%") % m_PXDMaskFileRunName);
    ignoredPixelsListName = str(format("%1%") % FileName);
    std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsList = unique_ptr<PXDIgnoredPixelsMap>(new PXDIgnoredPixelsMap(
        ignoredPixelsListName));
    MaskList = fopen(FileName.data(), "w");
    fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(MaskList, "<Meta>\n");
    fprintf(MaskList, "    <Date>19.02.2018</Date>\n");
    fprintf(MaskList, "    <Description short=\"Ignore strip list for PXD planes in 2018 VXD phase2 and phase3\">\n");
    fprintf(MaskList, "        Crude initial list of bad pixels\n");
    fprintf(MaskList, "    </Description>\n");
    fprintf(MaskList, "    <Author>Peter Kodys</Author>\n");
    fprintf(MaskList, "</Meta>\n");
    fprintf(MaskList, "<PXD>\n");
    for (int i = 0; i < nPXDSensors; i++) {
      if (m_nEventsPlane[i] == 0) continue;
      VxdID id = gTools->getSensorIDFromPXDIndex(i);
      int iLayer = id.getLayerNumber();
      int iLadder = id.getLadderNumber();
      int iSensor = id.getSensorNumber();
      fprintf(MaskList, "  <layer n=\"%i\">\n", iLayer);
      fprintf(MaskList, "    <ladder n=\"%i\">\n", iLadder);
      fprintf(MaskList, "      <sensor n=\"%i\">\n", iSensor);
      fprintf(MaskList, "        <!-- vxdDigitMasking rectangular parts of the sensor -->\n");
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
      int nMasked = 0;
      for (int i1 = 0; i1 < m_PXDMaskUV[i]->GetNbinsX(); ++i1) {
        for (int i2 = 0; i2 < m_PXDMaskUV[i]->GetNbinsY(); ++i2) {
          int ExistMask = 0;
          if (m_AppendMaskFile) {
            if (!m_ignoredPixelsBasicList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
              ExistMask += 1;
            }
            if (!m_ignoredPixelsList->pixelOK(VxdID(iLayer, iLadder, iSensor), PXDIgnoredPixelsMap::map_pixel(i1, i2))) {
              ExistMask += 1;
            }
          }
          if (ExistMask || (m_PXDHitMapUV[i]->GetBinContent(i1 + 1, i2 + 1) > PXDCut)) {
            fprintf(MaskList, "        <pixels uStart = \"%04i\" vStart = \"%04i\"></pixels>\n", i1, i2);
            m_PXDMaskUV[i]->SetBinContent(i1 + 1, i2 + 1, 1 + ExistMask);
            nMasked++;
          }
        }
      }
      fprintf(MaskList, "\n");
      fprintf(MaskList, "      </sensor>\n");
      fprintf(MaskList, "    </ladder>\n");
      fprintf(MaskList, "  </layer>\n");
    }
    fprintf(MaskList, "</PXD>\n");
    fclose(MaskList);
  }

  if (gTools->getNumberOfSVDLayers() != 0) {
    std::string FileName = str(format("%1%") % m_SVDMaskFileBasicName);
    std::string ignoredPixelsListName = str(format("%1%") % FileName);
    std::unique_ptr<SVDIgnoredStripsMap> m_ignoredStripsBasicList = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(
          ignoredPixelsListName));
    string FileName2 = str(format("%1%") % m_SVDMaskFileRunName);
    string ignoredPixelsListName2 = str(format("%1%") % FileName2);
    std::unique_ptr<SVDIgnoredStripsMap> m_ignoredStripsList = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(
        ignoredPixelsListName2));
    MaskList = fopen(FileName2.data(), "w");
    fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(MaskList, "<Meta>\n");
    fprintf(MaskList, "    <Date>19.02.2018</Date>\n");
    fprintf(MaskList, "    <Description short=\"Ignore strip list for SVD planes in 2018 VXD phase2 and phase3\">\n");
    fprintf(MaskList, "        Crude initial list of bad strips\n");
    fprintf(MaskList, "    </Description>\n");
    fprintf(MaskList, "    <Author>Peter Kodys</Author>\n");
    fprintf(MaskList, "</Meta>\n");
    fprintf(MaskList, "<SVD>\n");
    for (int i = 0; i < nSVDSensors; i++) {
      if (m_nEventsPlane[nPXDSensors + i] == 0) continue;
      VxdID id = gTools->getSensorIDFromSVDIndex(i);
      int iLayer = id.getLayerNumber();
      int iLadder = id.getLadderNumber();
      int iSensor = id.getSensorNumber();
      fprintf(MaskList, "    <layer n=\"%i\">\n", iLayer);
      fprintf(MaskList, "        <ladder n=\"%i\">\n", iLadder);
      fprintf(MaskList, "            <sensor n=\"%i\">\n", iSensor);
      fprintf(MaskList, "                <side side=\"u\">\n");
      fprintf(MaskList, "                    <!-- stripsFromTo fromStrip = \"620\" toStrip = \"767\"></stripsFromTo-->\n");
      fprintf(MaskList, "                    <!-- Individual strips can be masked, too -->\n");
      int nMaskedU = 0;
      int nMaskedV = 0;
      for (int i1 = 0; i1 < m_SVDMaskU[i]->GetNbinsX(); ++i1) {
        int ExistMask = 0;
        if (m_AppendMaskFile) {
          if (!m_ignoredStripsBasicList->stripOK(VxdID(iLayer, iLadder, iSensor, 1), (unsigned short) i1)) {
            ExistMask += 1;
          }
          if (!m_ignoredStripsList->stripOK(VxdID(iLayer, iLadder, iSensor, 1), (unsigned short) i1)) {
            ExistMask += 1;
          }
        }
        int sTS = 0;
        if (m_SVDHitMapU[i]->GetBinContent(i1 + 1) > SVDUCut)
          sTS = 1;

        if (ExistMask || (sTS)) {
          fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i1);
          m_SVDMaskU[i]->SetBinContent(i1 + 1, 1 + ExistMask);
          nMaskedU++;
        }
      }
      fprintf(MaskList, "                </side>\n");
      fprintf(MaskList, "                <side side=\"v\">\n");
      for (int i2 = 0; i2 < m_SVDMaskV[i]->GetNbinsX(); ++i2) {
        int ExistMask = 0;
        if (m_AppendMaskFile) {
          if (!m_ignoredStripsBasicList->stripOK(VxdID(iLayer, iLadder, iSensor, 0), (unsigned short) i2)) {
            ExistMask += 1;
          }
          if (!m_ignoredStripsList->stripOK(VxdID(iLayer, iLadder, iSensor, 0), (unsigned short) i2)) {
            ExistMask += 1;
          }
        }
        int sTS = 0;
        if (m_SVDHitMapV[i]->GetBinContent(i2 + 1) > SVDVCut)
          sTS = 1;

        if (ExistMask || (sTS)) {
          fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i2);
          m_SVDMaskV[i]->SetBinContent(i2 + 1, 1 + ExistMask);
          nMaskedV++;
        }
      }
      fprintf(MaskList, "                </side>\n");
      fprintf(MaskList, "            </sensor>\n");
      TString message = Form("SVD(%i,%i,%i) masked %i U strips in: %s", iLayer, iLadder, iSensor, nMaskedU, ignoredPixelsListName.data());
      B2INFO(message.Data());
      message = Form("SVD(%i,%i,%i) masked %i V strips in: %s", iLayer, iLadder, iSensor, nMaskedV, ignoredPixelsListName.data());
      B2INFO(message.Data());
      fprintf(MaskList, "        </ladder>\n");
      fprintf(MaskList, "    </layer>\n");
    }
    fprintf(MaskList, "</SVD>\n");
    fclose(MaskList);

//------------------------------------------------------------------
// the same for 2 and more fired samples in strip
//-----------------------------------------------------------------
    FileName = str(format("%1%.2samples") % m_SVDMaskFileBasicName);
    ignoredPixelsListName = str(format("%1%") % FileName);
    std::unique_ptr<SVDIgnoredStripsMap> m_ignoredStripsBasicList2 = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(
          ignoredPixelsListName));
    FileName2 = str(format("%1%.2samples") % m_SVDMaskFileRunName);
    ignoredPixelsListName2 = str(format("%1%") % FileName2);
    std::unique_ptr<SVDIgnoredStripsMap> m_ignoredStripsList2 = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(
        ignoredPixelsListName2));
    MaskList = fopen(FileName2.data(), "w");
    fprintf(MaskList, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(MaskList, "<Meta>\n");
    fprintf(MaskList, "    <Date>19.02.2018</Date>\n");
    fprintf(MaskList,
            "    <Description short=\"Ignore strip list for SVD planes for 2-samples and more over charge cut in 2018 VXD phase2 and phase3\">\n");
    fprintf(MaskList, "        Crude initial list of bad strips\n");
    fprintf(MaskList, "    </Description>\n");
    fprintf(MaskList, "    <Author>Peter Kodys</Author>\n");
    fprintf(MaskList, "</Meta>\n");
    fprintf(MaskList, "<SVD>\n");
    for (int i = 0; i < nSVDSensors; i++) {
      if (m_nEventsPlane[nPXDSensors + i] == 0) continue;
      VxdID id = gTools->getSensorIDFromSVDIndex(i);
      int iLayer = id.getLayerNumber();
      int iLadder = id.getLadderNumber();
      int iSensor = id.getSensorNumber();
      fprintf(MaskList, "    <layer n=\"%i\">\n", iLayer);
      fprintf(MaskList, "        <ladder n=\"%i\">\n", iLadder);
      fprintf(MaskList, "            <sensor n=\"%i\">\n", iSensor);
      fprintf(MaskList, "                <side side=\"u\">\n");
      fprintf(MaskList, "                    <!-- stripsFromTo fromStrip = \"620\" toStrip = \"767\"></stripsFromTo-->\n");
      fprintf(MaskList, "                    <!-- Individual strips can be masked, too -->\n");
      int nMaskedU = 0;
      int nMaskedV = 0;
      for (int i1 = 0; i1 < m_SVDMaskU2[i]->GetNbinsX(); ++i1) {
        int ExistMask = 0;
        if (m_AppendMaskFile) {
          if (!m_ignoredStripsBasicList2->stripOK(VxdID(iLayer, iLadder, iSensor, 1), (unsigned short) i1)) {
            ExistMask += 1;
          }
          if (!m_ignoredStripsList2->stripOK(VxdID(iLayer, iLadder, iSensor, 1), (unsigned short) i1)) {
            ExistMask += 1;
          }
        }
        int sTS = 0;
        if (m_SVDHitMapU2[i]->GetBinContent(i1 + 1) > SVDUCut)
          sTS = 1;

        if (ExistMask || (sTS)) {
          fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i1);
          m_SVDMaskU2[i]->SetBinContent(i1 + 1, 1 + ExistMask);
          nMaskedU++;
        }
      }
      fprintf(MaskList, "                </side>\n");
      fprintf(MaskList, "                <side side=\"v\">\n");
      for (int i2 = 0; i2 < m_SVDMaskV2[i]->GetNbinsX(); ++i2) {
        int ExistMask = 0;
        if (m_AppendMaskFile) {
          if (!m_ignoredStripsBasicList2->stripOK(VxdID(iLayer, iLadder, iSensor, 0), (unsigned short) i2)) {
            ExistMask += 1;
          }
          if (!m_ignoredStripsList2->stripOK(VxdID(iLayer, iLadder, iSensor, 0), (unsigned short) i2)) {
            ExistMask += 1;
          }
        }
        int sTS = 0;
        if (m_SVDHitMapV2[i]->GetBinContent(i2 + 1) > SVDVCut)
          sTS = 1;

        if (ExistMask || (sTS)) {
          fprintf(MaskList, "                    <strip stripNo = \"%i\"></strip>\n", i2);
          m_SVDMaskV2[i]->SetBinContent(i2 + 1, 1 + ExistMask);
          nMaskedV++;
        }
      }
      fprintf(MaskList, "                </side>\n");
      fprintf(MaskList, "            </sensor>\n");
      TString message = Form("SVD(%i,%i,%i) masked %i U strips in: %s", iLayer, iLadder, iSensor, nMaskedU, ignoredPixelsListName.data());
      B2INFO(message.Data());
      message = Form("SVD(%i,%i,%i) masked %i V strips in: %s", iLayer, iLadder, iSensor, nMaskedV, ignoredPixelsListName.data());
      B2INFO(message.Data());
      fprintf(MaskList, "        </ladder>\n");
      fprintf(MaskList, "    </layer>\n");
    }
    fprintf(MaskList, "</SVD>\n");
    fclose(MaskList);
  }

}

