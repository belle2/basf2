/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/


#ifndef CDCSEGMENT_H
#define CDCSEGMENT_H

#include "TObject.h"

#include <tracking/karlsruhe/CDCTrackHit.h>

//#include <vector>


namespace Belle2 {

  class CDCSegment : public TObject {
  public:

    /** Empty constructor. */
    CDCSegment();

    /** Constructor with superlayer Id and the unique segment Id. */
    CDCSegment(int superlayerId, int Id);

    /** Destructor. */
    ~CDCSegment();

    /** Adds a reference of a TrackHit object to the segment. */
    void addTrackHit(CDCTrackHit & aTrackHit);

    /** Sets the boolean to classify the segment. */
    void setIsGood(bool isGood);

    /** Sets the boolean to classify the segment as used. */
    void setIsUsed(bool isUsed);

    /** Adds an Id of a track candidate to which this segment may belong. */
    void setTrackCandId(int trackId);

    /** Adds several Ids of track candidates to which this segment may belong. */
    void setTrackCandId(std::vector<int> trackId);

    /** Returns the SuperlayerId.*/
    int getSuperlayerId() const {return m_superlayerId;};

    /** Returns the Id of the Segment. */
    int getId() const {return m_Id;};

    /*!Returns number of TrackHit objects in the segment. */
    int getNHits() const {return m_nHits;};

    /** Is true if the segment is classified as good. */
    bool getIsGood() const {return m_isGood;};

    /** Is true if the segment was already assigned to a track. */
    bool getIsUsed()  {return m_isUsed;};

    /** Is true if the segment belongs to an axial superlayer. */
    bool getIsAxial() const {return m_isAxial;};

    /** Return a vector with Ids of track candidates to which this segment may belong.  */
    std::vector<int>getTrackCandId() {return m_trackCandId;};

    /** Returns a vector with TrackHits objects in the Segment.
     * This vector contains all Hits which build this Segment.
     *  (The i's TrackHit can be accessed through getTrackHits()[i] or getTrackHits().at(i)).
     *
     */
    std::vector<CDCTrackHit> & getTrackHits() {return m_TrackHits;};

    /** Returns the direction of the segment.
     * The direction is calculated as a TVector3 of the innermost hit position relatively to the outermost hit position of the segment in the conformal plane.
     */
    TVector3 getDirection() const {return m_direction;};

    /** Returns the innermost hit (the one closest to the origin) of the segment. */
    CDCTrackHit getInnerMostHit() const {return m_innerMostHit;};

    /** Returns the outermost hit (the one farthest from the origin) of the segment. */
    CDCTrackHit getOuterMostHit() const {return m_outerMostHit;};

    /** Sets and updates some member variables of the segment.
     * This method is executed every time a new TrackHit is added to the Segment.
     * All member variables dependent on this are updated (number of Hits, direction, inner and outermost Hits).
     */
    void update();

    /** Returns the range of wires of the segment.
     *  The difference between the lowest and the highest wire Id in the segment is calculated.
     *  Some addition recalculations are performed if the segment is 'crossing' WireId = 0 to get the correct range.
     */
    int getWireIdDiff();

    /** Clears/Erases all stored Ids of track candidates. */
    void clearTrackCandId();

    /** Searches for the best position (z coordinate) of a stereo segment and overwrites the position coordinates with new values.
     * This method searches for a given track and this (stereo) segment for the z-coordinate of this segment (= hits in this segment), which fits best to the given track.
     * For this the wire length is parametrised and for each point the 'fitting' of this new position is checked by calculating the shortest (perpendicular) distance between one segment point and the track.
     * After the optimal wire position is found, the coordinates (member variables) of all TrackHits in this segment are overwritten according to it (!).
     * The return value is the index from the wire parametrisation, by using the same parametrisation for another (stereo) segments can be shiftet to new positions without further calculations.
     */
    int shiftAlongZ(TVector3 trackDirection, CDCTrackHit trackHit);

    /** Overwrites the position coordinates of this stereo segment with new values according to index.
     * The wire lenght is parametrised und the input value index indicates the new wire point coordinates.
     * The coordinates (member variables) of all Hits in this segment are overwritten according to it.
     */
    void shiftAlongZ(int index);

  private:

    int m_superlayerId; /**< ID of the superlayer of the segment (superlayer = group of 6/8 layers with the same orientation)*/
    int m_Id; /**< Unique Id of the segment*/
    bool m_isAxial; /**< Boolean to mark a segment as belonging to an axial superlayer (axial superlayers are 1,3,5,7,9)*/


    std::vector<CDCTrackHit> m_TrackHits; /**< vector to store TrackHits belonging to this segment*/
    int m_nHits;  /**<Number of hits in the segment */

    TVector3 m_direction; /**<Direction of the Ssgment in conformal plane */
    CDCTrackHit m_innerMostHit; /**<Innermost (closest to the origin) hit of the segment*/
    CDCTrackHit m_outerMostHit; /**<Outermost (farthest to the origin) hit of the segment*/

    bool m_isGood; /**< classifies segment as 'good' or 'bad', only 'good' Segments are used in the first step of track reconstruction*/
    bool m_isUsed; /**<Boolean to mark a segment as already used for reconstruction of a track candidate*/

    std::vector<int> m_trackCandId; /**< Vector to hold the Ids of track candidates to which this segment may belong */

    //! ROOT ClassDef macro to make this class a ROOT class.
    ClassDef(CDCSegment, 1);

  }; //end class CDCSegment
} //end namespace Belle2

#endif //CDCSEGMENT


