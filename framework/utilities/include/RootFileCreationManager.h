/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013-2018 Belle II Collaboration                          *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Pulvermacher                                   *
*               Simon Wehle                                              *
*               Martin Ritter                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <TFile.h>
#include <string>
#include <map>
#include <memory>

namespace Belle2 {
  /** This single instance class takes track of all open ROOT files open in
   * "create" mode, usually meant for all the ntuple output modules.
   *
   * Multiple different users can use the same file if they provide the same
   * filename. Currently there is no path name manipulation so "test.root" and
   * "./test.root" are not the same filename. Please keep the shared_ptr
   * returned by this class as long as you need the file to stay open.
   * Temporary objects might not work.
   *
   * The files are created and managed by this class. They get destroyed when
   * the last user is freeing its copy. Users of this module should free their
   * files in the `terminate()` function by just calling reset() on the
   * shared_ptr.
   */
  class RootFileCreationManager {

  public:

    /** Interface for the FileManager */
    static RootFileCreationManager& getInstance();
    /**
     * Get a file with a specific name, if is does not exist it will be created
     *
     * If the file cannot be created and opened an error is emitted and you get an empty
     * shared_ptr. If you don't want an error to be emitted you can provide
     * ignoreErrors=true as second argument.
     */
    std::shared_ptr<TFile> getFile(std::string, bool ignoreErrors = false);

  private:
    /** Constructor is private. */
    RootFileCreationManager() {}
    /** We so not want copy construction */
    RootFileCreationManager(RootFileCreationManager const&) = delete;
    /** We do not need this interface */
    void operator=(RootFileCreationManager const&) = delete;
    /** store for the open files */
    std::map<std::string, std::weak_ptr<TFile>> m_files;
  };
}
