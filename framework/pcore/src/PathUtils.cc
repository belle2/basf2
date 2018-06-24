/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/PathUtils.h>
#include <framework/core/ModuleManager.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/logging/LogMethod.h>
#include <set>

using namespace Belle2;

std::tuple<PathPtr, PathPtr, PathPtr> PathUtils::splitPath(const PathPtr& path)
{
  //modules that can be parallelized, but should not go into a parallel section by themselves
  std::set<std::string> uselessParallelModules({"HistoManager", "Gearbox", "Geometry"});

  PathPtr inputPath(new Path);
  PathPtr mainPath(new Path);
  PathPtr outputPath(new Path);

  int stage = 0; //0: in, 1: event/main, 2: out
  for (const ModulePtr& module : path->getModules()) {
    bool hasParallelFlag = module->hasProperties(Module::c_ParallelProcessingCertified);
    //entire conditional path must also be compatible
    if (hasParallelFlag and module->hasCondition()) {
      for (const auto& conditionPath : module->getAllConditionPaths()) {
        if (!ModuleManager::allModulesHaveFlag(conditionPath->getModules(), Module::c_ParallelProcessingCertified)) {
          hasParallelFlag = false;
        }
      }
    }
    //if modules have parallal flag -> stage = 1 , event/main
    if ((stage == 0 and hasParallelFlag) or (stage == 1 and !hasParallelFlag)) {
      stage++;

      if (stage == 2) {
        bool path_is_useful = false;
        for (auto parallelModule : mainPath->getModules()) {
          if (uselessParallelModules.count(parallelModule->getType()) == 0) {
            path_is_useful = true;
            break;
          }
        }
        if (not path_is_useful) {
          //merge mainPath back into input path
          inputPath->addPath(mainPath);
          mainPath.reset(new Path);
          //and search for further parallel sections
          stage = 0;
        }
      }
    }

    if (stage == 0) {
      inputPath->addModule(module);
    } else if (stage == 1) {
      mainPath->addModule(module);
    } else if (stage == 2) {
      outputPath->addModule(module);
    }
  }

  bool createAllPaths = false; //usually we might not need e.g. an output path
  for (const ModulePtr& module : path->getModules()) {
    if (module->hasProperties(Module::c_TerminateInAllProcesses)) {
      createAllPaths = true; //ensure there are all kinds of processes
    }
  }

  // if main path is empty, createAllPaths doesn't really matter, since we'll fall back to single-core processing
  if (mainPath->isEmpty() and not createAllPaths) {
    mainPath.reset();
  }
  if (inputPath->isEmpty() and not createAllPaths) {
    inputPath.reset();
  }
  if (outputPath->isEmpty() and not createAllPaths) {
    outputPath.reset();
  }

  return {inputPath, mainPath, outputPath};
}

