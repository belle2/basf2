/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik, Dino Tahirovic              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ARICHTRACK_H
#define ARICHTRACK_H


#include <arich/dataobjects/ARICHAeroHit.h>
#include <tracking/dataobjects/ExtHit.h>
#include <TObject.h>

namespace Belle2 {

  class ARICHGeometryPar;

  /**  This is class for ARICH track.
   *
   *   This is internal ARICH track class. It holds track parameters and
   *   track related functions needed by reconstruction.
   *   After reconstruction, it sets values of likelihood function
   *   and number of the expected photons for
   *   different particle hypotheses, associated with each track.
   */
  class ARICHTrack : public TObject {
  public:



    //! Empty constructor for ROOT IO
    ARICHTrack():
      m_originalPosition(0, 0, 0),
      m_originalDirection(0, 0, 0),
      m_originalMomentum(0),
      m_reconstructedPosition(0, 0, 0),
      m_reconstructedDirection(0, 0, 0),
      m_reconstructedMomentum(0),
      m_PDGCharge(0),
      m_PDGEncoding(-1),
      m_trackID(-1),
      m_extHitID(-1),
      m_identity(-1),
      m_aeroIndex(-1),
      m_flag(1) {
      /*! does nothing */
      for (int i = 0; i < c_noOfHypotheses; i++) {
        m_lkh[i] = 0;
        m_expectedPhotons[i] = 0;
        m_detectedPhotons[i] = 0;
        m_acc[i] = 0;
      }
    }

    //! Constructor from simulation hit on aerogel plane
    /*!
      \param aeroHit track hit on aerogel plane (ARICHAeroHit)
    */
    ARICHTrack(const ARICHAeroHit& aeroHit);

    //! Useful constructor
    /*!
      \param r vector of track position on aerogel plane
      \param dir vector of track direction on aerogel plane
      \param p momentum of track on aerogel plane
      \param type PDG id of particle
      \param trackID geant4 track id
     */
    ARICHTrack(TVector3 r, TVector3 dir, double p, int type, int trackID) :
      m_originalPosition(r),
      m_originalDirection(dir),
      m_originalMomentum(p),
      m_reconstructedPosition(TVector3(0, 0, 0)),
      m_reconstructedDirection(TVector3(0, 0, 0)),
      m_reconstructedMomentum(-1),
      m_PDGCharge(0),
      m_PDGEncoding(type),
      m_trackID(trackID),
      m_extHitID(-1),
      m_identity(-1),
      m_aeroIndex(-1),
      m_flag(1) {
      for (int i = 0; i < c_noOfHypotheses; i++) {
        m_lkh[i]  = 0;
        m_expectedPhotons[i] = 0;
        m_detectedPhotons[i] = 0;
        m_acc[i]  = 0;
      }
    };

    //! Constructor from hit in 'ext' module
    /*!
      \param extHit track hit on aerogel aluminum support plate
      \param charge charge of the reconstructed, fitted particle
      \param pdgCode In case of simulation; PDG code of the MC generated particle.
      \param trackID number of track in event
      \param aeroHitIndex Used to create relation from ARICHAeroHit(simulation) to ARICHLikelihood.
    */
    ARICHTrack(const ExtHit* extHit, int charge, int pdgCode, int trackID, int aeroHitIndex);

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

    //! returns track id
    int  getTrackID() const {return m_trackID;};

    //! returns track id from ext
    int getHitID() const {return m_extHitID;};

    //! return particle index (0 electron, 1 muon, 2 pion, 3 kaon, 4 proton, -1 else)
    int getIdentity() const {return m_identity;};

    //! returns aerogel hit (MC simulation) index
    int getAeroIndex() const {return m_aeroIndex;};

    //! returns value of likelihood function for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double getLikelihood(int i) const {return m_lkh[i]; };

    //! calculates likelihoods for 5 hypotheses
    void getLikelihood(double like[]) {like[0] = m_lkh[0]; like[1] = m_lkh[1]; like[2] = m_lkh[2]; like[3] = m_lkh[3]; like[4] = m_lkh[4];};

    //! returns expected number of detected photons for "i" particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double getExpectedPhotons(int i) const {return m_expectedPhotons[i]; };

