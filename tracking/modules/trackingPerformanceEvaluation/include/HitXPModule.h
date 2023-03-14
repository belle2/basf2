/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/hitXP.h>
#include <TTree.h>
#include <TFile.h>

#include <set>

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

  class HitXPModule : public Module {
  public:

    HitXPModule();

    ~HitXPModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.   */
    void initialize() override;

    /** Called when entering a new run.     */
    void beginRun() override;

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.   */
    void event() override;

    /** This method is called if the current run ends. */
    void endRun() override;

    /** This method is called at the end of the event processing.   */
    void terminate() override;

  private:
    TTree* m_tree = nullptr; /**< full output tree */
    TFile* m_outputFile = nullptr; /**< output file */
    std::vector<hitXP> m_hitXP; /**< vector of the hitXP, used to fill the output tree */
    std::set<hitXP, hitXP::timeCompare> m_hitXPSet; /**< set of the hitXP, used to order tn time he hit */
    bool c_addTree; /**<parameter to produce additional tree */

    int m_trackNumber = 0; /**< iterative number of the track in the run */
    int m_eventNumber = 0; /**< interative number of the event in the run */
    int m_numberHitPerTrack = 0; /**< counter of the number of the hit for each track */
    int m_trackIterator = 0; /**< iterator used to count track (m_trackNumber) */
    int m_eventIterator = 0; /**< iterator used to count event number (m_eventNumber) */

    TTree* m_treeSel = nullptr; /**< selected output tree */
    TFile* m_outputFileSel = nullptr;/**<  selected output file */
    std::vector<hitXP> m_hitXPSel; /**< selected vector of hitXP */
    int m_trackNumberSel = 0; /**< selected iterative number of the track in the run */
    int m_eventNumberSel = 0; /**< selected interative number of the event in the run */
    int m_numberHitPerTrackSel = 0;/**< selected counter of the number of the hit for each track */

    TTree* m_treeTiSel = nullptr; /**< tight selected output tree */
    TFile* m_outputFileTiSel = nullptr; /**<  tight selected output file */
    std::vector<hitXP> m_hitXPTiSel; /**< tight selected vector of hitXP */
    int m_trackNumberTiSel = 0; /**< tight selected iterative number of the track in the run */
    int m_eventNumberTiSel = 0; /**< tight selected interative number of the event in the run */
    int m_numberHitPerTrackTiSel = 0; /**< tight selected counter of the number of the hit for each track */

    TFile* m_outputFileExt = nullptr; /**< external output file */
    TTree* m_treeExt = nullptr; /**< external output tree */
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
    int m_EtrackNumber = 0; /**< external iterator of track in the run */
    int m_EeventNumber = 0; /**< external iterator of event in the run */
    int m_EnumberHitPerTrack = 0; /**< external numer of the hit for each track */
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
    int m_Eprimary = 0; /**< external flag for primary particles */

    StoreArray<MCParticle> m_MCParticles;   /**< MCParticles StoreArray */

  protected:

  };
}
