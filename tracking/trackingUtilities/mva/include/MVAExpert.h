/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/utilities/Named.h>
#include <RtypesCore.h>

#include <vector>
#include <memory>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {

    /// Class to interact with the MVA package
    class MVAExpert {

    public:
      /**
       *  Construct the Expert with the specified weight folder and
       *  the name of the training that was used in the teacher run.
       *  @param identifier       A database identifier or local file name.
       *  @param namedVariables   The names and pointers to the variables to be fed to the mva method
       */
      MVAExpert(const std::string& identifier, std::vector<Named<Float_t*>> namedVariables);

      /// Destructor must be defined in cpp because of PImpl pointer
      ~MVAExpert();

      /// Initialise the mva method
      void initialize();

      /// Update the mva method to the new run
      void beginRun();

      /// Evaluate the MVA method and return the MVAOutput
      double predict();

      /// Evaluate the MVA method and return the MVAOutput for multiple inputs at the same time
      std::vector<float> predict(float* /* test_data */, int /* nFeature */, int nRows);

      /// Get selected variable names
      std::vector<std::string> getVariableNames();
    private:
      /// Forward declaration of implementation.
      class Impl;

      /// Pointer to implementation hiding the details.
      std::unique_ptr<Impl> m_impl;
    };
  }
}
