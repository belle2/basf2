/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCPARTICLE_H
#define MCPARTICLE_H

#include <TObject.h>
#include <TClonesArray.h>
#include <TVector3.h>
#include <TLorentzVector.h>

#include <cmath>
#include <vector>

#define DEFAULT_MCPARTICLES "MCParticles"

namespace Belle2 {

  /**
   * A Class to store the Monte Carlo particle information.
   */
  class MCParticle: public TObject {
  public:
    enum StatusBit {
      PrimaryParticle = 1,    /**< bit 0: Particle is primary particle. */
      StableInGenerator = 2,  /**< bit 1: Particle is stable in the generator. */
      StableInSimulation = 4, /**< bit 2: Particle is stable in the simulation. */
      DecayedInBeampipe = 8,  /**< bit 3: Particle decayed in the beampipe. */
      DecayedInVTX = 16,      /**< bit 4: Particle decayed in the vertex detectors (PXD, SVD). */
      DecayedInCDC = 32,      /**< bit 5: Particle decayed in the drift chamber (CDC). */
      DecayedInPID = 64,      /**< bit 6: Particle decayed in the PID detectors. */
      DecayedInECL = 128,     /**< bit 7: Particle decayed in the elm calorimeter. */
      DecayedInKLM = 256      /**< bit 8: Particle decayed in the klm. */
    };

    /**
     * Default constructor for ROOT.
     */
    MCParticle():
        m_plist(0), m_index(0), m_status(0), m_pdg(0),
        m_mass(0), m_momentum_x(0),
        m_momentum_y(0), m_momentum_z(0),
        m_validVertex(false), m_productionTime(0),
        m_productionVertex_x(0), m_productionVertex_y(0),
        m_productionVertex_z(0),
        m_decayTime(0), m_decayVertex_x(0),
        m_decayVertex_y(0), m_decayVertex_z(0),
        m_first_daughter(0), m_last_daughter(0) {}

    /**
     * Construct MCParticle from a another MCParticle and the TClonesArray it is stored in.
     *
     * Used by MCParticleGraph to build a valid tree of MCParticles from particle and decay
     * information
     *
     * @see class MCParticleGraph
     */
    MCParticle(TClonesArray* plist, const MCParticle& p):
        m_plist(plist), m_index(p.m_index), m_status(p.m_status), m_pdg(p.m_pdg),
        m_mass(p.m_mass), m_momentum_x(p.m_momentum_x),
        m_momentum_y(p.m_momentum_y), m_momentum_z(p.m_momentum_z),
        m_validVertex(p.m_validVertex), m_productionTime(p.m_productionTime),
        m_productionVertex_x(p.m_productionVertex_x), m_productionVertex_y(p.m_productionVertex_y),
        m_productionVertex_z(p.m_productionVertex_z),
        m_decayTime(p.m_decayTime), m_decayVertex_x(p.m_decayVertex_x),
        m_decayVertex_y(p.m_decayVertex_y), m_decayVertex_z(p.m_decayVertex_z),
        m_first_daughter(p.m_first_daughter), m_last_daughter(p.m_last_daughter) {}

    /**
     * Return PDG code of particle.
     * @return The PDG code of the MonteCarlo particle.
     */
    const int getPDG()                       const { return m_pdg; }

    /**
     * Return status code of particle.
     * @param bitmask Takes an optional bitmask that is compared to the status of the particle.
     * @return Returns the status code of the particle. If a bitmask is given it returns 1 if the
     *                 bitmask matches the status and 0 if not.
     */
    const unsigned int getStatus(unsigned int bitmask = -1) const { return m_status & bitmask; }

    /**
     * Return if specific status bit is set.
     * @param bitmask The bitmask which is compared to the status of the particle.
     * @return Returns true if the bitmask matches the status code of the particle.
     */
    bool hasStatus(unsigned int bitmask)     const { return m_status & bitmask; }

    /**
     * Return the actual particle mass in [GeV].
     * @return The actual mass of the particle in [GeV].
     */
    const float getMass()                    const { return m_mass; }

    /**
     * Return actual particle energy in [GeV].
     * @return Returns the actual particle energy in [GeV].
     */
    const float getEnergy()                  const { return std::sqrt(m_momentum_x*m_momentum_x + m_momentum_y*m_momentum_y + m_momentum_z*m_momentum_z + m_mass*m_mass); }

    /**
     * Indication whether vertex and time information is useful or just default.
     * @return Returns True if the vertex and time information is useful.
     */
    const bool hasValidVertex()              const { return m_validVertex; }

    /**
     * Return production time in [s].
     * @return The timestamp of the MonteCarlo particle production in [ns].
     */
    const float getProductionTime()          const { return m_productionTime; }

