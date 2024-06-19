/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMatrixFfwd.h>
#include <TMatrixFSymfwd.h>

#include <vector>

class TClonesArray;

namespace Belle2 {

  // forward declarations
  class KLMCluster;
  class Track;
  class TrackFitResult;
  class MCParticle;
  class PIDLikelihood;
  class V0;

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
   *  - source of the object from which the particle is created (see EParticleSourceObject)
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
     * particle source enumerators
     */
    enum EParticleSourceObject {
      c_Undefined     = 0,
      c_Track         = 1,
      c_ECLCluster    = 2,
      c_KLMCluster    = 3,
      c_V0            = 4,
      c_MCParticle    = 5,
      c_Composite     = 6,
      c_NoMDSTSource  = 7
    };

    /** describes flavor type, see getFlavorType(). */
    enum EFlavorType {
      c_Unflavored = 0, /**< Is its own antiparticle or we don't know whether it is a particle/antiparticle. */
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
     * Flags that describe the particle property,
     * which are used in the MC matching.
     */
    enum PropertyFlags {
      c_Ordinary = 0, /** Ordinary particles */
      c_IsUnspecified = 1, /**< Is the particle unspecified by marking @ ? */
      c_IsIgnoreRadiatedPhotons = 2, /**< Is the particle MC matched with the ignore radiated photon flag set?*/
      c_IsIgnoreIntermediate = 4, /**< Is the particle MC matched with the ignore intermediate resonances flag set?*/
      c_IsIgnoreMassive = 8, /**< Is the particle MC matched with the ignore missing massive particle flag set?*/
      c_IsIgnoreNeutrino = 16, /**< Is the particle MC matched with the ignore missing neutrino flag set?*/
      c_IsIgnoreGamma = 32, /**< Is the particle MC matched with the ignore missing gamma flag set?*/
      c_IsIgnoreBrems = 64, /**< Is the particle MC matched with the ignore added Brems gamma flag set?*/
      c_IsIgnoreMisID = 128, /**< Is the particle MC matched with the ignore MisID flag set? */
      c_IsIgnoreDecayInFlight = 256, /**< Is the particle MC matched with the ignore DecayInFlight flag set?*/
    };

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
    Particle(const ROOT::Math::PxPyPzEVector& momentum, const int pdgCode);