    //! returns expected number of detected photons for all particle hypotheses
    void getExpectedPhotons(double nphot[]) {nphot[0] = m_expectedPhotons[0]; nphot[1] = m_expectedPhotons[1]; nphot[2] = m_expectedPhotons[2]; nphot[3] = m_expectedPhotons[3]; nphot[4] = m_expectedPhotons[4]; };

    //! returns no. of detected photons in +/- 3 sigma band around the expected cherenkov angle for "i"-th particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    int getDetectedPhotons(int i) const {return m_detectedPhotons[i];};

    //! returns expected number of detected photons for all particle hypotheses
    void getDetectedPhotons(int nphot[]) {nphot[0] = m_detectedPhotons[0]; nphot[1] = m_detectedPhotons[1]; nphot[2] = m_detectedPhotons[2]; nphot[3] = m_detectedPhotons[3]; nphot[4] = m_detectedPhotons[4]; };

    //! returns geometrical acceptance of emitted Cherenkov photons by particle "i" (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    double getGeometricalAcceptance(int i) const {return m_acc[i]; };

    //! returns the likelihood flag. 0 if outside ARICH geometrical acceptance.
    int getFlag() const {return  m_flag; };

    //! sets value of likelihood function for particle hypothesis "i" to "val" (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void setLikelihood(int i, double val) {m_lkh[i] = val; };

    //!  sets value of likelihood function for all particle hypotheses. "imax" is number of particle hypotheses, "val" is array of hypotheses values (val[0] electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void setLikelihood(double* val) {for (int i = 0; i < c_noOfHypotheses; i++) m_lkh[i] = val[i];};

    //! sets theoretically expected number of photons for i-th particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void setExpectedPhotons(int i, double val) {m_expectedPhotons[i] = val; };

    //! sets detected number of photons for i-th particle hypothesis
    /* Photons are detected if they are found to be in the theoretically expected Cherenkov cone
     * within +/- 42mrad (~3sigma, for now fixed).
     */
    void setDetectedPhotons(int i, double val) {m_detectedPhotons[i] = val;};

    //! sets the geometrical acceptance of Cherenkov ring for i-th particle hypothesis (here i is particle index: 0 electron, 1 muon, 2 pion, 3 kaon, 4 proton)
    void setGeometricalAcceptance(int i, double val) {m_acc[i] = val;};

    //! sets the likelihood flag. Set to 0 if outside ARICH geometrical acceptance.
    void setFlag(int fl) { m_flag = fl; };


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

    static const int c_noOfHypotheses = 5; /**< Number of hypothesis to loop over. */

    // track parameter from geant4 simulation
    TVector3 m_originalPosition;          /**< Original position on aerogel plane. */
    TVector3 m_originalDirection;         /**< Original direction on aerogel plane. */
    double   m_originalMomentum;          /**< Original momentum on aerogel plane. */

    // track parameter from tracking
    TVector3 m_reconstructedPosition;     /**< Reconstructed position.  */
    TVector3 m_reconstructedDirection;    /**< Reconstructed direction. */
    double   m_reconstructedMomentum;     /**< Reconstructed momentum. */

    double m_PDGCharge;                   /**< particle charge */
    int    m_PDGEncoding;                 /**< particle PDG id number */
    int    m_trackID;                     /**< track identification number in genfit::Track */
    int    m_extHitID;                    /**< track identification number in ext */
    int    m_identity;                    /**< particle index (0 electron, 1 muon, 2 pion, 3 kaon, 4 proton, -1 else). */
    int    m_aeroIndex;                   /**< Relation index needed to create relation from ARICHAeroHit(MC simulation) to ARICHLikelihood. */
    int    m_flag;                        /**< Flag for calculated likelihood usefulness. */


    double  m_lkh[c_noOfHypotheses];  /**< Value of likelihood function for different particle hypotheses. */
    double  m_expectedPhotons[c_noOfHypotheses]; /**< Number of theoretically expected photons for different particle hypotheses.  */
    int     m_detectedPhotons[c_noOfHypotheses]; /**< Number of detected photons in expected cone. */
    double  m_acc[c_noOfHypotheses];  /**< Geometrical acceptance of expected cherenkov ring for different particle hypotheses. */


    //! converts PDG particle code to particle index
    /*!
      \param PDG particle code
     */
    int Lund2Type(int ipart);



  };
} // namespace Belle2

#endif // ARICHTRACK_H
