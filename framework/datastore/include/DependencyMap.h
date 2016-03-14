#pragma once

#include <map>
#include <set>
#include <string>

namespace Belle2 {
  /** Collect information about the dependencies between modules.
   *
   * Filled inside DataStore during register/require/optional.
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

    /** Is the object/array/relation with given name used as specified input/output type (in any module)?
     *
     * E.g. is('EventMetaData', c_Output) asks if EventMetaData was registered,
     *      is('EventMetaData', c_Input) asks if any module has it as required input.
     */
    bool isUsedAs(std::string branchName, EEntryType type) const;

    /** Set the current module (for getCurrentModuleInfo()) */
    void setModule(const std::string& name) { m_currentModule = name; }

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
