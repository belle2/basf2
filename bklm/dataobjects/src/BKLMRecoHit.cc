/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan, Tadeas Bilka                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <bklm/dataobjects/BKLMRecoHit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/geometry/GeometryPar.h>

#include <genfit/DetPlane.h>
#include <TVector3.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;

ClassImp(BKLMRecoHit)

BKLMRecoHit::BKLMRecoHit():
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_bklmHit2d(0)
{}

BKLMRecoHit::BKLMRecoHit(const BKLMHit2d* hit, const genfit::TrackCandHit*):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_moduleID(0), m_bklmHit2d(hit)
{

  //derived from BKLMHit2d
  m_moduleID = hit->getModuleID();

  bklm::GeometryPar*  m_GeoPar = Belle2::bklm::GeometryPar::instance();
  const bklm::Module* module = m_GeoPar->findModule(hit->isForward(), hit->getSector(), hit->getLayer());

  //+++ global coordinates of the hit
  CLHEP::Hep3Vector global(hit->getGlobalPosition()[0], hit->getGlobalPosition()[1], hit->getGlobalPosition()[2]);

  //+++ local coordinates of the hit
  CLHEP::Hep3Vector local = module->globalToLocal(global);

  //+++ local coordinates in KLM layer of the hit
  double localU = local[1]; //phi
  double localV = local[2]; //z
  double errorU = module->getPhiStripWidth() / sqrt(12);
  double errorV = module->getZStripWidth() / sqrt(12);

  //Set positions
  rawHitCoords_(0) = localU;
  rawHitCoords_(1) = localV;
  //Set the error covariance matrix
  rawHitCov_(0, 0) = errorU * errorU; // error in U, squared
  rawHitCov_(0, 1) = 0.;
  rawHitCov_(1, 0) = 0.;
  rawHitCov_(1, 1) = errorV * errorV; // error in V, squared

  //+++ Construct vectors o, u, v of layer in global coords.
  CLHEP::Hep3Vector gOrigin = module->getGlobalOrigin();
  CLHEP::Hep3Vector uAxis(0, 1, 0);
  CLHEP::Hep3Vector vAxis(0, 0, 1);
  CLHEP::Hep3Vector gUaxis = module->localToGlobal(uAxis) - gOrigin;
  CLHEP::Hep3Vector gVaxis = module->localToGlobal(vAxis) - gOrigin;

  //!the position (in global coordinates) of this module's sensitive-volume origin
  TVector3 origin(gOrigin[0], gOrigin[1], gOrigin[2]);

  //!the directioin (in global coordinates) of this module's U axis
  TVector3 uGlobal(gUaxis[0], gUaxis[1], gUaxis[2]);
  //!the directioin (in global coordinates) of this module's V axis
  TVector3 vGlobal(gVaxis[0], gVaxis[1], gVaxis[2]);

  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin, uGlobal, vGlobal, 0));
  setPlane(detPlane, m_moduleID);
}

genfit::AbsMeasurement* BKLMRecoHit::clone() const
{
  return new BKLMRecoHit(*this);
}

std::vector<genfit::MeasurementOnPlane*> BKLMRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(),
                                                  state.getRep(), this->constructHMatrix(state.getRep())));
}



