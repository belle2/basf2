/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARBOXMODULE_H_
#define GEARBOXMODULE_H_

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
   * "sqlite:data/belle2.sqlite"
   *
   * will read all parameters from the sqlite database belle2.sqlite in the
   * data directory while
   *
   * "string:<Detector>..</Detector>"
   *
   * can be used to feed a custom xml document to the gearbox.
   *
   * Currently defined backends are file, string and sqlite:
   *
   * - file: will use normal file access to read the xml files.
   *   Empty path (just 'file:') will use data/
   * - string: will treat everything after the : as the data, no xincludes are
   *   supported with this backend
   * - sqlite: will look for the xml files in the sqlite database specified
   *   after the colon
   *
   * Parameters are always read from the first available backend.  So if you
   * want to override only a subset of Parameters you can prepend the list of
   * backends with one backend which only contains the information you want to
   * override.
   *
   * >>> gearbox.set
   */
  class GearboxModule : public Module {

  public:

    /* The constructor of the module.
     * Sets the description and the parameters of the module.
     */
    GearboxModule();

    /** Define backends */
    void initialize();

    /** Load the (possibly rundependent) parameters from the chosen backends */
    void beginRun();

  private:
    std::vector<std::string> m_backends;   /**< The backend specifier. */
    std::string m_fileName;  /**< The toplevel filename for the parameters */

    /** common prefix for all value overrides */
    std::string m_overridePrefix;
    /** overrides to override the value and unit of a parameter */
    std::vector<std::tuple<std::string, std::string, std::string>> m_unitOverrides;
    /** overrides to override the value and unit of many parameters */
    std::vector<std::tuple<std::string, std::string, std::string>> m_multipleOverrides;

    bool m_useXMLDatabase; /**< Enable the XML database. */
  };
}

#endif /* GEARBOXMODULE_H_ */
