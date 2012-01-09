#ifndef VERTEXFITTERMODULE_H
#define VERTEXFITTERMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  class VertexFitterModule : public Module {

  public:
    VertexFitterModule();
    virtual ~VertexFitterModule() {}

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    std::string m_GFTracksColName;     ///< GFTrack instance collection name.
    std::string m_RAVEVerticesColName; ///< rave::Vertex instance collection name.
  };
}

#endif
