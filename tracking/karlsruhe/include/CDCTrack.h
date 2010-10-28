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
#include <tracking/karlsruhe/HitCDC.h>
#include <tracking/karlsruhe/CDCTrackHit.h>
#include <tracking/karlsruhe/CDCSegment.h>



namespace Belle2 {

  class CDCTrack : public TObject {
  public:

    /*! Empty constructor. */
    CDCTrack();

    /*! Constructor. */
    CDCTrack(int Id);

    /*! Destructor. */
    ~CDCTrack();

    /*!Adds a Segment object to the track. */
    /*! All TrackHit object in the Segment are also added to the m_TrackHits. */
    void addSegment(CDCSegment aSegment);

    /*!Adds a TrackHit object to the track. */
    void addTrackHit(CDCTrackHit aTrackHit);

    /*!Sets and updates some member variables of the track. */
    /*! This method is executed every time a new TrackHit or a Segment is added to the Track. All member variables dependent on this are updated (number of Segments/Hits, direction, inner and outermost Segment/Hits)*/
    void update();

    /*!Returns the Id of the Track. */
    int getId() const {return m_Id;};

    /*!Returns number of Segment objects in the Track. */
    int getNSegments() const {return m_nSegments;};

    /*!Returns total number of TrackHits in the Track. */
    /*!Number of Hits from Segments + number of Hits added by Hit salvage. */
    int getNHits() const {return m_nHits;};

    /*!Returns the direction of the track in the conformal plane which is the sum of segment`s directions. */
    TVector3 getDirection() const {return m_direction;};

    /*!Returns a vector with all Segments in the Track. */
    std::vector<CDCSegment> getSegments() const {return m_Segments;};

    /*!Returns a vector with all Hits in the Track. */
    std::vector<CDCTrackHit> getTrackHits() const {return m_TrackHits;};

    /*!Returns the innermost axial hit (closest to the origin) of the track. */
    CDCTrackHit getInnerMostHit() const {return m_innerMostHit;};

    /*!Returns the outermost axial hit (farthest from the origin) of the track. */
    CDCTrackHit getOuterMostHit() const {return m_outerMostHit;};

    /*!Returns the innermost segment (closest to the origin) of the track. */
    CDCSegment getInnerMostSegment() const {return m_innerMostSegment;};

    /*!Returns the outermost segment (farthest to the origin) of the track. */
    CDCSegment getOuterMostSegment() const {return m_outerMostSegment;};


  private:

    int m_Id;
    int m_nSegments;
    int m_nHits;

    std::vector<CDCSegment> m_Segments;
    std::vector<CDCTrackHit> m_TrackHits;

    TVector3 m_direction;

    CDCTrackHit m_innerMostHit;
    CDCTrackHit m_outerMostHit;
    CDCSegment m_innerMostSegment;
    CDCSegment m_outerMostSegment;


    //! ROOT ClassDef macro to make this class a ROOT class.
    ClassDef(CDCTrack, 1);

  }; //end class CDCTrack
} //end namespace Belle2

#endif //CDCTRACK


