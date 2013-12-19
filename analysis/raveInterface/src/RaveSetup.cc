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

RaveSetup* RaveSetup::s_instance = NULL;

void RaveSetup::initialize(int verbosity, double MagneticField)
{
  if (s_instance == NULL) {
    s_instance = new RaveSetup();
  } else {
    delete s_instance;
    s_instance = new RaveSetup();
  }
  //now setup everything for the use of GFRave
//  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
//    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
//    geoManager.createTGeoRepresentation();
//    //pass the magnetic field to genfit
//    GFFieldManager::getInstance()->init(new GFGeant4Field());
//    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
//    GFMaterialEffects::getInstance()->setMscModel("Highland");
//  }
//  s_instance->m_GFRaveVertexFactory = new GFRaveVertexFactory(verbosity, true);

  //now setup everything for the dirct use of Rave without GFRave

  s_instance->m_raveVertexFactory = new rave::VertexFactory(rave::ConstantMagneticField(0, 0, MagneticField), rave::VacuumPropagator(), "kalman", verbosity);

  s_instance->m_raveKinematicTreeFactory = new rave::KinematicTreeFactory(rave::ConstantMagneticField(0, 0, MagneticField), rave::VacuumPropagator(), verbosity);

}

RaveSetup::RaveSetup(): m_useBeamSpot(false), m_raveVertexFactory(NULL)/*, m_GFRaveVertexFactory(NULL)*/, m_raveKinematicTreeFactory(NULL)
{
  ;
}

RaveSetup::~RaveSetup()
{
  //delete everything that could have potentially created with new in this class
  if (m_raveVertexFactory not_eq NULL) {
    delete m_raveVertexFactory;
  }
  if (m_raveKinematicTreeFactory not_eq NULL) {
    delete m_raveKinematicTreeFactory;
  }
//  if (m_GFRaveVertexFactory not_eq NULL) {
//    delete m_GFRaveVertexFactory;
//  }
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
  if (s_instance not_eq NULL) {
    if (s_instance->m_useBeamSpot == false) {
      cout << "use beam spot is false" << endl;
    } else {
      cout << "use beam spot is true and beam spot position and covariance matrix are:" << endl;
      s_instance->m_beamSpot.Print();
      s_instance->m_beamSpotCov.Print();
    }
    cout << "the pointer to rave::VertexFactory is " << s_instance->m_raveVertexFactory << endl;
//    cout << "the pointer to GFRaveVertexFactory is " << s_instance->m_GFRaveVertexFactory << endl;
  } else {
    cout << "RaveSetup::initialize was not called. There is nothing to Print." << endl;
  }
}
