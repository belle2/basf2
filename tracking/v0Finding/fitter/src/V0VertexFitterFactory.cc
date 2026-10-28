/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/v0Finding/fitter/V0VertexFitterFactory.h>

#include <tracking/v0Finding/fitter/KFitV0VertexFitter.h>
#include <tracking/v0Finding/fitter/RaveV0VertexFitter.h>

#include <framework/logging/Logger.h>

#include <functional>
#include <map>

using namespace Belle2;

namespace {

  /// Type of the functions creating the fitters.
  typedef std::function<std::unique_ptr<V0VertexFitter>()> CreatorFunction;

  /// Registry of the available fitters: add one line here to make a new fitter selectable.
  const std::map<std::string, CreatorFunction>& getRegistry()
  {
    static const std::map<std::string, CreatorFunction> registry = {
      {"KFit", []() -> std::unique_ptr<V0VertexFitter> {return std::make_unique<KFitV0VertexFitter>();}},
      {"Rave", []() -> std::unique_ptr<V0VertexFitter> {return std::make_unique<RaveV0VertexFitter>();}},
    };
    return registry;
  }

}

std::unique_ptr<V0VertexFitter> V0VertexFitterFactory::create(const std::string& name)
{
  const auto& registry = getRegistry();
  const auto iterator = registry.find(name);
  if (iterator == registry.end()) {
    B2FATAL("Unknown V0 vertex fitter requested."
            << LogVar("requested fitter", name)
            << LogVar("available fitters", getNamesAsString()));
  }
  return iterator->second();
}

std::vector<std::string> V0VertexFitterFactory::getNames()
{
  std::vector<std::string> names;
  for (const auto& entry : getRegistry()) names.push_back(entry.first);
  return names;
}

std::string V0VertexFitterFactory::getNamesAsString()
{
  std::string names;
  for (const auto& name : getNames()) {
    if (not names.empty()) names += ", ";
    names += name;
  }
  return names;
}
