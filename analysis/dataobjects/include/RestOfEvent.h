/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Matic Lubej, Sviatoslav Bilokin             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <analysis/VariableManager/Utility.h>
#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <string>
#include <set>
#include <map>

class TLorentzVector;

namespace Belle2 {

  // forward declarations
  class Particle;
  class ECLCluster;
  class KLMCluster;
  class Track;

  // TODO: Add support for the MdstVee dataobjects when they become available.


  /**
   * This is a general purpose class for collecting reconstructed MDST data objects
   * that are not used in reconstruction of given Particle -- referred also as
   * Rest Of the Event. From remaining photons, K_L0 and charged particles, etc.,
   * we infer for example the decay vertex and flavor of the tagging B meson, which are
   * needed in measurements of t-dependent CP violation, or we determine whether or not
   * the rest of the event is consistent with some B decay that involves neutrinos after
   * we have reconstructed Btag hadronically or semileptonically. Remaining charged tracks
   * and unused energy deposits are for example also needed in calculation of continuum
   * suppression variables, like Kakuno-Super-Fox-Wolfram moments. The RestOfEvent class
   * should be used as an input in all these cases.
   *
   * The RestOfEvent object is created for given existing Particle object by the RestOfEventBuilder
   * module and are related between each other with the BASF2 relation.
   *
   * Internally, the RestOfEvent class holds only StoreArray indices of all unused MDST particles.
   * Indices are stored in std::set and not std::vector, since the former ensures uniqueness of all its elements.
   */

  class RestOfEvent : public RelationsObject {

  public:
    /**
     * Structure of Rest of Event mask. It contains array indices of particles, which were selected and associated to this mask after some selection.
     * Host ROE object always check that masks do not contain extra particles,
     * which are not included in ROE initially for consistency.
     * TODO: Will it be written to StoreArray?
     * Maybe should be moved to private.
     */
    struct Mask {
    public:
      /**
       * Default constructor.
       * All private members are set to 0 (all vectors are empty).
       * @param name of mask
       * @param origin of mask, for debug
       */
      Mask(const std::string& name = "", const std::string& origin = "unknown"): m_name(name),
        m_origin(origin)
      {
        B2DEBUG(10, "Mask " << name << " is being initialized by " << origin);
        m_isValid = false;
      };
      /**
       * Get mask name
       */
      std::string getName() const
      {
        return m_name;
      }
      /**
       * Get mask validity
       */
      bool isValid() const
      {
        return m_isValid;
      }
      /**
       *  Add selected particles to the mask
       */
      void addParticles(std::vector<const Particle*>& particles)
      {
        if (isValid()) {
          B2INFO("Mask " + m_name + " is default and valid, cannot write to it!");
          return;
        }
        if (isValid()) {
          B2INFO("Mask " + m_name + " originating from "  + m_origin + " is  valid, cannot write to it!");
          return;
        } else {
          for (auto* particle : particles) {
            m_maskedParticleIndices.insert(particle->getArrayIndex());
          }
          m_isValid = true;
        }
      }
      /**
       *  Get selected particles associated to the mask
       */
      std::set<int> getParticles() const
      {
        return m_maskedParticleIndices;
      }
      /**
       *  Get selected particles associated to the V0 of mask
       */
      std::set<int> getV0s() const
      {
        return m_maskedV0Indices;
      }
      /**
       *  Get selected particles associated to the V0 of mask
       */
      void addV0(const Particle* v0, std::vector<int>& toErase)
      {
        m_maskedV0Indices.insert(v0->getArrayIndex());
        for (int& i : toErase) {
          m_maskedParticleIndices.erase(i);
        }
        m_maskedParticleIndices.insert(v0->getArrayIndex());
      }
      /**
       *  Has selected particles associated to the mask
       */
      bool hasV0(const Particle* v0) const
      {
        return m_maskedV0Indices.count(v0->getArrayIndex()) > 0;
      }
      /**
       *  Clear selected particles associated to the mask
       */

