/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/v0Finding/fitter/V0VertexFitter.h>

#include <memory>
#include <string>
#include <vector>

namespace Belle2 {

  /** Factory creating the V0VertexFitter selected at run time.
   *
   * To make a new fitter selectable: implement the V0VertexFitter interface and add one line
   * to the registry in V0VertexFitterFactory.cc, nothing else has to be touched.
   */
  class V0VertexFitterFactory {

  public:

    /** Create the fitter registered under the given name.
     * Aborts with a fatal message if the name is not registered.
     * @param name name of the fitter, as returned by getNames()
     * @return the newly created fitter
     */
    static std::unique_ptr<V0VertexFitter> create(const std::string& name);

    /// Get the names of all the registered fitters.
    static std::vector<std::string> getNames();

    /// Get the names of all the registered fitters as a single string, for messages and parameter descriptions.
    static std::string getNamesAsString();
  };

}
