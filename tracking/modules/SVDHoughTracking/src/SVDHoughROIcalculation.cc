/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/SVDHoughTracking/SVDHoughTrackingModule.h>
//#include <tracking/modules/svdHoughtracking/basf2_tracking.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/MaterialEffects.h>
#include <genfit/MeasurementFactory.h>
#include <genfit/RKTrackRep.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <geometry/GeometryManager.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <root/TMath.h>
#include <root/TGeoMatrix.h>
#include <root/TRandom.h>
#include <time.h>

#include <TH1F.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGeoManager.h>
#include <TDatabasePDG.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

/*
 * Analyse ROIs using first the TrueHits to check if the relevant
 * Hits are inside and then go over all SimHits to also include
 * background.
 */
void
SVDHoughTrackingModule::analyseROI()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  StoreArray<PXDSimHit> storePXDSimHit(m_storePXDSimHitsName);
  PXDTrueHit* currHit;
  MCParticle* particle;
  TVector3 pos, last_pos, local_pos;
  TVector3 diff, vec, abs_pos;
  TVector2 hit_pos;
  TVector2 v1, v2;
  VxdID sensorID, roi_sensorID;
  VxdID last_sensorID;
  int i, nMCParticles, nSimHit;
  int nMcTrue, nROI;
  int foundHits;
  unsigned int corrFound;
  unsigned int primaryTracks = 0;
  unsigned int nHits;
  bool layer[2], found_layer[2];
  double dist;
  int eventPixelInsideROI = 0;
  int eventTotalActivePixel = 0;


  RelationArray mcParticlesToTrueHits(storeMCParticles, storePXDTrueHits);

  nMcTrue = mcParticlesToTrueHits.getEntries();
  nMCParticles = storeMCParticles.getEntries();
  nROI = pxdROI.size();

  B2DEBUG(200, "Analyze ROIs: " << nROI);

  if (!mcParticlesToTrueHits.isValid()) {
    B2WARNING("  Relation Array not valid!");
  }

  B2DEBUG(200, "  Number of MCParticles: " << nMCParticles);
  foundHits = 0;
  corrFound = 0;
  nHits = 0;
  for (int i = 0; i < /*nMCParticles*/ nMcTrue; ++i) {
    particle = storeMCParticles[i];
    layer[0] = false;
    layer[1] = false;
    found_layer[0] = false;
    found_layer[1] = false;

    //  if(/*particle->getMomentum().Perp() < 0.04  ||*/ particle->getMomentum().Theta()/Unit::deg < 16 || particle->getMomentum().Theta()/Unit::deg > 152  ) {
    //    //B2WARNING("Particle has pT < 50MeV, and thus can't reach the outermost SVD layer and cannot be reconstructed. Skipping...!!!");
    //    continue;
    //  }

    //if (particle->getMother() == NULL) {
    //if (particle->hasStatus(MCParticle::c_PrimaryParticle) && particle->getCharge() != 0) {
    if (particle->hasStatus(MCParticle::c_PrimaryParticle) &&  particle->hasStatus(MCParticle::c_StableInGenerator) &&
        /* (particle->hasStatus(MCParticle::c_LeftDetector) || particle->hasStatus(MCParticle::c_StoppedInDetector)) && */
        particle->getCharge() != 0) {
      ++primaryTracks;
      /* Iterate over all TrueHits in a detector */
      for (unsigned int j = 0; j < mcParticlesToTrueHits[i].getToIndices().size(); j++) {
        bool currHitinROI = false;

        /* probably wrong definition of ntotPXDactivePixel for estimating DRF */
        //ntotPXDactivePixel += mcParticlesToTrueHits[i].getToIndices().size();

        currHit = storePXDTrueHits[mcParticlesToTrueHits[i].getToIndex(j)];
        sensorID = currHit->getSensorID();
        hit_pos.Set(currHit->getU(), currHit->getV());
        layer[sensorID.getLayerNumber() - 1] = true;

        B2DEBUG(200, "    Hit id: " << mcParticlesToTrueHits[i].getToIndex(j) << " hit in sensor " << sensorID
                << " pos: " << hit_pos.X() << ", " << hit_pos.Y());
        /* Iterate over all ROIs */
        for (int k = 0; k < nROI; ++k) {
          roi_sensorID = pxdROI[k].getSensorID();
          // do not continue looping over all ROI if currHit and estimated pxdROI are not on same layer
          if (!(roi_sensorID.getLayerNumber() == sensorID.getLayerNumber())) {
            continue;
          }

          if (roi_sensorID == sensorID) {
            v1 = pxdROI[k].getV1();
            v2 = pxdROI[k].getV2();
            B2DEBUG(200, "      ROI v1: " << v1.X() << ", " << v1.Y() << " v2: "
                    << v2.X() << ", " << v2.Y());

            if (hit_pos.X() >= v1.X() && hit_pos.X() <= v2.X() &&
                hit_pos.Y() >= v1.Y() && hit_pos.Y() <= v2.Y()) {
              found_layer[sensorID.getLayerNumber() - 1] = true;
              // count pixel only in one ROI, but not in another ROI
              if (!currHitinROI) {
                ntotPXDinsideROIPixel++;
                currHitinROI = true;
                /** For event DRF */
                eventPixelInsideROI++;
              }
              //++foundHits;        // wahrscheinlich überflüssig (zusammen mit dem ersten else if unten)
            }
          }
        }
      }

      if (layer[0] == true) {
        ++nHits;
      }
      if (layer[1] == true) {
        ++nHits;
      }

      /* Accept ROIs for this track if either 1 is found in both layers, or at
       * least two
       */
      if (found_layer[0] == true && found_layer[1] == true) {
        corrFound += 2;
        /*
        } else if (foundHits > 1) {     // wahrscheinlich überflüssig (siehe oben)
          corrFound += 2;
        } else if ((found_layer[0] == true || found_layer[1] == true) &&
                   (mcParticlesToTrueHits[i].getToIndices().size() < 2)) {
          corrFound += 1;
        }
        */
      } else if ((found_layer[0] == true || found_layer[1] == true)) {
        corrFound += 1;
      }
    }
  }
  /* probably better definition of ntotPXDactivePixel for estimating DRF */
  ntotPXDactivePixel += nHits;
  /* value used by Michael */
  ntotROITrueHits += nHits;
  /** Event ROI efficiency */
  eventROIEff += ((double)corrFound / (double)nHits);
  /** Event DRF */
  eventTotalActivePixel += nHits;


  nSimHit = storePXDSimHit.getEntries();
  /* Count numbers of clustered SimHits inside ROIs */
  svd_sensor_cluster_map.clear();
  dist = 1E+50;
  for (i = 0; i < nSimHit; ++i) {
    if (i == 0) {
      ntotPXDactivePixel += nSimHit;
      /** Event DRF */
      eventTotalActivePixel += nSimHit;
    }
    bool currSimHitinROI = false;

    vec = storePXDSimHit[i]->getPosIn();
    VxdID sensorID = storePXDSimHit[i]->getSensorID();
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

    /* Convert local to global position */
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    abs_pos = info.pointToGlobal(storePXDSimHit[i]->getPosIn());
    pos = abs_pos;

    /*    // this if...else created by Michael may be wrong, thus new version below
    if (i == 0) {
      if (i == (nSimHit - 1)) {
        for (int k = 0; k < nROI; ++k) {
          roi_sensorID = pxdROI[k].getSensorID();
          //if (roi_sensorID == last_sensorID) {
          if (roi_sensorID == sensorID) {
            v1 = pxdROI[k].getV1();
            v2 = pxdROI[k].getV2();

            if (vec.X() >= v1.X() && vec.X() <= v2.X() &&
                vec.Y() >= v1.Y() && vec.Y() <= v2.Y()) {
              ++ncurHitsInROIs;

              if (!currSimHitinROI) {
                ntotPXDinsideROIPixel++;
                currSimHitinROI = true;
              }

            }
          }
        }
        //++ntotROITrueHits;
        ntotROISimHits++;
      }
    } else {
      //diff = pos - last_pos;
      diff = abs_pos - last_pos;
      dist = diff.Mag();

      if (fabs(dist) < 0.02 && i != (nSimHit - 1)) {
      } else {
        for (int k = 0; k < nROI; ++k) {
          roi_sensorID = pxdROI[k].getSensorID();
          //if (roi_sensorID == last_sensorID) {
          if (roi_sensorID == sensorID) {
            v1 = pxdROI[k].getV1();
            v2 = pxdROI[k].getV2();

            if (vec.X() >= v1.X() && vec.X() <= v2.X() &&
                vec.Y() >= v1.Y() && vec.Y() <= v2.Y()) {
              ++ncurHitsInROIs;

              if (!currSimHitinROI) {
                ntotPXDinsideROIPixel++;
                currSimHitinROI = true;
              }

            }
          }
        }
        //++ntotROITrueHits;
        ntotROISimHits++;
      }
    }
    */

    for (int k = 0; k < nROI; ++k) {
      roi_sensorID = pxdROI[k].getSensorID();
      // do not continue looping over all ROI if currHit and estimated pxdROI are not on same layer
      if (!(roi_sensorID.getLayerNumber() == sensorID.getLayerNumber())) {
        continue;
      }

      if (roi_sensorID == sensorID) {
        v1 = pxdROI[k].getV1();
        v2 = pxdROI[k].getV2();

        if (vec.X() >= v1.X() && vec.X() <= v2.X() &&
            vec.Y() >= v1.Y() && vec.Y() <= v2.Y()) {
          ++ncurHitsInROIs;
          if (!currSimHitinROI) {
            ntotPXDinsideROIPixel++;
            currSimHitinROI = true;
            /** For event DRF */
            eventPixelInsideROI++;
          }
        }
      }
    }

    // belongs to last for loop, but not sure whether is needed
    //if (ncurHitsInROIs > 0) {
    //  ntotROISimHits++;
    //}

    last_pos = abs_pos;
    last_sensorID = sensorID;
  }

  B2DEBUG(200, "  -> Total ROIs: " << nHits << " Correct found: " << corrFound);

  if (nHits > 0) {
    curROIEff += ((double) corrFound) / ((double) nHits);
    m_histEventROIEfficiency->Fill((double)eventROIEff);// / (double)primaryTracks);
    m_treeEventROIEfficiency = (double)eventROIEff;// / (double)primaryTracks;
    m_histEventDRF->Fill((double)eventTotalActivePixel / (double)eventPixelInsideROI);
    m_treeEventDRF = (double)eventTotalActivePixel / (double)eventPixelInsideROI;
  }
  m_treeEventEventNo = evtNumber;
  m_rootTreeROI->Fill();
}


