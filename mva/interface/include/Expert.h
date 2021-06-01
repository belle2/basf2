/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_EXPERT_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_EXPERT_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Dataset.h>
#include <mva/interface/Weightfile.h>

#include <vector>

namespace Belle2 {
  namespace MVA {

    /**
     * Abstract base class of all Expert
     * Each MVA library has its own implementation of this class,
     * so all libraries can be accessed via this common interface
     */
    class Expert {
    public:
      /**
       * Default constructor
       */
      Expert() = default;

      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the expert
       */
      virtual void load(Weightfile& weightfile) = 0;

      /**
       * Apply this expert onto a dataset
       * @param test_data dataset
       */
      virtual std::vector<float> apply(Dataset& test_data) const = 0;

      /**
       * Apply this expert onto a dataset.
       * Multi-class mode signature.
       * Not pure virtual, since not all derived classes need to re-implement this.
       * @param test_data dataset
       * @param classID class identifier.
       */
      virtual std::vector<float> apply(Dataset& test_data, const unsigned int classID) const
      {

        (void) test_data;
        (void) classID;

        return std::vector<float>();
      };

      /**
        * Apply this expert onto a dataset of length 1 and return the score for all classes.
        * Multi-class mode signature.
        * Not pure virtual, since not all derived classes to re-implement this.
        * @param test_data dataset
        */
      virtual std::vector<float> applySingle(Dataset& test_data) const
      {

        (void) test_data;

        return std::vector<float>();
      };

      /**
       * Virtual destructor
       */
      virtual ~Expert() = default;
    protected:
      GeneralOptions m_general_options; /**< General options loaded from the weightfile */
    };

  }
}
#endif
