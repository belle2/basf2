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
     * As for the moment only stereo hits have some meaningful z-coordinates, the coordinates of the innermost stereo hit are normalized  with the momentum value and assigned to the momentum vector.
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

    // The following methods are used to match the CDCTrackCandidate with MCParticles

    /** Adds an MCParticle to the collection of MCParticles which produced the Hits of this TrackCandidate.
     *  The Id of the MCParticle is added to the m_mcParticles vector.
     *  If this MCParticle has already contributed to this TrackCandidate, the corresponding number of Hits is augmented by 1.
     */
    void addMCParticle(int id);

    /** Matches the TrackCandidate with an MCParticle.
     *  This method selects the largest contribution in m_mcParticle and assigns the Id of the matched MCParticle and the purity of the TrackCandidate.
     */
    void evaluateMC();

    /** Returns a vector of pairs: MCParticleId & Number of Hits produced by this MCParticle. */
    std::vector<std::pair<int, int> > getMCParticles() { return m_mcParticles; }

    /** Returns the Id of the MCParticle matched to this TrackCandidate. */
    int getMCId() { return m_mcId; }

    /** Returns the purity of this TrackCandidate.
     *  Purity means in this case: number of Hits contributed by the matched MCParticle/total number of Hits * 100 .
     */
    double getPurity() { return m_purity; }

    /** Returns the contribution of the given MCParticle to this TrackCandidate.
     *  Return value is: number of Hits contributed by the given MCParticle/total number of Hits * 100 .
     */
    double getRatioForMCP(int mcId);


  private:

    int m_Id;                               /**< Id of the track candidate*/
    int m_nSegments;                        /**< Number of segments in the TrackCandidate*/
    int m_nHits;                            /**< Number of hits in the TrackCandidate */

    std::vector<CDCSegment> m_Segments;     /**< vector to store segments belonging to this TrackCandidate */
    std::vector<CDCTrackHit> m_TrackHits;   /**< vector to store TrackCandidateHits belonging to this TrackCandidate */

    TVector3 m_direction;                   /**< Direction of the TrackCandidate in the conformal plane */

    CDCTrackHit m_innerMostHit;             /**< Innermost (closest to the origin) hit of the TrackCandidate */
    CDCTrackHit m_outerMostHit;             /**< Outermost (farthest to the origin) hit of the TrackCandidate */
    CDCSegment m_innerMostSegment;          /**< Innermost (closest to the origin) segment of the TrackCandidate */
    CDCSegment m_outerMostSegment;          /**< Outermost (farthest to the origin) segment of the TrackCandidate */

    double m_chi2;                          /**< Chi2 value to describe the quality of the TrackCandidate */

    double m_momentumValue;                 /**< Absolute momentum value of the TrackCandidate */
    TVector3 m_momentumVector;              /**< 3D momentum vector of the TrackCandidate */

    int m_chargeSign;                       /**< Charge of the TrackCandidate. (+1 or -1)*/

    //The following member variables are used to match the CDCTrackCandidate with MCParticles

    std:: vector <std::pair<int, int> > m_mcParticles;  /**< vector to store pairs <MCParticleId, Number of Hits from this MCParticle> */
    int m_mcId;                                         /**< Id of the MCParticle matched to this TrackCandidate */
    double m_purity;                                    /**< Purity of this TrackCandidate: contribution from the matched MCParticle */


    /** ROOT ClassDef macro to make this class a ROOT class.*/
    ClassDef(CDCTrackCandidate, 1);

  }; //end class CDCTrackCandidate
} //end namespace Belle2

#endif //CDCTRACKCANDIDATE


