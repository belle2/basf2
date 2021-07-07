/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/VertexFitting/RaveInterface/RaveSetup.h>

#include <rave/VacuumPropagator.h>
#include <rave/MagneticField.h>
#include <rave/ConstantMagneticField.h>
#include <rave/VertexFactory.h>
#include <rave/KinematicTreeFactory.h>

//stl stuff
#include <string>
using std::string;
#include <iostream>


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
//  getRawInstance()->m_GFRaveVertexFactory = new GFRaveVertexFactory(verbosity, true);


  getRawInstance()->m_raveVertexFactory = new rave::VertexFactory(rave::ConstantMagneticField(0, 0, magneticField),
      rave::VacuumPropagator(), "kalman", verbosity);
  getRawInstance()->m_raveKinematicTreeFactory = new rave::KinematicTreeFactory(rave::ConstantMagneticField(0, 0, magneticField),
      rave::VacuumPropagator(), verbosity);
  getRawInstance()->m_initialized = true;
}

RaveSetup::RaveSetup(): m_useBeamSpot(false), m_raveVertexFactory(nullptr)/*, m_GFRaveVertexFactory(NULL)*/,
  m_raveKinematicTreeFactory(nullptr), m_initialized(false)
{
}

RaveSetup::~RaveSetup()
{
  reset();
}

void RaveSetup::reset()
{
  //delete everything that could have potentially created with new in this class

  delete m_raveVertexFactory; // workaround to avoid crashes with V0 finder
  m_raveVertexFactory = nullptr;
  delete m_raveKinematicTreeFactory;
  m_raveKinematicTreeFactory = nullptr;
  //    delete m_GFRaveVertexFactory;

  m_initialized = false;
}


void RaveSetup::setBeamSpot(const TVector3& beamSpot, const TMatrixDSym& beamSpotCov)
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
  if (getRawInstance() not_eq nullptr) {
    if (getRawInstance()->m_useBeamSpot == false) {
      B2INFO("use beam spot is false");
    } else {
      B2INFO("use beam spot is true and beam spot position and covariance matrix are:");
      getRawInstance()->m_beamSpot.Print();
      getRawInstance()->m_beamSpotCov.Print();
    }
    B2INFO("the pointer to rave::VertexFactory is " << getRawInstance()->m_raveVertexFactory);
  } else {
    B2INFO("RaveSetup::initialize was not called. There is nothing to Print.");
  }
}
