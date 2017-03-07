/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener, Jakob Lettenbichler                     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

// framework
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>

// SpacePoint
#include <tracking/spacePointCreation/SpacePoint.h>

// stl
#include <vector>
// #include <utility> // for pair

// ROOT
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TVector3.h>

// USHRT_MAX
#include <climits>

namespace Belle2 {

  /** The SpacePointTrackCand class.
   *
   * Stores a track candidate using space points (instead of recohits).
   * tmadlener: the design follows rather closely the one from genfit::TrackCand
   */
  class SpacePointTrackCand: public RelationsObject {

  public:

    /**
     * Exception thrown, when an Unsupported Detector Type occurs. Momentarily supported: PXD & SVD
     */
    BELLE2_DEFINE_EXCEPTION(UnsupportedDetType, "The Detector Type is not supported by this class. Supported are: PXD and SVD");

    /**
     * Exception thrown, when trying to access SpacePoints by their index inside of SpacePointTrackCand, but index is out of bounds
     */
    BELLE2_DEFINE_EXCEPTION(SPTCIndexOutOfBounds, "Trying to acces a SpacePoint from a SpacePointTrackCand via an"\
                            " index that is out of bounds!");

    /**
     * Exception thrown, when the size of the vector containing the spacePoints has a different size than the vector containing the sorting parameters
     */
    BELLE2_DEFINE_EXCEPTION(SPTCSortingParameterSizeInvalid,
                            "Trying to modify SortingParameters, but number of new SortingParameters differs from number of SpacePoints");

    /** Status information that can be set to indicate several properties of the SpacePointTrackCand
     * NOTE: there are some properties that are at the moment stored in other members of the SpacePointTrackCand but can be moved
     * into here if memory usage is an issue
     * NOTE: if you add something here, it should also be added in the 'getRefereeStatusString' method!
     */
    enum RefereeStatusBit {
      c_checkedByReferee = 1, /**< bit 0: SpacePointTrackCand has been checked by a Referee (all possible tests) */
      c_checkedClean = 2, /**< bit 1: The SpacePointTrackCand shows no 'problematic' behaviour */
      c_hitsOnSameSensor = 4, /**< bit 2: SpacePointTrackCand has two (or more) SpacePoints on same sensor */
      /** bit 3: SpacePointTrackCand has two (or more) SpacePoints that are not far enough apart.
       * NOTE: distance is judged by referee (and also set there) */
      c_hitsLowDistance = 8,
      c_removedHits = 16, /**< bit 4: SpacePoints were removed from this SpacePointTrackCand */
      /** bit 5: All of the SpacePoints of the SpacePointTrackCand have a relation to (at least one) TrueHit(s) */
      c_checkedTrueHits =  32,
      /** bit 6: It has been checked if two consecutive SpacePoints are on the same sensor for this SpacePointTrackCand */
      c_checkedSameSensors = 64,
      c_checkedMinDistance =  128, /**< bit 7: It has been checked if two consecutive SpacePoints are far enough apart */
      c_curlingTrack = 256, /**< bit 8: SpacePointTrackCand is curling (resp. is part of a curling SpacePointTrackCand) */
      /** bit 9: Not all Clusters of the genfit::TrackCand have been used to create this SpacePointTrackCand */
      c_omittedClusters = 512,
      c_singleClustersSPs = 1024, /**< bit 10: The SpacePointTrackCand contains single Cluster SpacePoints */
      c_isActive =  2048, /**< bit 11: SPTC is active (i.e. if false, some module rejected it for further use */
      c_isReserved = 4096, /**< bit 12: SPTC is reserved (i.e. should not be altered and should be treated with high priority) */
      c_initialState = 0 + c_isActive, /**< this is the initialState which will always be set in the beginning and applied after reset */
    };

