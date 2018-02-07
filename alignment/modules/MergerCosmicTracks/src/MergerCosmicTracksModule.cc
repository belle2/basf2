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
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <framework/datastore/StoreArray.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace CDC;

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
}

void MergerCosmicTracksModule::initialize()
{
  StoreArray<KLMCluster> klmClusters("KLMClusters");
  klmClusters.isRequired();

  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<RecoTrack> mergedRecoTracks(m_param_mergedRecoTracksStoreArrayName);
  mergedRecoTracks.registerInDataStore();

  RecoTrack::registerRequiredRelations(mergedRecoTracks);

  Belle2::VXD::GeoCache::getInstance();
  Belle2::CDC::CDCGeometryPar::Instance();
}

void MergerCosmicTracksModule::MergingTracks(StoreArray<RecoTrack>recoTrackStoreArray, StoreArray<RecoTrack>mergedRecoTracks)
{

  // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
  float fittedValuesOfFunctions[3] = {50.2380, 21.9203, 19.8463}; // {p0, p1, p2}

  // Definition of upper and lower part of CosmicRecoTrack
  RecoTrack* upperTrack;
  RecoTrack* lowerTrack;

  // The RecoTrack with smaller timeSeed is upper
  if (recoTrackStoreArray[0]->getTimeSeed() < recoTrackStoreArray[1]->getTimeSeed()) {
    upperTrack = recoTrackStoreArray[0];
    lowerTrack = recoTrackStoreArray[1];
  } else {
    upperTrack = recoTrackStoreArray[1];
    lowerTrack = recoTrackStoreArray[0];
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
    TVector3 momentum = upperTrack->getMomentumSeed();
    float magnitudeMomentum = momentum.Mag();
    float newMomentumX = (momentum.Px() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
    float newMomentumY = (momentum.Py() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
    float newMomentumZ = (momentum.Pz() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
    TVector3 newMomentum(newMomentumX, newMomentumY, newMomentumZ);
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
      mergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
      sortingNumber++;
    }
  }

  if (lowerTrack->hasPXDHits()) {
    int PXDHits = lowerTrack->getNumberOfPXDHits();
    for (int i = 0; i < PXDHits; i++) {
      mergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);
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

  StoreArray<RecoTrack> recoTrackStoreArray(m_param_recoTracksStoreArrayName);
  StoreArray<RecoTrack> mergedRecoTracks(m_param_mergedRecoTracksStoreArrayName);
  StoreArray<KLMCluster> klmClustersStoreArray("KLMClusters");

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

  // One RecoTrack in an event
  if (recoTrackStoreArray.getEntries() == 1) {

    // No more than 2 KLMClusters

    int numberKLMClustersEnoughLayers = 0;

    for (int i = 0; i < klmClustersStoreArray.getEntries(); i++) {
      if (klmClustersStoreArray[i]->getLayers() > 4) {
        numberKLMClustersEnoughLayers++;
      }
    }

    if (numberKLMClustersEnoughLayers == 2 || klmClustersStoreArray.getEntries() == 2) {

      // A candidate of CosmicRecoTrack has at least m_minimumNumHitCut (40) CDCHits
      if (recoTrackStoreArray[0]->getNumberOfCDCHits() > m_minimumNumHitCut) {

        // A candidate of CosmicRecoTrack has B(E)KLM hits
        if (recoTrackStoreArray[0]->hasEKLMHits() || recoTrackStoreArray[0]->hasBKLMHits()) {

          // estimation of momentum tracks
          int numberOfCDCHits = recoTrackStoreArray[0]->getNumberOfCDCHits();
          float totalADCCount = 0;
          if (recoTrackStoreArray[0]->hasCDCHits()) {
            int CDCHits = recoTrackStoreArray[0]->getNumberOfCDCHits();
            for (int i = 0; i < CDCHits; i++) {
              totalADCCount = totalADCCount + recoTrackStoreArray[0]->getSortedCDCHitList()[i]->getADCCount();
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
            mergedRecoTrack = mergedRecoTracks.appendNew(recoTrackStoreArray[0]->getPositionSeed(),
                                                         recoTrackStoreArray[0]->getMomentumSeed(), recoTrackStoreArray[0]->getChargeSeed());
          }

          else {
            TVector3 momentum = recoTrackStoreArray[0]->getMomentumSeed();
            float magnitudeMomentum = momentum.Mag();
            float newMomentumX = (momentum.Px() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
            float newMomentumY = (momentum.Py() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
            float newMomentumZ = (momentum.Pz() * m_magnitudeOfMomentumWithoutMagneticField) / magnitudeMomentum;
            TVector3 newMomentum(newMomentumX, newMomentumY, newMomentumZ);
            std::cout << "No Magnetic Field: " << newMomentum.Mag() << "\n";
            mergedRecoTrack = mergedRecoTracks.appendNew(recoTrackStoreArray[0]->getPositionSeed(),
                                                         newMomentum, recoTrackStoreArray[0]->getChargeSeed());
          }

          // retain the seed time of the original track. Important for t0 extraction.
          mergedRecoTrack->setTimeSeed(recoTrackStoreArray[0]->getTimeSeed());

          int sortingNumber = 0;

          if (recoTrackStoreArray[0]->hasPXDHits()) {
            int PXDHits = recoTrackStoreArray[0]->getNumberOfPXDHits();
            for (int i = 0; i < PXDHits; i++) {
              mergedRecoTrack->addPXDHit(recoTrackStoreArray[0]->getSortedPXDHitList()[i], sortingNumber);
              sortingNumber++;
            }
          }

          if (recoTrackStoreArray[0]->hasSVDHits()) {
            int SVDHits = recoTrackStoreArray[0]->getNumberOfSVDHits();
            for (int i = 0; i < SVDHits; i++) {
              mergedRecoTrack->addSVDHit(recoTrackStoreArray[0]->getSortedSVDHitList()[i], sortingNumber);
              sortingNumber++;
            }
          }

          if (recoTrackStoreArray[0]->hasCDCHits()) {
            int CDCHits = recoTrackStoreArray[0]->getNumberOfCDCHits();
            for (int i = 0; i < CDCHits; i++) {
              mergedRecoTrack->addCDCHit(recoTrackStoreArray[0]->getSortedCDCHitList()[i], sortingNumber);
              sortingNumber++;
            }
          }

          if (recoTrackStoreArray[0]->hasBKLMHits()) {
            int BKLMHits = recoTrackStoreArray[0]->getNumberOfBKLMHits();
            for (int i = 0; i < BKLMHits; i++) {
              mergedRecoTrack->addBKLMHit(recoTrackStoreArray[0]->getSortedBKLMHitList()[i], sortingNumber);
              sortingNumber++;
            }
          }

          if (recoTrackStoreArray[0]->hasEKLMHits()) {
            int EKLMHits = recoTrackStoreArray[0]->getNumberOfEKLMHits();
            for (int i = 0; i < EKLMHits; i++) {
              mergedRecoTrack->addEKLMHit(recoTrackStoreArray[0]->getSortedEKLMHitList()[i], sortingNumber);
            }
          }
        }
      }
    }
  }

  // Two RecoTracks in an event
  else if (recoTrackStoreArray.getEntries() == 2) {

    // A candidate of CosmicRecoTrack has at least m_minimumNumHitCut (40) CDCHits
    if (recoTrackStoreArray[0]->getNumberOfCDCHits() + recoTrackStoreArray[1]->getNumberOfCDCHits() > m_minimumNumHitCut) {

      // One KLMCluster reconstructed in event
      if (klmClustersStoreArray.getEntries() == 1) {

        // One from RecoTracks has B(E)KLM hits
        if (recoTrackStoreArray[0]->hasEKLMHits() || recoTrackStoreArray[0]->hasBKLMHits() || recoTrackStoreArray[1]->hasEKLMHits()
            || recoTrackStoreArray[1]->hasBKLMHits()) {

          // Merging cosmic tracks
          MergingTracks(recoTrackStoreArray, mergedRecoTracks);

          /*
                // Definition of upper and lower part of CosmicRecoTrack
                RecoTrack* upperTrack;
                RecoTrack* lowerTrack;

                if (recoTrackStoreArray[0]->getTimeSeed() > recoTrackStoreArray[1]->getTimeSeed()) {
                  upperTrack = recoTrackStoreArray[0];
                  lowerTrack = recoTrackStoreArray[1];
                } else {
                  upperTrack = recoTrackStoreArray[1];
                  lowerTrack = recoTrackStoreArray[0];
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

          if (totalADCCount/numberOfCDCHits < 115) {
            // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
            m_magnitudeOfMomentumWithoutMagneticField = exp((-fittedValuesOfFunctions[0]+totalADCCount/numberOfCDCHits)/fittedValuesOfFunctions[1]);
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
            TVector3 momentum = upperTrack->getMomentumSeed();
            float magnitudeMomentum = momentum.Mag();
            float newMomentumX = (momentum.Px()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumY = (momentum.Py()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumZ = (momentum.Pz()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            TVector3 newMomentum(newMomentumX, newMomentumY, newMomentumZ);
            std::cout << "No Magnetic Field: " << newMomentum.Mag() << "\n";
            mergedRecoTrack = mergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                     newMomentum,
                     upperTrack->getChargeSeed());
          }
                mergedRecoTrack->setTimeSeed(upperTrack->getTimeSeed());

                int sortingNumber = 0;

                if (upperTrack->hasPXDHits()) {
                  int PXDHits = upperTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
                    sortingNumber++;
                  }
                }

                if (lowerTrack->hasPXDHits()) {
                  int PXDHits = lowerTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);
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
          */
        }
      }

      else if (klmClustersStoreArray.getEntries() == 2) {

        // One from RecoTracks has B(E)KLM hits
        if (recoTrackStoreArray[0]->hasEKLMHits() || recoTrackStoreArray[0]->hasBKLMHits() || recoTrackStoreArray[1]->hasEKLMHits()
            || recoTrackStoreArray[1]->hasBKLMHits()) {

          // Merging cosmic tracks
          MergingTracks(recoTrackStoreArray, mergedRecoTracks);

          /*
                // Definition of upper and lower part of CosmicRecoTrack
                RecoTrack* upperTrack;
                RecoTrack* lowerTrack;

                if (recoTrackStoreArray[0]->getTimeSeed() > recoTrackStoreArray[1]->getTimeSeed()) {
                  upperTrack = recoTrackStoreArray[0];
                  lowerTrack = recoTrackStoreArray[1];
                } else {
                  upperTrack = recoTrackStoreArray[1];
                  lowerTrack = recoTrackStoreArray[0];
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

          if (totalADCCount/numberOfCDCHits < 115) {
            // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
            m_magnitudeOfMomentumWithoutMagneticField = exp((-fittedValuesOfFunctions[0]+totalADCCount/numberOfCDCHits)/fittedValuesOfFunctions[1]);
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
            TVector3 momentum = upperTrack->getMomentumSeed();
            float magnitudeMomentum = momentum.Mag();
            float newMomentumX = (momentum.Px()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumY = (momentum.Py()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumZ = (momentum.Pz()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            TVector3 newMomentum(newMomentumX, newMomentumY, newMomentumZ);
            std::cout << "No Magnetic Field: " << newMomentum.Mag() << "\n";
            mergedRecoTrack = mergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                     newMomentum,
                     upperTrack->getChargeSeed());
          }

                mergedRecoTrack->setTimeSeed(upperTrack->getTimeSeed());

                int sortingNumber = 0;

                if (upperTrack->hasPXDHits()) {
                  int PXDHits = upperTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
                    sortingNumber++;
                  }
                }

                if (lowerTrack->hasPXDHits()) {
                  int PXDHits = lowerTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);
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
          */
        }
      } else {

        // Both RecoTracks have B(E)KLM hits
        if ((recoTrackStoreArray[0]->hasEKLMHits() || recoTrackStoreArray[0]->hasBKLMHits()) && (recoTrackStoreArray[1]->hasEKLMHits()
            || recoTrackStoreArray[1]->hasBKLMHits())) {

          // Merging cosmic tracks
          MergingTracks(recoTrackStoreArray, mergedRecoTracks);

          /*
                // Definition of upper and lower part of CosmicRecoTrack
                RecoTrack* upperTrack;
                RecoTrack* lowerTrack;

                if (recoTrackStoreArray[0]->getTimeSeed() > recoTrackStoreArray[1]->getTimeSeed()) {
                  upperTrack = recoTrackStoreArray[0];
                  lowerTrack = recoTrackStoreArray[1];
                } else {
                  upperTrack = recoTrackStoreArray[1];
                  lowerTrack = recoTrackStoreArray[0];
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

          if (totalADCCount/numberOfCDCHits < 115) {
            // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
            m_magnitudeOfMomentumWithoutMagneticField = exp((-fittedValuesOfFunctions[0]+totalADCCount/numberOfCDCHits)/fittedValuesOfFunctions[1]);
          } else {
            m_magnitudeOfMomentumWithoutMagneticField = fittedValuesOfFunctions[2];
          }

          // Creation of CosmicRecoTrack
          RecoTrack* mergedRecoTrack;
          if (m_usingMagneticField == true) {

            mergedRecoTrack = mergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                     upperTrack->getMomentumSeed(),
                     upperTrack->getChargeSeed());
          }  else {
            TVector3 momentum = upperTrack->getMomentumSeed();
            float magnitudeMomentum = momentum.Mag();
            float newMomentumX = (momentum.Px()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumY = (momentum.Py()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumZ = (momentum.Pz()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            TVector3 newMomentum(newMomentumX, newMomentumY, newMomentumZ);
            std::cout << "No Magnetic Field: " << newMomentum.Mag() << "\n";
            mergedRecoTrack = mergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                     newMomentum,
                     upperTrack->getChargeSeed());
          }
                mergedRecoTrack->setTimeSeed(upperTrack->getTimeSeed());

                int sortingNumber = 0;

                if (upperTrack->hasPXDHits()) {
                  int PXDHits = upperTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
                    sortingNumber++;
                  }
                }

                if (lowerTrack->hasPXDHits()) {
                  int PXDHits = lowerTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);
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
          */
        }
      }
    }
  } else {

    // Check how many KLMClusters have more than 4 layers and if two than continue

    int numberKLMClustersEnoughLayers = 0;

    for (int i = 0; i < klmClustersStoreArray.getEntries(); i++) {
      if (klmClustersStoreArray[i]->getLayers() > 4) {
        numberKLMClustersEnoughLayers++;
      }
    }

    if (numberKLMClustersEnoughLayers == 2 || klmClustersStoreArray.getEntries() == 2) {

      // Check how many tracks has B(E)KLM hits and store their index

      int indexRecoTracksWithKLMHits[recoTrackStoreArray.getEntries()];
      int numberRecoTracksWithKLMHits = 0;

      for (int i = 0; i < recoTrackStoreArray.getEntries(); i++) {
        if (recoTrackStoreArray[i]->hasEKLMHits() || recoTrackStoreArray[i]->hasBKLMHits()) {
          indexRecoTracksWithKLMHits[numberRecoTracksWithKLMHits] = i;
          numberRecoTracksWithKLMHits++;
        }
      }

      if (numberRecoTracksWithKLMHits > 2) {
        numberRecoTracksWithKLMHits = 0;
        for (int i = 0; i < numberRecoTracksWithKLMHits; i++) {
          int numberKLMHitsInRecoTrack = recoTrackStoreArray[indexRecoTracksWithKLMHits[i]]->getNumberOfBKLMHits() +
                                         recoTrackStoreArray[indexRecoTracksWithKLMHits[i]]->getNumberOfEKLMHits();
          if (numberKLMHitsInRecoTrack > 3) {
            indexRecoTracksWithKLMHits[numberRecoTracksWithKLMHits] = i;
            numberRecoTracksWithKLMHits++;
          }
        }
      }

      if (numberRecoTracksWithKLMHits == 2) {
        // A candidate of CosmicRecoTrack has at least m_minimumNumHitCut (40) CDCHits
        if (recoTrackStoreArray[indexRecoTracksWithKLMHits[0]]->getNumberOfCDCHits() +
            recoTrackStoreArray[indexRecoTracksWithKLMHits[1]]->getNumberOfCDCHits() > m_minimumNumHitCut) {

          // Merging cosmic tracks
          MergingTracks(recoTrackStoreArray, mergedRecoTracks);

          /*
                // Definition of upper and lower part of CosmicRecoTrack
                RecoTrack* upperTrack;
                RecoTrack* lowerTrack;

                if (recoTrackStoreArray[indexRecoTracksWithKLMHits[0]]->getTimeSeed() >
                    recoTrackStoreArray[indexRecoTracksWithKLMHits[1]]->getTimeSeed()) {
                  upperTrack = recoTrackStoreArray[indexRecoTracksWithKLMHits[0]];
                  lowerTrack = recoTrackStoreArray[indexRecoTracksWithKLMHits[1]];
                } else {
                  upperTrack = recoTrackStoreArray[indexRecoTracksWithKLMHits[1]];
                  lowerTrack = recoTrackStoreArray[indexRecoTracksWithKLMHits[0]];
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

          if (totalADCCount/numberOfCDCHits < 115) {
            // y = exp((-p0+x)/p1) + p2 ; where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum
            m_magnitudeOfMomentumWithoutMagneticField = exp((-fittedValuesOfFunctions[0]+totalADCCount/numberOfCDCHits)/fittedValuesOfFunctions[1]);
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
            TVector3 momentum = upperTrack->getMomentumSeed();
            float magnitudeMomentum = momentum.Mag();
            float newMomentumX = (momentum.Px()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumY = (momentum.Py()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            float newMomentumZ = (momentum.Pz()*m_magnitudeOfMomentumWithoutMagneticField)/magnitudeMomentum;
            TVector3 newMomentum(newMomentumX, newMomentumY, newMomentumZ);
            std::cout << "No Magnetic Field: " << newMomentum.Mag() << "\n";
            mergedRecoTrack = mergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                     newMomentum,
                     upperTrack->getChargeSeed());
          }
                mergedRecoTrack->setTimeSeed(upperTrack->getTimeSeed());

                int sortingNumber = 0;

                if (upperTrack->hasPXDHits()) {
                  int PXDHits = upperTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
                    sortingNumber++;
                  }
                }

                if (lowerTrack->hasPXDHits()) {
                  int PXDHits = lowerTrack->getNumberOfPXDHits();
                  for (int i = 0; i < PXDHits; i++) {
                    mergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);
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
          */
        }
      }
    }
  }
}
