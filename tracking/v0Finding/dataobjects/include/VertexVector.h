/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <genfit/GFRaveVertex.h>
#include <vector>

namespace Belle2 {

  /**
  * Need this container for exception-safe cleanup, GFRave's
  * interface isn't exception-safe as is.  I guess this could
  * interface fail if delete throws an exception ...
  */
  class VertexVector {

  public:

    /// Detor which cleans up the pointers created by the vertex factory.
    ~VertexVector() noexcept
    {
      for (size_t i = 0; i < v.size(); ++i)
        delete v[i];
    }

    /// Return size of vertex vector.
    size_t size() const noexcept { return v.size(); }

    /// Random Access operator.
    genfit::GFRaveVertex*& operator[](int idx) { return v[idx]; }

    /// Fitted vertices.
    std::vector<genfit::GFRaveVertex*> v;
  };

}
