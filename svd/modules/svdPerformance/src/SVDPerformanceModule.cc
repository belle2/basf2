#include <svd/modules/svdPerformance/SVDPerformanceModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <geometry/GeometryManager.h>
#include <framework/dataobjects/EventMetaData.h>
#include <time.h>
#include <list>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDPerformance)

SVDPerformanceModule::SVDPerformanceModule() : Module()
{

  setDescription("This module check performances of SVD reconstruction of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDPerformance_output.root"));

  addParam("ClustersName", m_ClusterName, "Name of Cluster Store Array.", std::string(""));
  addParam("TrackListName", m_TrackName, "Name of Track Store Array.", std::string(""));
  addParam("TrackFitResultListName", m_TrackFitResultName, "Name of TracksFitResult Store Array.", std::string(""));
}

SVDPerformanceModule::~SVDPerformanceModule()
{

}

void SVDPerformanceModule::initialize()
{

  StoreObjPtr<EventMetaData>::required();
  StoreArray<SVDShaperDigit>::required();
  StoreArray<SVDCluster>::required(m_ClusterName);
  StoreArray<Track>::required(m_TrackName);
  StoreArray<TrackFitResult>::required(m_TrackFitResultName);


  B2INFO("          Tracks: " << m_TrackName);
  B2INFO(" TrackFitResults: " << m_TrackFitResultName);


  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;
  m_histoList_cluster = new TList;
  m_histoList_clustertrk = new TList;
  m_histoList_shaper = new TList;
  m_histoList_track = new TList;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  TString NameOfHisto = "";
  TString TitleOfHisto = "";

  //create histograms
  for (int i = 0; i < 4; i ++) //loop on Layers
    for (int j = 0; j < 2; j ++) //loop on Sensors
      for (int k = 0; k < 2; k ++) { //loop on Sides
        TString nameLayer = "";
        TString nameSensor = "";
        TString nameSide = "";
        nameLayer += i + 3;
        if (i == 0)
          nameSensor += j + 1;
        else if (i == 1 || i == 2)
          nameSensor += j + 2;
        else if (i == 3)
          nameSensor += j + 3;
        if (k == 0)
          nameSide = "U";
        else if (k == 1)
          nameSide = "V";

        //Strip NOISE
        NameOfHisto = "hStripNoiseL" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "strip noise (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_stripNoise[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, 300, 1800, "strip noise", m_histoList_shaper);

        //CLUSTERS RELATED TO TRACKS
        NameOfHisto = "hnClTrkL" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "number of clusters related to Tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_nCltrk[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 12, 0, 12, "n clusters", m_histoList_clustertrk);

        NameOfHisto = "hnClTrkChargeL" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "1-strip cluster Charge (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_1cltrkCharge[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 50000, "charge(e-)", m_histoList_clustertrk);


        NameOfHisto = "hnClTrkSNL" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "1-strip cluster S/N (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_1cltrkSN[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 100, "S/N", m_histoList_clustertrk);

        NameOfHisto = "hnClTrkSizeL" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster size (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkSize[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 10, 0, 10, "cluster size", m_histoList_clustertrk);

        NameOfHisto = "htrkchrgVSsizeL" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster charge VS size (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        //  h_cltrkChargeVSSize[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 500, 0, 50000, "charge (ADC)", 10, 0, 10, "cl size", m_histoList_clustertrk);

      }

  //tracks
  m_nTracks = createHistogram1D("h1nTracks", "number of Tracks per event", 10, 0, 10, "n Tracks", m_histoList_track);
  m_Pvalue = createHistogram1D("h1pValue", "Tracks p value", 100, 0, 1, "p value", m_histoList_track);
  m_mom = createHistogram1D("h1momentum", " Tracks Momentum", 100, 0, 6, "p (GeV/c)", m_histoList_track);
  m_nSVDhits = createHistogram1D("h1nSVDhits", "# SVD hits per track", 20, 0, 20, "# SVD hits", m_histoList_track);


  m_ntracks = 0;
}

void SVDPerformanceModule::beginRun()
{

}

void SVDPerformanceModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  // SVD digits
  StoreArray<SVDShaperDigit> svdShapers;

  int nDigi[4][2][2];


  // SVD clusters
  StoreArray<SVDCluster> svdClusters(m_ClusterName);
  int nCl[4][2][2];
  int nCltrk[4][2][2];

  for (int i = 0; i < 4; i ++) //loop on Layers
    for (int j = 0; j < 2; j ++) //loop on Sensors
      for (int k = 0; k < 2; k ++) { //loop on Sides
        nDigi[i][j][k] = 0;
        nCl[i][j][k] = 0;
        nCltrk[i][j][k] = 0;
      }


  //tracks
  StoreArray<Track> Tracks(m_TrackName);
  if (Tracks) {
    m_nTracks->Fill(Tracks.getEntries());
    m_ntracks += Tracks.getEntries();
  }
  BOOST_FOREACH(Track & track, Tracks) {

    const TrackFitResult* tfr = track.getTrackFitResult(Const::electron);
    m_Pvalue->Fill(tfr->getPValue());
    m_mom->Fill(tfr->getMomentum().Mag());
    m_nSVDhits->Fill((tfr->getHitPatternVXD()).getNSVDHits());
    if ((tfr->getPValue() < 0.001) || (tfr->getMomentum().Mag() < 1))
      continue;

    //    if( tfr->getPValue() < 0.0001)
    //      continue;

    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
    RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);

    for (int cl = 0 ; cl < (int)svdClustersTrack.size(); cl++) {

      float clCharge = svdClustersTrack[cl]->getCharge();
      int clSize = svdClustersTrack[cl]->getSize();
      VxdID::baseType theVxdID = (VxdID::baseType)svdClustersTrack[cl]->getSensorID();
      int layer = VxdID(theVxdID).getLayerNumber() - 3;
      int sensor = getSensor(layer, VxdID(theVxdID).getSensorNumber());
      int side = svdClustersTrack[cl]->isUCluster();

      h_cltrkSize[layer][sensor][side]->Fill(clSize);
      //      h_cltrkChargeVSSize[layer][sensor][side]->Fill(clCharge, clSize);
      nCltrk[layer][sensor][side]++;

      if (clSize == 1) {
        h_1cltrkCharge[layer][sensor][side]->Fill(clCharge);
        h_1cltrkSN[layer][sensor][side]->Fill(svdClustersTrack[cl]->getSNR());
      }
    }

  }

  if (Tracks)
    B2DEBUG(1, "%%%%%%%% NEW EVENT,  number of Tracks =  " << Tracks.getEntries());

  //shaper digits
  for (int digi = 0 ; digi < svdShapers.getEntries(); digi++) {

    VxdID vxdid = VxdID(svdShapers[digi]->getSensorID());

    VxdID thisSensorID = svdShapers[digi]->getSensorID();
    bool thisSide = svdShapers[digi]->isUStrip();
    int thisCellID = svdShapers[digi]->getCellID();

    float ADCnoise = m_NoiseCal.getNoise(thisSensorID, thisSide, thisCellID);
    float thisNoise = m_PulseShapeCal.getChargeFromADC(thisSensorID, thisSide, thisCellID, ADCnoise);

    if (!svdShapers[digi]->isUStrip()) {
      if (((vxdid.getLayerNumber() == 3) && (svdShapers[digi]->getCellID() > 767)) ||
          ((vxdid.getLayerNumber() != 3) && (svdShapers[digi]->getCellID() > 511)))
        B2WARNING(" SVDShaperDigits: unexpected cellID for Layer " << vxdid.getLayerNumber() << " Ladder " << vxdid.getLadderNumber() <<
                  " Sensor " << vxdid.getSensorNumber() << " V side, strip = " << svdShapers[digi]->getCellID());
    } else {
      if (svdShapers[digi]->getCellID() > 767)
        B2WARNING(" SVDShaperDigits:  unexpected cellID for Layer " << vxdid.getLayerNumber() << " Ladder " << vxdid.getLadderNumber() <<
                  " Sensor " << vxdid.getSensorNumber() << " U side, strip = " << svdShapers[digi]->getCellID());
    }

    VxdID::baseType theVxdID = (VxdID::baseType)svdShapers[digi]->getSensorID();
    int layer = VxdID(theVxdID).getLayerNumber() - 3;
    int sensor = getSensor(layer, VxdID(theVxdID).getSensorNumber());
    int side = svdShapers[digi]->isUStrip();
    h_stripNoise[layer][sensor][side]->Fill(thisNoise);
    nDigi[layer][sensor][side]++;
  }


  for (int cl = 0 ; cl < svdClusters.getEntries(); cl++) {

    //    float clCharge = svdClusters[cl]->getCharge();
    //    int clSize = svdClusters[cl]->getSize();

    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(svdClusters[cl]);

    if ((int)theRC.size() < 0)
      continue;

    VxdID::baseType theVxdID = (VxdID::baseType)svdClusters[cl]->getSensorID();
    int layer = VxdID(theVxdID).getLayerNumber() - 3;
    int sensor = getSensor(layer, VxdID(theVxdID).getSensorNumber());
    int side = svdClusters[cl]->isUCluster();

    nCl[layer][sensor][side]++;

  }

  for (int i = 0; i < 4; i ++) //loop on Layers
    for (int j = 0; j < 2; j ++) //loop on Sensors
      for (int k = 0; k < 2; k ++) { //loop on Sides
        //  h_nDigi[i][j][k]->Fill(nDigi[i][j][k]);
        //  h_nCl[i][j][k]->Fill(nCl[i][j][k]);
        h_nCltrk[i][j][k]->Fill(nCltrk[i][j][k]);
      }
}


