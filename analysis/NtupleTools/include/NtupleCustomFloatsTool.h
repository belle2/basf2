/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLECUSTOMFLOATSTOOL_H
#define NTUPLECUSTOMFLOATSTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Manager.h>
#include <TTree.h>
#include <string>
#include <utility>
#include <vector>

namespace Belle2 {

  /** NtupleTool to write a custom set of float variables to a tree. */
  class NtupleCustomFloatsTool : public NtupleFlatTool {
  private:
    /** The float variables */
    float* m_fVars;
    /** Names of the float variables. */
    std::vector<std::string> m_strVarNames;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleCustomFloatsTool(TTree* tree, DecayDescriptor& decaydescriptor, const std::string& strOptions) : NtupleFlatTool(tree,
          decaydescriptor, strOptions) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

#endif // NTUPLECUSTOMFLOATSTOOL_H
