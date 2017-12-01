/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hyacinth Stypula                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef INCLUSIVEBTAGRECONSTRUCTIONMODULE_H
#define INCLUSIVEBTAGRECONSTRUCTIONMODULE_H

#include <framework/core/Module.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <map>
#include <vector>


namespace Belle2 {
  /**
   * Inclusively reconstructs `anti-B:tag` from input ParticleLists for given `B:sig`.
   *
   * `upsilonListName` ParticleList is filled with `upsilonListName -> bsigListName btagListName`,
   * where `anti-B:tag` Particle is inclusively reconstructed from particles in input ParticleLists,
   * which do not share any final state particles (mdstSource) with `B:sig`.
   *
   * Input ParticleLists are passed as std::vector containing their names (`inputListsNames`).
   */
  class InclusiveBtagReconstructionModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    InclusiveBtagReconstructionModule();

    /**  */
    virtual ~InclusiveBtagReconstructionModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void event();


  private:

    std::string m_upsilonListName;  /**< Name of the ParticleList to be filled with `Upsilon(4S) -> B:sig anti-B:tag` */
    std::string m_bsigListName;  /**< Name of the Bsig ParticleList */
    std::string m_btagListName;  /**< Name of the Btag ParticleList */
    std::vector<std::string> m_inputListsNames;  /**< Names of the ParticleLists to be used to reconstruct Btag */

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor for parsing the user specifed DecayString */
  };


  /**
   * Helper class to make a vector of all possible combinations of `int` numbers contained in the input vectors (passed as values in a map).
   * In every resulting combination, there is only one number from each input vector.
   *
   * It is used by InclusiveBtagReconstructionModule to combine Particle indices (in StoreArray) for different mdst sources (getMdstSource)
   * to form std::vector of all combinations not sharing common mdst source.
   */
  class Map2Vector {

  public:
    /**
     * Do the conversion using makeEntries().
     * (e.g convert map<mdstSource, vector<arrayIndex>> to vector<vector<daughterArrayIndex>>)
     *
     * @param input - the input map
     * @param output - an empty vector to push back output combinations
     */
    void convert(std::map<int, std::vector<int> >& input, std::vector<std::vector<int> >& output);

  protected:
    /**
     * Recursively iterates over a map until the end is reached, then the output is ready.
     *
     * @param positionOnTheMap - current position on the input map
     * @param end - the end of the input map
     * @param i - current m_combination index
     * @param output - a vector to push back resulting combinations
     */
    void makeEntries(std::map<int, std::vector<int>>::iterator positionOnTheMap,
                     const std::map<int, std::vector<int>>::const_iterator& end, unsigned i,
                     std::vector<std::vector<int>>& output);

  private:
    std::vector<int>
    m_combination; /**< Vector containing current combination of numbers (e.g. arrayIndices of current Btag candidate's children) */

  };
}

#endif /* INCLUSIVEBTAGRECONSTRUCTIONMODULE_H */