void SVDPerformanceModule::endRun()
{

}


void SVDPerformanceModule::terminate()
{

  /*
  B2RESULT(" number tracks = " << m_ntracks);
  B2RESULT(" average number of cluster per layer:");
  B2RESULT(" Layer 3, u = " << h_nCl_L3u->GetMean() << ", v = " << h_nCl_L3v->GetMean());
  B2RESULT(" Layer 4, u = " << h_nCl_L4u->GetMean() << ", v = " << h_nCl_L4v->GetMean());
  B2RESULT(" Layer 5, u = " << h_nCl_L5u->GetMean() << ", v = " << h_nCl_L5v->GetMean());
  B2RESULT(" Layer 6, u = " << h_nCl_L6u->GetMean() << ", v = " << h_nCl_L6v->GetMean());
  B2RESULT(" average cluster size per layer:");
  B2RESULT(" Layer 3, u = " << h_clSize_L3u->GetMean() << ", v = " << h_clSize_L3v->GetMean());
  B2RESULT(" Layer 4, u = " << h_clSize_L4u->GetMean() << ", v = " << h_clSize_L4v->GetMean());
  */
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;
    TObject* obj;

    TDirectory* dir_track = oldDir->mkdir("tracks");
    dir_track->cd();
    TIter nextH_track(m_histoList_track);
    while ((obj = nextH_track()))
      obj->Write();

    TDirectory* dir_shaper = oldDir->mkdir("shaper");
    dir_shaper->cd();
    TIter nextH_shaper(m_histoList_shaper);
    while ((obj = nextH_shaper()))
      obj->Write();

    TDirectory* dir_cluster = oldDir->mkdir("clusters");
    dir_cluster->cd();
    TIter nextH_cluster(m_histoList_cluster);
    while ((obj = nextH_cluster()))
      obj->Write();

    TDirectory* dir_clustertrk = oldDir->mkdir("clustersTrk");
    dir_clustertrk->cd();
    TIter nextH_clustertrk(m_histoList_clustertrk);
    while ((obj = nextH_clustertrk()))
      obj->Write();

    m_rootFilePtr->Close();
  }


}

