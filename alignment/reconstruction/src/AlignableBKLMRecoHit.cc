/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/reconstruction/AlignableBKLMRecoHit.h>

#include <alignment/GlobalDerivatives.h>
#include <alignment/GlobalLabel.h>
#include <alignment/Hierarchy.h>
#include <framework/geometry/B2Vector3.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dbobjects/bklm/BKLMAlignment.h>
#include <klm/dataobjects/KLMElementNumbers.h>

#include <genfit/DetPlane.h>

using namespace std;
using namespace Belle2;

AlignableBKLMRecoHit::AlignableBKLMRecoHit(const BKLMHit2d* hit, const genfit::TrackCandHit*):
  genfit::PlanarMeasurement(HIT_DIMENSIONS)
{
  int section = hit->getSection();
  int sector = hit->getSector();
  m_Layer = hit->getLayer();
  const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
  m_KLMModule = elementNumbers->moduleNumberBKLM(section, sector, m_Layer);

  bklm::GeometryPar* m_GeoPar = Belle2::bklm::GeometryPar::instance();
  m_Module = m_GeoPar->findModule(section, sector, m_Layer);

  //+++ global coordinates of the hit
  global[0] = hit->getGlobalPosition()[0];
  global[1] = hit->getGlobalPosition()[1];
  global[2] = hit->getGlobalPosition()[2];

  //+++ local coordinates of the hit
  CLHEP::Hep3Vector local = m_Module->globalToLocal(global);

  //+++ local coordinates in KLM m_Layer of the hit
  double localU = local[1]; //phi
  double localV = local[2]; //z
  double errorU = m_Module->getPhiStripWidth() / sqrt(12);
  double errorV = m_Module->getZStripWidth() / sqrt(12);

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

  //+++ Construct vectors o, u, v of m_Layer in global coords.
  CLHEP::Hep3Vector gOrigin = m_Module->getGlobalOrigin();
  CLHEP::Hep3Vector lOrigin = m_Module->globalToLocal(gOrigin) + m_Module->getLocalReconstructionShift();
  CLHEP::Hep3Vector gOrigin_midway = m_Module->localToGlobal(lOrigin);
  CLHEP::Hep3Vector uAxis(0, 1, 0);
  CLHEP::Hep3Vector vAxis(0, 0, 1);
  CLHEP::Hep3Vector gUaxis = m_Module->localToGlobal(uAxis) - gOrigin;
  CLHEP::Hep3Vector gVaxis = m_Module->localToGlobal(vAxis) - gOrigin;

  //!the position (in global coordinates) of this module's sensitive-volume origin
  B2Vector3D origin_mid(gOrigin_midway[0], gOrigin_midway[1], gOrigin_midway[2]);

  //!the directioin (in global coordinates) of this module's U axis
  B2Vector3D uGlobal(gUaxis[0], gUaxis[1], gUaxis[2]);
  //!the directioin (in global coordinates) of this module's V axis
  B2Vector3D vGlobal(gVaxis[0], gVaxis[1], gVaxis[2]);

  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin_mid, uGlobal, vGlobal, 0));
  setPlane(detPlane, m_KLMModule);
}

genfit::AbsMeasurement* AlignableBKLMRecoHit::clone() const
{
  return new AlignableBKLMRecoHit(*this);
}

std::vector<genfit::MeasurementOnPlane*> AlignableBKLMRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  TVectorD predFglo = state.get6DState();
  TVectorD correctedLocal(2);
  CLHEP::Hep3Vector localmom(predFglo[3], predFglo[4], predFglo[5]);
  localmom = m_Module->RotateToLocal(localmom);

  //do correction for scintillators
  if (m_Layer == 1 || m_Layer == 2) {
    CLHEP::Hep3Vector global_shift_z(0, 0, predFglo[5]*halfheight_sci / sqrt(predFglo[3]*predFglo[3] + predFglo[4]*predFglo[4]));
    CLHEP::Hep3Vector local_corrected_z = m_Module->globalToLocal(global - global_shift_z);
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

std::pair<std::vector<int>, TMatrixD> AlignableBKLMRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  std::vector<int> labGlobal;

  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaU));
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaV));
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaW));
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaAlpha));
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaBeta));
  labGlobal.push_back(GlobalLabel::construct<BKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaGamma));

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




