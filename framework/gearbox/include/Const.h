/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/FrameworkExceptions.h>

#include <TObject.h>

#include <cstddef>
#include <stdexcept>

class TDatabasePDG;
class TParticlePDG;

namespace Belle2 {

  /**
   * This class provides a set of constants for the framework.
   *
   * This class cannot be instantiated, use the static members directly.
   * The implementation can be found in UnitConst.cc
   */
  class Const {

  public:

    /**
     * Enum for identifying the detector components (detector and subdetector).
     * Edetector = subdetector * 256 + detector
     */
    enum EDetector {
      invalidDetector = 0x0,
      PXD = 0x1,
      SVD = 0x2,
      CDC = 0x3,
      TOP = 0x4,
      ARICH = 0x5,
      ECL = 0x6,
      KLM = 0x7,
      BKLM = 0x100 + KLM,
      EKLM = 0x200 + KLM,
      IR = 0x8,
      TRG = 0x9,
      DAQ = 0xA,
      BEAST = 0xB,
      TEST = 0xC,
    };

    /**
     * The DetectorSet class for sets of detector IDs in the form of EDetector
     * values. Only detectors may be included, not subdetectors.
     */
    class DetectorSet {
    public:

      /**
       * Default constructor.
       */
      DetectorSet(): m_bits(0) {};

      /**
       * Copy constructor.
       * @param set  The copied set of detector IDs.
       */
      DetectorSet(const DetectorSet& set): m_bits(set.m_bits) {}

      /**
       * Constructor for a set containig one detector ID.
       * @param det  The ID of the detector.
       */
      // cppcheck-suppress noExplicitConstructor
      DetectorSet(EDetector det): m_bits(getBit(det)) {}

      /**
       * Destructor.
       */
      virtual ~DetectorSet() {};

      /**
       * Addition of another set to this one.
       * @param set  The other set of detector IDs.
       */
      DetectorSet& operator += (const DetectorSet& set) {m_bits |= set.m_bits; return *this;}

      /**
       * Subtraction of another set from this one.
       * @param set  The other set of detector IDs.
       */
      DetectorSet& operator -= (const DetectorSet& set) {m_bits &= ~set.m_bits; return *this;}

      /**
       * Equality operator.
       * @param set  The other set of detector IDs.
       */
      bool operator == (const DetectorSet& set) const {return m_bits == set.m_bits;}

      /**
       * Inequality operator.
       * @param set  The other set of detector IDs.
       */
      bool operator != (const DetectorSet& set) const {return m_bits != set.m_bits;}

      /**
       * Check whether this set contains another set.
       * @param set  The other set of detector IDs.
       */
      bool contains(const DetectorSet& set) const {return (m_bits & set.m_bits) == set.m_bits;}

      /**
       * Getter for the index of a given detector in this set.
       * @param det  The detector ID.
       * @return     Index of the detector ID in this set, or -1 if the detector ID is not in this set.
       */
      int getIndex(EDetector det) const;

      /**
       * Accessor for a detector ID in this set.
       * @param index  The index in the set.
       * @return       The detector ID at the given index, or Const::invalidDetector if the index is out of range.
       */
      EDetector operator [](int index) const;

      /**
       * Getter for number of detector IDs in this set.
       */
      size_t size() const;

    private:

      /**
       * Constructor.
       * @param bits  The internal representation of the set as bit pattern.
       */
      explicit DetectorSet(unsigned short bits): m_bits(bits) {};

      /**
       * Conversion of detector ID to bit pattern.
       * @param det  The detector ID.
       * @return     The bit pattern representing the given detector ID.
       */
      static unsigned short getBit(EDetector det);

      /**
       * Conversion of bit pattern to detector ID.
       * @param bit  The bit pattern.
       * @return     The detector ID corresponding to the given bit pattern.
       */
      static EDetector getDetector(unsigned short bit);

      unsigned short m_bits;  /**< The internal representation of the set as bit pattern. */

      ClassDef(DetectorSet, 1) /**< A set of Detector IDs. */
    };