    /**
     * empty constructor sets pdg code to zero, such that it is possible to determine whether a particle hyptohesis has been asigned
     * to the track candidate or not
     * Also MCTrackID is initialized to -1,
     * Each SpacePointTrackCand is created in c_isActive-state and has to be deactivated manually if need be
     */
    SpacePointTrackCand();

    /**
     * constructor from a vector<SpacePoint*> and some additional information:
     * pdg code and charge estimate as well as the MCTrackID of the track candidate
     * sortingParameters are generated from the order of the SpacePoints!
     * Each SpacePointTrackCand is created in c_isActive-state and has to be deactivated manually if need be
     */
    SpacePointTrackCand(const std::vector<const Belle2::SpacePoint*>& spacePoints, int pdgCode = 0, double charge = 0,
                        int mcTrackID = -1);

    /** destructor */
    virtual ~SpacePointTrackCand() = default;


    /**
     * Checks the equality of the pointers to the contained SpacePoints (pdg-code and charge estimate are not compared!)
     * NOTE: returns false if both TrackCands do not contain any SpacePoints
     */
    bool operator == (const SpacePointTrackCand& rhs);


    /**
     * Compares two track candidates and determines if they share hits.
    *
    * optional parameter determines if TCs shall be compared at SpacePoint-level (standard, true) or at Cluster level (= false)
     * If at least one Hit is shared, return value is true -> tracks are overlapping.
     * Return value is false if no hits are shared.
     * */
    bool checkOverlap(const SpacePointTrackCand& rhs, bool compareSPs = true);

    /**
     * get hits (space points) of track candidate
     */
    const std::vector<const Belle2::SpacePoint*>& getHits() const { return m_trackSpacePoints; }

    /**
     * get hits (space points) sorted by their respective sorting parameter
     */
    const std::vector<const Belle2::SpacePoint*> getSortedHits() const;

    /** returns a const_iterator (begin()) for easily looping over hits of SpacePointTrackCand. Ignores sortingParameters! */
    std::vector<const Belle2::SpacePoint*>::const_iterator begin() const { return m_trackSpacePoints.begin(); }

    /** returns a const_iterator (end()) for easily looping over hits of SpacePointTrackCand. Ignores sortingParameters! */
    std::vector<const Belle2::SpacePoint*>::const_iterator end() const { return m_trackSpacePoints.end(); }

    /**
     * get hits (SpacePoints) in range (indices of SpacePoint inside SpacePointTrackCand)
     * including first index and excluding last index (the SpacePoint on firstInd but not the one on lastIndex wil be returned!).
     * NOTE: For all hits range is from 0 to getNHits(). throws an exception when trying to access hits outside the allowed range!
     */
    const std::vector<const Belle2::SpacePoint*> getHitsInRange(int firstInd, int lastInd) const;

    /**
     * get the number of hits (space points) in the track candidate
     */
    unsigned int getNHits() const { return m_trackSpacePoints.size(); }

    /**
    * get the number of hits (space points) in the track candidate
    */
    unsigned int size() const { return m_trackSpacePoints.size(); }

    /**
     * get pdg code
     */
    int getPdgCode() const { return m_pdg; }

    /**
     * get charge
     */
    double getChargeSeed() const { return m_q; }

    /** get the covariance matrix seed (6D).  */
    const TMatrixDSym& getCovSeed() const { return m_cov6D; }

    /** get state seed as 6D vector */
    const TVectorD& getStateSeed() const { return m_state6D; }

    /** get position seed as TVector3 */
    const TVector3 getPosSeed() const { return TVector3(m_state6D[0], m_state6D[1], m_state6D[2]); }

    /** get momentum seed as TVector3 */
    const TVector3 getMomSeed() const { return TVector3(m_state6D[3], m_state6D[4], m_state6D[5]); }

    /**
     * get the sorting parameters
     */
    const std::vector<double>& getSortingParameters() const { return m_sortingParameters; }

