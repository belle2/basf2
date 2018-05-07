/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TDirectory.h>

/* Belle2 headers. */
#include <eklm/modules/EKLMDQM/EKLMDQMModule.h>

using namespace Belle2;

REG_MODULE(EKLMDQM)

EKLMDQMModule::EKLMDQMModule() : HistoModule()
{
  setDescription("EKLM data quality monitor.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_HistogramDirectoryName,
           "Directory for EKLM DQM histograms in ROOT file.",
           std::string("EKLM"));
  m_Elements = &(EKLM::ElementNumbersSingleton::Instance());
  m_Sector = NULL;
  m_Time = NULL;
  m_StripLayer = NULL;
}

EKLMDQMModule::~EKLMDQMModule()
{
  if (m_StripLayer != NULL)
    delete m_StripLayer;
}

void EKLMDQMModule::defineHisto()
{
  int i;
  int endcap, layer, detectorLayer, stripMin, stripMax;
  int maxLayerGlobal, maxSector, maxPlane, maxStrip;
  std::string str, str2;
  TDirectory* oldDirectory, *newDirectory;
  oldDirectory = gDirectory;
  newDirectory = oldDirectory->mkdir(m_HistogramDirectoryName.c_str());
  newDirectory->cd();
  m_Sector = new TH1F("sector", "Sector number", 104, 0.5, 104.5);
  m_Time = new TH1F("time", "Hit time", 50, 0, 50);
  maxLayerGlobal = m_Elements->getMaximalLayerGlobalNumber();
  maxSector = m_Elements->getMaximalSectorNumber();
  maxPlane = m_Elements->getMaximalPlaneNumber();
  maxStrip = m_Elements->getMaximalStripNumber();
  m_StripLayer = new TH1F*[maxLayerGlobal];
  for (i = 0; i < maxLayerGlobal; i++) {
    detectorLayer = i + 1;
    str = "strip_layer_" + std::to_string(detectorLayer);
    str2 = "Strip number (layer " + std::to_string(detectorLayer) + ")";
    m_Elements->layerNumberToElementNumbers(detectorLayer, &endcap, &layer);
    stripMin = m_Elements->stripNumber(endcap, layer, 1, 1, 1);
    stripMax = m_Elements->stripNumber(endcap, layer,
                                       maxSector, maxPlane, maxStrip);
    m_StripLayer[i] = new TH1F(str.c_str(), str2.c_str(),
                               stripMax - stripMin + 1,
                               stripMin - 0.5, stripMax + 0.5);
  }
  oldDirectory->cd();
}

void EKLMDQMModule::initialize()
{
  REG_HISTOGRAM
  m_Digits.isRequired();
}

void EKLMDQMModule::beginRun()
{
  int i, n;
  m_Sector->Reset();
  m_Time->Reset();
  n = m_Elements->getMaximalLayerGlobalNumber();
  for (i = 0; i < n; i++)
    m_StripLayer[i]->Reset();
}

void EKLMDQMModule::event()
{
  int i, n;
  int endcap, layer, sector, plane, strip;
  int detectorLayer, sectorGlobal, stripGlobal;
  EKLMDigit* eklmDigit;
  n = m_Digits.getEntries();
  for (i = 0; i < n; i++) {
    eklmDigit = m_Digits[i];
    /*
     * Reject digits that are below the threshold (such digits may appear
     * for simulated events).
     */
    if (!eklmDigit->isGood())
      continue;
    endcap = eklmDigit->getEndcap();
    layer = eklmDigit->getLayer();
    sector = eklmDigit->getSector();
    plane = eklmDigit->getPlane();
    strip = eklmDigit->getStrip();
    detectorLayer = m_Elements->detectorLayerNumber(endcap, layer);
    sectorGlobal = m_Elements->sectorNumber(endcap, layer, sector);
    stripGlobal = m_Elements->stripNumber(endcap, layer, sector, plane, strip);
    m_Sector->Fill(sectorGlobal);
    m_StripLayer[detectorLayer - 1]->Fill(stripGlobal);
    m_Time->Fill(eklmDigit->getTime());
  }
}

void EKLMDQMModule::endRun()
{
}

void EKLMDQMModule::terminate()
{
}