/*
 * Calculate Region of Interests (ROI).
 */
void
SVDHoughTrackingModule::createROI()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  SVDHoughCluster* currHit;
  int size_u = m_fixedSizeUCells;
  int size_v = m_fixedSizeVCells;
  /** Reminder: 250 px in u-direction = r-phi, in total 768 (512+256) px in v-direction = z */
  //double offset_u, offset_v; // offset_x, offset_y;
  double u, v; // x, y;
  int ucell, vcell;
  int ucell_ll, vcell_ll, ucell_ur, vcell_ur; // ll = lower left, ur = upper right
  int ucell_ll_tmp, vcell_ll_tmp, ucell_ur_tmp, vcell_ur_tmp;
  double u_ll, v_ll, u_ur, v_ur;
  int sensorChange = 0, ladderChange = 0;
  int ucell_rest = 0, vcell_rest = 0;
  int ucells = 250, vcells = 768;
  int nExHits;
  bool fixedSize = true;    // initially: true
  VxdID hitSensorID;
  VxdID nextSensorID;
  TVector3 pos, local_pos;
  TVector2 v1, v2;
  ofstream of_l1, of_l2;

  /* Clear ROI vector first */
  pxdROI.clear();

  if (m_createPXDMap) {
    of_l1.open("dbg/rois_l1.dat", ofstream::out);
    of_l2.open("dbg/rois_l2.dat", ofstream::out);
  }

  B2DEBUG(200, "Create ROIs: ");

  nExHits = storeExtrapolatedHits.getEntries();
  for (int i = 0; i < nExHits; ++i) {
    currHit = storeExtrapolatedHits[i];
    pos = currHit->getHitPos();
    hitSensorID = currHit->getSensorID();
    unsigned qOH = currHit->getQOH();

    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    const VXD::SensorInfoBase& info = geo.get(hitSensorID);
    local_pos = info.pointToLocal(pos);
    u = local_pos.X();
    v = local_pos.Y();

    const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(hitSensorID));
    //double width = currentSensor->getWidth();
    //double length = currentSensor->getLength();

    ucell = currentSensor->getUCellID(u, v, false);
    vcell = currentSensor->getVCellID(v, false);
    ucells = currentSensor->getUCells();
    vcells = currentSensor->getVCells();

    int sensorNumber = hitSensorID.getSensorNumber();
    int layerNumber  = hitSensorID.getLayerNumber();
    int ladderNumber = hitSensorID.getLadderNumber();

    /* Determine size of ROI */
    if (fixedSize) {
      size_u = m_fixedSizeUCells;
      size_v = m_fixedSizeVCells;
    } else {
      /*
      switch (qOH) {
        case 0:
          if (layerNumber == 1) {
            size_u = 10;
            size_v = 64;
          }
          break;
        case 1:
          if (layerNumber == 1) {
            size_u = 10;
            size_v = 48;
          } else {
            size_u = 10;
            size_v = 64;
          }
          break;
        case 2:
          size_u = 8;
          size_v = 16;
          break;
      }
      */
      switch (qOH) {
        case 1:
          size_u = 10;
          size_v = 64;
          break;
        case 2:
          size_u = 8;
          size_v = 16;
      }
    }

    /** Lower left corner */
    //ucell_ll = ucell - size_u / 2;
    ucell_ll = ucell - 0.35 * size_u;
    vcell_ll = vcell - size_v / 2;

    if (sensorNumber == 1) {
      if (vcell_ll < 0) {
        vcell_ll = 0;
      }
    } else if (sensorNumber == 2) {
      if (vcell_ll < 0) {
        vcell_ll = 0;
        sensorChange = -1;
        vcell_rest = size_v / 2 - (vcell - vcell_ll);
      }
    }

    if (ucell_ll < 0) {
      ucell_ll = 0;
      ladderChange = -1;
      //ucell_rest = size_u / 2 - (ucell - ucell_ll);
      ucell_rest = 0.35 * size_u - (ucell - ucell_ll);
    }

    /** Upper right corner */
    //ucell_ur = ucell + size_u / 2;
    ucell_ur = ucell + 0.65 * size_u;
    vcell_ur = vcell + size_v / 2;

    if (sensorNumber == 1) {
      if (vcell_ur > vcells - 1) {
        vcell_ur = vcells - 1;
        sensorChange = +1;
        vcell_rest = size_v / 2 - (vcell_ur - vcell);
      }
    } else if (sensorNumber == 2) {
      if (vcell_ur >= vcells) {
        vcell_ur = vcells - 1;
      }
    }

    if (ucell_ur >= ucells) {
      ucell_ur = ucells - 1;
      ladderChange = +1;
      //ucell_rest = size_u / 2 - (ucell_ur - ucell);
      ucell_rest = 0.65 * size_u - (ucell_ur - ucell);
    }

    /** Convert CellID to sensor coords.
     *Maybe this is not necessary and one can directly send the CellID
     */
    u_ll = currentSensor->getUCellPosition(ucell_ll, vcell_ll);
    v_ll = currentSensor->getVCellPosition(vcell_ll);
    u_ur = currentSensor->getUCellPosition(ucell_ur, vcell_ur);
    v_ur = currentSensor->getVCellPosition(vcell_ur);

    v1.Set(u_ll, v_ll);
    v2.Set(u_ur, v_ur);

    pxdROI.push_back(SVDHoughROI(hitSensorID, v1, v2));

    /** ROI covering several sensors / ladder */
    if (ladderChange == -1 && sensorChange == 0) {
      /** case 1 */
      nextSensorID = hitSensorID;
      // Check for minimum ladderNumber, set to MaxLadder if required
      if (ladderNumber > 1) {
        nextSensorID.setLadderNumber(ladderNumber + ladderChange);
      } else {
        if (layerNumber == 1) {
          nextSensorID.setLadderNumber(8);
        } else if (layerNumber == 2) {
          nextSensorID.setLadderNumber(12);
        }
      }
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = ucells - 1 - ucell_rest;
      ucell_ur_tmp = ucells - 1;
      vcell_ll_tmp = vcell_ll;
      vcell_ur_tmp = vcell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    } else if (ladderChange == +1 && sensorChange == 0) {
      /** case 2 */
      nextSensorID = hitSensorID;
      if (layerNumber == 1) {
        if (ladderNumber < 8) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      } else if (layerNumber == 2) {
        if (ladderNumber < 12) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      }
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = 0;
      ucell_ur_tmp = ucell_rest;
      vcell_ll_tmp = vcell_ll;
      vcell_ur_tmp = vcell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    } else if (ladderChange == 0 && sensorChange == -1) {
      /** case 3 */
      nextSensorID = hitSensorID;
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(1);
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      vcell_ll_tmp = vcells - 1 - vcell_rest;
      vcell_ur_tmp = vcells - 1;
      ucell_ll_tmp = ucell_ll;
      ucell_ur_tmp = ucell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    } else if (ladderChange == 0 && sensorChange == +1) {
      /** case 4 */
      nextSensorID = hitSensorID;
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(2);
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      vcell_ll_tmp = 0;
      vcell_ur_tmp = vcell_rest;
      ucell_ll_tmp = ucell_ll;
      ucell_ur_tmp = ucell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    } else if (ladderChange == -1 && sensorChange == -1) {
      /** case 5 */
      nextSensorID = hitSensorID;
      // Check for minimum ladderNumber, set to MaxLadder if required
      if (ladderNumber > 1) {
        nextSensorID.setLadderNumber(ladderNumber + ladderChange);
      } else {
        if (layerNumber == 1) {
          nextSensorID.setLadderNumber(8);
        } else if (layerNumber == 2) {
          nextSensorID.setLadderNumber(12);
        }
      }
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = ucells - 1 - ucell_rest;
      ucell_ur_tmp = ucells - 1;
      vcell_ll_tmp = vcell_ll;
      vcell_ur_tmp = vcell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));


      nextSensorID = hitSensorID;
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(1);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      vcell_ll_tmp = 0;
      vcell_ur_tmp = vcell_rest;
      ucell_ll_tmp = ucell_ll;
      ucell_ur_tmp = ucell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

      nextSensorID = hitSensorID;
      // Check for minimum ladderNumber, set to MaxLadder if required
      if (ladderNumber > 1) {
        nextSensorID.setLadderNumber(ladderNumber + ladderChange);
      } else {
        if (layerNumber == 1) {
          nextSensorID.setLadderNumber(8);
        } else if (layerNumber == 2) {
          nextSensorID.setLadderNumber(12);
        }
      }
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(1);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      vcell_ll_tmp = 0;
      vcell_ur_tmp = vcell_rest;
      ucell_ll_tmp = ucells - 1 - ucell_rest;
      ucell_ur_tmp = ucells - 1;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    } else if (ladderChange == -1 && sensorChange == +1) {
      /** case 6 */
      nextSensorID = hitSensorID;
      // Check for minimum ladderNumber, set to MaxLadder if required
      if (ladderNumber > 1) {
        nextSensorID.setLadderNumber(ladderNumber + ladderChange);
      } else {
        if (layerNumber == 1) {
          nextSensorID.setLadderNumber(8);
        } else if (layerNumber == 2) {
          nextSensorID.setLadderNumber(12);
        }
      }
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = ucells - 1 - ucell_rest;
      ucell_ur_tmp = ucells - 1;
      vcell_ll_tmp = vcell_ll;
      vcell_ur_tmp = vcell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));


      nextSensorID = hitSensorID;
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(2);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      vcell_ll_tmp = 0;
      vcell_ur_tmp = vcell_rest;
      ucell_ll_tmp = ucell_ll;
      ucell_ur_tmp = ucell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

      nextSensorID = hitSensorID;
      // Check for minimum ladderNumber, set to MaxLadder if required
      if (ladderNumber > 1) {
        nextSensorID.setLadderNumber(ladderNumber + ladderChange);
      } else {
        if (layerNumber == 1) {
          nextSensorID.setLadderNumber(8);
        } else if (layerNumber == 2) {
          nextSensorID.setLadderNumber(12);
        }
      }
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(2);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = ucells - 1 - ucell_rest;
      ucell_ur_tmp = ucells - 1;
      vcell_ll_tmp = 0;
      vcell_ur_tmp = vcell_rest;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    } else if (ladderChange == +1 && sensorChange == -1) {
      /** case 7 */
      nextSensorID = hitSensorID;
      if (layerNumber == 1) {
        if (ladderNumber < 8) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      } else if (layerNumber == 2) {
        if (ladderNumber < 12) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      }
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = 0;
      ucell_ur_tmp = ucell_rest;
      vcell_ll_tmp = vcell_ll;
      vcell_ur_tmp = vcell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));


      nextSensorID = hitSensorID;
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(1);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      vcell_ll_tmp = vcells - 1 - vcell_rest;
      vcell_ur_tmp = vcells - 1;
      ucell_ll_tmp = ucell_ll;
      ucell_ur_tmp = ucell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

      nextSensorID = hitSensorID;
      if (layerNumber == 1) {
        if (ladderNumber < 8) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      } else if (layerNumber == 2) {
        if (ladderNumber < 12) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      }
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(1);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = 0;
      ucell_ur_tmp = ucell_rest;
      vcell_ll_tmp = vcells - 1 - vcell_rest;
      vcell_ur_tmp = vcells - 1;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    } else if (ladderChange == +1 && sensorChange == +1) {
      /** case 8 */
      nextSensorID = hitSensorID;
      if (layerNumber == 1) {
        if (ladderNumber < 8) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      } else if (layerNumber == 2) {
        if (ladderNumber < 12) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      }
      const PXD::SensorInfo* nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = 0;
      ucell_ur_tmp = ucell_rest;
      vcell_ll_tmp = vcell_ll;
      vcell_ur_tmp = vcell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));


      nextSensorID = hitSensorID;
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(2);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      vcell_ll_tmp = 0;
      vcell_ur_tmp = vcell_rest;
      ucell_ll_tmp = ucell_ll;
      ucell_ur_tmp = ucell_ur;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));


      nextSensorID = hitSensorID;
      if (layerNumber == 1) {
        if (ladderNumber < 8) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      } else if (layerNumber == 2) {
        if (ladderNumber < 12) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          nextSensorID.setLadderNumber(1);
        }
      }
      //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
      nextSensorID.setSensorNumber(2);
      nextSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(nextSensorID));
      ucell_ll_tmp = 0;
      ucell_ur_tmp = ucell_rest;
      vcell_ll_tmp = 0;
      vcell_ur_tmp = vcell_rest;

      u_ll = nextSensor->getUCellPosition(ucell_ll_tmp, vcell_ll_tmp);
      v_ll = nextSensor->getVCellPosition(vcell_ll_tmp);
      u_ur = nextSensor->getUCellPosition(ucell_ur_tmp, vcell_ur_tmp);
      v_ur = nextSensor->getVCellPosition(vcell_ur_tmp);

      v1.Set(u_ll, v_ll);
      v2.Set(u_ur, v_ur);

      pxdROI.push_back(SVDHoughROI(nextSensorID, v1, v2));

    }


    /** With the new ROI calculation this would not make sense -> dicard for now, fix later
    B2DEBUG(200, "  Local position of ROI in sensor: " << sensorID << " Coord: "
            << local_pos.Y() << " " << local_pos.X() << " v1: " <<
            v1.X() << ", " << v1.Y() << " v2: " << v2.X() << ", " << v2.Y());
    pxdROI.push_back(SVDHoughROI(sensorID, v1, v2));

    if (m_createPXDMap) {
      if (sensorID.getLayerNumber() == 1) {
        offset_x = 4.48 * (sensorID.getSensorNumber() - 1) + 4.48 / 2.0;
        offset_y = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + 1.25 / 2.0;

        of_l1 << "set object rect from " << v1.X() + offset_x << ", "
              << v1.Y() + offset_y << " to " << v2.X() + offset_x << ", "
              << v2.Y() + offset_y << " " << "fc rgb \"green\" fs solid 0.5 behind"
              << endl;
      } else {
        offset_x = 6.144 * (sensorID.getSensorNumber() - 1) + 6.144 / 2.0;
        offset_y = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + 1.25 / 2.0;

        of_l2 << "set object rect from " << v1.X() + offset_x << ", "
              << v1.Y() + offset_y << " to " << v2.X() + offset_x << ", "
              << v2.Y() + offset_y << " " << "fc rgb \"green\" fs solid 0.5 behind"
              << endl;
      }
    }
    */

  }

  if (m_createPXDMap) {
    of_l1.close();
    of_l2.close();
  }
}



