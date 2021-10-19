/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Const.h>
#include <framework/core/FrameworkExceptions.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/geometry/B2Vector3.h>

#include <TClonesArray.h>
#include <TVector3.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

#include <cmath>
#include <climits>
#include <vector>


namespace Belle2 {
  /**
   * A Class to store the Monte Carlo particle information.
   */
  class MCParticle: public RelationsObject {
  public:

    //Define exceptions
    /** Exception is thrown if the requested index for the last child is out of range. */
    BELLE2_DEFINE_EXCEPTION(LastChildIndexOutOfRangError, "Last child index out of range!")
    /** Exception is thrown if no pointer to the particle list was set. */
    BELLE2_DEFINE_EXCEPTION(NoParticleListSetError, "No Particle list set, cannot determine related particles!")
    /** Exception is thrown if the pdg value of the MCParticle is not known to the internal database (TDatabasePDG). */
    BELLE2_DEFINE_EXCEPTION(ParticlePDGNotKnownError, "The pdg value (%1%) of the MCParticle is not known!")


    /** The status information for the MCParticle. */
    enum StatusBit {
      /** bit 0:  Particle is primary particle. For example, All the particles from the generator.  */
      c_PrimaryParticle   = 1 << 0,
      /** bit 1:  Particle is stable, i.e., not decaying in the generator.  */
      c_StableInGenerator = 1 << 1,
      /** bit 2:  Particle left the detector (the simulation volume). */
      c_LeftDetector      = 1 << 2,
      /** bit 3:  Particle was stopped in the detector (the simulation volume). */
      c_StoppedInDetector = 1 << 3,
      /** bit 4:  Particle is virtual and not going to Geant4. Exchange boson, off-shell, unknown to Geant4, etc. */
      c_IsVirtual         = 1 << 4,
      /** bit 5:  Particle is initial such as e+ or e- and not going to Geant4 */
      c_Initial           = 1 << 5,
      /** bit 6: Particle is from initial state radiation */
      c_IsISRPhoton       = 1 << 6,
      /** bit 7: Particle is from finial state radiation */
      c_IsFSRPhoton       = 1 << 7,
      /** bit 8: Particle is an radiative photon from PHOTOS */
      c_IsPHOTOSPhoton    = 1 << 8,
      /** combined flag to test whether the particle is radiative */
      c_IsRadiativePhoton = c_IsISRPhoton | c_IsFSRPhoton | c_IsPHOTOSPhoton,
    };


    /**
     * Default constructor for ROOT.
     */
    MCParticle():
      m_plist(0), m_index(0), m_status(0),
      m_pdg(0), m_mass(0), m_energy(0),
      m_momentum_x(0), m_momentum_y(0), m_momentum_z(0),
      m_validVertex(false), m_productionTime(0),
      m_productionVertex_x(0), m_productionVertex_y(0),
      m_productionVertex_z(0),
      m_decayTime(0), m_decayVertex_x(0),
      m_decayVertex_y(0), m_decayVertex_z(0),
      m_mother(0),
      m_firstDaughter(0), m_lastDaughter(0),
      m_secondaryPhysicsProcess(0),
      m_seenIn() {}

    /**
     * Construct MCParticle from a another MCParticle and the TClonesArray it is stored in.
     *
     * Used by MCParticleGraph to build a valid tree of MCParticles from particle and decay
     * information
     *
     * @see class MCParticleGraph
     */
    MCParticle(TClonesArray* plist, const MCParticle& p):
      m_plist(plist), m_index(p.m_index), m_status(p.m_status),
      m_pdg(p.m_pdg), m_mass(p.m_mass), m_energy(p.m_energy),
      m_momentum_x(p.m_momentum_x), m_momentum_y(p.m_momentum_y), m_momentum_z(p.m_momentum_z),
      m_validVertex(p.m_validVertex), m_productionTime(p.m_productionTime),
      m_productionVertex_x(p.m_productionVertex_x), m_productionVertex_y(p.m_productionVertex_y),
      m_productionVertex_z(p.m_productionVertex_z),
      m_decayTime(p.m_decayTime), m_decayVertex_x(p.m_decayVertex_x),
      m_decayVertex_y(p.m_decayVertex_y), m_decayVertex_z(p.m_decayVertex_z),
      m_mother(p.m_mother),
      m_firstDaughter(p.m_firstDaughter), m_lastDaughter(p.m_lastDaughter),
      m_secondaryPhysicsProcess(p.m_secondaryPhysicsProcess),
      m_seenIn(p.m_seenIn) {}

