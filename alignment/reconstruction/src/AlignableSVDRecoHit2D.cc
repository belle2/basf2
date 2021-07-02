/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>

#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/GlobalDerivatives.h>
#include <alignment/Hierarchy.h>
#include <alignment/Manager.h>
#include <framework/geometry/BFieldManager.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;
using namespace alignment;

bool AlignableSVDRecoHit2D::s_enableLorentzGlobalDerivatives = false;

std::pair<std::vector<int>, TMatrixD> AlignableSVDRecoHit2D::globalDerivatives(const genfit::StateOnPlane* sop)
{
  auto alignment = GlobalCalibrationManager::getInstance().getAlignmentHierarchy().getGlobalDerivatives<VXDAlignment>(getPlaneId(),
                   sop);

  auto globals = GlobalDerivatives(alignment);

  if (s_enableLorentzGlobalDerivatives) {
    auto lorentz = GlobalCalibrationManager::getInstance().getLorentzShiftHierarchy().getGlobalDerivatives<VXDAlignment>(getPlaneId(),
                   sop, BFieldManager::getInstance().getField(sop->getPos()));
    globals.add(lorentz);
  }

  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(getSensorID()));

  // Legendre parametrization of deformation
  auto L1 = [](double x) {return x;};
  auto L2 = [](double x) {return (3 * x * x - 1) / 2;};
  auto L3 = [](double x) {return (5 * x * x * x - 3 * x) / 2;};
  auto L4 = [](double x) {return (35 * x * x * x * x - 30 * x * x + 3) / 8;};

  double du_dw = sop->getState()[1];       // slope in U direction
  double dv_dw = sop->getState()[2];       // slope in V direction
  double u = getU();                       // U coordinate of hit
  double v = getV();                       // V coordinate of hit
  double width = geometry.getWidth(v);     // Width of sensor (U side)
  double length = geometry.getLength();    // Length of sensor (V side)
  u = u * 2 / width;                       // Legendre parametrization required U in (-1, 1)
  v = v * 2 / length;                      // Legendre parametrization required V in (-1, 1)

  // Add parameters of surface deformation to alignment
  // Numbering of VXD alignment parameters:
  //  -> 0-6:   Rigid body alignment
  //  -> 31-33: First level of surface deformation
  //  -> 41-44: Second level of surface deformation
  //  -> 51-55: Third level of surface deformation
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 31), std::vector<double> {L2(u)*du_dw,       L2(u)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 32), std::vector<double> {L1(u)*L1(v)*du_dw, L1(u)*L1(v)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 33), std::vector<double> {L2(v)*du_dw,       L2(v)*dv_dw});

  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 41), std::vector<double> {L3(u)*du_dw,       L3(u)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 42), std::vector<double> {L2(u)*L1(v)*du_dw, L2(u)*L1(v)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 43), std::vector<double> {L1(u)*L2(v)*du_dw, L1(u)*L2(v)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 44), std::vector<double> {L3(v)*du_dw,       L3(v)*dv_dw});

  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 51), std::vector<double> {L4(u)*du_dw,       L4(u)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 52), std::vector<double> {L3(u)*L1(v)*du_dw, L3(u)*L1(v)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 53), std::vector<double> {L2(u)*L2(v)*du_dw, L2(u)*L2(v)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 54), std::vector<double> {L1(u)*L3(v)*du_dw, L1(u)*L3(v)*dv_dw});
  globals.add(GlobalLabel::construct<VXDAlignment>(getSensorID(), 55), std::vector<double> {L4(v)*du_dw,       L4(v)*dv_dw});

  return globals;
}
