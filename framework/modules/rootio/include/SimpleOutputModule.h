/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEOUTPUTMODULE_H
#define SIMPLEOUTPUTMODULE_H

#include <framework/modules/rootio/RootOutputModule.h>

namespace Belle2 {
  /** Write objects from DataStore into a ROOT file.
   *
   *  Please make sure, that you create the same objects in each call of event,
   *  otherwise the module might not write out everything or has dangling pointers.
   *  For more information consult the TWiki basf2 Software Portal.
   *
   *  @sa EDurability
   *  @author <a href="mailto:martin.heck@kit.edu?subject=Output Module">Martin Heck</a>
   *
   *  @deprecated Use RootOutputModule instead.
   */
  class SimpleOutputModule : public RootOutputModule {

  public:

    /** Constructor.
     */
    SimpleOutputModule(): RootOutputModule() { }

    /** Destructor.
     *
     *  Deletion of objects, that were created in the Constructor.
     */
    virtual ~SimpleOutputModule() { }
  };
} // end namespace Belle2

#endif // SIMPLEOUTPUTMODULE_H
