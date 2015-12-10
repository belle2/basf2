/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class AVarSet>
    class FilterOnVarSet: public Filter<typename AVarSet::Object> {

    public:
      /// Type of the object to be analysed.
      typedef typename AVarSet::Object Object;

    private:
      /// Type of the super class
      typedef Filter<Object> Super;

    public:
      /// Constructor of the filter.
      FilterOnVarSet() :
        Super(),
        m_varSet()
      {}

      /// Initialize the filter before event processing.
      virtual void initialize() override
      {
        Super::initialize();
        m_varSet.initialize();
      }

      /// Initialize the filter after event processing.
      virtual void terminate() override
      {
        m_varSet.terminate();
        Super::terminate();
      }

      /// Checks if any variables need Monte Carlo information.
      virtual bool needsTruthInformation() override
      {
        const std::vector<NamedFloatTuple*>& allVariables = m_varSet.getAllVariables();
        for (NamedFloatTuple* variables : allVariables) {
          size_t nVars = variables->size();
          for (size_t iVar = 0; iVar < nVars; ++iVar) {
            std::string name = variables->getNameWithPrefix(iVar);
            // If the name contains the word truth it is considered to have Monte carlo information.
            if (name.find("truth") != std::string::npos) {
              return true;
            }
          }
        }
        return false;
      }

    public:
      /// Function extracting the variables of the object into the variable set.
      virtual CellWeight operator()(const Object& obj) override
      {
        bool extracted = m_varSet.extract(&obj);
        return extracted ? 1 : NOT_A_CELL;
      }

    protected:
      /// Getter for the set of variables
      AVarSet& getVarSet()
      { return m_varSet; }

    private:
      /// VarSet to generate the variables from the object
      AVarSet m_varSet;
    };
  }
}
