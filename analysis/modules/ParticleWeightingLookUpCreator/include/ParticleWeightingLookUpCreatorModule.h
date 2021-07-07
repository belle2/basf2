/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <string>

namespace Belle2 {

  /**
    * Currently, std::pair can not be retrieved from python list
    * We will get ParticleWeightingLookUpTable entries with tuples and
    * transform tuples to pairs here.
    */

  typedef std::tuple<double, double> BinLimitsTuple; /**< the tuple of bin limits */
  typedef std::map<std::string, BinLimitsTuple> NDBinTuple; /**< the NDimensional tuple of bin limits */
  typedef std::tuple<WeightInfo, NDBinTuple> noIdEntry;/**< the ID */
  typedef std::tuple<noIdEntry, double> specificIDEntry; /**< the specific ID */
  typedef std::vector<noIdEntry> listOfNoIdEntries; /**< List of entries for table to be created from without specified ID */
  typedef std::vector<specificIDEntry> listOfSpecificIDEntries; /**< List of entries for table to be created from with specified ID */

  /**
   * Module that creates LookUpTable and upload it to the DB
   */
  class ParticleWeightingLookUpCreatorModule : public Module {

    listOfNoIdEntries m_tableIDNotSpec; /**< List of entries for table to be created from without specified ID */
    listOfSpecificIDEntries m_tableIDSpec; /**< List of entries for table to be created from with specified ID */
    WeightInfo m_outOfRangeWeight; /**< Weight info for out-of-range bin */
    std::string m_tableName; /**< Name of the table */
    int m_experimentLow; /**< Lowest valid experiment # */
    int m_experimentHigh; /**< Highest valid experiment # */
    int m_runLow; /**< Lowest valid run # */
    int m_runHigh; /**< Highest valid run # */

    /**
     * Currently, std::pair can not be retrieved from python list
     * We will get ParticleWeightingLookUpTable entries with tuples and transform tuples to pairs here.
     */
    NDBin NDBinTupleToNDBin(const NDBinTuple& bin_tuple);

  public:

    /**
     * Constructor
     */
    ParticleWeightingLookUpCreatorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /** Clean up anything created in initialize(). */
    virtual void terminate() override;

  private:
    ParticleWeightingBinLimits* lims{nullptr}; /**< object holding bin limits */
  };

} // Belle2 namespace
