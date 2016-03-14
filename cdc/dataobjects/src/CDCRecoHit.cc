/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCRecoHit.h>

//Comment out the following line since it introduces dependence on cdclib (or circular dependence betw. cdc_objects and cdclib).
//#include <cdc/geometry/CDCGeometryPar.h>
#include <genfit/WireTrackCandHit.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/RKTrackRep.h>
#include <genfit/Exception.h>
#include <genfit/HMatrixU.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

ClassImp(CDCRecoHit);

std::unique_ptr<ADCCountTranslatorBase>    CDCRecoHit::s_adcCountTranslator    = 0;
std::unique_ptr<CDCGeometryTranslatorBase> CDCRecoHit::s_cdcGeometryTranslator = 0;
std::unique_ptr<TDCCountTranslatorBase>    CDCRecoHit::s_tdcCountTranslator    = 0;
bool                                       CDCRecoHit::s_useTrackTime          = false;


void CDCRecoHit::setTranslators(ADCCountTranslatorBase*    const adcCountTranslator,
                                CDCGeometryTranslatorBase* const cdcGeometryTranslator,
                                TDCCountTranslatorBase*    const tdcCountTranslator,
                                bool useTrackTime)
{
  s_adcCountTranslator.reset(adcCountTranslator);
  s_cdcGeometryTranslator.reset(cdcGeometryTranslator);
  s_tdcCountTranslator.reset(tdcCountTranslator);
  s_useTrackTime = useTrackTime;
}

CDCRecoHit::CDCRecoHit()
  : genfit::AbsMeasurement(),
    m_tdcCount(0), m_wireID(WireID()), m_cdcHit(NULL), m_leftRight(0)
{
}

CDCRecoHit::CDCRecoHit(const CDCHit* cdcHit, const genfit::TrackCandHit* trackCandHit)
  : genfit::AbsMeasurement(), m_cdcHit(cdcHit), m_leftRight(0)
{
  if (s_adcCountTranslator == 0 || s_cdcGeometryTranslator == 0 || s_tdcCountTranslator == 0) {
    B2FATAL("Can't produce CDCRecoHits without setting of the translators.")
  }

  // get information from cdcHit into local variables.
  m_wireID = cdcHit->getID();
  m_tdcCount = cdcHit->getTDCCount();

  // set l-r info
  const genfit::WireTrackCandHit* aTrackCandHitPtr =  dynamic_cast<const genfit::WireTrackCandHit*>(trackCandHit);
  if (aTrackCandHitPtr) {
    signed char lrInfo = aTrackCandHitPtr->getLeftRightResolution();
    B2DEBUG(250, "l/r: " << int(lrInfo));
    setLeftRightResolution(lrInfo);
  }
}

CDCRecoHit* CDCRecoHit::clone() const
{
  return new CDCRecoHit(*this);
}


genfit::SharedPlanePtr CDCRecoHit::constructPlane(const genfit::StateOnPlane& state) const
{
  // We find the plane in two steps: first we neglect wire sag to get
  // a good estimate of the z coordinate of the crossing.  Then we use
  // this z coordinate to find the point of closest approach to the
  // sagging wire and its local direction.

  // Don't use clone: we don't want to extrapolate covariances if
  // state is a genfit::MeasuredStateOnPlane.
  genfit::StateOnPlane st(state);

  const TVector3& noSagWire1(s_cdcGeometryTranslator->getWireBackwardPosition(m_wireID));
  const TVector3& noSagWire2(s_cdcGeometryTranslator->getWireForwardPosition(m_wireID));

  // unit vector along the wire
  TVector3 noSagWireDirection = noSagWire2 - noSagWire1;
  noSagWireDirection.SetMag(1.);

  // point of closest approach
  const genfit::AbsTrackRep* rep = state.getRep();
  rep->extrapolateToLine(st, noSagWire1, noSagWireDirection);
  const TVector3& noSagPoca = rep->getPos(st);

  double zPOCA = (noSagWire1.Z()
                  + noSagWireDirection.Dot(noSagPoca - noSagWire1) * noSagWireDirection.Z());

  // Now re-extrapolate taking Z of trajectory into account.
  const TVector3& wire1(s_cdcGeometryTranslator->getWireBackwardPosition(m_wireID, zPOCA));
  const TVector3& wire2(s_cdcGeometryTranslator->getWireForwardPosition(m_wireID, zPOCA));

  // unit vector along the wire (will become V of plane)
  TVector3 wireDirection = wire2 - wire1;
  wireDirection.SetMag(1.);

  // point of closest approach
  rep->extrapolateToLine(st, wire1, wireDirection);
  const TVector3& poca = rep->getPos(st);
  TVector3 dirInPoca = rep->getMom(st);
  dirInPoca.SetMag(1.);
  const TVector3& pocaOnWire = wire1 + wireDirection.Dot(poca - wire1) * wireDirection;

  // check if direction is parallel to wire
  if (fabs(wireDirection.Angle(dirInPoca)) < 0.01) {
    genfit::Exception exc("CDCRecoHit::constructPlane(): Cannot construct detector plane, direction is parallel to wire", __LINE__,
                          __FILE__);
    throw exc;
  }

  // construct orthogonal (unit) vector
  const TVector3& U = wireDirection.Cross(dirInPoca);

  genfit::SharedPlanePtr pl = genfit::SharedPlanePtr(new genfit::DetPlane(pocaOnWire, U, wireDirection));
  //pl->Print();
  return pl;
}

