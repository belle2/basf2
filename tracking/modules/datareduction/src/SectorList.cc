#include <tracking/modules/datareduction/SectorList.h>
#include <tracking/modules/datareduction/SectorStraight.h>
#include <tracking/modules/datareduction/SectorArc.h>

using namespace std;
using namespace Belle2;

SectorList::SectorList()
{

}


SectorList::~SectorList()
{
  deleteAllSectors();
}

/*
int SectorList::addHit(EVENT::TrackerHit* hit)
{
  int numSectors = 0;
  bool hitAdded;

  SectorList::iterator listIter;
  for (listIter = begin(); listIter != end(); ++listIter) {
    SectorBasic* currSector = *listIter;
    hitAdded = currSector->addHit(hit);
    if (hitAdded) numSectors++;
  }
  return numSectors;
}
*/

int SectorList::addHit(TrackerHit* hit)
{
  int numSectors = 0;
  bool hitAdded;

  SectorList::iterator listIter;
  for (listIter = begin(); listIter != end(); ++listIter) {
    SectorBasic* currSector = *listIter;
    hitAdded = currSector->addHit(hit);
    if (hitAdded) numSectors++;
  }
  return numSectors;
}
void SectorList::doLadderIntersect(PXDLadderList& pxdLadderList)
{
  PXDLadderList::iterator ladderIter;
  SectorList::iterator sectorIter;
  PXDLadder* currentLadder;
  SectorBasic* currentSector;

  for (sectorIter = begin(); sectorIter != end(); ++sectorIter) {
    currentSector = *sectorIter;

    for (ladderIter = pxdLadderList.begin(); ladderIter != pxdLadderList.end(); ++ladderIter) {
      currentLadder = *ladderIter;
      currentSector->addPXDLadder(currentLadder);
    }
  }
}


void SectorList::clearAllHits()
{
  SectorList::iterator listIter;
  for (listIter = begin(); listIter != end(); ++listIter) {
    SectorBasic* currSector = *listIter;
    currSector->clearHits();
  }
}


void SectorList::clearAllLadders()
{
  SectorList::iterator listIter;
  for (listIter = begin(); listIter != end(); ++listIter) {
    SectorBasic* currSector = *listIter;
    currSector->clearLadders();
  }
}


void SectorList::deleteAllSectors()
{
  SectorList::iterator listIter;
  for (listIter = begin(); listIter != end(); ++listIter) {
    delete *listIter;
  }
  clear();
}
