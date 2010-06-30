/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4PRIMARYGENERATORACTION_H
#define B4PRIMARYGENERATORACTION_H

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VPrimaryGenerator.hh"
#include "G4ParticleGun.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"

namespace Belle2 {

  class B4PrimaryGeneratorMessenger;

//! The Class for Implementing a Simple Particle Generator
  /*! This class inherits from G4VUserPrimaryGeneratorAction in GEANT4.
      It implements a simple particle generator.
      Using this generator, you can generate one or more particles in a event.
      You can also set some properties on generated particles,
      like particle type, momentum range, polar angle range, azimuth angle range, generated position and so on.
  */

  class B4PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {

  public:

    //! Constructor
    B4PrimaryGeneratorAction();

    //! Destructor
    ~B4PrimaryGeneratorAction();

  public:

    //! The method to generate particles
    /*!
        \param anEvent The pointer of G4Event.
    */
    void GeneratePrimaries(G4Event* anEvent);

    //! The method to set generator name
    /*!
        \param generatorName Generator name.
    */
    void setGeneratorName(G4String generatorName) { m_generatorName = generatorName; }

    //! The method to set number of particles you want to generate
    /*!
        \param np The number of generated particles.
    */
    void setNParticle(G4int np) { m_nParticle = np; }

    //! The method to set generated particle type
    /*!
        \param pname The particle name.
    */
    void setParticleName(G4String pname) { m_particleName = pname; }

    //! The method to set minimal cosine value of polar angle
    /*!
        \param minCos Minimal cosine value of polar angle.
    */
    void setMinCos(G4double minCos) { m_minCos = minCos; }

    //! The method to set maximal cosine value of polar angle
    /*!
        \param maxCos Maximal cosine value of polar angle.
    */
    void setMaxCos(G4double maxCos) { m_maxCos = maxCos; }

    //! The method to set minimal value of azimuth angle
    /*!
        \param phiStart Mininal value of azimuth angle.
    */
    void setPhiStart(G4double phiStart) { m_phiStart = phiStart;}

    //! The method to set maximal value of azimuth angle
    /*!
        \param phiEnd Maximal value of azimuth angle.
    */
    void setPhiEnd(G4double phiEnd) { m_phiEnd = phiEnd; }

    //! The method to set momentum of generated particles
    /*!
        \param pMomentum The central momentum of generated particles.
    */
    void setMomentum(G4double pMomentum) { m_pMomentum = pMomentum; }

    //! The method to smear momentum of generated particles based on the flat function
    /*!
        \param deltaP The momentum range, the final momentum should be m_pMomentum +/- m_deltaP.
    */
    void setDeltaP(G4double deltaP) { m_deltaP = deltaP; }

    //! The method to set x position of generated particles at IP
    /*!
        \param posX X position at IP.
    */
    void setPosX(G4double posX) {m_posX = posX;}

    //! The method to set Y position of generated particles at IP
    /*!
        \param posY Y position at IP.
    */
    void setPosY(G4double posY) { m_posY = posY; }

    //! The method to set position in z direction of generated particles
    /*!
        \param posZ Z position at IP.
    */
    void setPosZ(G4double posZ) { m_posZ = posZ; }

    //! The method to set HEPEvt file name
    /*!
        \param HEPEvtFileName HEPEvt file name.
    */
    void setHEPEvtFileName(G4String HEPEvtFileName) { m_HEPEvtFileName = HEPEvtFileName; }

  private:

    G4String m_generatorName;                  /*!< Generator name. */

    G4VPrimaryGenerator* m_HEPEvt;             /*!< Pointer that points primary generator class. */
    G4String m_HEPEvtFileName;                 /*!< HEPEvt file name. */

    G4ParticleGun* m_particleGun;              /*!< Pointer that points class G4ParticleGun. */
    B4PrimaryGeneratorMessenger* m_messenger;  /*!< Pointer that points class B4PrimaryGeneratorMessenger. */

    G4int m_nParticle;                         /*!< The number of generated particles. */
    G4String m_particleName;                   /*!< Generated particle type. */
    G4double m_minCos;                         /*!< The minimal cosine value of polar angle. */
    G4double m_maxCos;                         /*!< The maximal cosine value of polar angle. */
    G4double m_phiStart;                       /*!< The minimal azimuth angle. */
    G4double m_phiEnd;                         /*!< The maximal azimuth angle. */
    G4double m_pMomentum;                      /*!< The central momentum of generated particles. */
    G4double m_deltaP;                         /*!< Momentum range */
    G4double m_posX;                           /*!< X position at IP */
    G4double m_posY;                           /*!< Y position at IP */
    G4double m_posZ;                           /*!< Z position at IP */
  };

} // end namespace Belle2

#endif /* B4PRIMARYGENERATORACTION_H */
