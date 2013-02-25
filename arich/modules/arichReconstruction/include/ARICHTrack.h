/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ARICHTRACK_H
#define ARICHTRACK_H


#include <arich/dataobjects/ARICHAeroHit.h>
#include <TObject.h>

#define MAXLKH 5

namespace Belle2 {

  //!  This is class for aRICH track.
  /*!  This track class holds track parameters and track related routines needed by reconstruction. After reconstruction, values of likelihood function for different particle hypotheses associated with track are written. This should be changed with more general track class eventually.
   */
  class ARICHGeometryPar;
  class ARICHTrack : public TObject {
  public:

    //! Default constructor for ROOT IO. */
    ARICHTrack():
      m_originalPosition(0, 0, 0),
      m_originalDirection(0, 0, 0),
      m_originalMomentum(0),
      m_reconstructedPosition(0, 0, 0),
      m_reconstructedDirection(0, 0, 0),
      m_reconstructedMomentum(0),
      m_PDGCharge(0),
      m_PDGEncoding(-1),
      m_G4TrackID(-1),
      m_identity(-1) {
      /*! does nothing */
    }

    //! Constructor from track hit on aerogel plane */
    /*!
      \param aeroHit track hit on aerogel plane (ARICHAeroHit)
    */
    ARICHTrack(const ARICHAeroHit& aeroHit);

    //! Useful constructor */
    /*!
      \param r vector of track position on aerogel plane
      \param dir vector of track direction on aerogel plane
      \param p momentum of track on aerogel plane
      \param type PDG id of particle
      \param trackID geant4 track id
     */

    ARICHTrack(TVector3 r, TVector3 dir, double p, int type, int trackID) {
      m_originalPosition  =  r;
      m_originalDirection =  dir;
      m_originalMomentum  =  p;
      m_reconstructedPosition = TVector3(0, 0, 0);
      m_reconstructedDirection = TVector3(0, 0, 0);
      m_reconstructedMomentum = -1;
      m_PDGCharge = 0;
      m_PDGEncoding = type;
      m_G4TrackID = trackID;
      m_identity = -1;
      for (int i = 0; i < MAXLKH; i++) {
        m_lkh[i]  = 0;
        m_sfot[i] = 0;
        m_acc[i]  = 0;
      }
    };


    //! destructor
    ~ARICHTrack() {
      /*! does nothing */
    }

    //! returns original track position (from geant4 simulation)
    const TVector3& getOriginalPosition() const { return  m_originalPosition;};

    //! returns original track direction (from geant4 simulation)
    const TVector3& getOriginalDirection() const { return m_originalDirection;} ;

    //! returns original track momentum (from geant4 simulation)
    double getOriginalMomentum() const { return m_originalMomentum;};

    //! returns reconstructed track position
    const TVector3& getReconstructedPosition() const { return  m_reconstructedPosition;};

    //! returns original track direction
    const TVector3& getReconstructedDirection() const { return m_reconstructedDirection;} ;

    //! returns original track momentum
    double getReconstructedMomentum() const { return m_reconstructedMomentum;};

    //! returns charge of particle
    double  getPDGCharge() const { return  m_PDGCharge; };

    //! returns PDG id number of particle
    int  getPDGEncoding() const { return m_PDGEncoding;};

    //! returns track id from geant4 simulation
    int  getG4TrackID() const {return m_G4TrackID;};

    //! return particle index (0 electron, 1 muon, 2 pion, 3 kaon, 4 proton, -1 else)
    int getIdentity() const {return m_identity;};

    //! returns value of likelihood function for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double getLikelihood(int i) const {return m_lkh[i]; };

    void getLikelihood(double like[]) {like[0] = m_lkh[0]; like[1] = m_lkh[1]; like[2] = m_lkh[2]; like[3] = m_lkh[3]; like[4] = m_lkh[4];};

    //! returns expected number of detected photons for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double getExpectedNOfPhotons(int i) const {return m_sfot[i]; };

    void getExpectedNOfPhotons(double nphot[]) {nphot[0] = m_sfot[0]; nphot[1] = m_sfot[1]; nphot[2] = m_sfot[2]; nphot[3] = m_sfot[3]; nphot[4] = m_sfot[4]; };

