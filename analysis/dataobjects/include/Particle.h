/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLE_H
#define PARTICLE_H

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixFSym.h>

#include <vector>

class TClonesArray;

namespace Belle2 {

  // forward declarations
  class ECLCluster;
  class Track;
  class MCParticle;

  /**
   * Class to store reconstructed particles.
   * This class is a common representation of all particle types, e.g.:
   *  - final state particles (FS particles):
   *    - charged kaons/pions/electrons/muons/protons reconstructed as Track
   *    - photons reconstructed as ECLCluster (without associated Track)
   *    - long lived neutral kaons reconstructed in KLM
   *  - composite particles:
   *    - pre-reconstructed Kshort, Lambda, ...
   *    - reconstructed in decays (via combinations)
   *
   * Private members are limited to those which completely define the
   * particle and that are common to all particle types. These are:
   *  - particle mass
   *    - nominal for FS particles
   *    - invariant for composite particles
   *  - momentum vector (px, py, pz)
   *  - position (x, y, z)
   *    - POCA for charged FS particles
   *    - IP for photons, Klong and pi0
   *    - decay vertex for composite particles
   *  - 7x7 error matrix (order is: px, py, pz, E, x, y, z)
   *  - chi^2 probability of the fit (track fit, pi0 mass-constr. fit or vertex fit)
   *  - PDG code
   *  - vector of StoreArray<Particle> indices of daughter particles
   *
   * Additional private members are needed in order to make composite particles
   * (via combinations):
   *  - mdst index of an object from which the FS particle is created
   *  - type of the object from which the particle is created (see EParticleType)
   *  - flavor type (unflavored/flavored) of a decay or flavor type of FS particle
   *
   * Finally, it is possible to store user-defined floating-point values using addExtraInfo() and getExtraInfo(), identified by a string key.
   * These are general purpose functions for collecting additional information about
   * reconstructed particles that can not be included into any existing data object.
   * Different Particle objects can store different sets of values, and there are no hard-coded limitations on the number of values stored.
   */
  class Particle : public RelationsObject {

  public:

    /**
     * particle type enumerators (to be completed when all Mdst dataobjects are defined)
     */
    enum EParticleType {c_Undefined, c_Track, c_ECLCluster, c_KLong , c_MCParticle, c_Composite};

    /** describes flavor type, see getFlavorType(). */
    enum EFlavorType {
      c_Unflavored = 0, /**< Is its own antiparticle or we don't know wether it is a particle/antiparticle. */
      c_Flavored = 1, /**< Is either particle or antiparticle. */
    };

    /**
     * error matrix dimensions and size of 1D representation
     */
    enum {c_DimPosition = 3, c_DimMomentum = 4, c_DimMatrix = 7,
          c_SizeMatrix = c_DimMatrix * (c_DimMatrix + 1) / 2
         };

    /**
     * enumerator used for error matrix handling,
     * shows also in which order the rows (columns) are defined
     */
    enum {c_Px, c_Py, c_Pz, c_E, c_X, c_Y, c_Z};

    /**
     * Default constructor.
     * All private members are set to 0. Particle type is set to c_Undefined.
     */
    Particle();

    /**
     * Constructor from a Lorentz vector and PDG code.
     * All other private members are set to their defalt values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     */
    Particle(const TLorentzVector& momentum, const int pdgCode);

    /**
     * Constructor for final state particles.
     * All other private members are set to their defalt values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     * @param flavorType flavor type
     * @param particleType particle type
     * @param mdstIndex mdst index
     */
    Particle(const TLorentzVector& momentum,
             const int pdgCode,
             EFlavorType flavorType,
             const EParticleType particleType,
             const unsigned mdstIndex);

    /**
     * Constructor for composite particles.
     * All other private members are set to their defalt values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     * @param flavorType decay flavor type
     * @param daughterIndices vector of daughter indices
     */
    Particle(const TLorentzVector& momentum,
             const int pdgCode,
             EFlavorType flavorType,
             const std::vector<int>& daughterIndices);

    /**
     * Constructor from a reconstructed track (mdst object Track);
     * @param track pointer to Track object
     * @param chargedStable Type of charged particle
     */
    Particle(const Track* track,
             const Const::ChargedStable& chargedStable);

    /**
     * Constructor of a photon from a reconstructed ECL cluster that is not matched to any charged track.
     * @param eclCluster pointer to ECLCluster object
     */
    Particle(const ECLCluster* eclCluster);

    /**
     * Constructor from MC particle (mdst object MCParticle)
     * @param MCparticle pointer to MCParticle object
     */
    Particle(const MCParticle* MCparticle);

    /**
     * Destructor
     */
    ~Particle();

  public:

    // setters

    /**
     * Sets Lorentz vector
     * @param p4 Lorentz vector
     */
    void set4Vector(const TLorentzVector& p4) {
      m_px = p4.Px();
      m_py = p4.Py();
      m_pz = p4.Pz();
      m_mass = p4.M();
    }

