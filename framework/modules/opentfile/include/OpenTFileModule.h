/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef OPENTFILE_H
#define OPENTFILE_H

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {
  /** Module to open a TFile which is used by several modules.
   *
   *  Does so by opening the requested TFile and stores a reference to it on the DataStore
   *  as a transient StoreObject.
   *
   *  @sa EDurability
  */
  class OpenTFileModule : public Module {
  public:

    /** Constructor. */
    OpenTFileModule();

    /** Destructor. */
    virtual ~OpenTFileModule();

    /** Initialize the Module */
    virtual void initialize();

    /** Is called at the end of your Module */
    virtual void terminate();

  private:
    //first the steerable variables:

    /// Parameter: File name to open.
    std::string m_param_fileName;

    /// Parameter: Name of the StoreObjPtr on the DataStore. Default is "".
    std::string m_param_storeName;

    /// Parameter: Option string to forward to the TFile constructor. Default is "RECREATE".
    std::string m_param_option;

  };
} // end namespace Belle2

#endif
