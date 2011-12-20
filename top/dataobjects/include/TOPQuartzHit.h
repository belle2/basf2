/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPQUARTZHIT_H
#define TOPQUARTZHIT_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Datastore class that holds information on track parameters at the entrance in aerogel.
  /*!  For now this information comes from ("TOPSensitiveQuartz" sensitive detector). This should be replaced with the information from tracking.
   */


  class TOPQuartzHit : public TObject {
  public:

    //! Empty constructor
    /*! Recommended for ROOT IO */
    TOPQuartzHit():
      m_trackID(-1),
      m_particleID(-1),
      m_charge(0),
      m_position(0, 0, 0),
      m_sposition(0, 0, 0),
      m_momentum(0, 0, 0),
      m_smomentum(0, 0, 0),
      m_length(0.),
      m_globaltime(0.),
      m_localtime(0.),
      m_eL(0.),
      m_muL(0.),
      m_piL(0.),
      m_KL(0.),
      m_pL(0.),
      m_recoflag(0),
      m_detfot(0) {
      /*! does nothing */
    }

    //! Useful Constructor
    /*!
     \param trackID geant4 track id
     \param particleID particle PDG id number
     \param charge the charge of the track
     \param position vector of track position on quartz bar plane
     \param production vertex
     \param momentum vector of track momentum on quarz bar plane
     \param production momentum
     \param eL electron likelihood
     \param eL electron likelihood
     \param eL electron likelihood
     \param eL electron likelihood
     \param eL electron likelihood
     \param recoflag is the flag from reconstruction
     \param detfot is the number of detected photons
     */
    TOPQuartzHit(
      int trackID,
      int particleID,
      int charge,
      TVector3 position,
      TVector3 sposition,
      TVector3 momentum,
      TVector3 smomentum,
      int barID,
      double length,
      double globaltime,
      double localtime,
      double eL,
      double muL,
      double piL,
      double KL,
      double pL,
      int recoflag,
      int detfot
    ) {
      m_trackID = trackID;
      m_particleID = particleID;
      m_charge = charge;
      m_position = position;
      m_sposition = sposition;
      m_momentum = momentum;
      m_smomentum = smomentum;
      m_barID = barID;
      m_length = length;
      m_globaltime = globaltime;
      m_localtime = localtime;
      m_eL = eL;
      m_muL = muL;
      m_piL = piL;
      m_KL = KL;
      m_pL = pL;
      m_recoflag = recoflag;
      m_detfot = detfot;
    }

    //! Get Geant4 track ID
    int getTrackID() const { return m_trackID; }

    //! Get particle PDG identity number
    int getParticleID() const { return m_particleID; }

    //! Get charge of track
    int getCharge() const { return m_charge;}

    //! Get track position (at entrance into bar)
    const TVector3& getPosition() const { return m_position; }

    //! Get vertex position (at entrance into bar)
    const TVector3& getVPosition() const { return m_sposition; }

    //! Get track momentum  (at entrance into bar)
    const TVector3& getMomentum() const { return m_momentum; }

    //! Get track momentum  (at entrance into bar)
    const TVector3& getVMomentum() const { return m_smomentum; }

    //! Get barID
    int getBarID() const { return m_barID;}

    //! Get track length
    double getLength() const { return m_length; }

    //! Get track length
    double getGlobalTime() const { return m_globaltime; }

    //! Get track length
    double getLocalTime() const { return m_localtime; }

    //! get electron likelihood
    double getElectronLikelihood() const { return m_eL;}

    //! get muon likelihood
    double getMuonLikelihood() const { return m_muL;}

    //! get pion likelihood
    double getPionLikelihood() const { return m_piL;}

    //! get kaon likelihood
    double getKaonLikelihood() const { return m_KL;}

    //! get proton likelihood
    double getProtonLikelihood() const { return m_pL;}

    //! get proton likelihood
    double getRecoFlag() const { return m_recoflag;}

    //! get number of detected photons
    int getNPhotons() const { return m_detfot;}

    //! Set Geant4 track ID
    void setTrackID(int trackID) { m_trackID = trackID; }

    //! set track charge
    void setCharge(int charge) {m_charge = charge;}

    //! Set particle PDG identity number
    void setParticleID(int particleId) { m_particleID = particleId; }

    //! Set track position (at entrance in 1. aerogel plane)
    void setPosition(TVector3 position) { m_position = position; }

    //! Set vertex position
    void setVPosition(TVector3 sposition) { m_sposition = sposition; }

    //! Set track momentum  (at entrance in 1. aerogel plane)
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    //! Set track momentum  (at entrance in 1. aerogel plane)
    void setVMomentum(TVector3 smomentum) { m_smomentum = smomentum; }

    //! set track charge
    void setBarID(int barID) {m_barID = barID;}

    //! Set track length
    void setLength(double length) { m_length = length; }

    //! Set electron likelihood
    void setElectronLikelihood(double eL)  {  m_eL = eL;}

    //! Set muon likelihood
    void setMuonLikelihood(double muL)  {  m_muL = muL;}

    //! Set pion likelihood
    void setPionLikelihood(double piL)  {  m_piL = piL;}

    //! Set kaon likelihood
    void setKaonLikelihood(double KL)  {  m_KL = KL;}

    //! Set proton likelihood
    void setProtonLikelihood(double pL)  {  m_pL = pL;}

    //! Set reconstruction flag
    void setRecoFlag(int recoflag) {m_recoflag = recoflag;}

    //! set the number of detected photons
    void setNPhotons(int detfot) {m_detfot = detfot;}


  private:

    int m_trackID;             /**< G4 id of track */
    int m_particleID;          /**< particle PDG id number */
    int m_charge;               /**< charge of track */
    TVector3 m_position;       /**< track position (at entrance in 1. aerogel plane) */
    TVector3 m_sposition;       /**< track position (at entrance in 1. aerogel plane) */
    TVector3 m_momentum;       /**< track position (at entrance in 1. aerogel plane) */
    TVector3 m_smomentum;       /**< track position (at entrance in 1. aerogel plane) */
    int m_barID;                /**< barid of the hit */
    double m_length;          /**< track length */
    double m_globaltime;
    double m_localtime;
    double m_eL;            /**< electron likelihood */
    double m_muL;            /**< muon likelihood */
    double m_piL;            /**< pion likelihood */
    double m_KL;            /**< kaon likelihood */
    double m_pL;            /**< proton likelihood */
    int m_recoflag;          /**< flag of the TOPrec reconstruction */
    int m_detfot;          /**< number of detected photons */


    ClassDef(TOPQuartzHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif  // TOPQUARTZHIT_H
