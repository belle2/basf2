/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost,                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  class DatabaseRepresentationOfWeightfile;
  namespace MVA {
    class Expert;
    class SingleDataset;
    class Weightfile;
  }

  namespace TrackFindingCDC {

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

      /// Initialise the mva method
      void initialize();

      /// Update the mva method to the new run
      void beginRun();

      /// Resolves the source of the weight file and unpacks it.
      std::unique_ptr<MVA::Weightfile> getWeightFile();

      /// Evaluate the MVA method and return the MVAOutput
      double predict();

    private:
      /// References to the named values from the source variable set.
      std::vector<Named<Float_t*> > m_namedVariables;

      /// Database pointer to the Database representation of the weightfile
      std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile> > m_weightfileRepresentation;

      /// Pointer to the current MVA Expert
      std::unique_ptr<MVA::Expert> m_expert;

      /// Pointer to the current dataset
      std::unique_ptr<MVA::Dataset> m_dataset;

      /// DB identifier of the expert or file name
      std::string m_identifier;
    };

  }
}
