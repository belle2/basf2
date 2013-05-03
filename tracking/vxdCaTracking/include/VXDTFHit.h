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

#include <vxd/dataobjects/VxdID.h>


namespace Belle2 {

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
      m_clusterIndexU(0), // SVD only
      m_clusterIndexV(0), // SVD only
      m_clusterIndexUV(0), // PXD only
      m_detectorType(-1), //  0 = PXD, 1 = SVD, -1 should not occur
      m_papaSector(),
      m_VxdID(),
      m_timeStamp(0) { m_attachedTrackCandidates = 0;/* m_attachedCells = 0;*/ }

    /** Constructor.
    //      * @param hitPos Hit coordinates (global).
    //      * @param passIndex index number of pass containing hit. (the VXDTF module supports several passes per event searching for different characteristics independently)
    //      * @param clusterIndexU index number of intermediate class storing index of SVDCluster containing U position.
    //      * @param clusterIndexV index number of intermediate class storing index of SVDCluster containing V position.
    //      * @param clusterIndexUV index number of intermediate class storing index of PXDCluster containing U and V position.
    //      * @param detectorType defining whether hit lies on PXD, SVD or IP.
    //      * @param papaSector address of Sector containing hit.
    //      * @param VxdID ID of sensor containing parent clusterHit(s)
    //      * @param timeStamp time of birth (only set when SVD hit, else 0).
    //      */
    VXDTFHit(TVector3 hitPos, int passIndex, int clusterIndexU, int clusterIndexV, int clusterIndexUV, int detectorType, unsigned int papaSector, VxdID aVxdID, float timeStamp);

    bool operator==(const VXDTFHit& b) const; /**< overloaded '=='-operator for sorting algorithms */
    bool operator<(const VXDTFHit& b) const; /**< overloaded '<'-operator for sorting algorithms */
    bool operator>(const VXDTFHit& b) const; /**< overloaded '>'-operator for sorting algorithms */

    TVector3 getHitCoordinates() const; /**< returns global hit coordinates */
    const std::vector<int>& getAttachedInnerCell() const; /**< returns all inner Cells attached to hit */
    const std::vector<int>& getAttachedOuterCell() const; /**< returns all outer Cells attached to hit */
    int getNumberOfSegments(); /**< returns number of segments connected to this hit (hits without attached segments are ignored during TF process) */
    int getNumberOfTrackCandidates() { return m_attachedTrackCandidates; } /**< returns number of TCs using this hit */
    int getPassIndex() const { return m_passIndex; } /**< VXDTFHits are pass-dependent. Returns the index number of the pass containing current hit */
    int getClusterIndexU() const { return m_clusterIndexU; } /**< only set for SVDHits */
    int getClusterIndexV() const { return m_clusterIndexV; } /**< only set for SVDHits */
    int getClusterIndexUV() const { return m_clusterIndexUV; } /**< only set for PXDHits */
    int getDetectorType() const { return m_detectorType; } /**< returns detectorType IP=-1,PXD=0,SVD=1 */
    unsigned int getSectorName() { return m_papaSector; } /**< returns name of sectors containing current hit (sectors are passDependent), in speed optimized int */
    std::string getSectorString(); /**< returns name of sectors containing current hit (sectors are passDependent), in human readable string */
    const VxdID getVxdID() const { return m_VxdID; } /**< returns VxdID of sensor carrying current sector */
    float getTimeStamp() const { return m_timeStamp; } /**< set for SVDHits, for PXDHits it's 0 */

    /** setter **/
    void addInnerCell(int newCell); /**< adds new Cell to vector of inner Cells attached to current hit */
    void addOuterCell(int newCell); /**< adds new Cell to vector of outer Cells attached to current hit */
    void addTrackCandidate() { m_attachedTrackCandidates++; } /**< counting them is enough to check their occupancy */
    void removeTrackCandidate() { m_attachedTrackCandidates--; } /**< decrease number of TCs using this hit */


  protected:
    TVector3 m_hit; /**< global hit position */

    int m_passIndex; /**< index number of pass containing VXDTFhit */
    int m_clusterIndexU; /**< index number of intermediate class storing index of SVDClusterU */
    int m_clusterIndexV; /**< index number of intermediate class storing index of SVDClusterV */
    int m_clusterIndexUV; /**< index number of intermediate class storing index of PXDCluster */
    int m_detectorType; /**< knows wheter hit is in PXD or SVD PXD = 0, SVD = 1, IP = -1 */

    unsigned int m_papaSector; /**< name of sector containing hit */ // convert to int? is that faster? (needed very often)
    VxdID m_VxdID; /**< VxdID of sensor containing hit */
    float m_timeStamp; /**< only filled when available (SVD), else 0  */

    std::vector<int> m_attachedInnerCells; /**< contains links to segments/cells using this hit as outer end  */
    std::vector<int> m_attachedOuterCells; /**< contains links to segments/cells using this hit as inner end  */
    int m_attachedTrackCandidates; /**< number of trackCandidates using this hit */
  };

} //Belle2 namespace
#endif
