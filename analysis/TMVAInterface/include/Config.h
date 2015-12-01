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
#include <analysis/VariableManager/Manager.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <vector>
#include <string>

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
       * @param extraData optional extra data
       */
      Config(const std::string& prefix, const std::string& workingDirectory,
             const std::map<std::string, std::vector<double>>& extraData = {});

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
       * Adds extra data to the config
       * @param key used to save this data
       * @param data as std::vector
       */
      void addExtraData(const std::string& key, const std::vector<double>& data);

      /**
       * Checks if key is available in the extra data
       * @param key of the extra data
       */
      bool hasExtraData(const std::string& key) const { return m_extraData.find(key) != m_extraData.end(); }

      /**
       * Gets the extra data
       */
      std::map<std::string, std::vector<double>> getExtraData() const { return m_extraData; }

      /**
       * Gets the extra data stored under the given key
       * @param key of the extra data
       */
      std::vector<double> getExtraData(const std::string& key) { return m_extraData[key]; }

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
      std::string m_treeName; /**< tree name in output file*/
      std::string m_workingDirectory; /**< where the config file and weight file directory is stored */
      std::vector<std::string> m_variables; /**< input variables */
      std::vector<std::string> m_spectators; /**< input spectators */
      std::map<std::string, std::vector<double>> m_extraData; /**< map of additional data */

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
       * @param extraData optional additional data
       */
      TeacherConfig(const std::string& prefix, const std::string& treeName, const std::string& workingDirectory,
                    const std::vector<std::string>& variables, const std::vector<std::string>& spectators,
                    const std::vector<Method>& methods, const std::map<std::string, std::vector<double>>& extraData = {});

      /**
       * Return TMVA Methods
       */
      const std::vector<Method>& getMethods() const;

      /**
       * Return tree name used in output file of teacher
       */
      std::string getTeacherTreeName() const;

      /**
       * Save config file for given signal class and fraction
       * @param signalClass
       * @param signalFraction
       */
      void save(int signalClass, double signalFraction) const;

    private:
      /**
       * Save config to given property tree
       */
      void saveToXML(int signalClass, double signalFraction) const;

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
                   double signal_fraction = -1);

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
      double getSignalFraction() const;

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
       * Load extraData from config file
       */
      std::map<std::string, std::vector<double>> getExtraDataFromXML(const boost::property_tree::ptree& pt) const;

      /**
       * Return Signal fraction from XML config file
       */
      double getSignalFractionFromXML(const boost::property_tree::ptree& pt) const;

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
      double m_signal_fraction; /**< Signal class fraction in training or as provided by user */

    };


  }
}
