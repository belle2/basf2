/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <mdst/dataobjects/Track.h>

#include <TFile.h>
#include <TNtuple.h>


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


    /** processes the event */
    virtual void event() override;

    /** terminates the module */
    virtual void terminate() override;

    /** init the module */
    virtual void initialize() override;

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

    static bool m_saveControlNtuples; /**< if true produces a rootfile with control ntupled*/
    std::string m_rootFileName;   /**< root file name */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing ntuples */

  private:

    std::string m_inputArrayName;  /**< StoreArray with the input tracks */
    std::string m_outputINArrayName;  /**< StoreArray with the selected output tracks */
    std::string m_outputOUTArrayName;  /**< StoreArray with the NOT selected output tracks */

    SelectSubset<Track> m_selectedTracks; /**< selected tracks */
    SelectSubset<Track> m_notSelectedTracks; /**< not selected tracks*/

    static bool isSelected(const Track* track); /**< determine if the track satisfies the selection criteria */

    static void fillControlNtuples(const Track* track,
                                   bool isSelected); /**< determine if the track does not satisfies the selection criteria */

    static TNtuple* m_selectedNtpl; /**< tuple of selected tracks */
    static TNtuple* m_rejectedNtpl; /**< tuple of rejected tracks */
  };
}



