/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOOKUPCREATORMODULE_H
#define LOOKUPCREATORMODULE_H

#include <framework/core/Module.h>
#include <analysis/dbobjects/KeyMap.h>
#include <analysis/dbobjects/LookupTable.h>
#include <string>

namespace Belle2 {

  /**
    * Currently, std::pair can not be retreived from python list
    * We will get LookUpTable entries with tuples and transform tuples to pairs here.
    */
  typedef std::tuple<double, double> BinLimitsTuple;
  typedef std::map<std::string, BinLimitsTuple> NDBinTuple;
  typedef std::tuple<WeightInfo, NDBinTuple> noIdEntry;
  typedef std::tuple<noIdEntry, double> specificIDEntry;
  typedef std::vector<noIdEntry> listOfNoIdEntries;
  typedef std::vector<specificIDEntry> listOfSpecificIDEntries;

  /**
   * prints particle list to screen
   */
  class LookUpCreatorModule : public Module {

    listOfNoIdEntries m_tableIDNotSpec;
    listOfSpecificIDEntries m_tableIDSpec;
    WeightInfo m_outOfRangeWeight;
    std::string m_tableName;
    /**
     * Currently, std::pair can not be retreived from python list
     * We will get LookUpTable entries with tuples and transform tuples to pairs here.
     */
    NDBin NDBinTupleToNDBin(NDBinTuple bin_tuple);

  public:

    /**
     * Constructor
     */
    LookUpCreatorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

  };

} // Belle2 namespace

#endif
