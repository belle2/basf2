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
#include <TMatrixFSymfwd.h>

#include <vector>

class TClonesArray;

namespace Belle2 {

  // forward declarations
  class ECLCluster;
  class KLMCluster;
  class Track;
  class TrackFitResult;
  class MCParticle;
  class PIDLikelihood;

  /**
   * Class to store reconstructed particles.
   * This class is a common representation of all particle types, e.g.:
   *  - final state particles (FS particles):
   *    - charged kaons/pions/electrons/muons/protons reconstructed as Track
   *    - photons reconstructed as ECLCluster (without associated Track)
   *    - long lived neutral kaons reconstructed in KLM (as KLMCluster without associated Track)
   *  - composite particles:
   *    - pre-reconstructed V0 particles: Kshort, Lambda baryon, converted photon, ...
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
    enum EParticleType {c_Undefined, c_Track, c_ECLCluster, c_KLMCluster, c_MCParticle, c_Composite};

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
     * All other private members are set to their default values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     */
    Particle(const TLorentzVector& momentum, const int pdgCode);

    /**
     * Constructor for final state particles.
     * All other private members are set to their default values (0).
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
     * All other private members are set to their default values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     * @param flavorType decay flavor type
     * @param daughterIndices indices of daughters in StoreArray<Particle>
     * @param arrayPointer pointer to store array which stores the daughters, if the particle itself is stored in the same array the pointer can be automatically determined
     */
    Particle(const TLorentzVector& momentum,
             const int pdgCode,
             EFlavorType flavorType,
             const std::vector<int>& daughterIndices,
             TClonesArray* arrayPointer = nullptr);

    /**
     * Constructor from a reconstructed track (mdst object Track);
     * @param track pointer to Track object
     * @param chargedStable Type of charged particle
     */
    Particle(const Track* track,
             const Const::ChargedStable& chargedStable);

    /**
     * Constructor from a reconstructed Track given as TrackFitResult.
     * To be used to create Particle objects from V0 daughters.
     * @param trackArrayIndex track StoreArray index
     * @param trackFit pointer to TrackFitResult object
     * @param chargedStable Type of charged particle
     * @param chargedStableUsedForFit Type of particle which has been used in the track fit.
     *        This can be different as chargedStable as we don't fit all tracks with
     *        all hypothesis.
     */
    Particle(const int trackArrayIndex,
             const TrackFitResult* trackFit,
             const Const::ChargedStable& chargedStable,
             const Const::ChargedStable& chargedStableUsedForFit);

    /**
     * Constructor of a photon from a reconstructed ECL cluster that is not matched to any charged track.
     * @param eclCluster pointer to ECLCluster object
     */
    explicit Particle(const ECLCluster* eclCluster);

    /**
     * Constructor of a KLong from a reconstructed KLM cluster that is not matched to any charged track.
     * @param klmCluster pointer to KLMCluster object
     */
    explicit Particle(const KLMCluster* klmCluster);

    /**
     * Constructor from MC particle (mdst object MCParticle)
     * @param MCparticle pointer to MCParticle object
     */
    explicit Particle(const MCParticle* MCparticle);

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
    void set4Vector(const TLorentzVector& p4)
    {
      m_px = p4.Px();
      m_py = p4.Py();
      m_pz = p4.Pz();
      m_mass = p4.M();
    }

