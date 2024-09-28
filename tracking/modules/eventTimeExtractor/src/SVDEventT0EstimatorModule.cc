/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/eventTimeExtractor/SVDEventT0EstimatorModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/dataobjects/EventT0.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dbobjects/SVDEventT0Configuration.h>

#include <cmath>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDEventT0Estimator);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDEventT0EstimatorModule::SVDEventT0EstimatorModule() : Module()
{
  setDescription("This module estimates the EventT0 as the average of cluster time of SVD clusters associated to tracks. The EventT0 is set to NaN if there are not RecoTracks or there are not SVD clusters associated to tracks or RecoTrack pt < ptMin OR RecoTrack pz < pzMin. The EventT0 estimated is added to the temporaryEventT0s to the StoreObjPtr as EventT0Component that cointains: eventT0, eventT0_error, detector=SVD, algorithm, quality.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //* Definition of input parameters */
  addParam("RecoTracks", m_recoTracksName, "Name of the StoreArray with the input RecoTracks", std::string(""));
  addParam("EventT0", m_eventT0Name, "Name of the StoreObjPtr with the input EventT0", std::string(""));
  addParam("ptMinSelection", m_ptSelection, "Cut on minimum transverse momentum pt for RecoTrack selection", m_ptSelection);
  addParam("absPzMinSelection", m_absPzSelection,
           "Cut on minimum absolute value of the longitudinal momentum, abs(pz), for RecoTrack selection",
           m_absPzSelection);
  addParam("absD0Selection", m_absD0Selection,
           "Cut on maximum absolute value of the d0 for RecoTrack selection", m_absD0Selection);
  addParam("absZ0Selection", m_absZ0Selection,
           "Cut on maximum absolute value of the z0 for RecoTrack selection", m_absZ0Selection);
  addParam("selectTracksFromIP", m_selectTracksFromIP,
           "Apply the selection based on the absolute values of d0 and z0 to select tracks from the IP to compute SVDEventT0",
           m_selectTracksFromIP);
  addParam("useDB", m_useDB, "To compute EvetT0, use configuration of selections stored in the DB", m_useDB);
}


SVDEventT0EstimatorModule::~SVDEventT0EstimatorModule()
{
}


void SVDEventT0EstimatorModule::initialize()
{
  B2DEBUG(20, "RecoTracks: " << m_recoTracksName);
  B2DEBUG(20, "EventT0: " << m_eventT0Name);

  /** Register the data object */
  m_eventT0.registerInDataStore();
  m_recoTracks.isRequired(m_recoTracksName);
}

void SVDEventT0EstimatorModule::beginRun()
{
  if (m_useDB) {
    if (!m_svdEventT0Config.isValid())
      B2FATAL("no valid configuration found for SVD EventT0 computation");
    else
      B2DEBUG(20, "SVDEventT0Configuration: from now on we are using " << m_svdEventT0Config->get_uniqueID());


    m_selectTracksFromIP = m_svdEventT0Config->getSelectTracksFromIP();
    m_ptSelection = m_svdEventT0Config->getMinimumPtSelection();
    m_absPzSelection = m_svdEventT0Config->getAbsPzSelection();
    m_absD0Selection = m_svdEventT0Config->getAbsD0Selection();
    m_absZ0Selection = m_svdEventT0Config->getAbsZ0Selection();
  }
}

void SVDEventT0EstimatorModule::event()
{

  double evtT0 = std::numeric_limits<double>::quiet_NaN();
  double evtT0Err = std::numeric_limits<double>::quiet_NaN();
  double armTimeSum = 0;
  double armTimeErrSum = 0;
  double quality = std::numeric_limits<double>::quiet_NaN();
  int numberOfSVDClusters = 0;
  int numberOfRecoTracksUsed = 0;
  float outgoingArmTime = 0;
  float ingoingArmTime = 0;
  float outgoingArmTimeError = 0;
  float ingoingArmTimeError = 0;

  // loop on recotracks
  for (auto& recoTrack : m_recoTracks) {
    const B2Vector3D& p = recoTrack.getMomentumSeed();
    const UncertainHelix uncertainHelix = constructUncertainHelix(recoTrack);
    double d0 = uncertainHelix.getD0();
    double z0 = uncertainHelix.getZ0();

    // selection on recoTracks
    if (p.Perp() < m_ptSelection || std::fabs(p.Z()) < m_absPzSelection) continue;

    if (m_selectTracksFromIP) {
      if (std::fabs(d0) > m_absD0Selection || std::fabs(z0) > m_absZ0Selection) continue;
    }

    // use outgoing/ingoing arm time to compute SVD EventT0
    // if both outgoing and ingoing are estimated we take the smallest one
    // else if only outgoing or only ingoing is computed we use the only one available
    // the probability that the ingoing arm is an outgoing arm wrongly classified is higher than the probability that it is a real ingoing arm
    outgoingArmTime = recoTrack.getOutgoingArmTime();
    ingoingArmTime = recoTrack.getIngoingArmTime();
    outgoingArmTimeError = recoTrack.getOutgoingArmTimeError();
    ingoingArmTimeError = recoTrack.getIngoingArmTimeError();
    bool hasOutgoingArm = recoTrack.hasOutgoingArmTime();
    bool hasIngoingArm = recoTrack.hasIngoingArmTime();

    // check if it has both ingoing and outgoing arms
    if (hasOutgoingArm && hasIngoingArm) {
      // consider the smallest arm time
      if (outgoingArmTime <= ingoingArmTime) {
        armTimeSum += outgoingArmTime * recoTrack.getNSVDHitsOfOutgoingArm();
        armTimeErrSum += outgoingArmTimeError * outgoingArmTimeError * recoTrack.getNSVDHitsOfOutgoingArm() *
                         (recoTrack.getNSVDHitsOfOutgoingArm() - 1);
        numberOfSVDClusters += recoTrack.getNSVDHitsOfOutgoingArm();
      } else {
        armTimeSum += ingoingArmTime * recoTrack.getNSVDHitsOfIngoingArm();
        armTimeErrSum += ingoingArmTimeError * ingoingArmTimeError * recoTrack.getNSVDHitsOfIngoingArm() *
                         (recoTrack.getNSVDHitsOfIngoingArm() - 1);
        numberOfSVDClusters += recoTrack.getNSVDHitsOfIngoingArm();
      }
      numberOfRecoTracksUsed += 1;
    } else if (hasOutgoingArm && !hasIngoingArm) { // check if it has only outgoing arm
      armTimeSum += outgoingArmTime * recoTrack.getNSVDHitsOfOutgoingArm();
      armTimeErrSum += outgoingArmTimeError * outgoingArmTimeError * recoTrack.getNSVDHitsOfOutgoingArm() *
                       (recoTrack.getNSVDHitsOfOutgoingArm() - 1);
      numberOfSVDClusters += recoTrack.getNSVDHitsOfOutgoingArm();
      numberOfRecoTracksUsed += 1;
    } else if (!hasOutgoingArm && hasIngoingArm) { // check if it has only ingoing arm
      armTimeSum += ingoingArmTime * recoTrack.getNSVDHitsOfIngoingArm();
      armTimeErrSum += ingoingArmTimeError * ingoingArmTimeError * recoTrack.getNSVDHitsOfIngoingArm() *
                       (recoTrack.getNSVDHitsOfIngoingArm() - 1);
      numberOfSVDClusters += recoTrack.getNSVDHitsOfIngoingArm();
      numberOfRecoTracksUsed += 1;
    } else continue;
  }


  // do nothing if no recoTracks are used (no outgoing/ingoing arm time exists = no SVD clusters associated to tracks exist), or if EventT0 is not valid
  if ((numberOfRecoTracksUsed == 0) || !(m_eventT0.isValid())) return;

  // otherwise, eventT0 is the average of outgoing/ingoing arm time
  // that are estimated using SVD clusters associated to recoTracks
  evtT0 = armTimeSum / numberOfSVDClusters;
  quality = numberOfSVDClusters;

  // now compute the error
  if (numberOfSVDClusters > 1)
    evtT0Err = std::sqrt(armTimeErrSum / (numberOfSVDClusters * (numberOfSVDClusters - 1)));
  else
    evtT0Err = std::sqrt(armTimeErrSum);

  // and finally set a temporary EventT0
  EventT0::EventT0Component evtT0Component(evtT0, evtT0Err, Const::SVD, m_algorithm, quality);
  m_eventT0->addTemporaryEventT0(evtT0Component);
  m_eventT0->setEventT0(evtT0Component);

}

const UncertainHelix SVDEventT0EstimatorModule::constructUncertainHelix(const RecoTrack& recoTrack)
{

  const ROOT::Math::XYZVector& position = recoTrack.getPositionSeed();
  const B2Vector3D& momentum = recoTrack.getMomentumSeed();
  const TMatrixDSym& covariance = recoTrack.getSeedCovariance();
  short int charge = recoTrack.getChargeSeed();
  if (charge < 0) charge = -1;
  else charge = 1;
  const float pValue = float(std::numeric_limits<double>::quiet_NaN());
  const float bField = Belle2::BFieldManager::getFieldInTesla(ROOT::Math::XYZVector(0, 0, 0)).Z();

  const UncertainHelix uncertainHelix(position, momentum, charge, bField, covariance, pValue);

  return uncertainHelix;
}
