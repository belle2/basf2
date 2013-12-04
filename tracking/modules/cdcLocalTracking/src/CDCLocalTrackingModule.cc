/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcLocalTracking/CDCLocalTrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/dataobjects/EventMetaData.h>

#include <cdc/geometry/CDCGeometryPar.h>

//in type
#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>

//typedefs
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>


//#include <tracking/cdcLocalTracking/algorithms/CellularAutomat.h>
//#include <tracking/cdcLocalTracking/algorithms/CellularPathFollower.h>
//#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>

//#include <tracking/cdcLocalTracking/algorithms/NeighborhoodBuilder.h>
//#include <tracking/cdcLocalTracking/neighbor_chooser/WireHitNeighborChooser.h>
//#include <tracking/cdcLocalTracking/neighbor_chooser/SimpleFacetNeighborChooser.h>
//#include <tracking/cdcLocalTracking/neighbor_chooser/SimpleSegmentTripleNeighborChooser.h>


//#include <tracking/cdcLocalTracking/creators/FacetCreator.h>
//#include <tracking/cdcLocalTracking/creators/FacetSegmentReducer.h> //no decisions to optimize
//#include <tracking/cdcLocalTracking/creators/SegmentSelecter.h>
//#include <tracking/cdcLocalTracking/creators/SegmentReverser.h>

//#include <tracking/cdcLocalTracking/creators/SegmentTripleCreator.h>
//#include <tracking/cdcLocalTracking/creators/SegmentTripleTrackReducer.h> //no decisions to optimize
//#include <tracking/cdcLocalTracking/creators/GFTrackCandCreator.h>


//out type
#include "genfit/TrackCand.h"

//test to be removed
//#include <tracking/cdcLocalTracking/testcode/MockTest.h>
//#include <tracking/cdcLocalTracking/testcode/TemplateTest.h>

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif
#include <time.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


REG_MODULE(CDCLocalTracking)

CDCLocalTrackingModule::CDCLocalTrackingModule() : Module()
{
  setDescription("Performs patter recognition in the CDC based on local hit following");

  addParam("Input collection name",  m_cdcHitsName, "Name of the CDCHit input collection ", string(""));

}

CDCLocalTrackingModule::~CDCLocalTrackingModule()
{
}

void CDCLocalTrackingModule::initialize()
{

  //StoreObjPtr<CDCWireHit>::registerPersistent();
  //register a single cdchit under the name "AnotherHit" and do not write
  //it to the output file by default
  //StoreObjPtr<CDCWireHit>::registerTransient("AnotherHit");

#ifdef USE_ROOT_IN_CDCLOCALTRACKING
  StoreObjPtr< CDCWireHitCollection >::registerTransient("CDCAllWireHitCollection");
#endif


  //output collection
  StoreArray < genfit::TrackCand >::registerTransient("GFTrackCands");

  m_segmentWorker.init();
  m_trackingWorker.init();

  //StoreArray with digitized CDCHits
  StoreArray <CDCHit> cdcHits(m_cdcHitsName);
  cdcHits.isRequired();

  StoreArray <CDCSimHit> storedSimhits;
  storedSimhits.isRequired();

  StoreArray <MCParticle> storedMCParticles;
  storedMCParticles.isRequired();

  //preload geometry during initialization
  //maked as unused intentionally to avoid a compile warning
  CDC::CDCGeometryPar& cdcGeo __attribute__((unused)) = CDC::CDCGeometryPar::Instance();
  CDCWireTopology& topo __attribute__((unused)) = CDCWireTopology::getInstance();

  // initialize the lowest CDCWireHit, also after each geometry change!
  CDCWireHit::initializeLowest();
  const CDCWireHit& lowest = CDCWireHit::getLowest();
  B2DEBUG(100, lowest);

}

void CDCLocalTrackingModule::beginRun()
{

}

