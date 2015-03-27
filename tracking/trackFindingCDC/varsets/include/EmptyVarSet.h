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

#include "VarSet.h"

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** A sentinal var set that does not contain variables and does no extraction.
     */
    template<>
    class VarSet<void> {

    private:
      /// Number of floating point values represented by this class.
      static const size_t nVars = 0;

    public:
      /// Constructure taking a optional prefix that can be attached to the names if request.
      VarSet(const std::string& /*prefix*/ = "") {;}

      /// Main method that extracts the variable values from the complex object.
      bool extract(const void*)
      {
        return true;
      }

      /** Initialize the variable set before event processing.
       *  Nothing to do  for the empty variable set.
       */
      void initialize() {;}

      /** Terminate the variable set after event processing.
       *  Nothing to do  for the empty variable set.
       */
      void terminate() {;}

      /** Getter for the named tuples storing the values of all the (possibly nested) VarSets
       *  EmptyVarSet returns an empty vector.
       */
      std::vector<Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables()
      {
        return std::vector<Belle2::TrackFindingCDC::NamedFloatTuple*>();
      }

      /** Const getter for the named tuples storing the values of all the (possibly nested)
       *  variable sets. EmptyVarSet returns an empty vector.
       */
      std::vector<const Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables() const
      {
        return std::vector<const Belle2::TrackFindingCDC::NamedFloatTuple*>();
      }

    }; //end class

    /// Short hand for the empty var set.
    typedef VarSet<void> EmptyVarSet;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
