/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef VXDTFTRACKCANDIDATE_H
#define VXDTFTRACKCANDIDATE_H

#include <framework/gearbox/Const.h>

#include "VXDTFHit.h"
#include "VXDSegmentCell.h"
#include "SharedFunctions.h"

#include <list>
#include <string>
#include <vector>

#include <TVector3.h>

namespace Belle2 {
  namespace Tracking {


    /** The VXD Track Finder Track Candidate class
    * This class stores all information needed for reconstructing track candidates within the VXDonlyTrackFinder.
    * It's for internal and analysis use only and will be exported as a genfit::TrackCandidate which is the official
    * interface for track reconstruction modules.
    */
    class VXDTFTrackCandidate {
    public:
      /** Default constructor. */
      VXDTFTrackCandidate():
        m_overlapping(false),
        m_alive(true),
        m_reserved(false),
        m_qualityIndex(1.0),
        m_qqq(1.0),
        m_neuronValue(0.0),
        m_estRadius(0.),
        m_pdgCode(0),
        m_passIndex(-1),
        m_fitSucceeded(false),
        m_trackNumber(-1),
        m_initialValuesSet(false) {
        m_attachedHits.reserve(12);
        m_attachedCells.reserve(10);
        m_pxdHitIndices.reserve(4);
        m_svdHitIndices.reserve(16);
      }

      /**copy constructor**/
      VXDTFTrackCandidate(VXDTFTrackCandidate*& other):
        m_attachedHits((*other).m_attachedHits),
        m_attachedCells((*other).m_attachedCells),
        m_bookingRivals((*other).m_bookingRivals),
        m_svdHitIndices((*other).m_svdHitIndices),
        m_pxdHitIndices((*other).m_pxdHitIndices),
        m_hopfieldHitIndices((*other).m_hopfieldHitIndices),
        m_overlapping((*other).m_overlapping),
        m_alive((*other).m_alive),
        m_reserved((*other).m_reserved),
        m_qualityIndex((*other).m_qualityIndex),
        m_qqq((*other).m_qqq),
        m_neuronValue((*other).m_neuronValue),
        m_estRadius((*other).m_estRadius),
        m_pdgCode((*other).m_pdgCode),
        m_passIndex((*other).m_passIndex),
        m_fitSucceeded((*other).m_fitSucceeded),
        m_trackNumber((*other).m_trackNumber),
        m_initialHit((*other).m_initialHit),
        m_initialMomentum((*other).m_initialMomentum),
        m_initialValuesSet((*other).m_initialValuesSet) {
        if (m_alive == true) { for (VXDTFHit * aHit : m_attachedHits) { aHit->addTrackCandidate(); } }  // each time it gets copied, its hits have to be informed about that step
        /*m_neuronValue = 0; m_overlapping = false; m_alive = true; m_qualityIndex = 1.0;*/
      }

      /** getter **/
      std::vector<VXDSegmentCell*> getSegments() { return m_attachedCells; } /**< returns segments forming current TC */


      const std::vector<VXDTFHit*>& getHits() { return m_attachedHits; } /**< returns hits forming current TC. Currently, the first hit in the vector is the outermost hit  */


      const std::vector<PositionInfo*>* getPositionInfos() {
        if (m_attachedPositionInfos.size() == m_attachedHits.size()) { return &m_attachedPositionInfos; }
        m_attachedPositionInfos.clear();
        m_attachedPositionInfos.reserve(m_attachedHits.size());
        for (auto * aHit : m_attachedHits) { m_attachedPositionInfos.push_back(aHit->getPositionInfo()); }
//         for (std::vector<VXDTFHit*>::iterator it = m_attachedHits.begin(); it != m_attachedHits.end(); ++it) {
//           m_attachedPositionInfos.push_back((*it)->getPositionInfo());
//         }
        return &m_attachedPositionInfos;
      } /**< returns position infos (global hit coordinates and errors for x and y coordinates) forming current TC. Currently, the first hit in the vector is the outermost hit  */


      std::vector<TVector3*> getHitCoordinates() { /**< returns hit positions forming current TC */
        std::vector<TVector3*> coordinates;
        coordinates.reserve(m_attachedHits.size());
        for (VXDTFHit * hit : m_attachedHits) {
          coordinates.push_back(hit->getHitCoordinates());
        }
        return coordinates;
      }


      const std::vector<int>& getSVDHitIndices() {
        m_svdHitIndices.clear();
        m_svdHitIndices.reserve(m_attachedHits.size());
        int index;
        for (VXDTFHit * aHit : m_attachedHits) {
          if (aHit->getDetectorType() == Const::SVD) { /* SVD */
            index = aHit->getClusterIndexU();
            if (index != -1) { m_svdHitIndices.push_back(index); }
            index = aHit->getClusterIndexV();
            if (index != -1) { m_svdHitIndices.push_back(index); }
          }
        }
        return m_svdHitIndices;
      }  /**< returns real indices of svdClusters forming current TC */


