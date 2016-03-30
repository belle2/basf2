#pragma once

#include <genfit/GFRaveVertex.h>
#include <vector>

namespace Belle2 {

  class VertexVector {
    // Need this container for exception-safe cleanup, GFRave's
    // interface isn't exception-safe as is.  I guess this could
    // fail if delete throws an exception ...
  public:
    ~VertexVector() noexcept
    {
      for (size_t i = 0; i < v.size(); ++i)
        delete v[i];
    }
    size_t size() const noexcept { return v.size(); }
    genfit::GFRaveVertex*& operator[](int idx) { return v[idx]; }
    std::vector<genfit::GFRaveVertex*> v;
  };

}
