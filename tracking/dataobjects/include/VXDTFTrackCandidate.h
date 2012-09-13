/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/// NOTE:These Classes are needed by dataobjects, but shall not be compatible to the datastore for performance reasons, therefore only implemented as .h-file (compare to VXDID)
#ifndef VXDTFTRACKCANDIDATE_H
#define VXDTFTRACKCANDIDATE_H



#include <tracking/dataobjects/VXDTFHit.h>
#include <tracking/dataobjects/VXDSegmentCell.h>

#include <list>
#include <string>
#include <vector>

#include <TVector3.h>
#include <TObject.h>

namespace Belle2 {

//  class VXDTFHit;
//
//  class VXDSegmentCell;

  /** \addtogroup dataobjects
   * @{
   */

  /** The VXD Track Finder Track Candidate class
   * This class stores all information needed for reconstructing track candidates within the VXDonlyTrackFinder.
   * It's for internal and analysis use only and will be exported as a GFTrackCandidate which is the official
   * interface for track reconstruction modules.
   */
  class VXDTFTrackCandidate: public TObject {
  public:
    /** Default constructor for the ROOT IO. */
    VXDTFTrackCandidate() { m_overlapping = false; m_alive = true; m_qualityIndex = 1.0; }

    /**copy constructor**/
    VXDTFTrackCandidate(VXDTFTrackCandidate*& other) :
      m_attachedHits((*other).m_attachedHits),
      m_attachedCells((*other).m_attachedCells),
      m_svdHitIndices((*other).m_svdHitIndices),
      m_pxdHitIndices((*other).m_pxdHitIndices),
      m_hopfieldHitIndices((*other).m_hopfieldHitIndices),
      m_overlapping((*other).m_overlapping),
      m_alive((*other).m_alive),
      m_qualityIndex((*other).m_qualityIndex) { /*m_neuronValue = 0; m_overlapping = false; m_alive = true; m_qualityIndex = 1.0;*/ }

    /** getter **/
    std::vector<Belle2::VXDSegmentCell*> getSegments(); /**< returns segments forming current TC */
    std::vector<Belle2::VXDTFHit*> getHits(); /**< returns hits forming current TC */
    std::vector<int> getSVDHitIndices() { return m_svdHitIndices; } /**< returns indices of svdClusters forming current TC */
    std::vector<int> getPXDHitIndices() { return m_pxdHitIndices; } /**< returns indices of pxdClusters forming current TC */
    std::list<int> getHopfieldHitIndices() { return m_hopfieldHitIndices; } /**< returns slightly adapted indices for hopfield use only (no real indices, but only unique ones...) */
    bool getOverlappingState() const { return m_overlapping; } /**< returns flag whether TC is sharing hits with other TCs or not */
    bool getCondition() const { return m_alive; } /**< returns flag whether TC is still "alive" (part of the set of TCs which are probably real tracks based on the knowledge of the TF at the point of calling that function) */
    double getTrackQuality() { return m_qualityIndex; } /**< returns quality index of TC, has to be between 0 (bad) and 1 (perfect) */
    double getQQQ() { return m_qqq; } /**< returns aditional quality index */
    float getNeuronValue() const { return m_neuronValue; } /**< returns state of neuron during hopfield network */
    TVector3 getInitialCoordinates() const { return m_initialHit; } /**< returns initial coordinates needed for export as GFTrackCandid */
    TVector3 getInitialMomentum() const { return m_initialMomentum; } /**< returns initial momentum needed for export as GFTrackCandid */
    int getPDGCode() const { return m_pdgCode; } /**< returns estimated PDGCode (its always a pion, but charge changes depending of the sign of the curvature of the track) needed for export as GFTrackCandid */
    int getPassIndex() { return m_passIndex; } /**< TCs are passDependent and are merged at the end of the TF-process to be filtered there */

    /** setter **/
    void addSVDClusterIndex(int anIndex); /**< add index number of SVDCluster attached to current TC */
    void addPXDClusterIndex(int anIndex); /**< add index number of PXDCluster attached to current TC */
    void addHopfieldClusterIndex(int anIndex); /**< add index number of Cluster attached to current TC (SVD and PXD), index is unique but does not point to real clusters */
    void addSegments(VXDSegmentCell* pCell); /**< add segment attached to current TC */
    void addHits(VXDTFHit* pHit); /**< add hit attached to current TC */
    void setOverlappingState(bool newState); /**< set whether current TC is overlapped or not */
    void setTrackQuality(double newVal); /**< set estimated quality of TC */
    void setQQQ(double qqqScore, double maxScore); /**< set estimated extended quality of TC (a potential minimal replacement for kalman filter, interesting for online-use) */
    void setCondition(bool newCondition); /**< set condition. If true, TC is part of set of final TCs which are exported for further use */
    void setNeuronValue(float aValue);  /**< set neuron value, needed by the hopfield network */
    void removeVirtualHit(); /**< removes virtual hit which is needed for most filtering steps */
    void setInitialValue(TVector3 aHit, TVector3 pVector, int pdg); /**< set initial values for TC, needed by GFTrackCand */
    void setPassIndex(int anIndex); /**< sets pass index number containing current TC */


  protected:
    std::vector<Belle2::VXDTFHit*> m_attachedHits; /**< contains pointer to all VXDTFHits attached to current TC */
    std::vector<Belle2::VXDSegmentCell*> m_attachedCells; /**< contains pointer to all VXDSegmentCells attached to current TC */
    std::vector<int> m_svdHitIndices;  /**< to be able to export TCs */
    std::vector<int> m_pxdHitIndices;  /**< to be able to export TCs */
    std::list<int> m_hopfieldHitIndices;  /**< slightly adapted indices for hopfield use only (no real indices, but only unique ones...) */

    bool m_overlapping;  /**< defines whether TC shares Hits with another valid TC or not */
    bool m_alive; /**< only tcs which are alive can leave the TF as final TCs */
    double m_qualityIndex;  /**< e.g. Chi²-Value */
    double m_qqq;  /**< QQQ = quasi quality indicator */
    float m_neuronValue;  /**< QI for hopfield network */
    int m_pdgCode;  /**< estimated PDGCode of TC */
    int m_passIndex;  /**< TF supports several passes per event, this value is needed for some passSpecific parameters. */

    TVector3 m_initialHit;  /**< coordinates of initial hit of TC */
    TVector3 m_initialMomentum;  /**< momentum of initial hit of TC */

    ClassDef(VXDTFTrackCandidate, 2)

  };

  /** @}*/
} //Belle2 namespace
#endif
