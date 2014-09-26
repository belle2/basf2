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
//#include <genfit/ConstField.h>
//#include <genfit/FieldManager.h>
//#include <genfit/FieldManager.h>
//#include <TGeoManager.h>
//#include <geometry/GeometryManager.h>
//#include <geometry/bfieldmap/BFieldMap.h>
//#include <tracking/gfbfield/GFGeant4Field.h>
//#include <genfit/TGeoMaterialInterface.h>
//#include <genfit/MaterialEffects.h>
//stl stuff
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::cerr; using std::endl;


using namespace Belle2;
using namespace analysis;

RaveSetup* RaveSetup::getRawInstance()
{
  static RaveSetup instance;
  return &instance;
}

void RaveSetup::initialize(int verbosity, double magneticField)
{
  if (getRawInstance()->m_initialized)
    getRawInstance()->reset();
  //now setup everything for the use of GFRave
//  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
//    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
//    geoManager.createTGeoRepresentation();
//    //pass the magnetic field to genfit
//    GFFieldManager::getInstance()->init(new GFGeant4Field());
//    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
//    GFMaterialEffects::getInstance()->setMscModel("Highland");
//  }
//  getRawInstance()->m_GFRaveVertexFactory = new GFRaveVertexFactory(verbosity, true);


  getRawInstance()->m_raveVertexFactory = new rave::VertexFactory(rave::ConstantMagneticField(0, 0, magneticField), rave::VacuumPropagator(), "kalman", verbosity);
  getRawInstance()->m_raveKinematicTreeFactory = new rave::KinematicTreeFactory(rave::ConstantMagneticField(0, 0, magneticField), rave::VacuumPropagator(), verbosity);
  getRawInstance()->m_initialized = true;
}

RaveSetup::RaveSetup(): m_useBeamSpot(false), m_raveVertexFactory(NULL)/*, m_GFRaveVertexFactory(NULL)*/, m_raveKinematicTreeFactory(NULL), m_initialized(false)
{
}

RaveSetup::~RaveSetup()
{
  reset();
}

void RaveSetup::reset()
{
  //delete everything that could have potentially created with new in this class
  delete m_raveVertexFactory;
  delete m_raveKinematicTreeFactory;
  //    delete m_GFRaveVertexFactory;
  m_raveVertexFactory = nullptr;
  m_raveKinematicTreeFactory = nullptr;

  m_initialized = false;
}


void RaveSetup::setBeamSpot(TVector3 beamSpot, TMatrixDSym beamSpotCov)
{
  m_beamSpot = beamSpot;
  m_beamSpotCov.ResizeTo(beamSpotCov);
  m_beamSpotCov = beamSpotCov;
  m_useBeamSpot = true;
}

void RaveSetup::unsetBeamSpot()
{
  m_useBeamSpot = false;
}



void RaveSetup::Print()
{
  if (getRawInstance() not_eq NULL) {
    if (getRawInstance()->m_useBeamSpot == false) {
      cout << "use beam spot is false" << endl;
    } else {
      cout << "use beam spot is true and beam spot position and covariance matrix are:" << endl;
      getRawInstance()->m_beamSpot.Print();
      getRawInstance()->m_beamSpotCov.Print();
    }
    cout << "the pointer to rave::VertexFactory is " << getRawInstance()->m_raveVertexFactory << endl;
//    cout << "the pointer to GFRaveVertexFactory is " << getRawInstance()->m_GFRaveVertexFactory << endl;
  } else {
    cout << "RaveSetup::initialize was not called. There is nothing to Print." << endl;
  }
}
