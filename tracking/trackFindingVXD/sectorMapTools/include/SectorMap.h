/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#ifndef SECTORMAP_HH
#define SECTORMAP_HH

#include <TObject.h>
#include <string>
#ifndef __CINT__
#include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
#include <unordered_map>
#include <string>
#endif
namespace Belle2 {


  /** This class contains everything needed by the VXDTF that is not going
  * to change during a RUN, i.e. IP position, B field @ IP, static sectors,
  * 2 space points filters, 3 space points filters,  etc. etc.
  * It will be put in the datastore with duration RUN.
  * This class owns her members and she is in charge for their deletions. */
  class SectorMap : public TObject {
  private:
    /** Contains all the Filters and configurations indexed by their setupNames
     * m_SegmentFilters is a pointer to an unsorted_map, since ROOT 5 is
     * not able to digest it... I will hide its identity to ROOTCINT
     * cfr. PIMPL or Opaque Pointers */
    void* m_allSetupsFilters;  //! transient value


    SectorMap(const SectorMap& sectorMap)
    // ROOT/CINT needs a copy constructor,
#ifndef __CINT__
    // but I do not want to copy this class which is a singleton so:
    // if __CINT__ is not defined I delete the copy constructor
      = delete
#endif
        ;

  public:

    SectorMap();
    virtual ~SectorMap();

    /** returns filters. */
    const VXDTFFilters* getFilters(const std::string& setupName);

#ifndef __CINT__
    /** returns setups. */
    const std::unordered_map< std::string, VXDTFFilters*>& getAllSetups(void);
#endif

    /** assigns filters. */
    void assignFilters(const std::string& setupName ,
                       VXDTFFilters* filters);


    /** root class definition. */
    ClassDef(SectorMap , 11);
  };
}
#endif
