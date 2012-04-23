#include <tracking/modules/datareduction/DataReduction.h>

#include <cmath>
#include <iomanip>
//
//DataStore Objects

#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <tracking/modules/datareduction/TrackerHit.h>

//Include LCIO/Marlin header files
/*
#include <EVENT/LCCollection.h>
#include <EVENT/SimTrackerHit.h>
#include <EVENT/TrackerHit.h>
#include "marlin/Global.h"

//Include GEAR header files
#include <gear/GEAR.h>
#include <gear/GearParameters.h>
#include <gear/VXDLayerLayout.h>
#include <gear/VXDParameters.h>
*/
//Include ROOT header files
#include <TVector3.h>
#include <TVector2.h>

#include <tracking/modules/datareduction/SectorStraight.h>
#include <tracking/modules/datareduction/SectorArc.h>
#include <tracking/modules/datareduction/HoughTransformStraight.h>
#include <tracking/modules/datareduction/HoughTransformSine.h>
#include <tracking/modules/datareduction/PXDLadder.h>

using namespace std;
using namespace Belle2;

REG_MODULE(DataReduction);

DataReductionModule::DataReductionModule() : Module()
{
  //Processor description
  setDescription("DataReduction of PXD readout by using the concept of fast hough transformation on digitized SVD hits");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Input Collections
  /*
  registerInputCollection(LCIO::TRACKERHIT, "InputDigiSVDCollection", "Name of tracker hits collection with digitized SVDTrueHits",
                          _colNameDigiSVDTrueHits, std::string("SVDTrackerHits"));

  registerInputCollection(LCIO::TRACKERHIT, "InputDigiPXDCollection", "Name of tracker hits collection with digitized PXDTrueHits",
                          _colNameDigiPXDTrueHits, std::string("PXDTrackerHits"));

  registerInputCollection(LCIO::SIMTRACKERHIT, "InputSimPXDCollection", "Name of tracker hits collection with digitized PXDTrueHits",
                          _colNameSimPXDTrueHits, std::string("PXDCollection"));
  */

  addParam("InputDigiSVDCollection", _colNameDigiSVDTrueHits,
           "Name of the SVDTrueHitCollection", string("SVDTrueHits"));

  addParam("InputDigiPXDCollection", _colNameDigiPXDTrueHits,
           "Name of the PXDTrueHitCollection", string("PXDTrueHits"));

  addParam("InputSimPXDCollection", _colNameSimPXDTrueHits,
           "Name of the PXDSimHitCollection", string("PXDSimHits"));

  _sectorList = new SectorList();

  _pxdLadderList = new PXDLadderList();
}


DataReductionModule::~DataReductionModule()
{
  delete _pxdLadderList;
  delete _sectorList;
}