    /**
     * Sets position (decay vertex)
     * @param vertex position
     */
    void setVertex(const TVector3& vertex) {
      m_x = vertex.X();
      m_y = vertex.Y();
      m_z = vertex.Z();
    };

    /**
     * Sets 7x7 error matrix
     * @param errMatrix 7x7 momentum and vertex error matrix (order: px,py,pz,E,x,y,z)
     */
    void setMomentumVertexErrorMatrix(const TMatrixFSym& errMatrix);

    /**
     * Sets chi^2 probability of fit
     * @param pValue p-value of fit
     */
    void setPValue(float pValue) {
      m_pValue = pValue;
    }

    /**
     * Sets Lorentz vector, position, 7x7 error matrix and p-value
     * @param p4 Lorentz vector
     * @param vertex point (position or vertex)
     * @param errMatrix 7x7 momentum and vertex error matrix (order: px,py,pz,E,x,y,z)
     * @param pValue chi^2 probability of the fit
     */
    void updateMomentum(const TLorentzVector& p4,
                        const TVector3& vertex,
                        const TMatrixFSym& errMatrix,
                        float pValue) {
      set4Vector(p4);
      setVertex(vertex);
      setMomentumVertexErrorMatrix(errMatrix);
      m_pValue = pValue;
    }

    /**
     * Appends index of daughter to daughters index array
     * @param daughter pointer to the daughter particle
     */
    void appendDaughter(const Particle* daughter);

    /**
     * Appends index of daughter to daughters index array
     * @param particleIndex daughter particle store array index
     */
    void appendDaughter(int particleIndex) {
      m_particleType = c_Composite;

      m_daughterIndices.push_back(particleIndex);
    }

    /**
     * Removes index of daughter from daughters index array
     * @param daughter pointer to the daughter particle
     */
    void removeDaughter(const Particle* daughter);

    // getters

    /**
     * Returns PDG code
     * @return PDG code
     */
    int getPDGCode(void) const {
      return m_pdgCode;
    }

    /**
     * Returns particle charge
     * @return particle charge in units of elementary charge
     */
    float getCharge(void) const;

    /**
     * Returns flavor type of the decay (for FS particles: flavor type of particle)
     * @return flavor type (0=unflavored, 1=flavored)
     */
    EFlavorType getFlavorType() const {
      return m_flavorType;
    }

    /**
     * Returns particle type as defined with enum EParticleType
     * @return particle type
     */
    EParticleType getParticleType() const {
      return m_particleType;
    }

    /**
     * Returns 0-based index of MDST store array object (0 for composite particles)
     * @return index of MDST store array object
     */
    unsigned getMdstArrayIndex(void) const {
      return m_mdstIndex;
    }

    /**
     * Returns invariant mass (= nominal for FS particles)
     * @return invariant mass
     */
    float getMass() const {
      return m_mass;
    }

    /**
     * Returns mass error (not implemented yet!)
     * @return mass error
     */
    float getMassError() const { return 0.0; }; // TODO

    /**
     * Returns particle nominal mass
     * @return nominal mass
     */
    float getPDGMass(void) const;

    /**
     * Returns total energy
     * @return total energy
     */
    float getEnergy() const {
      return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz + m_mass * m_mass);
    }

    /**
     * Returns Lorentz vector
     * @return Lorentz vector
     */
    TLorentzVector get4Vector() const {
      TLorentzVector vec;
      vec.SetXYZM(m_px, m_py, m_pz, m_mass);
      return vec;
    }

    /**
     * Returns momentum vector
     * @return momentum vector
     */
    TVector3 getMomentum() const {
      return TVector3(m_px, m_py, m_pz);
    };

    /**
     * Returns momentum magnitude
     * @return momentum magnitude
     */
    float getMomentumMagnitude() const {
      return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
    };

    /**
     * Returns momentum magnitude (same as getMomentumMagnitude but with shorter name)
     * @return momentum magnitude
     */
    float getP() const {
      return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
    };

    /**
     * Returns x component of momentum
     * @return x component of momentum
     */
    float getPx() const {
      return m_px;
    }

    /**
     * Returns y component of momentum
     * @return y component of momentum
     */
    float getPy() const {
      return m_py;
    }

    /**
     * Returns z component of momentum
     * @return z component of momentum
     */
    float getPz() const {
      return m_pz;
    }

    /**
     * Returns vertex position (POCA for charged, IP for neutral FS particles)
     * @return vertex position
     */
    TVector3 getVertex() const {
      return TVector3(m_x, m_y, m_z);
    };

    /**
     * Returns x component of vertex position
     * @return x component of vertex position
     */
    float getX() const {
      return m_x;
    }

    /**
     * Returns y component of vertex position
     * @return y component of vertex position
     */
    float getY() const {
      return m_y;
    }

    /**
     * Returns z component of vertex position
     * @return z component of vertex position
     */
    float getZ() const {
      return m_z;
    }

