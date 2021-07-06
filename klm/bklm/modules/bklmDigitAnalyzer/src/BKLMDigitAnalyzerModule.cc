/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/modules/bklmDigitAnalyzer/BKLMDigitAnalyzerModule.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

/* ROOT headers. */
#include <TGaxis.h>

using namespace Belle2;


REG_MODULE(BKLMDigitAnalyzer)


BKLMDigitAnalyzerModule::BKLMDigitAnalyzerModule() : Module(),
  m_runNumber(0),
  m_outputRootFile(nullptr),
  m_histoList(nullptr)
{
  for (int i = 0; i < 2; ++i) {
    m_histoLayerVsSector[i] = nullptr;
    m_histoLayerVsSectorPerPlane[i][0] = nullptr;
    m_histoLayerVsSectorPerPlane[i][1] = nullptr;
    for (int s = 0; s < 8; ++s) {
      m_histoLayer[i][s] = nullptr;
      for (int p = 0; p < 2; ++p) {
        m_histoChannel[i][s][p] = nullptr;
        m_histoStrip[i][s][p] = nullptr;
        m_histoTdc[i][s][p] = nullptr;
        m_histoCTimeDiff[i][s][p] = nullptr;
      }
    }
  }

  // Set module properties
  setDescription("Module useful to quickly analyze BKLM unpacked data.");

  // Parameter definitions
  addParam("outputRootName", m_outputRootName, "Name of output .root file (without .root!)", std::string("bklmHitmap"));

}

BKLMDigitAnalyzerModule::~BKLMDigitAnalyzerModule()
{
}

void BKLMDigitAnalyzerModule::initialize()
{
  m_digit.isRequired();
  m_digitRaw.isRequired();
  m_digitOutOfRange.isRequired("KLMDigitsOutOfRange");
  m_digitEventInfo.isRequired();
}