void DataReductionModule::initialize()
{
  _nRun = 0;
  _nEvt = 0;
  _numberPXDTrueHitsTotal = 0;
  _numberPXDTrueHitsFound = 0;


  //------- Add sectors -------
  /*int num=64;
  double opening = 2.0*M_PI/num;
  double down = 0.5;
  double width = 2*down;
  for(int i=0; i<num; i++){
    _sectorList->push_back(new SectorArc(opening*i,down,width,-75,-105));
    _sectorList->back()->setColor(0.8,0,0);
    _sectorList->push_back(new SectorArc(opening*i,down,width,75,105));
    _sectorList->back()->setColor(0.8,0,0);
    _sectorList->push_back(new SectorArc(opening*i,down,width,-100,-160));
    _sectorList->back()->setColor(0.8,0.8,0);
    _sectorList->push_back(new SectorArc(opening*i,down,width,100,160));
    _sectorList->back()->setColor(0.8,0.8,0);
    _sectorList->push_back(new SectorArc(opening*i,down,width,-150,-415));
    _sectorList->back()->setColor(0,0.8,0);
    _sectorList->push_back(new SectorArc(opening*i,down,width,150,415));
    _sectorList->back()->setColor(0,0.8,0);
    //_sectorList->push_back(new SectorStraight(opening*i,down,width,M_PI/10));
    _sectorList->push_back(new SectorArc(opening*i,down,width,-400,400));
    _sectorList->back()->setColor(0,0,0.8);
  }*/
  /*_sectorList->push_back(new SectorArc(0,0,2,-400,400));
  _sectorList->back()->setColor(0.8,0,0);
  _sectorList->push_back(new SectorArc(M_PI/2,0,2,75,300));
  _sectorList->back()->setColor(0,0.8,0);
  _sectorList->push_back(new SectorArc(1.5*M_PI,0,2,-75,-300));
  _sectorList->push_back(new SectorStraight( M_PI/4,0,2,M_PI/8));*/
  int num = 60;
  double opening = 2.0 * M_PI / num;
  for (int i = 0; i < num; i++) {
    _sectorList->push_back(new SectorStraight(opening * i, 0, 2, opening * 1.1));
  }
  for (int i = 0; i < num; i++) {
    _sectorList->push_back(new SectorArc(opening * i, 2, 2, 75, 105));
    _sectorList->back()->setColor(0, 0.8, 0);
  }
  for (int i = 0; i < num; i++) {
    _sectorList->push_back(new SectorArc(opening * i, 2, 2, -75, -105));
    _sectorList->back()->setColor(0, 0.8, 0);
  }
  for (int i = 0; i < num; i++) {
    _sectorList->push_back(new SectorArc(opening * i, 2, 2, 100, 160));
    _sectorList->back()->setColor(0.8, 0.8, 0);
  }
  for (int i = 0; i < num; i++) {
    _sectorList->push_back(new SectorArc(opening * i, 2, 2, -100, -160));
    _sectorList->back()->setColor(0.8, 0.8, 0);
  }
  for (int i = 0; i < num; i++) {
    _sectorList->push_back(new SectorArc(opening * i, 2, 2, 150, 415));
    _sectorList->back()->setColor(0.8, 0, 0);
  }
  for (int i = 0; i < num; i++) {
    _sectorList->push_back(new SectorArc(opening * i, 2, 2, -150, -415));
    _sectorList->back()->setColor(0.8, 0, 0);
  }
  /*_sectorList->back()->setColor(0,0.8,0);
  _sectorList->push_back(new SectorStraight(M_PI,0,2,M_PI/8));
  _sectorList->back()->setColor(0.8,0,0);*/




  //------- Add PXD ladders -------
  /*
  const gear::VXDParameters  & gearVXD             = Global::GEAR->getVXDParameters();
  const gear::VXDLayerLayout & gearVXDLayerLayout  = gearVXD.getVXDLayerLayout();
  const gear::GearParameters & gearVXDParams       = Global::GEAR->getGearParameters("VXDParameters");
  std::vector<int>    vecLayerType                 = gearVXDParams.getIntVals("ActiveLayerType");
  std::vector<double> vecLayerOffsetZ              = gearVXDParams.getDoubleVals("ActiveLayerOffsetZ");
  */


  int nLadders       = 0;
  float currentPhi   = 0.0;
  float ladderPhiRot = 0.0;
  float layerPhi0    = 0.0;
  float layerRadius  = 0.0;
  float layerOffsetY = 0.0;
  float layerOffsetZ = 0.0;
  float ladderThick  = 0.;
  float ladderWidth  = 0.;
  float ladderLength = 0.;

  // Get number of layers of VXD (PXD+SVD)
  /* int nLayersVXD = gearVXDLayerLayout.getNLayers();

   for (int iLayer = 0; iLayer < nLayersVXD; ++iLayer) {

     if (vecLayerType[iLayer] != pixel) continue; //Only take PXD layers into account

     //Get parameters
     layerPhi0    = float(gearVXDLayerLayout.getPhi0(iLayer)) / 180.0 * M_PI;
     layerRadius  = float(gearVXDLayerLayout.getSensitiveDistance(iLayer));
     layerOffsetY = float(gearVXDLayerLayout.getSensitiveOffset(iLayer));
     layerOffsetZ = float(vecLayerOffsetZ[iLayer]);
     ladderThick  = float(gearVXDLayerLayout.getSensitiveThickness(iLayer));
     ladderWidth  = float(gearVXDLayerLayout.getSensitiveWidth(iLayer));
     ladderLength = float(gearVXDLayerLayout.getSensitiveLength(iLayer));

     //get number of ladders per layer
     nLadders = gearVXDLayerLayout.getNLadders(iLayer);
     ladderPhiRot = 2.0 * M_PI / nLadders;

     for (int iLadder = 0; iLadder < nLadders; ++iLadder) {
       currentPhi = layerPhi0 + ladderPhiRot * iLadder;

       TVector3 ladderPos(layerRadius, layerOffsetY, layerOffsetZ); // Ladder starting position
       TVector3 ladderNorm(1.0, 0.0, 0.0);
       TVector3 ladderSize(ladderWidth, ladderThick, ladderLength);
       ladderPos.RotateZ(currentPhi);  //Ladder final position
       ladderNorm.RotateZ(currentPhi); //Normal vector

       _pxdLadderList->push_back(new PXDLadder(ladderPos, ladderNorm, ladderSize));
     }
   } */

  // Only PXD Layers
  int nLayersVXD = 2;

  // Data taken from PXDBelleII_PXD1600.xml and PXDSensorBelleII.xml
  float        vlayerPhi0[2] = {90.0 / 180.0 * M_PI, 90.0 / 180.0 * M_PI};
  float        vlayerRadius[2] = {14.0, 22.0}; //whole ladder as sensitive area
  float        vlayerOffsetY[2] = { -3.3, -3.3}; //whole ladder as sensitive area
  float        vlayerOffsetZ[2] = {11.7, 18.1};
  float        vladderThick[2] = {0.075, 0.075};
  float        vladderWidth[2] = {12.5, 12.5};
  float        vladderLength[2] = {44.725, 44.725};
  int        vnLadders[2] = {8, 12};

  for (int iLayer = 0; iLayer < nLayersVXD; ++iLayer) {

    //Get parameters
    layerPhi0    = vlayerPhi0[iLayer];
    layerRadius  = vlayerRadius[iLayer];
    layerOffsetY = vlayerOffsetY[iLayer];
    layerOffsetZ = vlayerOffsetZ[iLayer];
    ladderThick  = vladderThick[iLayer];
    ladderWidth  = vladderWidth[iLayer];
    ladderLength = vladderLength[iLayer];

    //get number of ladders per layer
    nLadders = vnLadders[iLayer];
    ladderPhiRot = 2.0 * M_PI / nLadders;

    for (int iLadder = 0; iLadder < nLadders; ++iLadder) {
      currentPhi = layerPhi0 + ladderPhiRot * iLadder;

      TVector3 ladderPos(layerRadius, layerOffsetY, layerOffsetZ); // Ladder starting position
      TVector3 ladderNorm(1.0, 0.0, 0.0);
      TVector3 ladderSize(ladderWidth, ladderThick, ladderLength);
      ladderPos.RotateZ(currentPhi);  //Ladder final position
      ladderNorm.RotateZ(currentPhi); //Normal vector

      _pxdLadderList->push_back(new PXDLadder(ladderPos, ladderNorm, ladderSize));
    }
  }

  _sectorList->doLadderIntersect(*_pxdLadderList);

#ifdef CAIRO_OUTPUT
  cairo_surface = cairo_pdf_surface_create("DataReduction.pdf", CAIRO_SIZE, CAIRO_SIZE);
  cairo = cairo_create(cairo_surface);
  cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);
  makePDF(false);
  makePDF(false, num);
