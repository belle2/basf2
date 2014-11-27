/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

#include <TMVA/Types.h>

#include <string>
#include <vector>

namespace Belle2 {
  namespace TMVAInterface {

    /**
     * Represents a TMVA Method, containing name, type, config and variables of the method.
     * If type is "plugin" the shared library of the plugin associated to the given name is loaded automatically.
     * If the type corresponds to a builtin TMVA method, the name can be arbirary.
     */
    class Method {

    public:

      /**
       * Constructs new Method from its defining elements (name, type, config, variables)
       * @param name name of the Method: Arbitrary for builtin methods, Equal to the method name for plugin methods
       * @param type string representation of type, see TMVA::Types::EMVA for possible names (dicarding the leading k).
       * @param config string with the config which is given to the TMVA method when its booked. See TMVA UserGuide for possible options
       * @param variables vector of variable names, which have to be registered in the VariableManager
       */
      Method(std::string name, std::string type, std::string config, std::vector<std::string> variables);

      /**
       * Getter for name of the method
       * @return name
       */
      std::string getName() { return m_name; }

      /**
       * Getter for type of the method
       * @return type
       */
      TMVA::Types::EMVA getType() { return m_type; }

      /**
       * Getter for type as string of the method
       * @return type
       */
      std::string getTypeAsString() { return m_type_as_string; }

      /**
       * Getter for config of the method
       * @return config
       */
      std::string getConfig() { return m_config; }

      /**
       * Getter for variabels of the method
       * @return variables
       */
      const std::vector<const Variable::Manager::Var*>& getVariables() { return m_variables; }

    private:
      std::string m_name; /**< name of the method */
      std::string m_config; /**< config string given to the method */
      std::vector<const Variable::Manager::Var*> m_variables; /**< Pointers to the input variables loaded from VariableManager */
      TMVA::Types::EMVA m_type; /**< type of the method */
      std::string m_type_as_string; /**< type of the method as string */
    };

  }
}