    /**
     * A class for sets of detector IDs whose content is limited to restricted set of valid detector IDs.
     */
    template <class ASetType> class RestrictedDetectorSet: public DetectorSet {
    public:

      /**
       * Exception that is thrown if there is the attempt to add an invalid detector ID to a restricted set.
       */
      BELLE2_DEFINE_EXCEPTION(InvalidDetectorTypeError, "The given detector is not a valid element of the set!")

      /**
       * Default constructor.
       */
      RestrictedDetectorSet(): DetectorSet() {}

      /**
       * (Copy) constructor.
       * @param set  The copied set of detector IDs.
       */
      // cppcheck-suppress noExplicitConstructor
      RestrictedDetectorSet(const DetectorSet& set): DetectorSet(set) {checkSet();}

      /**
       * Constructor for a set containg one detector ID.
       * @param det  The detector ID.
       */
      // cppcheck-suppress noExplicitConstructor
      RestrictedDetectorSet(EDetector det): DetectorSet(det) {checkSet();}

      /**
       * Addition of another set to this one.
       * @param set  The other set of detector IDs.
       */
      RestrictedDetectorSet& operator += (const DetectorSet& set) {DetectorSet::operator +=(set); checkSet(); return *this;}

      /**
       * Accessor for the set of valid detector IDs.
       * @return  The set of valid detector IDs.
       */
      static DetectorSet set() {return ASetType::set();}

    private:

      /**
       * Check of set validity. If an invalid detector ID is found an exception is thrown.
       */
      void checkSet() const {if (!ASetType::set().contains(*this)) throw InvalidDetectorTypeError();}
    };


    /**
     * A class that defines the valid set of VXD detectors.
     * To be used as a template argument for RestrictedDetectorSet.
     */
    class VXDDetectors {
    public:
      static DetectorSet set() {return c_set;}  /**< Accessor function for the set of valid detectors */
      static const DetectorSet c_set;           /**< The set of valid tracking detectors */
    };
    /** Typedef for set of VXD detectors. */
    typedef RestrictedDetectorSet<VXDDetectors> VXDDetectorSet;

    /**
     * A class that defines the valid set of tracking detectors.
     * To be used as a template argument for RestrictedDetectorSet.
     */
    class TrackingDetectors {
    public:
      static DetectorSet set() {return c_set;}  /**< Accessor function for the set of valid detectors */
      static const DetectorSet c_set;           /**< The set of valid tracking detectors */
    };
    /** Typedef for set of tracking detectors. */
    typedef RestrictedDetectorSet<TrackingDetectors> TrackingDetectorSet;

    /**
     * A class that defines the valid set of PID detectors.
     * To be used as a template argument for RestrictedDetectorSet.
     */
    class PIDDetectors {
    public:
      static DetectorSet set() {return c_set;}  /**< Accessor function for the set of valid detectors */
      static const DetectorSet c_set;           /**< The set of valid PID detectors */
      static const size_t c_size = 6;           /**< Number of PID detectors, temporary workaround */
    };
    /** Typedef for set of PID detectors. */
    typedef RestrictedDetectorSet<PIDDetectors> PIDDetectorSet;

    /**
     * A class that defines the valid set of Cluster detectors.
     * To be used as a template argument for RestrictedDetectorSet.
     */
    class ClusterDetectors {
    public:
      static DetectorSet set() {return c_set;}  /**< Accessor function for the set of valid detectors */
      static const DetectorSet c_set;           /**< The set of valid tracking detectors */
      static const size_t c_size = 2;           /**< Number of PID detectors, temporary workaround */
    };
    /** Typedef for set of VXD detectors. */
    typedef RestrictedDetectorSet<ClusterDetectors> ClusterDetectorSet;


    /**
     * The set of all detectors.
     */
    static const DetectorSet allDetectors;

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
      explicit ParticleType(int pdgCode, const ParticleSet* set = NULL, int index = -1): m_pdgCode(pdgCode), m_set(set),
        m_index(index)  {};

      explicit ParticleType(int pdgCode, int charge, const ParticleSet* set = NULL, int index = -1): m_pdgCode(pdgCode), m_charge(charge),
        m_set(set),
        m_index(index)  {};

      /** Copy constructor.
       *
       *  The created object will be part of the same set.
       */
      ParticleType(const ParticleType& other) : m_pdgCode(other.m_pdgCode), m_charge(other.m_charge), m_set(other.m_set),
        m_index(other.m_index) { };

      /**
       * Comparison operator to be usable in sets.
       * @param other the type of the other particle for the comparison.
       */
      bool operator < (const ParticleType& other) const;

