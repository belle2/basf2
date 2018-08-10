/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <mdst/dataobjects/Track.h>

#include <TFile.h>
#include <TDirectory.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>


namespace Belle2 {
  /**
   * generates a new StoreArray from the input StoreArray which has all specified Tracks removed
   */
  class TrackFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TrackFilterModule();

    /** if required */
    virtual ~TrackFilterModule();

    /** initializes the module */
    virtual void beginRun();

    /** processes the event */
    virtual void event();

    /** end the run */
    virtual void endRun();

    /** terminates the module */
    virtual void terminate();

    /** init the module */
    virtual void initialize();

    static double m_min_d0; /**< d0 miminum value*/
    static double m_max_d0; /**< d0 maximum value*/
    static double m_min_z0; /**< z0 miminum value*/
    static double m_max_z0; /**< z0 maximum value*/
    static int m_min_NumHitsPXD; /**< miminum value of PXD hits*/
    static int m_min_NumHitsSVD; /**< miminum value of SVD hits*/
    static int m_min_NumHitsCDC; /**< miminum value of CDC hits*/
    static double m_min_pCM; /**< miminum value of the center of mass momentum*/
    static double m_min_pT; /**< miminum value of the transverse momentum*/
    static double m_min_Pval;  /**< miminum P-value of the track fit*/

    static bool m_saveControlHistos; /**< if true produces a rootfile with control histograms*/
    std::string m_rootFileName;   /**< root file name */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    TList* m_histoList_selected; /**<TList of TH1 for selected tracks*/
    TList* m_histoList_excluded; /**<TList of TH1 for excluded tracks*/

    static TH1F* m_d0_sel; /**< TH1F of d0 for selected tracks*/
    static TH1F* m_d0_exc; /**< TH1F of d0 for excluded tracks*/
    static TH1F* m_z0_sel; /**< TH1F of z0 for selected tracks*/
    static TH1F* m_z0_exc; /**< TH1F of z0 for excluded tracks*/
    static TH1F* m_nPXD_sel; /**< TH1F of # PXD hits for selected tracks*/
    static TH1F* m_nPXD_exc; /**< TH1F of # PXD hits for excluded tracks*/
    static TH1F* m_nSVD_sel; /**< TH1F of # SVD hits for selected tracks*/
    static TH1F* m_nSVD_exc; /**< TH1F of # SVD hits for excluded tracks*/
    static TH1F* m_nCDC_sel; /**< TH1F of # CDC Hits for selected tracks*/
    static TH1F* m_nCDC_exc; /**< TH1F of # CDC hits for excluded tracks*/
    static TH1F* m_pval_sel; /**< TH1F of pValue for selected tracks*/
    static TH1F* m_pval_exc; /**< TH1F of pValue for excluded tracks*/
    static TH1F* m_pCM_sel; /**< TH1F of center of mass momentum of selected tracks*/
    static TH1F* m_pCM_exc; /**< TH1F of center of mass momentum of excluded tracks*/
    static TH1F* m_pT_sel; /**< TH1F of transverse momentum of selected tracks*/
    static TH1F* m_pT_exc; /**< TH1F of transverse momentum of excluded tracks*/

  private:

    std::string m_inputArrayName;  /**< StoreArray with the input tracks */
    std::string m_outputINArrayName;  /**< StoreArray with the selected output tracks */
    std::string m_outputOUTArrayName;  /**< StoreArray with the NOT selected output tracks */

    SelectSubset<Track> m_selectedTracks; /**< selected tracks */
    SelectSubset<Track> m_notSelectedTracks; /**< not selected tracks*/

    static bool isSelected(const Track* track); /**< determine if the track satisfies the selection criteria */

    static void fillControlHistograms(const Track* track,
                                      bool isSelected); /**< determine if the track does not satisfies the selection criteria */
  };
}



