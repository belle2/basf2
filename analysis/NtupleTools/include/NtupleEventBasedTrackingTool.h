/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2017 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Sam Cunliffe, Martin Heck                                *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>

namespace Belle2 {
  /** NtupleTool to write event based tracking quantities to a flat ntuple */
  class NtupleEventBasedTrackingTool : public NtupleFlatTool {
  private:
    int m_nExtraCDCHits;             /**< number of CDC hits in the event not assigned to any track */
    int m_nExtraCDCHitsPostCleaning; /**< number of CDC hits in the event not assigned to any track nor very likely beam background*/
    int m_nExtraCDCSegments;         /**< number of segments that couldn't be assigned to any track */
    int m_nExtraVXDHits;             /**< number of VXD hits not assigned to any track */
    double m_svdFirstSampleTime;     /**< time of first SVD sample relatvie to event T0 */
    bool m_trackFindingFailureFlag;  /**< flag set by the tracking if there is reason to assume there was a missed track in the event */
    void setupTree();                /**< declare branches */
  public:
    /** constuctor */
    NtupleEventBasedTrackingTool(TTree* tree, DecayDescriptor& decaydescriptor)
      : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** evaluate the variables */
    void eval(const Particle* p);
  };
} // namepspace Belle2
