/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONST_H
#define CONST_H

#include <TDatabasePDG.h>

namespace Belle2 {

  /**
   * The Const class for various constants.
   *
   * This class provides a set of constants for the framework.
   *
   * This class cannot be instantiated, use the static members directly.
   */
  class Const {

  public:

    /**
     * The enum for identifying the detector components.
     */
    enum EDetector {IR, PXD, SVD, CDC, TOP, ARICH, ECL, BKLM, EKLM};

    /**
     * The ParticleType class for identifying different particle types.
     */
    class ParticleType {
    public:

      /**
       * Constructor.
       * @param pdgCode the Particle Data Group code that identifies the particle.
       */
      explicit ParticleType(int pdgCode): m_pdgCode(pdgCode) {};

      /**
       * Accessor for root TParticlePDG object.
       * @return The root TParticlePDG object for this type of particle.
       */
      const TParticlePDG* particlePDG() const;

    private:
      int m_pdgCode;  /** PDG code of the particle **/
    };

    static const ParticleType electron;  /**< electron particle */
    static const ParticleType muon;      /**< muon particle */
    static const ParticleType pion;      /**< pion particle */
    static const ParticleType kaon;      /**< kaon particle */
    static const ParticleType proton;    /**< proton particle */

    static const double electronMass;    /**< electron mass */
    static const double muonMass;        /**< muon mass */
    static const double pionMass;        /**< pion mass */
    static const double kaonMass;        /**< kaon mass */
    static const double protonMass;      /**< proton mass */

    static const double speedOfLight; /**< [cm/ns] */
    static const double kBoltzmann;   /**< Boltzmann constant in GeV/K. */
    static const double ehEnergy;     /**< Energy needed to create an electron-hole pair in Si at std. T. */
    static const double fineStrConst; /**< The fine structure constant. */
    static const double permSi;       /**< Permittivity of Silicon */
    static const double uTherm;       /**< Thermal Voltage at room temperature */
    static const double eMobilitySi;  /**< Electron mobility in intrinsic Silicon at room temperature */

  private:
    /**
     * @{
     * no Const instances allowed.
     */
    Const();
    Const(const Const&);
    Const& operator=(const Const&);
    ~Const() {};
    /** @} */

    /**
     * Helper class for loading the particle properties from EvtGen.
     */
    class EvtGenDatabasePDG: public TDatabasePDG {
    public:
      static TDatabasePDG* instance();  /**< Instance method that loads the EvtGen table. */
    };
  };

}

#endif /* CONST_H */
