/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#ifndef VXDTFENVIRONMENT_HH
#define VXDTFENVIRONMENT_HH

#include <TObject.h>
#include <string>

namespace Belle2 {


  class VXDTFFilters;


  // This class contains everything needed by the VXDTF that is not going
  // to change during a RUN, i.e. IP position, B field @ IP, collections of
  // sector maps etc. etc.
  // It will be put in the datastore with duration RUN.
  // This class owns her members and she is in charge for their deletions.
  class VXDTFEnvironment : public TObject {
  private:
    // Contains all the Filters indexed by their setupNames
    // m_SegmentFilters is a pointer to an unsorted_map, since ROOT 5 is
    // not able to digest it... I will hide its identity to ROOTCINT
    // cfr. PIMPL or Opaque Pointers
    void* m_allSetupsFilters;  //! transient value


    VXDTFEnvironment(const VXDTFEnvironment& VXDTFEnvironment)
    // ROOT/CINT needs a copy constructor,
#ifndef __CINT__
    // but I do not want to copy this class which is a singleton so:
    // if __CINT__ is not defined I delete the copy constructor
      = delete
#endif
        ;

  public:

    VXDTFEnvironment();
    virtual ~VXDTFEnvironment();

    const VXDTFFilters* getFilters(const std::string& setupName);


    void assignFilters(const std::string& setupName ,
                       VXDTFFilters* filters);


    ClassDef(VXDTFEnvironment , 0);
  };
}
#endif