    /**
     * Return PDG code of particle.
     * @return The PDG code of the MonteCarlo particle.
     */
    int getPDG() const { return m_pdg; }

    /**
     * Return status code of particle.
     * @param bitmask Takes an optional bitmask that is compared to the status
     *        of the particle.
     * @return Returns the status code of the particle. If a bitmask is given
     *         and it matches the the status it returns the value of the
     *         bitmask and 0 if not.
     */
    unsigned int getStatus(unsigned short int bitmask = USHRT_MAX) const { return m_status & bitmask; }

    /**
     * Return if specific status bit is set.
     * @param bitmask The bitmask which is compared to the status of the particle.
     * @return Returns true if the bitmask matches the status code of the particle.
     */
    bool hasStatus(unsigned short int bitmask) const { return (m_status & bitmask) == bitmask; }

    /**
     * Return the particle mass in GeV.
     * @return The mass of the particle in GeV.
     */
    float getMass() const { return m_mass; }

    /**
     * Return the particle charge defined in TDatabasePDG.
     * @return The charge of the particle in units of q(positron).
     */
    float getCharge() const;

    /**
     * Return particle energy in GeV.
     * @return Returns the particle energy in GeV.
     */
    float getEnergy() const { return m_energy; }

    /**
     * Indication whether vertex and time information is useful or just default.
     * @return Returns True if the vertex and time information is useful.
     */
    bool hasValidVertex() const { return m_validVertex; }

    /**
     * Return production time in ns.
     * @return The timestamp of the MonteCarlo particle production in ns.
     */
    float getProductionTime() const { return m_productionTime; }

    /**
     * Return the decay time in ns.
     * @return The timestamp of the decay of the MonteCarlo particle in ns.
     *         If the particle is stable the time is set to infinity.
     *         If the particle crosses the simulation volume boundary,
     *         it is set to the crossing time.
     */
    float getDecayTime() const { return m_decayTime; }

    /**
     * Return the lifetime in ns.
     * A convenient method to get the lifetime of the MonteCarlo particle.
     * @return The lifetime of the MonteCarlo particle in ns.
     *         If the particle crosses the simulation volume boundary,
     *         it is set to the time spent inside the volume.
     */
    float getLifetime() const { return m_decayTime - m_productionTime; }

    /**
     * Return production vertex position, shorthand for getProductionVertex().
     * @return The production vertex of the MonteCarlo particle in cm.
     */
    ROOT::Math::XYZVector getVertex() const { return getProductionVertex(); }

    /**
     * Return production vertex position.
     * @return The production vertex of the MonteCarlo particle in cm.
     */
    ROOT::Math::XYZVector getProductionVertex() const
    {
      return ROOT::Math::XYZVector(m_productionVertex_x, m_productionVertex_y, m_productionVertex_z);
    }

    /**
     * Return momentum.
     * @return The momentum of the MonteCarlo particle in GeV.
     */
    TVector3 getMomentum() const
    {
      return TVector3(m_momentum_x, m_momentum_y, m_momentum_z);
    }

    /**
     * Return 4Vector of particle.
     * @return The 4-vector of the MonteCarlo particle.
     */
    ROOT::Math::PxPyPzEVector get4Vector() const
    {
      return ROOT::Math::PxPyPzEVector(m_momentum_x, m_momentum_y, m_momentum_z, m_energy);
    }


    /**
     * Return decay vertex.
     * @return The decay vertex of the MonteCarlo particle in cm.
     *         If the particle crosses the simulation volume boundary,
     *         it is set to the crossing position.
     */
    TVector3 getDecayVertex() const
    {
      return TVector3(m_decayVertex_x, m_decayVertex_y, m_decayVertex_z);
    }

