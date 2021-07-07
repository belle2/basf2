/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /** Forward declarations */
  class ECLCalDigit;
  class ECLCluster;
  class ECLTRGInformation;
  class ECLTriggerCell;
  class TRGECLUnpackerStore;
  class TRGECLUnpackerEvtStore;
  class TrgEclMapping;

  /**
   * Module to get ECL TRG energy information
   */
  class ECLTRGInformationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLTRGInformationModule();

    /** initialize */
    virtual void initialize() override;

    /** event */
    virtual void event() override;

    /** terminate */
    virtual void terminate() override;

  private:

    /** threshold for eclcluster energy sum */
    double m_clusterEnergyThreshold;

    /** map TCId, energy*/
    typedef std::map <unsigned, float> tcmap;


    /** vector (8736+1 entries) with cell id to store array positions */
    std::vector< int > m_calDigitStoreArrPosition;

    /** vector (576+1 entries) with TC id to store array positions */
    std::vector< int > m_TCStoreArrPosition;

    /** TC mapping class */
    TrgEclMapping* m_trgmap{nullptr};

    StoreArray<ECLCalDigit> m_eclCalDigits; /**< Required input array of ECLCalDigits  */
    StoreArray<ECLCluster> m_eclClusters; /**< Required input array of ECLClusters  */
    StoreArray<TRGECLUnpackerStore> m_trgUnpackerStore; /**< Required input array of TRGECLUnpackerStore  */
    StoreArray<TRGECLUnpackerEvtStore> m_trgUnpackerEvtStore; /**< Required input array of TRGECLUnpackerEvtStore  */

    StoreArray<ECLTriggerCell> m_eclTCs; /**< Output array of ECLTCs  */
    StoreObjPtr<ECLTRGInformation> m_eclTRGInformation; /**< Analysis level information per event holding TRG information*/
  };
}