void BKLMDigitAnalyzerModule::beginRun()
{
  time_t rawTime;
  time(&rawTime);
  struct tm* tm = gmtime(&rawTime);

  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  m_runNumber = eventMetaData->getRun();

  TString runNumberTString(toString(m_runNumber).c_str());

  TString outputRootNameTString(m_outputRootName);
  outputRootNameTString += "_run" + runNumberTString + ".root";

  TString label[2] = {"BF", "BB"};

  m_outputRootFile = new TFile(outputRootNameTString, "RECREATE");
  B2INFO("BKLMDigitAnalyzer:: the output file '" << outputRootNameTString.Data() << "' will be created for run " << m_runNumber);

  int exp = -1;
  int run = -1;
  int year = -1;
  int month = -1;
  int day = -1;
  int hour = -1;
  int min = -1;
  int sec = -1;
  m_extraInfo = new TTree("extraInfo", "Extra informations");
  m_extraInfo->Branch("exp", &exp, "exp/I");
  m_extraInfo->Branch("run", &run, "run/I");
  m_extraInfo->Branch("year", &year, "year/I");
  m_extraInfo->Branch("month", &month, "month/I");
  m_extraInfo->Branch("day", &day, "day/I");
  m_extraInfo->Branch("hour", &hour, "hour/I");
  m_extraInfo->Branch("min", &min, "min/I");
  m_extraInfo->Branch("sec", &sec, "sec/I");
  exp = eventMetaData->getExperiment();
  run = m_runNumber;
  year = tm->tm_year + 1900;
  month = tm->tm_mon + 1;
  day = tm->tm_mday;
  hour = tm->tm_hour;
  min = tm->tm_min;
  sec = tm->tm_sec;
  m_extraInfo->Fill();

  m_histoList = new TList;

  for (int fb = 0; fb < 2; fb++) {

    m_histoLayerVsSector[fb] = createTH2("SectLay" + label[fb], label[fb] + " Hitmap -- run" + runNumberTString, 31, -0.5, 15.,
                                         "Layer (0-based)", 17, -0.5, 8., "BF Sector", 0, m_histoList);

    for (int isRPCorPhi = 0; isRPCorPhi < 2; isRPCorPhi++) {

      if (isRPCorPhi == 0)
        m_histoLayerVsSectorPerPlane[fb][isRPCorPhi] = createTH2("SectLayPlaneZ" + label[fb],
                                                                 label[fb] + " Hitmap of plane z -- run" + runNumberTString, 31, -0.5, 15., "Layer (0-based)", 17, -0.5, 8., "BF Sector", 0,
                                                                 m_histoList);
      else
        m_histoLayerVsSectorPerPlane[fb][isRPCorPhi] = createTH2("SectLayPlanePhi" + label[fb],
                                                                 label[fb] + " Hitmap of plane phi -- run" + runNumberTString, 31, -0.5, 15., "Layer (0-based)", 17, -0.5, 8., "BF Sector", 0,
                                                                 m_histoList);

      for (int iSector = 0; iSector < 8; iSector++) {

        TString iSectorTString(toString(iSector).c_str());
        TString nameSector = label[fb] + iSectorTString;

        if (isRPCorPhi == 0) {

          // Create the histogram belowo only one time
          m_histoLayer[fb][iSector] = createTH1("Layer" + nameSector, nameSector + " Layer -- run" + runNumberTString, 31, -0.5, 15.,
                                                "Layer (0-based)", "Counts", 1, m_histoList);

          m_histoChannel[fb][iSector][isRPCorPhi] = createTH2("PlaneZ" + nameSector,
                                                              nameSector + " Plane z, electronic channels -- run" + runNumberTString, 31,
                                                              -0.5, 15., "Layer (0-based)", 130, -0.5, 64.5, "Channel", 1, m_histoList);

          m_histoStrip[fb][iSector][isRPCorPhi] = createTH2("PlaneZStrip" + nameSector,
                                                            nameSector + " Plane z, strips -- run" + runNumberTString,
                                                            31, -0.5, 15., "Layer (0-based)", 130, -0.5, 64.5, "Strip", 1, m_histoList);

          m_histoTdc[fb][iSector][isRPCorPhi] = createTH1("SciTdc" + nameSector,
                                                          nameSector + " TDC (Scintillators) -- run" + runNumberTString, 60, 0, 30, "TDC", "Counts", 1, m_histoList);

          m_histoCTimeDiff[fb][iSector][isRPCorPhi] = createTH1("SciCTimeDiff" + nameSector,
                                                                nameSector + " CTime diff. (Scintillators) -- run" + runNumberTString, 200, -200, -100, "Sci_CTime - Trg_CTime [ns]", "Counts", 1,
                                                                m_histoList);

        } else {

          m_histoChannel[fb][iSector][isRPCorPhi] = createTH2("PlanePhi" + nameSector,
                                                              nameSector + " Plane phi, electronic channels -- run" + runNumberTString,
                                                              31, -0.5, 15., "Layer (0-based)", 130, -0.5, 64.5, "Channel", 1, m_histoList);

          m_histoStrip[fb][iSector][isRPCorPhi] = createTH2("PlanePhiStrip" + nameSector,
                                                            nameSector + " Plane phi, strips -- run" + runNumberTString,
                                                            31, -0.5, 15., "Layer (0-based)", 130, -0.5, 64.5, "Strip", 1, m_histoList);

          m_histoTdc[fb][iSector][isRPCorPhi] = createTH1("RPCTdc" + nameSector, nameSector + " TDC (RPCs) -- run" + runNumberTString, 230,
                                                          -100, 2200, "TDC", "Counts", 1, m_histoList);

          m_histoCTimeDiff[fb][iSector][isRPCorPhi] = createTH1("RPCCTimeDiff" + nameSector,
                                                                nameSector + " CTime diff. (RPCs) -- run" + runNumberTString, 350, -3000, 500, "RPC_CTime - Trg_CTime [ns]", "Counts", 1,
                                                                m_histoList);

        }
      }
    }
  }
}

