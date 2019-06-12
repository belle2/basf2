#include <geometry/GeometryManager.h>
#include <time.h>
#include <list>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/PayloadFile.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <svd/calibration/SVDOccupancyCalibrations.h>
#include <svd/modules/svdCalibration/SVDHotStripFinderModule.h>


#include <boost/foreach.hpp>

using namespace std;

using namespace Belle2;

REG_MODULE(SVDHotStripFinder)
SVDHotStripFinderModule::SVDHotStripFinderModule() : Module()
{
  setDescription("The svdHotStripFinder module finds hot strips in SVD data SVDShaperDigit");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDHotStripFinder.root"));
  addParam("threshold", m_thr, "Threshold cut for Hot strip finder in percent", float(4.0));
  addParam("searchBase", m_base, "0 -> 32, 1 -> 64, 2 -> 128", int(0));
  //additional paramters to import on the DB:
  addParam("firstExp", m_firstExp, "experiment number");
  addParam("firstRun", m_firstRun, "run number");
  addParam("lastExp", m_lastExp, "open iov", int(-1));
  addParam("lastRun", m_lastRun, "open iov", int(-1));
  addParam("ShaperDigits", m_ShaperDigitName, "shaper digit name", std::string(""));
  //new parameters for HSFinderV2:
  addParam("useHSFinderV1", m_useHSFinderV1, "Set to false only if you want to test the second version of the algorithm", bool(true));
  addParam("absOccThreshold", m_absThr,
           "Absolute occupancy threshold: at a first loop, flag as Hot Strip (HS) all those whose occupancy > absOccThreshold", float(0.2));
  addParam("relOccPrec", m_relOccPrec,
           "Precision level on occupancy which is defined to be negligible the calculation of the hit background rate", float(0.1));


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
  if (m_useHSFinderV1)
    B2RESULT("You are using the first version of the HSFinder algorithm (see SVDHotStripFinder::terminate in the module)");
  else  B2RESULT("You are using the modified version of the HSFinder algorithm (see SVDHotStripFinder::endRun in the module)");

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

  TH1F hOccAll("occAll_L@layerL@ladderS@sensor@view", "Strip Occupancy Distribution of @layer.@ladder.@sensor @view/@side side", 200,
               0, m_absThr);
  hOccAll.GetXaxis()->SetTitle("occupancy");
  hm_occAll = new SVDHistograms<TH1F>(hOccAll);

  TH1F hOccHot("occHot_L@layerL@ladderS@sensor@view", "Hot Strip Occupancy Distribution of @layer.@ladder.@sensor @view/@side side",
               200, 0, 1);
  hOccHot.GetXaxis()->SetTitle("occupancy");
  hm_occHot = new SVDHistograms<TH1F>(hOccHot);

  TH1F hOccAfter("occAfter_L@layerL@ladderS@sensor@view",
                 "Non-Hot Strip Occupancy Distribution of @layer.@ladder.@sensor @view/@side side", 200, 0, m_absThr);
  hOccAfter.GetXaxis()->SetTitle("occupancy");
  hm_occAfter = new SVDHistograms<TH1F>(hOccAfter);

  //
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
  h_nevents->Fill(0.0); // number of events count

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

  if (!m_useHSFinderV1) {

    TDirectory* oldDir = NULL;
    TDirectory* dir_occuL[4] = {NULL, NULL, NULL, NULL};

    //prepare ROOT FILE
    if (m_rootFilePtr != NULL) {
      m_rootFilePtr->cd();
      oldDir = gDirectory;
      dir_occuL[0] = oldDir->mkdir("layer3");
      dir_occuL[1] = oldDir->mkdir("layer4");
      dir_occuL[2] = oldDir->mkdir("layer5");
      dir_occuL[3] = oldDir->mkdir("layer6");
    }

    //Scale strip occupancy plots (per each sensor side) by the number of events. Fill SVDOccupancyCalibrations payload with the measured strip occupancy.

    int hsflag[768]; //found hot strips list;  hsflag[i]==1 for indetified Hot Strip
    int nevents =  h_nevents->GetEntries(); //number of events processed in events

    //Define the DBObj pointers to create the needed payloads

    DBImportObjPtr< SVDOccupancyCalibrations::t_payload> occDBObjPtr(SVDOccupancyCalibrations::name);
    occDBObjPtr.construct(-99.);

    DBImportObjPtr< SVDHotStripsCalibrations::t_payload> hotStripsDBObjPtr(SVDHotStripsCalibrations::name);
    hotStripsDBObjPtr.construct(0);

    B2RESULT("number of events " << nevents);

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

          for (int k = 0; k < m_nSides; k ++) { //loop on Sides , k = isU(), k=0 is v-side, k=1 is u-side

            /* we start indexing from 0 to avoid empty histograms */
            int layer = itSvdSensors->getLayerNumber();
            int ladder =  itSvdSensors->getLadderNumber();
            int sensor = itSvdSensors->getSensorNumber();

            // int nafter =0; //number of good strips after first preselection cut
            int nstrips = 768;
            if (!k && layer != 3) nstrips = 512;

            float stripOcc[768];
            for (int i = 0; i < nstrips; i++) {stripOcc[i] = 0; hsflag[i] = 0;} //initialize vector to zero
            float stripOccAfterAbsCut[768]; // vector of strip occupancy after first preselection based on absOccupThres cut
            (hm_occupancy->getHistogram(*itSvdSensors, k))->Scale(1. / nevents);
            for (int l = 0; l < nstrips; l++) {

              //normalized to the total number of events to have the correct occupancy per strip and fill the corresponding dbobject

              stripOcc[l] = (float)(hm_occupancy->getHistogram(*itSvdSensors, k)->GetBinContent(l + 1));

              //0. Fill SVDOccupancyCalibrations Payload with the measured strip occupancy
              occDBObjPtr->set(layer, ladder, sensor, k, l, stripOcc[l]);
              hm_occAll->fill(*itSvdSensors, k, stripOcc[l]);

              //1. Cut based  on absOccupancyThreshold
              if (stripOcc[l] > m_absThr) {
                stripOccAfterAbsCut[l] = 0;
                hsflag[l] = 1;

              } else {
                stripOccAfterAbsCut[l] = stripOcc[l];
                hsflag[l] = 0;
              }
              B2DEBUG(1, "Measured strip occupancy for strip " << l << ":" << stripOccAfterAbsCut[l]);
            }

            // 2. flag hot strips that has occ_Strip > sensor average Occupancy * relOccPrec
            bool moreHS = true;

            while (moreHS && theHSFinder(stripOccAfterAbsCut, hsflag, nstrips)) {
              moreHS = theHSFinder(stripOccAfterAbsCut, hsflag, nstrips);
            }

            //3. after second step: fill HS histograms and occupancy histograms of survived strips; fill HS payload, SVDHotStripsCalibrations
            for (int l = 0; l < nstrips; l++) {
              hotStripsDBObjPtr->set(layer, ladder, sensor, k, l, hsflag[l]);
              if (hsflag[l] == 0) {
                hm_occupancy_after->getHistogram(*itSvdSensors, k)->SetBinContent(l + 1 , stripOccAfterAbsCut[l]);
                hm_occAfter->fill(*itSvdSensors, k, stripOccAfterAbsCut[l]);
              } else {
                hm_hot_strips->getHistogram(*itSvdSensors, k)->SetBinContent(l + 1, 1);
                hm_occHot->fill(*itSvdSensors, k, stripOcc[l]);
              }
            }

            for (int s = 0; s < hm_hot_strips->getHistogram(*itSvdSensors, k)->GetEntries(); s++)
              m_hHotStripsSummary->fill(*itSvdSensors, k, 1);

            if (m_rootFilePtr != NULL) {
              dir_occuL[layer - 3]->cd();
              hm_occupancy->getHistogram(*itSvdSensors, k)->Write();
              hm_hot_strips->getHistogram(*itSvdSensors, k)->SetLineColor(kBlack);
              hm_hot_strips->getHistogram(*itSvdSensors, k)->SetMarkerColor(kBlack);
              hm_hot_strips->getHistogram(*itSvdSensors, k)->Write();
              hm_occupancy_after->getHistogram(*itSvdSensors, k)->SetLineColor(kRed);
              hm_occupancy_after->getHistogram(*itSvdSensors, k)->SetMarkerColor(kRed);
              hm_occupancy_after->getHistogram(*itSvdSensors,  k)->Write();
              hm_occAll->getHistogram(*itSvdSensors,  k)->Write();
              hm_occHot->getHistogram(*itSvdSensors,  k)->SetLineColor(kBlack);
              hm_occHot->getHistogram(*itSvdSensors,  k)->SetMarkerColor(kBlack);
              hm_occHot->getHistogram(*itSvdSensors,  k)->Write();
              hm_occAfter->getHistogram(*itSvdSensors,  k)->SetLineColor(kRed);
              hm_occAfter->getHistogram(*itSvdSensors,  k)->SetMarkerColor(kRed);
              hm_occAfter->getHistogram(*itSvdSensors,  k)->Write();
            }

            B2DEBUG(1, " L" << layer << "." << ladder << "." << sensor << ".isU=" << k);

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
    }

    m_rootFilePtr->Close();
//import the filled dbobjects to the ConditionDB
    IntervalOfValidity iov(m_firstExp, m_firstRun, m_lastExp, m_lastRun);
    occDBObjPtr.import(iov);
    hotStripsDBObjPtr.import(iov);
    B2RESULT("Imported to database.");
  }
}