    /**
     * Get 1-based index of the particle in the corresponding MCParticle list.
     * This is used by the MCParticle Graph.
     * @return The index of the MonteCarlo particle in the corresponding
     *         MCParticle list (starts with 1)
     */
    int getIndex() const { fixParticleList(); return m_index; }

    /**
     * Get 0-based index of the particle in the corresponding MCParticle list.
     * This is the function for users who want to use the indices of the
     * TClonesArray. To get the corresponding mother and daughter indices use
     * ->getMother()->getArrayIndex().
     *
     * @return The index of the MonteCarlo particle in the corresponding
     *         MCParticle array
     *         Careful: These indices do not follow the standard from (Fortran)
     *         generators, where the first particle has index 1. In the array
     *         the first particle has index 0.
     */
    int getArrayIndex() const { fixParticleList(); return m_index - 1;  }

    /**
     * Get 1-based index of first daughter, 0 if no daughters.
     * @return The index of the first daughter of the MonteCarlo particle. The
     *         index is 0 if the MonteCarlo particle doesn't have any daughters.
     */
    int getFirstDaughter() const { return m_firstDaughter; }


    /**
     * Get 1-based index of last daughter, 0 if no daughters.
     * @return The index of the last daughter of the MonteCarlo particle. The index is 0 if the
     *         MonteCarlo particle doesn't have any daughters.
     */
    int getLastDaughter() const { return m_lastDaughter; }

    /**
     * Get vector of all daughter particles, empty vector if none.
     * Throws an exception of type LastChildIndexOutOfRangError if the last
     * daughter is out of the index range.
     *
     * @return A list of all daughter particles. The list is empty if
     *         the MonteCarlo particle doesn't have any daughters.
     */
    std::vector<Belle2::MCParticle*> getDaughters() const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /** Return i-th daughter */
    const MCParticle* getDaughter(int i) const;

    /** Return number of daughter MCParticles. */
    int getNDaughters() const;

    /**
     * Returns a pointer to the mother particle. NULL if the particle doesn't
     * have a mother.
     * @return A pointer to the mother particle. NULL if no mother was defined
     *         for the particle.
     */
    MCParticle* getMother() const;

    /**
     * Returns the physics process type of a secondary particle.
     * @return Returns an integer indicating the physics process type of a secondary particle.
     *                 0 if the particle is primary.
     *                -1 if no information is found.
     *         For the details, see the Geant4 package:
     *         (*G4Track)->GetCreatorProcess()->GetProcessSubType()
     *         processes/electromagnetic/utils/include/G4EmProcessSubType.hh
     *         processes/hadronic/management/include/G4HadronicProcessType.hh
     *         processes/decay/include/G4DecayProcessType.hh
     */
    int getSecondaryPhysicsProcess() const {return m_secondaryPhysicsProcess;}

    /**
     * Return the seen-in flags of the entire Belle II subdetectors for an MC particle.
     * @return Returns the entire seen-in flags
     */
    Const::DetectorSet getSeenInDetector() const { return m_seenIn; }

    /**
     * Return if the seen-in flag for a specific subdetector is set or not.
     * @param set The ID for Belle II subdetector(s), in either Const::DetectorSet or Const::EDetector format.
     * @return Returns true if the corresponding bit is set.
     */
    bool hasSeenInDetector(Const::DetectorSet set) const { return m_seenIn.contains(set); }

    /**
     * Check if particle is virtual
     *
     */
    bool isVirtual() const;

    /**
     * Check if particle is an initial particle such as ISR
     *
     */
    bool isInitial() const;

    /**
     * Check if particle is a primary particle which was created
     * by the generator (and not, for example material interaction)
     *
     */
    bool isPrimaryParticle() const;

    /**
     * Set PDG code of the particle.
     * @param pdg The PDG code of the MonteCarlo particle.
     */
    void setPDG(int pdg) { m_pdg = pdg; }

    /**
     * Sets the mass for the particle from the particle's PDG code.
     */
    void setMassFromPDG();

    /**
     * Set Status code for the particle.
     * @param status The status code of the MonteCarlo particle.
     */
    void setStatus(unsigned short int status) { m_status = status; }