    //! returns geometrical acceptance of emitted Cherenkov photos by particle "i"(here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double getGeometricalAcceptance(int i) const {return m_acc[i]; };

    //! sets value of likelihood function for particle hypothesis "i" to "val" (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void setLikelihood(int i, double val) {m_lkh[i] = val; };

    //!  sets value of likelihood function for all particle hypotheses. "imax" is number of particle hypotheses, "val" is array of hypotheses values (val[0] electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void setLikelihood(int imax, double* val) {for (int i = 0; i < imax; i++) m_lkh[i] = val[i];};

    //! sets expected number of detected photons for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void setExpectedNOfPhotons(int i, double val) {m_sfot[i] = val; };

    //! sets the reconstructed value of track parameters.
    // "r" is position of track, "dir" direction and "p" momentum of track on aerogel plane.
    void   setReconstructedValues(TVector3 r, TVector3 dir, double p) {
      m_reconstructedPosition = r;
      m_reconstructedDirection = dir;
      m_reconstructedMomentum = p;
    }

    //! Returns mean emission position of Cherenkov photons from i-th aerogel layer.
    TVector3 getMeanEmissionPosition(int i) const;

    //! Returns the distance between the point of mean emission position from i-th aerogel layer and the track exit point from that layer.
    double   getMeanEmissionLength(int i) const;

    //! Returns original mean emission direction of Cherenkov photons (same as track direction from geant4 simulation)
    const TVector3& getOriginalMeanEmissionDirection() const {return m_originalDirection;};

    //! Returns mean emission direction of Cherenkov photons (same as reconstructed track direction)
    const TVector3& getMeanEmissionDirection() const {return m_reconstructedDirection;};

    //! Returns mean emission direction of Cherenkov photons from i-th aerogel (same as reconstructed track direction)
    const TVector3& getMeanEmissionDirection(int) const {return m_reconstructedDirection;};

    //! Returns track position at the aergel exit (exit from last layer).
    const TVector3 getAerogelExit(void) const {
      return getAerogelExit(2);
    }

    //! Returns track position at the exit of i-th aerogel layer.
    const TVector3 getAerogelExit(int i) const ;

    //! Returns track position at the entrance in i-th aerogel layer.
    const TVector3 getAerogelInput(int i) const ;

    //! Returns track direction at point with z coordinate "zout" (assumes straight track).
    const TVector3& getDirectionAtZ(double) const {
      return m_originalDirection;
    }

    //! Returns track position at point with z coordinate "zout" (assumes straight track).
    const TVector3 getPositionAtZ(double zout) const;



  private:

    // track parameter from geant4 simulation
    TVector3 m_originalPosition;          /**< Original position on aerogel plane. */
    TVector3 m_originalDirection;         /**< Original direction on aerogel plane. */
    double   m_originalMomentum;          /**< Original momentum on aerogel plane. */

    // track parameter from tracking (for now just "smeared" parameters from simulation)
    TVector3 m_reconstructedPosition;     /**< Reconstructed position.  */
    TVector3 m_reconstructedDirection;    /**< Reconstructed direction. */
    double   m_reconstructedMomentum;     /**< Reconstructed momentum. */

    double m_PDGCharge;                   /**< particle charge */
    int  m_PDGEncoding;                   /**< particle PDG id number */
    int  m_G4TrackID;                     /**< track ID from geant4 simulation */
    int m_identity;                       /**< particle index (0 electron, 1 muon, 2 pion, 3 kaon, 4 proton, -1 else). */

    //! converts PDG particle code to particle index
    /*
      \param PDG particle code
     */
    int Lund2Type(int ipart);

    double  m_lkh[MAXLKH];  /**< Value of likelihood function for different particle hypotheses. */
    double  m_sfot[MAXLKH]; /**< Number of expected detected photons for different particle hypotheses.  */
    double  m_acc[MAXLKH];  /**< Geometrical acceptance of expected cherenkov ring for different particle hypotheses. */

    ClassDef(ARICHTrack, 1); /**< the class title */

  };
} // namespace Belle2

#endif // ARICHTRACK_H
