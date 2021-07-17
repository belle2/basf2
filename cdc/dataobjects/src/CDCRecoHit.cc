/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/dataobjects/CDCRecoHit.h>

//Comment out the following line since it introduces dependence on cdclib (or circular dependence betw. cdc_objects and cdclib).
//#include <cdc/geometry/CDCGeometryPar.h>
#include <genfit/WireTrackCandHit.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/RKTrackRep.h>
#include <genfit/TrackPoint.h>
#include <genfit/AbsFitterInfo.h>
#include <genfit/Exception.h>
#include <genfit/HMatrixU.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

std::unique_ptr<ADCCountTranslatorBase>    CDCRecoHit::s_adcCountTranslator    = 0;
std::unique_ptr<CDCGeometryTranslatorBase> CDCRecoHit::s_cdcGeometryTranslator = 0;
std::unique_ptr<TDCCountTranslatorBase>    CDCRecoHit::s_tdcCountTranslator    = 0;
bool                                       CDCRecoHit::s_useTrackTime          = false;
//temp4cosmics
bool                                       CDCRecoHit::s_cosmics = false;


void CDCRecoHit::setTranslators(ADCCountTranslatorBase*    const adcCountTranslator,
                                CDCGeometryTranslatorBase* const cdcGeometryTranslator,
                                TDCCountTranslatorBase*    const tdcCountTranslator,
                                //temp4cosmics                                bool useTrackTime)
                                bool useTrackTime, bool cosmics)
{
  s_adcCountTranslator.reset(adcCountTranslator);
  s_cdcGeometryTranslator.reset(cdcGeometryTranslator);
  s_tdcCountTranslator.reset(tdcCountTranslator);
  s_useTrackTime = useTrackTime;
  //temp4cosmics
  s_cosmics = cosmics;
}

CDCRecoHit::CDCRecoHit()
  : genfit::AbsMeasurement(1),
    m_tdcCount(0), m_adcCount(0), m_wireID(WireID()), m_cdcHit(nullptr), m_leftRight(0)
{
}

