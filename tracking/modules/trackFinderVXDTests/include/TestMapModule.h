/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/vxdCaTracking/VXDSector.h>
#include <map>
#include <vector>
#ifndef __CINT__
#include <boost/chrono.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/flat_map.hpp>
#endif


namespace Belle2 {
  namespace Tracking {

    /** The TestMapModule
    *
    * this module simply counts the number of events (and prints every x-th event onto the screen, where x is user-defined). Useful when executing modules which do not provide this feature themselves
    *
    */
    class TestMapModule : public Module {

    public:
      typedef boost::chrono::high_resolution_clock
      boostClock; /**< used for measuring time comsumption */ // high_resolution_clock, process_cpu_clock
//       typedef boost::chrono::microseconds boostNsec; /**< defines time resolution (currently mictroseconds) */ // microseconds, milliseconds
      typedef boost::chrono::nanoseconds boostNsec; /**< defines time resolution (currently nanoseconds) */ // microseconds, milliseconds
      typedef std::map<unsigned int, VXDSector>
      MapOfSectors; /**< defines a sectorMap using the stl::map, key is sectorID coded using FullSecID(), second is Sector itself */
      typedef boost::container::flat_map<unsigned int, VXDSector>
      FlatMapOfSectors; /**< defines a sectorMap using the boost::flat_map, key is sectorID coded using FullSecID(), second is Sector itself */
      typedef boost::unordered_map<unsigned int, VXDSector>
      UnorderedMapOfSectors; /**< defines a sectorMap using the boost::unordered_map, key is sectorID coded using FullSecID(), second is Sector itself */

      /**
      * Constructor of the module.
      */
      TestMapModule();

      /** Destructor of the module. */
      ~TestMapModule();

      /** Initializes the Module.
      */
      void initialize() override;

      /**
      * Prints a header for each new run.
      */
      void beginRun() override;

      /** Prints the full information about the event, run and experiment number. */
      void event() override;

      /**
      * Prints a footer for each run which ended.
      */
      void endRun() override;

      /**
      * Termination action.
      */
      void terminate() override;

      /** Fills Map with test entries */
      void FillMaps();

      /** generates a vector filled with randomly generated sector addresses */
      void FillVector(std::vector<unsigned int>& vec);

      /** jumbles/randomizes the entries of given vector */
      void JumbleVector(std::vector<unsigned int>& vec);

    protected:
      void doNothing(); /**< simple useless member, delete if you want */

      boostNsec m_eventMapStuff; /**< time consumption of the secMap-access-step (events) */
      boostNsec m_fillMapStuff; /**< time consumption of the secMap-creation (initialize) */
      boostNsec m_eventFlatMapStuff; /**< time consumption of the secMap-access-step (events) */
      boostNsec m_fillFlatMapStuff; /**< time consumption of the secMap-creation (initialize) */
      boostNsec m_fillUnorderedMapStuff; /**< time consumption of the secMap-creation (initialize) */
      boostNsec m_eventUnorderedMapStuff; /**< time consumption of the secMap-access-step (events) */
      int m_eventCounter; /**< knows current event number */
      int m_PARAMnumLayers; /**< sets how many layers are assumed (useful value: 4-6) */
      int m_PARAMnumSensors; /**< sets how many sensors per layer are assumed (useful value: ~227/numLayers) */
      int m_PARAMnumSectors;  /**< sets how many sectors per sensor are assumed (useful value: 4-50) */
      int m_PARAMnumFriends; /**< sets how many friends per sector are assumed (useful value: 2-20) */
      MapOfSectors m_testMap; /**< .first, key coded with FullSecID, .second, VXDSector filled with some friends */
      FlatMapOfSectors m_testFlatMap; /**< .first, key coded with FullSecID, .second, VXDSector filled with some friends */
      UnorderedMapOfSectors m_testUnorderedMap; /**< .first, key coded with FullSecID, .second, VXDSector filled with some friends */
      std::vector<unsigned int> m_sectorAdresses; /**< contains all keys of the sectorMap */

    private:

    };
  }
}
