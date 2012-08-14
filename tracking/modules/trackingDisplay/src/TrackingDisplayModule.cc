#include <tracking/modules/trackingDisplay/TrackingDisplayModule.h>

#include <tracking/modules/trackingDisplay/GenfitDisplay.h>
#include <framework/datastore/StoreArray.h>
#include <geometry/GeometryManager.h>
#include <vxd/geometry/GeoCache.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <GFTrack.h>
#include <GFFieldManager.h>
#include "TGeoManager.h"
#include "TSystem.h"


using namespace Belle2;

REG_MODULE(TrackingDisplay)

TrackingDisplayModule::TrackingDisplayModule() : Module(), m_display(0)
{
  setDescription("Interactive visualisation of GFTrack objects and PXD/SVD/CDCSimHits (plus geometry)");

  addParam("Options", m_options, "Drawing options, a combination of ADHRGMPSTX. See GenfitDisplay::setOptions or the trackingDisplay.py example for an explanation.", std::string("MHTGR"));
  addParam("AddPXDHits", m_addPXDHits, "Add PXDSimHits to display? Use with 'R' option.", true);
  addParam("AddSVDHits", m_addSVDHits, "Add SVDSimHits to display? Use with 'R' option.", true);
  addParam("AddCDCHits", m_addCDCHits, "Add CDCSimHits to display? Use with 'R' option.", true);

  //make sure dictionaries for PXD/SVDrecohits are loaded
  //needs to be done here to have dictionaries available during SimpleInput::initialize()
  gSystem->Load("libpxd");
  gSystem->Load("libsvd");
  gSystem->Load("libgenfitRK");
}


TrackingDisplayModule::~TrackingDisplayModule()
{
  delete m_display;
}


void TrackingDisplayModule::initialize()
{
  if (!gGeoManager) { //TGeo geometry not initialized, do it ourselves
    //convert geant4 geometry to TGeo geometry
    //in the moment tesselated solids used for the glue within the PXD cannot be converted to TGeo, the general solution still has to be found, at the moment you can just comment out lines 6 and 13 in  pxd/data/PXD-Components.xml.
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();

    //initialize magnetic field for genfit
    GFFieldManager::getInstance()->init(new GFGeant4Field());
  }

  if (m_addPXDHits or m_addSVDHits)
    VXD::GeoCache::getInstance();

  m_display = new GenfitDisplay();
  m_display->setOptions(m_options);
}

void TrackingDisplayModule::event()
{
  //gather tracks
  StoreArray<GFTrack> gftracks;
  const int nTracks = gftracks.getEntries();
  std::vector<GFTrack* > gftracks_vec;
  for (int i = 0; i < nTracks; i++)
    gftracks_vec.push_back(gftracks[i]);

  //gather simhits

  //given to GenfitDisplay: vector of hits (x,y,z, sigma x, sigma y, sigma z)
  std::vector<std::vector<double> > hitVector;

  if (m_addCDCHits) {
    StoreArray<CDCSimHit> cdchits;
    const int nCDCHits = cdchits.getEntries();
    for (int i = 0; i < nCDCHits; i++) {
      std::vector<double> hit(3, 0.0);
      hit[0] = cdchits[i]->getPosWire().x();
      hit[1] = cdchits[i]->getPosWire().y();
      hit[2] = cdchits[i]->getPosWire().z();
      hitVector.push_back(hit);
    }
  }

  if (m_addPXDHits) {
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    StoreArray<PXDSimHit> pxdhits;
    const int nPXDHits = pxdhits.getEntries();
    for (int i = 0; i < nPXDHits; i++) {
      std::vector<double> hit(3, 0.0);
      const TVector3 global_pos = geo.get(pxdhits[i]->getSensorID()).pointToGlobal(pxdhits[i]->getPosIn());
      hit[0] = global_pos.x();
      hit[1] = global_pos.y();
      hit[2] = global_pos.z();

      hitVector.push_back(hit);
    }
  }

  if (m_addSVDHits) {
    //As the SVD tends to have a particularly high number of simhits, we'll add them last to avoid losing CDC hits
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    StoreArray<SVDSimHit> svdhits;
    int nSVDHits = svdhits.getEntries();
    for (int i = 0; i < nSVDHits; i++) {
      std::vector<double> hit(3, 0.0);
      const TVector3 global_pos = geo.get(svdhits[i]->getSensorID()).pointToGlobal(svdhits[i]->getPosIn());
      hit[0] = global_pos.x();
      hit[1] = global_pos.y();
      hit[2] = global_pos.z();
      hitVector.push_back(hit);
    }
  }

  //delete event part
  m_display->reset();

  //add current event to display
  m_display->addEvent(gftracks_vec);
  m_display->setHits(hitVector);

  m_display->open();
}
