/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerObject.h>

#include <memory>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Variable Manager for the software trigger cuts.
     * This class fulfills all the requirement of a variable manager, as stated in the GeneralCut class
     * and can (and should) therefore be used together with it, which is done in the SoftwareTriggerCut.
     *
     * The idea behind it is as follows: The SoftwareTriggerVariableManager itself does not know
     * on how to calculate your needed variables, but you as the user of the SoftwareTriggerCut
     * has to know. This gives you the opportunity to calculate them
     * differently before or after the full reconstruction (because you may not have all information
     * at hand at all time) or use temporary shared calculation objects when compiling the numbers.
     * Whenever a SoftwareTriggerCut has a variable it it and the check function asks the variable
     * manager for its value, the variable manager will collect this value from the
     * SoftwareTriggerObject (a map from std::string to double) with the given variable name. So you as
     * the user has to make sure that the needed variables can be found in the SoftwareTriggerObject,
     * that you hand in to the checkPreScaled function of the SoftwareTriggerCut.
     */
    class SoftwareTriggerVariableManager {
    private:
      /**
       * Class which represents an abstract variable in the SoftwareTriggerVariableManager.
       * It fulfills all requirements for a "Variable", that are given by the GeneralCut class,
       * namely a name member and a function to calculate.
       * In the SoftwareTrigger case, this function is really simple: Just take the needed value
       * (with the name given by the variable's name) out of the already precompiled map
       * of values given as a SoftwareTriggerObject.
       *
       * In normal use cases, you do not have to create a variable on your own and you should
       * not have contact with them (this is why they are a private class of the VariableManager).
       */
      class SoftwareTriggerVariable {
      public:
        /**
         * Function wich is called by the SoftwareTriggerCut whenever the value of this variable is needed.
         * As the values are all already compiled, it just takes the corresponding number
         * from the map of values given as the SoftwareTriggerObject.
         */
        double function(const SoftwareTriggerObject* mapOfValues) const
        {
          return mapOfValues->at(name);
        }

        /// Name of this particular variable.
        std::string name = "";

      private:
        /// Private constructor. Should only be called by the SoftwareTriggerVariableManager.
        explicit SoftwareTriggerVariable(const std::string& theName) : name(theName) { }

        /// Make the object move constructable
        SoftwareTriggerVariable(SoftwareTriggerVariable&&) = default;

        /// Do not copy a variable.
        SoftwareTriggerVariable(const SoftwareTriggerVariable& rhs) = delete;

        /// Do not copy a variable.
        SoftwareTriggerVariable& operator=(const SoftwareTriggerVariable& rhs) = delete;

        /// Let the SoftwareTriggerVariableManager create variables.
        friend class SoftwareTriggerVariableManager;
      };

    public:
      /// As an object handed in for every cut to be checked, use a map of variable name -> precompiled value.
      typedef SoftwareTriggerObject Object;
      /**
       * Use a very slim object for the variables: only draw out the corresponding value from the
       * precompiled map of values.
       */
      typedef SoftwareTriggerVariable Var;

      /**
       * Make this variable manager a singleton and get the only single instance of the manager.
       * You can still use it multiple times in different modules, as it depends on the map of values
       * you hand in when checking the cut if the variable is defined or not.
       */
      static SoftwareTriggerVariableManager& Instance();

      /**
       * Get the variable with the given name. If it is not already present in the internal map
       * of variables, create a new one.
       * Please note, that this function will *never* mark a variable as undefined, so there
       * is no registration process as in the analysis variable manager. If a variable is defined
       * or not depends only on the SoftwareTriggerObject that is used when checking the cut
       * (and if there is an entry with the same name as the variable in this object).
       */
      SoftwareTriggerVariable* getVariable(const std::string& variableName);

    private:
      /// Make the constructor private: only use this class as a singleton.
      SoftwareTriggerVariableManager() { }

      /// Delete the copy constructor: only use this class as a singleton.
      SoftwareTriggerVariableManager(const SoftwareTriggerVariableManager&) = delete;

      /// Delete the assignment operator: only use this class as a singleton.
      SoftwareTriggerVariableManager& operator=(SoftwareTriggerVariableManager& rhs) = delete;

      /// Internal storage of the variables with their name.
      std::map<std::string, std::shared_ptr<SoftwareTriggerVariable>> m_variableNameToVariable;
    };
  }
}