      void clearParticles()
      {
        m_maskedParticleIndices.clear();
        m_maskedV0Indices.clear();
        m_isValid = false;
      }
      /**
       *  Sets ChargedStable fractions
       */
      void setChargedStableFractions(std::vector<double>& chargedStableFractions)
      {
        m_chargedStableFractions = chargedStableFractions;
      }
      /**
       *  Gets ChargedStable fractions
       */
      std::vector<double> getChargedStableFractions() const
      {
        return m_chargedStableFractions;
      }
      /**
       *  Print mask and selected particles associated to the mask
       */
      void print()
      {
        B2INFO("Mask name: " + m_name + " originating from " + m_origin);
        if (!m_isValid) {
          B2INFO("\tNot valid!");
        }
        std::string printout =  "\tIndices: ";
        for (const int index : m_maskedParticleIndices) {
          printout += std::to_string(index) +  ", ";
        }
        if (m_chargedStableFractions.size() > 0) {
          printout += "\n ChargedStable fractions: ";
          for (unsigned int i = 0; i < m_chargedStableFractions.size(); i++) {
            printout += std::to_string(m_chargedStableFractions[i]) + ", ";
          }
        }
        B2INFO(printout);
      }
    private:
      std::string m_name;                       /**< Mask name */
      std::string m_origin;                     /**< Mask origin  for debug */
      bool m_isValid;                           /**< Check if mask has elements or correctly initialized*/
      std::set<int> m_maskedParticleIndices;    /**< StoreArray indices for masked ROE particles */
      std::set<int> m_maskedV0Indices;          /**< StoreArray indices for masked V0 ROE particles */
      std::vector<double> m_chargedStableFractions;          /**<  ChargedStable fractions, never used */
    };
    /**
     * Default constructor.
     * All private members are set to 0 (all vectors are empty).
     */
    explicit RestOfEvent(bool isNested = false): m_isNested(isNested) { };

    // setters
    /**
     * Add StoreArray indices of given Particles to the list of unused particles in the event.
     *
     * @param Reference to a vector of unused Particles
     */
    void addParticles(const std::vector<const Particle*>& particle);
    /**
     * Check if ROE has StoreArray index of given  to the list of unused tracks in the event.
     * @param Pointer to the Particle
     * @param Name of the mask to work with
     */
    bool hasParticle(const Particle* particle, const std::string& maskName = "") const;
    /**
     * Initialize new mask
     * @param Name of the mask to work with
     * @param Name of the creator module
     */
    void initializeMask(const std::string& name, const std::string& origin = "unknown");
    /**
     * Update mask with cuts
     * @param Name of the mask to work with
     * @param Cut on Track particles
     * @param Cut on ECL particles
     * @param Cut on KLM particles
     * @param Update existing mask if true or not if false
     */
    void updateMaskWithCuts(const std::string& name, std::shared_ptr<Variable::Cut> trackCut = nullptr,
                            std::shared_ptr<Variable::Cut> eclCut = nullptr, std::shared_ptr<Variable::Cut> klmCut = nullptr, bool updateExisting = false);
    /**
     * Update mask by keeping or excluding particles
     * @param Name of the mask to work with
     * @param Reference to particle collection
     * @param ParticleType of the collection
     * @param Update the ROE mask by passing or discarding particles in the provided particle list
     */
    void excludeParticlesFromMask(const std::string& maskName, std::vector<const Particle*>& particles,
                                  Particle::EParticleType listType,
                                  bool discard);
    /**
     * True if this ROE object has mask
     * @param Name of the mask to work with
     */
    bool hasMask(const std::string& name) const;
    /**
     * Update mask with composite particle
     * @param Name of the mask to work with
     * @param Pointer to composite particle
     */
    void updateMaskWithV0(const std::string& name, const Particle* particleV0);
    /**
     * Check if V0 can be added, maybe should be moved to private
     */
    bool checkCompatibilityOfMaskAndV0(const std::string& name, const Particle* particleV0);
    /**
     * Get charged stable fractions with a specific mask name
     *
     * @param name of mask
     * @return fractions
     */
    std::vector<double> getChargedStableFractions(const std::string& maskName) const;
    /**
     * Returns true if the ROE is nested
     */
    bool getIsNested() const {return m_isNested;}
    /**
     * Update or add a priori ChargedStable fractions for a specific mask name in the ROE object.
     *
     * @param name of mask
     * @param a priori fractions
     */
    void updateChargedStableFractions(const std::string& maskName, std::vector<double>& fractions);
    // getters
    /**
     * Get all Particles from ROE mask.
     *
     * @param name of mask
     * @param return daughters of composite particles
     * @return vector of pointers to ROE Particles
     */
    std::vector<const Particle*> getParticles(const std::string& maskName = "", bool unpackComposite = true) const;
    /**
    * Get photons from ROE mask.
    *
    * @param name of mask
    * @param return daughters of composite particles
    * @return vector of pointers to unused Particles
    */
    std::vector<const Particle*> getPhotons(const std::string& maskName = "", bool unpackComposite = true) const;
    /**
     * Get hadrons from ROE mask.
     *
     * @param name of mask
     * @param return daughters of composite particles
     * @return vector of pointers to ROE Particles
     */
    std::vector<const Particle*> getHadrons(const std::string& maskName = "", bool unpackComposite = true) const;
    /**
    * Get charged particles from ROE mask.
    *
    * @param name of mask
    * @param absolute value of PDG code of charged particle
    * @param return daughters of composite particles
    * @return vector of pointers to ROE Particles
    */
    std::vector<const Particle*> getChargedParticles(const std::string& maskName = "", unsigned int pdg = 0,
                                                     bool unpackComposite = true) const;

