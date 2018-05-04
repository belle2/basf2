#pragma once

#include <framework/core/Module.h>
#include <framework/core/Path.h>

namespace Belle2 {


  enum class ProcType {
    c_Input = 'i',
    c_Worker = 'w',
    c_Output = 'o',
    c_Proxy = 'p',
    c_Monitor = 'm',
    c_Init = '0'
  };

  class ProcHelper {
  public:
    ProcHelper() = delete;

    /** Return only modules which have the given Module flag set. */
    static ModulePtrList getModulesWithFlag(const ModulePtrList& modules, Module::EModulePropFlags flag)
    {
      ModulePtrList tmpModuleList;
      for (const ModulePtr& m : modules) {
        if (m->hasProperties(flag))
          tmpModuleList.push_back(m);
      }

      return tmpModuleList;
    }

    /** Return only modules which do not have the given Module flag set. */
    static ModulePtrList getModulesWithoutFlag(const ModulePtrList& modules, Module::EModulePropFlags flag)
    {
      ModulePtrList tmpModuleList;
      for (const ModulePtr& m : modules) {
        if (!m->hasProperties(flag))
          tmpModuleList.push_back(m);
      }
      return tmpModuleList;
    }

    /** Prepend given 'prependModules' to 'modules', if they're not already present. */
    static void prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules)
    {
      for (const ModulePtr& m : prependModules) {
        if (std::find(modules->begin(), modules->end(), m) == modules->end()) { //not present
          modules->push_front(m);
        }
      }
    }


    static void appendModule(PathPtr& path, ModulePtr module)
    {
      path->addModule(module);
    }


    static void prependModule(PathPtr& path, ModulePtr module)
    {
      PathPtr newPath(new Path());
      newPath->addModule(module);
      newPath->addPath(path);
      path.swap(newPath);
    }



  };

}