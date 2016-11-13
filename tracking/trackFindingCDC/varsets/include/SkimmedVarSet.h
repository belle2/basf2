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

#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Variable set that filters on a specific condition
    template<class ABaseVarSet>
    class SkimmedVarSet : public ABaseVarSet {

    private:
      /// Type of the base class
      using Super = ABaseVarSet;

    public:
      /// Object type from which the variables shall be extracted
      using Object = typename ABaseVarSet::Object;

    public:
      /// Main method that extracts the variable values from the complex object.
      bool extract(const Object* obj) override
      {
        bool extracted = Super::extract(obj);
        return extracted and accept(obj);
      }

      /**
       *  Method to be specialised in a derived class after to accept or reject the extracted values.
       *
       *  The method may use variables of the object as well as already extracted values.
       */
      virtual bool accept(const Object* object __attribute__((unused)))
      {
        return true;
      }

    };
  }
}
