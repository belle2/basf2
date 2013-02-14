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

#include <TClonesArray.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixFSym.h>
#include <framework/gearbox/Const.h>

#include <tracking/dataobjects/Track.h>

#include <vector>


namespace Belle2 {

  // forward declarations
  class ECLShower;
  class ECLGamma;
  class ECLPi0;
  // TODO: check with Martin
  //class Track;
  class MCParticle;

  /** \addtogroup dataobjects
   * @{
   */

  /**
   * Class to store reconstructed particles.
   * This class is a common representation of all particle types, e.g.:
   *  - final state particles (FS particles):
   *    - charged kaons/pions/electrons/muons/protons reconstructed as Track
   *    - photons reconstructed as ECLGamma
   *    - long lived neutral kaons reconstructed in KLM
   *  - composite particles:
   *    - pre-reconstructed pi0, Kshort, Lambda
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
   *  - PDG code
   *  - vector of StoreArray<Particle> indices of daughter particles
   *
   * Additional private members are needed to make composite particles (via combinations):
   *  - mdst index of an Object from which the FS particle is created
   *  - type of the Object from which the particle is created (see EParticleType)
   *  - flavor type (unflavored/flavored) of a decay or flavor type of FS particle
   */

  class Particle : public RelationsObject {

  public:

    /**
     * particle type enumerators (to be completed when all Mdst dataobject are defined)
     */
    enum EParticleType {c_Undefined, c_Track, c_ECLGamma, c_KLong, c_Pi0,
                        c_MCParticle, c_Composite
                       };

    /**
     * error matrix dimensions and size of 1D representation
     */
    enum {c_DimPosition = 3, c_DimMomentum = 4, c_DimMatrix = 7,
          c_SizeMatrix = c_DimMatrix * (c_DimMatrix + 1) / 2
         };

    /**
     * enumerator used for error matrix handling,
     * shows also how rows/columns are defined
     */
    enum {c_Px, c_Py, c_Pz, c_E, c_X, c_Y, c_Z};

    /**
     * Default constructor
     * All private members are set to 0. Particle type is set to c_Undefined.
     */
    Particle();

    /**
     * Constructor from a Lorentz vector and PDG code.
     * All other private members are set to their defalt values (0).
     * @param Lorentz vector
     * @param PDG code
     */
    Particle(const TLorentzVector&, const int pdgCode);

    /**
     * Constructor for final state particles.
     * All other private members are set to their defalt values (0).
     * @param Lorentz vector
     * @param PDG code
     * @param flavor type
     * @param mdst index
     * @param particle type
     */
    Particle(const TLorentzVector&, const int pdgCode, const unsigned flavorType,
             const unsigned index, const EParticleType type);

    /**
     * Constructor for composite particles.
     * All other private members are set to their defalt values (0).
     * @param Lorentz vector
     * @param PDG code
     * @param decay flavor type
     * @param vector of daughter indices
     */
    Particle(const TLorentzVector&, const int pdgCode, const unsigned flavorType,
             const std::vector<int>&);

    /**
     * Constructor from a reconstructed track (mdst object Track)
     * @param pointer to Track object
     * @param store array index of Track object
     * @param Type of charged particle
     */
    Particle(const Track*, const unsigned index,
             const Const::ChargedStable& chargedStable);

    /**
     * Constructor from a reconstructed gamma candidate (mdst object ECLGamma)
     * @param pointer to ECLGamma object
     * @param store array index of ECLGamma object
     */
    Particle(const ECLGamma*, const unsigned index);

    /**
     * Constructor from a reconstructed pi0 candidate (mdst object ECLPi0)
     * @param pointer to ECLPi0 object
     * @param store array index of ECLPi0 object
     */
    Particle(const ECLPi0*, const unsigned index);

    /**
     * Constructor from MC particle (mdst object MCParticle)
     * @param pointer to MCParticle object
     */
    Particle(const MCParticle*);

    /**
     * Constructor from MC particle (mdst object MCParticle)
     * @param pointer to MCParticle object
     * @param store array index of MCParticle object
     */
    Particle(const MCParticle*, const unsigned index);

    /**
     * Destructor
     */
    ~Particle();

  public:

    // setters

    /**
     * Sets PDG code
     * @param PDG code
     */
    void setPDGCode(int pdgCode) { m_pdgCode = pdgCode; }

    /**
     * Sets Lorentz vector
     * @param Lorentz vector
     */
    void set4Vector(const TLorentzVector& p4) {
      m_px = p4.Px();
      m_py = p4.Py();
      m_pz = p4.Pz();
      m_mass = p4.M();
    }

    /**
     * Sets position (decay vertex)
     * @param point (position or decay vertex)
     */
    void setVertex(const TVector3& vertex) {
      m_x = vertex.X();
      m_y = vertex.Y();
      m_z = vertex.Z();
    };

