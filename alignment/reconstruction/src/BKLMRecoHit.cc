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
#include <bklm/dataobjects/BKLMElementID.h>
#include <alignment/dbobjects/BKLMAlignment.h>

#include <alignment/Hierarchy.h>
#include <alignment/GlobalDerivatives.h>


#include <genfit/DetPlane.h>
#include <TVector3.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;


BKLMRecoHit::BKLMRecoHit(const BKLMHit2d* hit, const genfit::TrackCandHit*):
  genfit::PlanarMeasurement(HIT_DIMENSIONS)
{
  m_moduleID = hit->getModuleID();
  m_IsForward = hit->isForward();
  m_Sector = hit->getSector();
  m_Layer = hit->getLayer();

  bklm::GeometryPar*  m_GeoPar = Belle2::bklm::GeometryPar::instance();
  module = m_GeoPar->findModule(m_IsForward, m_Sector, m_Layer);

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

  if (hit->inRPC()) {
    //+++ scale localU and localV based on measured-in-Belle resolution
    int nStrips = hit->getPhiStripMax() - hit->getPhiStripMin() + 1;
    double dn = nStrips - 1.5;
    double factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.60;//measured-in-Belle resolution
    errorU = errorU * sqrt(factor);

    nStrips = hit->getZStripMax() - hit->getZStripMin() + 1;
    dn = nStrips - 1.5;
    factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.55;//measured-in-Belle resolution
    errorV = errorV * sqrt(factor);
  }

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
  CLHEP::Hep3Vector lOrigin = module->globalToLocal(gOrigin) + module->getLocalReconstructionShift();
  CLHEP::Hep3Vector gOrigin_midway = module->localToGlobal(lOrigin);
  CLHEP::Hep3Vector uAxis(0, 1, 0);
  CLHEP::Hep3Vector vAxis(0, 0, 1);
  CLHEP::Hep3Vector gUaxis = module->localToGlobal(uAxis) - gOrigin;
  CLHEP::Hep3Vector gVaxis = module->localToGlobal(vAxis) - gOrigin;

  //!the position (in global coordinates) of this module's sensitive-volume origin
  TVector3 origin_mid(gOrigin_midway[0], gOrigin_midway[1], gOrigin_midway[2]);

  //!the directioin (in global coordinates) of this module's U axis
  TVector3 uGlobal(gUaxis[0], gUaxis[1], gUaxis[2]);
  //!the directioin (in global coordinates) of this module's V axis
  TVector3 vGlobal(gVaxis[0], gVaxis[1], gVaxis[2]);

  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin_mid, uGlobal, vGlobal, 0));
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
  CLHEP::Hep3Vector localmom(predFglo[3], predFglo[4], predFglo[5]);
  localmom = module->RotateToLocal(localmom);

  //do correction for scintillators
  if (m_Layer == 1 || m_Layer == 2) {
    CLHEP::Hep3Vector global_shift_z(0, 0, predFglo[5]*halfheight_sci / sqrt(predFglo[3]*predFglo[3] + predFglo[4]*predFglo[4]));
    CLHEP::Hep3Vector local_corrected_z = module->globalToLocal(global - global_shift_z);
    double local_shift_u = localmom[1] / localmom[0] * halfheight_sci;
    correctedLocal[0] = local_corrected_z[1] + local_shift_u;
    //correctedLocal[0] = local_corrected_z[1];
    correctedLocal[1] = local_corrected_z[2];
  } else {
    correctedLocal[0] = rawHitCoords_[0];
    correctedLocal[1] = rawHitCoords_[1];
  }

  // return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(),
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(correctedLocal, rawHitCov_, state.getPlane(),
                                                  state.getRep(), this->constructHMatrix(state.getRep())));
}

std::pair<std::vector<int>, TMatrixD> BKLMRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{

  BKLMElementID klmid;
  klmid.setIsForward(m_IsForward);
  klmid.setSectorNumber(m_Sector);
  klmid.setLayerNumber(m_Layer);
  std::vector<int> labGlobal;

  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(klmid, BKLMAlignment::dU)); // du
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(klmid, BKLMAlignment::dV));// dv
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(klmid, BKLMAlignment::dW)); // dw
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(klmid, BKLMAlignment::dAlpha)); // dalpha
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(klmid, BKLMAlignment::dBeta)); // dbeta
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(klmid, BKLMAlignment::dGamma)); // dgamma

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

  return alignment::GlobalDerivatives(labGlobal, derGlobal);

}




