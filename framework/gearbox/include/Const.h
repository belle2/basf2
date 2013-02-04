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

#include <vector>

class TDatabasePDG;
class TParticlePDG;

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

    class ParticleSet;

    /**
     * The ParticleType class for identifying different particle types.
     *
     * Particle Data Group (PDG) codes are used for specifying types, for a detailed explanation and tables please refer to:
     * https://pdg.web.cern.ch/pdg/2012/mcdata/mc_particle_id_contents.html
     */
    class ParticleType {
    public:

      /**
       * Constructor.
       * @param pdgCode the Particle Data Group code that identifies the particle.
       * @param set     Pointer to set this particle belongs to (or NULL if stand-alone).
       * @param index   Index of this particle in 'set'.
       */
      explicit ParticleType(int pdgCode, const ParticleSet* set = NULL, unsigned int index = 0): m_pdgCode(pdgCode), m_set(set), m_index(index)  {};

      /** Copy constructor.
       *
       *  The created object will be part of the same set.
       */
      ParticleType(const ParticleType& other) : m_pdgCode(other.m_pdgCode), m_set(other.m_set), m_index(other.m_index) { };

      /**
       * Comparison operator to be usable in sets.
       * @param other the type of the other particle for the comparison.
       */
      bool operator < (const ParticleType& other) const;

      /** Test equality. */
      bool operator==(const ParticleType& o) const { return m_pdgCode == o.m_pdgCode; }

      /** Test inequality. */
      bool operator!=(const ParticleType& o) const { return !((*this) == o); }

      /** Prefix increment.
       *
       * Incrementing past the last element of a set will return an invalid particle.
       */
      ParticleType& operator++();

      /** Postfix increment.
       *
       * Incrementing past the last element of a set will return an invalid particle.
       */
      ParticleType operator++(int);

      /**
       * Conversion of ParticleType to ParticleSet.
       */
      operator ParticleSet() const { ParticleSet s; s.add(*this); return s; }

      /** This particle's index in the associated set. */
      unsigned int getIndex() const { return m_index; }

      /**
       * Accessor for ROOT TParticlePDG object.
       * @return The TParticlePDG object for this type of particle.
       */
      const TParticlePDG* getParticlePDG() const;

      /**
       * PDG code.
       * @return The PDG code of the particle.
       */
      int getPDGCode() const {return m_pdgCode;};

      /**
       * Particle mass.
       * @return The mass of the particle.
       */
      double getMass() const;


      //Deprecated, remove soonish
      /** This particle's index in the associated set. */
      unsigned int index() const __attribute__((deprecated)) { return getIndex(); }

      /**
       * PDG code.
       * @return The PDG code of the particle.
       */
      int pdgCode() const __attribute__((deprecated)) {return getPDGCode();};

    private:
      int m_pdgCode;  /**< PDG code of the particle **/
      const ParticleSet* m_set; /**< set this particle belongs to, or NULL if stand-alone. */
      unsigned int m_index;    /**< for m_set != 0, the index in the associated set. */
    };

    /** A set of ParticleType objects, with defined order.
     *
     * Allows easy iteration over a set of particles, e.g. to print indices and PDG codes:
     *
        \code
        B2INFO("index -> PDG code");
        const Const::ParticleSet set = Const::chargedStable;
        for(Const::ParticleType pdgIter = set.begin(); pdgIter != set.end(); ++pdgIter) {
          B2INFO(pdgIter.getIndex() << " -> " << pdgIter.getPDGCode());
        }
        \endcode
     *
     * ParticleSets can be created by merging ParticleType objects or other ParticleSets:
     *
        \code
        Const::ParticleSet set = Const::electron + Const::muon;
        Const::ParticleSet set2 = set + Const::photon;
        \endcode
     *
     */
    class ParticleSet {
    public:
      /** Emtpy constructor. */
      ParticleSet() { };

      /** Copy constructor to make sure particles belong to correct set. */
      ParticleSet(const ParticleSet& other) {
        for (ParticleType pdgIter = other.begin(); pdgIter != other.end(); ++pdgIter) {
          add(pdgIter);
        }
      }

      /** Add a copy of the given ParticleType to this set.
       *
       *  If the set already contains the given particle, it remains unchanged.
       */
      void add(const ParticleType& p);

      /** Returns true if and only if the set contains 'p'. */
      bool contains(const ParticleType& p) const;

      /** Return particle at given index, or end() if out of range. */
      const ParticleType& at(unsigned int index) const {
        if (index < m_particles.size())
          return m_particles[index];
        return end();
      }

      /** Returns first particle. */
      ParticleType begin() const {
        if (m_particles.empty())
          return end();
        return m_particles[0];
      }

      /** Returns an invalid particle to check if iteration should be stopped. */
      const ParticleType& end() const {
        return invalidParticle;
      }

      /** Returns particle in set with given PDG code, or invalidParticle if not found. */
      const ParticleType& find(int pdg) const {
        for (ParticleType pdgIter = begin(); pdgIter != end(); ++pdgIter)
          if (pdgIter.getPDGCode() == pdg)
            return m_particles[pdgIter.getIndex()];

        return invalidParticle;
      }
    private:
      std::vector<ParticleType> m_particles; /**< Actual particles. */
    };

    /** Provides a type-safe way to pass members of the chargedStable set.
     *
     * As the defined static members are members of this set, this also defines
     * a fixed index from 0 to 4 for each particle.
     *
     * Conversion from a general ParticleType is not possible, so users have
     * to use Const::ChargedStable::electron instead of Const::electron.
     */
    class ChargedStable : public ParticleType {
    public:
      static const ChargedStable electron;  /**< electron particle */
      static const ChargedStable muon;      /**< muon particle */
      static const ChargedStable pion;      /**< charged pion particle */
      static const ChargedStable kaon;      /**< charged kaon particle */
      static const ChargedStable proton;    /**< proton particle */
    private:
      /** Constructor (private to disallow conversions). */
      ChargedStable(const ParticleType& p): ParticleType(p) { }
    };

    static const ParticleSet chargedStable; /**< set of charged stable particles */


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
    static const ParticleType invalidParticle;     /**< Invalid particle, used internally. */

    static const double electronMass;    /**< electron mass */
    static const double muonMass;        /**< muon mass */
    static const double pionMass;        /**< charged pion mass */
    static const double kaonMass;        /**< charged kaon mass */
    static const double protonMass;      /**< proton mass */
    static const double pi0Mass;         /**< neutral pion mass */
    static const double neutronMass;     /**< neutron mass */
    static const double K0Mass;          /**< neutral kaon mass */

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
    class EvtGenDatabasePDG {
    public:
      static TDatabasePDG* instance();  /**< Instance method that loads the EvtGen table. */
    private:
      /**
       * @{
       * no instances allowed.
       */
      EvtGenDatabasePDG();
      /** @} */
    };
  };

  /**
   * Combination of particle sets.
   */
}

Belle2::Const::ParticleSet operator + (const Belle2::Const::ParticleSet& firstSet, const Belle2::Const::ParticleSet& secondSet);

#endif /* CONST_H */
