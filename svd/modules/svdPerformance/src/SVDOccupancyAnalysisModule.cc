#include <svd/modules/svdPerformance/SVDOccupancyAnalysisModule.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDOccupancyAnalysis)

SVDOccupancyAnalysisModule::SVDOccupancyAnalysisModule() : Module()
{

  setDescription("This module check performances of SVD reconstruction of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDOccupancyAnalysis_output.root"));

  addParam("groupNevents", m_group, "Number of events to group", float(10000));
  addParam("minZScut", m_minZS, "Minimum ZS cut", float(3));
  addParam("maxZScut", m_maxZS, "Maximum ZS cut", float(6));
  addParam("pointsZScut", m_pointsZS, "Number of ZS cuts", int(8));

  addParam("ShaperDigitsName", m_ShaperDigitName, "Name of ShaperDigit Store Array.", std::string(""));
}

SVDOccupancyAnalysisModule::~SVDOccupancyAnalysisModule()
{

}

void SVDOccupancyAnalysisModule::initialize()
{

  m_eventMetaData.isRequired();
  m_svdShapers.isRequired(m_ShaperDigitName);


  B2INFO("    ShaperDigits: " << m_ShaperDigitName);


  //create list of histograms to be saved in the rootfile

  for (int i = 0; i < m_nLayers; i++) {
    m_histoList_shaper[i] = new TList;
  }

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  TString NameOfHisto = "";
  TString TitleOfHisto = "";

  for (int s = 0; s < m_nLayers; s++)
    sensorsOnLayer[s] = s + 2;

  //create histograms
  for (int i = 0; i < m_nLayers; i ++) //loop on Layers
    for (int j = 0; j < (int)sensorsOnLayer[i]; j ++) //loop on Sensors
      for (int k = 0; k < m_nSides; k ++) { //loop on Sides

        TString nameLayer = "";
        nameLayer += i + 3;

        TString nameSensor = "";
        nameSensor += j + 1;

        TString nameSide = "";
        if (k == 1)
          nameSide = "U";
        else if (k == 0)
          nameSide = "V";


        NameOfHisto = "occupancy_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "Occupancy (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_occ[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 768, 0, 768, "cellID", m_histoList_shaper[i]);

        NameOfHisto = "occVSzs_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "Average Occupancy VS ZS cut (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_zsOcc[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, m_pointsZS, m_minZS, m_maxZS, "ZS cut", m_histoList_shaper[i]);


        NameOfHisto = "occVSzsSQ_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "Average Occupancy VS (ZS cut)^2 (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_zsOccSQ[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, TMath::Power(m_minZS, 2) - 5, TMath::Power(m_maxZS, 2),
                                               "(ZS cut)^2", m_histoList_shaper[i]);

        NameOfHisto = "occVSevt_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "Occupancy vs Evt number(L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_occtdep[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto,
                                               1000, 0, 1000, "evt number/10000",
                                               768, 0, 768, "cellID",
                                               m_histoList_shaper[i]);

      }
  m_nEvents = 0;

}


void SVDOccupancyAnalysisModule::beginRun()
{

}

void SVDOccupancyAnalysisModule::event()
{

  m_nEvents++;
  int nEvent = m_eventMetaData->getEvent();

  //ShaperDigits

  //shaper digits
  for (int digi = 0 ; digi < m_svdShapers.getEntries(); digi++) {


    VxdID::baseType theVxdID = (VxdID::baseType)m_svdShapers[digi]->getSensorID();
    int layer = VxdID(theVxdID).getLayerNumber() - 3;
    int sensor = getSensor(VxdID(theVxdID).getSensorNumber());
    int side = m_svdShapers[digi]->isUStrip();

    //fill standard occupancy plot, for default zero suppression
    h_occtdep[layer][sensor][side]->Fill(nEvent / m_group, m_svdShapers[digi]->getCellID());

    //fill standard occupancy plot, for default zero suppression
    h_occ[layer][sensor][side]->Fill(m_svdShapers[digi]->getCellID());

    float noise = m_NoiseCal.getNoise(theVxdID, side, m_svdShapers[digi]->getCellID());
    float step = (m_maxZS - m_minZS) / m_pointsZS;

    for (int z = 0; z <= m_pointsZS; z++) {
      int nOKSamples = 0;
      float cutMinSignal = (m_minZS + step * z) * noise;

      Belle2::SVDShaperDigit::APVFloatSamples samples_vec = m_svdShapers[digi]->getSamples();

      for (int k = 0; k < 6; k ++)
        if (samples_vec[k] > cutMinSignal)
          nOKSamples++;

      if (nOKSamples > 0) {
        h_zsOcc[layer][sensor][side]->Fill(m_minZS + z * step);
        h_zsOccSQ[layer][sensor][side]->Fill(TMath::Power(m_minZS + z * step, 2));
      }
    }

  }

}


void SVDOccupancyAnalysisModule::endRun()
{

}


void SVDOccupancyAnalysisModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;
    TH1F* obj;

    for (int i = 0; i < m_nLayers; i++) {
      TString layerName = "shaperL";
      layerName += i + 3;
      TDirectory* dir_layer = oldDir->mkdir(layerName.Data());
      dir_layer->cd();
      TIter nextH_shaper(m_histoList_shaper[i]);
      while ((obj = (TH1F*)nextH_shaper())) {
        int nStrips = 768;
        TString name = obj->GetName();
        if (name.Contains("occupancy"))
          obj->Scale(1. / m_nEvents);
        else if (name.Contains("occVSevt"))
          obj->Scale(1. / m_group);
        else {
          if ((! name.Contains("L3")) &&
              (name.Contains("1V") || name.Contains("2V") || name.Contains("3V") || name.Contains("4V") || name.Contains("5V")))
            nStrips = 512;

          obj->Scale(1. / m_nEvents / nStrips);
        }
        obj->Write();
      }

    }
    m_rootFilePtr->Close();

  }
}


TH1F*  SVDOccupancyAnalysisModule::createHistogram1D(const char* name, const char* title,
                                                     Int_t nbins, Double_t min, Double_t max,
                                                     const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH2F*  SVDOccupancyAnalysisModule::createHistogram2D(const char* name, const char* title,
                                                     Int_t nbinsX, Double_t minX, Double_t maxX,
                                                     const char* titleX,
                                                     Int_t nbinsY, Double_t minY, Double_t maxY,
                                                     const char* titleY, TList* histoList)
{

  TH2F* h = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);

  if (histoList)
    histoList->Add(h);

  return h;
}
