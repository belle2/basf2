/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPFrontEndMap.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <map>
#include <unordered_set>


namespace Belle2 {
  namespace TOP {

    /**
     * Provides mapping between electronics module position within a TOP module and
     * SCROD ID, COPPER and Finesse board
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
       * initialize: get mapping from DB
       * @param frontEndMapping xpath to the mapping
       */
      void initialize(const GearDir& frontEndMapping);

      /**
       * Return map from TOP module side
       * @param barID TOP module ID
       * @param col column
       * @return pointer to map element or NULL
       */
      const TOPFrontEndMap* getMap(int barID, int col) const
      {
        barID--;
        if (barID >= 0 and barID < c_numModules and col >= 0 and col < c_numColumns)
          return m_fromBar[barID][col];
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
      int getMapSize() const {return m_mapping.size();}

      /**
       * Return a set of copper ID's
       * @return copper ID's
       */
      const std::unordered_set<unsigned int>& getCopperIDs() const
      {
        return m_copperIDs;
      }

    private:
      /**
       * Number of TOP modules (number of physical ones can be less),
       * number of electronic modules (SCROD's) per TOP module
       */
      enum {c_numModules = 16, c_numColumns = 4};

      std::vector<TOPFrontEndMap> m_mapping; /**< mapping vector */

      std::unordered_set<unsigned int> m_copperIDs; /**< COPPER ID's */
      const TOPFrontEndMap* m_fromBar[c_numModules][c_numColumns]; /**< conversion array */
      std::map<unsigned short, const TOPFrontEndMap*> m_fromScrod; /**< conversion map */
      std::map<unsigned int, const TOPFrontEndMap*> m_fromCopper;  /**< conversion map */

    };

  } // TOP namespace
} // Belle2 namespace

