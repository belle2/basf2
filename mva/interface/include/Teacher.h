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
#ifndef INCLUDE_GUARD_BELLE2_MVA_TEACHER_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_TEACHER_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Dataset.h>
#include <mva/interface/Weightfile.h>

namespace Belle2 {
  namespace MVA {

    /**
     * Abstract base class of all Teachers
     * Each MVA library has its own implementation of this class,
     * so all libraries can be accessed via this common interface
     */
    class Teacher {
    public:
      /**
       * Constructs a new teacher using the GeneralOptions for this training
       * @param general_options defining all shared options
       */
      Teacher(const GeneralOptions& general_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const = 0;

      /**
       * Virtual destructor
       */
      virtual ~Teacher() = default;

    protected:
      GeneralOptions m_general_options; /**< GeneralOptions containing all shared options */
    };

  }
}
#endif