#endif
}

void DataReductionModule::beginRun()
{
}

void DataReductionModule::event()
{
  _sectorList->clearAllHits(); //Clear all hits
  _pxdLadderList->clearAllRegions(); //Clear all regions
  /*
    //1) Prepare collections
    LCCollection* digiSVDTrueHitsCol = 0;
    LCCollection* digiPXDTrueHitsCol = 0;
    LCCollection* simPXDTrueHitsCol = 0;
  */
  StoreArray<PXDSimHit> pxdSimHitArray(_colNameSimPXDTrueHits);
  StoreArray<PXDTrueHit>    pxdHitArray(_colNameDigiPXDTrueHits);
  StoreArray<SVDTrueHit>    svdHitArray(_colNameDigiSVDTrueHits);
  B2INFO("Number of simulated SVDTrueHits:  " << svdHitArray.GetEntries());
  B2INFO("Number of simulated PXDSimHits:  " << pxdSimHitArray.GetEntries());
  B2INFO("Number of simulated PXDTrueHits:  " << pxdHitArray.GetEntries());

  /*
    //2) Assign digitized SVD Hits hits to Sectors
    try {
      digiSVDTrueHitsCol = evt->getCollection(_colNameDigiSVDTrueHits.c_str());
      int ndigiSVDTrueHits = digiSVDTrueHitsCol->getNumberOfElements();

      digiPXDTrueHitsCol = evt->getCollection(_colNameDigiPXDTrueHits.c_str());
      simPXDTrueHitsCol = evt->getCollection(_colNameSimPXDTrueHits.c_str());

      for (int iHit = 0; iHit < ndigiSVDTrueHits; ++iHit) {
        TrackerHit* currentHit = dynamic_cast<TrackerHit*>(digiSVDTrueHitsCol->getElementAt(iHit));
        _sectorList->addHit(currentHit);
      }

      printSectorInfo();

    } catch (DataNotAvailableException &e) {
      streamlog_out(MESSAGE4) << "Collection not found: " << _colNameDigiSVDTrueHits << endl;
      streamlog_out(MESSAGE4) << "Skip event: " << _nEvt << endl;
      return;
    }
  */
  int ndigiSVDTrueHits = svdHitArray.GetEntries();

  for (int iHit = 0; iHit < ndigiSVDTrueHits; ++iHit) {
    TrackerHit* currentHit = new TrackerHit();
    double hitPos[3] = {svdHitArray[iHit]->getU(), svdHitArray[iHit]->getV(), 0.0};
    B2DEBUG(99, svdHitArray[iHit]->getU() << "  " << svdHitArray[iHit]->getV());
    currentHit->setPosition(hitPos);
    currentHit->setdEdx(svdHitArray[iHit]->getEnergyDep());
    _sectorList->addHit(currentHit);
  }
//3)

#ifdef CAIRO_OUTPUT
  cairo_pdf_surface_set_size(cairo_surface, 2 * CAIRO_SIZE, 2 * CAIRO_SIZE);
#endif


  HoughTransformSine sinehough;
  HoughTransformStraight straighthough;
  HoughTransformBasic* currhough;
  sinehough.setMinHoughBoxSizeParamA(1.0e-4);//5.0e-3
  sinehough.setMinHoughBoxSizeParamB(1.0e-4);//5.0e-2;

  for (SectorList::iterator sectorIter = _sectorList->begin(); sectorIter != _sectorList->end(); sectorIter++) {
    SectorBasic& sect = **sectorIter;
    SectorArc* arc = dynamic_cast<SectorArc*>(*sectorIter);
    if (arc != 0) {
      sinehough.setRadius(fabs(arc->getSmallRadius()), fabs(arc->getBigRadius()));
      currhough = &sinehough;
    } else {
      currhough = &straighthough;
    }
#ifdef CAIRO_OUTPUT
    if (sect.getHitNumber() >= 3) {
      cairo_save(cairo);
      cairo_plot(cairo, -150, 150, -150, 150);
      sect.draw(cairo);
      cairo_restore(cairo);
      cairo_save(cairo);
      cairo_translate(cairo, CAIRO_SIZE, 0);
      currhough->setCairo(cairo);
    }
#endif
    currhough->doHoughSearch(sect);
#ifdef CAIRO_OUTPUT
    if (sect.getHitNumber() >= 3) {
      cairo_restore(cairo);
      cairo_save(cairo);
      cairo_translate(cairo, 0, CAIRO_SIZE);
      cairo_plot(cairo, -300, 350, 0, 150, 2 * CAIRO_SIZE, CAIRO_SIZE);
      currhough->drawRZ(cairo);
      cairo_restore(cairo);
      cairo_show_page(cairo);
    }
#endif
  }
#ifdef CAIRO_OUTPUT
  int pxwidth = 13;
  int pxheight = 120;
  double pxscale = 3;
  cairo_pdf_surface_set_size(cairo_surface, (pxwidth * pxscale + 1)*_pxdLadderList->size() + 1, pxheight * pxscale + 2);
  int nl = 0;
#endif

  for (PXDLadderList::iterator it = _pxdLadderList->begin(); it != _pxdLadderList->end(); it++) {
    PXDLadder& l = **it;
    TVector3 p = l.getPosition();
    TVector3 s = 0.5 * l.getSize();
    TVector3 n = l.getNormal();
#ifdef CAIRO_OUTPUT
    cairo_save(cairo);
    cairo_set_line_width(cairo, 1);
    cairo_translate(cairo, nl * (pxwidth * pxscale + 1) + 1, pxheight * pxscale + 1);
    cairo_scale(cairo, pxscale * s(0) * 2, -s(2)*pxscale * 2);
    cairo_rectangle(cairo, 0, 0, 1, 1);
    cairo_set_source_rgb(cairo, 0, 0, 0);
    cairo_stroke_abs(cairo, true);
    cairo_set_source_rgb(cairo, 0.9, 0.9, 0.9);
    cairo_fill(cairo);
    cairo_set_line_width(cairo, 0.1);
    //cairo_clip(cairo);
    cairo_color color(0, 0, 0);

    for (list<RegionOfInterest*>::iterator itR = l.getRegionList().begin(); itR != l.getRegionList().end(); itR++) {
      RegionOfInterest& r = **itR;
      if (r.color != color) {
        cairo_set_source_rgba(cairo, color, 0.3);
        color = r.color;
        cairo_fill(cairo);
      }
      //streamlog_out(DEBUG) << r.widthStart << " " << r.lengthStart << " ";
      //streamlog_out(DEBUG) << r.widthEnd << " " << r.lengthEnd << endl;
      cairo_rectangle(cairo, r.widthStart, r.lengthStart, r.widthEnd - r.widthStart, r.lengthEnd - r.lengthStart);
      cairo_set_source_rgba(cairo, 0, 0, 0.8, 0.2);
    }
    cairo_set_source_rgba(cairo, color, 0.2);
    cairo_fill(cairo);
    cairo_set_source_rgba(cairo, 1, 0, 0, 1);
#endif

    //Draw PXD Hits
    int ndigiPXDTrueHits = pxdHitArray.GetEntries();

    for (int iHit = 0; iHit < ndigiPXDTrueHits; ++iHit) {
      TrackerHit* currentHit = new TrackerHit();
      double hitPos[3] = {pxdHitArray[iHit]->getU(), pxdHitArray[iHit]->getV(), 0.0};
      currentHit->setPosition(hitPos);
      currentHit->setdEdx(pxdHitArray[iHit]->getEnergyDep());


      //Shift center to local ladder frame
      if (!l.isInLadder(currentHit->getPosition())) continue;
      _numberPXDTrueHitsTotal++;

      TVector2 hitPosNorm = l.convertToRelative(currentHit->getPosition());

      list<RegionOfInterest*>& regions = l.getRegionList();
      for (list<RegionOfInterest*>::iterator regionIter = regions.begin(); regionIter != regions.end(); ++regionIter) {
        RegionOfInterest* currRegion = *regionIter;

        if ((hitPosNorm.X() >= currRegion->widthStart) && (hitPosNorm.X() <= currRegion->widthEnd) &&
            (hitPosNorm.Y() >= currRegion->lengthStart) && (hitPosNorm.Y() <= currRegion->lengthEnd)) {
          _numberPXDTrueHitsFound++;
          break;
        }
      }
#ifdef CAIRO_OUTPUT
      cairo_save(cairo);
      cairo_scale(cairo, 1.0 / 250, 1.0 / 1600);
      cairo_arc(cairo, hitPosNorm.X() * 250, hitPosNorm.Y() * 1600, 3, 0, 2 * M_PI);
      cairo_fill(cairo);
      cairo_restore(cairo);
#endif
    }
    //Draw PXD SimHits
    int nsimPXDTrueHits = pxdSimHitArray.GetEntries();

    for (int iHit = 0; iHit < nsimPXDTrueHits; ++iHit) {
      TrackerHit* currentHit = new TrackerHit();
      TVector3 posV =  pxdSimHitArray[iHit]->getPosIn();
      double hitPos[3] = {posV(0), posV(1), 0.0};
      currentHit->setPosition(hitPos);


      //EVENT::MCParticle &mcParticle = *currentHit->getMCParticle();
      /*streamlog_out(MESSAGE4) << mcParticle.getGeneratorStatus()
          << mcParticle.getPDG()
          << endl;*/
      //if (mcParticle.getGeneratorStatus() != 1) continue;


      //Shift center to local ladder frame
      if (!l.isInLadder(currentHit->getPosition())) continue;
      _numberSimPXDTrueHitsTotal++;

      TVector2 hitPosNorm = l.convertToRelative(currentHit->getPosition());

      list<RegionOfInterest*>& regions = l.getRegionList();
      for (list<RegionOfInterest*>::iterator regionIter = regions.begin(); regionIter != regions.end(); ++regionIter) {
        RegionOfInterest* currRegion = *regionIter;

        if ((hitPosNorm.X() >= currRegion->widthStart) && (hitPosNorm.X() <= currRegion->widthEnd) &&
            (hitPosNorm.Y() >= currRegion->lengthStart) && (hitPosNorm.Y() <= currRegion->lengthEnd)) {
          _numberSimPXDTrueHitsFound++;
          break;
        }
      }
#ifdef CAIRO_OUTPUT
      cairo_save(cairo);
      cairo_scale(cairo, 1.0 / 250, 1.0 / 1600);
      cairo_arc(cairo, hitPosNorm.X() * 250, hitPosNorm.Y() * 1600, 3, 0, 2 * M_PI);
      cairo_set_source_rgb(cairo, 0, 1, 0);
      cairo_fill(cairo);
      cairo_restore(cairo);
#endif
    }
#ifdef CAIRO_OUTPUT
    cairo_restore(cairo);
    nl++;
#endif
  }
#ifdef CAIRO_OUTPUT
  cairo_surface_show_page(cairo_surface);
#endif

  _nEvt++;
}


