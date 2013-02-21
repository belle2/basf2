/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/raveInterface/RaveSetup.h>

//framework genfit and root stuff for GFRave setup
#include <GFConstField.h>
#include <GFFieldManager.h>
#include <GFFieldManager.h>
#include <TGeoManager.h>
#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <GFTGeoMaterialInterface.h>
#include <GFMaterialEffects.h>
//stl stuff
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::cerr; using std::endl;


using namespace Belle2;
using namespace analysis;

RaveSetup* RaveSetup::s_instance = NULL;

void RaveSetup::initialize(string options)
{
  if (s_instance == NULL) {
    s_instance = new RaveSetup();
  } else {
    delete s_instance;
    s_instance = new RaveSetup();
  }
  if (options == "GFRave" or options == "default") { // at the moment only the interface to GFRave works so it has to be the default
    s_instance->m_gfRave = true;
    s_instance->m_gfPropagation = true;
    if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
      geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
      geoManager.createTGeoRepresentation();
      //pass the magnetic field to genfit
      GFFieldManager::getInstance()->init(new GFGeant4Field());
      GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
      GFMaterialEffects::getInstance()->setMscModel("Highland");
    }


  } else if (options == "Rave") {// use Rave directly without GFRave
    s_instance->m_gfRave = false;
    //TODO get magentic field from framework
    s_instance->m_raveVertexFactory = new rave::VertexFactory(rave::ConstantMagneticField(0, 0, 1.5), rave::VacuumPropagator(), "kalman", s_instance->m_raveVerbosity);
  } else {
    B2FATAL("You passed the unknown option " << options <<  " to RaveSetup::initialize. Cannot continue");
  }

}

RaveSetup::RaveSetup(): m_gfRave(false), m_gfPropagation(false), m_raveVerbosity(0), m_useBeamSpot(false),
  m_raveVertexFactory(NULL), m_GFRaveVertexFactory(NULL)
{
  ;
}

RaveSetup::~RaveSetup()
{
  //delete everything that could have potentially created with new in this class
  if (m_raveVertexFactory not_eq NULL) {
    delete m_raveVertexFactory;
  }
  if (m_GFRaveVertexFactory not_eq NULL) {
    delete m_GFRaveVertexFactory;
  }
}


void RaveSetup::setBeamSpot(TVector3 beamSpot, TMatrixDSym beamSpotCov)
{
  m_beamSpot = beamSpot;
  m_beamSpotCov.ResizeTo(beamSpotCov);
  m_beamSpotCov = beamSpotCov;
  m_useBeamSpot = true;
}

void RaveSetup::Print()
{
  cout << "use GFRave: " << m_gfRave << " | use Genfit propagation: " << m_gfPropagation << endl;
  if (m_useBeamSpot == false) {
    cout << "use beam spot is false" << endl;
  } else {
    cout << "use beam spot is true and beam spot position and covariance matrix are:" << endl;
    m_beamSpot.Print();
    m_beamSpotCov.Print();
  }
}