    /**
     * Add bitmask to current status.
     * @param bitmask The status code which should be added to the existing
     *        MonteCarlo particle status code.
     */
    void addStatus(unsigned short int bitmask) { m_status |= bitmask; }

    /**
     * Remove bitmask from current status.
     * @param bitmask The status code which should be removed from the existing
     *        MonteCarlo particle status code.
     */
    void removeStatus(unsigned short int bitmask) { m_status &= (~bitmask); }

    /**
     * Set particle mass.
     * @param mass The MonteCarlo particle mass.
     */
    void setMass(float mass) { m_mass = mass; }

    /**
     * Set energy.
     * @param energy The energy of the MonteCarlo particle.
     */
    void setEnergy(float energy) { m_energy = energy; }

    /**
     * Set indication wether vertex and time information is valid or just default.
     * @param valid Set to true if the vertex is valid.
     */
    void setValidVertex(bool valid) { m_validVertex = valid; }

    /**
     * Set production time.
     * @param time The timestamp of the production of the MonteCarlo particle.
     */
    void setProductionTime(float time) { m_productionTime = time; }

    /**
     * Set decay time.
     * @param time The timestamp of the decay of the MonteCarlo time.
     */
    void setDecayTime(float time) { m_decayTime = time; }

    /**
     * Set production vertex position.
     * @param vertex The position of the production vertex given as TVector3.
     */
    void setProductionVertex(const TVector3& vertex)
    {
      m_productionVertex_x = vertex.X(); m_productionVertex_y = vertex.Y(), m_productionVertex_z = vertex.Z();
    }

    /**
     * Set production vertex position.
     * The position of the production vertex is given as three float parameters.
     * @param x The x position of the production vertex.
     * @param y The y position of the production vertex.
     * @param z The z position of the production vertex.
     */
    void setProductionVertex(float x, float y, float z)
    {
      m_productionVertex_x = x; m_productionVertex_y = y; m_productionVertex_z = z;
    }

    /**
     * Set particle momentum.
     * @param momentum The momentum of the MonteCarlo particle given as TVector3.
     */
    void setMomentum(const TVector3& momentum)
    {
      m_momentum_x = momentum.X(); m_momentum_y = momentum.Y(), m_momentum_z = momentum.Z();
    }

    /**
     * Set particle momentum.
     * The momentum of the MonteCarlo particle is given as three float parameters.
     * @param px The x component of the momentum vector.
     * @param py The y component of the momentum vector.
     * @param pz The z component of the momentum vector.
     */
    void setMomentum(float px, float py, float pz)
    {
      m_momentum_x = px, m_momentum_y = py; m_momentum_z = pz;
    }

    /**
     * Sets the 4Vector of particle.
     * @param p4 4Vector
     */
    void set4Vector(const ROOT::Math::PxPyPzEVector& p4) { setMomentum(B2Vector3D(p4.Vect())); m_energy = p4.energy(); }

    /**
     * Set decay vertex.
     * @param vertex The position of the decay vertex given as TVector3.
     */
    void setDecayVertex(const TVector3& vertex)
    {
      m_decayVertex_x = vertex.X(); m_decayVertex_y = vertex.Y(), m_decayVertex_z = vertex.Z();
    }

    /**
     * Set decay vertex.
     * The position of the decay vertex is given as three float parameters.
     * @param x The x position of the decay vertex.
     * @param y The y position of the decay vertex.
     * @param z The z position of the decay vertex.
     */
    void setDecayVertex(float x, float y, float z)
    {
      m_decayVertex_x = x; m_decayVertex_y = y; m_decayVertex_z = z;
    }

    /**
     * Sets the physics process type of a secondary particle.
     * @param physicsProcess physics process type as an integer number for a secondary particle.
     */
    void setSecondaryPhysicsProcess(int physicsProcess) { m_secondaryPhysicsProcess = physicsProcess; }

    /**
     * Set the seen-in flags for the entire Belle II subdetectors for an Monte Carlo particle.
     * @param set The ID for Belle II subdetector(s), in either Const::DetectorSet or Const::EDetector format
     */
    void setSeenInDetector(Const::DetectorSet set) { m_seenIn = set; }

