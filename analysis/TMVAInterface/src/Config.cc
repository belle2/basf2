/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/TMVAInterface/Config.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>

#include <TMVA/Tools.h>
#include <TMVA/Reader.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/version.hpp>

namespace Belle2 {

  namespace TMVAInterface {

    Config::Config(const std::string& prefix, const std::string& workingDirectory,
                   const std::map<std::string, std::vector<double>>& extraData) : m_prefix(prefix),
      m_workingDirectory(workingDirectory), m_extraData(extraData)
    {
      if (m_workingDirectory.back() != '/') {
        m_workingDirectory += '/';
      }
    }

    std::string Config::getPrefix() const
    {
      return m_prefix;
    }

    std::string Config::getTreeName() const
    {
      return m_prefix + "_tree";
    }

    std::string Config::getFileName() const
    {
      return m_prefix + ".root";
    }

    std::string Config::getWorkingDirectory() const
    {
      return m_workingDirectory;
    }

    std::string Config::getConfigFileName(int signal_class) const
    {
      return m_prefix + "_" + std::to_string(signal_class) + ".config";
    }

    std::vector<const Variable::Manager::Var*> Config::getVariablesFromManager() const
    {
      Variable::Manager& manager = Variable::Manager::Instance();
      const auto& vars =  manager.getVariables(m_variables);
      if (vars.size() != m_variables.size()) {
        B2ERROR("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
      }
      return vars;
    }

    std::vector<const Variable::Manager::Var*> Config::getSpectatorsFromManager() const
    {
      Variable::Manager& manager = Variable::Manager::Instance();
      const auto& vars =  manager.getVariables(m_spectators);
      if (vars.size() != m_spectators.size()) {
        B2ERROR("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
      }
      return vars;
    }

    void Config::addExtraData(const std::string& key, const std::vector<double>& data)
    {
      m_extraData[key] = data;
    }


    TeacherConfig::TeacherConfig(const std::string& prefix, const std::string& treeName, const std::string& workingDirectory,
                                 const std::vector<std::string>& variables,
                                 const std::vector<std::string>& spectators,
                                 const std::vector<Method>& methods,
                                 const std::map<std::string, std::vector<double>>& extraData) :
      Config(prefix, workingDirectory, extraData), m_methods(methods)
    {
      m_variables = variables;
      m_spectators = spectators;
      m_treeName = treeName;
    }


    const std::vector<Method>& TeacherConfig::getMethods() const
    {
      return m_methods;
    }

    std::string TeacherConfig::getTeacherTreeName() const
    {
      return m_treeName;
    }

    void TeacherConfig::save(int signalClass, double signalFraction) const
    {
      saveToXML(signalClass, signalFraction);
    }

    void TeacherConfig::saveToXML(int signalClass, double signalFraction) const
    {
      boost::property_tree::ptree pt;

      pt.put("SignalID", signalClass);
      pt.put("SignalFraction", signalFraction);

      for (auto& x : m_variables) {
        boost::property_tree::ptree node;
        node.put("Name", x);
        pt.add_child("Variables.Variable", node);
      }

      for (auto& x : m_spectators) {
        boost::property_tree::ptree node;
        node.put("Name", x);
        pt.add_child("Spectators.Spectator", node);
      }

      for (auto& x : m_extraData) {
        boost::property_tree::ptree node;
        node.put("Name", x.first);
        node.put("Size", x.second.size());
        for (unsigned int i = 0; i < x.second.size(); ++i)
          node.put(std::to_string(i), x.second[i]);
        pt.add_child("ExtraData.ExtraDatum", node);
      }

      // Append the trained methods to the config xml file
      for (auto& method : m_methods) {
        boost::property_tree::ptree node;
        node.put("MethodName", method.getName());
        node.put("MethodType", method.getTypeAsString());
        node.put("MethodConfig", method.getConfig());
        node.put("Samplefile", m_prefix + ".root");
        node.put("Weightfile", std::string("weights/") + m_prefix + std::string("_") + std::to_string(signalClass)
                 + std::string("_") + method.getName() + std::string(".weights.xml"));
        pt.add_child("Methods.Method", node);
      }

#if BOOST_VERSION < 105600
      boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#else
      boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#endif
      boost::property_tree::xml_parser::write_xml(getConfigFileName(signalClass), pt, std::locale(), settings);
    }

    ExpertConfig::ExpertConfig(const std::string& prefix, const std::string& workingDirectory, const std::string& method,
                               int signal_class, double signal_fraction) :
      Config(prefix, workingDirectory), m_method(method), m_signal_class(signal_class), m_signal_fraction(signal_fraction)
    {
      std::string type;
      std::string config;

      std::ifstream configstream(m_workingDirectory + getConfigFileName(signal_class));
      if (configstream.good()) {

        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(configstream, pt);

        m_variables = getVariablesFromXML(pt);
        m_spectators = getSpectatorsFromXML(pt);
        m_weightfile = getMethodPropertyFromXML(pt, "Weightfile");
        type = getMethodPropertyFromXML(pt, "MethodType");
        config = getMethodPropertyFromXML(pt, "MethodConfig");

        if (signal_fraction < 0) {
          m_signal_fraction = getSignalFractionFromXML(pt);
        }

        m_extraData = getExtraDataFromXML(pt);

      } else {
        // Legacy mode
        B2WARNING("Failed to load config file. Fallback to legacy mode!");
        std::ifstream legacyconfigstream(m_workingDirectory + m_prefix + ".config");
        if (not legacyconfigstream.good()) {
          B2FATAL("Couldn't open config file '" << m_workingDirectory << getConfigFileName(signal_class) << "'");
        }

        B2WARNING("Found legacy config file. In legacy mode the signalFraction can be wrong if you used inverseSamplingRates or weights!");

        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(legacyconfigstream, pt);

        // Get Variables from XML
        for (const auto& f : pt.get_child("Setup.Variables")) {
          if (f.first.data() != std::string("Variable"))
            continue;
          m_variables.push_back(f.second.get<std::string>("Name"));
        }

        // Get Spectators from XML
        boost::optional<boost::property_tree::ptree& > child = pt.get_child_optional("Setup.Spectators");
        if (child) {
          for (const auto& f : pt.get_child("Setup.Spectators")) {
            if (f.first.data() != std::string("Spectator"))
              continue;
            m_spectators.push_back(f.second.get<std::string>("Name"));
          }
        }

        // Get Weightfile from XML
        for (const auto& f : pt.get_child("Setup.Trainings")) {
          if (f.first.data() != std::string("Training"))
            continue;

          for (const auto& g : f.second.get_child("Methods")) {
            if (g.first.data() != std::string("Method"))
              continue;

            if (m_method != g.second.get<std::string>("MethodName"))
              continue;

            if (m_signal_class != g.second.get<int>("SignalID"))
              continue;

            type = g.second.get<std::string>("MethodType");
            m_weightfile = g.second.get<std::string>("Weightfile");
          }
        }

        if (m_signal_fraction < 0) {
          std::map<int, double> classFractions;
          for (const auto& f : pt.get_child("Setup.Clusters")) {

            if (f.first.data() != std::string("Cluster"))
              continue;

            int id = f.second.get<float>("ID");
            double fraction = f.second.get<double>("Fraction");
            classFractions[id] = fraction;
          }
          m_signal_fraction = classFractions[m_signal_class];
        }

      }

      // Make sure that plugin is loaded
      Method dummy(m_method, type, config);

    }

    double ExpertConfig::getSignalFraction() const
    {
      return m_signal_fraction;
    }

    std::string ExpertConfig::getMethod() const
    {
      return m_method;
    }

    int ExpertConfig::getSignalClass() const
    {
      return m_signal_class;
    }


    std::string ExpertConfig::getWeightfile() const
    {
      return m_weightfile;
    }

    double ExpertConfig::getSignalFractionFromXML(const boost::property_tree::ptree& pt) const
    {
      return pt.get<double>("SignalFraction");
    }

    std::vector<std::string> ExpertConfig::getVariablesFromXML(const boost::property_tree::ptree& pt) const
    {
      std::vector<std::string> variables;
      for (const auto& f : pt.get_child("Variables")) {
        if (f.first.data() != std::string("Variable"))
          continue;
        variables.push_back(f.second.get<std::string>("Name"));
      }
      return variables;
    }

    std::map<std::string, std::vector<double>> ExpertConfig::getExtraDataFromXML(const boost::property_tree::ptree& pt) const
    {
      std::map<std::string, std::vector<double>> extraData;
      boost::optional< const boost::property_tree::ptree& > child = pt.get_child_optional("ExtraData");
      if (!child)
        return extraData;

      for (const auto& f : pt.get_child("ExtraData")) {
        if (f.first.data() != std::string("ExtraDatum"))
          continue;
        unsigned int size = f.second.get<int>("Size");
        std::vector<double> temp(size, 0);
        for (unsigned int i = 0; i < size; ++i) {
          temp[i] = f.second.get<double>(std::to_string(i));
        }
        std::string name = f.second.get<std::string>("Name");
        extraData[name] = temp;
      }
      return extraData;
    }


    std::vector<std::string> ExpertConfig::getSpectatorsFromXML(const boost::property_tree::ptree& pt) const
    {
      std::vector<std::string> spectators;
      boost::optional< const boost::property_tree::ptree& > child = pt.get_child_optional("Spectators");
      if (!child)
        return spectators;

      for (const auto& f : pt.get_child("Spectators")) {
        if (f.first.data() != std::string("Spectator"))
          continue;
        spectators.push_back(f.second.get<std::string>("Name"));
      }
      return spectators;
    }

    std::string ExpertConfig::getMethodPropertyFromXML(const boost::property_tree::ptree& pt, const std::string& property) const
    {
      for (const auto& g : pt.get_child("Methods")) {
        if (g.first.data() != std::string("Method"))
          continue;
        if (m_method != g.second.get<std::string>("MethodName"))
          continue;
        return g.second.get<std::string>(property);

      }
      B2FATAL("Could not find given method " << m_method << " in config file");
      return "";
    }


  }
}


