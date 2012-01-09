#ifndef VERTEXFITTERVERTEXCONTAINER_H
#define VERTEXFITTERVERTEXCONTAINER_H

#include <TObject.h>
#include <rave/Vertex.h>
#include <stdexcept>

namespace Belle2 {
  class VertexFitterVertexContainer : public TObject {

  public:
    VertexFitterVertexContainer() : vertex(NULL) {
    }

    explicit VertexFitterVertexContainer(const rave::Vertex& vert) : vertex(new rave::Vertex(vert)) {
    }

    VertexFitterVertexContainer(const VertexFitterVertexContainer& rhs) : vertex(rhs.vertex == NULL ? NULL : new rave::Vertex(*rhs.vertex)) {
    }

    virtual ~VertexFitterVertexContainer() {
      delete vertex;
    }

    bool containsValidVertex() const {
      return vertex != NULL && vertex->isValid();
    }

    const rave::Vertex& getVertex() const {
      if (vertex == NULL)
        throw std::runtime_error("Called getVertex() on empty RaveVertexContainer.");
      return *vertex;
    }

    const VertexFitterVertexContainer& operator=(const VertexFitterVertexContainer& rhs) {
      if (this == &rhs)
        return *this;

      delete vertex;
      vertex = rhs.vertex == NULL ? NULL : new rave::Vertex(*rhs.vertex);
      return *this;
    }

  private:
    rave::Vertex* vertex;

    //ClassDef(RaveVertexContainer, 2);
  };
}

#endif
