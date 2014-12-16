/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/TMVAInterface/Expert.h>
#include <analysis/TMVAInterface/Method.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>

#include <TMVA/Tools.h>
#include <TMVA/Reader.h>

#include <TSystem.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace Belle2 {

  namespace TMVAInterface {

    Expert::Expert(std::string prefix, std::string workingDirectory, std::string methodName, int signalClass) : m_signalClass(signalClass), m_methodName(methodName), m_reverse(false)
    {

      // Change directory to user defined working directory
      std::string oldDirectory = gSystem->WorkingDirectory();
      gSystem->ChangeDirectory(workingDirectory.c_str());

      // Initialize TMVA and ROOT stuff
      TMVA::Tools::Instance();

      // Open configfile
      std::string configfile = prefix + ".config";
      std::ifstream configstream(configfile);
      if (not configstream.good()) {
        B2FATAL("Couldn't open config file '" << configfile << "'")
      }

      // Read out configfile
      try {
        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(configstream, pt);

        m_classFractions = getClassFractionsFromXML(pt);
        m_reader = getReaderFromXML(pt);

        // Set signalClass to backgroundClass if there was no reader for signalClass, signalClass itself is a valid class
        // and there are only two classes in the trained sample.
        if (m_reader == nullptr and m_classFractions.size() == 2 and m_classFractions.find(m_signalClass) != m_classFractions.end()) {
          for (const auto & pair : m_classFractions) {
            if (signalClass != pair.first) {
              m_signalClass = pair.first;
            }
          }
          m_reader = getReaderFromXML(pt);
          m_reverse = true;
        }


      } catch (const std::exception& ex) {
        B2FATAL("There was an error while scanning the stream of the file for the used variables and the used method: " << ex.what())
      }

      if (m_reader == nullptr) {
        B2FATAL("Failed to load a TMVA::Reader with the given name and signalClass from the config file.")
        return;
      }

      // Change back to old directory
      gSystem->ChangeDirectory(oldDirectory.c_str());

    }

    float Expert::analyse(const Particle* particle, float signalFraction)
    {

      if (signalFraction > 1.0) {
        B2ERROR("Given Signal Fraction exceeds 1.0!");
      }

      if (signalFraction >= 0 and m_reverse) {
        signalFraction = 1 - signalFraction;
      }

      for (unsigned int i = 0; i < m_variables.size(); ++i) {
        m_input[i] = m_variables[i]->function(particle);
      }

      double result = 0;
      if (signalFraction < 0 and signalFraction > -1.5)
        result = m_reader->EvaluateMVA(m_methodName);
      else if (signalFraction < -1.5 and signalFraction > -2.5)
        result = m_reader->GetProba(m_methodName, m_classFractions[m_signalClass]);
      else
        result = m_reader->GetProba(m_methodName, signalFraction);

      if (m_reverse) {
        return 1 - result;
      } else {
        return result;
      }

    }


    std::vector<std::string> Expert::getVariablesFromXML(const boost::property_tree::ptree& pt) const
    {
      std::vector<std::string> variables;
      for (const auto & f : pt.get_child("Setup.Variables")) {
        if (f.first.data() != std::string("Variable"))
          continue;
        variables.push_back(f.second.get<std::string>("Name"));
      }
      return variables;
    }

    std::map<int, float> Expert::getClassFractionsFromXML(const boost::property_tree::ptree& pt) const
    {
      std::map<int, float> classFractions;
      for (const auto & f : pt.get_child("Setup.Clusters")) {

        if (f.first.data() != std::string("Cluster"))
          continue;

        int id = f.second.get<float>("ID");
        float fraction = f.second.get<float>("Fraction");
        classFractions[id] = fraction;
      }
      return classFractions;
    }

    std::shared_ptr<TMVA::Reader> Expert::getReaderFromXML(const boost::property_tree::ptree& pt)
    {

      auto variables = getVariablesFromXML(pt);

      for (const auto & f : pt.get_child("Setup.Trainings")) {
        if (f.first.data() != std::string("Training"))
          continue;

        for (const auto & g : f.second.get_child("Methods")) {
          if (g.first.data() != std::string("Method"))
            continue;

          if (m_methodName != g.second.get<std::string>("MethodName"))
            continue;

          if (m_signalClass != g.second.get<int>("SignalID"))
            continue;

          std::string methodType = g.second.get<std::string>("MethodType");
          Method method(m_methodName, methodType, std::string(), variables);

          auto reader = std::make_shared<TMVA::Reader>("!Color:!Silent");
          m_variables = method.getVariables();
          m_input.resize(m_variables.size(), 0);
          for (unsigned int i = 0; i < m_variables.size(); ++i) {
            reader->AddVariable(Variable::makeROOTCompatible(m_variables[i]->name), &m_input[i]);
          }

          std::string weightfile = g.second.get<std::string>("Weightfile");
          if (!reader->BookMVA(m_methodName, weightfile)) {
            B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
          }

          return reader;
        }
      }
      return nullptr;
    }
  }
}
