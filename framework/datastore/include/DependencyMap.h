/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <map>
#include <set>
#include <string>

namespace Belle2 {
  class Module;

  /** Collect information about the dependencies between modules.
   *
   * Filled inside DataStore during register/require/optional, and used by the
   * --visualize-dataflow and --module-io arguments to basf2 to create
   * graphs of data dependencies.
   *
   * @sa DataFlowVisualization
   */
  class DependencyMap {
  public:
    /** Possible types of entries/relations for a module. */
    enum EEntryType {
      c_Input, /**< required input. */
      c_OptionalInput, /**< optional input. */
      c_Output, /**< registered output. */

      c_NEntryTypes /**< size of this enum. */
    };

    /** Stores information on inputs/outputs of a module, as obtained by requireInput()/optionalInput()/registerEntry(); */
    struct ModuleInfo {
      std::set<std::string> entries[c_NEntryTypes]; /**< objects/arrays. */
      std::set<std::string> relations[c_NEntryTypes]; /**< relations between them. */

      /** Adds given entry/relation. */
      void addEntry(const std::string& name, EEntryType type, bool isRelation);
    };

    /** Return unique ID for given module. */
    static std::string getModuleID(const Module& mod);

    /** Is the object/array/relation with given name used as specified input/output type (in any module)?
     *
     * E.g. is('EventMetaData', c_Output) asks if EventMetaData was registered,
     *      is('EventMetaData', c_Input) asks if any module has it as required input.
     */
    bool isUsedAs(const std::string& branchName, EEntryType type) const;

    /** Set the current module (for getCurrentModuleInfo()) */
    void setModule(const Module& mod) { m_currentModule = getModuleID(mod); }

    /** Get info for current module. */
    ModuleInfo& getCurrentModuleInfo() { return m_moduleInfo[m_currentModule]; }

    /** return information on inputs/outputs of each module, as obtained by requireInput()/optionalInput()/registerEntry(); */
    const std::map<std::string, ModuleInfo>& getModuleInfoMap() const { return m_moduleInfo; }

    /** Reset all collected data. */
    void clear() { m_moduleInfo.clear(); m_currentModule.clear(); }

  private:
    /** Stores information on inputs/outputs of each module, as obtained by requireInput()/optionalInput()/registerEntry(); */
    std::map<std::string, ModuleInfo> m_moduleInfo;

    /** Stores the current module, used to fill m_moduleInfo. */
    std::string m_currentModule;
  };
}
