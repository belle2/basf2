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
  : GFRecoHitIfc<GFWireHitPolicy> (c_nParHitRep),
    m_adcCount(0), m_charge(0), m_tdcCount(0), m_driftLength(0), m_driftLengthResolution(0), m_wireID(WireID())
{
  fPolicy.setLeftRightResolution(0);
}

CDCRecoHit::CDCRecoHit(const CDCHit* cdcHit)
  : GFRecoHitIfc<GFWireHitPolicy> (c_nParHitRep)
{
  fPolicy.setLeftRightResolution(0);
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
  fHitCoord[0][0] = dummyVector3.X();
  fHitCoord[1][0] = dummyVector3.y();
  fHitCoord[2][0] = dummyVector3.z();
  // backward wire position
  dummyVector3 = s_cdcGeometryTranslator->getWireBackwardPosition(m_wireID);
  fHitCoord[3][0] = dummyVector3.X();
  fHitCoord[4][0] = dummyVector3.Y();
  fHitCoord[5][0] = dummyVector3.Z();

  fHitCoord[6][0] = m_driftLength;
  fHitCov[6][6] = m_driftLengthResolution;

  B2DEBUG(250, "CDCRecoHit assigned drift-length " << m_driftLength
          << ", drift-length resolution" << m_driftLengthResolution << ", dummyVector3"
          << dummyVector3.X()  <<  ", " << dummyVector3.Y() << ", " << dummyVector3.Z());
}

GFAbsRecoHit* CDCRecoHit::clone()
{
  return new CDCRecoHit(*this);
}

TMatrixD CDCRecoHit::getHMatrix(const GFAbsTrackRep* /*stateVector*/)
{
  //don't check for specific Track Representation at the moment, as RKTrackRep is the only one we are currently using.
  return (c_HMatrix);
}

void CDCRecoHit::getMeasurement(const GFAbsTrackRep* trackRep, const GFDetPlane& plane, const TMatrixT<double>& whatever, const TMatrixT<double>& whatever2,
                                TMatrixT<double>& m, TMatrixT<double>& V)
{
  if (s_update) {
    B2FATAL("The extraction of track/hit parameters for the getMeasurement function has still to be implemented. \n"
            << "Please avoid setting s_update to true for the moment.");
  } else {
    GFRecoHitIfc<GFWireHitPolicy>::getMeasurement(trackRep, plane, whatever, whatever2, m, V);
  }
}
