/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDTFHIT_H
#define VXDTFHIT_H

#include <TVector3.h>
#include "SharedFunctions.h"

#include <vxd/dataobjects/VxdID.h>


namespace Belle2 {
  namespace Tracking {

    class ClusterInfo;
    /** The VXD Track Finder Hit class
    * This class stores all information needed for reconstructing track candidates within the VXDonlyTrackFinder.
    * VXDTFHits are generated individually for each pass.
    * Although this generates n times the number of hits per event (where n is the number of passes executed),
    * the total information has a low degree of redundancy since each pass has got its individual segments and TCs
    * Track candidates use the same hit, when they share the same VXDID and at least one of the clusterIndices.
    * This definition for the comparing operators allows pass-independent comparisons.
    */
    class VXDTFHit {
    public:

      /** Default constructor for root compatibility */
      VXDTFHit():
        m_hit(),
        m_passIndex(0),
        m_clusterIndexU(NULL), // SVD only
        m_clusterIndexV(NULL), // SVD only
        m_clusterIndexUV(NULL), // PXD only
        m_detectorType(-1), //  0 = PXD, 1 = SVD, -1 should not occur
        m_papaSector(),
        m_VxdID(),
        m_timeStamp(0),
        m_attachedTrackCandidates(0) {}

      /** Constructor.
      //      * @param hitPos contains Hit coordinates (global) and position errors (global).
      //      * @param passIndex index number of pass containing hit. (the VXDTF module supports several passes per event searching for different characteristics independently)
      //      * @param clusterIndexU Pointer to ClusterInfo an intermediate class storing index of SVDCluster containing U position.
      //      * @param clusterIndexV Pointer to ClusterInfo an intermediate class storing index of SVDCluster containing V position.
      //      * @param clusterIndexUV Pointer to ClusterInfo an intermediate class storing index of PXDCluster containing U and V position.
      //      * @param detectorType defining whether hit lies on PXD, SVD or IP (using e.g Const::PXD for PXD-hits).
      //      * @param papaSector address of Sector containing hit.
      //      * @param VxdID ID of sensor containing parent clusterHit(s)
      //      * @param timeStamp time of birth (only set when SVD hit, else 0).
      //      */
      VXDTFHit(PositionInfo hitPos, int passIndex, ClusterInfo* clusterIndexU, ClusterInfo* clusterIndexV, ClusterInfo* clusterIndexUV, int detectorType, unsigned int papaSector, VxdID aVxdID, float timeStamp):
        m_hit(hitPos),
        m_passIndex(passIndex),
        m_clusterIndexU(clusterIndexU), // SVD only
        m_clusterIndexV(clusterIndexV), // SVD only
        m_clusterIndexUV(clusterIndexUV),
        m_detectorType(detectorType),
        m_papaSector(papaSector),
        m_VxdID(aVxdID),
        m_timeStamp(timeStamp),
        m_attachedTrackCandidates(0) {}

      bool operator==(const VXDTFHit& b) const; /**< overloaded '=='-operator for sorting algorithms */
      bool operator<(const VXDTFHit& b) const; /**< overloaded '<'-operator for sorting algorithms */
      bool operator>(const VXDTFHit& b) const; /**< overloaded '>'-operator for sorting algorithms */

