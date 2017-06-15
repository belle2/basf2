/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
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
    setupNameToFilters_t* m_allSetupsFilters;


    /// Copies of this class, which is a singleton, are not allowed so the
    /// copy constructor method is deleted.
    FiltersContainer(const FiltersContainer& filtersContainer)  = delete;
    /// dont allow copies
    FiltersContainer& operator = (const FiltersContainer&) = delete;

    /// Singleton so use a hidden Constructor.
    FiltersContainer() : m_allSetupsFilters(NULL)
    {
      m_allSetupsFilters = new setupNameToFilters_t;
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
      for (auto& filter : * m_allSetupsFilters)
        delete filter.second;
      // Thomas: added this delete as it is newed in the constructor, check if it makes problems
      delete m_allSetupsFilters;
    }


    /// Gives access to the sector map and filters of a given setup
    VXDTFFilters<point_t>* getFilters(const std::string& setupName)
    {
      auto  result = m_allSetupsFilters->find(setupName);
      if (result == m_allSetupsFilters->end())
        return NULL;
      return result->second;
    }


    /** returns all the available setups. */
    const setupNameToFilters_t& getAllSetups(void)
    {
      return * m_allSetupsFilters;
    }


    /** assigns filters. */
    void assignFilters(const std::string& setupName ,
                       VXDTFFilters<point_t>* filters)
    {
      if ((*m_allSetupsFilters).count(setupName)) {
        // case there is already a filter with this name in the container, we dont allow that it is overwritten!
        B2WARNING("Trying to add a filter which is already in the container! Will not add it! And delete it!");
        // assignFilters assumes that the ownership is taken by the container,
        // so to not have a mem leak delete it if it is not the same as in the container
        if (filters && filters != (*m_allSetupsFilters)[ setupName ]) delete filters;

        return;
      }
      (*m_allSetupsFilters)[ setupName ] = filters;
    }

  };
}
#endif
