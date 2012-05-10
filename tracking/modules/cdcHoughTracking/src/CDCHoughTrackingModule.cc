//framework includes

#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <tracking/modules/cdcHoughTracking/CDCHoughHit.h>
#include <tracking/modules/cdcHoughTracking/CDCHoughTrackingModule.h>
#include <tracking/modules/cdcHoughTracking/HoughTransform.h>


//externals
#include "GFTrackCand.h"

//other includes

#include <vector>
#include <math.h>
#include <boost/foreach.hpp>
#include <fstream>
//#include <boost/tr1/tuple.hpp>
//#include <fstream>
//#include <time.h>

using namespace Belle2;
using namespace Belle2::Tracking;



REG_MODULE(CDCHoughTracking);

CDCHoughTrackingModule::CDCHoughTrackingModule() :
  Module()
{
  setDescription("Pattern Recognition in CDC with Hough Transform");

  addParam("CDCHits", m_cdcDigiHitCollectionName,
           "Name of the CDCDigiHitCollection (Input)", std::string("CDCHits"));
  addParam("GFTrackCandidates", m_gfTrackCandCollectionName, "Output Collection Name",
           std::string("GFTrackCandidates"));
  addParam("Precision", m_precision,
           "The amount of votes per hit", 360);
  addParam("ZFinder", m_zfinder,
           "Which zfinder to use. 0 for simple, 1 for improved.", 1);
  addParam("PeakFinder", m_pfinder,
           "Which peak finder to use. 0 for building regions, 1 for window approach.", 0);
  addParam("PhiBins", m_pBins,
           "The amount of phi bins in hough space.", 360);
  addParam("CurveBins", m_cBins,
           "The amount of curvature bins in hough space.", 250);

}

CDCHoughTrackingModule::~CDCHoughTrackingModule()
{
}

void CDCHoughTrackingModule::initialize()
{
  //StoreArray with GFTrack Candidates. Output of this module.
  StoreArray<GFTrackCand> gfTrackCands(m_gfTrackCandCollectionName);

}

void CDCHoughTrackingModule::beginRun()
{
}

void CDCHoughTrackingModule::event()
{
  //StoreArray with digitized CDCHits, should already be created by CDCDigitized module


  StoreArray<CDCHit> cdcHitArray(m_cdcDigiHitCollectionName);




  std::vector<CDCHoughHit> cdcHoughHits;
  cdcHoughHits.reserve(cdcHitArray.getEntries());

  //Create a CDCTrackHits from the CDCHits and store them in the same order in a vector, takes about 4ms for 534 hits
  const int numOfCDCHits = cdcHitArray.getEntries();
  for (int i = 0; i < numOfCDCHits; i++) {
    cdcHoughHits.push_back(CDCHoughHit(*cdcHitArray[i], i));
  }

  //sort hits by polar r from inner to outer layer
  std::sort(cdcHoughHits.begin(), cdcHoughHits.end(), SortHoughHitsByRadius());

  B2INFO("Number of Hits: " << cdcHoughHits.size());

  StoreArray<GFTrackCand> gfTrackCands(m_gfTrackCandCollectionName);

  HoughTransform hough(m_precision, m_pBins, m_cBins, &cdcHoughHits);

  hough.vote();

  B2INFO("PeakFinder");
  hough.peakFinder(-1);
  hough.peakFinder(1);

  B2INFO("TrackFinder");
  hough.trackBuilder(-1);
  hough.trackBuilder(1);

  B2INFO("ZFinder");
  hough.zFinder(-1);
  hough.zFinder(1);

  B2INFO("Fill DataStore");
  const std::vector<GFTrackCand>& cands = hough.getGfTracks();
  for (int i = 0, s = cands.size(); i < s ; ++i) {
    const GFTrackCand& element = cands[i];
    new(gfTrackCands->AddrAt(i)) GFTrackCand(element);
  }

  B2INFO("Tracks found: " << cands.size());

  hough.printDebugInfo();

  //execute search and fill gfTrackCands StoreArray
}

void CDCHoughTrackingModule::endRun()
{

}

void CDCHoughTrackingModule::terminate()
{
}