void DataReductionModule::endRun()
{
  _sectorList->deleteAllSectors();
  _pxdLadderList->deleteAllLadders();

  B2INFO("Number PXD Hits total: " << _numberPXDTrueHitsTotal << endl);
  B2INFO("Number PXD Hits found: " << _numberPXDTrueHitsFound << endl);
  B2INFO("Efficiency:            " << setprecision(5) << double(_numberPXDTrueHitsFound) / double(_numberPXDTrueHitsTotal) << endl);
  B2INFO("Number SimPXD Hits total: " << _numberSimPXDTrueHitsTotal << endl);
  B2INFO("Number SimPXD Hits found: " << _numberSimPXDTrueHitsFound << endl);
  B2INFO("Efficiency:               " << setprecision(5) << double(_numberSimPXDTrueHitsFound) / double(_numberSimPXDTrueHitsTotal) << endl);
#ifdef CAIRO_OUTPUT
  cairo_destroy(cairo);
  cairo_surface_finish(cairo_surface);
  cairo_surface_destroy(cairo_surface);
#endif
}

void DataReductionModule::terminate()
{
}

void DataReductionModule::printSectorInfo()
{
  SectorList::iterator sectorIter;
  int iSector = 0;

  B2INFO("=== Sector Information (index,#Hits,#Ladders) ===" << endl);

  for (sectorIter = _sectorList->begin(); sectorIter != _sectorList->end(); ++sectorIter) {
    SectorBasic* currSector = *sectorIter;
    B2INFO(iSector << ": " << currSector->getHitNumber() << " " << currSector->getLadderNumber() << endl);
    iSector++;
  }
}
#ifdef CAIRO_OUTPUT
void DataReductionModule::makePDF(bool split, int group)
{
  cairo_save(cairo);
  cairo_plot(cairo, -150, 150, -150, 150);
  SectorList::iterator sectorIter;
  int n = 0;
  for (sectorIter = _sectorList->begin(); sectorIter != _sectorList->end(); ++sectorIter) {
    if (group > 1 && n > 0 && n % group == 0) {
      cairo_restore(cairo);
      cairo_show_page(cairo);
      cairo_save(cairo);
      cairo_plot(cairo, -150, 150, -150, 150);
    }

    SectorBasic* currSector = *sectorIter;

    currSector->draw(cairo);
    n++;
  }
  cairo_show_page(cairo);
  cairo_restore(cairo);
}
#endif
