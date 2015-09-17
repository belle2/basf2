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
   * Combines Particle indices (in StoreArray) for different mdst sources (getMdstSource)
   * to form std::vector of all combinations not sharing common mdst source.
   */
  class Map2Vector {

  public:
    /**
     * Converts map<mdstSource, vector<arrayIndex>> to vector<vector<daughterArrayIndex>>
     */
    void convert(std::map<int, std::vector<int> >& d, std::vector<std::vector<int> >& out);

  protected:
    void makeEntries(std::map<int, std::vector<int>>::iterator j, const std::map<int, std::vector<int>>::const_iterator& e, unsigned i,
                     std::vector<std::vector<int>>& out);

  private:
    std::vector<int> o;
  };
}

#endif /* INCLUSIVEBTAGRECONSTRUCTIONMODULE_H */