      const std::vector<int>& getPXDHitIndices() {
        m_pxdHitIndices.clear();
        m_pxdHitIndices.reserve(m_attachedHits.size());
        int index;
        for (VXDTFHit * aHit : m_attachedHits) {
          if (aHit->getDetectorType() == Const::PXD) { /* PXD */
            index = aHit->getClusterIndexUV();
            if (index != -1) { m_pxdHitIndices.push_back(index); }
          }
        }
        return m_pxdHitIndices;
      } /**< returns real indices of pxdClusters forming current TC */


      //     std::list<int> getHopfieldHitIndices() { return m_hopfieldHitIndices; } /**< returns slightly adapted indices for hopfield use only (no real indices, but only unique ones...) */
      std::list<int> getHopfieldHitIndices(); /**< returns slightly adapted indices for hopfield use only (no real indices, but only unique ones...) */


      const std::vector<VXDTFTrackCandidate*>& getBookingRivals() { return m_bookingRivals; } /**< returns all TCs sharing hits with current one */


      bool getOverlappingState() { return m_overlapping; } /**< returns flag whether TC is sharing hits with other TCs or not (no manual check) */


      bool checkOverlappingState() { /**< returns flag whether TC is sharing hits with other TCs or not, after manual check, whether its rivals are still alive */
        int rivalsAlive = 0;
        for (VXDTFTrackCandidate * rival : m_bookingRivals) {
          if (rival->getCondition() == false) continue;
          rivalsAlive++;
        }
        if (rivalsAlive != 0) { m_overlapping = true; return true; } else { m_overlapping = false; return false; }
      }


      /** fast getter telling whether TC has full ownership on its Clusters or not */
      bool isReserved() { return m_reserved; }


      /** checks whether TC has full ownership on its Clusters or not, slower than isReserved, but more safe */
      bool checkReserved();


      unsigned int getTrackNumber() { return m_trackNumber; } /**< returns position of TC in vector containing all TCs of current event */


      bool getCondition() const { return m_alive; } /**< returns flag whether TC is still "alive" (part of the set of TCs which are probably real tracks based on the knowledge of the TF at the point of calling that function) */


      double getTrackQuality() { return m_qualityIndex; } /**< returns quality index of TC, has to be between 0 (bad) and 1 (perfect) */


      double getQQQ() { return m_qqq; } /**< returns aditional quality index */


      double getNeuronValue() const { return m_neuronValue; } /**< returns state of neuron during hopfield network */


      double getEstRadius() { return m_estRadius; } /**< returns the estimated radius of the track circle in the x-y-plane */


      TVector3 getInitialCoordinates() const { return m_initialHit; } /**< returns initial coordinates needed for export as genfit::TrackCand */


      TVector3 getInitialMomentum(); /**< returns initial momentum needed for export as genfit::TrackCand */


      int getPDGCode() const { return m_pdgCode; } /**< returns estimated PDGCode (its always a pion, but charge changes depending of the sign of the curvature of the track) needed for export as genfit::TrackCandid */


      int getPassIndex() { return m_passIndex; } /**< TCs are passDependent and are merged at the end of the TF-process to be filtered there */


      bool getFitSucceeded() { return m_fitSucceeded; } /**< returns true if kalman fit was possible and returned a result itself */


      int size() { return m_attachedHits.size(); } /**< returns number of attached hits */




      /** setter **/
      void addSVDClusterIndex(int anIndex) { m_svdHitIndices.push_back(anIndex); } /**< add index number of SVDCluster attached to current TC */


      void addPXDClusterIndex(int anIndex) { m_pxdHitIndices.push_back(anIndex); } /**< add index number of PXDCluster attached to current TC */


      void addBookingRival(VXDTFTrackCandidate* aTC) { /**< adds a TC sharing hits with current one */
        for (VXDTFTrackCandidate * rival : m_bookingRivals) { if (aTC == rival) { return; } } // filter double entries
        m_overlapping = true;
        m_bookingRivals.push_back(aTC);
      }


      void addHopfieldClusterIndex(int anIndex) { m_hopfieldHitIndices.push_back(anIndex); } /**< add index number of Cluster attached to current TC (SVD and PXD), index is unique but does not point to real clusters */


      void addSegments(VXDSegmentCell* pCell) { m_attachedCells.push_back(pCell); } /**< add segment attached to current TC */