bool SVDPerformanceModule::isDUT_L3(VxdID::baseType theVxdID)
{

  bool DUT = false;

  if (VxdID(theVxdID).getLayerNumber() == 3)
    //    if (VxdID(theVxdID).getLadderNumber() == 1)
    if (VxdID(theVxdID).getSensorNumber() == 2)
      DUT = true;

  return DUT;
}


bool SVDPerformanceModule::isDUT_L4(VxdID::baseType theVxdID)
{

  bool DUT = false;

  if (VxdID(theVxdID).getLayerNumber() == 4)
    //    if (VxdID(theVxdID).getLadderNumber() == 1)
    if (VxdID(theVxdID).getSensorNumber() == 2)
      DUT = true;


  return DUT;
}

bool SVDPerformanceModule::isDUT_L5(VxdID::baseType theVxdID)
{

  bool DUT = false;

  if (VxdID(theVxdID).getLayerNumber() == 5)
    //    if (VxdID(theVxdID).getLadderNumber() == 1)
    //      if (VxdID(theVxdID).getSensorNumber() == 2)
    DUT = true;


  return DUT;
}




bool SVDPerformanceModule::isDUT_L6(VxdID::baseType theVxdID)
{

  bool DUT = false;

  if (VxdID(theVxdID).getLayerNumber() == 6)
    //    if (VxdID(theVxdID).getLadderNumber() == 1)
    //      if (VxdID(theVxdID).getSensorNumber() == 2)
    DUT = true;


  return DUT;
}

