/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/reconstruction/AlignableCDCRecoHit.h>

#include <alignment/GlobalLabel.h>
#include <alignment/GlobalDerivatives.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include "Math/ChebyshevPol.h"

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace alignment;

bool AlignableCDCRecoHit::s_enableTrackT0LocalDerivative = true;
bool AlignableCDCRecoHit::s_enableWireSaggingGlobalDerivative = false;
bool AlignableCDCRecoHit::s_enableWireByWireAlignmentGlobalDerivatives = false;

std::pair<std::vector<int>, TMatrixD> AlignableCDCRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  GlobalDerivatives globals;
  unsigned short LR = (int(m_leftRight) > 0.) ? 1 : 0;

  const TVector3& mom = sop->getMom();
  const TVector3& wirePositon = sop->getPlane()->getO();
  const unsigned short layer = getWireID().getICLayer();

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  double alpha = cdcgeo.getAlpha(wirePositon, mom);
  double theta = cdcgeo.getTheta(mom);
  const TVectorD& stateOnPlane = sop->getState();
  const double driftLengthEstimate = std::abs(stateOnPlane(3));
  const double driftTime = cdcgeo.getDriftTime(driftLengthEstimate, layer, LR, alpha, theta);
  const double driftVelocity = cdcgeo.getDriftV(driftTime, layer, LR, alpha, theta);

  // CDC Calibration -------------------------------------------------

  // Time zero calibration (per wire)
  if (driftTime > 20 && driftTime < 200 && fabs(driftVelocity) < 1.0e-2) {
    globals.add(
      GlobalLabel::construct<CDCTimeZeros>(getWireID(), 0),
      driftVelocity * double(int(m_leftRight))
    );
  }

  // Time walk calibration (per board)
  if (driftTime > 20 && driftTime < 200 && fabs(driftVelocity) < 1.0e-2 && m_adcCount < 400 && m_adcCount > 2) {
    DBObjPtr<CDCTimeWalks> tws;
    unsigned short board = CDCGeometryPar::Instance().getBoardID(getWireID());
    const std::vector<float> param  = tws->getTimeWalkParams(board);
    globals.add(
      GlobalLabel::construct<CDCTimeWalks>(board, 0),
      -driftVelocity * exp(-param[1]* m_adcCount) * double(int(m_leftRight))
    );
    globals.add(
      GlobalLabel::construct<CDCTimeWalks>(board, 1),
      driftVelocity * m_adcCount * param[0]*exp(-param[1]* m_adcCount) * double(int(m_leftRight))
    );
  }

  // Space time relations calibration
  if (driftTime > 20 && driftTime < 500 && fabs(driftVelocity) < 1.0e-2) {
    // TODO: ugly to need to ask XTRelations for something here...
    // Can't I get this CDCGeometryPar or sth. like this?

    theta = cdcgeo.getOutgoingTheta(alpha, theta);
    alpha = cdcgeo.getOutgoingAlpha(alpha);
    DBObjPtr<CDCXtRelations> xts;
    auto xtid = xts->getXtID(getWireID().getICLayer(), LR, (float)alpha, (float)theta);
    const auto& par = xts->getXtParams(xtid);
    auto boundary = par.at(6);
    if (driftTime < boundary) {
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 0),
        ROOT::Math::Chebyshev5(driftTime, 1, 0, 0, 0, 0, 0) * double(int(m_leftRight))
      );
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 1),
        ROOT::Math::Chebyshev5(driftTime, 0, 1, 0, 0, 0, 0) * double(int(m_leftRight))
      );
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 2),
        ROOT::Math::Chebyshev5(driftTime, 0, 0, 1, 0, 0, 0) * double(int(m_leftRight))
      );
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 3),
        ROOT::Math::Chebyshev5(driftTime, 0, 0, 0, 1, 0, 0) * double(int(m_leftRight))
      );
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 4),
        ROOT::Math::Chebyshev5(driftTime, 0, 0, 0, 0, 1, 0) * double(int(m_leftRight))
      );
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 5),
        ROOT::Math::Chebyshev5(driftTime, 0, 0, 0, 0, 0, 1) * double(int(m_leftRight))
      );
    } else {
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 7),
        double(int(m_leftRight))
      );
      globals.add(
        GlobalLabel::construct<CDCXtRelations>(xtid, 8),
        (driftTime - boundary) * double(int(m_leftRight))
      );
    }
  }


  // CDC Alignment ---------------------------------------------------

  auto tdir = sop->getDir();
  auto ndir = sop->getPlane()->getNormal();
  auto udir = sop->getPlane()->getU();
  auto vdir = sop->getPlane()->getV();
  auto pos  = sop->getPos();

  auto tn = tdir[0] * ndir[0] + tdir[1] * ndir[1] + tdir[2] * ndir [2];
  // d residual / d measurement
  auto drdm = TMatrixD(3, 3);
  drdm.UnitMatrix();
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      drdm(i, j) -= tdir[i] * ndir[j] / tn;
    }
  }
  // d measurement / d global rigid body param
  auto dmdg = TMatrixD(3, 6);
  dmdg.Zero();
  dmdg[0][0] = 1.; dmdg[0][4] = -pos[2]; dmdg[0][5] =  pos[1];
  dmdg[1][1] = 1.; dmdg[1][3] =  pos[2]; dmdg[1][5] = -pos[0];
  dmdg[2][2] = 1.; dmdg[2][3] = -pos[1]; dmdg[2][4] =  pos[0];
  // d local residual / d global residual
  auto drldrg = TMatrixD(3, 3);
  for (int i = 0; i < 3; ++i) {
    drldrg(0, i) = udir[i];
    drldrg(1, i) = vdir[i];
    drldrg(2, i) = ndir[i];
  }
  // d local residual / d global rigid body param
  auto drldg = drldrg * (drdm * dmdg);

  // wire ends
  const double zWireM = s_cdcGeometryTranslator->getWireBackwardPosition(getWireID(), CDCGeometryPar::c_Aligned)[2];
  const double zWireP = s_cdcGeometryTranslator->getWireForwardPosition(getWireID(), CDCGeometryPar::c_Aligned)[2];
  // relative Z position [0..1]
  const double zRel = std::max(0., std::min(1., (pos[2] - zWireM) / (zWireP - zWireM)));

  // Layer alignment
  // wire 511 = no wire (0 is reserved for existing wires) - this has to be compatible with code in CDCGeometryPar::setWirPosAlignParams
  auto layerID = WireID(getWireID().getICLayer(), 511);

  // Alignment of layer X (bwd)
  globals.add(
    GlobalLabel::construct<CDCAlignment>(layerID, CDCAlignment::layerX),
    drldg(0, 0)
  );

  // Alignment of layer Y (bwd)
  globals.add(
    GlobalLabel::construct<CDCAlignment>(layerID, CDCAlignment::layerY),
    drldg(0, 1)
  );

  // Alignment of layer rotation (gamma) (bwd)
  globals.add(
    GlobalLabel::construct<CDCAlignment>(layerID, CDCAlignment::layerPhi),
    drldg(0, 5)
  );

  // Difference between wire ends (end plates)
  // Alignment of layer dX, dX = foward - backward endplate
  globals.add(
    GlobalLabel::construct<CDCAlignment>(layerID, CDCAlignment::layerDx),
    drldg(0, 0) * zRel
  );

  // Alignment of layer dY, dY = foward - backward endplate
  globals.add(
    GlobalLabel::construct<CDCAlignment>(layerID, CDCAlignment::layerDy),
    drldg(0, 1) * zRel
  );

  // Alignment of layer rotation difference d(gamma or phi), dPhi = foward - backward endplate
  globals.add(
    GlobalLabel::construct<CDCAlignment>(layerID, CDCAlignment::layerDPhi),
    drldg(0, 5) * zRel
  );

  //WARNING: experimental (disabled by default)
  // Wire-by-wire alignment
  if (s_enableWireByWireAlignmentGlobalDerivatives) {
    // How much shift (in X or Y) on BWD wire-end will change the residual at estimated track crossing
    // with the wire (at relative z-position on wire = zRel)
    double zRelM = fabs(1. - zRel);
    // Same as above but for FWD wire-end (residual at zRel = zRel * delta(X or Y at FWD wire-end)
    double zRelP = fabs(zRel - 0.);

    // Alignment of wires X in global coords at BWD wire-end
    globals.add(
      GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), CDCAlignment::wireBwdX),
      drldg(0, 0) * zRelM
    );
    // Alignment of wires X in global coords at FWD wire-end
    globals.add(
      GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), CDCAlignment::wireFwdX),
      drldg(0, 0) * zRelP
    );

    // Alignment of wires Y in global coords at BWD wire-end
    globals.add(
      GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), CDCAlignment::wireBwdY),
      drldg(0, 1) * zRelM
    );
    // Alignment of wires Y in global coords at FWD wire-end
    globals.add(
      GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), CDCAlignment::wireFwdY),
      drldg(0, 1) * zRelP
    );
  }

  //WARNING: experimental (disabled by default)
  //TODO: missing some factors (we do not align directly the wire-sag coefficient, but
  // wire tension ... coef = pi * ro * r * r / 8 / tension
  //TODO: need to get these numbers from CDCGeometryPar!
  if (s_enableWireSaggingGlobalDerivative) {
    globals.add(
      GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), CDCAlignment::wireTension),
      drldg(0, 1) * 4.0 * zRel * (1.0 - zRel)
    );
  }



  return globals;
}


TMatrixD AlignableCDCRecoHit::localDerivatives(const genfit::StateOnPlane* sop)
{
  if (!s_enableTrackT0LocalDerivative)
    return TMatrixD();

  unsigned short LR = (int(m_leftRight) > 0.) ? 1 : 0;

  const TVector3& mom = sop->getMom();
  const TVector3& wirePositon = sop->getPlane()->getO();
  const unsigned short layer = getWireID().getICLayer();

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  const double alpha = cdcgeo.getAlpha(wirePositon, mom);
  const double theta = cdcgeo.getTheta(mom);
  const TVectorD& stateOnPlane = sop->getState();
  const double driftLengthEstimate = std::abs(stateOnPlane(3));
  const double driftTime = cdcgeo.getDriftTime(driftLengthEstimate, layer, LR, alpha, theta);
  const double driftVelocity = cdcgeo.getDriftV(driftTime, layer, LR, alpha, theta);

  TMatrixD locals(2, 1);
  if (driftTime > 20 && driftTime < 200 && fabs(driftVelocity) < 1.0e-2) {
    locals(0, 0) = - double(int(m_leftRight)) * driftVelocity;
    locals(1, 0) = 0.; // insesitive coordinate along wire
  }

  return locals;
}
