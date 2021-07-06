/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclFinalize/ECLFinalizerModule.h>

// FRAMEWORK
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventT0.h>

//ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/utility/utilityFunctions.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

// NAMESPACES
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLFinalizer)
REG_MODULE(ECLFinalizerPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLFinalizerModule::ECLFinalizerModule() : Module()
{
  // Set description
  setDescription("ECLFinalizerModule: Converts ecl shower dataobjects to mdst eclcluster dataobjects");

  addParam("clusterEnergyCutMin", m_clusterEnergyCutMin, "Min value for the cluster energy cut.",
           20.0 * Belle2::Unit::MeV);
  addParam("clusterTimeCutMaxEnergy", m_clusterTimeCutMaxEnergy, "All clusters above this energy are kept.",
           50.0 * Belle2::Unit::MeV);
  addParam("clusterLossyFraction", m_clusterLossyFraction,
           "Keep neutral hadron hypothesis clusters only if the number of crystals is different from that of the n photons hypothesis cluster by this fraction: abs(n(neutral hadron)-n(photon))/n(photon)",
           1e-4);

  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLFinalizerModule::~ECLFinalizerModule()
{
}

void ECLFinalizerModule::initialize()
{
  // Register in datastore.
  m_eclShowers.isRequired(eclShowerArrayName());
  m_eclClusters.registerInDataStore(eclClusterArrayName());
  m_eclCalDigits.isRequired(eclCalDigitArrayName());
  m_eventLevelClusteringInfo.registerInDataStore();
  m_eventT0.isRequired();

  // Register relations.
  m_eclClusters.registerRelationTo(m_eclShowers);
  m_eclClusters.registerRelationTo(m_eclCalDigits);

}

void ECLFinalizerModule::beginRun()
{
  // Do not use this blindly for database updates, they will probably not follow the concept of a "run"
  ;
}

void ECLFinalizerModule::event()
{
  //EventLevelClusteringInfo counters
  uint rejectedShowersFwd = 0;
  uint rejectedShowersBrl = 0;
  uint rejectedShowersBwd = 0;

  // event T0
  double eventT0 = 0.;
  if (m_eventT0->hasEventT0()) {
    eventT0 = m_eventT0->getEventT0();
  }

  // map connected regions to different cluster hypothesis: key (CRId, HypothesisID) -> list arrayindex of shower
  std::map<std::pair<int, int>, std::vector<int>> hypoShowerMap;
  for (const auto& eclShower : m_eclShowers) {
    // if they pass cuts, put them in this map
    // get shower time, energy and highest energy for cuts
    const double showerTime = eclShower.getTime() - eventT0;
    const double showerdt99 = eclShower.getDeltaTime99();
    const double showerEnergy = eclShower.getEnergy();

    if (showerEnergy > m_clusterEnergyCutMin and ((fabs(showerTime) < showerdt99) or (showerEnergy > m_clusterTimeCutMaxEnergy))) {
      hypoShowerMap[std::make_pair(eclShower.getConnectedRegionId(), eclShower.getHypothesisId())].push_back(eclShower.getArrayIndex());
    } else {
      if (eclShower.getHypothesisId() == Belle2::ECLShower::c_nPhotons) {

        // Get detector region
        const auto detectorRegion = eclShower.getDetectorRegion();

        B2DEBUG(39, "ECLFinalizerModule::event: Rejected shower with energy " << showerEnergy << ", time = " << showerTime << ", theta = "
                << eclShower.getTheta()
                << ", region " << detectorRegion);
        // Increment counters
        if (detectorRegion == static_cast<int>(ECL::DetectorRegion::FWD)) {
          ++rejectedShowersFwd;
        } else if (detectorRegion == ECL::DetectorRegion::BRL) {
          ++rejectedShowersBrl;
        } else if (detectorRegion == ECL::DetectorRegion::BWD) {
          ++rejectedShowersBwd;
        }
      }
    }
  }

  // map connected regions to different cluster hypothesis: key (CRId, HypothesisID) -> list arrayindex of cluster
  std::map<std::pair<int, int>, std::vector<int>> hypoClusterMap;

  // now loop over all photon showers from the map and make clusters for those and put them in the map
  for (const auto & [keypair, indexlist] : hypoShowerMap) {
    if (keypair.second == Belle2::ECLShower::c_nPhotons) {
      for (const auto& index : indexlist) {
        hypoClusterMap[keypair].push_back(makeCluster(index, eventT0));
      }
    }
  }

  // now loop over all other showers
  for (const auto & [keypair, indexlist] : hypoShowerMap) {
    if (keypair.second != Belle2::ECLShower::c_nPhotons) {
      for (const auto& index : indexlist) {
        // no photon entry exists (maybe we did not run the splitter or selection cuts failed)
        if (hypoShowerMap.count(std::make_pair(keypair.first, Belle2::ECLShower::c_nPhotons)) < 1) {
          hypoClusterMap[keypair].push_back(makeCluster(index, eventT0));
        }
        // there is more than one photon cluster, i.e. CR is split into multiple clusters
        else if (hypoShowerMap[std::make_pair(keypair.first, Belle2::ECLShower::c_nPhotons)].size() > 1) {
          hypoClusterMap[keypair].push_back(makeCluster(index, eventT0));
        } else {
          // get the already existing nPhotons cluster index
          const int indexOfCorrespondingNPhotonsCluster = hypoClusterMap[std::make_pair(keypair.first, Belle2::ECLShower::c_nPhotons)][0];
          double lossfraction = fabs(m_eclClusters[indexOfCorrespondingNPhotonsCluster]->getNumberOfCrystals() -
                                     m_eclShowers[index]->getNumberOfCrystals()) / m_eclClusters[indexOfCorrespondingNPhotonsCluster]->getNumberOfCrystals();

          B2DEBUG(35, "ECLFinalizerModule::event n(photon shower) = " <<
                  m_eclClusters[indexOfCorrespondingNPhotonsCluster]->getNumberOfCrystals() << ", n(hadron shower): " <<
                  m_eclShowers[index]->getNumberOfCrystals());
          B2DEBUG(35, "ECLFinalizerModule::event lossfraction = " << lossfraction);

          if (lossfraction > m_clusterLossyFraction) {
            hypoClusterMap[keypair].push_back(makeCluster(index, eventT0));
          } else {
            if (keypair.second == ECLShower::c_neutralHadron) m_eclClusters[indexOfCorrespondingNPhotonsCluster]->addHypothesis(
                ECLCluster::EHypothesisBit::c_neutralHadron);
            else {
              B2ERROR("This ECLShower hypothesis is not supported yet: " << keypair.second);
            }
          }
        }
      }
    }
  }

  // Save EventLevelClusteringInfo
  if (!m_eventLevelClusteringInfo) {
    m_eventLevelClusteringInfo.create();
  }
  m_eventLevelClusteringInfo->setNECLShowersRejectedFWD(rejectedShowersFwd);
  m_eventLevelClusteringInfo->setNECLShowersRejectedBarrel(rejectedShowersBrl);
  m_eventLevelClusteringInfo->setNECLShowersRejectedBWD(rejectedShowersBwd);

  B2DEBUG(35, "ECLFinalizerModule::event found " << rejectedShowersFwd << ", " << rejectedShowersBrl << ", " << rejectedShowersBwd
          << " rejected showers in FWD, BRL, BWD");
}

void ECLFinalizerModule::endRun() { ; }

void ECLFinalizerModule::terminate() { ; }


int ECLFinalizerModule::makeCluster(int index, double evtt0)
{

  const auto eclShower = m_eclShowers[index];

  // create an mdst cluster for each ecl shower
  const auto eclCluster = m_eclClusters.appendNew();

  // status between showers and clusters may be different:
  if (eclShower->hasPulseShapeDiscrimination()) {
    eclCluster->addStatus(ECLCluster::EStatusBit::c_PulseShapeDiscrimination);
  }

  eclCluster->setConnectedRegionId(eclShower->getConnectedRegionId());

  // ECLShowers have *one* hypothesisID...
  const int hyp = eclShower->getHypothesisId();

  // ECLClusters can have *multiple*, but not at the creation of an ECLCluster: use "set" and not "add"
  if (hyp == ECLShower::c_nPhotons) {
    eclCluster->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
  } else if (hyp == ECLShower::c_neutralHadron) {
    eclCluster->setHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
  } else {
    B2ERROR("This ECLShower hypothesis is not supported yet: " << hyp);
  }

  eclCluster->setClusterId(eclShower->getShowerId());

  eclCluster->setEnergy(eclShower->getEnergy());
  eclCluster->setEnergyRaw(eclShower->getEnergyRaw());
  eclCluster->setEnergyHighestCrystal(eclShower->getEnergyHighestCrystal());
  eclCluster->setMaxECellId(static_cast<unsigned short>(eclShower->getCentralCellId()));

  double covmat[6] = {
    eclShower->getUncertaintyEnergy()* eclShower->getUncertaintyEnergy(),
    0.0,
    eclShower->getUncertaintyPhi()* eclShower->getUncertaintyPhi(),
    0.0,
    0.0,
    eclShower->getUncertaintyTheta()* eclShower->getUncertaintyTheta()
  };
  eclCluster->setCovarianceMatrix(covmat);

  eclCluster->setAbsZernike40(eclShower->getAbsZernike40());
  eclCluster->setAbsZernike51(eclShower->getAbsZernike51());
  eclCluster->setZernikeMVA(eclShower->getZernikeMVA());
  eclCluster->setE1oE9(eclShower->getE1oE9());
  eclCluster->setE9oE21(eclShower->getE9oE21());
  eclCluster->setSecondMoment(eclShower->getSecondMoment());
  eclCluster->setLAT(eclShower->getLateralEnergy());
  eclCluster->setNumberOfCrystals(eclShower->getNumberOfCrystals());
  eclCluster->setTime(eclShower->getTime() - evtt0);   // If bad timing fit, this value is changed later in the code
  eclCluster->setDeltaTime99(eclShower->getDeltaTime99());
  eclCluster->setTheta(eclShower->getTheta());
  eclCluster->setPhi(eclShower->getPhi());
  eclCluster->setR(eclShower->getR());
  eclCluster->setPulseShapeDiscriminationMVA(eclShower->getPulseShapeDiscriminationMVA());
  eclCluster->setNumberOfHadronDigits(eclShower->getNumberOfHadronDigits());

  // set relation to ECLShower
  eclCluster->addRelationTo(eclShower);

  // set relation to ECLCalDigits and set failed timing flags
  auto cellIDOfMaxEnergyCrystal = eclShower->getCentralCellId() ;
  auto showerDigitRelations = eclShower->getRelationsTo<ECLCalDigit>(eclCalDigitArrayName());
  for (unsigned int iRel = 0; iRel < showerDigitRelations.size(); ++iRel) {
    const auto calDigit = showerDigitRelations.object(iRel);
    const auto weight = showerDigitRelations.weight(iRel);

    eclCluster->addRelationTo(calDigit, weight);


    // Set the failed timing flag for the crystal that defines the cluster time
    if (calDigit->getCellId() == cellIDOfMaxEnergyCrystal) {
      if (calDigit->isFailedFit()) {
        eclCluster->addStatus(ECLCluster::EStatusBit::c_fitTimeFailed);
        eclCluster->setTime(eclShower->getTime());
      }
      if (calDigit->isTimeResolutionFailed()) {
        eclCluster->addStatus(ECLCluster::EStatusBit::c_timeResolutionFailed);
      }
    }
  }

  return eclCluster->getArrayIndex();

}
