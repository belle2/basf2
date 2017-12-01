/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HITHIT_H
#define HITHIT_H


#include <framework/core/Module.h>
#include <TTree.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/hitXP.h>
#include <TFile.h>



namespace Belle2 {

  /**  This module from a data root file builds a tree of hitXP
  **  (see the class to know all the informations contained). The tree
  **  filled with tracks (so each "event" is a track), as vector of
  **  hitXP.
  **
  ** The module create 4 tree:
  **  - one complete (TTree_hitXP.root),
  **  - one with only tracks with at least one hit per SVD layer (TTree_hitXPSel.root)
  **  - one with only tracks with exaclty one hit per SVD layer, so no overlap
  **    are allowed (TTree_hitXPTiSel.root)
  **  - one that does not use hitXP class but uses standard classes, it is
  **    lighter and cointains less informations but is usable externally from
  **    basf2 (TTree_hitXP_ext.root)
  */

  class hitXPModule : public Module {
  public:

    hitXPModule();

    virtual ~hitXPModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.   */
    virtual void initialize() override;

    /** Called when entering a new run.     */
    virtual void beginRun() override;

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.   */
    virtual void event() override;

    /** This method is called if the current run ends. */
    virtual void endRun() override;

    /** This method is called at the end of the event processing.   */
    virtual void terminate() override;

  private:
    TTree* m_tree; /**< full output tree */
    TFile* m_outputFile; /**< output file */
    std::vector<hitXP> m_hitXP; /**< vector of the hitXP, used to fill the output tree */
    std::set<hitXP, hitXP::timeCompare> m_hitXPSet; /**< set of the hitXP, used to order tn time he hit */
    bool c_addTree; /**<parameter to produce additional tree */

    int m_trackNumber; /**< iterative number of the track in the run */
    int m_eventNumber; /**< interative number of the event in the run */
    int m_numberHitPerTrack; /**< counter of the number of the hit for each track */
    int m_trackIterator; /**< iterator used to count track (m_trackNumber) */
    int m_eventIterator; /**< iterator used to count event number (m_eventNumber) */

    TTree* m_treeSel; /**< selected output tree */
    TFile* m_outputFileSel;/**<  selected output file */
    std::vector<hitXP> m_hitXPSel; /**< selected vector of hitXP */
    int m_trackNumberSel; /**< selected iterative number of the track in the run */
    int m_eventNumberSel; /**< selected interative number of the event in the run */
    int m_numberHitPerTrackSel;/**< selected counter of the number of the hit for each track */

    TTree* m_treeTiSel; /**< tight selected output tree */
    TFile* m_outputFileTiSel; /**<  tight selected output file */
    std::vector<hitXP> m_hitXPTiSel; /**< tight selected vector of hitXP */
    int m_trackNumberTiSel; /**< tight selected iterative number of the track in the run */
    int m_eventNumberTiSel; /**< tight selected interative number of the event in the run */
    int m_numberHitPerTrackTiSel; /**< tight selected counter of the number of the hit for each track */

    TFile* m_outputFileExt; /**< external output file */
    TTree* m_treeExt; /**< external output tree */
    std::vector<double> m_EpositionEntryX; /**< exteral position at entry point, coordinate x */
    std::vector<double> m_EpositionEntryY; /**< exteral position at entry point, coordinate y */
    std::vector<double> m_EpositionEntryZ; /**< exteral position at entry point, coordinate z */
    std::vector<double> m_EmomentumEntryX; /**< exteral momentum at entry point, coordinate x */
    std::vector<double> m_EmomentumEntryY; /**< exteral momentum at entry point, coordinate x */
    std::vector<double> m_EmomentumEntryZ; /**< exteral momentum at entry point, coordinate x */
    std::vector<double> m_EpositionLocalEntryX; /**< exteral local position at entry point, coordinate x */
    std::vector<double> m_EpositionLocalEntryY; /**< exteral local position at entry point, coordinate y */
    std::vector<double> m_EpositionLocalEntryZ; /**< exteral local position at entry point, coordinate z */
    std::vector<int> m_EPDGID; /**< external PDGID */
    std::vector<double> m_Eposition0X; /**< extral position at IP, coordinate x */
    std::vector<double> m_Eposition0Y; /**< extral position at IP, coordinate y */
    std::vector<double> m_Eposition0Z; /**< extral position at IP, coordinate z */
    std::vector<double> m_Emomentum0X; /**< extral momentum at IP, coordinate x */
    std::vector<double> m_Emomentum0Y; /**< extral momentum at IP, coordinate y */
    std::vector<double> m_Emomentum0Z; /**< extral momentum at IP, coordinate z */
    std::vector<double> m_Etime; /**< external time of the hit */
    std::vector<int> m_EsensorSensor; /**< external sensor of the hit */
    std::vector<int> m_EsensorLayer; /**< external layer of the hit */
    std::vector<int> m_EsensorLadder; /**< external ladder of the hit */
    std::vector<int> m_Ereconstructed; /**< external flag of track-reconstruction or not */
    std::vector<int> m_EclusterU; /**< external flag of u-cluster */
    std::vector<int> m_EclusterV; /**< external flag of v-cluster */
    std::vector<double> m_Echarge; /**< external charge */
    int m_EtrackNumber; /**< external iterator of track in the run */
    int m_EeventNumber; /**< external iterator of event in the run */
    int m_EnumberHitPerTrack; /**< external numer of the hit for each track */
    std::vector<double> m_EomegaEntry; /**< external omega parameter at entry point */
    std::vector<double> m_Eomega0;  /**< external omega parameter at IP */
    std::vector<double> m_Ed0Entry; /**< external d0 parameter at entry point */
    std::vector<double> m_Ed00; /**< external d0 parameter at IP */
    std::vector<double> m_Ephi0Entry; /**< external phi0 parameter at entry point */
    std::vector<double> m_Ephi00; /**< external phi0 parameter at IP */
    std::vector<double> m_Ez0Entry; /**< external z0 parameter at entry point */
    std::vector<double> m_Ez00; /**< external z0 parameter at IP */
    std::vector<double> m_EtanlambdaEntry; /**< external tanlambda parameter at entry point */
    std::vector<double> m_Etanlambda0; /**< external tanlambda parameter at IP */
    int m_Eprimary; /**< external flag for primary particles */

  protected:

  };
}
#endif