    /**
     * Return the decay time in [s].
     * @return The timestamp of the decay of the MonteCarlo particle in [ns].
     *         If the particle is stable the time is set to infinity.
     */
    const float getDecayTime()               const { return m_decayTime; }

    /**
     * Return the lifetime in [s].
     * A convenient method to get the lifetime of the MonteCarlo particle.
     * @return The lifetime of the MonteCarlo particle in [ns].
     */
    const float getLifetime()                const { return m_decayTime - m_productionTime; }

    /**
     * Return production vertex position, shorthand for getProductionVertex().
     * @return The production vertex of the MonteCarlo particle in [cm].
     */
    const TVector3 getVertex()               const { return getProductionVertex(); }

    /**
     * Return production vertex position.
     * @return The production vertex of the MonteCarlo particle in [cm].
     */
    const TVector3 getProductionVertex()     const { return TVector3(m_productionVertex_x, m_productionVertex_y, m_productionVertex_z); }

    /**
     * Return momentum.
     * @return The momentum of the MonteCarlo particle in [GeV].
     */
    const TVector3 getMomentum()             const { return TVector3(m_momentum_x, m_momentum_y, m_momentum_z); }

    /**
     * Return 4Vector of particle.
     * @return The 4-vector of the MonteCarlo particle.
     */
    const TLorentzVector get4Vector()        const { TLorentzVector vec; vec.SetXYZM(m_momentum_x, m_momentum_y, m_momentum_z, m_mass); return vec; }

    /**
     * Return decay vertex.
     * @return The decay vertex of the MonteCarlo particle in [cm].
     */
    const TVector3 getDecayVertex()          const { return TVector3(m_decayVertex_x, m_decayVertex_y, m_decayVertex_z); }

    /**
     * Get 1-based index of the particle in the corresponding MCParticle list.
     * @return The index of the MonteCarlo particle in the corresponding MCParticle list (starts with 1)
     */
    const int getIndex()                     const { return m_index; }

    /**
     * Get 1-based index of first daughter, 0 if no daughters.
     * @return The index of the first daughter of the MonteCarlo particle. The index is 0 if the
     *         MonteCarlo particle doesn't have any daughters.
     */
    const int getFirstDaughter()             const { return m_first_daughter; }

    /**
     * Get 1-based index of last daughter, 0 if no daughters.
     * @return The index of the last daughter of the MonteCarlo particle. The index is 0 if the
     *         MonteCarlo particle doesn't have any daughters.
     */
    const int getLastDaughter()              const { return m_last_daughter; }

    /**
     * Get vector of all daughter particles, empty vector if none.
     * Throws an exception of type LastChildIndexOutOfRangError if the last daughter is out of the index range.
     * @return A list of all daughter particles. The list is empty if
     *         the MonteCarlo particle doesn't have any daughters.
     */
    const std::vector<Belle2::MCParticle*> getDaughters() const; //Need namespace qualifier because ROOT CINT as troubles otherwise

    /**
     * Get vector of all mother particles, empty vector if none.
     * @return A list of all mother particles. The list is empty if
     *         the MonteCarlo particle doesn't have any mother particles.
     */
    const std::vector<Belle2::MCParticle*> getMothers() const; //Need namespace qualifier because ROOT CINT as troubles otherwise

    /**
     * Set PDG code of the particle.
     * @param pdg The PDG code of the MonteCarlo particle.
     */
    void setPDG(int pdg);

    /**
     * Set Status code for the particle.
     * @param status The status code of the MonteCarlo particle.
     */
    void setStatus(unsigned int status)                 { m_status = status; }

    /**
     * Add bitmask to current status.
     * @param bitmask The status code which should be added to the existing MonteCarlo particle status code.
     */
    void addStatus(unsigned int bitmask)                { m_status |= bitmask; }

    /**
     * Remove bitmask from current status.
     * @param bitmask The status code which should be removed from the existing MonteCarlo particle status code.
     */
    void removeStatus(unsigned int bitmask)             { m_status &= (!bitmask); }

    /**
     * Set particle mass.
     * @param mass The MonteCarlo particle mass.
     */
    void setMass(float mass)                            { m_mass = mass; }

    /**
     * Set energy.
     * @param energy The energy of the MonteCarlo particle.
     */
    void setEnergy(float energy);

    /**
     * Set indication wether vertex and time information is valid or just default.
     * @param valid Set to true if the vertex is valid.
     */
    void setValidVertex(bool valid)                     { m_validVertex = valid; }

