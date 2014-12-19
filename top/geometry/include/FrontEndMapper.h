/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FRONTENDMAPPER_H
#define FRONTENDMAPPER_H

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <map>
#include <unordered_set>


namespace Belle2 {
  namespace TOP {

    /**
     * Mapping of electronic module position to SCROD and COPPER/Finesse
     */
    struct FEEMap {
      int barID;              /**< TOP module ID */
      int column;             /**< column within TOP module */
      unsigned short scrodID; /**< SCROD ID */
      unsigned int copperID;  /**< COPPER ID */
      int finesseID;          /**< Finesse slot number (0-based) */
      int index;              /**< index of this element in std::vector */

      /**
       * Default constructor
       */
      FEEMap(): barID(0), column(0), scrodID(0), copperID(0), finesseID(0), index(0)
      {}

      /**
       * Full constructor
       * @param bar TOP module ID
       * @param col column within TOP module
       * @param scrod SCROD ID
       * @param copper COPPER ID
       * @param finesse Finesse slot number
       * @param i index of this element in std::vector
       */
      FEEMap(int bar,
             int col,
             unsigned short scrod,
             unsigned copper,
             int finesse,
             int i) {
        barID = bar;
        column = col;
        scrodID = scrod;
        copperID = copper;
        finesseID = finesse;
        index = i;
      }
    };


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
      const FEEMap* getMap(int barID, int col) const {
        barID--;
        if (barID >= 0 and barID < c_numModules and col >= 0 and col < c_numColumns)
          return m_fromBarToScrod[barID][col];
        return 0;
      }

      /**
       * Return map from SCROD side
       * @param scrodID SCROD ID
       * @return pointer to map element or NULL
       */
      const FEEMap* getMap(unsigned short scrodID) const {
        std::map<unsigned short, const FEEMap*>::const_iterator it =
          m_fromScrodToBar.find(scrodID);
        if (it == m_fromScrodToBar.end()) return 0;
        return it->second;
      }

      /**
       * Return map from COPPER/Finesse side
       * @param copperID COPPER ID
       * @param finesse Finesse slot number
       * @return pointer to map element or NULL
       */
      const FEEMap* getMapFromCopper(unsigned copperID, int finesse) const {
        std::map<unsigned int, const FEEMap*>::const_iterator it =
          m_fromCopperInputToBar.find(copperID * 4 + finesse);
        if (it == m_fromCopperInputToBar.end()) return 0;
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
      const std::unordered_set<unsigned int>& getCopperIDs() const {
        return m_copperIDs;
      }

    private:
      /**
       * Number of TOP modules (number of physical ones can be less),
       * number of electronic modules (SCROD's) per TOP module
       */
      enum {c_numModules = 16, c_numColumns = 4};

      std::vector<FEEMap> m_mapping; /**< mapping */
      std::unordered_set<unsigned int> m_copperIDs; /**< COPPER ID's */
      const FEEMap* m_fromBarToScrod[c_numModules][c_numColumns]; /**< conversion */
      std::map<unsigned short, const FEEMap*> m_fromScrodToBar;   /**< conversion */
      std::map<unsigned int, const FEEMap*> m_fromCopperInputToBar;   /**< conversion */

    };

  } // TOP namespace
} // Belle2 namespace

#endif
