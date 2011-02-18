#ifndef SECTORLIST_H_
#define SECTORLIST_H_

#include <list>

#include <tracking/modules/datareduction/TrackerHit.h>

#include <tracking/modules/datareduction/SectorBasic.h>
#include <tracking/modules/datareduction/PXDLadderList.h>

namespace Belle2 {

  class SectorList : public std::list<SectorBasic*> {
  public:

    SectorList();
    virtual ~SectorList();

    int addHit(TrackerHit* hit);
    void doLadderIntersect(PXDLadderList& pxdLadderList);

    void clearAllHits();
    void clearAllLadders();
    void deleteAllSectors();


  protected:


  private:

  };
}
#endif /* SECTORLIST_H_ */
