/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <algorithm>

namespace Belle2 {
  /** Creates a vector of vectors, that knows which track is conflicting with which other. */
  class OverlapMatrixCreator {
  public:
    /** Constructor taking information necessary to perform algorithm.
     *
     * The index of the first vector of hitRelatedTracks is the hit index.
     * The vector at that place holds all tracks, that are connected to that index.
     * m_overlapMatrix is the output and needs to know for how many tracks overlaps need to be saved,
     * hence the second parameter.
     */
    OverlapMatrixCreator(std::vector <std::vector <unsigned short> > const& hitRelatedTracks,
                         unsigned short nSpacePointTrackCandidates) :
      m_hitRelatedTracks(hitRelatedTracks), m_overlapMatrix(nSpacePointTrackCandidates)
    {}

    /** Fills and returns the overlap matrix.
     *
     *  It is fairly easily possible to extend the algorithm to allow a number of overlaps
     *  without telling the tracks to be in conflict.
     */
    std::vector <std::vector<unsigned short> > getOverlapMatrix(unsigned allowedOverlaps = 0)
    {
      //Loop over all the hits and make corresponding connections for the tracks
      //This yields unordered indices of tracks, one for each shared hit.
      for (auto && tracks : m_hitRelatedTracks) {
        for (unsigned short ii = 0; ii < tracks.size(); ii++) {
          for (unsigned short jj = ii + 1; jj < tracks.size(); jj++) {
            m_overlapMatrix[tracks[ii]].push_back(tracks[jj]);
            m_overlapMatrix[tracks[jj]].push_back(tracks[ii]);
          }
        }
      }

      //If it makes sense to have this can be explored by Jonas and Felix...
      //... so let's start with something that works, not necessarily what is fastest.
      //This is probably slower then the version below for 0 overlaps, but shall work with
      //any number of allowed overlaps.
      if (allowedOverlaps) {
        std::vector <unsigned short> overlapChache;

        for (auto && overlapTracks : m_overlapMatrix) {
          std::sort(overlapTracks.begin(), overlapTracks.end());

          auto endIter    = overlapTracks.end();
          auto cacheIter  = overlapTracks.begin();
          for (auto iter = overlapTracks.begin(); iter != endIter; iter++) {
            if (*iter != *cacheIter) {
              cacheIter = iter;
            } else if (iter - cacheIter == allowedOverlaps) {
              overlapChache.push_back(*iter);
            }
          }
          overlapTracks = overlapChache;
          overlapChache.clear();
        }
        return m_overlapMatrix;
      }

      //sort and erase duplicate overlaps
      //TODO: Check in realistic situation alternative approach:
      //see http://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
      for (auto && overlapTracks : m_overlapMatrix) {
        std::sort(overlapTracks.begin(), overlapTracks.end());
        overlapTracks.erase(std::unique(overlapTracks.begin(), overlapTracks.end()), overlapTracks.end());
      }

      return m_overlapMatrix;
    }

  private:
    std::vector<std::vector <unsigned short> >const& m_hitRelatedTracks;///<Input information, see constructor.
    std::vector<std::vector <unsigned short> >        m_overlapMatrix;  ///<Output information, see getOverlapMatrix.
  };
}