    /**
     * Sets 7x7 error matrix
     * @param The 7x7 momentum and vertex error matrix (order: px,py,pz,E,x,y,z)
     */
    void setMomentumVertexErrorMatrix(const TMatrixFSym& m);

    /**
     * Sets Lorentz vector, position and 7x7 error matrix
     * @param Lorentz vector
     * @param point (position or vertex)
     * @param 7x7 momentum and vertex error matrix (order: px,py,pz,E,x,y,z)
     */
    void updateMomentum(const TLorentzVector& p4, const TVector3& vertex,
                        const TMatrixFSym& m) {
      set4Vector(p4);
      setVertex(vertex);
      setMomentumVertexErrorMatrix(m);
    }

    // getters

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
      return TLorentzVector(m_px, m_py, m_pz, getEnergy());
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
    float getTotalMomentum() const {
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
    float getMassError() const { return 0.0; };

    /**
     * Returns vertex position (POCA for charged, IP for neutral FS particles)
     * @return vertex position
     */
    TVector3 getVertex() const {
      return TVector3(m_x, m_y, m_z);
    };

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
     * Returns particle type as defined with enum EParticleType
     * @return particle type
     */
    EParticleType getParticleType() const { return m_particleType; }

    /**
     * Returns PDG code
     * @return PDG code
     */
    int getPDGCode(void) const { return m_pdgCode; }

    /**
     * Returns flavor type of the decay (for FS particles: flavor type of particle)
     * @return flavor type (0=unflavored, 1=flavored)
     */
    int getFlavorType() const {return m_flavorType;}

    /**
     * Returns particle nominal mass
     * @return nominal mass
     */
    float getPDGMass(void) const;

    /**
     * Returns particle charge
     * @return particle charge in units of elementary charge
     */
    float getCharge(void) const;

    /**
     * Returns 0-based index of MDST store array object (0 for composite particles)
     * @return index of MDST store array object
     */
    unsigned getMdstArrayIndex(void) const { return m_mdstIndex; }

    /**
     * Returns unique identifier of final state particle (needed in particle combiner)
     * @return unique identifier of final state particle
     */
    int getMdstSource() const {return m_mdstIndex + (m_particleType << 24);}

    // Interface for obtaining/appending daughter Particles

    /**
     * Appends index of daughter to daughters index array
     * @param Pointer to the daughter particle
     */
    void appendDaughter(const Particle* daughter);

    /**
     * Removes index of daughter from daughters index array
     * @param Pointer to the daughter particle
     */
    void removeDaughter(const Particle* daughter);

    /**
     * Returns number of daughter particles
     * @return number of daughter particles
     */
    unsigned getNDaughters(void) const {
      return m_daughterIndices.size();
    }

    /**
     * Returns a pointer to the i-th daughter particle (i is 0-based index)
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
     * Retruns a vector of store array indices of daughter particles
     * @return vector of store array indices of daughter particle
     */
    const std::vector<int>& getDaughterIndices() const { return m_daughterIndices; }

    /**
     * Returns a vector of pointers to Final State daughter particles
     * @return vector of pointers to final state daughter particles
     */
    const std::vector<const Belle2::Particle*> getFinalStateDaughters() const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /**
     * Returns true if final state ancessors overlap
     * @return True if overlap and false otherwise
     */
    bool overlapsWith(const Particle* oParticle) const;

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
    std::vector<int> m_daughterIndices;  /**< daughter particle indices */
    unsigned m_flavorType;  /**< flavor type: 0=unflavored, 1=flavored */
    EParticleType m_particleType;  /**< particle type */
    unsigned m_mdstIndex;  /**< 0-based index of MDST store array object */

    // transient data members
    /**
     * Internal pointer to DataStore Array containing particles belonging to
     * this collection.
     *
     * This is a transient member and will not be written to file. The pointer
     * will be set correctly on first access after deserialisation
     */
    TClonesArray* m_plist; //! transient pointer to particle StoreArray

    // private methods
    /**
     * Resets particle's 7x7 error matrix
     * All elements are set to 0.0
     */
    void resetErrorMatrix();

    /**
     * Stores 7x7 error matrix into private member m_errMatrix
     * @param 7x7 error matrix
     */
    void storeErrorMatrix(const TMatrixFSym& m);

    /**
     * Search the DataStore for the corresponding Particle array.
     *
     * This function is called automatically if the pointer to the
     * particle list is not set when needed, e.g. after deserialization.
     */
    void fixParticleList() const;

    /**
     * Fill final state particle daughters into vector
     * @param vector of daughter particles
     */
    void fillFSPDaughters(std::vector<const Belle2::Particle*> &fspDaughters) const;

    /**
     * sets m_flavorType using m_pdgCode
     */
    void setFlavorType();

    ClassDef(Particle, 2); /**< class definition */
  };

  /** @}*/

} // end namespace Belle2

#endif