    /**
     * Constructor for final state particles.
     * All other private members are set to their default values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     * @param flavorType flavor type
     * @param particleType particle source
     * @param mdstIndex mdst index
     */
    Particle(const ROOT::Math::PxPyPzEVector& momentum,
             const int pdgCode,
             EFlavorType flavorType,
             const EParticleSourceObject particleType,
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
    Particle(const ROOT::Math::PxPyPzEVector& momentum,
             const int pdgCode,
             EFlavorType flavorType,
             const std::vector<int>& daughterIndices,
             TClonesArray* arrayPointer = nullptr);

    /**
     * Constructor for composite particles.
     * All other private members are set to their default values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     * @param flavorType decay flavor type
     * @param daughterIndices indices of daughters in StoreArray<Particle>
     * @param properties particle property
     * @param arrayPointer pointer to store array which stores the daughters, if the particle itself is stored in the same array the pointer can be automatically determined
     */
    Particle(const ROOT::Math::PxPyPzEVector& momentum,
             const int pdgCode,
             EFlavorType flavorType,
             const std::vector<int>& daughterIndices,
             int properties,
             TClonesArray* arrayPointer = nullptr);

    /**
     * Constructor for composite particles.
     * All other private members are set to their default values (0).
     * @param momentum Lorentz vector
     * @param pdgCode PDG code
     * @param flavorType decay flavor type
     * @param daughterIndices indices of daughters in StoreArray<Particle>
     * @param properties particle property
     * @param daughterProperties daughter particle properties
     * @param arrayPointer pointer to store array which stores the daughters, if the particle itself is stored in the same array the pointer can be automatically determined
     */
    Particle(const ROOT::Math::PxPyPzEVector& momentum,
             const int pdgCode,
             EFlavorType flavorType,
             const std::vector<int>& daughterIndices,
             int properties,
             const std::vector<int>& daughterProperties,
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
     * To be used to create Particle objects from tracks with full control over
     * the hypothesis (e.g. V0 daughters).
     * @param trackArrayIndex track StoreArray index
     * @param trackFit pointer to TrackFitResult object
     * @param chargedStable Type of charged particle
     */
    Particle(int trackArrayIndex, const TrackFitResult* trackFit,
             const Const::ChargedStable& chargedStable);

    /**
     * Constructor of a photon from a reconstructed ECL cluster that is not matched to any charged track.
     * @param eclCluster pointer to ECLCluster object
     * @param type the kind of ParticleType we want (photon by default)
     */
    explicit Particle(const ECLCluster* eclCluster,
                      const Const::ParticleType& type = Const::photon);

    /**
     * Constructor from a reconstructed KLM cluster.
     * @param klmCluster pointer to KLMCluster object
     * @param pdgCode PDG code (Klong by default)
     */
    explicit Particle(const KLMCluster* klmCluster, const int pdgCode = Const::Klong.getPDGCode());

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
     * Sets PDG code
     * @param pdg PDG code
     */
    void setPDGCode(const int pdg)
    {
      m_pdgCode = pdg;
    }

    /**
     * Sets Lorentz vector
     * @param p4 Lorentz vector
     */
    void set4Vector(const ROOT::Math::PxPyPzEVector& p4)
    {
      m_px = p4.Px();
      m_py = p4.Py();
      m_pz = p4.Pz();
      m_mass = p4.M();
    }

    /**
     * Sets Lorentz vector dividing by the momentum scaling factor
     * @param p4 Lorentz vector
     */
    void set4VectorDividingByMomentumScaling(const ROOT::Math::PxPyPzEVector& p4)
    {
      m_px = p4.Px() / m_momentumScale;
      m_py = p4.Py() / m_momentumScale;
      m_pz = p4.Pz() / m_momentumScale;
      m_mass = p4.M();
    }

    /**
     * Sets position (decay vertex)
     * @param vertex position
     */
    void setVertex(const ROOT::Math::XYZVector& vertex)
    {
      m_x = vertex.X();
      m_y = vertex.Y();
      m_z = vertex.Z();
    };

    /**
     * Sets Energy loss correction
     * @param energyLossCorrection Correction factor
     */
    void setEnergyLossCorrection(double energyLossCorrection)
    {
      m_energyLossCorrection = energyLossCorrection;
    }

    /**
      * Returns effect of energy correction on the particle momentum
      * @return momentum change
      */
    double getMomentumLossCorrectionFactor() const
    {
      if (m_energyLossCorrection == 0.0) {
        return 1.0;
      }
      double origP = m_momentumScale * sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
      if (origP == 0.0) {
        return 1.0;
      }
      double origE = sqrt(origP * origP + m_mass * m_mass);

      double newP = sqrt((origE - m_energyLossCorrection) * (origE - m_energyLossCorrection) - m_mass * m_mass);
      return newP / origP;
    }

    /**
     * Sets momentum scaling
     * @param momentumScalingFactor scaling factor
     */
    void setMomentumScalingFactor(double momentumScalingFactor)
    {
      m_momentumScalingFactor = momentumScalingFactor;
      m_momentumScale = m_momentumScalingFactor * m_momentumSmearingFactor;
    }

    /**
     * Sets momentum smearing
     * @param momentumSmearingFactor scaling factor
     */
    void setMomentumSmearingFactor(double momentumSmearingFactor)
    {
      m_momentumSmearingFactor = momentumSmearingFactor;
      m_momentumScale = m_momentumScalingFactor * m_momentumSmearingFactor;
    }

    /**
     * Sets 4x6 jacobi matrix
     * @param jacobiMatrix 4x6 momentum and vertex error matrix (order: px,py,pz,E,x,y,z)
     */
    void setJacobiMatrix(const TMatrixF& jacobiMatrix);

    /**
     * Sets 7x7 error matrix
     * @param errMatrix 7x7 momentum and vertex error matrix (order: px,py,pz,E,x,y,z)
     */
    void setMomentumVertexErrorMatrix(const TMatrixFSym& errMatrix);

    /**
     * Sets chi^2 probability of fit
     * @param pValue p-value of fit
     */
    void setPValue(double pValue)
    {
      m_pValue = pValue;
    }

    /**
     * sets m_properties
     */
    void setProperty(const int properties)
    {
      m_properties = properties;
    }

    /**
     * Sets Lorentz vector, position, 7x7 error matrix and p-value
     * @param p4 Lorentz vector
     * @param vertex point (position or vertex)
     * @param errMatrix 7x7 momentum and vertex error matrix (order: px,py,pz,E,x,y,z)
     * @param pValue chi^2 probability of the fit
     */
    void updateMomentum(const ROOT::Math::PxPyPzEVector& p4,
                        const ROOT::Math::XYZVector& vertex,
                        const TMatrixFSym& errMatrix,
                        double pValue)
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
     * @param updateType bool to set whether particle type should be updated
     * @param daughterProperty property of the daughter particle
     */
    void appendDaughter(const Particle* daughter, const bool updateType = true, const int daughterProperty = c_Ordinary);

    /**
     * Appends index of daughter to daughters index array
     * @param particleIndex index of daughter in StoreArray<Particle>
     * @param updateType bool to set whether particle type should be updated
     */
    void appendDaughter(int particleIndex, const bool updateType = true)
    {
      if (updateType) {
        // is it a composite particle or fsr corrected?
        m_particleSource = c_Composite;
      }
      m_daughterIndices.push_back(particleIndex);
      m_daughterProperties.push_back(c_Ordinary);
    }

    /**
     * Removes index of daughter from daughters index array
     * @param daughter pointer to the daughter particle
     * @param updateType bool whether particle type should be updated if last daughter was removed
     */
    void removeDaughter(const Particle* daughter, const bool updateType = true);

    /**
     * Replace index of given daughter with new daughter, return true if a replacement is made
     * @param oldDaughter pointer to the daughter that will be removed
     * @param newDaughter pointer to the particle that will be added as a daughter
     */
    bool replaceDaughter(const Particle* oldDaughter, Particle* newDaughter);

    /**
     * Apply replaceDaughter to all Particles in the decay tree by looping recursively through
     * it, return true if a replacement is made
     * @param oldDaughter pointer to the daughter that will be removed
     * @param newDaughter pointer to the particle that will be added as a daughter
     */
    bool replaceDaughterRecursively(const Particle* oldDaughter, Particle* newDaughter);

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
    double getCharge(void) const;

    /**
     * Returns flavor type of the decay (for FS particles: flavor type of particle)
     * @return flavor type (0=unflavored, 1=flavored)
     */
    EFlavorType getFlavorType() const
    {
      return m_flavorType;
    }

    /**
     * Returns particle source as defined with enum EParticleSourceObject
     * @return particle type
     */
    EParticleSourceObject getParticleSource() const
    {
      return m_particleSource;
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
     * Returns particle property as a bit pattern
     * The values are defined in the PropertyFlags enum and described in detail there.
     *
     * @return Combination of Properties describing the particle property
     */
    int getProperty() const
    {
      return m_properties;
    }

    /**
     * Returns invariant mass (= nominal for FS particles)
     * @return invariant mass
     */
    double getMass() const
    {
      return m_mass;
    }

    /**
     * Returns uncertainty on the invariant mass
     * (requires valid momentum error matrix)
     * @return mass error
     */
    //double getMassError() const;

    /**
     * Returns particle nominal mass
     * @return nominal mass
     */
    double getPDGMass(void) const;

    /**
     * Returns particle nominal lifetime
     * @return nominal lifetime [sec]
     */
    double getPDGLifetime() const;

    /**
     * Returns total energy
     * @return total energy
     */
    double getEnergy() const
    {
      return sqrt(m_momentumScale * m_momentumScale * m_px * m_px +
                  m_momentumScale * m_momentumScale * m_py * m_py +
                  m_momentumScale * m_momentumScale * m_pz * m_pz +
                  m_mass * m_mass) - m_energyLossCorrection;
    }

    /**
     * Returns Lorentz vector
     * @return Lorentz vector
     */
    ROOT::Math::PxPyPzEVector get4Vector() const
    {
      double correction = getMomentumLossCorrectionFactor();
      return ROOT::Math::PxPyPzEVector(m_momentumScale * correction * m_px,
                                       m_momentumScale * correction * m_py,
                                       m_momentumScale * correction * m_pz,
                                       getEnergy());
    }

    /**
     * Returns momentum vector
     * @return momentum vector
     */
    ROOT::Math::XYZVector getMomentum() const
    {
      return m_momentumScale * getMomentumLossCorrectionFactor() * ROOT::Math::XYZVector(m_px, m_py, m_pz);
    };

    /**
     * Returns momentum magnitude
     * @return momentum magnitude
     */
    double getMomentumMagnitude() const
    {
      return getP();
    };

    /**
     * Returns momentum magnitude (same as getMomentumMagnitude but with shorter name)
     * @return momentum magnitude
     */
    double getP() const
    {
      return m_momentumScale * getMomentumLossCorrectionFactor() * sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
    };

    /**
     * Returns x component of momentum
     * @return x component of momentum
     */
    double getPx() const
    {
      return m_momentumScale * getMomentumLossCorrectionFactor() * m_px;
    }

    /**
     * Returns y component of momentum
     * @return y component of momentum
     */
    double getPy() const
    {
      return m_momentumScale * getMomentumLossCorrectionFactor() * m_py;
    }

    /**
     * Returns z component of momentum
     * @return z component of momentum
     */
    double getPz() const
    {
      return m_momentumScale * getMomentumLossCorrectionFactor() * m_pz;
    }

    /**
     * Returns effective momentum scale which is the product of the momentum scaling and smearing factors
     * @return momentum scaling factor
     */
    double getEffectiveMomentumScale() const
    {
      return m_momentumScale;
    }

    /**
     * Returns Energy Loss Correction
     * @return Energy Loss Correction
     */
    double getEnergyLossCorrection() const
    {
      return m_energyLossCorrection;
    }
    /**
     * Returns vertex position (POCA for charged, IP for neutral FS particles)
     * @return vertex position
     */
    ROOT::Math::XYZVector getVertex() const
    {
      return ROOT::Math::XYZVector(m_x, m_y, m_z);
    };

    /**
     * Returns x component of vertex position
     * @return x component of vertex position
     */
    double getX() const
    {
      return m_x;
    }

    /**
     * Returns y component of vertex position
     * @return y component of vertex position
     */
    double getY() const
    {
      return m_y;
    }

    /**
     * Returns z component of vertex position
     * @return z component of vertex position
     */
    double getZ() const
    {
      return m_z;
    }

    /**
     * Returns chi^2 probability of fit if done or -1
     * @return p-value of fit (nan means no fit done)
     */
    double getPValue() const
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
     * Returns cosine of the helicity angle
     * The helicity angle is defined in the rest frame of the particle as the angle between the negative momentum of the mother and
     * - the momentum of the first daughter for two body decays
     * - the momentum of the photon for pi0 Dalitz decays
     * - the direction perpendicular to the daughter momenta for three body decays
     * @param mother mother particle, if not given the center of mass system is taken as mother frame
     * @return cosine of the helicity angle
     */
    double getCosHelicity(const Particle* mother = nullptr) const;

    /**
     * Returns cosine of the helicity angle of the given daughter defined by given grand daughter
     * @param iDaughter 0-based index of daughter particle
     * @param iGrandDaughter 0-based index of grand daughter particle
     * @return cosine of the helicity angle
     */
    double getCosHelicityDaughter(unsigned iDaughter, unsigned iGrandDaughter = 0) const;

    /**
     * Returns acoplanarity angle defined as the angle between the decay planes of the grand daughters in the particle's rest frame
     * This assumes that the particle and its daughters have two daughters each
     * @return acoplanarity angle
     */
    double getAcoplanarity() const;


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
     * Returns a vector of store array indices of daughter particles
     * @return vector of store array indices of daughter particle
     */
    const std::vector<int>& getDaughterIndices() const
    {
      return m_daughterIndices;
    }

    /**
     * Returns a vector of properties of daughter particles
     * @return vector of daughter properties
     */
    const std::vector<int>& getDaughterProperties() const
    {
      return m_daughterProperties;
    }

    /**
     * Returns a pointer to the i-th daughter particle
     * @param i 0-based index of daughter particle
     * @return Pointer to i-th daughter particles
     */
    const Particle* getDaughter(unsigned i) const;

    /** Apply a function to all daughters of this particle
     *
     * @param function function object to run on each daughter. If this
     *    function returns true the processing will be stopped immediately.
     * @param recursive if true go through all daughters of daughters as well
     * @param includeSelf if true also apply the function to this particle
     * @return true if the function returned true for any of the particles it
     *    was applied to
     */
    bool forEachDaughter(const std::function<bool(const Particle*)>& function,
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
     * Returns a vector of pointers to all generations' daughter particles
     * @return vector of pointers to all generations' daughter particles
     */
    std::vector<const Belle2::Particle*> getAllDaughters() const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /**
     * Returns a vector of StoreArray indices of given MDST dataobjects
     *
     * @param type EParticleSourceObject corresponding to a given MDST dataobject
     * @return vector of StoreArray indices of a given MDST dataobjects
     */
    std::vector<int> getMdstArrayIndices(EParticleSourceObject type) const;

    /**
     * Returns true if final state ancestors of oParticle overlap
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
     * @param doDetailedComparison if true, this means that particles of different PDG codes,
     *        but created from the same track or cluster will be indicated as copies.
     *        Returns B2FATAL in case of comparison of c_MCParticle type to a non c_MCParticle.
     * @return true if particles are copies of each-other, otherwise false
     */
    bool isCopyOf(const Particle* oParticle, bool doDetailedComparison = false) const;

    /**
     * Returns the pointer to the Track object that was used to create this Particle (ParticleType == c_Track).
     * NULL pointer is returned, if the Particle was not made from Track.
     * @return const pointer to the Track
     */
    const Track* getTrack() const;

    /**
     * Returns the pointer to the TrackFitResult that was used to create this Particle (ParticleType == c_Track).
     * NULL pointer is returned, if the Particle was not made from Track.
     * @return const pointer to the TrackFitResult
     */
    const TrackFitResult* getTrackFitResult() const;

    /**
     * Returns the pointer to the V0 object that was used to create this
     * Particle (if ParticleType == c_V0). NULL pointer is returned if the
     * Particle was not made from a V0.
     * @return const pointer to the V0
     */
    const V0* getV0() const;

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
     * Returns the pointer to the ECLCluster object that was used to create this Particle (if ParticleType == c_ECLCluster).
     * Returns the pointer to the most energetic ECLCluster matched to the track (if ParticleType == c_Track).
     * NULL pointer is returned, if the Particle has no relation to an ECLCluster (either the particle is a different type or there was no track match).
     * @return const pointer to the ECLCluster
     */
    const ECLCluster* getECLCluster() const;

    /**
     * Returns the energy of the ECLCluster for the particle.
     * The return value depends on the ECLCluster hypothesis.
     * @return energy of the ECLCluster
     */
    double getECLClusterEnergy() const;

    /**
     * Returns the pointer to the KLMCluster object that was used to create this Particle (ParticleType == c_KLMCluster).
     * Returns the pointer to the KLMCluster object associated to this Particle if ParticleType == c_Track.
     * NULL pointer is returned, if the Particle has no relation to the KLMCluster.
     * @return const pointer to the KLMCluster
     */
    const KLMCluster* getKLMCluster() const;

    /**
     * Returns the pointer to the MCParticle object that was used to create this Particle (ParticleType == c_MCParticle).
     * Returns the best MC match for this particle (if ParticleType == c_Track or c_ECLCluster or c_KLMCluster)
     * NULL pointer is returned, if the Particle was not made from MCParticle or not matched.
     *
     * @return const pointer to the MCParticle
     */
    const MCParticle* getMCParticle() const;

    /** Return name of this particle. */
    std::string getName() const override;

    /** Return a short summary of this object's contents in HTML format. */
    std::string getInfoHTML() const override;

    /**
     * Prints the contents of a Particle object to standard output.
     */
    void print() const;

    /** get a list of the extra info names */
    std::vector<std::string> getExtraInfoNames() const;

    /**
     * Remove all stored extra info fields
     */
    void removeExtraInfo();

    /** Return given value if set.
     *
     * throws std::runtime_error if variable is not set.
     */
    double getExtraInfo(const std::string& name) const;

    /** Return whether the extra info with the given name is set. */
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

    /**
     * Sets the user defined extraInfo. Adds it if necessary, overwrites existing ones if they share the same name.
     * */
    void writeExtraInfo(const std::string& name, const double value);

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::runtime_error if variable isn't set.
     */
    void setExtraInfo(const std::string& name, double value);

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::runtime_error if variable is already set.
     */
    void addExtraInfo(const std::string& name, double value);

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
    int getPDGCodeUsedForFit() const
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

    /**
     * Returns true if the (track-based) particle is created with its most likely mass hypothesis
     * based on PID likelihood.
     */
    bool isMostLikely() const;

    /**
     * For a (track-based) particle, returns the charged stable mass hypothesis associated to the most probable TrackFitResult,
     * and the TrackFitResult itself.
     */
    std::pair<Const::ChargedStable, const TrackFitResult*> getMostLikelyTrackFitResult() const;

    /**
     * Returns true if the (track-based) particle is created with its most likely mass hypothesis
     * based on TrackFitResult.
     */
    bool isMostLikelyTrackFitResult() const;

    /**
    * Returns the ECLCluster EHypothesisBit for this Particle.
    */
    ECLCluster::EHypothesisBit getECLClusterEHypothesisBit() const
    {
      const int pdg = abs(getPDGCode());
      if ((pdg == Const::photon.getPDGCode())
          or (pdg == Const::electron.getPDGCode())
          or (pdg == Const::muon.getPDGCode())
          or (pdg == Const::pion.getPDGCode())
          or (pdg == Const::kaon.getPDGCode())
          or (pdg == Const::proton.getPDGCode())
          or (pdg == Const::deuteron.getPDGCode())) {
        return ECLCluster::EHypothesisBit::c_nPhotons;
      } else if ((pdg == Const::Klong.getPDGCode())
                 or (pdg == Const::neutron.getPDGCode())) {
        return ECLCluster::EHypothesisBit::c_neutralHadron;
      } else {
        return ECLCluster::EHypothesisBit::c_none;
      }
    }

    /**
    * Explores the decay tree of the particle and returns the (grand^n)daughter identified by a generalized index.
    * The generalized index consists of a colon-separated list of daughter indexes, starting from the root particle:
    * 0:1:3 identifies the fourth daughter (3) of the second daughter (1) of the first daughter (0) of the mother particle.
    * @param generalizedIndex the generalized index of the particle to be returned
    * @return a particle in the decay tree of the root particle.
    */
    const Particle* getParticleFromGeneralizedIndexString(const std::string& generalizedIndex) const;

    /**
     * Propagate the photon energy scaling to jacobian elements that were calculated using energy
     */
    void updateJacobiMatrix();

    /**
     * Fill final state particle daughters into a vector
     *
     * Function is called recursively
     * @param fspDaughters vector of daughter particles
     */
    void fillFSPDaughters(std::vector<const Belle2::Particle*>& fspDaughters) const;

    /**
     * Fill all generations' daughters into a vector
     *
     * Function is called recursively
     * @param allDaughters vector of daughter particles
     */
    void fillAllDaughters(std::vector<const Belle2::Particle*>& allDaughters) const;


  private:

    // persistent data members
    int m_pdgCode;  /**< PDG code */
    int m_pdgCodeUsedForFit = 0; /**< PDG code used for the track fit */
    double m_mass;   /**< particle (invariant) mass */
    double m_px;     /**< momentum component x */
    double m_py;     /**< momentum component y */
    double m_pz;     /**< momentum component z */
    double m_momentumScale = 1.0; /**< effective momentum scale factor */
    double m_momentumScalingFactor = 1.0; /**< momentum scaling factor */
    double m_momentumSmearingFactor = 1.0; /**< momentum smearing factor */
    double m_energyLossCorrection = 0.0; /**< energy loss correction. defined as 'm_energyLossCorrection = E_measured - E_true'*/
    double m_x;      /**< position component x */
    double m_y;      /**< position component y */
    double m_z;      /**< position component z */
    double m_errMatrix[c_SizeMatrix] = {}; /**< error matrix (1D representation) */
    double m_jacobiMatrix[c_SizeMatrix] = {}; /**< error matrix (1D representation) */
    double m_pValue;   /**< chi^2 probability of the fit. Default is nan */
    std::vector<int> m_daughterIndices;  /**< daughter particle indices */
    EFlavorType m_flavorType;  /**< flavor type. */
    EParticleSourceObject m_particleSource;  /**< (mdst) source of particle */
    unsigned m_mdstIndex;  /**< 0-based index of MDST store array object */
    int m_properties; /**< particle property */
    std::vector<int> m_daughterProperties; /**< daughter particle properties */

    /**
     * Identifier that can be used to identify whether the particle is unique
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
    std::vector<double> m_extraInfo;

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
     * Resets 4x6 error matrix
     * All elements are set to 0.0
     */
    void resetJacobiMatrix();

    /**
     * Stores 7x7 error matrix into private member m_errMatrix
     * @param errMatrix 7x7 error matrix
     */
    void storeErrorMatrix(const TMatrixFSym& errMatrix);

    /**
     * Stores 4x6 Jacobi matrix into private member m_jacobiMatrix
     * @param jacobiMatrix 4x6 error matrix
     */
    void storeJacobiMatrix(const TMatrixF& jacobiMatrix);

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

    /**
     * Generate the PDG code with correct sign, using the charge
     * @param chargedSign charge of the particle
     * @param chargedStable Type of charged particle
     */
    int generatePDGCodeFromCharge(const int chargedSign, const Const::ChargedStable& chargedStable);

    ClassDefOverride(Particle, 17); /**< Class to store reconstructed particles. */
    // v8: added identifier, changed getMdstSource
    // v9: added m_pdgCodeUsedForFit
    // v10: added m_properties
    // v11: added m_daughterProperties
    // v12: renamed EParticleType m_particleType to EParticleSourceObject m_particleSource
    // v13: added m_momentumScale
    // v14: added m_jacobiMatrix
    // v15: added m_momentumScalingFactor and m_momentumSmearingFactor
    // v16: use double precision for private members
    // v17: added m_energyLossCorrection
    friend class ParticleSubset;
  };

} // end namespace Belle2
