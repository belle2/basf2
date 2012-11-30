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
#include <set>

namespace Belle2 {

  /**
   * The Const class for various constants.
   *
   * This class provides a set of constants for the framework.
   *
   * This class cannot be instantiated, use the static members directly.
   * The implemantation can be found in UnitConst.cc
   */
  class Const {

  public:

    /**
     * The enum for identifying the detector components.
     */
    enum EDetector {IR = 0x0001, PXD = 0x0002, SVD = 0x0004, CDC = 0x0008,
                    TOP = 0x0010, ARICH = 0x0020, ECL = 0x0040, BKLM = 0x0080,
                    EKLM = 0x0100
                   };

    /**
     * The vector of all detectors.
     */
    static const std::vector<EDetector> detectors;

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
       * Comparison operator to be usable in sets.
       * @param other the type of the other particle for the comparison.
       */
      bool operator < (const ParticleType& other) const;

      /**
       * Conversion of ParticleType to ParticleSet.
       */
      operator std::set<ParticleType>() const {return std::set<ParticleType>(this, this + 1);}

      /**
       * Accessor for root TParticlePDG object.
       * @return The root TParticlePDG object for this type of particle.
       */
      const TParticlePDG* particlePDG() const;

      /**
       * Particle mass.
       * @return The mass of the particle.
       */
      double mass() const;

    private:
      int m_pdgCode;  /**< PDG code of the particle **/
    };

    static const ParticleType electron;  /**< electron particle */
    static const ParticleType muon;      /**< muon particle */
    static const ParticleType pion;      /**< charged pion particle */
    static const ParticleType kaon;      /**< charged kaon particle */
    static const ParticleType proton;    /**< proton particle */
    static const ParticleType photon;    /**< photon particle */
    static const ParticleType pi0;       /**< neutral pion particle */
    static const ParticleType neutron;   /**< neutron particle */
    static const ParticleType Kshort;    /**< K^0_S particle */
    static const ParticleType Klong;     /**< K^0_L particle */

    static const double electronMass;    /**< electron mass */
    static const double muonMass;        /**< muon mass */
    static const double pionMass;        /**< charged pion mass */
    static const double kaonMass;        /**< charged kaon mass */
    static const double protonMass;      /**< proton mass */
    static const double pi0Mass;         /**< neutral pion mass */
    static const double neutronMass;     /**< neutron mass */
    static const double K0Mass;          /**< neutral kaon mass */

    typedef std::set<ParticleType> ParticleSet;  /**< typedef for sets of particles */
    static const ParticleSet chargedStable;      /**< set of charged stable particles */

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

  /**
   * Combination of particle sets.
   */
}

Belle2::Const::ParticleSet operator + (const Belle2::Const::ParticleSet& firstSet, const Belle2::Const::ParticleSet& secondSet);

#endif /* CONST_H */
