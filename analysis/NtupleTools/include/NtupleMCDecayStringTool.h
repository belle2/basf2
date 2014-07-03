/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Bastian Kronenbitter                                     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <string>

class TTree;

namespace Belle2 {

  /** Add the Monte Carlo decay string to the NTuple.
   *  The ParticleMCDecayStringModule must have been run first. */
  class NtupleMCDecayStringTool : public NtupleFlatTool {

  public:
    /** Constructor. */
    NtupleMCDecayStringTool(TTree* tree, DecayDescriptor& decaydescriptor);
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);

  private:
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  private:
    /** Bookkeeping variable **/
    static const unsigned int c_lengthOfDecayString = 128;

    /** The decay string **/
    char m_decayString[c_lengthOfDecayString];

    const std::string c_ExtraInfoName = "DecayHash"; /** Name of the extraInfo, which is stored in each Particle **/
  };

} // namespace Belle2
