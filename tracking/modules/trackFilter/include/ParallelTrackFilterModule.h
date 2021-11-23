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
   * Generates a new StoreArray from the input StoreArray which has all specified Tracks removed.
   * Like TrackFilterModule, but this one does not produce NTuples and is ParallelProcessingCertified.
   */
  class ParallelTrackFilterModule : public Module {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module. */
    ParallelTrackFilterModule();

    /** processes the event */
    virtual void event() override;

    /** init the module */
    virtual void initialize() override;

    /** called at beginning of run */
    virtual void beginRun() override;

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

  private:

    void initializeSelectSubset();

    std::string m_inputArrayName;  /**< StoreArray with the input tracks */
    std::string m_outputINArrayName;  /**< StoreArray with the selected output tracks */
    std::string m_outputOUTArrayName;  /**< StoreArray with the NOT selected output tracks */

    SelectSubset<Track> m_selectedTracks; /**< selected tracks */
    SelectSubset<Track> m_notSelectedTracks; /**< not selected tracks*/

    static bool isSelected(const Track* track); /**< determine if the track satisfies the selection criteria */

  };
}