      /** Test equality. */
      bool operator==(const ParticleType& o) const { return m_pdgCode == o.m_pdgCode; }

      /** Test inequality. */
      bool operator!=(const ParticleType& o) const { return !((*this) == o); }

      /** dummy dereferencing for range-based for. */
      ParticleType& operator*() { return *this; }

      /** Prefix increment.
       *
       * Incrementing past the last element of a set will return an invalid particle.
       */
      ParticleType& operator++();

      /** Postfix increment.
       *
       * Incrementing past the last element of a set will yield an invalid particle.
       */
      ParticleType operator++(int);

      /**
       * Conversion of ParticleType to ParticleSet.
       */
      operator ParticleSet() const { ParticleSet s; s.add(*this); return s; }

      /** This particle's index in the associated set.
       *
       * In case the particle has no set, -1 is returned.
       */
      int getIndex() const { return m_index; }

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

      int getCharge() const { return m_charge; };

      /**
       * Particle mass.
       * @return The mass of the particle.
       */
      double getMass() const;


    private:
      int m_pdgCode;  /**< PDG code of the particle **/
      int m_charge;  /**< Electric charge of the particle **/
      const ParticleSet* m_set; /**< set this particle belongs to, or NULL if stand-alone. */
      int m_index;    /**< index in the associated set, -1 if there's no set. */
    };

    /** A set of ParticleType objects, with defined order.
     *
     * Allows easy iteration over a set of particles, e.g. to print indices and PDG codes:
     *
        \code
        B2INFO("index -> PDG code");
        const Const::ParticleSet set = Const::chargedStableSet;
        //via range-based for
        for(const Const::ChargedStable& pdgIter : set) {
          B2INFO(pdgIter.getIndex() << " -> " << pdgIter.getPDGCode());
        }
        //or, using iterator syntax:
        for(Const::ChargedStable pdgIter = set.begin(); pdgIter != set.end(); ++pdgIter) {
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
      ParticleSet(const ParticleSet& other)
      {
        for (ParticleType pdgIter : other) {
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

      /** Returns number of particles in this set. */
      unsigned int size() const { return m_particles.size(); }

      /** Return particle at given index, or end() if out of range. */
      const ParticleType& at(unsigned int index) const
      {
        if (index < m_particles.size())
          return m_particles[index];
        return invalidParticle;
      }

      /** Returns first particle. */
      ParticleType begin() const
      {
        if (m_particles.empty())
          return end();
        return m_particles[0];
      }

      /** Returns an invalid particle to check if iteration should be stopped. */
      ParticleType end() const
      {
        return invalidParticle;
      }