    /**
     * Get vector of all (no mask) or a subset (use mask) of all Tracks in ROE.
     *
     * @param name of mask
     * @return vector of pointers to unused Tracks
     */
    std::vector<const Track*> getTracks(const std::string& maskName = "") const;

    /**
     * Get vector of all (no mask) or a subset (use mask) of all ECLClusters in ROE.
     *
     * @param name of mask
     * @return vector of pointers to unused ECLClusters
     */
    std::vector<const ECLCluster*> getECLClusters(const std::string& maskName = "") const;

    /**
     * Get vector of all unused KLMClusters.
     *
     * @param name of mask
     * @return vector of pointers to unused KLMClusters
     */
    std::vector<const KLMCluster*> getKLMClusters(const std::string& maskName = "") const;

    /**
     * Get 4-momentum vector all (no mask) or a subset (use mask) of all Tracks and ECLClusters in ROE.
     *
     * @param name of mask
     * @return 4-momentum of unused Tracks and ECLClusters in ROE
     */
    TLorentzVector get4Vector(const std::string& maskName = "") const;

    /**
     * OBSOLETE:
     * Get 4-momentum vector all (no mask) or a subset (use mask) of all Tracks in ROE.
     *
     * @param name of mask
     * @return 4-momentum of unused Tracks and ECLClusters in ROE
     */
    TLorentzVector get4VectorTracks(const std::string& maskName = "") const;

    /**
     * Get 4-momentum vector all (no mask) or a subset (use mask) of all ECLClusters in ROE.
     *
     * @param name of mask
     * @return 4-momentum of unused Tracks and ECLClusters in ROE
     */
    TLorentzVector get4VectorNeutralECLClusters(const std::string& maskName = "") const;

    /**
     * Get number of all (no mask) or a subset (use mask) of all Tracks in ROE.
     *
     * @param name of mask
     * @return number of all remaining tracks
     */
    int getNTracks(const std::string& maskName = "") const;

    /**
     * Get number of all (no mask) or a subset (use mask) of all ECLclusters in ROE.
     *
     * @param name of mask
     * @return number of all remaining ECL clusters
     */
    int getNECLClusters(const std::string& maskName = "") const;

    /**
     * Get number of all remaining KLM clusters.
     *
     * @param name of mask
     * @return number of all remaining KLM clusters
     */
    int getNKLMClusters(const std::string& maskName = "") const;

    /**
     * Get vector of all mask names of the ROE object
     * @return list of all mask names
     */
    std::vector<std::string> getMaskNames() const;

    /**
     * OBSOLETE:
     * Added helper function so creation of temporary particles and setting pid relations is not needed
     */
    double atcPIDBelleKpiFromPID(const PIDLikelihood* pid) const;

    /**
     * Prints the contents of a RestOfEvent object to screen
     */
    void print() const;

    /**
     * Compares particles. TODO: This doesn't belong here
     */
    static bool compareParticles(const Particle* roeParticle, const Particle* toAddParticle);

  private:

    // persistent data members
    std::set<int> m_particleIndices;   /**< StoreArray indices to unused particles */
    std::vector<Mask> m_masks;         /**< List of the ROE masks */
    bool m_isNested;                   /**< Nested ROE indicator */
    // Private methods
    /**
     *  Checks if a particle has its copy in the provided list
     */
    bool isInParticleList(const Particle* roeParticle, std::vector<const Particle*>& particlesToUpdate) const;

    /**
     *  Helper method to find ROE mask
     */
    Mask* findMask(const std::string& name);
    /**
     * Prints indices in the given set in a single line
     */
    void printIndices(std::set<int> indices) const;
    ClassDef(RestOfEvent, 5) /**< class definition */

  };


} // end namespace Belle2