std::vector<genfit::MeasurementOnPlane*> CDCRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  double z = state.getPos().Z();
  const TVector3& p = state.getMom();
  // Calculate alpha and theta.  A description was given by H. Ozaki in
  // https://indico.mpp.mpg.de/getFile.py/access?contribId=5&sessionId=3&resId=0&materialId=slides&confId=3195

//Comment out the following 2 lines since they introduce dependence on cdclib (or circular dependence betw. cdc_objects and cdclib).
//  double alpha = CDCGeometryPar::Instance().getAlpha(state.getPlane()->getO(), p);
//  double theta = CDCGeometryPar::Instance().getTheta(p);

//N.B. The folowing 8 lines are tentative to avoid the circular dependence mentioned above ! The definitions of alpha and theta should be identical to those defined in CDCGeometryPar.
  const double wx = state.getPlane()->getO().x();
  const double wy = state.getPlane()->getO().y();
  const double px = p.x();
  const double py = p.y();
  const double cross = wx * py - wy * px;
  const double dot   = wx * px + wy * py;
  double alpha = atan2(cross, dot);
  double theta = atan2(p.Perp(), p.z());
  /*
  double alpha0 =  CDCGeometryPar::Instance().getAlpha(state.getPlane()->getO(), p);
  double theta0 =  CDCGeometryPar::Instance().getTheta(p);
  if (alpha != alpha0) {
    std::cout <<"alpha,alpha0= " << alpha <<" "<< alpha0 << std::endl;
    exit(-1);
  }
  if (theta != theta0) {;
    std::cout <<"theta,theta0= " << theta <<" "<< theta0 << std::endl;
    exit(-2);
  }
  */

  double trackTime = s_useTrackTime ? state.getTime() : 0;

  // The meaning of the left / right flag (called
  // 'ambiguityDiscriminator' in TDCCounTranslatorBase) is inferred
  // from CDCGeometryPar::getNewLeftRightRaw().
  double mL = s_tdcCountTranslator->getDriftLength(m_tdcCount, m_wireID, trackTime,
                                                   false, //left
                                                   z, alpha, theta);
  double mR = s_tdcCountTranslator->getDriftLength(m_tdcCount, m_wireID, trackTime,
                                                   true, //right
                                                   z, alpha, theta);
  double VL = s_tdcCountTranslator->getDriftLengthResolution(mL, m_wireID,
                                                             false, //left
                                                             z, alpha, theta);
  double VR = s_tdcCountTranslator->getDriftLengthResolution(mR, m_wireID,
                                                             true, //right
                                                             z, alpha, theta);

  // static to avoid constructing these over and over.
  static TVectorD m(1);
  static TMatrixDSym cov(1);

  m(0) = mR;
  cov(0, 0) = VR;
  auto mopR = new genfit::MeasurementOnPlane(m, cov, state.getPlane(), state.getRep(),
                                             constructHMatrix(state.getRep()));
  m(0) = -mL; // Convert from unsigned drift length to signed coordinate.
  cov(0, 0) = VL;
  auto mopL = new genfit::MeasurementOnPlane(m, cov, state.getPlane(), state.getRep(),
                                             constructHMatrix(state.getRep()));

  // set left/right weights
  if (m_leftRight < 0) {
    mopL->setWeight(1);
    mopR->setWeight(0);
  } else if (m_leftRight > 0) {
    mopL->setWeight(0);
    mopR->setWeight(1);
  } else {
    // In absence of L/R information set equal weight for mirror hits.
    // We have this weight decrease as the drift distance increases.
    // Since the average will always be close to the wire, the bias
    // introduced by the averaged hit would increase as the drift
    // radius increases.  Reducing the initial weight effectively
    // counteracts this.  This way the DAF can figure the ambiguities
    // out quickly.
    //
    // Wire spacing in the radial direction according to TDR is 10 mm
    // in the innermost superlayer, 18 mm otherwise.  Use these times
    // a safety margin of 1.5 as upper bound for the drift radii.  The
    // max distance between the mirror hits is twice the maximal drift
    // radius.
    double rMax = m_wireID.getISuperLayer() == 0 ? 1.5 : 2.7;
    double weight = 0.5 * pow(std::max(0., 1 - (mR + mL) / 2 / rMax), 2);
    mopL->setWeight(weight);
    mopR->setWeight(weight);
  }

  return {mopL, mopR};
}


const genfit::HMatrixU* CDCRecoHit::constructHMatrix(const genfit::AbsTrackRep* rep) const
{
  if (!dynamic_cast<const genfit::RKTrackRep*>(rep)) {
    B2FATAL("CDCRecoHit can only handle state vectors of type RKTrackRep!");
  }

  return new genfit::HMatrixU();
}

std::vector< int > CDCRecoHit::labels()
{
  return std::vector< int >();
}

TMatrixD CDCRecoHit::derivatives(const genfit::StateOnPlane*)
{
  return TMatrixD();
}

TMatrixD CDCRecoHit::localDerivatives(const genfit::StateOnPlane*)
{
  return TMatrixD();
}

std::vector< int > CDCRecoHit::localLabels()
{
  return std::vector< int >();
}
