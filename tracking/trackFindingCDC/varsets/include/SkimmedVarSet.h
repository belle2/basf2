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
    /** Generic class that generates the same variables from a each of a pair of instances.
     **/
    template<class ABaseVarSet>
    class SkimmedVarSet : public ABaseVarSet {

    private:
      /// Type of the base class
      typedef ABaseVarSet Super;

    public:
      /// Object type from which the variables shall be extracted
      typedef typename ABaseVarSet::Object Object;

    public:
      /// Constructure taking a optional prefix that can be attached to the names if request.
      explicit SkimmedVarSet(const std::string& prefix = "") :
        Super(prefix)
      {}

      /// Main method that extracts the variable values from the complex object.
      virtual bool extract(const Object* obj)
      {
        bool extracted = Super::extract(obj);
        return extracted and accept(obj);
      }

      /** Method to be specialised in a derived class after to accept or reject the extracted values.
       *
       *  The method may use variables of the object as well as already extracted values.
       */
      virtual bool accept(const Object*)
      {
        return true;
      }

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
