#ifndef PXDLADDERLIST_H_
#define PXDLADDERLIST_H_

#include <list>

#include <tracking/modules/datareduction/PXDLadder.h>

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
