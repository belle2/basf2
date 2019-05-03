#include <svd/modules/svdCalibration/SVDHotStripFinderModule.h>
#include <geometry/GeometryManager.h>
#include <time.h>
#include <list>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/foreach.hpp>

using namespace std;

using namespace Belle2;

REG_MODULE(SVDHotStripFinder)
SVDHotStripFinderModule::SVDHotStripFinderModule() : Module()
{
  setDescription("The svdHotStripFinder module finds hot strips in SVD data SVDShaperDigit");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDHotStripFinder.root"));
  addParam("threshold", m_thr, "Threshold cut for Hot strip finder in percent", float(0.75));
  addParam("searchBase", m_base, "0 -> 32, 1 -> 64, 2 -> 128", int(0));

}


SVDHotStripFinderModule::~SVDHotStripFinderModule()
{
}


void SVDHotStripFinderModule::initialize()
{

  m_eventMetaData.isRequired();
  m_storeDigits.isRequired(m_ShaperDigitName);

  B2DEBUG(25, "    ShaperDigits: " << m_ShaperDigitName);

  m_histoList_occu = new TList;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");


}
void SVDHotStripFinderModule::beginRun()
{

  //create histograms

  TH1F hOccupancy768("Occupancy768_L@layerL@ladderS@sensor@view", "DSSD Occupancy of @layer.@ladder.@sensor @view/@side side", 768, 0,
                     768);
  hOccupancy768.GetXaxis()->SetTitle("cellID");
  TH1F hOccupancy512("Occupancy512_L@layerL@ladderS@sensor@view", "DSSD Occupancy of @layer.@ladder.@sensor @view/@side side", 512, 0,
                     512);
  hOccupancy512.GetXaxis()->SetTitle("cellID");
  hm_occupancy = new SVDHistograms<TH1F>(hOccupancy768, hOccupancy768, hOccupancy768, hOccupancy512);

  TH1F hHotStrips768("HotStrips768_L@layerL@ladderS@sensor@view", "DSSD HotStrips of @layer.@ladder.@sensor @view/@side side", 768, 0,
                     768);
  hHotStrips768.GetXaxis()->SetTitle("cellID");
  TH1F hHotStrips512("HotStrips512_L@layerL@ladderS@sensor@view", "DSSD HotStrips of @layer.@ladder.@sensor @view/@side side", 512, 0,
                     512);
  hHotStrips512.GetXaxis()->SetTitle("cellID");
  hm_hot_strips = new SVDHistograms<TH1F>(hHotStrips768, hHotStrips768, hHotStrips768, hHotStrips512);

  TH1F hOccupancy_after768("OccupancyAfter768_L@layerL@ladderS@sensor@view",
                           "DSSD Occupancy after HSF of @layer.@ladder.@sensor @view/@side side", 768, 0, 768);
  hOccupancy_after768.GetXaxis()->SetTitle("cellID");
  TH1F hOccupancy_after512("OccupancyAfter512_L@layerL@ladderS@sensor@view",
                           "DSSD Occupancy after HSF of @layer.@ladder.@sensor @view/@side side", 512, 0, 512);
  hOccupancy_after512.GetXaxis()->SetTitle("cellID");
  hm_occupancy_after = new SVDHistograms<TH1F>(hOccupancy_after768, hOccupancy_after768, hOccupancy_after768, hOccupancy_after512);

  TH1F hDist("dist_L@layerL@ladderS@sensor@view", "DSSD occupancy distribution of @layer.@ladder.@sensor @view/@side side", 100, 0,
             0.05);
  hDist.GetXaxis()->SetTitle("occupancy");
  hm_dist = new SVDHistograms<TH1F>(hDist);

  TH1F hDist1("dist1_L@layerL@ladderS@sensor@view", "DSSD true occupancy distribution of @layer.@ladder.@sensor @view/@side side",
              100, 0, 0.05);
  hm_dist1 = new SVDHistograms<TH1F>(hDist1);
  hDist.GetXaxis()->SetTitle("occupancy");

  TH2F hDist12("dist2d_L@layerL@ladderS@sensor@view",
               "DSSD true vs sensor occupancy distribution of @layer.@ladder.@sensor @view/@side side", 1000, 0, 0.05, 1000, 0, 0.05);
  hDist12.GetXaxis()->SetTitle("sensor occupancy");
  hDist12.GetYaxis()->SetTitle("occupancy");
  hm_dist12 = new SVDHistograms<TH2F>(hDist12);

  //summary plot of the hot strips per sensor
  m_hHotStripsSummary = new SVDSummaryPlots("hotStripsSummary@view", "Number of HotStrips on @view/@side Side");


  // DQM style historgram  number of hs vs sensor plane
  h_tot_dqm =  createHistogram1D("htodqm", "HS per sensor", 28, 0, 28.0, "HS per sensor", m_histoList_occu);
  h_tot_dqm1 =  createHistogram1D("htodqm1", "HS per sensor1", 350, 0, 350.0, "HS per sensor ", m_histoList_occu);

  h_tot_dist = createHistogram1D("htotdist", "Occupancy distribution", 1000, 0, 0.05, "Relative occupancy", m_histoList_occu);
  h_tot_dist1 = createHistogram1D("htotdist1", "True occupancy distribution", 1000, 0, 0.05, "occupancy", m_histoList_occu);
  h_tot_dist12 = createHistogram2D("htotdist2d", "True vs sensor occupancy distribution", 1000, 0, 0.05, "sensor occupancy", 1000, 0,
                                   0.05, "occupancy", m_histoList_occu);
  h_nevents = createHistogram1D("hnevents", "Number of events", 1, 0, 1, "", m_histoList_occu);

}

void SVDHotStripFinderModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  int nDigits = m_storeDigits.getEntries();
  h_nevents->Fill(0.0);

  if (nDigits == 0)
    return;

  //loop over the SVDShaperDigits
  int i = 0;
  while (i < nDigits) {
    VxdID theVxdID = m_storeDigits[i]->getSensorID();
    int side = m_storeDigits[i]->isUStrip();
    int CellID = m_storeDigits[i]->getCellID();

    hm_occupancy->fill(theVxdID, side, CellID);

    i++;
  }



}

void SVDHotStripFinderModule::endRun()
{
}

void SVDHotStripFinderModule::terminate()
{

  TDirectory* oldDir = NULL;
  //prepare ROOT FILE
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    oldDir = gDirectory;
    TDirectory* dir_occu = oldDir->mkdir("occupancy");
    dir_occu->cd();
  }



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

  B2DEBUG(1, "number of events " << nevents);

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

            position1[l] = hm_occupancy->getHistogram(*itSvdSensors, k)->GetBinContent(l);
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

          for (int l = 0; l < 768; l++) {
            div_t test = div(l, ibase);
            float tmp_occ = position1[l] / (float)nCltrk[test.quot]; //for hot strip search
            float  tmp_occ1 = position1[l] / (float)nevents; //for Giuliana - occupancy distribution
            position1[l] = tmp_occ; //vector used for Hot strip search
            if (tmp_occ > 0.0) {
              hm_dist->fill(*itSvdSensors, k, tmp_occ); // ..
              h_tot_dist->Fill(tmp_occ);
              hm_dist1->fill(*itSvdSensors, k, tmp_occ1); //occupancy as probablity to fire the strip
              h_tot_dist1->Fill(tmp_occ1);
              hm_dist12->fill(*itSvdSensors, k, tmp_occ, tmp_occ1); // 2D distribution
              h_tot_dist12->Fill(tmp_occ, tmp_occ1);
            }
          }

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
              B2RESULT("1st pass HS found! Layer: " << i + 3 << " Ladder: " << m << " Sensor: " << j << " Side: " << k << " channel: " << l);
            } else {
              hsflag[l] = 0; // not a HS
              //recalculate the occupancy in DSSD only for good strip after first pass
              occupancy[test.quot] = occupancy[test.quot] + hm_occupancy->getHistogram(*itSvdSensors, k)->GetBinContent(l);
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
              B2RESULT("2nd pass HS FOUND! Layer: " << i + 3 << " Ladder: " << m << " Sensor: " << j << " Side: " << k << " channel: " << l);
            }
          }
          /* for Laura .. HS flags, place interface for DB */
          for (int l = 0; l < 768; l++) {
            B2DEBUG(1, hsflag[l]);
            if (flag[l] == 1) {
              hm_occupancy_after->getHistogram(*itSvdSensors, k)->SetBinContent(l, hm_occupancy->getHistogram(*itSvdSensors,
                  k)->GetBinContent(l));
            } else
              hm_hot_strips->getHistogram(*itSvdSensors, k)->SetBinContent(l, 1);
          }

          if (m_rootFilePtr != NULL) {
            hm_occupancy->getHistogram(*itSvdSensors, k)->Write();
            hm_hot_strips->getHistogram(*itSvdSensors, k)->Write();
            hm_occupancy_after->getHistogram(*itSvdSensors, k)->Write();
            hm_dist12->getHistogram(*itSvdSensors, k)->Write();
          }

          B2DEBUG(1, " side " << i << " " << j << " " << m << " " << k);

          /* end */

          for (int iy = 0; iy < iths; iy++) {
            h_tot_dqm->Fill(float(itsensor));
            h_tot_dqm1->Fill(float(itsensor));
          }

          for (int s = 0; s < hm_hot_strips->getHistogram(*itSvdSensors, k)->GetEntries(); s++)
            m_hHotStripsSummary->fill(*itSvdSensors, k, 1);

          itsensor++;
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;

  }

  if (m_rootFilePtr != NULL) {
    oldDir->cd();

    m_hHotStripsSummary->getHistogram(0)->Write();
    m_hHotStripsSummary->getHistogram(1)->Write();

    TObject* obj;
    TIter nextH_occu(m_histoList_occu);
    while ((obj = nextH_occu()))
      obj->Write();

    m_rootFilePtr->Close();
  }
}

TH1F*  SVDHotStripFinderModule::createHistogram1D(const char* name, const char* title,
                                                  Int_t nbins, Double_t min, Double_t max,
                                                  const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);


  return h;
}


TH2F*  SVDHotStripFinderModule::createHistogram2D(const char* name, const char* title,
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