CDCRecoHit::CDCRecoHit(const CDCHit* cdcHit, const genfit::TrackCandHit* trackCandHit)
  : genfit::AbsMeasurement(1), m_cdcHit(cdcHit), m_leftRight(0)
{
  if (s_adcCountTranslator == 0 || s_cdcGeometryTranslator == 0 || s_tdcCountTranslator == 0) {
    B2FATAL("Can't produce CDCRecoHits without setting of the translators.");
  }

  // get information from cdcHit into local variables.
  m_wireID = cdcHit->getID();
  m_tdcCount = cdcHit->getTDCCount();
  m_adcCount = cdcHit->getADCCount();

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
  //temp
  //  std::cout << (noSagWire1 + noSagWireDirection.Dot(noSagPoca - noSagWire1) * noSagWireDirection).y() <<" "<< pocaOnWire.y() <<" " << (noSagWire1 + noSagWireDirection.Dot(noSagPoca - noSagWire1) * noSagWireDirection - pocaOnWire).y() << std::endl;
  //  std::cout << (noSagWire1 + noSagWireDirection.Dot(noSagPoca - noSagWire1) * noSagWireDirection).Perp() <<" "<< pocaOnWire.Perp() << std::endl;
  //  std::cout << (noSagWire1 + noSagWireDirection.Dot(noSagPoca - noSagWire1) * noSagWireDirection).z() <<" "<< pocaOnWire.z() << std::endl;

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
  // Calculate alpha and theta.  A description was given in
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
  //temp4cosmics
  //  std::cout <<"phi,trackTime= " << atan2(py,px) <<" "<< trackTime << std::endl;
  if (s_cosmics) {
    if (atan2(py, px) > 0.) {
      //    if (atan2(wy,wx) > 0.) {
      trackTime *= -1.;
    }
  }

  // The meaning of the left / right flag (called
  // 'ambiguityDiscriminator' in TDCCounTranslatorBase) is inferred
  // from CDCGeometryPar::getNewLeftRightRaw().
  double mL = s_tdcCountTranslator->getDriftLength(m_tdcCount, m_wireID, trackTime,
                                                   false, //left
                                                   z, alpha, theta, m_adcCount);
  double mR = s_tdcCountTranslator->getDriftLength(m_tdcCount, m_wireID, trackTime,
                                                   true, //right
                                                   z, alpha, theta, m_adcCount);
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
    // in the innermost superlayer, 18 mm otherwise.  Use these values
    // times a safety margin of 1.5 as upper bound for the drift
    // radii.  The max distance between the mirror hits is twice the
    // maximal drift radius.
    double rMax = 1.5 * (m_wireID.getISuperLayer() == 0 ? 1. : 1.8);
    double weight = 0.5 * pow(std::max(0., 1 - (mR + mL) / 2 / rMax), 2);
    mopL->setWeight(weight);
    mopR->setWeight(weight);
  }

  // Ignore hits with negative drift times.  For these, the
  // TDCCountTranslator returns a negative drift length.
  if (mL < 0. || mR < 0.) {
    B2DEBUG(150, "Ignoring hit with negative drift time.");
    mopL->setWeight(0);
    mopR->setWeight(0);
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


std::vector<double> CDCRecoHit::timeDerivativesMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  double z = state.getPos().Z();
  const TVector3& p = state.getMom();
  // Calculate alpha and theta.  A description was given by in
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
  auto fL = [&](const double & t) -> double {
    return s_tdcCountTranslator->getDriftLength(m_tdcCount, m_wireID, t,
    false, //left
    z, alpha, theta, m_adcCount); };
  auto fR = [&](const double & t) -> double {
    return s_tdcCountTranslator->getDriftLength(m_tdcCount, m_wireID, t,
    true, //right
    z, alpha, theta, m_adcCount); };

  // Calculate derivative for all left and right mirror hit.
  //
  // It's a polynomial, but let's not meddle with the innard of the
  // CDC code for now.
  //
  // The algorithm follows the one in TF1::Derivative() :
  //   df(x) = (4 D(h/2) - D(h)) / 3
  // with D(h) = (f(x + h) - f(x - h)) / (2 h).
  double rightShort[2], rightFull[2];
  double leftShort[2], leftFull[2];
  const double defaultStepT = 1e-3 * trackTime;
  double stepT;
  {
    double temp = trackTime + defaultStepT / 2;
    // Find the actual size of the step, which will differ from
    // defaultStepX due to roundoff.  This is the step-size we will
    // use for this direction.  Idea taken from Numerical Recipes,
    // 3rd ed., section 5.7.
    //
    // Note that if a number is exactly representable, it's double
    // will also be exact.  Outside denormals, this also holds for
    // halving.  Unless the exponent changes (which it only will in
    // the vicinity of zero) adding or subtracing doesn't make a
    // difference.
    //
    // We determine the roundoff error for the half-step.  If this
    // is exactly representable, the full step will also be.
    stepT = 2 * (temp - trackTime);

    rightShort[0] = fL(trackTime + .5 * stepT);
    rightShort[1] = fR(trackTime + .5 * stepT);
  }
  {
    leftShort[0] = fL(trackTime - .5 * stepT);
    leftShort[1] = fR(trackTime - .5 * stepT);
  }
  {
    rightFull[0] = fL(trackTime + stepT);
    rightFull[1] = fR(trackTime + stepT);
  }
  {
    leftFull[0] = fL(trackTime - stepT);
    leftFull[1] = fR(trackTime - stepT);
  }

  // Calculate the derivatives for the individual components of
  // the track parameters.
  double derivFull[2];
  double derivShort[2];
  for (size_t j = 0; j < 2; ++j) {
    derivFull[j] = (rightFull[j] - leftFull[j]) / (2.*stepT);
    derivShort[j] = (rightShort[j] - leftShort[j]) / stepT;
  }
  //std::cout << rightShort[0] << " " << derivShort[0] << " " << trackTime << std::endl;
  return { +(4.*derivShort[0] - derivFull[0]) / 3.,
           -(4.*derivShort[1] - derivFull[1]) / 3.};
}



bool CDCRecoHit::getFlyByDistanceVector(B2Vector3D& pointingVector,
                                        B2Vector3D& trackDir,
                                        const genfit::AbsTrackRep* rep,
                                        bool usePlaneFromFit)
{
  const genfit::TrackPoint* tp = this->getTrackPoint();
  if (!tp) {
    B2ERROR("No genfit::TrackPoint for CDCRecoHit.");
    return false;
  }
  const genfit::AbsFitterInfo* fi = tp->getFitterInfo(rep);
  if (!fi) {
    B2DEBUG(200, "No genfit::AbsFitterInfo for this CDCRecoHit.");
    return false;
  }

  const genfit::MeasuredStateOnPlane& mop = fi->getFittedState();
  B2Vector3D fittedPoca = mop.getPos();
  // constructPlane places the coordinate center in the POCA to the
  // wire.  Using this is the default behavior.  If this should be too
  // slow, as it has to re-evaluate the POCA, alternatively the user
  // can set usePlaneFromFit which uses the plane determined by the
  // track fit.
  B2Vector3D pocaOnWire = (usePlaneFromFit
                           ? mop.getPlane()->getO()
                           : this->constructPlane(mop)->getO());

  // The vector from the wire to the track.
  pointingVector = fittedPoca - pocaOnWire;

  trackDir = mop.getMom();
  trackDir.SetMag(1.);
  return true;
}
