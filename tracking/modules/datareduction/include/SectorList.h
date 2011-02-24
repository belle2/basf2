#ifndef SECTORLIST_H
#define SECTORLIST_H
#include <tracking/modules/datareduction/PXDLadderList.h>

#include <list>
#include <tracking/modules/datareduction/TrackerHit.h>
#include <tracking/modules/datareduction/SectorBasic.h>


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
#endif /* SECTORLIST_H */