      TVector3* getHitCoordinates() { return &(m_hit.hitPosition); } /**< returns global hit coordinates */
      PositionInfo* getPositionInfo() { return &m_hit; } /**< returns global hit coordinates and errors for x and y coordinates */
      const std::vector<int>& getAttachedInnerCell() const { return m_attachedInnerCells; } /**< returns all inner Cells attached to hit */
      const std::vector<int>& getAttachedOuterCell() const { return m_attachedOuterCells; } /**< returns all outer Cells attached to hit */
      int getNumberOfSegments() { return int(m_attachedInnerCells.size() + m_attachedOuterCells.size()); } /**< returns number of segments connected to this hit (hits without attached segments are ignored during TF process) */
      int getNumberOfTrackCandidates() { return m_attachedTrackCandidates; } /**< returns number of TCs using this hit */
      int getPassIndex() const { return m_passIndex; } /**< VXDTFHits are pass-dependent. Returns the index number of the pass containing current hit */
      int getClusterIndexU() const; /*{
        if (m_clusterIndexU != NULL ) { return m_clusterIndexU->getOwnIndex(); }
        return -1;
      }*/ /**< returns index position of clusterInfo in container, only set for SVDHits */
      ClusterInfo* getClusterInfoU(); /*{ return m_clusterIndexU; }*/ /**< returns pointer to ClusterInfo U, is NULL if value is not set */
      int getClusterIndexV() const; /*{
        if (m_clusterIndexV != NULL ) { return m_clusterIndexV->getOwnIndex(); }
        return -1;
      }*/ /**< returns index position of clusterInfo in container,  only set for SVDHits */
      ClusterInfo* getClusterInfoV(); /*{ return m_clusterIndexV; }*/ /**< returns pointer to ClusterInfo V, is NULL if value is not set */
      int getClusterIndexUV() const; /*{
        if (m_clusterIndexUV != NULL ) { return m_clusterIndexUV->getOwnIndex(); }
        return -1;
      }*/ /**< returns index position of clusterInfo in container,  only set for PXDHits */
      ClusterInfo* getClusterInfoUV(); /* { return m_clusterIndexUV; } */ /**< returns pointer to ClusterInfo UV, is NULL if value is not set */
      int getDetectorType() const { return m_detectorType; } /**< returns detectorType IP=Const::IR,PXD=Const::PXD,SVD=Const::SVD */
      unsigned int getSectorName() { return m_papaSector; } /**< returns name of sectors containing current hit (sectors are passDependent), in speed optimized int */
      std::string getSectorString(); /**< returns name of sectors containing current hit (sectors are passDependent), in human readable string */
      const VxdID getVxdID() const { return m_VxdID; } /**< returns VxdID of sensor carrying current sector */
      float getTimeStamp() const { return m_timeStamp; } /**< set for SVDHits, for PXDHits it's 0 */
      bool isReserved(); /**< is true, if a valid TC is already using any Cluster attached to this hit */ /* {
        if (m_clusterIndexU != NULL) { if (m_clusterIndexU->isReserved() == true) { return true; } }
        if (m_clusterIndexV != NULL) { if (m_clusterIndexV->isReserved() == true) { return true; } }
        if (m_clusterIndexUV != NULL) { if (m_clusterIndexUV->isReserved() == true) { return true; } }
        return false;
      } */

      /** setter **/
      void addInnerCell(int newCell) { m_attachedInnerCells.push_back(newCell); } /**< adds new Cell to vector of inner Cells attached to current hit */
      void addOuterCell(int newCell) { m_attachedOuterCells.push_back(newCell); } /**< adds new Cell to vector of outer Cells attached to current hit */
      void addTrackCandidate() { m_attachedTrackCandidates++; } /**< counting them is enough to check their occupancy */
      void removeTrackCandidate() { m_attachedTrackCandidates--; } /**< decrease number of TCs using this hit */

      // isReserved
    protected:
      PositionInfo m_hit; /**< global hit position and x and y errors of hit (global)*/

      int m_passIndex; /**< index number of pass containing VXDTFhit */
      ClusterInfo* m_clusterIndexU; /**< pointer to intermediate class storing index of SVDClusterU */
      ClusterInfo* m_clusterIndexV; /**< pointer to intermediate class storing index of SVDClusterV */
      ClusterInfo* m_clusterIndexUV; /**< pointer to  intermediate class storing index of PXDCluster */
      int m_detectorType; /**< knows wheter hit is in IP=Const::IR (only virtual hit), PXD=Const::PXD or SVD=Const::SVD  */

      unsigned int m_papaSector; /**< name of sector containing hit */ // convert to int? is that faster? (needed very often)
      VxdID m_VxdID; /**< VxdID of sensor containing hit */
      float m_timeStamp; /**< only filled when available (SVD), else 0  */

      std::vector<int> m_attachedInnerCells; /**< contains links to segments/cells using this hit as outer end  */
      std::vector<int> m_attachedOuterCells; /**< contains links to segments/cells using this hit as inner end  */
      int m_attachedTrackCandidates; /**< number of trackCandidates using this hit */
    };
  } // Tracking namespace
} //Belle2 namespace
#endif
