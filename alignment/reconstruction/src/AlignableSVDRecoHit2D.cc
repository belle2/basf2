/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>

#include <alignment/GlobalLabel.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/database/DBObjPtr.h>
#include <alignment/dbobjects/VXDAlignment.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorPlane.h>

#include <TGeoMatrix.h>

using namespace std;
using namespace Belle2;

ClassImp(AlignableSVDRecoHit2D);

std::string AlignableSVDRecoHit2D::m_misalignmentDBObjPtrName = "";


TMatrixD AlignableSVDRecoHit2D::derivatives(const genfit::StateOnPlane* sop)
{

  // values for global derivatives
  //TMatrixD derGlobal(2, 6);
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

  //Global derivatives for alignment in sensor local coordinates

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

vector< int > AlignableSVDRecoHit2D::labels()
{
  VxdID vxdid(getPlaneId());

  std::vector<int> labGlobal;

  labGlobal.push_back(GlobalLabel(vxdid, 1)); // du
  labGlobal.push_back(GlobalLabel(vxdid, 2)); // dv
  labGlobal.push_back(GlobalLabel(vxdid, 3)); // dw
  labGlobal.push_back(GlobalLabel(vxdid, 4)); // dalpha
  labGlobal.push_back(GlobalLabel(vxdid, 5)); // dbeta
  labGlobal.push_back(GlobalLabel(vxdid, 6)); // dgamma

  return labGlobal;
}

void AlignableSVDRecoHit2D::setDetectorPlane()
{
  double du = 0., dv = 0., dw = 0., dalpha = 0., dbeta = 0., dgamma = 0.;

  if (m_misalignmentDBObjPtrName != "") {
    DBObjPtr<VXDAlignment> misalignment(m_misalignmentDBObjPtrName);
    if (misalignment.isValid()) {
      du = misalignment->get(getSensorID(), 0);
      dv = misalignment->get(getSensorID(), 1);
      dw = misalignment->get(getSensorID(), 2);
      dalpha = misalignment->get(getSensorID(), 3);
      dbeta = misalignment->get(getSensorID(), 4);
      dgamma = misalignment->get(getSensorID(), 5);
    }
  }

  DBObjPtr<VXDAlignment> alignment;
  if (alignment.isValid()) {
    du = du - alignment->get(getSensorID(), 0);
    dv = dv - alignment->get(getSensorID(), 1);
    dw = dw - alignment->get(getSensorID(), 2);
    dalpha = dalpha - alignment->get(getSensorID(), 3);
    dbeta = dbeta - alignment->get(getSensorID(), 4);
    dgamma = dgamma - alignment->get(getSensorID(), 5);
  }

  // Construct a finite detector plane and set it.
  auto& geometry = VXD::GeoCache::get(getSensorID());

  // Construct vectors o, u, v
  double clocal[3];
  double cmaster[3];

  TVector3 localOrigin(0, 0, 0);
  TVector3 localU(1, 0, 0);
  TVector3 localV(0, 1, 0);

  TGeoHMatrix trafo(geometry.getTransformation());
  TGeoTranslation T;
  TGeoRotation R;

  T.SetTranslation(du, dv, dw);
  Double_t m[9];
  double alfa = dalpha;
  double beta = dbeta;
  double gama = dgamma;
  m[0] = cos(beta) * cos(gama);
  m[1] = cos(alfa) * sin(gama) + sin(alfa) * sin(beta) * cos(gama);
  m[2] = sin(alfa) * sin(gama) - cos(alfa) * sin(beta) * cos(gama);
  m[3] = - cos(beta) * sin(gama);
  m[4] = cos(alfa) * cos(gama) - sin(alfa) * sin(beta) * sin(gama);
  m[5] = sin(alfa) * cos(gama) + cos(alfa) * sin(beta) * cos(gama);
  m[6] = sin(beta);
  m[7] = - sin(alfa) * cos(beta);
  m[8] = cos(alfa) * cos(beta);
  R.SetMatrix(m);

  TGeoCombiTrans TR(T, R);
  trafo = trafo * TR;

  localOrigin.GetXYZ(clocal);
  trafo.LocalToMaster(clocal, cmaster);
  TVector3 origin(cmaster);
  localU.GetXYZ(clocal);
  trafo.LocalToMasterVect(clocal, cmaster);
  TVector3 uGlobal(cmaster);
  localV.GetXYZ(clocal);
  trafo.LocalToMasterVect(clocal, cmaster);
  TVector3 vGlobal(cmaster);

  //Construct the detector plane
  genfit::DetPlane* p = new genfit::DetPlane(origin, uGlobal, vGlobal, new VXD::SensorPlane(getSensorID(), 20, 20));
  genfit::SharedPlanePtr detPlane(p);
  setPlane(detPlane, getSensorID());

}
