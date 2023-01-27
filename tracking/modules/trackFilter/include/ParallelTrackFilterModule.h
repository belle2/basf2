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
#include <string>


namespace Belle2 {
  /**
   * Generates a new StoreArray from the input StoreArray which contains only tracks that meet the specified criteria.
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


  private:

    bool isSelected(const Track* track); /**< determine if the track satisfies the selection criteria */

    std::string m_inputArrayName = "Tracks";  /**< StoreArray with the input tracks */
    std::string m_outputINArrayName = "TracksIN";  /**< StoreArray with the selected output tracks */
    std::string m_outputOUTArrayName = "TracksOUT";  /**< StoreArray with the NOT selected output tracks */

    SelectSubset<Track> m_selectedTracks; /**< selected tracks */
    SelectSubset<Track> m_notSelectedTracks; /**< not selected tracks*/

    double m_minD0 = -100; /**< d0 miminum value*/
    double m_maxD0 = 100; /**< d0 maximum value*/
    double m_minZ0 = -500; /**< z0 miminum value*/
    double m_maxZ0 = 500; /**< z0 maximum value*/
    int m_minNumHitsPXD = 0; /**< miminum value of PXD hits*/
    int m_minNumHitsSVD = 0; /**< miminum value of SVD hits*/
    int m_minNumHitsCDC = 0; /**< miminum value of CDC hits*/
    double m_minPCM = 0; /**< miminum value of the center of mass momentum*/
    double m_minPT = 0; /**< miminum value of the transverse momentum*/
    double m_minPval = 0;  /**< miminum P-value of the track fit*/
  };
}
