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

#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/version.hpp>

#include <vector>
#include <string>
#include <map>
#include <memory>

namespace Belle2 {

  namespace TMVAInterface {

    /**
     * TMVA Interface Teacher Config
     */
    class Config {

    public:

      /**
       * Create config by defining all properties of a config
       * @param prefix which used to identify the outputted training files weights/$prefix_$method.class.C,
       *        weights/$prefix_$method.weights.xml as well the config file $prefix.config
       * @param workingDirectory where the config file and weight file directory is stored
       */
      Config(const std::string& prefix, const std::string& workingDirectory);

      /**
       * Load feature variables from VariableManager
       */
      std::vector<const Variable::Manager::Var*> getVariablesFromManager() const;

      /**
       * Load spectator spectators from VariableManager
       */
      std::vector<const Variable::Manager::Var*> getSpectatorsFromManager() const;

      /**
       * Load feature variables
       */
      std::vector<std::string> getVariables() const { return m_variables; }

      /**
       * Load spectator spectators
       */
      std::vector<std::string> getSpectators() const { return m_spectators; }

      /**
       * Return working directory
       */
      std::string getWorkingDirectory() const;

      /**
       * Return name of ROOT tree storing the data
       */
      std::string getTreeName() const;

      /**
       * Return name of ROOT file storing the data
       */
      std::string getFileName() const;

      /**
       * Returns config file name for given class id
       */
      std::string getConfigFileName(int signal_class) const;

      /**
       * Return prefix
       */
      std::string getPrefix() const;

    protected:
      std::string m_prefix; /**< Prefix used to identify files */
      std::string m_workingDirectory; /**< where the config file and weight file directory is stored */
      std::vector<std::string> m_variables; /**< input variables */
      std::vector<std::string> m_spectators; /**< input spectators */

    };


    /**
     * TMVA Interface Teacher Config
     */
    class TeacherConfig : public Config {

    public:

      /**
       * Create config by defining all properties of a config
       * @param prefix which used to identify the outputted training files weights/$prefix_$method.class.C,
       *        weights/$prefix_$method.weights.xml as well the config file $prefix.config
       * @param workingDirectory where the config file and weight file directory is stored
       * @param variables the names of the feature variables (registered in Variable::Manager), which are used as input for the chosen TMVA methods
       * @param spectators the names of the spectator variables (registered in Variable::Manager), which are saved as additional spectators
       * @param methods vector of Method config objects
       */
      TeacherConfig(const std::string& prefix, const std::string& workingDirectory,
                    const std::vector<std::string>& variables, const std::vector<std::string>& spectators,
                    const std::vector<Method>& methods);

      /**
       * Return TMVA Methods
       */
      const std::vector<Method>& getMethods() const;


      /**
       * Save config file for given signal class and fraction
       * @param signalClass
       * @param signalFraction
       */
      void save(int signalClass, float signalFraction) const;

    private:
      /**
       * Save config to given property tree
       */
      void saveToXML(int signalClass, float signalFraction) const;

    private:

      std::vector<Method> m_methods; /**< vector of Method config objects */

    };

    /**
     * TMVA Interface Expert Config
     */
    class ExpertConfig : public Config {

    public:

      /**
       * Create config from $prefix.config file
       * @param prefix prefix used to find the correct config file
       * @param workingDirectory where the config file and weight file directory is stored
       */
      ExpertConfig(const std::string& prefix, const std::string& workingDirectory, const std::string& method, int signal_class,
                   float signal_fraction = -1);

      /**
       * Return TMVA Method
       */
      std::string getMethod() const;

      /**
       * Return Weightfile
       */
      std::string getWeightfile() const;

      /**
       * Return signal fraction
       */
      float getSignalFraction() const;

      /**
       * Return signal class
       */
      int getSignalClass() const;


    private:

      /**
       * Return weight file from XML config file
       */
      std::string getMethodPropertyFromXML(const boost::property_tree::ptree& pt, const std::string& property) const;

      /**
       * Return Signal fraction from XML config file
       */
      float getSignalFractionFromXML(const boost::property_tree::ptree& pt) const;

      /**
       * Load target from config file
       */
      std::string getTargetFromXML(const boost::property_tree::ptree& pt) const;

      /**
       * Load variables from config file
       */
      std::vector<std::string> getVariablesFromXML(const boost::property_tree::ptree& pt) const;

      /**
       * Load spectators from config file
       */
      std::vector<std::string> getSpectatorsFromXML(const boost::property_tree::ptree& pt) const;

    private:
      std::string m_method; /**< Used TMVA method */
      std::string m_weightfile; /**< Used TMVA method */
      int m_signal_class; /**< Signal class id */
      float m_signal_fraction; /**< Signal class fraction in training or as provided by user */

    };


  }
}
