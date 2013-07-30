/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef CDCSEGMENT_H
#define CDCSEGMENT_H

#include "TObject.h"
#include <tracking/cdcConformalTracking/CDCTrackHit.h>



namespace Belle2 {

  /** Class for CDC tracklets (hits from one superlayer belonging to the same track). */
  class CDCSegment : public TObject {
  public:

    /** Empty constructor. */
    CDCSegment();

    /** Constructor with superlayer Id and the unique segment Id. */
    CDCSegment(int superlayerId, int Id);

    /** Destructor. */
    ~CDCSegment();

    /** Adds a reference(!) of a TrackHit object to the segment. */
    void addTrackHit(CDCTrackHit& aTrackHit);

    /** Adds an Id of a track candidate to which this segment may belong. */
    void setTrackCandId(const int trackId);

    /** Adds several Ids of track candidates to which this segment may belong. */
    void setTrackCandId(const std::vector<int> trackId);

    /** Clears/Erases all stored Ids of track candidates. */
    void clearTrackCandId();

    /** Sets and updates some member variables of the segment.
     * This method is executed every time a new TrackHit is added to the Segment.
     * All member variables dependent on this are updated (number of Hits, direction, inner and outermost Hits).
     */
    void update();

    /** Returns the SuperlayerId.*/
    int getSuperlayerId() const {return m_superlayerId;};

    /** Returns the Id of the Segment. */
    int getId() const {return m_Id;};

    /** Returns number of TrackHit objects in the segment. */
    int getNHits() const {return m_nHits;};

    /** Is true if the segment belongs to an axial superlayer. */
    bool getIsAxial() const {return m_isAxial;};

    /** Return a vector with Ids of track candidates to which this segment may belong.  */
    const std::vector<int>& getTrackCandId() const {return m_trackCandId;};

    /** Returns a vector with TrackHits objects in the Segment.
     * This vector contains all Hits which build this Segment.
     */
    std::vector<Belle2::CDCTrackHit>& getTrackHits() {return m_TrackHits;};

    /** Returns the direction of the segment.
     * The direction is calculated as a TVector3 of the innermost hit position relatively to the outermost hit position of the segment in the conformal plane.
     */
    TVector3 getDirection() {return m_direction;};

    /** Returns the innermost hit (the one closest to the origin) of the segment. */
    CDCTrackHit& getInnerMostHit() {return m_innerMostHit;};

    /** Returns the outermost hit (the one farthest from the origin) of the segment. */
    CDCTrackHit& getOuterMostHit() {return m_outerMostHit;};

    /** Returns the range of wires of the segment.
     *  The difference between the lowest and the highest wire Id in the segment is calculated.
     *  Some addition recalculations are performed if the segment is 'crossing' WireId = 0 to get the correct range.
     */
    int getWireIdDiff() const;

    /** Returns the average r-phi position of the segment. */
    float getCenterPosR() const;

    /** Returns the average z position of the segment. */
    float getCenterPosZ() const;


  private:

    int m_superlayerId;                    /**< ID of the superlayer of the segment (superlayer = group of 6/8 layers with the same orientation)*/
    int m_Id;                              /**< Unique Id of the segment*/

    bool m_isAxial;                        /**< Boolean to mark a segment as belonging to an axial superlayer */

    int m_nHits;                           /**< Number of hits in the segment */
    std::vector<CDCTrackHit> m_TrackHits;  /**< Vector to store TrackHits belonging to this segment*/

    TVector3 m_direction;                  /**< Direction of the Segment in conformal plane */

    CDCTrackHit m_innerMostHit;            /**< Innermost (closest to the origin) hit of the segment*/
    CDCTrackHit m_outerMostHit;            /**< Outermost (farthest to the origin) hit of the segment*/

    std::vector<int> m_trackCandId;        /**< Vector to hold the Ids of track candidates to which this segment may belong */


    //! ROOT ClassDef macro to make this class a ROOT class.
    ClassDef(CDCSegment, 1);

  }; //end class CDCSegment
} //end namespace Belle2

#endif //CDCSEGMENT