      /** Returns particle in set with given PDG code, or invalidParticle if not found. */
      const ParticleType& find(int pdg) const
      {
        for (ParticleType pdgIter : *this) {
          if (pdgIter.getPDGCode() == pdg)
            return m_particles[pdgIter.getIndex()];
        }

        return invalidParticle;
      }
    private:
      std::vector<ParticleType> m_particles; /**< Actual particles. */
    };

    /** Provides a type-safe way to pass members of the chargedStableSet set.
     *
     * As the defined static members (Const::electron, etc. ) are members of this set,
     * this also defines a fixed index from 0 to 4 for each particle.
     */
    class ChargedStable : public ParticleType {
    public:
      /** Constructor from the more general ParticleType.
       *
       * Throws a runtime_error if p is not in chargedStableSet.
       */
      // cppcheck-suppress noExplicitConstructor
      ChargedStable(const ParticleType& p)
        : ParticleType(chargedStableSet.find(p.getPDGCode()))
      {
        if ((*this) == invalidParticle) {
          throw std::runtime_error("Given ParticleType is not a charged stable particle!");
        }
      }

      /** Constructor from PDG code
       *
       * Throws a runtime_error if pdg is not in chargedStableSet.
       */
      explicit ChargedStable(int pdg)
        : ParticleType(chargedStableSet.find(pdg))
      {
        if ((*this) == invalidParticle) {
          throw std::runtime_error("Given PDG code is not a charged stable particle!");
        }
      }

      static const unsigned int c_SetSize = 11; /**< Number of elements (for use in array bounds etc.) */

    };

    static const ParticleSet chargedStableSet; /**< set of charged stable particles */


    /** Provides a type-safe way to pass members of the clusterSet set.
     *
     * As the defined static members (Const::photon, etc. ) are members of this set,
     * this also defines a fixed index from 0 to 4 for each particle.
     */
    class Cluster : public ParticleType {
    public:
      /** Constructor from the more general ParticleType.
       *
       * Throws a runtime_error if p is not in chargedStableSet.
       */
      // cppcheck-suppress noExplicitConstructor
      Cluster(const ParticleType& p)
        : ParticleType(clusterSet.find(p.getPDGCode()))
      {
        if ((*this) == invalidParticle) {
          throw std::runtime_error("Given ParticleType is not a cluster particle!");
        }
      }

      /** Constructor from PDG code
       *
       * Throws a runtime_error if pdg is not in chargedStableSet.
       */
      explicit Cluster(int pdg)
        : ParticleType(clusterSet.find(pdg))
      {
        if ((*this) == invalidParticle) {
          throw std::runtime_error("Given PDG code is not a cluster particle!");
        }
      }
      static const unsigned int c_SetSize = 6; /**< Number of elements (for use in array bounds etc.) */
    };

    static const ParticleSet clusterSet; /**< set of cluster particles */

    static const ChargedStable electron;  /**< electron particle */
    static const ChargedStable muon;      /**< muon particle */
    static const ChargedStable pion;      /**< charged pion particle */
    static const ChargedStable kaon;      /**< charged kaon particle */
    static const ChargedStable proton;    /**< proton particle */
    static const ChargedStable deuteron;  /**< deuteron particle */
    static const ChargedStable antielectron;  /**< anti-electron particle */
    static const ChargedStable antimuon;      /**< anti-muon particle */
    static const ChargedStable antipion;      /**< anti-charged pion particle */
    static const ChargedStable antikaon;      /**< anti-charged kaon particle */
    static const ChargedStable antiproton;    /**< anti-proton particle */

    static const Cluster clusterphoton;   /**< electron cluster */
    static const Cluster clusterKlong;    /**< K^0_L cluster */
    static const Cluster clusterelectron; /**< electron cluster  */
    static const Cluster clusterpion;     /**< charged hadron cluster */
    static const Cluster clustermuon;     /**< muon cluster */
    static const Cluster clusterjunk;     /**< junk cluster */

    static const ParticleType photon;    /**< photon particle */
    static const ParticleType pi0;       /**< neutral pion particle */
    static const ParticleType neutron;   /**< neutron particle */
    static const ParticleType Kshort;    /**< K^0_S particle */
    static const ParticleType Klong;     /**< K^0_L particle */
    static const ParticleType Lambda;    /**< Lambda particle */
    static const ParticleType antiLambda; /**< Anti-Lambda particle */
    static const ParticleType invalidParticle;     /**< Invalid particle, used internally. */
    /** Unspecified particle, used when no other particle type fits. (no valid PDG code, but different from invalidParticle) */
    static const ParticleType unspecifiedParticle;

    static const double electronMass;    /**< electron mass */
    static const double muonMass;        /**< muon mass */
    static const double pionMass;        /**< charged pion mass */
    static const double kaonMass;        /**< charged kaon mass */
    static const double protonMass;      /**< proton mass */
    static const double deuteronMass;    /**< deuteron mass */
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
    Const() { };
    Const(const Const&) { };
    Const& operator=(const Const&) { return *this; };
    ~Const() {};
    /** @} */

  };
}

/**
 * Combination of detector sets.
 */
Belle2::Const::DetectorSet operator + (const Belle2::Const::DetectorSet& firstSet, const Belle2::Const::DetectorSet& secondSet);

/**
 * Subtraction of one detector sets from another one.
 */
Belle2::Const::DetectorSet operator - (const Belle2::Const::DetectorSet& firstSet, const Belle2::Const::DetectorSet& secondSet);

/**
 * Combination of two detector IDs to a set.
 */
Belle2::Const::DetectorSet operator + (Belle2::Const::EDetector firstDet, Belle2::Const::EDetector secondDet);

/**
 * Combination of particle sets.
 */
Belle2::Const::ParticleSet operator + (const Belle2::Const::ParticleSet& firstSet, const Belle2::Const::ParticleSet& secondSet);
