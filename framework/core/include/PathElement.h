/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <memory>
#include <list>
#include <string>

namespace Belle2 {

  class Module;

  /** Base for classes that can be elements of a Path.
   *
   * Currently, this includes Module and the Path class itself.
   */
  class PathElement {
  public:
    /** Constructor. */
    PathElement() { }

    /** Destructor. */
    virtual ~PathElement() { }

    /** Return a string representation of the Modules in this path */
    virtual std::string getPathString() const = 0;

    /** Create an independent copy of this element.  */
    virtual std::shared_ptr<PathElement> clone() const = 0;

    /** Returns a sequential list of Modules in this path element. */
    virtual std::list<std::shared_ptr<Module> > getModules() const = 0;
  };
} // end namespace Belle2
