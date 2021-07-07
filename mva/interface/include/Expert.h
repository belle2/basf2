/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
       * Apply this expert onto a dataset.
       * @param test_data dataset
       */
      virtual std::vector<float> apply(Dataset& test_data) const = 0;

      /**
        * Apply this m_expert onto a dataset. Multiclass mode.
        * Not pure virtual, since not all derived classes to re-implement this.
        * @param test_data dataset.
      * @return vector of size N=test_data.getNumberOfEvents() with N=m_classes.size() scores for each event in the dataset.
        */
      virtual std::vector<std::vector<float>> applyMulticlass(Dataset& test_data) const
      {

        (void) test_data;

        return std::vector<std::vector<float>>();
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