/*
 * For occupancy and ROI reduction studies, convert the PXD Sim hits
 * into a local format.
 */
void
SVDHoughTrackingModule::createPXDMap()
{
  StoreArray<PXDSimHit> storePXDSimHit(m_storePXDSimHitsName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  int i, cluster_cnt;
  int nSimHit, nTrueHit;
  double dist;
  double u, v;
  VxdID sensorID, last_sensorID;
  TVector3 pos, last_pos, local_pos;
  TVector3 diff, vec, abs_pos;
  TVector2 truehit_pos;
  ofstream of_l1, of_l2;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);
  PXDTrueHit* currTrueHit;

  nSimHit = storePXDSimHit.getEntries();
  nTrueHit = storePXDTrueHits.getEntries();
  if (nSimHit == 0 && nTrueHit == 0) {
    return;
  }

  of_l1.open("dbg/pxd_occ_l1.dat", ofstream::out);
  of_l2.open("dbg/pxd_occ_l2.dat", ofstream::out);

  B2DEBUG(200, "Create PXD map: " << nSimHit);

  /* First convert to absolute hits and save into a map */
  svd_sensor_cluster_map.clear();
  dist = 1E+50;
  cluster_cnt = 0;
  for (i = 0; i < nSimHit; ++i) {
    vec = storePXDSimHit[i]->getPosIn();
    VxdID sensorID = storePXDSimHit[i]->getSensorID();
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

    /* Convert local to global position */
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    abs_pos = info.pointToGlobal(storePXDSimHit[i]->getPosIn());
    pos = abs_pos;

    if (i == 0) {
      if (i == (nSimHit - 1)) {
        B2DEBUG(200, "  Add in Layer " << sensorID.getLayerNumber() << " sensor: " << sensorID << " coord: "
                << vec.X() << " " << vec.Y() << " " << vec.Z());
        if (sensorID.getLayerNumber() == 1) {
          v = 4.48 * (sensorID.getSensorNumber() - 1) + vec.Y() + 4.48 / 2.0;
          u = (1.2 + 0.2) * (sensorID.getLadderNumber() - 1) + vec.X() + 1.2 / 2.0;
          m_histROIPXD_l1->Fill(v, u);
          of_l1 << v << " " << u << endl;
        } else {
          v = 6.144 * (sensorID.getSensorNumber() - 1) + vec.Y() + 6.144 / 2.0;
          u = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + vec.X() + 1.25 / 2.0;
          m_histROIPXD_l2->Fill(v, u);
          of_l2 << v << " " << u << endl;
        }
        ++cluster_cnt;
      }
    } else {
      //diff = pos - last_pos;
      diff = abs_pos - last_pos;
      dist = diff.Mag();
      B2DEBUG(250, "   Distance: " << dist);

      if (fabs(dist) < 0.02 && i != (nSimHit - 1)) {
      } else {
        B2DEBUG(200, "  Add in sensor: " << last_sensorID << " coord: " << vec.X() << " " << vec.Y() << " " << vec.Z()
                << " Layer " << last_sensorID.getLayerNumber() << " Ladder: " << last_sensorID.getLadderNumber()
                << " Sensor: " << last_sensorID.getSensorNumber());
        if (last_sensorID.getLayerNumber() == 1) {
          v = 4.48 * (last_sensorID.getSensorNumber() - 1) + vec.Y() + 4.48 / 2.0;
          u = (1.2 + 0.2) * (last_sensorID.getLadderNumber() - 1) + vec.X() + 1.2 / 2.0;
          m_histROIPXD_l1->Fill(v, u);
          of_l1 << v << " " << u << endl;
        } else {
          v = 6.144 * (last_sensorID.getSensorNumber() - 1) + vec.Y() + 6.144 / 2.0;
          u = (1.25 + 0.2) * (last_sensorID.getLadderNumber() - 1) + vec.X() + 1.25 / 2.0;
          m_histROIPXD_l2->Fill(v, u);
          of_l2 << v << " " << u << endl;
        }
        ++cluster_cnt;
      }
    }

    //last_pos = pos;
    last_pos = abs_pos;
    last_sensorID = sensorID;
  }

  for (int i = 0; i < nTrueHit; ++i) {
    currTrueHit = storePXDTrueHits[i];
    sensorID = currTrueHit->getSensorID();

    truehit_pos.Set(currTrueHit->getU(), currTrueHit->getV());
    B2DEBUG(200, "    Add PXD TrueHit SensorID: " << sensorID << " Coord: "
            << local_pos.X() << ", " << local_pos.Y());
    if (sensorID.getLayerNumber() == 1) {
      v = 4.48 * (sensorID.getSensorNumber() - 1) + truehit_pos.Y() + 4.48 / 2.0;
      u = (1.2 + 0.2) * (sensorID.getLadderNumber() - 1) + truehit_pos.X() + 1.2 / 2.0;
      m_histROIPXD_l1->Fill(v, u);
      of_l1 << v << " " << u << endl;
    } else {
      v = 6.144 * (sensorID.getSensorNumber() - 1) + truehit_pos.Y() + 6.144 / 2.0;
      u = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + truehit_pos.X() + 1.25 / 2.0;
      m_histROIPXD_l2->Fill(v, u);
      of_l2 << v << " " << u << endl;
    }
    ++cluster_cnt;
  }

  of_l1.close();
  of_l2.close();
}



