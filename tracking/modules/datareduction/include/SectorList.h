#ifndef SECTORLIST_H_
#define SECTORLIST_H_

#include <list>

#include "lcio.h"
#include <EVENT/MCParticle.h>
#include <EVENT/TrackerHit.h>

#include "SectorBasic.h"
#include "PXDLadderList.h"


class SectorList : public std::list<SectorBasic*> {
public:

  SectorList();
  virtual ~SectorList();

  int addHit(EVENT::TrackerHit* hit);
  void doLadderIntersect(PXDLadderList& pxdLadderList);

  void clearAllHits();
  void clearAllLadders();
  void deleteAllSectors();


protected:


private:

};

#endif /* SECTORLIST_H_ */
