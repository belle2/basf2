#include <tracking/modules/datareduction/PXDLadderList.h>

using namespace std;
using namespace Belle2;


PXDLadderList::PXDLadderList()
{

}


PXDLadderList::~PXDLadderList()
{
  deleteAllLadders();
}


void PXDLadderList::deleteAllLadders()
{
  PXDLadderList::iterator listIter;
  for (listIter = begin(); listIter != end(); ++listIter) {
    delete *listIter;
  }
  clear();
}


void PXDLadderList::clearAllRegions()
{
  PXDLadderList::iterator listIter;
  for (listIter = begin(); listIter != end(); ++listIter) {
    PXDLadder* ladder = *listIter;
    ladder->clearRegions();
  }
}
