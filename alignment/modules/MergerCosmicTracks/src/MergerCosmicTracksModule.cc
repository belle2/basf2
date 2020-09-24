/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:    Dong Van Thanh, Jakub Kandra                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MergerCosmicTracks/MergerCosmicTracksModule.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>
#include <framework/datastore/StoreArray.h>
#include <framework/geometry/B2Vector3.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;

REG_MODULE(MergerCosmicTracks);

MergerCosmicTracksModule::MergerCosmicTracksModule() : Module()
{
  setDescription("Select cosmic events containing two tracks (up/down) and merge two tracks");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName,
           "StoreArray containing the RecoTracks to read from and delete afterwards.",
           m_param_recoTracksStoreArrayName);
  addParam("mergedRecoTracksStoreArrayName", m_param_mergedRecoTracksStoreArrayName,
           "StoreArray to where to copy the merged RecoTrack.",
           m_param_mergedRecoTracksStoreArrayName);
  addParam("usingMagneticField", m_usingMagneticField,
           "Flag to using magnetic field during reconstruction.",
           m_usingMagneticField);
  addParam("minimumNumHitCut", m_minimumNumHitCut, "Number of CDC hit per track required for cosmic track", m_minimumNumHitCut);
  addParam("minimumClusterSize", m_minimumClusterSize, "Minimal cluster size for used PXD clusters for cosmic track",
           m_minimumClusterSize);
}

void MergerCosmicTracksModule::initialize()
{
  m_Tracks.isRequired();
  m_RecoTracks.isRequired(m_param_recoTracksStoreArrayName);
  m_MergedRecoTracks.registerInDataStore(m_param_mergedRecoTracksStoreArrayName);
  m_ECLConnectedRegions.isRequired();
  m_KLMClusters.isRequired();

  RecoTrack::registerRequiredRelations(m_MergedRecoTracks);

  Belle2::VXD::GeoCache::getInstance();
  Belle2::CDC::CDCGeometryPar::Instance();
}

