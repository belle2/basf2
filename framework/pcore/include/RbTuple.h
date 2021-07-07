/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>

class TFile;

namespace Belle2 {
  class Module;

  /** Class to manage histograms defined in registered modules */
  /*
     Modules with histogram definitions are supposed to be derived from
     HistoModule class and histograms are defined in HistoModule::defineHisto() function.
     It is supposed to be hooked by register_module(*this) function.
  */
  class RbTupleManager {
  public:

    // Constructor and destructor are private for singleton access
    RbTupleManager();
    ~RbTupleManager();

    /** Constructor to use hadd() */
    RbTupleManager(int nprocess, const char* filename, const char* workdir = ".");

    /** Access to singleton */
    static RbTupleManager& Instance();

    /** Global initialization */
    void init(int nprocess, const char* filename, const char* workdir = ".");

    /** Functions called by analysis modules in mother process */
    //  void define ( void (*func)( void ) );
    void register_module(Module*);


    /** Function called by HistoManager module for the first event */
    int begin(int pid);

    /** Function called by HistoManager module at the end */
    int terminate();

    // Functions to collect histograms from event processing on the fly
    //  int collect ();

    /** Function to dump histograms/tuples to the file */
    int dump();

    /** Functions to add up all histogram files */
    int hadd(bool deleteflag = true);

    /** Return the list of modules that have defined histograms */
    const std::vector<Module*>& getHistDefiningModules() const
    {
      return m_histdefs;
    }

  private:
    static RbTupleManager* s_instance; /**< singleton instance. */

    std::vector<Module*> m_histdefs; /**< registered HistoModules. */
    int m_nproc{0}; /**< Number of parallel processes. */
    std::string m_filename; /**< Name of histogram output file. */
    std::string m_workdir;  /**< Name of working directory */
    TFile* m_root{nullptr}; /**< Histogram output file. */
  };

} // Belle2 namespace