/*
 * Take the hough tracks and do the extrapolation to the PXD layer.
 */
void
SVDHoughTrackingModule::createResiduals()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  std::map<int, TVector3>::iterator iter;
  TVector3 vec;
  double dist;

  for (iter = pxd_cluster_map.begin(); iter != pxd_cluster_map.end(); ++iter) {
    unsigned int num_hits = storeExtrapolatedHits.getEntries();
    for (unsigned int i = 0; i < num_hits; ++i) {
      TVector3 hit = storeExtrapolatedHits[i]->getHitPos();
      VxdID sensor = storeExtrapolatedHits[i]->getSensorID();
      TVector3 clus = iter->second;
      vec = hit - clus;
      dist = vec.Mag();
      const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
      if (dist < 0.5) {
        B2INFO("Residuals for Sensor: " << sensor);
        B2INFO("Dx: " << fabs(vec.x()) << " (" << fabs(vec.x()) / currentSensor->getUPitch() << " px)"
               << " Dy: " <<  fabs(vec.y())
               << " Dz: " << fabs(vec.z()) << " (" <<  fabs(vec.z()) / currentSensor->getVPitch()
               << " px)");
      }
    }
  }
}

/*
 * Take the hough tracks and do the extrapolation to the PXD
 * layer -- Testbeam version.
 */
void
SVDHoughTrackingModule::createTbResiduals()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  std::map<int, TVector3>::iterator iter;
  TVector3 vec;
  double dist;
  double pitch = 50 * Unit::um;

  for (iter = pxd_cluster_map.begin(); iter != pxd_cluster_map.end(); ++iter) {
    unsigned int num_hits = storeExtrapolatedHits.getEntries();
    for (unsigned int i = 0; i < num_hits; ++i) {
      TVector3 hit = storeExtrapolatedHits[i]->getHitPos();
      VxdID sensor = storeExtrapolatedHits[i]->getSensorID();
      TVector3 clus = iter->second;
      vec = hit - clus;
      dist = vec.Mag();
      if (dist < 0.5) {
        B2INFO("Residuals for Sensor: " << sensor);
        B2INFO("Dx: " << fabs(vec.x()) << " (" << fabs(vec.x()) / pitch << " px)"
               << " Dy: " <<  fabs(vec.y()) << " (" << abs(vec.y()) / pitch << " px)"
               << " Dz: " << fabs(vec.z()) << " (" <<  fabs(vec.z()) / pitch
               << " px)");
      }
    }
  }
}

