/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCTRACKCANDIDATE_H
#define CDCTRACKCANDIDATE_H

#include "TObject.h"
#include <tracking/cdcConformalTracking/CDCTrackHit.h>
#include <tracking/cdcConformalTracking/CDCSegment.h>
//#include "TVector2.h"



namespace Belle2 {

  /** Class for track candidates after CDC pattern recognition. */
  class CDCTrackCandidate : public TObject {
  public:

    /** Empty constructor. */
    CDCTrackCandidate();

    /** Constructor.
     * Creates a new track with an Id.
     */
    CDCTrackCandidate(int Id);

    /** Copy Constructor.
     * Creates a new track from a copy of another track and a new Id.
     */
    CDCTrackCandidate(CDCTrackCandidate &candidate, int Id);

    /** Destructor. */
    ~CDCTrackCandidate();

    /** Adds a segment object to the track.
     *  All TrackCandidateHit objects in the segment are also added to the m_TrackHits.
     */
    void addSegment(CDCSegment & aSegment);

    /**Removes a segment with a given Id from the track. */
    void removeSegment(int Id);

    /** Sets a Chi2 value to describe the quality of the TrackCandidate. */
    void setChiSquare(double chi2);

    /** Sets an absolute value of the momentum of the TrackCandidate. */
    void setMomentumValue(double momentum);

    /** Sets and updates some member variables of the TrackCandidate.
     * This method is executed every time a new Segment is added to the TrackCandidate.
     * All member variables dependent on this are updated (number of Segments/Hits, direction, inner and outermost Segment).
     */
    void update();

    /** Performs estimation of the momentum direction.
     * Assuming the TrackCandidate starting point to be at (0,0,0), the direction of the TrackCandidate can be estimated as the position of the innermost hit.
     * As for the moment only stereo hits have some meaningful z-coordinates, the coordinates of the innermost stereo hit are normalized and assigned to the momentum vector.
     * (This is a very rough estimation, no drift time as taken into account but only the coordinates of the hit wire. The z-coordinate is also only a first estimation. It has to be checked if this is good enough for this tracking step.)
     */
    void estimateMomentum();

    /** Sets the charge of the track. */
    void setChargeSign(int sign);

    /** Returns the Id of the TrackCandidate. */
    int getId() const {return m_Id;};

    /** Returns number of segments in the TrackCandidate. */
    int getNSegments() const {return m_nSegments;};

    /** Returns the total number hits in the TrackCandidate. */
    int getNHits() const {return m_nHits;};

    /** Returns the direction of the TrackCandidate in the conformal plane.
     * This direction is the sum of segment`s directions.
     */
    TVector3 getDirection() const {return m_direction;};

    /** Returns a vector with all segments in the TrackCandidate. */
    std::vector<Belle2::CDCSegment> & getSegments() {return m_Segments;};

    /** Returns a vector with all hits in the TrackCandidate. */
    std::vector<Belle2::CDCTrackHit> & getTrackHits() {return m_TrackHits;};

    /** Returns the innermost axial hit (closest to the origin) of the TrackCandidate. */
    CDCTrackHit getInnerMostHit() const {return m_innerMostHit;};

    /** Returns the outermost axial hit (farthest from the origin) of the TrackCandidate. */
    CDCTrackHit getOuterMostHit() const {return m_outerMostHit;};

    /** Returns the innermost axial segment (closest to the origin) of the TrackCandidate. */
    CDCSegment getInnerMostSegment() const {return m_innerMostSegment;};

    /** Returns the outermost axial segment (farthest to the origin) of the TrackCandidate. */
    CDCSegment getOuterMostSegment() const {return m_outerMostSegment;};

    /** Returns a Chi2 value of the TrackCandidate. */
    double getChiSquare() const {return m_chi2;};

    /** Returns the absolute value of the momentum of the TrackCandidate. */
    double getMomentumValue() {return m_momentumValue;}

    /** Returns the momentum vector of the TrackCandidate. */
    TVector3 getMomentumVector() {return m_momentumVector;}

    /** Returns the estimated charge of the track . */
    int getChargeSign() { return m_chargeSign; }


    //Methods to match the tracks with mc particles, will be explain and used in the next commit...
    //------------------------------------------------------------
    /*
    std::vector<TVector2> getMCParticles(){ return m_mcParticles;}

    void addMCParticle(int Id);

    void evaluateMC();

    double evaluateMC(int mcId);

    double getCorrectMC(){ return m_correctMC; }

    double getMCIndex(){return m_mcIndex;}
    */
    //--------------------------------------------------------------



  private:

    int m_Id;                               /**< Id of the track candidate*/
    int m_nSegments;                        /**< Number of segments in the TrackCandidate*/
    int m_nHits;                            /**< Number of hits in the TrackCandidate */

    std::vector<CDCSegment> m_Segments;     /**< vector to store segments belonging to this TrackCandidate */
    std::vector<CDCTrackHit> m_TrackHits;   /**< vector to store TrackCandidateHits belonging to this TrackCandidate */

    TVector3 m_direction;                   /**< Direction of the TrackCandidate in the conformal plane */

    CDCTrackHit m_innerMostHit;           /**< Innermost (closest to the origin) hit of the TrackCandidate */
    CDCTrackHit m_outerMostHit;             /**< Outermost (farthest to the origin) hit of the TrackCandidate */
    CDCSegment m_innerMostSegment;          /**< Innermost (closest to the origin) segment of the TrackCandidate */
    CDCSegment m_outerMostSegment;          /**< Outermost (farthest to the origin) segment of the TrackCandidate */

    double m_chi2;                          /**< Chi2 value to describe the quality of the TrackCandidate */

    double m_momentumValue;                 /**< Absolut momentum value of the TrackCandidate */
    TVector3 m_momentumVector;              /**< 3D momentum vector of the TrackCandidate (unit vector)*/

    int m_chargeSign;                       /**< Charge of the TrackCandidate. (+1 or -1)*/


    //Member variables to match the tracks with mc particles
    //----------------------------------------------------------------
    //std::vector<TVector2> m_mcParticles;
    //double m_correctMC;
    //int m_mcIndex;
    //---------------------------------------------------------------

    /** ROOT ClassDef macro to make this class a ROOT class.*/
    ClassDef(CDCTrackCandidate, 1);

  }; //end class CDCTrackCandidate
} //end namespace Belle2

#endif //CDCTRACKCANDIDATE


