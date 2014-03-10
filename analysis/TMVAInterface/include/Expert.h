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
       * @param weightfile path to weightfile which is used to load the correct TMVA method
       */
      Expert(std::string weightfile);

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
      TMVA::Reader* m_reader; /**< TMVA::Reader steers the booked TMVA method */
      Method* m_method; /**< Method parameters */
      std::vector<float> m_input; /**< Pointers to the input variables */
    };
  }
}