bool SVDPerformanceModule::isDUT(VxdID::baseType theVxdID)
{

  bool DUT = false;

  if (isDUT_L3(theVxdID) || isDUT_L4(theVxdID) || isDUT_L5(theVxdID) || isDUT_L6(theVxdID))
    DUT = true;

  return DUT;
}

TH1F*  SVDPerformanceModule::createHistogram1D(const char* name, const char* title,
                                               Int_t nbins, Double_t min, Double_t max,
                                               const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}


TH2F*  SVDPerformanceModule::createHistogram2D(const char* name, const char* title,
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

TH3F*  SVDPerformanceModule::createHistogram3D(const char* name, const char* title,
                                               Int_t nbinsX, Double_t minX, Double_t maxX,
                                               const char* titleX,
                                               Int_t nbinsY, Double_t minY, Double_t maxY,
                                               const char* titleY,
                                               Int_t nbinsZ, Double_t minZ, Double_t maxZ,
                                               const char* titleZ,
                                               TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY, nbinsZ, minZ, maxZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F*  SVDPerformanceModule::createHistogram3D(const char* name, const char* title,
                                               Int_t nbinsX, Double_t* binsX,
                                               const char* titleX,
                                               Int_t nbinsY, Double_t* binsY,
                                               const char* titleY,
                                               Int_t nbinsZ, Double_t* binsZ,
                                               const char* titleZ,
                                               TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, binsX, nbinsY, binsY, nbinsZ, binsZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH1*  SVDPerformanceModule::duplicateHistogram(const char* newname, const char* newtitle,
                                               TH1* h, TList* histoList)
{

  TH1F* h1 =  dynamic_cast<TH1F*>(h);
  TH2F* h2 =  dynamic_cast<TH2F*>(h);
  TH3F* h3 =  dynamic_cast<TH3F*>(h);

  TH1* newh = 0;

  if (h1)
    newh = new TH1F(*h1);
  if (h2)
    newh = new TH2F(*h2);
  if (h3)
    newh = new TH3F(*h3);

  newh->SetName(newname);
  newh->SetTitle(newtitle);

  if (histoList)
    histoList->Add(newh);


  return newh;
}

TH1F*  SVDPerformanceModule::createHistogramsRatio(const char* name, const char* title,
                                                   TH1* hNum, TH1* hDen, bool isEffPlot,
                                                   int axisRef)
{

  TH1F* h1den =  dynamic_cast<TH1F*>(hDen);
  TH1F* h1num =  dynamic_cast<TH1F*>(hNum);
  TH2F* h2den =  dynamic_cast<TH2F*>(hDen);
  TH2F* h2num =  dynamic_cast<TH2F*>(hNum);
  TH3F* h3den =  dynamic_cast<TH3F*>(hDen);
  TH3F* h3num =  dynamic_cast<TH3F*>(hNum);

  TH1* hden = 0;
  TH1* hnum = 0;

  if (h1den) {
    hden = new TH1F(*h1den);
    hnum = new TH1F(*h1num);
  }
  if (h2den) {
    hden = new TH2F(*h2den);
    hnum = new TH2F(*h2num);
  }
  if (h3den) {
    hden = new TH3F(*h3den);
    hnum = new TH3F(*h3num);
  }

  TAxis* the_axis;
  TAxis* the_other1;
  TAxis* the_other2;

  if (axisRef == 0) {
    the_axis = hden->GetXaxis();
    the_other1 = hden->GetYaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 1) {
    the_axis = hden->GetYaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 2) {
    the_axis = hden->GetZaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetYaxis();
  } else
    return NULL;


  TH1F* h;
  if (the_axis->GetXbins()->GetSize())
    h = new TH1F(name, title, the_axis->GetNbins(), (the_axis->GetXbins())->GetArray());
  else
    h = new TH1F(name, title, the_axis->GetNbins(), the_axis->GetXmin(), the_axis->GetXmax());
  h->GetXaxis()->SetTitle(the_axis->GetTitle());

  h->GetYaxis()->SetRangeUser(0.00001, 1);

  double num = 0;
  double den = 0;
  Int_t bin = 0;
  Int_t nBins = 0;


  for (int the_bin = 1; the_bin < the_axis->GetNbins() + 1; the_bin++) {

    num = 0;
    den = 0 ;

    for (int other1_bin = 1; other1_bin < the_other1->GetNbins() + 1; other1_bin++)
      for (int other2_bin = 1; other2_bin < the_other2->GetNbins() + 1; other2_bin++) {

        if (axisRef == 0) bin = hden->GetBin(the_bin, other1_bin, other2_bin);
        else if (axisRef == 1) bin = hden->GetBin(other1_bin, the_bin, other2_bin);
        else if (axisRef == 2) bin = hden->GetBin(other1_bin, other2_bin, the_bin);

        if (hden->IsBinUnderflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), UNDERFLOW");
        if (hden->IsBinOverflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), OVERFLOW");

        num += hnum->GetBinContent(bin);
        den += hden->GetBinContent(bin);

        nBins++;

      }

    double eff = 0;
    double err = 0;

    if (den > 0) {
      eff = (double)num / den;
      err = sqrt(eff * (1 - eff)) / sqrt(den);
    }

    if (isEffPlot) {
      h->SetBinContent(the_bin, eff);
      h->SetBinError(the_bin, err);
    } else {
      h->SetBinContent(the_bin, 1 - eff);
      h->SetBinError(the_bin, err);
    }

  }

  return h;

}


void  SVDPerformanceModule::addInefficiencyPlots(TList* histoList, TH3F* h3_xPerMCParticle, TH3F* h3_MCParticle)
{

  if ((h3_xPerMCParticle == NULL) || (h3_MCParticle == NULL))
    return;

  //normalized to MCParticles
  TH1F* h_ineff_pt = createHistogramsRatio("hineffpt", "inefficiency VS pt, normalized to MCParticles", h3_xPerMCParticle,
                                           h3_MCParticle, false, 0);
  histoList->Add(h_ineff_pt);

  TH1F* h_ineff_theta = createHistogramsRatio("hinefftheta", "inefficiency VS #theta, normalized to MCParticles",
                                              h3_xPerMCParticle, h3_MCParticle, false, 1);
  histoList->Add(h_ineff_theta);

  TH1F* h_ineff_phi = createHistogramsRatio("hineffphi", "inefficiency VS #phi, normalized to MCParticles", h3_xPerMCParticle,
                                            h3_MCParticle, false, 2);
  histoList->Add(h_ineff_phi);

}

void  SVDPerformanceModule::addEfficiencyPlots(TList* histoList, TH3F* h3_xPerMCParticle, TH3F* h3_MCParticle)
{
  if ((h3_xPerMCParticle == NULL) || (h3_MCParticle == NULL))
    return;

  //normalized to MCParticles
  TH1F* h_eff_pt = createHistogramsRatio("heffpt", "efficiency VS pt, normalized to MCParticles", h3_xPerMCParticle,
                                         h3_MCParticle, true, 0);
  histoList->Add(h_eff_pt);

  TH1F* h_eff_theta = createHistogramsRatio("hefftheta", "efficiency VS #theta, normalized to MCParticles", h3_xPerMCParticle,
                                            h3_MCParticle, true, 1);
  histoList->Add(h_eff_theta);

  TH1F* h_eff_phi = createHistogramsRatio("heffphi", "efficiency VS #phi, normalized to MCParticles", h3_xPerMCParticle,
                                          h3_MCParticle, true, 2);
  histoList->Add(h_eff_phi);

}



void  SVDPerformanceModule::addPurityPlots(TList* histoList, TH3F* h3_MCParticlesPerX, TH3F* h3_X)
{
  if ((h3_X == NULL) || (h3_MCParticlesPerX == NULL))
    return;

//purity histograms
  TH1F* h_pur_pt = createHistogramsRatio("hpurpt", "purity VS pt", h3_MCParticlesPerX, h3_X, true, 0);
  histoList->Add(h_pur_pt);

  TH1F* h_pur_theta = createHistogramsRatio("hpurtheta", "purity VS #theta", h3_MCParticlesPerX, h3_X, true, 1);
  histoList->Add(h_pur_theta);

  TH1F* h_pur_phi = createHistogramsRatio("hpurphi", "purity VS #phi", h3_MCParticlesPerX, h3_X, true, 2);
  histoList->Add(h_pur_phi);

}




