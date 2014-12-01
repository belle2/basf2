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

namespace Belle2 {

  /** The SpacePointTrackCand class.
   *
   * Stores a track candidate using space points (instead of recohits).
   * tmadlener: the design follows rather closely the one from genfit::TrackCand
   */
  class SpacePointTrackCand: public RelationsObject {

  public:

    // constructor
    /**
     * empty constructor sets pdg code to zero, such that it is possible to determine whether a particle hyptohesis has been asigned to the track candidate or not
     * Also MCTrackID is initialized to -1,
     */
    SpacePointTrackCand();

    /**
     * constructor from a vector<SpacePoint*> and some additional information: pdg code and charge estimate as well as the MCTrackID of the track candidate
     */
    SpacePointTrackCand(const std::vector<const Belle2::SpacePoint*> spacePoints, int pdgCode = 0, double charge = 0, int mcTrackID = -1);

    // destructor
    virtual ~SpacePointTrackCand();


    // simple getters and setters (similar to genfit track candidate
    // ===================================================================
    /**
     * get hits (space points) of track candidate
     */
    const std::vector<const Belle2::SpacePoint*>& getHits() const { return m_trackSpacePoints; }

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

    /**
     * get the MC Track ID
     */
    int getMcTrackID() const { return m_MCTrackID; }

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

    // ========================================================================
    /**
     * add a new space point to the track candidate
     */
    void addSpacePoint(const SpacePoint* newSP) { m_trackSpacePoints.push_back(newSP); }


    // =========================================================================
    /*
     * print the Track Candidate in its "full beauty". NOTE: prints someparts to stdout, since for printing the state seed the print method form TVectorD is invoked!
     */
    void print(int debuglevel = 100, const Option_t* = "") const;

    // EXCEPTIONS
    //     BELLE2_DEFINE_EXCEPTION(No_SpacePointRelation_Found, "No Relation with a SpacePoint has been found"); // unused at the moment
    BELLE2_DEFINE_EXCEPTION(UnsupportedDetType, "The Detector Type is not supported by this class. Supported are: PXD, SVD and Tel");

    /**
     * Checks the equality of the pointers to the contained SpacePoints (pdg-code and charge estimate are not compared!), NOTE: returns false if both TrackCands do not contain any SpacePoints
     */
    bool operator == (const SpacePointTrackCand& rhs);

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

    ClassDef(SpacePointTrackCand, 1) // last member added: -
  };
}
