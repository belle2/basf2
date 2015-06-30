/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TMVA/Types.h>

#include <string>

namespace Belle2 {
  namespace TMVAInterface {

    /**
     * Represents a TMVA Method, containing name, type, config of the method.
     * If type is "plugin" the shared library of the plugin associated to the given name (and a possible suffix) is loaded automatically.
     * If the type corresponds to a builtin TMVA method, the name can be arbitrary.
     */
    class Method {

    public:

      /**
       * Constructs new Method from its defining elements (name, type, config, variables)
       * @param name name of the Method: Arbitrary for builtin methods, Equal to the method name for plugin methods
       * @param type string representation of type, see TMVA::Types::EMVA for possible names (discarding the leading k).
       * @param config string with the config which is given to the TMVA method when its booked. See TMVA UserGuide for possible options
       */
      Method(const std::string& name, const std::string& type, const std::string& config);

      /**
       * Getter for name of the method
       * @return name
       */
      std::string getName() const { return m_name; }

      /**
       * Getter for type as enum of the method
       * @return type
       */
      TMVA::Types::EMVA getTypeAsEnum() const { return m_type_enum; }

      /**
       * Getter for type as string of the method
       * @return type
       */
      std::string getTypeAsString() const { return m_type; }

      /**
       * Getter for config of the method
       * @return config
       */
      std::string getConfig() const { return m_config; }

    private:
      std::string m_name; /**< name of the method */
      std::string m_type; /**< type of the method */
      std::string m_config; /**< config string given to the method */
      TMVA::Types::EMVA m_type_enum; /**< type of the method as corresponding TMVA enum */
    };

  }
}
