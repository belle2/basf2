/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <analysis/TMVAInterface/Config.h>
#include <boost/property_tree/ptree_fwd.hpp>

#include <vector>
#include <string>
#include <map>
#include <memory>
namespace TMVA { class Reader; }

namespace Belle2 {
  class Particle;

  namespace TMVAInterface {

    /**
     * Interface to ROOT TMVA Reader.
     * Requires the output of TMVATeacher.
     */
    class Expert {

    public:

      /**
       * @param prefix which used to identify the outputted training files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml
       * @param workingDirectory where the config file and weight file directory is stored
       * @param methodName name of the method which is used by this expert
       * @param signalClass Class which is considered as signal
       */
      Expert(ExpertConfig config, bool transformToProbability);

      /**
       * Analyse a Particle with the given method and calculates signal probability
       * @param particle the particle which should be analysed
       * @return signal probability of the particle
       */
      float analyse(const Particle*);

      /**
       * Analyse a vector with the given method and calculates signal probability
       * @param vector containing features and spectators in correct order
       * @return signal probability of the particle
       */
      float analyse(const std::vector<float>&);


    private:

      /**
       * Internal analyse function
       */
      float _analyse();

      float m_signalFraction; /**< Signal fraction in training */
      ExpertConfig m_config; /**< Expert config */
      bool m_transformToProbability; /**< Transform output of classifier to a probability */

      std::shared_ptr<TMVA::Reader> m_reader; /**<TMVA::Reader, which steers the booked TMVA method */
      std::vector<float> m_input; /**< Store place for the input variables */
      std::vector<const Variable::Manager::Var*> m_variables; /**< Pointers to the input variables */
      std::vector<const Variable::Manager::Var*> m_spectators; /**< Pointers to the input spectators */

    };
  }
}