    /**
     * get the sorting parameters in range (indices of SpacePoints inside SpacePointTrackCand)
     * including firstIndex and excluding lastIndex
     * NOTE: for all hits range is from zero to getNHits(). throws an exception when trying to access hits outside the allowed range!
     */
    const std::vector<double> getSortingParametersInRange(int firstIndex, int lastIndex) const;

    /** get TrackStub Index */
    int getTrackStubIndex() const { return m_iTrackStub; }

    /**
     * get the MC Track ID
     */
    int getMcTrackID() const { return m_MCTrackID; }

    /**
     * get the MC Track ID (same writing as in genfit::TrackCand)
     */
    int getMcTrackId() const { return m_MCTrackID; }

    /**
     * Return the refere status code of the SpacePointTrackCand.
     * @param bitmask is an optional bitmask that is compared to the referee status of the SpacePointTrackCand
     */
    unsigned short int getRefereeStatus(unsigned short int bitmask = USHRT_MAX) const { return m_refereeStatus & bitmask; }

    /**
     * Check if the SpacePointTrackCand has the status characterized by the bitmask
     */
    bool hasRefereeStatus(unsigned int short bitmask) const { return (m_refereeStatus & bitmask) == bitmask; }

    /**
    * returns the current status of the estimated quality of this track candidate.
    * */
    double getQualityIndex() const { return m_qualityIndex; }

    /**
     * Check if the SpacePointTrackCand contains consecutive SpacePoints that are on the same sensor
     * WARNING: does not check if this has actually been assigned!
     */
    bool hasHitsOnSameSensor() const { return hasRefereeStatus(c_hitsOnSameSensor); }

    /**
     * Check if the SpacePointTrackCand has been checked for consecutive hits on same sensor
     */
    bool checkedSameSensors() const { return hasRefereeStatus(c_checkedSameSensors); }

    /**
     * Check if consecutive SpacePoints are far enough apart throughout the SpacePointTrackCand
     * WARNING: does not check if this has actually been assigned!
     */
    bool hasHitsLowDistance() const { return hasRefereeStatus(c_hitsLowDistance); }

    /**
     * Check if the SpacePointTrackCand has been checked for consecutive hits being far enough apart
     */
    bool checkedMinDistance() const { return hasRefereeStatus(c_checkedMinDistance); }

    /**
     * Check if a SpacePointTrackCand has removed hits
     * (i.e. at some point it contained more hits than it does when this function is called, removal of hits e.g. by a referee module)
     */
    bool hasRemovedHits() const { return hasRefereeStatus(c_removedHits); }

    /**
     * check if particle is outgoing (simply returns member m_flightDirection)!
     */
    bool isOutgoing() const { return m_flightDirection; }

    /**
     * get if the TrackCand is curling.
     * WARNING: does not check if this has actually been assigned!
     */
    bool isCurling() const { return hasRefereeStatus(c_curlingTrack); }

    /** check if the TrackCand has been checked for Curling. */
    bool checkedForCurling() const { return m_iTrackStub != -1; }

    /** check if the TrackCand is part of a curling TrackCand. NOTE: returns false if curling has not been checked */
    bool isPartOfCurlingTrack() const { return m_iTrackStub > 0; }

    /**
     * print the Track Candidate in its "full beauty".
     * NOTE: prints some parts to stdout, since for printing the state seed the print method form TVectorD is invoked!
     */
    void print(int debuglevel = 150, const Option_t* = "") const;

    /**
     * get the refereeStatus as a string (easier to read than an unsigned short int)
     * @param delimiter delimiter to be put inbetween the individual strings, defaults to whitespace (1 character)
     * NOTE: mainly for easier readability of debug output!
     */
    std::string getRefereeStatusString(std::string delimiter = " ") const;

    /**
     * set the sorting parameters
     */
    void setSortingParameters(const std::vector<double>& sortParams);

    /**
     * set a hypothesis for the particle by setting a pdgcode (will also set the appropriate charge)
     */
    void setPdgCode(int pdgCode);