void SVDHotStripFinderModule::terminate()
{
  if (m_useHSFinderV1) {
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
    int flag[768]; // list of working (non zero) strips
    int hsflag[768]; //found hot strips list;  hsflag[i]==1 for indetified Hot Strip
    int nevents =  h_nevents->GetEntries(); //number of events processed in events
    int ibase = 768; // interval used for the hot strip finding
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

            // we start indexing from 0 to avoid empty histograms
            int i = itSvdSensors->getLayerNumber() - 3;
            int m =  itSvdSensors->getLadderNumber() - 1;
            int j = itSvdSensors->getSensorNumber() - 1;
            float position1[768]; // vector of hits in the sensor
            float nCltrk[24]; // index to interval if we search in smaller intervals then full sensor
            int it = 0;
            int iths = 0;
            // it is safer to initialize the nCltrk vector
            for (int l = 0; l < 24; l++) {
              nCltrk[l] = 0.0;
            }

            for (int l = 0; l < 768; l++) {

              position1[l] = hm_occupancy->getHistogram(*itSvdSensors, k)->GetBinContent(l);
              //if no hits in strip, mark the strip as bad

              if (position1[l] == 0) { flag[l] = 0;}
              else {
                flag[l] = 1;
                it++; //number of good (non zero ) strips
                // find in which interval the strip lays
                div_t test = div(l, ibase);
                nCltrk[test.quot] = nCltrk[test.quot] + position1[l]; // number of entries in given interval
              }

            }

            for (int l = 0; l < 768; l++) {
              div_t test = div(l, ibase);

              //     tmp_occ - relative occupancy for Hot Strip search interval,  for channel l
              //   tmp_occ1 - occupancy

              float tmp_occ = position1[l] / (float)nCltrk[test.quot]; //for hot strip search
              float  tmp_occ1 = position1[l] / (float)nevents; //for SVDOccupancyCalibration
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
            // first pass
            for (int l = 0; l < 768; l++) {
              div_t test = div(l, ibase);
              float threshold_corrections = 1.0;
              if (ibase == 32) threshold_corrections = 24.0;
              if (ibase == 64) threshold_corrections = 12.0;
              if (ibase == 128) threshold_corrections = 6.0;

              if (position1[l] > 0.01 * m_thr * threshold_corrections) { // if probablity is larger then threshold mark as Hot strip
                hsflag[l] = 1; // HS vector
                flag[l] = 0; // mark strip as bad for second pass
                iths++;
                B2RESULT("1st pass HS found! Layer: " << i + 3 << " Ladder: " << m << " Sensor: " << j << " Side: " << k << " channel: " << l);
              } else {
                hsflag[l] = 0; // not a HS
                //recalculate the occupancy in DSSD only for good strip after first pass
                occupancy[test.quot] = occupancy[test.quot] + hm_occupancy->getHistogram(*itSvdSensors, k)->GetBinContent(l);
                it++; //number of good strips after first pass
              }

            }
            // Second pass of Hot strip finder, After the first pass we remove already found Host strips we do the second pass with the same threshold

            // second pass
            for (int l = 0; l < 768; l++) {
              div_t test = div(l, ibase);
              position1[l] = position1[l] * nCltrk[test.quot] / (float)occupancy[test.quot];
              float threshold_corrections = 1.0;
              if (ibase == 32) threshold_corrections =  24.0;
              if (ibase == 64) threshold_corrections =  12.0;
              if (ibase == 128) threshold_corrections = 6.0;

              if ((flag[l]) && (position1[l] > 0.01 * m_thr * threshold_corrections)) { //HS
                hsflag[l] = 1;// HS vector
                flag[l] = 0; // mark strip as bad after second pass
                iths++;
                B2RESULT("2nd pass HS FOUND! Layer: " << i + 3 << " Ladder: " << m << " Sensor: " << j << " Side: " << k << " channel: " << l);
              }
            }
            // for Laura .. HS flags, place interface for DB
            // outputs : hsflag[l]  1- HS 0- non HS, flag[l]    0- bad strip, 1 working strip, h_tot_dist1   occupancy as probablity of firing the strip


            for (int l = 0; l < 768; l++) {
              B2DEBUG(1, hsflag[l]);
              if (hsflag[l] == 0) {
                hm_occupancy_after->getHistogram(*itSvdSensors, k)->SetBinContent(l, hm_occupancy->getHistogram(*itSvdSensors,
                    k)->GetBinContent(l)); //alive strips without identified HS
              } else
                hm_hot_strips->getHistogram(*itSvdSensors, k)->SetBinContent(l, 1);  // Not only hot strips but also masked strips
              // to get only  hot strips                  if(hsflag[l])

            }

            if (m_rootFilePtr != NULL) {
              hm_occupancy->getHistogram(*itSvdSensors, k)->Write();
              hm_hot_strips->getHistogram(*itSvdSensors, k)->Write();
              hm_occupancy_after->getHistogram(*itSvdSensors, k)->Write();
              // hm_dist12->getHistogram(*itSvdSensors, k)->Write();
            }

            B2DEBUG(1, " side " << i << " " << j << " " << m << " " << k);
            /* end */


            //              store number hot strips per sensor


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


bool SVDHotStripFinderModule::theHSFinder(float* stripOccAfterAbsCut, int* hsflag, int nstrips)
{
  float sensorOccAverage = 0;
  bool found = false;
  int nafter = 0;
  for (int l = 0; l < nstrips; l++) {
    sensorOccAverage = sensorOccAverage + stripOccAfterAbsCut[l];
    if (stripOccAfterAbsCut[l] > 0) nafter++;

  }
  sensorOccAverage = sensorOccAverage / nafter;
  B2DEBUG(1, "Average occupancy: " << sensorOccAverage);

  for (int l = 0; l < nstrips; l++) {

    // flag additional HS by comparing each strip occupancy with the sensor-based average occupancy

    if (stripOccAfterAbsCut[l] > sensorOccAverage * m_relOccPrec) {
      hsflag[l] = 1;
      found = true;
      stripOccAfterAbsCut[l] = 0;
    }
    //    else hsflag[l]=0;


  }
  return found;
}
