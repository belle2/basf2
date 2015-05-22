/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDTFENVIRONMENT_HH
#define VXDTFENVIRONMENT_HH

#include <TObject.h>
#include <string>

namespace Belle2 {


  class VXDTFSegmentFilter;


  // This class contains everything needed by the VXDTF that is not going
  // to change during a RUN, i.e. IP position, B field @ IP, collections of
  // sector maps etc. etc.
  // It will be put in the datastore with duration RUN.
  // This class owns her members and she is in charge for their deletions.
  class VXDTFEnvironment : public TObject {
  private:
    // Contains all the SegmentFilters indexed by their names
    // m_SegmentFilters is a pointer to an unsorted_map, since ROOT 5 is
    // not able to digest it... I will hide its identity to ROOTCINT
    // cfr. PIMPL or Opaque Pointers
    void* m_SegmentFilters;  //! transient value

    // Eugenio's comment: this is a temporary solution.
    VXDTFEnvironment(const VXDTFSegmentFilter& VXDTFEnvironment)
#ifndef __CINT__
      = delete
#endif
        ;

  public:

    VXDTFEnvironment();
    virtual ~VXDTFEnvironment();

    const VXDTFSegmentFilter* getSegmentFilter(const std::string& name);

    void assignSegmentFilter(const std::string& name ,
                             VXDTFSegmentFilter* theFilter);


    ClassDef(VXDTFEnvironment , 0);
  };
}
#endif
