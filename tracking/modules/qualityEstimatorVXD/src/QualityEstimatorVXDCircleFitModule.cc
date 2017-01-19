/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorVXDCircleFitModule.h>
#include <framework/logging/Logger.h>

#include <tracking/vxdCaTracking/TrackletFilters.h>
#include <tracking/vxdCaTracking/SharedFunctions.h> // e.g. PositionInfo

// ROOT
#include <TVector3.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;


REG_MODULE(QualityEstimatorVXDCircleFit)

QualityEstimatorVXDCircleFitModule::QualityEstimatorVXDCircleFitModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates using a circleFit.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it", string(""));

}



void QualityEstimatorVXDCircleFitModule::event()
{
  m_eventCounter++;
  B2DEBUG(1, "\n" << "QualityEstimatorVXDCircleFitModule:event: event " << m_eventCounter << "\n");
  m_nTCsTotal += m_spacePointTrackCands.getEntries();


  /** WARNING!
   *  THIS IS A TEMPORARY SOLUTION!
   * The Karimäki-circleFit used here is using the interface of the old VXDTF and it is planned to use the CDC-Version of the same fitter to reduce redundancy of implementations.
   * Please remove this reminder as soon as this is done.
   * Jakob Lettenbichler Feb 29th, 2016
   *
   * WARNING hardcoded values so far, should be passed by parameter (or be solved in a general way)!
   * */


  double bFieldValue = 1.5; /**< magnetic field. WARNING hardcoded! */

  auto fitter = TrackletFilters();
  fitter.resetMagneticField(bFieldValue);

  unsigned nTC = 0;
  // assign a QI computed using a circleFit for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    unsigned nHits = aTC.size();

    // prepare interface-container for TrackletFilters:
    std::vector<PositionInfo> convertedPathrawData;
    convertedPathrawData.reserve(nHits);
    std::vector<PositionInfo*> convertedPath;
    convertedPath.reserve(nHits);

    // collecting actual hits
    for (const SpacePoint* aHit : aTC.getSortedHits()) { // is a const SpacePoint* here
      PositionInfo convertedHit{
        TVector3(aHit->getPosition()),
        TVector3(aHit->getPositionError()),
        0,
        0};
      convertedPathrawData.push_back(std::move(convertedHit));
      convertedPath.push_back(&convertedPathrawData.back());
    }

    fitter.resetValues(&convertedPath);

    std::pair<double, TVector3> result = fitter.circleFit();
    double chi2 = result.first;

    if (chi2 < 0) { B2WARNING("QualityEstimatorVXDCircleFitModule: event " << m_eventCounter << ": chi2 is reset to 0! (before: " << chi2 << ")"); chi2 = 0; }
    double probability = TMath::Prob(chi2, nHits -
                                     3); // -3: 3 parameters are estimated, which reduces the nDF (number of Hits, since only one ndF per hit is added in a circleFit (unlike the situation for helix fits)

    B2DEBUG(1, "QualityEstimatorVXDCircleFitModule: event " << m_eventCounter
            << ": TC " << nTC
            << " with " << nHits
            << " hits has pT: " << result.second.Perp()
            << ", chi^2: " << chi2
            << " and probability: " << probability);

    aTC.setQualityIndex(probability);
    ++nTC;
  }
}



void QualityEstimatorVXDCircleFitModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("QualityEstimatorVXDCircleFitModule:endRun: events: " << m_eventCounter
         << ", nSPTCsPerEvent: " << invEvents * float(m_nTCsTotal)
        );
}



void QualityEstimatorVXDCircleFitModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_nTCsTotal = 0;
}
