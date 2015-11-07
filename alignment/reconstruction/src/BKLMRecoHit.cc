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
#include <alignment/reconstruction/BKLMRecoHit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/geometry/GeometryPar.h>
#include <alignment/GlobalLabel.h>

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
  module = m_GeoPar->findModule(hit->isForward(), hit->getSector(), hit->getLayer());

  //+++ layer number
  layer = hit->getLayer();

  //+++ global coordinates of the hit
  global[0] = hit->getGlobalPosition()[0];
  global[1] = hit->getGlobalPosition()[1];
  global[2] = hit->getGlobalPosition()[2];

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
  TVectorD predFglo = state.get6DState();
  TVectorD correctedLocal(2);

  //do correction for scintillators
  if (layer == 1 || layer == 2) {
    CLHEP::Hep3Vector global_shift_z(0, 0, predFglo[5]*halfheight_sci / sqrt(predFglo[3]*predFglo[3] + predFglo[4]*predFglo[4]));
    CLHEP::Hep3Vector local_corrected_z = module->globalToLocal(global - global_shift_z);
    correctedLocal[0] = local_corrected_z[1];
    correctedLocal[1] = local_corrected_z[2];
  } else {
    correctedLocal[0] = rawHitCoords_[0];
    correctedLocal[1] = rawHitCoords_[1];
  }

  // return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(),
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(correctedLocal, rawHitCov_, state.getPlane(),
                                                  state.getRep(), this->constructHMatrix(state.getRep())));
}

vector< int > BKLMRecoHit::labels()
{
  int barrel = 1;
  int forward = 1; // m_bklmHit2d->isForward() ? 1 : 0;
  int sector = m_bklmHit2d->getSector();
  int layer = m_bklmHit2d->getLayer();

  // Encode some KLM-unique id to identify alignable structure
  // - it should be smaller than 1.000.000, which
  // means moduleID cannot be used for this!
  // This is Millepede limitation. One int must be enough to identify any
  // single alignment parameter in the whole Belle 2 detector.
  int klmid(layer + 100 * sector + 1000 * forward + 10000 * barrel);

  std::vector<int> labGlobal;

  labGlobal.push_back(GlobalLabel(klmid, 1)); // du
  labGlobal.push_back(GlobalLabel(klmid, 2)); // dv
  labGlobal.push_back(GlobalLabel(klmid, 3)); // dw
  labGlobal.push_back(GlobalLabel(klmid, 4)); // dalpha
  labGlobal.push_back(GlobalLabel(klmid, 5)); // dbeta
  labGlobal.push_back(GlobalLabel(klmid, 6)); // dgamma

  return labGlobal;
}

TMatrixD BKLMRecoHit::derivatives(const genfit::StateOnPlane* sop)
{
  // Matrix of global derivatives
  TMatrixD derGlobal(2, 6);
  derGlobal.Zero();

  // track u-slope in local sensor system
  double uSlope = sop->getState()[1];
  // track v-slope in local sensor system
  double vSlope = sop->getState()[2];
  // Predicted track u-position in local sensor system
  double uPos = sop->getState()[3];
  // Predicted track v-position in local sensor system
  double vPos = sop->getState()[4];

  // Global derivatives for alignment in module local coordinates
  derGlobal(0, 0) = 1.0;
  derGlobal(0, 1) = 0.0;
  derGlobal(0, 2) = - uSlope;
  derGlobal(0, 3) = vPos * uSlope;
  derGlobal(0, 4) = -uPos * uSlope;
  derGlobal(0, 5) = vPos;

  derGlobal(1, 0) = 0.0;
  derGlobal(1, 1) = 1.0;
  derGlobal(1, 2) = - vSlope;
  derGlobal(1, 3) = vPos * vSlope;
  derGlobal(1, 4) = -uPos * vSlope;
  derGlobal(1, 5) = -uPos;

  return derGlobal;
}




