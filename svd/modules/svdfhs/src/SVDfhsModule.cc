#include <svd/modules/svdfhs/inc/SVDfhsModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <geometry/GeometryManager.h>
#include <framework/dataobjects/EventMetaData.h>
#include <time.h>
#include <list>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/foreach.hpp>

using namespace std;

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
REG_MODULE(SVDfhs)
SVDfhsModule::SVDfhsModule() : Module()
{
  setDescription("The svdfhs module finds hot strips in SVD data SVDShaperDigit");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDfhs_output.root"));

  addParam("is2017TBanalysis", m_is2017TBanalysis, "True if analyzing 2017 TB data.", bool(false));
  addParam("mfhsThreshold", m_thr, "Threshold cut for Hot strip finder in percent", float(0.75));
  addParam("searchBase", m_base, "Threshold cut for Hot strip finder in percent", int(0));

}


SVDfhsModule::~SVDfhsModule()
{
}


void SVDfhsModule::initialize()
{
  //StoreObjPtr<EventMetaData>::required();
  //StoreArray<SVDShaperDigit>::required(m_ShaperDigitName);
  //StoreArray<SVDRecoDigit>::optional(m_RecoDigitName);
  StoreArray<SVDRecoDigit> storeDigits(m_RecoDigitName);


  B2INFO("    ShaperDigits: " << m_ShaperDigitName);
  B2INFO("      RecoDigits: " << m_RecoDigitName);
  B2INFO("        Clusters: " << m_ClusterName);


  m_histoList_occu = new TList;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");


}
void SVDfhsModule::beginRun()
{
  TString NameOfHisto = "";
  TString TitleOfHisto = "";
  for (int s = 0; s < m_nLayers; s++)
    if (m_is2017TBanalysis)
      sensorsOnLayer[s] = s + 2; //change
    else
      sensorsOnLayer[s] = s + 2;

  //create histograms

//call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();
//loop to create the initial histos
  while ((itSvdLayers != svdLayers.end())
         && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers
    //  for (int i = 0; i < m_nLayers; i ++) //loop on Layers
    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        //    for (int j = 0; j < (int)sensorsOnLayer[i]; j ++) //loop on Sensors
        for (int k = 0; k < m_nSides; k ++) { //loop on Sides

          /* we start indexing from 0 to avoid empty histograms */
          int layer = itSvdSensors->getLayerNumber() - 3;
          int ladder =  itSvdSensors->getLadderNumber() - 1;
          int sensor = itSvdSensors->getSensorNumber() - 1;
          //std::cout<< layer << " " << ladder << " " << sensor << std::endl;

          TString nameLayer = "";
          nameLayer += layer;

          TString nameLadder = "";
          nameLadder += ladder;

          TString nameSensor = "";
          nameSensor += sensor;

          TString nameSide = "";
          if (k == 1)
            nameSide = "U";
          else if (k == 0)
            nameSide = "V";
          TString nameLayerb = "";
          nameLayerb += (layer + 3);

          NameOfHisto = "DSSD_Occupancy_L" + nameLayerb + "L" + nameLadder + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "DSSD_occupancy (L" + nameLayerb + "L" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_occupancy[layer][ladder][sensor][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 768, 0, 768, "strip", m_histoList_occu);
          NameOfHisto = "DSSD_Occupancy_after_HSS_L" + nameLayerb + "L" + nameLadder + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "DSSD_occupancy after HSS (L" + nameLayerb + "L" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_occupancy_after[layer][ladder][sensor][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 768, 0, 768, "strip", m_histoList_occu);
          NameOfHisto = "DSSD_Occupancy_dist_L" + nameLayerb + "L" + nameLadder  + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "DSSD occupancy distribution (L" + nameLayerb + "L" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_dist[layer][ladder][sensor][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 0.05, "Relative occupancy",
                                                               m_histoList_occu);
          NameOfHisto = "DSSD_Occupancy_dist1_L" + nameLayerb + "L" + nameLadder + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "DSSD true occupancy distribution (L" + nameLayerb + "L" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_dist1[layer][ladder][sensor][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 0.05, "occupancy", m_histoList_occu);
          NameOfHisto = "DSSD_Occupancy_dist2d_L" + nameLayerb  + "L" + nameLadder + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "DSSD true vs sensor occupancy distribution (L" + nameLayerb  + "L" + nameLadder + ", sensor" + nameSensor + "," +
                         nameSide + " side)";
          h_dist12[layer][ladder][sensor][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 1000, 0, 0.05, "sensor occupancy ", 1000, 0, 0.05,
                                                                 "occupancy ", m_histoList_occu);

          // TH1F* lowChrgClsPosition[nSensors][nSides];

        }
        //histogram created
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }
  // DQM style historgram  number of hs vs sensor plane
  h_tot_dqm =  createHistogram1D("htodqm", "HS per sensor", 28, 0, 28.0, "HS per sensor", m_histoList_occu);
  h_tot_dqm1 =  createHistogram1D("htodqm1", "HS per sensor1", 350, 0, 350.0, "HS per sensor ", m_histoList_occu);

  h_tot_dist = createHistogram1D("htotdist", "Occupancy distribution", 1000, 0, 0.05, "Relative occupancy", m_histoList_occu);
  h_tot_dist1 = createHistogram1D("htotdist1", "True occupancy distribution", 1000, 0, 0.05, "occupancy", m_histoList_occu);
  h_tot_dist12 = createHistogram2D("htotdist2d", "True vs sensor occupancy distribution", 1000, 0, 0.05, "sensor occupancy", 1000, 0,
                                   0.05, "occupancy", m_histoList_occu);
  h_nevents = createHistogram1D("hnevets", "Number of events", 1000, 0, 0.05, "something", m_histoList_occu);



}