    /**
     * Returns chi^2 probability of fit if done or -1
     * @return p-value of fit (-1 means no fit done)
     */
    float getPValue() const {
      return m_pValue;
    }

    /**
     * Returns 7x7 error matrix
     * @return 7x7 error matrix (order: px,py,pz,E,x,y,z)
     */
    TMatrixFSym getMomentumVertexErrorMatrix() const;

    /**
     * Returns the 4x4 momentum error matrix
     * @return The 4x4 momentum error matrix (order: px,py,pz,E)
     */
    TMatrixFSym getMomentumErrorMatrix() const;

    /**
     * Returns the 3x3 position error sub-matrix
     * @return The 3x3 position error matrix (order: x,y,z)
     */
    TMatrixFSym getVertexErrorMatrix() const;

    /**
     * Returns unique identifier of final state particle (needed in particle combiner)
     * @return unique identifier of final state particle
     */
    int getMdstSource() const {
      return m_mdstIndex + (m_particleType << 24);
    }

    /**
     * Returns number of daughter particles
     * @return number of daughter particles
     */
    unsigned getNDaughters(void) const {
      return m_daughterIndices.size();
    }

    /**
     * Retruns a vector of store array indices of daughter particles
     * @return vector of store array indices of daughter particle
     */
    const std::vector<int>& getDaughterIndices() const {
      return m_daughterIndices;
    }

    /**
     * Returns a pointer to the i-th daughter particle
     * @param i 0-based index of daughter particle
     * @return Pointer to i-th daughter particles
     */
    const Particle* getDaughter(unsigned i) const;

    /**
     * Returns a vector of pointers to daughter particles
     * @return vector of pointers to daughter particles
     */
    const std::vector<Belle2::Particle*> getDaughters() const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /**
     * Returns a vector of pointers to Final State daughter particles
     * @return vector of pointers to final state daughter particles
     */
    const std::vector<const Belle2::Particle*> getFinalStateDaughters() const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /**
     * Returns a vector of StoreArray indices of given MDST dataobjects
     *
     * @param EParticleType corresponding to a given MDST dataobject
     * @return vector of StoreArray indices of a given MDST dataobjects
     */
    std::vector<int> getMdstArrayIndices(EParticleType type) const;

    /**
     * Returns true if final state ancessors of oParticle overlap
     * @param oParticle pointer to particle
     * @return true if overlap, otherwise false
     */
    bool overlapsWith(const Particle* oParticle) const;

    /**
     * Prints the contents of a Particle object to standard output.
     */
    void print() const;

    /** Return given value if set.
     *
     * throws std::runtime_error if variable is not set.
     */
    float getExtraInfo(const std::string& name) const;

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::runtime_error if variable is already set.
     */
    void addExtraInfo(const std::string& name, float value);


  private:

    // persistent data members
    int m_pdgCode;  /**< PDG code */
    float m_mass;   /**< particle (invariant) mass */
    float m_px;     /**< momentum component x */
    float m_py;     /**< momentum component y */
    float m_pz;     /**< momentum component z */
    float m_x;      /**< position component x */
    float m_y;      /**< position component y */
    float m_z;      /**< position component z */
    float m_errMatrix[c_SizeMatrix]; /**< error matrix (1D representation) */
    float m_pValue;   /**< chi^2 probability of the fit */
    std::vector<int> m_daughterIndices;  /**< daughter particle indices */
    EFlavorType m_flavorType;  /**< flavor type. */
    EParticleType m_particleType;  /**< particle type */
    unsigned m_mdstIndex;  /**< 0-based index of MDST store array object */

    /** Stores associated user defined values.
     *
     * Order is given by string -> index mapping in ParticleExtraInfoMap.
     * entry 0 is reserved specifies which map to use.
     */
    std::vector<float> m_extraInfo;

    // transient data members
    /**
     * Internal pointer to DataStore Array containing the particle belonging to
     * this collection.
     *
     * This is a transient member and will not be written to file. The pointer
     * is set when it is the first time needed.
     */
    TClonesArray* m_arrayPointer; //! transient pointer to particle StoreArray

    // private methods
    /**
     * Resets 7x7 error matrix
     * All elements are set to 0.0
     */
    void resetErrorMatrix();

    /**
     * Stores 7x7 error matrix into private member m_errMatrix
     * @param errMatrix 7x7 error matrix
     */
    void storeErrorMatrix(const TMatrixFSym& errMatrix);

    /**
     * Search the DataStore for the corresponding Particle array.
     *
     * This function is called automatically if the pointer
     * is not set when needed
     */
    void fixArrayPointer() const;

    /**
     * Fill final state particle daughters into a vector
     *
     * Function is called recursively
     * @param fspDaughters vector of daughter particles
     */
    void fillFSPDaughters(std::vector<const Belle2::Particle*>& fspDaughters) const;

    /**
     * sets m_flavorType using m_pdgCode
     */
    void setFlavorType();

    ClassDef(Particle, 6); /**< Class to store reconstructed particles. */
  };

} // end namespace Belle2

#endif