    /**
     * Set production time.
     * @param time The timestamp of the production of the MonteCarlo particle.
     */
    void setProductionTime(float time)                  { m_productionTime = time; }

    /**
     * Set decay time.
     * @param time The timestamp of the decay of the MonteCarlo time.
     */
    void setDecayTime(float time)                       { m_decayTime = time; }

    /**
     * Set production vertex position.
     * @param vertex The position of the production vertex given as TVector3.
     */
    void setProductionVertex(const TVector3 &vertex)    { m_productionVertex_x = vertex.X(); m_productionVertex_y = vertex.Y(), m_productionVertex_z = vertex.Z(); }

    /**
     * Set production vertex position.
     * The position of the production vertex is given as three float parameters.
     * @param x The x position of the production vertex.
     * @param y The y position of the production vertex.
     * @param z The z position of the production vertex.
     */
    void setProductionVertex(float x, float y, float z) { m_productionVertex_x = x; m_productionVertex_y = y; m_productionVertex_z = z; }

    /**
     * Set particle momentum.
     * @param momentum The momentum of the MonteCarlo particle given as TVector3.
     */
    void setMomentum(const TVector3 &momentum)          { m_momentum_x = momentum.X(); m_momentum_y = momentum.Y(), m_momentum_z = momentum.Z(); }

    /**
     * Set particle momentum.
     * The momentum of the MonteCarlo particle is given as three float parameters.
     * @param px The x component of the momentum vector.
     * @param py The y component of the momentum vector.
     * @param pz The z component of the momentum vector.
     */
    void setMomentum(float px, float py, float pz)      { m_momentum_x = px, m_momentum_y = py; m_momentum_z = pz; }

    /**
     * Set decay vertex.
     * @param vertex The position of the decay vertex given as TVector3.
     */
    void setDecayVertex(const TVector3 &vertex)         { m_decayVertex_x = vertex.X(); m_decayVertex_y = vertex.Y(), m_decayVertex_z = vertex.Z(); }

    /**
     * Set decay vertex.
     * The position of the decay vertex is given as three float parameters.
     * @param x The x position of the decay vertex.
     * @param y The y position of the decay vertex.
     * @param z The z position of the decay vertex.
     */
    void setDecayVertex(float x, float y, float z)      { m_decayVertex_x = x; m_decayVertex_y = y; m_decayVertex_z = z; }

    /**
     * Search the DataStore for the corresponding MCParticle array.
     *
     * This function should not be needed by normal users and is called automatically
     * if the pointer to the particle list is not set when needed, e.g. after deserialization.
     */
    void fixParticleList() const;


  protected:

    /**
     * Internal pointer to DataStore Array containing particles belonging to this collection.
     *
     * This is a transient member and will not be written to file. The pointer will be set
     * correctly on first access after deserialisation
     */
    TClonesArray* m_plist; //! transient pointer to particle list

    /** 1-based index of the particle, will be set automatically after deserialisation if needed. */
    int m_index; //! transient 1-based index of particle

    unsigned int m_status;      /**< status code */
    int m_pdg;                  /**< PDG-Code of the particle */
    float m_mass;               /**< mass of the particle */
    float m_momentum_x;         /**< momentum of particle, x component */
    float m_momentum_y;         /**< momentum of particle, y component */
    float m_momentum_z;         /**< momentum of particle, z component */

    bool m_validVertex;         /**< indication wether vertex and time information is useful or just default */

    float m_productionTime;     /**< production time */
    float m_productionVertex_x; /**< production vertex of particle, x component */
    float m_productionVertex_y; /**< production vertex of particle, y component */
    float m_productionVertex_z; /**< production vertex of particle, z component */

    float m_decayTime;          /**< decay time */
    float m_decayVertex_x;      /**< decay vertex of particle, x component */
    float m_decayVertex_y;      /**< decay vertex of particle, y component */
    float m_decayVertex_z;      /**< decay vertex of particle, z component */

    int m_first_daughter;       /**< 1-based index of first daughter particle in collection, 0 if no daughters */
    int m_last_daughter;        /**< 1-based index of last daughter particle in collection, 0 if no daughters */

    ClassDef(MCParticle, 1);
  };

  inline void MCParticle::setEnergy(float energy)
  {
    if (energy <= m_mass) {
      m_momentum_x = 0;
      m_momentum_y = 0;
      m_momentum_z = 0;
      return;
    }
    TVector3 p(m_momentum_x, m_momentum_y, m_momentum_z);
    p.SetMag(sqrt(energy*energy - m_mass*m_mass));
    m_momentum_x = p.X();
    m_momentum_y = p.Y();
    m_momentum_z = p.Z();
  }

} // end namespace Belle2

#endif
