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


    /** Helper function that determines position on the globalHelix where the CDC layer iclayer is crossed, and adds corresponding
        boardID of the CDC board at that position to output vector.
        Per call three boardIDs of CDC boards are added to the boardCands vector. One at the actual crossing and other two left and right of it.
        @param boardCands : reference to vector with bad board candidates. Function will add to it.
        @param globalHelix : helix of a track in global coordinates
        @param iclayer : continuous CDC sense wire layer (0..56)
        @param geometryPar : reference to the CDC geometry needed to determine the board number.
    */
    void addBoardCandsAtLayer(std::vector<unsigned int>& boardCands, const Belle2::TrackingUtilities::Helix& globalHelix,
                              Belle2::CDC::ILayer iclayer, const CDC::CDCGeometryPar& geometryPar);
  }
}