//void SVDfhsModule::beginRun()
//{
//}

void SVDfhsModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;


  const StoreArray<SVDShaperDigit> storeDigits(m_ShaperDigitName);
  int nDigits = storeDigits.getEntries();
  h_nevents->Fill(0.0);
  if (nDigits == 0)
    return;
  //loop over the SVDRecoDigits
  int i = 0;
  while (i < nDigits) {
    VxdID theVxdID = storeDigits[i]->getSensorID();

    int layer = VxdID(theVxdID).getLayerNumber() - 3; //starting from 0
    int ladder = VxdID(theVxdID).getLadderNumber() - 1; // starting from 0
    int sensor = theVxdID.getSensorNumber() - 1; //starting from 0
    int side = storeDigits[i]->isUStrip();
    int CellID = storeDigits[i]->getCellID();

    h_occupancy[layer][ladder][sensor][side]->Fill(CellID);

    i++;
  }



}

void SVDfhsModule::endRun()
{
}

void SVDfhsModule::terminate()
{
  /**************************************************************************
  * Hotstrips finding algorithm                                             *
  ***************************************************************************/

  //Find low charged clusters with high occupancy.
  int flag[768];
  int hsflag[768];
  int nevents =  h_nevents->GetEntries();
  int ibase = 768;
  if (m_base == 1) {ibase = 32;};
  if (m_base == 2) {ibase = 64;};
  if (m_base == 3) {ibase = 128;};

  std::cout << "number of events " << nevents << std::endl;

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();
  int itsensor = 0; //sensor numbering
  while ((itSvdLayers != svdLayers.end())
         && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) { //loop on sensors

        for (int k = 0; k < m_nSides; k ++) { //loop on Sides

          /* we start indexing from 0 to avoid empty histograms */
          int i = itSvdSensors->getLayerNumber() - 3;
          int m =  itSvdSensors->getLadderNumber() - 1;
          int j = itSvdSensors->getSensorNumber() - 1;
          float position1[768];
          float nCltrk[24];
          int it = 0;
          int iths = 0;
          for (int l = 0; l < 24; l++) {
            nCltrk[l] = 0.0;
          }

          for (int l = 0; l < 768; l++) {

            position1[l] = h_occupancy[i][m][j][k]->GetBinContent(l);
            /*if no hits in strip, mark the strip as bad */

            if (position1[l] == 0) { flag[l] = 0;}
            else {
              flag[l] = 1;
              it++; //number of good (non zero ) strips
              //nCltrk = nCltrk + position1[l]; //number of digits in sensor
              div_t test = div(l, ibase);
              nCltrk[test.quot] = nCltrk[test.quot] + position1[l];
            }

          }

          //  float average=nCltrk/(float)it; //average per alive strip

          for (int l = 0; l < 768; l++) {
            div_t test = div(l, ibase);
            float tmp_occ = position1[l] / (float)nCltrk[test.quot]; //for hot strip search
            float  tmp_occ1 = position1[l] / (float)nevents; //for Giuliana - occupancy distribution
            position1[l] = tmp_occ; //vector used for Hot strip search
            if (tmp_occ > 0.0) {
              h_dist[i][m][j][k]->Fill(tmp_occ); // ..
              h_tot_dist->Fill(tmp_occ);
              h_dist1[i][m][j][k]->Fill(tmp_occ1); //occupancy as probablity to fire the strip
              h_tot_dist1->Fill(tmp_occ1);
              h_dist12[i][m][j][k]->Fill(tmp_occ, tmp_occ1); // 2D distribution
              h_tot_dist12->Fill(tmp_occ, tmp_occ1);
            }
          }
          //cout <<"average =" << average <<endl;
          float occupancy[24]; //occupancy for second pass
          for (int l = 0; l < 24; l++) {
            occupancy[l] = 0.0;
          }
          it = 0;
          //iths=0;

          // first pass
          for (int l = 0; l < 768; l++) {
            div_t test = div(l, ibase);
            float threshold_corrections = 1.0;
            if (ibase == 32) threshold_corrections = 24.0;
            if (ibase == 64) threshold_corrections = 12.0;
            if (ibase == 128) threshold_corrections = 6.0;

            if (position1[l] > 0.01 * m_thr * threshold_corrections) { // if probablity is larger then 1% in given dssd mark it as

              hsflag[l] = 1; // HS
              flag[l] = 0;
              iths++;
              std::cout << "Layer: " << i + 2 << " Ladder: " << m << " Sensor: " << j << " Side: " << k << " channel: " << l << std::endl;
            } else {
              hsflag[l] = 0; // not a HS
              //recalculate the occupancy in DSSD only for good strip after first pass
              occupancy[test.quot] = occupancy[test.quot] + h_occupancy[i][m][j][k]->GetBinContent(l);
              it++; //number of good strips after first pass
            }

          }
          // second pass
          for (int l = 0; l < 768; l++) {
            div_t test = div(l, ibase);
            position1[l] = position1[l] * nCltrk[test.quot] / (float)occupancy[test.quot];
            float threshold_corrections = 1.0;
            if (ibase == 32) threshold_corrections =  24.0;
            if (ibase == 64) threshold_corrections =  12.0;
            if (ibase == 128) threshold_corrections = 6.0;

            if ((flag[l]) && (position1[l] > 0.01 * m_thr * threshold_corrections)) { //HS
              hsflag[l] = 1;
              flag[l] = 0;
              iths++;
              std::cout << "2nd pass Layer: " << i + 2 << " Ladder: " << m << " Sensor: " << j << " Side: " << k << " channel: " << l <<
                        std::endl;
            }
          }
          /* for Laura .. HS flags, place interface for DB */
          for (int l = 0; l < 768; l++) {
            std::cout << hsflag[l] << " ";
            if (flag[l])h_occupancy_after[i][m][j][k]->SetBinContent(l, h_occupancy[i][m][j][k]->GetBinContent(l));
          }
          std::cout << std::endl;
          //    std::cout << " side " << i << " " << j <<" "<< m << " " << k << std::endl;
          /* end */
          for (int iy = 0; iy < iths; iy++) {
            h_tot_dqm->Fill(float(itsensor));
            h_tot_dqm1->Fill(float(itsensor));
          }

          itsensor++;
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;

  }

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    TDirectory* oldDir = gDirectory;
    TObject* obj;

    TDirectory* dir_occu = oldDir->mkdir("occupancy");
    dir_occu->cd();
    TIter nextH_occu(m_histoList_occu);
    while ((obj = nextH_occu()))
      obj->Write();



  }

  m_rootFilePtr->Close();

}

TH1F*  SVDfhsModule::createHistogram1D(const char* name, const char* title,
                                       Int_t nbins, Double_t min, Double_t max,
                                       const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);


  return h;
}


TH2F*  SVDfhsModule::createHistogram2D(const char* name, const char* title,
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

TH3F*  SVDfhsModule::createHistogram3D(const char* name, const char* title,
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

TH3F*  SVDfhsModule::createHistogram3D(const char* name, const char* title,
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

TH1*  SVDfhsModule::duplicateHistogram(const char* newname, const char* newtitle,
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

TH1F*  SVDfhsModule::createHistogramsRatio(const char* name, const char* title,
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
  Int_t nnBins = 0;


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


        nnBins++;

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



