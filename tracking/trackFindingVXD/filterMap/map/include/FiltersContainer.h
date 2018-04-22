/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni,  Thomas Lueck                                 *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#ifndef FILTERSCONTAINER_HH
#define FILTERSCONTAINER_HH

#include <string>
#include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
#include <unordered_map>
#include <string>

namespace Belle2 {

  /** This class  contains everything needed by the VXDTF that is not going
  * to change during a RUN, i.e. IP position, B field @ IP, static sectors,
  * 2 space points filters, 3 space points filters,  etc. etc.
  * It is a singleton and is shared by all other processes
  * This class owns her members and she is in charge for their deletions. */
  template<class point_t>
  class FiltersContainer {
  public:

    /// typedef for better readability. Index is name of the setup.
    using setupNameToFilters_t = std::unordered_map< std::string, Belle2::VXDTFFilters<point_t>*>;

  private:

    /** Container of all the Filters indexed by their setupNames. */
    setupNameToFilters_t m_allSetupsFilters;


    /// Copies of this class, which is a singleton, are not allowed so the
    /// copy constructor method is deleted.
    FiltersContainer(const FiltersContainer& filtersContainer)  = delete;
    /// dont allow copies
    FiltersContainer& operator = (const FiltersContainer&) = delete;

    /// Singleton so use a hidden Constructor.
    FiltersContainer()
    {
    }

  public:

    /// one and only way to access the singleton object
    static FiltersContainer& getInstance()
    {
      /// the unique_ptr takes care for the deletion of the object!
      static std::unique_ptr<FiltersContainer> instance(new FiltersContainer());
      return * instance;
    }


    /// Destructor deleting all filters stored.
    virtual ~FiltersContainer()
    {
      for (auto& filter : m_allSetupsFilters)
        delete filter.second;
    }


    /// Gives access to the sector map and filters of a given setup
    VXDTFFilters<point_t>* getFilters(const std::string& setupName)
    {
      auto  result = m_allSetupsFilters.find(setupName);
      if (result == m_allSetupsFilters.end())
        return NULL;
      return result->second;
    }


    /** returns all the available setups. */
    const setupNameToFilters_t& getAllSetups(void)
    {
      return  m_allSetupsFilters;
    }


    /** assigns filters. The ownership of filters is taken by FiltersContainer! */
    void assignFilters(const std::string& setupName ,
                       VXDTFFilters<point_t>* filters)
    {
      // case there is already a filter with this name in the container, we delete that filter!
      if (m_allSetupsFilters.count(setupName)) {
        B2INFO("FiltersContainer: Replacing existing filter for setup name: " << setupName);
        if (m_allSetupsFilters[ setupName ] && filters != m_allSetupsFilters[ setupName ]) delete m_allSetupsFilters[ setupName ];
      }
      m_allSetupsFilters[ setupName ] = filters;
    }

  };
}
#endif
