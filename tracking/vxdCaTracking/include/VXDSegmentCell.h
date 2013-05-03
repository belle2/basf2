/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDSEGMENTCELL_H
#define VXDSEGMENTCELL_H



#include <list>
#include <map>
#ifndef __CINT__
#include <boost/unordered_map.hpp>
#endif

#include <TVector3.h>

#include "VXDTFHit.h"
#include "VXDSector.h"

namespace Belle2 {

//  class VXDSector;
//  class VXDTFHit;

  /** \addtogroup dataobjects
   * @{
   */

  /** The VXD SegmentCell class
   * This class represents segments of track candidates needed for the Cellular automaton (CA) implemented in the VXDTF.
   */
  class VXDSegmentCell {
  public:
//    using boost::unordered_map;
#ifndef __CINT__
    typedef boost::unordered_map<unsigned int, VXDSector*> MapOfSectors; /**< stores whole sectorMap used for storing cutoffs */
#endif

    /** Default constructor for the ROOT IO. */
    VXDSegmentCell():
      m_pOuterHit(NULL),
      m_pInnerHit(NULL),
      m_pOuterSector(NULL),
      m_pInnerSector(NULL) { m_state = 0; m_activated = true; m_seed = true; m_stateUpgrade = false; }

    /** Constructor.
    //      * @param pOuterHit pointer to hit forming the outer end of the SegmentCell.
    //      * @param pInnerHit pointer to hit forming the inner end of the SegmentCell.
    //      * @param pOuterSector pointer to sector in which the outer hit lies.
    //      * @param pInnerSector pointer to sector in which the inner hit lies.
    //      */
#ifndef __CINT__
    VXDSegmentCell(VXDTFHit* pOuterHit, VXDTFHit* pInnerHit, MapOfSectors::iterator pOuterSector, MapOfSectors::iterator pInnerSector);
#endif

    int getState() const { return m_state; } /**< returns state of Cell (CA-feature) */
    bool isSeed() const { return m_seed; } /**< returns whether Cell is allowed to be a seed for TCs */
    bool isActivated() const { return m_activated; } /**< returns activationState (CA-feature) */
    bool isUpgradeAllowed() const { return m_stateUpgrade; } /**< returns info whether stateIncrease is allowed or not (CA-feature) */
    VXDTFHit* getInnerHit() const; /**< returns inner hit of current Cell */
    VXDTFHit* getOuterHit() const; /**< returns outer hit of current Cell */

    std::list<Belle2::VXDSegmentCell*>& getInnerNeighbours(); /**< returns list of inner Neighbours (CA-feature and needed by TC-Collector), does deliver different results depending on when you call that function */
    const std::list<Belle2::VXDSegmentCell*>& getAllInnerNeighbours() const; /**< returns list of all inner neighbours (does not change during event) */
    std::list<Belle2::VXDSegmentCell*>& getOuterNeighbours(); /**< returns list of outer Neighbours */
    std::list<Belle2::VXDSegmentCell*>::iterator eraseInnerNeighbour(std::list<VXDSegmentCell*>::iterator it); /**< incompatible neighbours get kicked when new information about the situation recommends that step */

    void kickFalseFriends(TVector3 primaryVertex); /**<  checks state of inner neighbours and removes incompatible and virtual ones */
    void copyNeighbourList(); /**<   makes a copy of m_innerNeighbours (to be used before CA!) */
    void increaseState() { m_state++; } /**< increases state during CA update step */
    void allowStateUpgrade(bool upgrade) { m_stateUpgrade = upgrade; } /**< sets flag whether Cell is allowed to increase state during update step within CA */
    void setSeed(bool seedValue) { m_seed = seedValue; } /**< sets flag whether Cell is allowed to be the seed of a new track candidate or not */
    void setActivationState(bool activationState) { m_activated = activationState; } /**< sets flag whether Cell is active (takes part during current CA iteration) or inactive (does not take part, it is 'dead') */
    void addInnerNeighbour(VXDSegmentCell* aSegment); /**< adds an inner neighbour-cell */
    void addOuterNeighbour(VXDSegmentCell* aSegment); /**< adds an outer neighbour-cell */

  protected:
    VXDTFHit* m_pOuterHit; /**< pointer to hit forming the outer end of the SegmentCell. */
    VXDTFHit* m_pInnerHit; /**< pointer to hit forming the inner end of the SegmentCell. */
#ifndef __CINT__
    MapOfSectors::iterator m_pOuterSector; /**< link to sector carrying outer hit */
    MapOfSectors::iterator m_pInnerSector; /**< link to sector carrying inner hit */
#endif
    int m_state; /**< state of Cell during CA process, begins with 0 */
    bool m_activated; /**< activation state. Living Cells (active) are allowed to evolve in the CA, dead ones (inactive) are not allowed */
    bool m_stateUpgrade; /**< sets flag whether Cell is allowed to increase state during update step within CA */
    bool m_seed; /**< sets flag whether Cell is allowed to be the seed of a new track candidate or not */

    std::list<Belle2::VXDSegmentCell*> m_innerNeighbours; /**< segments attached at the inner end of current segment. Since this list gets reduced during CA process, a copy is made before that step. If you want to see all neighbours, use getAllInnerNeighbours */
    std::list<Belle2::VXDSegmentCell*> m_allInnerNeighbours; /**< carries full list of all inner neighbour-Cells. */
    std::list<Belle2::VXDSegmentCell*> m_outerNeighbours; /**< carries list of outer neighbour-Cells */

  };

  /** @}*/
} //Belle2 namespace
#endif
