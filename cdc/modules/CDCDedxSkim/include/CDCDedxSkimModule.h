/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>

#include <vector>

namespace Belle2 {

  /** This module may be used to skim a data sample according to a
   * specific set of cuts. Cuts for certain types of events (e.g. bhabha)
   * are hard coded for calibration purposes, but simple clean up cuts
   * may be applied to any sample with tracks in order to speed up
   * subsequent use of the sample.
   */

  class CDCDedxSkimModule : public Module {

  public:

    /** Constructor, for setting module description and parameters */
    CDCDedxSkimModule();

    /** Destructor */
    virtual ~CDCDedxSkimModule();

    /** Initialize routine **/
    virtual void initialize() override;

    /** Check the event and track quality and apply clean up cuts */
    virtual void event() override;

    /** End of the event processing */
    virtual void terminate() override;

    /** A method to check whether a track passes some nominal cuts */
    bool isGoodTrack(const Track* track, const Const::ChargedStable& chargedStable);

  private:

    // required input
    StoreArray<Track> m_tracks; /**< Required array of input tracks */

    /** the event number */
    int m_eventID;
    /** the track number (for one event) */
    int m_trackID;

    /** Event type: (0) bhabha, (1) radiative bhabha, (2) two photon (e+e-),
     * (3) di-muon, (4) radiative di-muon, (5) D-decays (D*->D0 pi; D0 -> K pi) */
    std::vector<int> m_eventType;

    /** number of unmatched clusters per event */
    int m_unmatchedCluster;
    /** range for E/p per track */
    std::vector<double> m_EoverP;
    /** range for Ecc/Ecm */
    std::vector<double> m_EccOverEcm;

  };
}
