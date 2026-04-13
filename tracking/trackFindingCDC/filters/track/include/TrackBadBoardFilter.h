/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackingUtilities {

    class CDCTrack;

    // helper function to filter CDCTracks which have bad CDC boards where holes
    // in the track are detected
    bool trackBadBoardFilter(const TrackingUtilities::CDCTrack& aCDCTrack);

  }
}
