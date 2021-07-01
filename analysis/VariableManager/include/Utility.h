/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <framework/utilities/GeneralCut.h>

namespace Belle2 {

  class Particle;

  namespace Variable {
    /**
     * A cut in the analysis package is a general cut using the analysis variable manager.
     *
     * How to use it in your module in detail:
     *
     * private section of the module:
     *   std::string m_cutParameter;
     *   std::unique_ptr<Variable::Cut> m_cut;
     *
     * constructor of the module:
     *   addParam("cut", m_cutParameter, "Selection criteria to be applied", std::string(""));
     *
     * initialize method of the module:
     *   m_cut = Variable::Cut::compile(m_cutParameter);
     *
     * event function of the module:
     *   if(m_cut->check(particlePointer)) {
     *     do something
     *   }
     *
     */
    using Cut = GeneralCut<Belle2::Variable::Manager>;
    /**
     * A check if this variable string is a counter type variable to be handled
     * by the VariablesTo* output modules directly.
     */
    bool isCounterVariable(const std::string& variableName);
  }
}
