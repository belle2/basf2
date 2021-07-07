/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>
#include <vector>


namespace Belle2 {
  /**
   * The Gearbox module.
   *
   * This module loads the parameters from the specified backends.
   *
   * A backend is consists of a prefix and accessor, for example
   *
   * "file:data/"
   *
   * will look for xml files in the data directory (local or central release) and
   *
   * "string:<Detector>..</Detector>"
   *
   * can be used to feed a custom xml document to the gearbox.
   *
   * Currently defined backends are file and string
   *
   * - file: will use normal file access to read the xml files.
   *   Empty path (just 'file:') will use data/
   * - string: will treat everything after the : as the data, no xincludes are
   *   supported with this backend
   *
   * Parameters are always read from the first available backend.  So if you
   * want to override only a subset of Parameters you can prepend the list of
   * backends with one backend which only contains the information you want to
   * override.
   */
  class GearboxModule : public Module {

  public:

    /* The constructor of the module.
     * Sets the description and the parameters of the module.
     */
    GearboxModule();

    /** Define backends */
    void initialize() override;

    /** Load the (possibly rundependent) parameters from the chosen backends */
    void beginRun() override;

  private:
    std::vector<std::string> m_backends;   /**< The backend specifier. */
    std::string m_fileName;  /**< The toplevel filename for the parameters */

    /** common prefix for all value overrides */
    std::string m_overridePrefix;
    /** overrides to override the value and unit of a parameter */
    std::vector<std::tuple<std::string, std::string, std::string>> m_unitOverrides;
    /** overrides to override the value and unit of many parameters */
    std::vector<std::tuple<std::string, std::string, std::string>> m_multipleOverrides;
  };
}
