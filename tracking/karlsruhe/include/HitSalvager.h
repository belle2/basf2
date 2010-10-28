/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#ifndef HITSALVAGER_H
#define HITSALVAGER_H

#include <tracking/karlsruhe/CDCTrackHit.h>
#include <tracking/karlsruhe/CDCSegment.h>
#include <tracking/karlsruhe/CDCTrack.h>


namespace Belle2 {

  class HitSalvager {
  public:

    /*! Constructor. */
    HitSalvager();

    /*! Destructor.*/
    ~HitSalvager();

    /*!Calculates the shortest distance between one Hit and the straight line build through the outer most point of the Track and his direction in the conformal plane. */
    /*!Performs some simple geometrical calculations in the conformal plane (where segments and tracks are assumed to form straight lines). A perpendicular (to the track direction) line through the Hit is formed, then the intersection point between this line and a line build through the outer most point of the Track and his direction is calculated. The distance between the hit point and this intersection point is returned.
     */
    static double ShortestDistance(CDCTrack track, CDCTrackHit hit);

    static bool CheckSuperlayer(CDCTrack track, CDCTrackHit hit);

    /*!Goes through unused Hits and adds matching Hit to already existing Tracks.
     */
    /*!This algorithm goes through unused Hits in the SegmentsCDCArray (mostly 'bad' Segments) and calculates the shortest distance between the Hit and the alredy existing Tracks from TracksCDCArray. If exactly one calculated distance is below the maxDistance cut, the hit is added to the corresponding track and removed from further search (marked as used). If more than one Track is below the cut, the hit is assigned to none of them (to avoid wrong assigments of track crossing hits).
      */

    static void SalvageHits(std::string SegmentsCDCArray, std::string TracksCDCArray, double maxDistance);


  private:


  }; //end class HitSalvager
} //end namespace Belle2

#endif //HITSALVAGER

