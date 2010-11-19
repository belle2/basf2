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

    /*! Empty constructor. */
    CDCSegment();

    /*! Constructor with superlayer Id and the Id of the segment within this superlayer. */
    CDCSegment(int superlayerId, int Id);

    /*! Destructor. */
    ~CDCSegment();

    /*!Adds a TrackHit object to the segment. */
    void addTrackHit(CDCTrackHit &aTrackHit);

    /*!Sets the boolean to classify the segment. */
    void setIsGood(bool isGood);

    /*!Sets the boolean to classify the segment as used. */
    void setIsUsed(bool isUsed);

    /*!Returns the SuperlayerId.*/
    int getSuperlayerId() const {return m_superlayerId;};

    /*!Returns the Id of the Segment (to classify segments within the same superlayer).*/
    int getId() const {return m_Id;};

    /*!Returns number of TrackHit objects in the segment. */
    int getNHits() const {return m_nHits;};

    /*!Is true if the segment is classified as good. */
    bool getIsGood() const {return m_isGood;};

    /*!Is true if the segment was already assigned to a track. */
    bool getIsUsed() const {return m_isUsed;};

    /*!Is true if the segment belong to an axial superlayer. */
    bool getIsAxial() const {return m_isAxial;};

    /*! Returns a vector with TrackHits objects in the Segment. */
    /*! This vector contains all Hits which build this Segment. (The i's TrackHit can be accessed through getTrackHits()[i] or getTrackHits().at(i)). */
    std::vector<CDCTrackHit> & getTrackHits() {return m_TrackHits;};

    /*!Returns the direction of the segment. */
    /*! The direction is computed as a TVector3 of the innermost hist position relatively to the outermost hit position of the segment in the conformal plane.    */
    TVector3 getDirection() const {return m_direction;};

    /*!Returns the innermost hit (the one closest to the origin) of the segment. */
    CDCTrackHit getInnerMostHit() const {return m_innerMostHit;};

    /*!Returns the outermost hit (the one farthest from the origin) of the segment. */
    CDCTrackHit getOuterMostHit() const {return m_outerMostHit;};

    /*!Sets and updates some member variables of the segment. */
    /*! This method is executed every time a new TrackHit is added to the Segment. All member variables dependent on this are updated (number of Hits, direction, inner and outermost Hits)*/
    void update();

    /*!Searches for the best position (z coordinate) of a stereo segment and overwites the position coordinates with new values. */
    /*! This method searches for a given track and this (stereo) segment for the z-coordinate of this segment (= hits in this segment), which fits best to the given track. For this the wire length is parametrised and for each point the 'fitting' of this new position is checked by calculating the shortest (perpendicular) distance between one segment point and the track. After the optimal wire position is found, the coordinates (member variables) of all TrackHits in this segment are overwritten according to it (!). The return value is the index from the wire parametrisation, by using the same paramatrisation for another (stereo) segments can be shiftet to new positions without further calculations.*/
    int shiftAlongZ(TVector3 trackDirection, CDCTrackHit trackHit);

    /*!Overwrites the position coordinates of this stereo segment with new values according to index. */
    /*!The wire lenght is parametrised und the input value index indicates the new wire point coordinates. The coordinates (member variables) of all Hits in this segment are overwritten according to it.
    */
    void shiftAlongZ(int index);

  private:

    int m_superlayerId; /**< ID of the superlayer of the Segment (superlayer = group of 6/8 layers with the same orientation)*/
    int m_Id; /**< Id to count segments within one superlayer*/
    bool m_isAxial; /**< Boolean to mark a Segment as belonging to an axial superlayer (axial superlayers are 1,3,5,7,9)*/


    std::vector<CDCTrackHit> m_TrackHits; /**< vector to store TrackHits belonging to this segment*/
    int m_nHits;  /**<Number of Hits in the Segment */

    TVector3 m_direction; /**<Direction of the Segment in conformal plane */
    CDCTrackHit m_innerMostHit; /**<Innermost (closest to the origin) Hit of the Segment*/
    CDCTrackHit m_outerMostHit; /**<Outermost (farthest to the origin) Hit of the Segment*/

    bool m_isGood; /**< classifies Segment as 'good' or 'bad', only 'good' Segments are used in the first step of track reconstruction*/
    bool m_isUsed; /**<Boolean to mark a Segment as already used for reconstruction of a track candidate*/

    //! ROOT ClassDef macro to make this class a ROOT class.
    ClassDef(CDCSegment, 1);

  }; //end class CDCSegment
} //end namespace Belle2

#endif //CDCSEGMENT


