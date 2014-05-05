/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/TMVAInterface/Expert.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

#include <TMVA/Reader.h>
#include <TMVA/Tools.h>
#include <TString.h>
#include <TSystem.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <fstream>

namespace Belle2 {

  namespace TMVAInterface {

    Expert::Expert(std::string prefix, std::string workingDirectory, std::string methodName, int signalCluster) : m_signalCluster(signalCluster)
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
        B2ERROR("Couldn't open configfile" << configfile)
      }

      // Read out configfile
      try {
        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(configstream, pt);

        // First read out the different clusters
        for (const auto & f : pt.get_child("Setup.Clusters")) {
          if (f.first.data() != std::string("Cluster")) continue;
          int id = f.second.get<int>("ID");
          float fraction = f.second.get<float>("Fraction");
          m_clusters[id] = fraction;
        }

        // Now read out the weightfiles for the trainings which are needed for calculating
        // tje signal probability for the given signal cluster
        for (const auto & f : pt.get_child("Setup.Trainings")) {
          if (f.first.data() != std::string("Training")) continue;
          for (const auto & g : f.second.get_child("Methods")) {
            if (g.first.data() != std::string("Method")) continue;

            // We're only interested in one of the methods! So skip the others
            std::string _methodName = g.second.get<std::string>("MethodName");
            if (_methodName != methodName) continue;

            // Now check if the signalCluster was used as signal or background in this training
            int signalID = g.second.get<int>("SignalID");
            int backgroundID = g.second.get<int>("BackgroundID");
            if (signalID == signalCluster or backgroundID == signalCluster) {

              TMVA::Reader* reader = new TMVA::Reader("!Color:!Silent");

              // Create method frm the weightfile
              std::string weightfile = g.second.get<std::string>("Weightfile");
              std::ifstream istream(weightfile);
              if (not istream.good()) {
                B2ERROR("Couldn't open weightfile" << weightfile)
              }
              Method* method = new Method(istream);

              // Set the variablaes
              const auto& variables = method->getVariables();
              m_input.resize(variables.size(), 0);
              for (unsigned int i = 0; i < variables.size(); ++i) {
                reader->AddVariable(variables[i]->name, &m_input[i]);
              }

              // Book the methoid
              if (!reader->BookMVA(method->getName(), weightfile)) {
                B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
              }

              m_readers.push_back(reader);
              // Against stores the cluster ID against the signalCluster was trained
              m_against.push_back(backgroundID == signalCluster ? signalID : backgroundID);
              // Reverse stores whether the signalCluster was used as background in this training,
              // if so we need to modify the calculation of the signal probability for this training
              m_reverse.push_back(backgroundID == signalCluster);
              m_methods.push_back(method);

            }
          }
        }
      } catch (const std::exception& ex) {
        B2ERROR("There was an error while scanning the stream of the file for the used variables and the used method: " << ex.what())
      }

      // Change back to old directory
      gSystem->ChangeDirectory(oldDirectory.c_str());

    }

    float Expert::analyse(const Particle* particle)
    {
      // Set the input variables, we only do this once, with the variables of the first method
      // because all methods use the same variables, so there's no need to set the input variables
      // for every method seperately
      const auto& variables = m_methods[0]->getVariables();
      for (unsigned int i = 0; i < variables.size(); ++i) {
        m_input[i] = variables[i]->function(particle);
      }

      // Get the results of the different methods.
      // It's assumed that the mva output corresponds to a probability
      std::vector<float> mva_results(m_readers.size());
      for (unsigned int i = 0; i < m_readers.size(); ++i) {
        //mva_results[i] = m_readers[i]->EvaluateMVA(m_methods[i]->getName());
        mva_results[i] = m_readers[i]->GetProba(m_methods[i]->getName(), 0.5);
      }

      // Now calculate the signal probability with the mva outputs
      float result = m_clusters[m_signalCluster];
      for (unsigned int i = 0; i < m_clusters.size() - 1; ++i) {
        if (m_reverse[i])
          result +=  m_clusters[ m_against[i] ] * mva_results[i] / (1.0 - mva_results[i]);
        else
          result +=  m_clusters[ m_against[i] ] * (1.0 - mva_results[i]) / mva_results[i];
      }
      return m_clusters[m_signalCluster] / result;

    }

    Expert::~Expert()
    {
      for (auto & reader : m_readers)
        delete reader;
      for (auto & method : m_methods)
        delete method;
    }

  }
}

