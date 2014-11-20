/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <tuple>
#include <iostream>
using namespace std;

namespace Belle2 {

  /** The most CPU efficient Observer for the VXDTF filter tools (even if useless).
   *
   * The VoidObserver is ment to be the simplest (and most useless) Observer
   * for the VXDTF filter tools.
   * It is notified of relevant informations by the
   * observed object via the the notify method. This particular
   * observer does not take any action to record the messages, hence
   * the compiler will simply skip the call (in optimized compilation mode).
   * More evolved and useful observers can derive from this one (even if not
   * strictly necessary).
   */

  class VoidObserver {
  public:

    /** _static_ method used by the observed object to notify the observer
     */
    template <  typename ... types >
    static void notify(types ...) {};

    /** An empty constructor for an empty class */
    VoidObserver() {};

    /** An empty destructor for an empty class */
    ~VoidObserver() {};
  };
}
