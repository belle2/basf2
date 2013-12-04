/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck,                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCRecoHit.h>
#include <genfit/WireTrackCandHit.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

ClassImp(CDCRecoHit);

const double CDCRecoHit::c_HMatrixContent[5] = {0, 0, 0, 1, 0};
const TMatrixD CDCRecoHit::c_HMatrix = TMatrixD(1, 5, c_HMatrixContent);

//--- Translator initialization; should be removed, once this are shared_ptr ----------------------------------------------------
ADCCountTranslatorBase*    CDCRecoHit::s_adcCountTranslator    = 0;
CDCGeometryTranslatorBase* CDCRecoHit::s_cdcGeometryTranslator = 0;
TDCCountTranslatorBase*    CDCRecoHit::s_tdcCountTranslator    = 0;
bool CDCRecoHit::s_update = false;


void CDCRecoHit::setTranslators(ADCCountTranslatorBase*    const adcCountTranslator,
                                CDCGeometryTranslatorBase* const cdcGeometryTranslator,
                                TDCCountTranslatorBase*    const tdcCountTranslator)
/*
static void setTranslators(boost::shared_ptr<ADCCountTranslatorBase>    const& adcCountTranslator,
                           boost::shared_ptr<CDCGeometryTranslatorBase> const& cdcGeometryTranslator,
                           boost::shared_ptr<TDCCountTranslatorBase>   const& driftTimeTranslator)
 */
{
  s_adcCountTranslator    = adcCountTranslator;
  s_cdcGeometryTranslator = cdcGeometryTranslator;
  s_tdcCountTranslator    = tdcCountTranslator;
}

void CDCRecoHit::setUpdate(bool update)
{
  s_update = update;
}

CDCRecoHit::CDCRecoHit()
  : genfit::WireMeasurement(c_nParHitRep),
    m_adcCount(0), m_charge(0), m_tdcCount(0), m_driftLength(0), m_driftLengthResolution(0), m_wireID(WireID()), m_cdcHit(NULL)
{
}

CDCRecoHit::CDCRecoHit(const CDCHit* cdcHit, const genfit::TrackCandHit* trackCandHit)
  : genfit::WireMeasurement(c_nParHitRep), m_cdcHit(cdcHit)
{
  if (s_adcCountTranslator == 0 || s_cdcGeometryTranslator == 0 || s_tdcCountTranslator == 0) {
    B2FATAL("Can't produce CDCRecoHits without setting of the translators.")
  }

  // get information from cdcHit into local variables.
  m_wireID      = cdcHit->getID();

  m_tdcCount              = cdcHit->getTDCCount();
  m_driftLength           = s_tdcCountTranslator->getDriftLength(m_tdcCount, m_wireID);
  m_driftLengthResolution = s_tdcCountTranslator->getDriftLengthResolution(m_driftLength, m_wireID);

  m_adcCount    = cdcHit->getADCCount();
  m_charge      = s_adcCountTranslator->getCharge(m_adcCount, m_wireID);

  // forward wire position
  TVector3 dummyVector3 = s_cdcGeometryTranslator->getWireForwardPosition(m_wireID);
  rawHitCoords_(0) = dummyVector3.X();
  rawHitCoords_(1) = dummyVector3.y();
  rawHitCoords_(2) = dummyVector3.z();
  // backward wire position
  dummyVector3 = s_cdcGeometryTranslator->getWireBackwardPosition(m_wireID);
  rawHitCoords_(3) = dummyVector3.X();
  rawHitCoords_(4) = dummyVector3.Y();
  rawHitCoords_(5) = dummyVector3.Z();

  rawHitCoords_(6) = m_driftLength;
  rawHitCov_(6, 6) = m_driftLengthResolution;

  B2DEBUG(250, "CDCRecoHit assigned drift-length " << m_driftLength
          << ", drift-length resolution" << m_driftLengthResolution << ", dummyVector3"
          << dummyVector3.X()  <<  ", " << dummyVector3.Y() << ", " << dummyVector3.Z());

  // set l-r info
  const genfit::WireTrackCandHit* aTrackCandHitPtr =  dynamic_cast<const genfit::WireTrackCandHit*>(trackCandHit);
  if (aTrackCandHitPtr not_eq NULL) {
    char lrInfo = aTrackCandHitPtr->getLeftRightResolution();
    B2DEBUG(250, "l/r: " << int(lrInfo));
    setLeftRightResolution(lrInfo);
  }
}

genfit::AbsMeasurement* CDCRecoHit::clone() const
{
  return new CDCRecoHit(*this);
}


std::vector<genfit::MeasurementOnPlane*> CDCRecoHit::constructMeasurementsOnPlane(const genfit::AbsTrackRep* rep,
    const genfit::SharedPlanePtr& pl) const
{
  if (s_update) {
    B2FATAL("The extraction of track/hit parameters for the getMeasurement function has still to be implemented. \n"
            << "Please avoid setting s_update to true for the moment.");
  }
  return this->genfit::WireMeasurement::constructMeasurementsOnPlane(rep, pl);
}

