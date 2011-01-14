/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#ifndef CDCTRACK_H
#define CDCTRACK_H

#include "TObject.h"
#include <tracking/karlsruhe/CDCTrackHit.h>
#include <tracking/karlsruhe/CDCSegment.h>



namespace Belle2 {

  class CDCTrack : public TObject {
  public:

    /** Empty constructor. */
    CDCTrack();

    /** Constructor.
     * Creates a new track with an Id.
     */
    CDCTrack(int Id);

    /** Copy Constructor.
     * Creates a new track from a copy of another track and a new Id.
     */
    CDCTrack(CDCTrack &candidate, int Id);

    /** Destructor. */
    ~CDCTrack();

    /** Adds a segment object to the track.
     *  All TrackHit objects in the segment are also added to the m_TrackHits.
     */
    void addSegment(CDCSegment aSegment);

    /** Adds a TrackHit object to the track. */
    void addTrackHit(CDCTrackHit aTrackHit);

    /** Sets a Chi2 value to describe the quality of the Track. */
    void setChiSquare(double chi2);

    /** Sets an absolute value of the momentum of the Track. */
    void setMomentumValue(double momentum);

    /** Sets and updates some member variables of the track.
     * This method is executed every time a new TrackHit or a Segment is added to the Track.
     * All member variables dependent on this are updated (number of Segments/Hits, direction, inner and outermost Segment/Hits).
     */
    void update();

    /** Returns the Id of the Track. */
    int getId() const {return m_Id;};

    /** Returns number of segments in the Track. */
    int getNSegments() const {return m_nSegments;};

    /** Returns the total number hits in the Track. */
    int getNHits() const {return m_nHits;};

    /** Returns the direction of the track in the conformal plane.
     * This direction is the sum of segment`s directions.
     */
    TVector3 getDirection() const {return m_direction;};

    /** Returns a vector with all segments in the Track. */
    std::vector<CDCSegment> & getSegments() {return m_Segments;};

    /** Returns a vector with all hits in the Track. */
    std::vector<CDCTrackHit> getTrackHits() const {return m_TrackHits;};

    /** Returns the innermost axial hit (closest to the origin) of the track. */
    CDCTrackHit getInnerMostHit() const {return m_innerMostHit;};

    /** Returns the outermost axial hit (farthest from the origin) of the track. */
    CDCTrackHit getOuterMostHit() const {return m_outerMostHit;};

    /** Returns the innermost segment (closest to the origin) of the track. */
    CDCSegment getInnerMostSegment() const {return m_innerMostSegment;};

    /** Returns the outermost segment (farthest to the origin) of the track. */
    CDCSegment getOuterMostSegment() const {return m_outerMostSegment;};

    /** Returns a Chi2 value of the track. */
    double getChiSquare() const {return m_chi2;};

    /** Returns the absolute value of the momentum of the Track. */
    double getMomentumValue() {return m_momentumValue;}



  private:

    int m_Id; /**< Id of the track candidate*/
    int m_nSegments; /**< Number of segments in the Track*/
    int m_nHits;  /**< Number of hits in the Track */

    std::vector<CDCSegment> m_Segments;  /**< vector to store segments belonging to this Track */
    std::vector<CDCTrackHit> m_TrackHits;  /**< vector to store TrackHits belonging to this Track */

    TVector3 m_direction; /**< Direction of the Track in the conformal plane */

    CDCTrackHit m_innerMostHit; /**< Innermost (closest to the origin) hit of the Track */
    CDCTrackHit m_outerMostHit; /**< Outermost (farthest to the origin) hit of the Track */
    CDCSegment m_innerMostSegment; /**< Innermost (closest to the origin) segment of the Track */
    CDCSegment m_outerMostSegment; /**< Outermost (farthest to the origin) segment of the Track */

    double m_chi2; /**< Chi2 value to describe the quality of the Track */

    double m_momentumValue;  /**< Absolut momentum value of the Track */

    //! ROOT ClassDef macro to make this class a ROOT class.
    ClassDef(CDCTrack, 1);

  }; //end class CDCTrack
} //end namespace Belle2

#endif //CDCTRACK


