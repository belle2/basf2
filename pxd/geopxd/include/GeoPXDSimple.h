#include <geometry/geodetector/CreatorBase.h>
#ifndef GEOPXDSIMPLE_H_
#define GEOPXDSIMPLE_H_

namespace Belle2 {
  class GearDir;
  class GeoPXDSimple : public CreatorBase {
  public:
    GeoPXDSimple();
    virtual ~GeoPXDSimple();
    virtual void create(GearDir& content);
  protected:
  private:
  };
}

#endif
