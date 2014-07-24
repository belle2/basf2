/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/TMVAInterface/Method.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>

#include <TMVA/Reader.h>
#include <TMVA/Tools.h>
#include <TSystem.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <memory>

namespace Belle2 {

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
      Expert(std::string prefix, std::string workingDirectory, std::string methodName, int signalClass);

      /**
       * Analyse a Particle with the given method and calculates signal probability
       * @param particle the particle which should be analysed
       * @param signalFraction which is used to transform the method output to a probability. -1 to disable the transformation.
       * @return signal probability of the particle
       */
      float analyse(const Particle*, float signalFraction = -1);


    private:
      /**
       * Load variables from config file
       */
      std::vector<std::string> getVariablesFromXML(const boost::property_tree::ptree& pt) const;

      /**
       * Load class fractions from config file
       */
      std::map<int, float> getClassFractionsFromXML(const boost::property_tree::ptree& pt) const;

      /**
       * Load TMVA::Reader from config file
       */
      std::shared_ptr<TMVA::Reader> getReaderFromXML(const boost::property_tree::ptree& pt);

    private:
      int m_signalClass; /**< Class which is considered as signal */
      std::string m_methodName; /**< name of the method which is used for the expert */
      bool m_reverse; /**< true if the reader returns background probability */
      std::map<int, float> m_classFractions; /**< class fraction in training sample */

      std::shared_ptr<TMVA::Reader> m_reader; /**<TMVA::Reader, which steers the booked TMVA method */
      std::vector<float> m_input; /**< Store place for the input variables */
      std::vector<const Variable::Manager::Var*> m_variables; /**< Pointers to the input variables */

    };
  }
}