ModulePtr PathUtils::getHistogramManager(PathPtr& inputPath, PathPtr& mainPath, PathPtr& outputPath)
{
  ModulePtr histoManagerModule;
  for (const ModulePtr& module : inputPath->getModules()) {
    if (module->hasProperties(Module::c_HistogramManager)) {
      // Initialize histogram manager if found in the path
      histoManagerModule = module;

      //add histoman to other paths
      mainPath->addModule(histoManagerModule);
      outputPath->addModule(histoManagerModule);
    }
  }

  return histoManagerModule;
}
ModulePtrList PathUtils::preparePaths(PathPtr& inputPath, PathPtr& mainPath, PathPtr& outputPath,
                                      const std::string& socketAddress)
{
  B2ASSERT("The main part is empty. This is a bug in the framework.",
           mainPath and not mainPath->isEmpty());

  ModuleManager& moduleManager = ModuleManager::Instance();

  const auto inputSocketAddress(ZMQHelper::getSocketAddress(socketAddress, ZMQAddressType::c_input));
  const auto outputSocketAddress(ZMQHelper::getSocketAddress(socketAddress, ZMQAddressType::c_output));
  const auto pubSocketAddress(ZMQHelper::getSocketAddress(socketAddress, ZMQAddressType::c_pub));
  const auto subSocketAddress(ZMQHelper::getSocketAddress(socketAddress, ZMQAddressType::c_sub));

  if (inputPath) {
    // Add TXInput after input path
    ModulePtr zmqTxInputModule = moduleManager.registerModule("ZMQTxInput");
    zmqTxInputModule->getParam<std::string>("socketName").setValue(inputSocketAddress);
    zmqTxInputModule->getParam<std::string>("xpubProxySocketName").setValue(pubSocketAddress);
    zmqTxInputModule->getParam<std::string>("xsubProxySocketName").setValue(subSocketAddress);
    appendModule(inputPath, zmqTxInputModule);

    // Add RXWorker before main path
    ModulePtr zmqRxWorkerModule = moduleManager.registerModule("ZMQRxWorker");
    zmqRxWorkerModule->getParam<std::string>("socketName").setValue(inputSocketAddress);
    zmqRxWorkerModule->getParam<std::string>("xpubProxySocketName").setValue(pubSocketAddress);
    zmqRxWorkerModule->getParam<std::string>("xsubProxySocketName").setValue(subSocketAddress);
    prependModule(mainPath, zmqRxWorkerModule);
  }

  if (outputPath) {
    // Add TXWorker after main path
    ModulePtr zmqTxWorkerModule = moduleManager.registerModule("ZMQTxWorker");
    zmqTxWorkerModule->getParam<std::string>("socketName").setValue(outputSocketAddress);
    zmqTxWorkerModule->getParam<std::string>("xpubProxySocketName").setValue(pubSocketAddress);
    zmqTxWorkerModule->getParam<std::string>("xsubProxySocketName").setValue(pubSocketAddress);
    appendModule(mainPath, zmqTxWorkerModule);

    // Add RXOutput before output path
    ModulePtr zmqRxOutputModule = moduleManager.registerModule("ZMQRxOutput");
    zmqRxOutputModule->getParam<std::string>("socketName").setValue(outputSocketAddress);
    zmqRxOutputModule->getParam<std::string>("xpubProxySocketName").setValue(pubSocketAddress);
    zmqRxOutputModule->getParam<std::string>("xsubProxySocketName").setValue(subSocketAddress);
    prependModule(outputPath, zmqRxOutputModule);
  }

  if (inputPath) {
    B2INFO("Input Path " << inputPath->getPathString());
  }
  if (mainPath) {
    B2INFO("Main Path " << mainPath->getPathString());
  }
  if (outputPath) {
    B2INFO("Output Path " << outputPath->getPathString());
  }

  Path mergedPath;
  if (inputPath) {
    mergedPath.addPath(inputPath);
  }
  mergedPath.addPath(mainPath);
  if (outputPath) {
    mergedPath.addPath(outputPath);
  }
  return mergedPath.buildModulePathList();
}

ModulePtrList PathUtils::getTerminateGloballyModules(const ModulePtrList& modules)
{
  ModulePtrList tmpModuleList;
  for (const ModulePtr& m : modules) {
    if (m->hasProperties(Module::c_TerminateInAllProcesses))
      tmpModuleList.push_back(m);
  }
  return tmpModuleList;
}

void PathUtils::prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules)
{
  for (const ModulePtr& m : prependModules) {
    if (std::find(modules->begin(), modules->end(), m) == modules->end()) { //not present
      modules->push_front(m);
    }
  }
}

void PathUtils::appendModule(PathPtr& path, ModulePtr module)
{
  path->addModule(module);
}

void PathUtils::prependModule(PathPtr& path, ModulePtr module)
{
  PathPtr newPath(new Path());
  newPath->addModule(module);
  newPath->addPath(path);
  path.swap(newPath);
}