      void addHits(VXDTFHit* pHit) { m_attachedHits.push_back(pHit); } /**< add hit attached to current TC */


      bool setReserved(); /**< claims full ownership of its clusters and returns whether this was successfull (true) or not (e.g. at least one Cluster was already reserved) */


      void setOverlappingState(bool newState) { m_overlapping = newState; } /**< set whether current TC is overlapped or not */


      void setTrackNumber(unsigned int newNumber) { m_trackNumber = newNumber; } /**< tells the TC which position in the tcList it has got. Allows some faster overlap-procedures */


      void setTrackQuality(double newVal) { m_qualityIndex = newVal; } /**< set estimated quality of TC */


      void setQQQ(double qqqScore, double maxScore) { m_qqq = sqrt(qqqScore / maxScore); } /**< set estimated extended quality of TC (a potential minimal replacement for kalman filter, interesting for online-use) */


      void setCondition(bool newCondition) {
        if (m_alive == true && newCondition == false) {   // in this case, the TC will be deactivated
          for (VXDTFHit * aHit : m_attachedHits) {
            aHit->removeTrackCandidate();
            // TODO: for each ClusterInfo in aHit-> removeTrackCandidate(this);
          }
        } else if (m_alive == false && newCondition == true) {   // in this case the TC will be (re)activated
          for (VXDTFHit * aHit : m_attachedHits) {
            aHit->addTrackCandidate();
          }
        }
        m_alive = newCondition;
      } /**< set condition. If true, TC is part of set of final TCs which are exported for further use */


      void setNeuronValue(double aValue) { m_neuronValue = aValue; }  /**< set neuron value, needed by the hopfield network */


      void setEstRadius(double radius) { if (radius < 0) radius *= -1.; m_estRadius = radius; }  /**< sets the estimated radius of the track circle in the x-y-plane */


      void removeVirtualHit(); /**< removes virtual hit which is needed for most filtering steps */


      void setInitialValue(const TVector3& aHit, const TVector3& pVector, int pdg) { m_initialHit = aHit; m_initialMomentum = pVector; m_pdgCode = pdg; m_initialValuesSet = true; } /**< set initial values for TC, needed by GFTrackCand */


      void setPassIndex(int anIndex) { m_passIndex = anIndex; } /**< sets pass index number containing current TC */


      void setFitSucceeded(bool yesNo) { m_fitSucceeded = yesNo; } /**< set true, if kalman fit was possible, else: false */


      void clearRivals() { m_bookingRivals.clear(); } /**< deletes entries of the rivals vector */


    protected:
      std::vector<VXDTFHit*> m_attachedHits; /**< contains pointer to all VXDTFHits attached to current TC. Currently, the first hit in the vector is the outermost hit */
      std::vector<PositionInfo*> m_attachedPositionInfos; /**< contains positionInfos (coordinates, sigmaValues) for each hit */
      std::vector<VXDSegmentCell*> m_attachedCells; /**< contains pointer to all VXDSegmentCells attached to current TC */
      std::vector<VXDTFTrackCandidate*> m_bookingRivals; /**< contains all TCs sharing hits with current one */
      std::vector<int> m_svdHitIndices;  /**< to be able to export TCs */
      std::vector<int> m_pxdHitIndices;  /**< to be able to export TCs */
      std::list<int> m_hopfieldHitIndices;  /**< slightly adapted indices for hopfield use only (no real indices, but only unique ones...) */

      bool m_overlapping;  /**< defines whether TC shares Hits with another valid TC or not */
      bool m_alive; /**< only tcs which are alive can leave the TF as final TCs */
      bool m_reserved; /**< means that hit has reserved its clusters which can not be used by other TCs any more */
      double m_qualityIndex;  /**< e.g. Chi²-Value */
      double m_qqq;  /**< QQQ = quasi quality indicator */
      double m_neuronValue;  /**< QI for hopfield network */
      double m_estRadius; /**< calculated by circleFit or another technique estimating the radius of the track circle in the x-y-plane */
      int m_pdgCode;  /**< estimated PDGCode of TC */
      int m_passIndex;  /**< TF supports several passes per event, this value is needed for some passSpecific parameters. */
      bool m_fitSucceeded; /**< returns true if kalman fit was possible and returned a result itself */
      unsigned int m_trackNumber; /**< position of the TC in the vector storing all TCs of event */

      TVector3 m_initialHit;  /**< coordinates of initial hit of TC */
      TVector3 m_initialMomentum;  /**< momentum of initial hit of TC */

      bool m_initialValuesSet; /**< if false, no initial values for position and momentum are calculated yet */

    };
  } // Tracking namespace
} //Belle2 namespace
#endif