void MergerCosmicTracksModule::MergingTracks(RecoTrack* firstRecoTrack, RecoTrack* secondRecoTrack,
                                             StoreArray<RecoTrack>& mergedRecoTracks)
{

  // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
  float fittedValuesOfFunctions[3] = {50.2380, 21.9203, 19.8463}; // {p0, p1, p2}

  // Definition of upper and lower part of CosmicRecoTrack
  RecoTrack* upperTrack;
  RecoTrack* lowerTrack;

  // The RecoTrack with smaller timeSeed is upper
  if (firstRecoTrack->getTimeSeed() < secondRecoTrack->getTimeSeed()) {
    upperTrack = firstRecoTrack;
    lowerTrack = secondRecoTrack;
  } else {
    upperTrack = secondRecoTrack;
    lowerTrack = firstRecoTrack;
  }

  // estimation of momentum tracks
  int numberOfCDCHits = upperTrack->getNumberOfCDCHits() + lowerTrack->getNumberOfCDCHits();
  float totalADCCount = 0;
  if (upperTrack->hasCDCHits()) {
    int CDCHits = upperTrack->getNumberOfCDCHits();
    for (int i = 0; i < CDCHits; i++) {
      totalADCCount = totalADCCount + upperTrack->getSortedCDCHitList()[i]->getADCCount();
    }
  }
  if (lowerTrack->hasCDCHits()) {
    int CDCHits = lowerTrack->getNumberOfCDCHits();
    for (int i = 0; i < CDCHits; i++) {
      totalADCCount = totalADCCount + lowerTrack->getSortedCDCHitList()[i]->getADCCount();
    }
  }
  if (totalADCCount / numberOfCDCHits < 115) {
    // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
    m_magnitudeOfMomentumWithoutMagneticField = exp((-fittedValuesOfFunctions[0] + totalADCCount / numberOfCDCHits) /
                                                    fittedValuesOfFunctions[1]);
  } else {
    m_magnitudeOfMomentumWithoutMagneticField = fittedValuesOfFunctions[2];
  }

  // Creation of CosmicRecoTrack
  RecoTrack* mergedRecoTrack;
  if (m_usingMagneticField == true) {
    mergedRecoTrack = mergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                                                 upperTrack->getMomentumSeed(),
                                                 upperTrack->getChargeSeed());
  } else {
    B2Vector3D momentum = upperTrack->getMomentumSeed();
    float magnitudeMomentum = momentum.Mag();
    float newMomentumX = (momentum.Px() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
    float newMomentumY = (momentum.Py() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
    float newMomentumZ = (momentum.Pz() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
    B2Vector3D newMomentum(newMomentumX, newMomentumY, newMomentumZ);
    // std::cout << "No Magnetic Field: " << newMomentum.Mag() << "\n";
    mergedRecoTrack = mergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                                                 newMomentum,
                                                 upperTrack->getChargeSeed());
  }
  mergedRecoTrack->setTimeSeed(upperTrack->getTimeSeed());

  int sortingNumber = 0;

  if (upperTrack->hasPXDHits()) {
    int PXDHits = upperTrack->getNumberOfPXDHits();
    for (int i = 0; i < PXDHits; i++) {
      if (upperTrack->getSortedPXDHitList()[i]->getSize() > m_minimumClusterSize) {mergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);}
      // B2INFO("Cluster size of included PXD hit: " << upperTrack->getSortedPXDHitList()[i]->getSize());
      sortingNumber++;
    }

  }

  if (lowerTrack->hasPXDHits()) {
    int PXDHits = lowerTrack->getNumberOfPXDHits();
    for (int i = 0; i < PXDHits; i++) {
      if (lowerTrack->getSortedPXDHitList()[i]->getSize() > m_minimumClusterSize) {mergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);}
      // B2INFO("Cluster size of included PXD hit: " << lowerTrack->getSortedPXDHitList()[i]->getSize());
      sortingNumber++;
    }
  }

  if (upperTrack->hasSVDHits()) {
    int SVDHits = upperTrack->getNumberOfSVDHits();
    for (int i = 0; i < SVDHits; i++) {
      mergedRecoTrack->addSVDHit(upperTrack->getSortedSVDHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }

  if (lowerTrack->hasSVDHits()) {
    int SVDHits = lowerTrack->getNumberOfSVDHits();
    for (int i = 0; i < SVDHits; i++) {
      mergedRecoTrack->addSVDHit(lowerTrack->getSortedSVDHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }


  if (upperTrack->hasCDCHits()) {
    int CDCHits = upperTrack->getNumberOfCDCHits();
    for (int i = 0; i < CDCHits; i++) {
      mergedRecoTrack->addCDCHit(upperTrack->getSortedCDCHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }

  if (lowerTrack->hasCDCHits()) {
    int CDCHits = lowerTrack->getNumberOfCDCHits();
    for (int i = CDCHits - 1; i >= 0; i--) {
      mergedRecoTrack->addCDCHit(lowerTrack->getSortedCDCHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }

  if (upperTrack->hasBKLMHits()) {
    int BKLMHits = upperTrack->getNumberOfBKLMHits();
    for (int i = 0; i < BKLMHits; i++) {
      mergedRecoTrack->addBKLMHit(upperTrack->getSortedBKLMHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }

  if (upperTrack->hasEKLMHits()) {
    int EKLMHits = upperTrack->getNumberOfEKLMHits();
    for (int i = 0; i < EKLMHits; i++) {
      mergedRecoTrack->addEKLMHit(upperTrack->getSortedEKLMHitList()[i], sortingNumber);
    }
  }

  if (lowerTrack->hasBKLMHits()) {
    int BKLMHits = lowerTrack->getNumberOfBKLMHits();
    for (int i = 0; i < BKLMHits; i++) {
      mergedRecoTrack->addBKLMHit(lowerTrack->getSortedBKLMHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }

  if (lowerTrack->hasEKLMHits()) {
    int EKLMHits = lowerTrack->getNumberOfEKLMHits();
    for (int i = 0; i < EKLMHits; i++) {
      mergedRecoTrack->addEKLMHit(lowerTrack->getSortedEKLMHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }
}

void MergerCosmicTracksModule::event()
{
  // VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  // CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

  // numberRecoTracks | numberKLMClusters | numberRecoTracksWithB(E)KLMHits => CosmicRecoTracks
  //        1                   1                         1                       Create
  //        1                   2                         1                       Create
  //        2                   1                         1                       Create
  //        2                   2                         2                       Create
  //        2              More than 2                    2                       Create
  //   More than 2              2                         2                       Create


  // Estimation of momentum from dE/dx, when magnetic field is off
  // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
  float fittedValuesOfFunctions[3] = {50.2380, 21.9203, 19.8463}; // {p0, p1, p2}

  // bool mergedTracks = false;

  if (m_RecoTracks.getEntries() == 1) {
    int numberKLMClusterEnoughLayers = 0;
    for (int i = 0; i < m_KLMClusters.getEntries(); i++) {
      if (m_KLMClusters[i]->getLayers() > 4) {
        numberKLMClusterEnoughLayers++;
      }
    }

    if (m_ECLConnectedRegions.getEntries() == 2 || m_ECLConnectedRegions.getEntries() == 1
        || numberKLMClusterEnoughLayers == 2 || numberKLMClusterEnoughLayers == 1) {
      if (m_RecoTracks[0]->getNumberOfCDCHits() > m_minimumNumHitCut) {
        RelationVector<Track> track = m_RecoTracks[0]->getRelationsWith<Track>();
        if (track.size() != 0) {
          RelationVector<ECLShower> eclShower = track[0]->getRelationsTo<ECLShower>();
          if (eclShower.size() != 0 || m_RecoTracks[0]->hasEKLMHits() || m_RecoTracks[0]->hasBKLMHits()) {

            int numberOfCDCHits = m_RecoTracks[0]->getNumberOfCDCHits();
            float totalADCCount = 0;
            if (m_RecoTracks[0]->hasCDCHits()) {
              int CDCHits = m_RecoTracks[0]->getNumberOfCDCHits();
              for (int i = 0; i < CDCHits; i++) {
                totalADCCount = totalADCCount + m_RecoTracks[0]->getSortedCDCHitList()[i]->getADCCount();
              }
            }
            if (totalADCCount / numberOfCDCHits < 115) {
              // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
              m_magnitudeOfMomentumWithoutMagneticField = exp((-fittedValuesOfFunctions[0] + totalADCCount / numberOfCDCHits) /
                                                              fittedValuesOfFunctions[1]);
            } else {
              m_magnitudeOfMomentumWithoutMagneticField = fittedValuesOfFunctions[2];
            }
            RecoTrack* mergedRecoTrack;
            if (m_usingMagneticField == true) {
              mergedRecoTrack = m_MergedRecoTracks.appendNew(m_RecoTracks[0]->getPositionSeed(),
                                                             m_RecoTracks[0]->getMomentumSeed(), m_RecoTracks[0]->getChargeSeed());
            } else {
              B2Vector3D momentum = m_RecoTracks[0]->getMomentumSeed();
              float magnitudeMomentum = momentum.Mag();
              float newMomentumX = (momentum.Px() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
              float newMomentumY = (momentum.Py() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
              float newMomentumZ = (momentum.Pz() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
              B2Vector3D newMomentum(newMomentumX, newMomentumY, newMomentumZ);
              mergedRecoTrack = m_MergedRecoTracks.appendNew(m_RecoTracks[0]->getPositionSeed(),
                                                             newMomentum, m_RecoTracks[0]->getChargeSeed());
            }
            mergedRecoTrack->setTimeSeed(m_RecoTracks[0]->getTimeSeed());

            int sortingNumber = 0;
            if (m_RecoTracks[0]->hasPXDHits()) {
              int PXDHits = m_RecoTracks[0]->getNumberOfPXDHits();
              for (int i = 0; i < PXDHits; i++) {
                if (m_RecoTracks[0]->getSortedPXDHitList()[i]->getSize() > 1) {mergedRecoTrack->addPXDHit(m_RecoTracks[0]->getSortedPXDHitList()[i], sortingNumber);}
                // B2INFO("Cluster size of included PXD hit: " << m_RecoTracks[0]->getSortedPXDHitList()[i]->getSize());
                sortingNumber++;
              }
            }
            if (m_RecoTracks[0]->hasSVDHits()) {
              int SVDHits = m_RecoTracks[0]->getNumberOfSVDHits();
              for (int i = 0; i < SVDHits; i++) {
                mergedRecoTrack->addSVDHit(m_RecoTracks[0]->getSortedSVDHitList()[i], sortingNumber);
                sortingNumber++;
              }
            }
            if (m_RecoTracks[0]->hasCDCHits()) {
              int CDCHits = m_RecoTracks[0]->getNumberOfCDCHits();
              for (int i = 0; i < CDCHits; i++) {
                mergedRecoTrack->addCDCHit(m_RecoTracks[0]->getSortedCDCHitList()[i], sortingNumber);
                sortingNumber++;
              }
            }
            if (m_RecoTracks[0]->hasBKLMHits()) {
              int BKLMHits = m_RecoTracks[0]->getNumberOfBKLMHits();
              for (int i = 0; i < BKLMHits; i++) {
                mergedRecoTrack->addBKLMHit(m_RecoTracks[0]->getSortedBKLMHitList()[i], sortingNumber);
                sortingNumber++;
              }
            }
            if (m_RecoTracks[0]->hasEKLMHits()) {
              int EKLMHits = m_RecoTracks[0]->getNumberOfEKLMHits();
              for (int i = 0; i < EKLMHits; i++) {
                mergedRecoTrack->addEKLMHit(m_RecoTracks[0]->getSortedEKLMHitList()[i], sortingNumber);
              }
            }
            // mergedTracks = true;
          }
        }
      }
    }
  }

  else if (m_RecoTracks.getEntries() == 2) {
    if (m_RecoTracks[0]->getNumberOfCDCHits() + m_RecoTracks[1]->getNumberOfCDCHits() > m_minimumNumHitCut) {
      RelationVector<Track> trackFirst = m_RecoTracks[0]->getRelationsWith<Track>();
      RelationVector<Track> trackSecond = m_RecoTracks[1]->getRelationsWith<Track>();
      if (trackFirst.size() != 0 && trackSecond.size() != 0) {
        RelationVector<ECLShower> eclShowerFirst = trackFirst[0]->getRelationsTo<ECLShower>();
        RelationVector<ECLShower> eclShowerSecond = trackSecond[0]->getRelationsTo<ECLShower>();
        if ((eclShowerFirst.size() != 0 && eclShowerSecond.size() != 0) || ((m_RecoTracks[0]->hasEKLMHits()
            || m_RecoTracks[0]->hasBKLMHits()) && (m_RecoTracks[1]->hasEKLMHits() || m_RecoTracks[1]->hasBKLMHits()))) {
          if (m_ECLConnectedRegions.getEntries() >= 2 || m_KLMClusters.getEntries() >= 2) {
            MergingTracks(m_RecoTracks[0], m_RecoTracks[1], m_MergedRecoTracks);
            // mergedTracks = true;
          }
        } else if (eclShowerFirst.size() != 0 || eclShowerSecond.size() != 0 || m_RecoTracks[0]->hasEKLMHits()
                   || m_RecoTracks[0]->hasBKLMHits() || m_RecoTracks[1]->hasEKLMHits() || m_RecoTracks[1]->hasBKLMHits()) {
          if (m_ECLConnectedRegions.getEntries() == 2 || m_ECLConnectedRegions.getEntries() == 1
              || m_KLMClusters.getEntries() == 2 || m_KLMClusters.getEntries() == 1) {
            MergingTracks(m_RecoTracks[0], m_RecoTracks[1], m_MergedRecoTracks);
            // mergedTracks = true;
          }
        }
      }
    }
  }

  else {
    for (int i = 0; i < m_RecoTracks.getEntries(); i++) {
      RelationVector<Track> trackFirst = m_RecoTracks[i]->getRelationsWith<Track>();
      if (trackFirst.size() != 0) {
        for (int j = i + 1; j < m_RecoTracks.getEntries(); j++) {
          RelationVector<Track> trackSecond = m_RecoTracks[j]->getRelationsWith<Track>();
          if (trackSecond.size() != 0) {
            if (m_RecoTracks[i]->getNumberOfCDCHits() + m_RecoTracks[j]->getNumberOfCDCHits() > m_minimumNumHitCut) {
              RelationVector<ECLShower> eclShowerFirst = trackFirst[0]->getRelationsTo<ECLShower>();
              RelationVector<ECLShower> eclShowerSecond = trackSecond[0]->getRelationsTo<ECLShower>();
              if (eclShowerFirst.size() != 0 && eclShowerSecond.size() != 0) {
                if (eclShowerFirst[0]->getRelationsWith<Track>().size() == 1 && eclShowerSecond[0]->getRelationsWith<Track>().size() == 1) {
                  MergingTracks(m_RecoTracks[i], m_RecoTracks[j], m_MergedRecoTracks);
                  // mergedTracks = true;
                }
              } else if ((m_RecoTracks[i]->hasEKLMHits() || m_RecoTracks[i]->hasBKLMHits())
                         && (m_RecoTracks[j]->hasEKLMHits() || m_RecoTracks[j]->hasBKLMHits())) {
                if (m_RecoTracks[i]->getNumberOfBKLMHits() + m_RecoTracks[i]->getNumberOfEKLMHits() > 3
                    && m_RecoTracks[j]->getNumberOfBKLMHits() + m_RecoTracks[j]->getNumberOfEKLMHits() > 3) {
                  MergingTracks(m_RecoTracks[i], m_RecoTracks[j], m_MergedRecoTracks);
                  // mergedTracks = true;
                }
              }
            }
          }
        }
      }
    }
  }
}