    /**
     * Sets position (decay vertex)
     * @param vertex position
     */
    void setVertex(const TVector3& vertex)
    {
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
    void setPValue(float pValue)
    {
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
                        float pValue)
    {
      set4Vector(p4);
      setVertex(vertex);
      setMomentumVertexErrorMatrix(errMatrix);
      m_pValue = pValue;
    }

    /**
     * Updates particle mass with the mass of the particle corresponding to the given PDG.
     * @param pdgCode PDG code of the particle with the desired mass
     */
    void updateMass(const int pdgCode);

    /**
     * Appends index of daughter to daughters index array
     * @param daughter pointer to the daughter particle
     */
    void appendDaughter(const Particle* daughter);

    /**
     * Appends index of daughter to daughters index array
     * @param particleIndex index of daughter in StoreArray<Particle>
     */
    void appendDaughter(int particleIndex)
    {
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
    int getPDGCode(void) const
    {
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
    EFlavorType getFlavorType() const
    {
      return m_flavorType;
    }

    /**
     * Returns particle type as defined with enum EParticleType
     * @return particle type
     */
    EParticleType getParticleType() const
    {
      return m_particleType;
    }

    /**
     * Returns 0-based index of MDST store array object (0 for composite particles)
     * @return index of MDST store array object
     */
    unsigned getMdstArrayIndex(void) const
    {
      return m_mdstIndex;
    }

    /**
     * Returns invariant mass (= nominal for FS particles)
     * @return invariant mass
     */
    float getMass() const
    {
      return m_mass;
    }

    /**
     * Returns uncertaint on the invariant mass
     * (requiers valid momentum error matrix)
     * @return mass error
     */
    //float getMassError() const;

    /**
     * Returns particle nominal mass
     * @return nominal mass
     */
    float getPDGMass(void) const;

    /**
     * Returns total energy
     * @return total energy
     */
    float getEnergy() const
    {
      return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz + m_mass * m_mass);
    }

    /**
     * Returns Lorentz vector
     * @return Lorentz vector
     */
    TLorentzVector get4Vector() const
    {
      TLorentzVector vec;
      vec.SetXYZM(m_px, m_py, m_pz, m_mass);
      return vec;
    }

    /**
     * Returns momentum vector
     * @return momentum vector
     */
    TVector3 getMomentum() const
    {
      return TVector3(m_px, m_py, m_pz);
    };

    /**
     * Returns momentum magnitude
     * @return momentum magnitude
     */
    float getMomentumMagnitude() const
    {
      return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
    };

    /**
     * Returns momentum magnitude (same as getMomentumMagnitude but with shorter name)
     * @return momentum magnitude
     */
    float getP() const
    {
      return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
    };

    /**
     * Returns x component of momentum
     * @return x component of momentum
     */
    float getPx() const
    {
      return m_px;
    }

    /**
     * Returns y component of momentum
     * @return y component of momentum
     */
    float getPy() const
    {
      return m_py;
    }

    /**
     * Returns z component of momentum
     * @return z component of momentum
     */
    float getPz() const
    {
      return m_pz;
    }

    /**
     * Returns vertex position (POCA for charged, IP for neutral FS particles)
     * @return vertex position
     */
    TVector3 getVertex() const
    {
      return TVector3(m_x, m_y, m_z);
    };

    /**
     * Returns x component of vertex position
     * @return x component of vertex position
     */
    float getX() const
    {
      return m_x;
    }

    /**
     * Returns y component of vertex position
     * @return y component of vertex position
     */
    float getY() const
    {
      return m_y;
    }

    /**
     * Returns z component of vertex position
     * @return z component of vertex position
     */
    float getZ() const
    {
      return m_z;
    }

    /**
     * Returns chi^2 probability of fit if done or -1
     * @return p-value of fit (nan means no fit done)
     */
    float getPValue() const
    {
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
    int getMdstSource() const;

    /**
     * Returns number of daughter particles
     * @return number of daughter particles
     */
    unsigned getNDaughters(void) const
    {
      return m_daughterIndices.size();
    }

    /**
     * Retruns a vector of store array indices of daughter particles
     * @return vector of store array indices of daughter particle
     */
    const std::vector<int>& getDaughterIndices() const
    {
      return m_daughterIndices;
    }

    /**
     * Returns a pointer to the i-th daughter particle
     * @param i 0-based index of daughter particle
     * @return Pointer to i-th daughter particles
     */
    const Particle* getDaughter(unsigned i) const;

    /** Apply a function to all daughters of this particle
     *
     * @param function function object to run on each daugther. If this
     *    function returns true the processing will be stopped immeddiately.
     * @param recursive if true go through all daughters of daughters as well
     * @param includeSelf if true also apply the function to this particle
     * @return true if the function returned true for any of the particles it
     *    was applied to
     */
    bool forEachDaughter(std::function<bool(const Particle*)> function,
                         bool recursive = true, bool includeSelf = true) const;

    /**
     * Returns a vector of pointers to daughter particles
     * @return vector of pointers to daughter particles
     */
    std::vector<Belle2::Particle*> getDaughters() const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /**
     * Returns a vector of pointers to Final State daughter particles
     * @return vector of pointers to final state daughter particles
     */
    std::vector<const Belle2::Particle*> getFinalStateDaughters() const;
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
     * Returns true if this Particle and oParticle are copies of each other.
     * Copies are defined as:
     *  o) The decay chain of both Particles is exactly the same
     *  o) Both Particles are constructed from exactly the same final state particles
     *
     *   - Examples: M1 -> (C1 -> F1 F2) (C2 -> F3 F4)
     *               M2 -> (C1 -> F1 F2) (C2 -> F3 F5)
     *               M3 -> (C1 -> F1 F2) F3 F4
     *               M4 -> (C1 -> F1 F2) (C2 -> F3 F4) and M4 is kinematically fitted (its momentum updated)
     *
     * M1 and M2 are not copies since condition 2 is not fulfilled.
     * M1 and M3 are not copies since condition 1 is not fulfilled.
     * M1 and M4 are copies since both conditions are fulfilled.
     *
     * @param oParticle pointer to other particle
     * @return true if particles are copies of each-other, otherwise false
     */
    bool isCopyOf(const Particle* oParticle) const;

    /**
     * Returns the pointer to the Track object that was used to create this Particle (ParticleType == c_Track).
     * NULL pointer is returned, if the Particle was not made from Track.
     * @return const pointer to the Track
     */
    const Track* getTrack() const;

    /**
     * Returns the pointer to the PIDLikelihood object that is related to the Track, which
     * was used to create this Particle (ParticleType == c_Track).
     * NULL pointer is returned, if the Particle was not made from Track or if the Track has no
     * relation to the PIDLikelihood
     *
     * @return const pointer to the Track
     */
    const PIDLikelihood* getPIDLikelihood() const;

    /**
     * Returns the pointer to the ECLCluster object that was used to create this Particle (ParticleType == c_ECLCluster).
     * NULL pointer is returned, if the Particle was not made from ECLCluster.
     * @return const pointer to the ECLCluster
     */
    const ECLCluster* getECLCluster() const;

    /**
     * Returns the pointer to the KLMCluster object that was used to create this Particle (ParticleType == c_KLMCluster).
     * NULL pointer is returned, if the Particle was not made from KLMCluster.
     * @return const pointer to the KLMCluster
     */
    const KLMCluster* getKLMCluster() const;

    /**
     * Returns the pointer to the MCParticle object that was used to create this Particle (ParticleType == c_MCParticle).
     * NULL pointer is returned, if the Particle was not made from MCParticle.
     * @return const pointer to the MCParticle
     */
    const MCParticle* getMCParticle() const;

    /** Return name of this particle. */
    virtual std::string getName() const;

    /** Return a short summary of this object's contents in HTML format. */
    virtual std::string getInfoHTML() const;

    /**
     * Prints the contents of a Particle object to standard output.
     */
    void print() const;

    /**
     * Remove all stored extra info fields
     */
    void removeExtraInfo();

    /** Return given value if set.
     *
     * throws std::runtime_error if variable is not set.
     */
    float getExtraInfo(const std::string& name) const;

    /** Return wether the extra info with the given name is set. */
    bool hasExtraInfo(const std::string& name) const;

    /** Return the id of the associated ParticleExtraInfoMap or -1 if no map is set. */
    int getExtraInfoMap() const
    {
      return !m_extraInfo.empty() ? m_extraInfo[0] : -1;
    }

    /** Return the size of the extra info array */
    unsigned int getExtraInfoSize() const
    {
      return m_extraInfo.size();
    }

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::runtime_error if variable isn't set.
     */
    void setExtraInfo(const std::string& name, float value);

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::runtime_error if variable is already set.
     */
    void addExtraInfo(const std::string& name, float value);

    /** Returns the pointer to the store array which holds the daughter particles
     *
     *  \warning TClonesArray is dangerously easy to misuse, please avoid.
     */
    TClonesArray* getArrayPointer() const
    {
      if (!m_arrayPointer)
        m_arrayPointer = RelationsObject::getArrayPointer();
      return m_arrayPointer;
    }

    /** Return the always positive PDG code which was used for the
     * track fit (if there was a track fit) of this particle. This can
     * be different than the Particle's PDG id as not all mass hypothesis
     * are fitted during the reconstruction.
     */
    int getPDGCodeUsedForFit()
    {
      return std::abs(m_pdgCodeUsedForFit);
    }

    /**
     * Returns true if the type represented by this Particle object
     * was used use as a mass hypothesis during the track of this Particle's
     * parameters.
     */
    bool wasExactFitHypothesisUsed() const
    {
      return std::abs(m_pdgCodeUsedForFit) == std::abs(m_pdgCode);
    }

  private:

    // persistent data members
    int m_pdgCode;  /**< PDG code */
    int m_pdgCodeUsedForFit = 0; /**< PDG code used for the track fit */
    float m_mass;   /**< particle (invariant) mass */
    float m_px;     /**< momentum component x */
    float m_py;     /**< momentum component y */
    float m_pz;     /**< momentum component z */
    float m_x;      /**< position component x */
    float m_y;      /**< position component y */
    float m_z;      /**< position component z */
    float m_errMatrix[c_SizeMatrix]; /**< error matrix (1D representation) */
    float m_pValue;   /**< chi^2 probability of the fit. Default is nan */
    std::vector<int> m_daughterIndices;  /**< daughter particle indices */
    EFlavorType m_flavorType;  /**< flavor type. */
    EParticleType m_particleType;  /**< particle type */
    unsigned m_mdstIndex;  /**< 0-based index of MDST store array object */

    /**
     * Identifier that can be used to identify whether the particle is unqiue
     * or is a copy or representation of another. For example a kaon and pion
     * particles constructed from the same Track are representations of the
     * same physical object in the detector and cannot be used in the reconstruction
     * of the same decay chain
     */
    int m_identifier = -1;

    /** Stores associated user defined values.
     *
     * Order is given by string -> index mapping in ParticleExtraInfoMap.
     * entry 0 is reserved specifies which map to use.
     */
    std::vector<float> m_extraInfo;

    // transient data members
    /**
     * Internal pointer to DataStore array containing the daughters of this particle.
     *
     * This is a transient member and will not be written to file. The pointer
     * is set by getArrayPointer() when first called.
     */
    mutable TClonesArray* m_arrayPointer; //!

    // private methods
    /**
     * Sets the momentum, position and error matrix for this particle (created from charged Track)
     */
    void setMomentumPositionErrorMatrix(const TrackFitResult* trackFit);

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
     * Fill final state particle daughters into a vector
     *
     * Function is called recursively
     * @param fspDaughters vector of daughter particles
     */
    void fillFSPDaughters(std::vector<const Belle2::Particle*>& fspDaughters) const;

    /**
     * Fill vector with (PDGCode, MdstSource) pairs for the entire decay chain.
     * Used to determine if two Particles are copies or not.
     *
     * Function is called recursively
     * @param decayChain vector of (PDGCode, MdstSource) pairs for each particle in the decay chain of this particle
     */
    // TODO: this can be optimized for speed
    void fillDecayChain(std::vector<int>& decayChain) const;

    /**
     * sets m_flavorType using m_pdgCode
     */
    void setFlavorType();


    /**
     * set mdst array index
     */
    void setMdstArrayIndex(const int arrayIndex);

    ClassDef(Particle, 9); /**< Class to store reconstructed particles. */
    // v8: added identifier, changed getMdstSource
    // v9: added m_pdgCodeUsedForFit

    friend class ParticleSubset;
  };

} // end namespace Belle2

#endif
