/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorVXDTripletFitModule.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;


REG_MODULE(QualityEstimatorVXDTripletFit)

QualityEstimatorVXDTripletFitModule::QualityEstimatorVXDTripletFitModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates using a circleFit.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it", string(""));

}



void QualityEstimatorVXDTripletFitModule::event()
{
  m_eventCounter++;
  B2DEBUG(1, "\n" << "QualityEstimatorVXDTripletFitModule:event: event " << m_eventCounter << "\n");
  m_nTCsTotal += m_spacePointTrackCands.getEntries();


  /** WARNING!
   * Hardcoded values so far, should be passed by parameter (or be solved in a general way)!
   **/

  double bFieldValue = 1.5; /**< magnetic field. WARNING hardcoded! */

  auto fitter = QualityEstimators();
  fitter.resetMagneticField(bFieldValue);

  unsigned nTC = 0;
  // assign a QI computed using a triplet fit for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    unsigned nHits = aTC.size();

    // prepare interface-container for TrackletFilters:
    std::vector<PositionInfo> convertedPathrawData;
    convertedPathrawData.reserve(nHits);
    std::vector<PositionInfo*> convertedPath;
    convertedPath.reserve(nHits);

    // collecting actual hits
    for (const SpacePoint* aHit : aTC) { // is a const SpacePoint* here
      PositionInfo convertedHit{
        TVector3(aHit->getPosition()),
        TVector3(aHit->getPositionError()),
        0,
        0};
      convertedPathrawData.push_back(std::move(convertedHit));
      convertedPath.push_back(&convertedPathrawData.back());
    }

    fitter.resetValues(&convertedPath);

    std::pair<double, TVector3> result = fitter.tripletFit();
    double chi2 = result.first;

    if (chi2 < 0) { B2WARNING("QualityEstimatorVXDTripletFitModule: event " << m_eventCounter << ": chi2 is reset to 0! (before: " << chi2 << ")"); chi2 = 0; }

    // Calculating a probaility from the Chi2 value which considers the degrees of freedom
    // TODO: check degrees of freedom! Look at comment remaining from Jakobs circle fit!
    double probability = TMath::Prob(chi2, 2 * nHits - 5);

    B2WARNING("QualityEstimatorVXDTripletFitModule: event " << m_eventCounter
              << ": TC " << nTC
              << " with " << nHits
              << " hits has pT: " << result.second.Perp()
              << ", chi^2: " << chi2
              << " and probability: " << probability);

    std::pair<double, TVector3> CompResult = fitter.circleFit();
    double CompChi2 = CompResult.first;
    double CompProbability = TMath::Prob(CompChi2, nHits - 3);

    B2WARNING("QualityEstimatorVXDTripletFitModule: event " << m_eventCounter
              << " TripletChi2: " << chi2
              << " CircleChi2: " << CompChi2
              << " Triplet/Circle: " << chi2 / CompChi2
              << " TripletProb: " << probability
              << " CircleProb: " << CompProbability
              << " Triplet/Circle: " << probability / CompProbability);

    aTC.setQualityIndex(probability);
    ++nTC;
  }
}



void QualityEstimatorVXDTripletFitModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("QualityEstimatorVXDTripletFitModule:endRun: events: " << m_eventCounter
         << ", nSPTCsPerEvent: " << invEvents * float(m_nTCsTotal)
        );
}



void QualityEstimatorVXDTripletFitModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_nTCsTotal = 0;
}
