/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <vector>
#include <string>


namespace Belle2 {

  /**
   * HistModule
   * Alternative to HistoModule
   * No need for an extra module in the Module Path.
   * Inherit from this module, implement writeHists by calling Write() on your histograms in this method.
   * SetFilename in your initialize method.
   * Call saveHists() in terminate() of your module.
   *
   * See HistMaker Module for an example.
   */
  class HistModule : public Module {
  public:
    /**
     * @param filename name of the file in which the histograms are saved
     */
    void setFilename(std::string filename);

    /**
     * Call this function in the terminate method of your module
     */
    void saveHists();

    /**
     * Implement this method by calling Write on all your histograms
     */
    virtual void writeHists() = 0;

    /**
     * This static function is calld by the pEventProcessor to merge the histograms of different processes
     */
    static void mergeFiles();

  private:
    static std::vector<std::string> m_filenames; /**< The filenames set by all Module which inherit from this class */
    std::string m_filename; /**< The filename of the file where the histograms of this modules are stored */
  };

}