    /**
     * Flag/Add a bit if the MC particle is seen in a specific subdetector.
     * @param set The ID for Belle II subdetector(s), in either Const::DetectorSet or Const::EDetector format
     */
    void addSeenInDetector(Const::DetectorSet set) { m_seenIn += set; }

    /**
     * Unflag/Remove the bit if the MC particle is not seen in a specific subdetector.
     * @param set The ID for Belle II subdetector(s), in either Const::DetectorSet or Const::EDetector format
     */
    void removeSeenInDetector(Const::DetectorSet set) { m_seenIn -= set; }

    /**
     * Search the DataStore for the corresponding MCParticle array.
     *
     * This function should not be needed by normal users and is called
     * automatically if the pointer to the particle list is not set when
     * needed, e.g. after deserialization.
     */
    void fixParticleList() const;

    /**
     * Set particle to virtual. (A bit more convinient)
     */
    void setVirtual() {  addStatus(c_IsVirtual); }

    /**
     * Set particle to initial. (A bit more convinient)
     */
    void setInitial() {  addStatus(c_Initial); }

    /** Return name of this particle. */
    virtual std::string getName() const override;

    /** Return a short summary of this object's contents in HTML format. */
    virtual std::string getInfoHTML() const override;

    /**
    * Explores the decay tree of the MC particle and returns the (grand^n)daughter identified by a generalized index.
    * The generalized index consists of a colon-separated list of daughter indexes, starting from the root particle:
    * 0:1:3 identifies the fourth daughter (3) of the second daughter (1) of the first daughter (0) of the mother particle.
    * This method mirrors the method used in the Particle class.
    * @param generalizedIndex the generalized index of the particle to be returned
    * @return a particle in the decay tree of the root particle.
    */
    const MCParticle* getParticleFromGeneralizedIndexString(const std::string& generalizedIndex) const;

  protected:

    /**
     * Internal pointer to DataStore Array containing particles belonging to
     * this collection.
     *
     * This is a transient member and will not be written to file. The pointer
     * will be set correctly on first access after deserialisation
     */
    TClonesArray* m_plist; //! transient pointer to particle list

    /**
     * 1-based index of the particle, will be set automatically after
     * deserialisation if needed.
     */
    int m_index; //! transient 1-based index of particle

    unsigned short int m_status;      /**< status code */
    int m_pdg;                  /**< PDG-Code of the particle */
    float m_mass;               /**< mass of the particle */
    float m_energy;             /**< energy of the particle */
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

    int m_mother;               /**< 1-based index of the mother particle */
    int m_firstDaughter;        /**< 1-based index of first daughter particle in collection, 0 if no daughters */
    int m_lastDaughter;         /**< 1-based index of last daughter particle in collection, 0 if no daughters */
    static const double c_epsilon;  /**< limit of precision for two doubles to be the same. */

    int m_secondaryPhysicsProcess;  /**< physics process type of a secondary particle */

    Const::DetectorSet m_seenIn;  /**< Each bit is a seen-in flag for the corresoponding subdetector of Belle II */

    ClassDefOverride(MCParticle, 5); /**< A Class to store the Monte Carlo particle information. */
  };


  inline bool MCParticle::isVirtual() const
  {
    bool virtuality = hasStatus(c_IsVirtual);
    if (!virtuality) {
      double E2 = m_energy * m_energy;
      double m2 = m_mass * m_mass;
      double p2 = m_momentum_x * m_momentum_x;
      p2 += m_momentum_y * m_momentum_y;
      p2 += m_momentum_z * m_momentum_z;

      virtuality = (fabs(E2 - (p2 + m2)) > c_epsilon * E2);
    }
    return virtuality;
  }

  inline bool MCParticle::isInitial() const
  {
    return hasStatus(c_Initial);
  }

  inline bool MCParticle::isPrimaryParticle() const
  {
    return hasStatus(c_PrimaryParticle);
  }

  inline MCParticle* MCParticle::getMother() const
  {
    if (m_mother == 0)
      return nullptr;
    fixParticleList();
    return static_cast<MCParticle*>(m_plist->At(m_mother - 1));
  }

} // end namespace Belle2
