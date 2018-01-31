/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <string>

namespace Belle2 {
  template< class T >
  class RelationVector;

  /** This module uses the simulated truth information (MCParticles and their relations) to determine which
   * track candidates created separately in the CDC and VXD belong together
   *
   * It looks at all the hits in every track candidate and when 60 % of the belong to one MCParticle this track candidate will be combined
   * with another tack candidate also having 60 % of its hits coming from the same MCParticle. The 60 % ratio is not fixed and can be changed
   * by the user via the HitsRatio parameter. This module will also "clean up" the track candidates: if one TF finds more than on TC sharing more
   * most of the hits if will select the TC with most "correct" hits and ignore the others.
   * With the parameter InsertCorrectPDGCode the user can the this module to insert the correct PDC code into the TC (this options will set the correct
   * MCParticle index into the TC). TCs without a partner but at least 60 % of the hits comming from one real track will also be put on the output storearray
   */
  class MCTrackCandCombinerModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    MCTrackCandCombinerModule();

    /** Destructor of the module. */
    ~MCTrackCandCombinerModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize() override;

    /** Called when entering a new run.
     */
    void beginRun() override;

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event() override;
    /** This method is called if the current run ends.
     */
    void endRun() override;

    /** End of the event processing. */
    void terminate() override;


  private:
    /** remove hits/clusters (orignating from secondary particles - hits with negative weights)
     * from MCParticle<--> class T relation vector
     * @param relationVector: hits in this vector are checked
     * @return: vector< T* > with the good hits */
    template< class T >
    std::vector< T* > removeHitsWithNegativeWeights(RelationVector< T >& relationVector);

    /** checks if the PDG code belongs to a charged stable particle
     * @param pdgCode: this PDG code will be tested
     * @return: if the PDG code DOES NOT belong to a charged stable particle, TRUE will be returned. */
    bool isNotAChargedStable(int pdgCode);


    std::string m_cdcTrackCandColName;                          /**< CDC TrackCandidates collection name */
    std::string m_vxdTrackCandColName;                          /**< VXD TrackCandidates collection name */
    std::string m_combinedTrackCandColName;                     /**< Output TrackCandidates collection name */
    double m_hitsRatio;                                         /**< minimal ratio of hits belonging to one MCParticle to declare a track candidate coming from this MCParticle*/
    int m_nIgnoredVxdTcs;                         /**< counts the number of VXD track candidates that would not be combined with a CDC track candidate */
    int m_nIgnoredCdcTcs;                         /**< counts the number of CDC track candidates that would not be combined with a VXD track candidate */
    int m_nCdcTcs;                                /**< total number of input CDC TCs processed by the module */
    int m_nVxdTcs;                                /**< total number of input VXD TCs processed by the module */
//    int m_nGoodCdcTcs;                            /**< Number of CDC track candidates where at least a ratio of m_hitsRatio of the hits is coming from the same MCParticle*/
//    int m_nGoodVxdTcs;                            /**< Number of VXD track candidates where at least a ratio of m_hitsRatio of the hits is coming from the same MCParticle*/
    int m_nOutputTcs;                             /**< total number of output TCs created by the module */
    int m_nCdcTcsWithoutPartner;                  /**< total number of output TCs that have only hits in the CDC */
    int m_nVxdTcsWithoutPartner;                  /**< total number of output TCs that have only hits in the VXD */
    bool m_useMCpdg;                             /**< if true the correct PDG code from the MCParticle and the MCParticleID will be put in the output track candidates*/

    StoreArray<MCParticle> m_mcParticles;
  };
}
