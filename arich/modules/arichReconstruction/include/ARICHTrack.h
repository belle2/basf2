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

#include <arich/hitarich/ARICHAeroHit.h>

#include <TObject.h>

#define MAXLKH 5

namespace Belle2 {

  /**
   *  This is class for aRICH track.
   *  This track class holds track parameters and track related routines needed by reconstruction. After reconstruction, values of likelihood function for different particle hypotheses associated with track are written.
   */

  class ARICHTrack : public TObject {
  public:

    /** Default constructor for ROOT IO. */
    ARICHTrack() {;}

    /** Constructor from track hit on aerogel plane */
    ARICHTrack(const ARICHAeroHit &aeroHit);

    /** Useful constructor */
    ARICHTrack(TVector3 r, TVector3 dir, double p, int type, int trackID) {
      _OriginalPosition  =  r;
      _OriginalDirection =  dir;
      _OriginalMomentum  =  p;
      _PDGEncoding = type;
      _G4TrackID = trackID;
    };

  private:


    // track parameter from geant4 simulation
    TVector3 _OriginalPosition;          /**< Original position on aerogel plane. */
    TVector3 _OriginalDirection;         /**< Original direction on aerogel plane. */
    double   _OriginalMomentum;          /**< Original momentum on aerogel plane. */

    // track parameter from tracking (for now just "smeared" parameters from simulation)
    TVector3 _ReconstructedPosition;     /**< Reconstructed position.  */
    TVector3 _ReconstructedDirection;    /**< Reconstructed direction. */
    double   _ReconstructedMomentum;     /**< Reconstructed momentum. */

    double _PDGCharge;                   /**< particle charge */
    int  _PDGEncoding;                   /**< particle PDG id number */
    int  _G4TrackID;                     /**< track ID from geant4 simulation */
    int _Identity;                       /**< particle index (0 electron, 1 muon, 2 pion, 3 kaon, 4 proton, -1 else). */

    /**
     *  converts PDG particle code to particle index
     */
    int Lund2Type(int ipart);

    double  _lkh[MAXLKH];  /**< Value of likelihood function for different particle hypotheses. */
    double  _sfot[MAXLKH]; /**< Number of expected detected photons for different particle hypotheses.  */
    double  _acc[MAXLKH];  /**< Geometrical acceptance of expected cherenkov ring for different particle hypotheses. */

    ClassDef(ARICHTrack, 1); /*!< the class title */

  public:

    //! returns original track position (from geant4 simulation)
    const TVector3 &GetOriginalPosition() const { return  _OriginalPosition;};

    //! returns original track direction (from geant4 simulation)
    const TVector3 &GetOriginalDirection() const { return _OriginalDirection;} ;

    //! returns original track momentum (from geant4 simulation)
    double GetOriginalMomentum() const { return _OriginalMomentum;};

    //! returns reconstructed track position
    const TVector3 &GetReconstructedPosition() const { return  _ReconstructedPosition;};

    //! returns original track direction
    const TVector3 &GetReconstructedDirection() const { return _ReconstructedDirection;} ;

    //! returns original track momentum
    double GetReconstructedMomentum() const { return _ReconstructedMomentum;};

    //! returns charge of particle
    double  GetPDGCharge() const { return  _PDGCharge; };

    //! returns PDG id number of particle
    int  GetPDGEncoding() const { return _PDGEncoding;};

    //! returns track id from geant4 simulation
    int  GetG4TrackID() const {return _G4TrackID;};

    //! return particle index (0 electron, 1 muon, 2 pion, 3 kaon, 4 proton, -1 else)
    int GetIdentity() const {return _Identity;};

    //! returns value of likelihood function for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double GetLikelihood(int i) const {return _lkh[i]; };

    //! returns expected number of detected photons for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double GetExpectedNOfPhotons(int i) const {return _sfot[i]; };

    //! returns geometrical acceptance of emitted Cherenkov photos by particle "i"(here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double GetGeometricalAcceptance(int i) const {return _acc[i]; };

    //! Sets value of likelihood function for particle hypothesis "i" to "val" (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void SetLikelihood(int i, double val) {_lkh[i] = val; };

    //!  Sets value of likelihood function for all particle hypotheses. "imax" is number of particle hypotheses, "val" is array of hypotheses values (val[0] electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void SetLikelihood(int imax, double *val) {for (int i = 0; i < imax; i++)_lkh[i] = val[i];};

    //! Sets expected number of detected photons for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void SetExpectedNOfPhotons(int i, double val) {_sfot[i] = val; };

    //! Sets the reconstructed value of track parameters.
    // "r" is position of track, "dir" direction and "p" momentum of track on aerogel plane.
    void   SetReconstructedValues(TVector3 r, TVector3 dir, double p) {
      _ReconstructedPosition = r;
      _ReconstructedDirection = dir;
      _ReconstructedMomentum = p;
    }

    //! Returns mean emission position of Cherenkov photons from i-th aerogel layer.
    TVector3 GetMeanEmissionPosition(int i) const;

    //! Returns the distance between the point of mean emission position from i-th aerogel layer and the track exit point from that layer.
    double   GetMeanEmissionLength(int i) const;

    //! Returns original mean emission direction of Cherenkov photons (same as track direction from geant4 simulation)
    const TVector3 & GetOriginalMeanEmissionDirection() const {return _OriginalDirection;};

    //! Returns mean emission direction of Cherenkov photons (same as reconstructed track direction)
    const TVector3 & GetMeanEmissionDirection() const {return _ReconstructedDirection;};

    //! Returns mean emission direction of Cherenkov photons from i-th aerogel (same as reconstructed track direction)
    const TVector3 & GetMeanEmissionDirection(int i) const {return _ReconstructedDirection;};

    //! Returns track position at the aergel exit (exit from last layer).
    const TVector3 GetAerogelExit(void) const {
      return GetAerogelExit(2);
    }

    //! Returns track position at the exit of i-th aerogel layer.
    const TVector3 GetAerogelExit(int i) const ;

    //! Returns track position at the entrance in i-th aerogel layer.
    const TVector3 GetAerogelInput(int i) const ;

    //! Returns track direction at point with z coordinate "zout" (assumes straight track).
    const TVector3 & GetDirectionAtZ(double zout) const {
      return _OriginalDirection;
    }

    //! Returns track position at point with z coordinate "zout" (assumes straight track).
    const TVector3 GetPositionAtZ(double zout) const;

  };
}

#endif // ARICHTRACK_H
