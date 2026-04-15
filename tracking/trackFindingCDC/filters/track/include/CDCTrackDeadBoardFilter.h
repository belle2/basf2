/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {

  // forward declaration
  namespace TrackingUtilities {
    class CDCTrack;
  }

  namespace TrackFindingCDC {

    /** Helper function to filter CDCTracks which have bad CDC boards where holes
       in the track are detected
       @param aCDCTrack CDC track to be checked
       @return returns true if bad board has been detected
    */
    bool cdcTrackDeadBoardFilter(const Belle2::TrackingUtilities::CDCTrack& aCDCTrack);

  }
}
