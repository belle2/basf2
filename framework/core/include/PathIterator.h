/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Path.h>
#include <memory>


namespace Belle2 {

  class Module;

  /** Iterator over a Path (returning Module pointers).
   *
   */
  class PathIterator {
  public:
    /** Constructor. */
    explicit PathIterator(const PathPtr& path) :
      m_path(path),
      m_iter(path->m_elements.begin()),
      m_end(path->m_elements.end()),
      m_parentIterator()
    {
      descendIfNecessary();
    }

    /** Constructor with back-reference to an iterator over the parent path. */
    explicit PathIterator(const PathPtr& path, const PathIterator& parentIterator) :
      m_path(path),
      m_iter(path->m_elements.begin()),
      m_end(path->m_elements.end()),
      m_parentIterator(new PathIterator(parentIterator))
    {
      descendIfNecessary();
    }

    /** increment. */
    void next()
    {
      if (!isDone())
        ++m_iter;
      descendIfNecessary();
    }

    /** Check if we're pointing to another path and descend if that is the case. */
    void descendIfNecessary()
    {
      if (!isDone() and dynamic_cast<Path*>(m_iter->get())) {
        //we're pointing to another Path
        *this = PathIterator(std::static_pointer_cast<Path>(*m_iter), *this);
      }
      //check _afterwards_ if we need to jump back up
      if (isDone() and m_parentIterator) {
        //jump back to parent iterator
        *this = *(m_parentIterator.get());
        next(); //go to next module
      }
    }

    /** Are we finished iterating? */
    bool isDone() const { return (m_iter == m_end); }

    /** dereference. */
    Module* get() const { return dynamic_cast<Module*>(m_iter->get()); }


  private:
    PathPtr m_path; /**< keep the path around to ensure iterators remain valid. */
    std::list<std::shared_ptr<PathElement> >::const_iterator m_iter; /**< wrapped path list iterator. */
    std::list<std::shared_ptr<PathElement> >::const_iterator m_end; /**< iterator to end of Path. */
    std::shared_ptr<PathIterator> m_parentIterator; /**< If this is non-NULL, we jump to m_parentIterator+1 after m_iter is done. */
  };

} // end namespace Belle2