    void setChargeSeed(double charge) { m_q = charge; }

    /**
     * set the 6D state seed
     */
    void set6DSeed(const TVectorD& state6D) { m_state6D = state6D; }

    /**
     * set the covariance matrix seed
     */
    void setCovSeed(const TMatrixDSym& cov) { m_cov6D = cov; }

    /**
    * sets the new status of the estimated quality of this track candidate.
    * valid are values in range [0;1]
    * if value < 0, the track candidate is invalid and one should not use it anymore.
    * 0.5 is the standard value if not been set otherwise
    * */
    void setQualityIndex(double newIndex) { m_qualityIndex = newIndex; }

    /**
     * add a new SpacePoint and its according sorting parameter to the track candidate
     */
    void addSpacePoint(const SpacePoint* newSP, double sortParam)
    {
      m_trackSpacePoints.push_back(newSP);
      m_sortingParameters.push_back(sortParam);
    }

    /**
     * set the direction of flight (true is outgoing, false is ingoing). Initialized to true by default!
     */
    void setFlightDirection(bool direction) { m_flightDirection = direction; }

    /** set TrackStub index */
    void setTrackStubIndex(int trackStubInd) { m_iTrackStub = trackStubInd; }

    /** set referee status (resets the complete to the passed status!) */
    void setRefereeStatus(unsigned short int bitmask) { m_refereeStatus = bitmask; }

    /** add a referee status */
    void addRefereeStatus(unsigned short int bitmask) { m_refereeStatus |= bitmask; }

    /** remove a referee status */
    void removeRefereeStatus(unsigned short int bitmask) { m_refereeStatus &= (~bitmask); }

    /** clear the referee status. WARNING this does not set the value to standard, it completely clears all the states stored! */
    void clearRefereeStatus() { m_refereeStatus = 0; }

    /** resets the referee status to the initial value */
    void resetRefereeStatus() { m_refereeStatus = c_initialState; }

    /** remove a SpacePoint (and its sorting parameter) from the SpacePointTrackCand */
    void removeSpacePoint(int indexInTrackCand);

  protected:
    /**
     * pointers to SpacePoints in the datastore
     */
    std::vector<const SpacePoint*> m_trackSpacePoints;

    /**
     * sorting Parameters, can be used to sort the SpacePoints.
     */
    std::vector<double> m_sortingParameters;

    /**
     * PDG code of particle
     */
    int m_pdg;

    /**
     * track ID from MC simulation
     */
    int m_MCTrackID;

    /**
     * global momentum plus position state (seed) vector
     */
    TVectorD m_state6D;

    /**
     * global momentum plus position state (seed) covariance matrix
     */
    TMatrixDSym m_cov6D;

    /**
     * charge of the particle in units of elementary charge
     */
    double m_q;

    /**
     * direction of flight. true is outgoing, false is ingoing
     */
    bool m_flightDirection;

    /**
     * Index of TrackStub in a curling Track Candidate.
     * + If the TrackCandidate is not curling this value is set to 0.
     * + If it is not yet checked if the TrackCand is curling it is set to -1.
     * + If it is a curling TrackCand the counter starts at 1, indicating that this is the first outgoing part of the TrackCand.
     * COULDDO: implement such a feature via something like a linked list (would be much nicer, but for the moment this little workaround works)
     */
    int m_iTrackStub;

    /**
     * bit-field to indicate different properties that are checked by the referee module
     */
    unsigned short int m_refereeStatus;

    /**
     * An estimation for the quality of the track.
     *
     * Normally defined between 0-1 to describe the propability that this track is real(istic).
     * The quality of the track has to be determined by another function or module.
     * */
    double m_qualityIndex;

    // last members added: RefereeStatutsBit(5), m_refereeProperties(5) m_iTrackStub(4), m_flightDirection(3), m_sortingParameters (2), m_qualityIndex
    ClassDef(SpacePointTrackCand, 8)
  };
}
