/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
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

#include <vector>

#include <generators/dataobjects/MCParticle.h>

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>

#include <tracking/dataobjects/Track.h>

// TODO: should this go somehwere elese (Constants.h?)?
static const int C_POSITION_ERROR_MATRIX_DIMENSION                 = 3;  //! Dimension of position error matrix
static const int C_MOMENTUM_ERROR_MATRIX_DIMENSION                 = 4;  //! Dimension of 4-momentum error matrix
static const int C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION        = 7;  //! Dimension of 4-momentun and position error matrix
static const int C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION_1D_REP = 28; //! Dimension of 1-dimensional representation of 7x7 error matrix

namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  class Particle : public RelationsObject {

  public:

    /**
     * A Class to store the reconstructed Particle information. This class
     * is a common representation of all particle types, e.g.:
     *  - final state particles (FSParticles):
     *    - charged kaons/pions/electrons/muons/protons reconstructed as Tracks by tracking detectors
     *    - neutral pions and photons reconstructed as ECLShowers or combination of them
     *    - long lived neutral kaons reconstructed in the KLM
     *  - and composite particles, like Kshorts, D, B mesons, etc...
     *
     * Private members of the Particle class are limited to only those which "define" the
     * particle and are common to all particle types. These are:
     *  - 4-momentum
     *  - vertex/position at which the 4-momentum is estimated
     *  - 7x7 error matrix ( 3x3 for vertex/position and 4x4 for the 4-momentum)
     *  - PDG code
     *  - vector of StoreArray indices of daughter particles
     * Other useful members are:
     *  - mdstID (index of mdst Object in Tracks/ECLShowers/... StoreArray from which the Particle was created)
     *  - enum for particle type [Track, ECLShower, KLMHit(?), Composite]
     */

    /**
     * particle type enumerators
     */
    enum EParticleType {c_Track, c_ECLShower, c_MCParticle, c_Composite, c_Undefined};

    // CONSTRUCTORS

    /**
     * Default constructor
     * All private members are set to 0. Particle type is set to c_Undefined.
     */
    Particle();

    /**
     * Constructor from a 4-momentum vector and PDG code.
     * All other private members are set to their defalt values (0).
     * @param 4-momentum vector
     * @param PDG code
     */
    Particle(const TLorentzVector&, const int pdgCode);

    /**
     * Constructor from a 4-momentum vector, PDG code and vector of daughter indices.
     * All other private members are set to their defalt values (0).
     * @param 4-momentum vector
     * @param PDG code
     * @param vector of daughter indices
     */
    Particle(const TLorentzVector&, const int pdgCode, const std::vector<int>&);

    /**
     * Constructor from a reconstructed track (mdst object Track)
     * @param pointer to the Track object
     * @param StoreArray index of Track object
     * @param PDG code
     */
    Particle(const Track*, const unsigned index, const int pdgCode);

    /**
     * Constructor from a reconstructed gamma candidate (mdst object ECLGamma)
     * @param pointer to the ECLGamma object
     */
    Particle(const ECLGamma*);

    /**
     * Constructor from a reconstructed pi0 candidate (mdst object ECLPi0)
     * @param pointer to the ECLPi0 object
     */
    Particle(const ECLPi0*);

    /**
     *  Constructor from a reconstructed Vee2 candidate (MDSTVee2?)
     */
    // TODO: implement this constructor once MDSTVee2 data object becomes available
    //Particle(const MdstVee2*, const int pdgCode);

    /**
     * Missing constructors: Klong, ??
     */

    /**
     * Constructor from a generated MCParticle (mdst object MCParticle)
     * @param pointer to the MCParticle
     */
    Particle(const MCParticle*);

  public:

    // setters

    /**
     * Sets the 4-momentum vector of the Particle
     * @param 4-momentum vector
     */
    void set4Vector(const TLorentzVector& p4) {
      m_momentum_x = p4.Px();
      m_momentum_y = p4.Py();
      m_momentum_z = p4.Pz();
      m_energy     = p4.Energy();
    }

    /**
     * Sets the point at which 4-momentum vector is estimated in (units??)
     * @param point (position or vertex) at which 4-momentum is estimated in (units??)
     */
    void setVertex(const TVector3& vertex) {
      m_position_x = vertex.X();
      m_position_y = vertex.Y();
      m_position_z = vertex.Z();
    };

    /**
     * Sets the 7x7 error matrix of momentum and vertex/position
     * Diagonal elements are: sigma_px^2, sigma_py^2, sigma_pz^2, sigma_E^2, sigma_x^2, sigma_y^2, sigma_z^2
     * @param The 7x7 momentum and vertex error matrix
     */
    void setMomentumVertexErrorMatrix(const TMatrixFSym& m);

    /**
     * Sets the 4x4 momentum error sub-matrix
     * All other error matrix elements (for position and correlation between
     * position and error) are set to 0
     * @param The 4x4 momentum error matrix
     */
    void setMomentumErrorMatrix(const TMatrixFSym& m_mom);

    /**
     * Sets the 3x3 vertex/position error sub-matrix
     * All other error matrix elements (for momentum and correlation between
     * position and error) are set to 0
     * @param The 4x4 position error matrix
     */
    void setVertexErrorMatrix(const TMatrixFSym& m_pos);

    /**
     * Sets PDG code of the Particle
     * @param The PDG code of the Particle
     */
    void setPDGCode(int pdgCode) { m_pdgCode = pdgCode; }

    // getters
    /**
     * Returns Particle's 4-momentum vector
     * @return The 4-momentum vector of the Particle
     */
    const TLorentzVector get4Vector() const {
      return TLorentzVector(m_momentum_x, m_momentum_y, m_momentum_z, m_energy);
    }

    /**
     * Returns Particle's 3-momentum vector
     * @return The 3-momentum vector of the Particle
     */
    const TVector3 getMomentum() const {
      return TVector3(m_momentum_x, m_momentum_y, m_momentum_z);
    };

    /**
     * Returns Particle's total momentum in GeV
     * @return The total momentum in GeV
     */
    float getTotalMomentum() const {
      return sqrt(m_momentum_x * m_momentum_x + m_momentum_y * m_momentum_y + m_momentum_z * m_momentum_z);
    };

    /**
     * Returns Particle's energy in GeV
     *
     * @return The Particle's energy in GeV
     */
    float getEnergy() const {
      return m_energy;
    }

    /**
     * Returns Particle's x component of momentum in GeV
     *
     * @return x component of mometum in GeV
     */
    float getPx() const {
      return m_momentum_x;
    }

    /**
     * Returns Particle's y component of momentum in GeV
     *
     * @return y component of mometum in GeV
     */
    float getPy() const {
      return m_momentum_y;
    }

    /**
     * Returns Particle's z component of momentum in GeV
     *
     * @return z component of mometum in GeV
     */
    float getPz() const {
      return m_momentum_z;
    }

    /**
     * Returns Particle's invariant mass (= magnitude of 4-momentum) in GeV
     * @return Measured mass
     */
    float getMass() const {
      float mm = m_energy * m_energy - (m_momentum_x * m_momentum_x + m_momentum_y * m_momentum_y + m_momentum_z * m_momentum_z);
      return mm < 0.0 ? -sqrt(-mm) : sqrt(mm);
    }

    /**
     * Returns estimated error of measured mass in GeV
     * @return Estimated error of measured mass
     */
    // TODO: Implement getMassError function
    float getMassError() const { return 0.0; };

    /**
     * Returns Particle's vertex/position in (units??)
     * For Composite particle this is the decay vertex, while for FSParticles it's point
     * at which the momentum is estimated (POCA)
     * @return point at which 4-momentum is estimated
     */
    TVector3 getVertex() const {
      return TVector3(m_position_x, m_position_y, m_position_z);
    };

    /**
     * Returns the 7x7 error matrix of momentum and vertex/position
     * Diagonal elements are: sigma_px^2, sigma_py^2, sigma_pz^2, sigma_E^2, sigma_x^2, sigma_y^2, sigma_z^2
     * @return The 7x7 momentum and vertex error matrix
     */
    TMatrixFSym getMomentumVertexErrorMatrix() const;

    /**
     * Returns the 4x4 momentum error sub-matrix
     * @return The 4x4 momentum error matrix
     */
    TMatrixFSym getMomentumErrorMatrix() const;

    /**
     * Returns the 3x3 position error sub-matrix
     * @return The 3x3 position error matrix
     */
    TMatrixFSym getVertexErrorMatrix() const;

    /**
     * Returns the particle type enum of the Particle
     * @return The particle type enum
     */
    const EParticleType getParticleType() const { return m_particleType; }

    /**
     * Returns PDG code of the Particle
     * @return The PDG code of the Particle
     */
    int getPDGCode(void) const { return m_pdgCode; }

    /**
     * Returns Particle's nominal mass in GeV
     *
     * @return The nominal mass of the Particle
     */
    float getPDGMass(void) const;

    /**
     * Returns Particle's charge in units of e
     *
     * @return The charge of the Particle in units of e
     */
    float getCharge(void) const;

    /**
     * Get 0-based index of the particle in the corresponding Particle StoreArray.
     *
     * @return The index of the Particle in the corresponding Particle StoreArray.
     */
    int getArrayIndex() const { fixParticleList(); return m_particleIndex;  }

    /**
     * Returns index of MDST Object from corresponding StoreArray.
     * Value makes sense only for Final State Particles constructed
     * out of Track/EclShower/... . For other particle types (Composite)
     * the index is 0.
     * @return The index of the MDST Object in the corresponding StoreArray list
     */
    unsigned getMdstArrayIndex(void) const { return m_mdstIndex; }

    // Interface for obtaining/appending daughter Particles
    /**
     * Appends index of daughter Particle to daughters index array of this Particle.
     *
     * @param Pointer to the daughter particle
     */
    void appendDaughter(const Particle* daughter);

    /**
     * Removes index of daughter Particle fromt daughters index array of this Particle.
     *
     * @param Pointer to the daughter particle
     */
    void removeDaughter(const Particle* daughter);

    /**
     * Returns number of daughter particles related to this particle
     * @return Number of daughter particles
     */
    unsigned getNDaughters(void) const {
      return m_daughterIndices.size();
    }

    /**
     * Returns a pointer to the i-th daughter particle
     * (Note that i is 0-based index)
     *
     * @return Pointer to i-th daughter particles
     */
    const Particle* getDaughter(unsigned i) const;

    /**
     * Returns a vector of pointers to daughter particles
     *
     * @return Vector of pointers to daughter particles
     */
    const std::vector<Belle2::Particle*> getDaughters() const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /**
     * Retruns vector of StoreArray indices of daughter particles
     *
     * @return Vector of StoreArray indices of daughter particle
     */
    const std::vector<int>& getDaughterIndices() const { return m_daughterIndices; }

    /**
     * Fills a vector of pointers to Final State daughter particles
     *
     * @param Vector of pointers to particles
     */
    void getFinalStateDaughters(std::vector<const Belle2::Particle*> &fspDaughters) const;
    //Need namespace qualifier because ROOT CINT has troubles otherwise

    /**
     * Returns true if Particles overlap, e.g. consist of same Track/ECLShower/...
     *
     * @return True if particles overlap and false otherwise
     */
    bool overlapsWith(const Particle* oParticle) const;

  private:

    // Private members

    /**
     * Internal pointer to DataStore Array containing particles belonging to
     * this collection.
     *
     * This is a transient member and will not be written to file. The pointer
     * will be set correctly on first access after deserialisation
     */
    TClonesArray* m_plist; //! transient pointer to particle StoreArray

    /**
     * 0-based index of the particle, will be set automatically after
     * deserialisation if needed.
     */
    int m_particleIndex; //! transient 0-based index of particle

    int             m_pdgCode;       /**< PDG-code of the Particle */
    EParticleType   m_particleType;  /**< Particle type: Track/ECLShower/Composite/?? */
    unsigned        m_mdstIndex;     /**< 0-based index of MDST Object in corresponding Store Array. Variable is used only for FSParticles. */

    /**
     * Indices of daughter particles in the StoreArray
     */
    std::vector<int> m_daughterIndices; /**< Daughter particle indices */

    /**
     * 4-momentum vector of particle
     */
    float m_energy;         /**< energy of the particle */
    float m_momentum_x;     /**< momentum of particle, x component */
    float m_momentum_y;     /**< momentum of particle, y component */
    float m_momentum_z;     /**< momentum of particle, z component */

    /**
     * position or vertex of the particle
     * For particles constructed from Track: point of closest approach
     * For composite particles: reconstructed decay vertex
     */
    float m_position_x;     /**< decay vertex (position) of particle, x component */
    float m_position_y;     /**< decay vertex (position) of particle, y component */
    float m_position_z;     /**< decay vertex (position) of particle, z component */

    /**
     * 7x7 (symetric) error matrix of 4-momentum and vertex/position
     * To save on disk space the error matrix is internally in particle class
     * represented as 1D array with 28 elements, however the getter/setter
     * functions return or take as an argument TMatrixFSym
     *
     * The relation between 1-dimensional representation and the 7x7 error matrix is the following:
     * 0 - [0][0] = sigma_px*sigma_px element of the error matrix
     * 1 - [0][1] = sigma_px*sigma_py element of the error matrix
     * 2 - [0][2] = sigma_px*sigma_pz element of the error matrix
     * 3 - [0][3] = sigma_px*sigma_e  element of the error matrix
     * 4 - [0][4] = sigma_px*sigma_x  element of the error matrix
     * 5 - [0][5] = sigma_px*sigma_y  element of the error matrix
     * 6 - [0][6] = sigma_px*sigma_z  element of the error matrix
     *
     * 7  - [1][1] = sigma_py*sigma_py element of the error matrix
     * 8  - [1][2] = sigma_py*sigma_pz element of the error matrix
     * 9  - [1][3] = sigma_py*sigma_e  element of the error matrix
     * 10 - [1][4] = sigma_py*sigma_x  element of the error matrix
     * 11 - [1][5] = sigma_py*sigma_y  element of the error matrix
     * 12 - [1][6] = sigma_py*sigma_z  element of the error matrix
     *
     * 13 - [2][2] = sigma_pz*sigma_pz element of the error matrix
     * 14 - [2][3] = sigma_pz*sigma_e  element of the error matrix
     * 15 - [2][4] = sigma_pz*sigma_x  element of the error matrix
     * 16 - [2][5] = sigma_pz*sigma_y  element of the error matrix
     * 17 - [2][6] = sigma_pz*sigma_z  element of the error matrix
     *
     * 18 - [3][3] = sigma_e*sigma_e  element of the error matrix
     * 19 - [3][4] = sigma_e*sigma_x  element of the error matrix
     * 20 - [3][5] = sigma_e*sigma_y  element of the error matrix
     * 21 - [3][6] = sigma_e*sigma_z  element of the error matrix
     *
     * 22 - [4][4] = sigma_x*sigma_x  element of the error matrix
     * 23 - [4][5] = sigma_x*sigma_y  element of the error matrix
     * 24 - [4][6] = sigma_x*sigma_z  element of the error matrix
     *
     * 25 - [5][5] = sigma_y*sigma_y  element of the error matrix
     * 26 - [5][6] = sigma_y*sigma_z  element of the error matrix
     *
     * 27 - [6][6] = sigma_z*sigma_z  element of the error matrix
     */
    float m_momentumPositionError[C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION_1D_REP]; //< 1-dimensional representation of 7x7 error matrix*/

    // private functions
    /**
     * Resets particle's 7x7 Momentum-Position Error Matrix
     * All elements are set to 0.0
     */
    void resetErrorMatrix();

    /**
     * Converts 7x7 TMatrixFSym to 1 dimensional array holding matrix elements
     */
    void fillErrorMatrix(TMatrixFSym m);

    /**
     * Search the DataStore for the corresponding Particle array.
     *
     * This function is called automatically if the pointer to the
     * particle list is not set when needed, e.g. after deserialization.
     */
    void fixParticleList() const;

    ClassDef(Particle, 1);
  };

  /** @}*/

} // end namespace Belle2

#endif
