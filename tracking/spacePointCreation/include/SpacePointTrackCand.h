/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

// framework
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>

// genfit TrackCand
#include <genfit/TrackCand.h>
#include <genfit/TrackCandHit.h>

// SpacePoint
#include <tracking/spacePointCreation/SpacePoint.h>

// stl
#include <vector>
// #include <utility> // for pair

// ROOT
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TVector3.h>

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
    BELLE2_DEFINE_EXCEPTION(SPTCIndexOutOfBounds, "Trying to acces a SpacePoint from a SpacePointTrackCand via an index that is out of bounds!");

    /** Status information that can be set to indicate several properties of the SpacePointTrackCand
     * NOTE: there are some properties that are at the moment stored in other members of the SpacePointTrackCand but can be moved into here if memory usage is an issue
     */
    enum RefereeStatusBit {
      c_checkedByReferee = 1, /**< bit 0: SpacePointTrackCand has been checked by a Referee (all possible tests) */
      c_checkedClean = 2, /**< bit 1: The SpacePointTrackCand shows no 'problematic' behaviour */
      c_hitsOnSameSensor = 4, /**< bit 2: SpacePointTrackCand has two (or more) SpacePoints on same sensor */
      c_hitsLowDistance = 8, /**< bit 3: SpacePointTrackCand has two (or more) SpacePoints that are not far enough apart. NOTE: distance is judged by referee (and also set there) */
      c_removedHits = 16, /**< bit 4: SpacePoints were removed from this SpacePointTrackCand */
      c_checkedTrueHits =  32, /**< bit 5: All of the SpacePoints of the SpacePointTrackCand have a relation to (at least one) TrueHit(s) */
      c_checkedSameSensors = 64, /**< bit 6: It has been checked if two consecutive SpacePoints are on the same sensor for this SpacePointTrackCand */
      c_checkedMinDistance =  128, /**< bit 7: It has been checked if two consecutive SpacePoints are far enough apart */
      c_curlingTrack = 256, /**< bit 8: SpacePointTrackCand is curling (resp. is part of a curling SpacePointTrackCand) */
    };

    /**
     * empty constructor sets pdg code to zero, such that it is possible to determine whether a particle hyptohesis has been asigned to the track candidate or not
     * Also MCTrackID is initialized to -1,
     */
    SpacePointTrackCand();

    /**
     * constructor from a vector<SpacePoint*> and some additional information: pdg code and charge estimate as well as the MCTrackID of the track candidate
     */
    SpacePointTrackCand(const std::vector<const Belle2::SpacePoint*>& spacePoints, int pdgCode = 0, double charge = 0, int mcTrackID = -1);

    // destructor
    virtual ~SpacePointTrackCand();

    /**
     * get hits (space points) of track candidate
     */
    const std::vector<const Belle2::SpacePoint*>& getHits() const { return m_trackSpacePoints; }

    /**
     * get hits (SpacePoints) in range (indices of SpacePoint inside SpacePointTrackCand) [firstInd,lastInd] (the SpacePoint on firstInd and on lastInd wil be returned as well!).
     * NOTE: For all hits range is from 0 to getNHits - 1. throws an exception when trying to access hits outside the allowed range!
     */
    const std::vector<const Belle2::SpacePoint*> getHitsInRange(int firstInd, int lastInd) const;

    /**
     * get the number of hits (space points) in the track candidate
     */
    unsigned int getNHits() const { return m_trackSpacePoints.size(); }

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
     * get the sorting parameters in range (indices of SpacePoints inside SpacePointTrackCand) including firstIndex and lastIndex
     * NOTE: for all hits range is from zero to getNHits - 1. throws an exception when trying to access hits outside the allowed range!
     */
    const std::vector<double> getSortingParametersInRange(int firstIndex, int lastIndex) const;

    /** get TrackStub Index */
    int getTrackStubIndex() const { return m_iTrackStub; }

    /**
     * get the MC Track ID
     */
    int getMcTrackID() const { return m_MCTrackID; }

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
     * Check if a SpacePointTrackCand has removed hits (i.e. at some point it contained more hits than it does when this function is called, removal of hits e.g. by a referee module)
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

    /**
     * print the Track Candidate in its "full beauty". NOTE: prints some parts to stdout, since for printing the state seed the print method form TVectorD is invoked!
     */
    void print(int debuglevel = 150, const Option_t* = "") const;

    /**
     * Checks the equality of the pointers to the contained SpacePoints (pdg-code and charge estimate are not compared!), NOTE: returns false if both TrackCands do not contain any SpacePoints
     */
    bool operator == (const SpacePointTrackCand& rhs);

    /**
     * set the sorting parameters
     */
    void setSortingParameters(const std::vector<double>& sortParams);

    /**
     * set a hypothesis for the particle by setting a pdgcode (will also set the appropriate charge)
     */
    void setPdgCode(int pdgCode);

    /**
     * set the 6D state seed
     */
    void set6DSeed(const TVectorD& state6D) { m_state6D = state6D; }

    /**
     * set the covariance matrix seed
     */
    void setCovSeed(const TMatrixDSym& cov) { m_cov6D = cov; }

    /**
     * add a new space point to the track candidate
     */
    void addSpacePoint(const SpacePoint* newSP) { m_trackSpacePoints.push_back(newSP); }

    /**
     * add a new SpacePoint and its according sorting parameter to the track candidate
     */
    void addSpacePoint(const SpacePoint* newSP, double sortParam) { m_trackSpacePoints.push_back(newSP); m_sortingParameters.push_back(sortParam); }

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

    /** clear the referee status */
    void clearRefereeStatus() { m_refereeStatus = 0; }

    /** remove a SpacePoint (and its sorting parameter) from the SpacePointTrackCand */
    void removeSpacePoint(int indexInTrackCand);

  protected:
    /**
     * pointers to SpacePoints in the datastore
     */
    std::vector<const SpacePoint*> m_trackSpacePoints;

    /**
     * indices to SpacePoints in the datastore WARNING: name of storeArray not preserved this way!
     */
    std::vector<unsigned int> m_trackSpacePointIndices;

    /**
     * sorting Parameters, can be used to sort the SpacePoints. Also needed to store the sorting parameters of e.g. a genfit::TrackCand if the SpacePointTrackCand is generated by a conversion from a genfit::TrackCand
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
     * Index of TrackStub in a curling Track Candidate. If the TrackCandidate is not curling this value is set to 0. If it is not yet checked if the TrackCand is curling it is set to -1. If it is a curling TrackCand the counter starts at 1, indicating that this is the first outgoing part of the TrackCand. COULDDO: implement such a feature via something like a linked list (would be much nicer, but for the moment this little workaround works)
     */
    int m_iTrackStub;

    /**
     * bit-field to indicate different properties that are checked by the referee module
     */
    unsigned short int m_refereeStatus;

    ClassDef(SpacePointTrackCand, 5) // last members added: RefereeStatutsBit(5), m_refereeProperties(5) m_iTrackStub(4), m_flightDirection(3), m_sortingParameters (2)
  };
}