void CDCLocalTrackingModule::event()
{
  B2DEBUG(100, "########## CDCLocalTracking begin ##########");
#ifdef HAS_CALLGRIND
  CALLGRIND_START_INSTRUMENTATION;
#endif
  //Start callgrind recording
  //To profile start basf2 with
  //  nohup valgrind --tool=callgrind --instr-atstart=no basf2 [basf2-options] > output.txt &
  // since that takes a while.
  // Do a callgrind_control -b for an intermediate output or callgrind_control -b -e
  // Definitions need callgrind.h

  //fetch the CDCHits from the datastore
  B2DEBUG(100, "Getting the CDCHits from the data store");
  StoreArray <CDCHit> storedCDCHits(m_cdcHitsName);
  B2DEBUG(100, "  storedCDCHits.getEntries() == " << storedCDCHits.getEntries());

  //create the wirehits
  B2DEBUG(100, "Creating all CDCWireHits");

#ifdef USE_ROOT_IN_CDCLOCALTRACKING
  B2DEBUG(100, "  Getting storedCDCWireHits from DataStore");
  StoreObjPtr< CDCWireHitCollection > storeCDCWireHits("CDCAllWireHitCollection");
  storeCDCWireHits.create();
  B2DEBUG(100, "  storedCDCWireHits.isValid() == " <<  storeCDCWireHits.isValid())
  B2DEBUG(100, "  storedCDCWireHits->size() == " <<  storeCDCWireHits->size());
  CDCWireHitCollection& nonConstAllWireHits = *storeCDCWireHits;
#else
  B2DEBUG(100, "  Allocating CDCWireHitNeighborhood");
  CDCWireHitCollection nonConstAllWireHits;
#endif
  //Fill the wireHits
  B2DEBUG(100, "  Creating the CDCWireHits");
  m_wirehitCreator.create(storedCDCHits, nonConstAllWireHits);
  B2DEBUG(100, "  Created " << nonConstAllWireHits.size() << " CDCWireHits");

  //use a constant reference to prevent any alteration of the wirehits from this point on
  const CDCWireHitCollection& allWireHits = nonConstAllWireHits;
  B2DEBUG(100, "  Created " << allWireHits.size() << " CDCWireHits");
  //CALLGRIND_STOP_INSTRUMENTATION;

  //check the relations between the particles simhit and hits before using them
  StoreArray <MCParticle> storedMCParticles;
  StoreArray <CDCSimHit> storedSimhits;

  //CDCMCLookUp::checkComposition(storedCDCHits,storedSimhits,storedMCParticles);

  //create mc look up
  CDCMCLookUp& mcLookUp = CDCMCLookUp::Instance();
  mcLookUp.clear();
  mcLookUp.addAllSimHits(allWireHits, storedCDCHits, storedSimhits);
  mcLookUp.addAllMCParticle(allWireHits, storedCDCHits, storedMCParticles);

  //mcLookUp.checkSimToMCTrackIdEquivalence(allWireHits);

  //CALLGRIND_START_INSTRUMENTATION;
  //build the segments
  m_recoSegments.clear();
  m_segmentWorker.apply(allWireHits, m_recoSegments);
  B2DEBUG(100, "Received " << m_recoSegments.size() << " RecoSegments from worker");


  //register the segments to the mclookup in order to enable descisions based on the alignement of them
  mcLookUp.addSegments(m_recoSegments);

  //build the gfTracks
  StoreArray < genfit::TrackCand > storedGFTrackCands("GFTrackCands");
  storedGFTrackCands.create();
  m_trackingWorker.apply(m_recoSegments, storedGFTrackCands);


  //End callgrind recording
#ifdef HAS_CALLGRIND
  CALLGRIND_STOP_INSTRUMENTATION;
#endif

  B2DEBUG(100, "########## CDCLocalTracking end ############");

}

void CDCLocalTrackingModule::endRun()
{
}

void CDCLocalTrackingModule::terminate()
{
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS;
#endif
}

