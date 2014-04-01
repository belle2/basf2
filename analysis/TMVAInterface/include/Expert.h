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

#include <vector>
#include <string>

/**
 * Forward declaration
 */
namespace TMVA {
  class Reader;
}

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
       * @param identifier which used to identify the outputted training files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml
       * @param workingDirectory where the config file and weight file directory is stored
       * @param methodName name of the method which is used by this expert
       * @param signalCluster Number of the cluster which is considered as signal
       */
      Expert(std::string identifier, std::string workingDirectory, std::string methodName, int signalCluster);

      /**
       * Disallow copy
       */
      Expert(const Expert&) = delete;

      /**
       * Disallow assign
       */
      Expert& operator=(const Expert&) = delete;

      /**
       * Destructor, closes outputFile, deletes TMVA::Reader
       */
      ~Expert();

      /**
       * Analyse a Particle with the given method and calculates signal probability
       * @param particle the particle which should be analysed
       * @return signal probability of the particle
       */
      float analyse(const Particle*);

    private:
      std::vector<TMVA::Reader*> m_readers; /**< Vector of TMVA::Reader, which steers the booked TMVA method, foreach cluster */
      std::vector<float> m_input; /**< Pointers to the input variables */
      int m_signalCluster; /**< Number of the cluster which is considered as signal */
      std::vector<Method*> m_methods; /**< methods used by the expert to classify */
      std::map<int, float> m_clusters; /**< map of cluster id and cluster fraction */
      std::vector<bool> m_reverse; /**< true if signalCluster was considered background in the training */
      std::vector<int> m_against; /**< cluster ID against signalCluster was trained */
    };
  }
}

