/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPFrontEndMap.h>
#include <framework/gearbox/GearDir.h>
#include <framework/database/DBArray.h>
#include <framework/database/IntervalOfValidity.h>

#include <map>
#include <unordered_set>


namespace Belle2 {
  namespace TOP {

    /**
     * Provides mapping between electronics module position within a TOP module and
     * SCROD ID, COPPER and Finesse slot
     */
    class FrontEndMapper {

    public:

      /**
       * constructor
       */
      FrontEndMapper();

      /**
       * destructor
       */
      ~FrontEndMapper();

      /**
       * Initialize from Gearbox (XML)
       * @param frontEndMapping XML data directory
       */
      void initialize(const GearDir& frontEndMapping);

      /**
       * Initialize from database
       */
      void initialize();

      /**
       * check if the mapping is available
       * @return true if available
       */
      bool isValid() const {return m_valid;}

      /**
       * import mappings to database
       * @param iov     Interval of validity.
       */
      void import(const IntervalOfValidity& iov) const;

      /**
       * Return map from TOP module side
       * @param moduleID TOP module ID
       * @param bs boardstack number
       * @return pointer to map element or NULL
       */
      const TOPFrontEndMap* getMap(int moduleID, int bs) const
      {
        moduleID--;
        if (moduleID >= 0 and moduleID < c_numModules and bs >= 0 and bs < c_numColumns)
          return m_fromModule[moduleID][bs];
        return 0;
      }

      /**
       * Return map from SCROD side
       * @param scrodID SCROD ID
       * @return pointer to map element or NULL
       */
      const TOPFrontEndMap* getMap(unsigned short scrodID) const
      {
        std::map<unsigned short, const TOPFrontEndMap*>::const_iterator it =
          m_fromScrod.find(scrodID);
        if (it == m_fromScrod.end()) return 0;
        return it->second;
      }

      /**
       * Return map from COPPER/Finesse side
       * @param copperID COPPER ID
       * @param finesse Finesse slot number
       * @return pointer to map element or NULL
       */
      const TOPFrontEndMap* getMapFromCopper(unsigned copperID, int finesse) const
      {
        std::map<unsigned int, const TOPFrontEndMap*>::const_iterator it =
          m_fromCopper.find(copperID * 4 + finesse);
        if (it == m_fromCopper.end()) return 0;
        return it->second;
      }

      /**
       * Return size of the map
       * @return size
       */
      int getMapSize() const
      {
        if (m_fromDB) {
          return m_mappingDB->getEntries();
        } else {
          return m_mapping.size();
        }
      }

      /**
       * Return a set of copper ID's
       * @return copper ID's
       */
      const std::unordered_set<unsigned int>& getCopperIDs() const
      {
        return m_copperIDs;
      }

      /**
       * Returns module construction number
       * @param moduleID module ID (slot number)
       * @return module construction number
       */
      int getModuleCNumber(int moduleID) const
      {
        for (int bs = 0; bs < c_numColumns; bs++) {
          const auto* map = getMap(moduleID, bs);
          if (map) return map->getModuleCNumber();
        }
        return 0;
      }

      /**
       * Print mappings to terminal screen
       */
      void print() const;

    private:

      /**
       * Number of TOP modules (number of physical ones can be less),
       * number of electronic modules (SCROD's) per TOP module
       */
      enum {c_numModules = 16, c_numColumns = 4};

      /**
       * copy constructor
       */
      FrontEndMapper(const FrontEndMapper&)
      {}

      /**
       * assignment operator
       */
      FrontEndMapper& operator=(const FrontEndMapper&)
      {return *this;}

      /**
       * Clear
       */
      void clear();

      /**
       * re-do conversion maps when DBArray has changed
       */
      void update();

      std::vector<TOPFrontEndMap> m_mapping;      /**< mappings from gearbox */
      DBArray<TOPFrontEndMap>* m_mappingDB = 0;   /**< mappings from database */
      bool m_valid = false;                       /**< true, if mapping available */
      bool m_fromDB = false;                      /**< true, if from database */

      std::unordered_set<unsigned int> m_copperIDs; /**< COPPER ID's */
      const TOPFrontEndMap* m_fromModule[c_numModules][c_numColumns] = {{0}}; /**< conversion */
      std::map<unsigned short, const TOPFrontEndMap*> m_fromScrod; /**< conversion */
      std::map<unsigned int, const TOPFrontEndMap*> m_fromCopper;  /**< conversion */

    };

  } // TOP namespace
} // Belle2 namespace

