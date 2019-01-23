/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>

#include <alignment/Manager.h>
#include <alignment/Hierarchy.h>
#include <alignment/dbobjects/VXDAlignment.h>

#include <alignment/GlobalDerivatives.h>

#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;
using namespace alignment;

std::pair<std::vector<int>, TMatrixD> AlignableSVDRecoHit2D::globalDerivatives(const genfit::StateOnPlane* sop)
{
  auto alignment = GlobalCalibrationManager::getInstance().getAlignmentHierarchy().getGlobalDerivatives<VXDAlignment>(getPlaneId(),
                   sop);

  auto globals = GlobalDerivatives(alignment);

  bool applyDeformation(true); // To determine planar deformation

  //if (applyDeformation and getSensorID() == VxdID("4.3.2")) {
  if (applyDeformation) {

    const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(getSensorID()));

    auto L1 = [](double x) {return x;};
    auto L2 = [](double x) {return (3 * pow(x, 2) - 1) / 2;};
    auto L3 = [](double x) {return (5 * pow(x, 3) - 3 * x) / 2;};
    auto L4 = [](double x) {return (35 * pow(x, 4) - 30 * pow(x, 2) + 3) / 8;};

    double du_dw = sop->getState()[1];       // slope in U direction
    double dv_dw = sop->getState()[2];       // slope in V direction
    double u = getU();                       // U coordinate of hit
    double v = getV();                       // V coordinate of hit
    double width = geometry.getWidth(v);     // Width of sensor (U side)
    double length = geometry.getLength();    // Length of sensor (V side)
    u = u * 2 / width;                       // Legendre parametrization required U in (-1, 1)
    v = v * 2 / length;                      // Legendre parametrization required V in (-1, 1)

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
  }
  return globals;
}
