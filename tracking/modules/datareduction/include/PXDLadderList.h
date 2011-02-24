#ifndef PXDLADDERLIST_H
#define PXDLADDERLIST_H
#include <tracking/modules/datareduction/PXDLadder.h>

#include <list>

namespace Belle2 {
  class PXDLadderList : public std::list<PXDLadder*> {
  public:

    PXDLadderList();
    virtual ~PXDLadderList();

    void deleteAllLadders();
    void clearAllRegions();


  protected:


  private:

  };
}
#endif /* PXDLADDERLIST_H_ */