void BKLMDigitAnalyzerModule::event()
{
  for (int i = 0; i < m_digitEventInfo.getEntries(); i++) {

    KLMDigitEventInfo* digitEventInfo = m_digitEventInfo[i];
    // Some warnings (they should never appear, but it's better to be sure)
    if ((digitEventInfo->getRPCHits() + digitEventInfo->getSciHits()) != (int)digitEventInfo->getRelationsFrom<KLMDigit>().size())
      B2WARNING("BKLMDigitAnalyzer:: the total number of BKLMDigit differs from the sum of RPC and scintillator hits stored in BKLMEventDigitDebug!");
    if (digitEventInfo->getOutOfRangeHits() != (int)digitEventInfo->getRelationsTo<KLMDigit>("BKLMDigitsOutOfRange").size())
      B2WARNING("BKLMDigitAnalyzer:: the total number of BKLMDigit differs from the number of outOfRange-flagged hits stored in BKLMEventDigitDebug!");

    for (const KLMDigit& digit : digitEventInfo->getRelationsFrom<KLMDigit>()) {
      if (digit.getSubdetector() != KLMElementNumbers::c_BKLM)
        continue;

      KLMDigitRaw* digitRaw = digit.getRelatedTo<KLMDigitRaw>();

      m_histoLayerVsSector[1 - digit.getSection()]->Fill(digit.getLayer() - 1, digit.getSector() - 1);

      m_histoLayerVsSectorPerPlane[1 - digit.getSection()][digit.isPhiReadout()]->Fill(digit.getLayer() - 1, digit.getSector() - 1);

      m_histoLayer[1 - digit.getSection()][digit.getSector() - 1]->Fill(digit.getLayer() - 1);

      m_histoChannel[1 - digit.getSection()][digit.getSector() - 1][digit.isPhiReadout()]->Fill(digit.getLayer() - 1,
          digitRaw->getChannel() - 1);

      m_histoStrip[1 - digit.getSection()][digit.getSector() - 1][digit.isPhiReadout()]->Fill(digit.getLayer() - 1,
          digit.getStrip() - 1);

      // getTime() retruns the TDC
      m_histoTdc[1 - digit.getSection()][digit.getSector() - 1][digit.inRPC()]->Fill(digit.getTime());

      m_histoCTimeDiff[1 - digit.getSection()][digit.getSector() - 1][digit.inRPC()]->Fill(digit.getCTime() -
          digitEventInfo->getIntTriggerCTime());

    }
  }
}

void BKLMDigitAnalyzerModule::endRun()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);

  // Save the .root file
  if (m_outputRootFile != nullptr) {
    m_outputRootFile->cd();

    m_extraInfo->Write();

    TIter nextHisto(m_histoList);
    TObject* obj;
    while ((obj = nextHisto()))
      obj->Write("", TObject::kWriteDelete);
    m_outputRootFile->Close();
  }
}

void BKLMDigitAnalyzerModule::terminate()
{
}

TH1F* BKLMDigitAnalyzerModule::createTH1(const char* name, const char* title, Int_t nbinsX, Double_t minX, Double_t maxX,
                                         const char* titleX, const char* titleY, bool drawStat, TList* histoList)
{
  TH1F* hist = new TH1F(name, title, nbinsX, minX, maxX);
  hist->GetXaxis()->SetTitle(titleX);
  hist->GetYaxis()->SetTitle(titleY);
  TGaxis::SetMaxDigits(3);
  hist->SetLineWidth(1);
  hist->SetLineColor(kRed);
  if (!drawStat)
    hist->SetStats(0);
  if (histoList)
    histoList->Add(hist);
  return hist;
}

TH2F* BKLMDigitAnalyzerModule::createTH2(const char* name, const char* title, Int_t nbinsX, Double_t minX, Double_t maxX,
                                         const char* titleX, Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY, bool drawStat, TList* histoList)
{
  TH2F* hist = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);
  hist->GetXaxis()->SetTitle(titleX);
  hist->GetYaxis()->SetTitle(titleY);
  TGaxis::SetMaxDigits(3);
  if (!drawStat)
    hist->SetStats(0);
  if (histoList)
    histoList->Add(hist);
  return hist;
}
