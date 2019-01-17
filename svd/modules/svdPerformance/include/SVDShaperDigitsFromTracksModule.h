/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSHAPERDIGITSFROMTRACKSMODULE_H
#define SVDSHAPERDIGITSFROMTRACKSMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

namespace Belle2 {
  /**
   * generates a new StoreArray from the input StoreArray which has all specified ShaperDigits removed
   *
   *
   */
  class SVDShaperDigitsFromTracksModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDShaperDigitsFromTracksModule();

    /** if required */
    virtual ~SVDShaperDigitsFromTracksModule();

    /** initializes the module */
    virtual void beginRun() override;

    /** processes the event */
    virtual void event() override;

    /** end the run */
    virtual void endRun() override;

    /** terminates the module */
    virtual void terminate() override;

    /** init the module */
    virtual void initialize() override;

  private:

    std::string m_svdshaper;  /**< StoreArray with the input shaperdigits */
    std::string m_svdreco;
    std::string m_svdcluster;
    std::string m_recotrack;
    std::string m_track;

    std::string m_outputINArrayName;  /**< StoreArray with the selected output shaperdigits*/
    std::string m_outputOUTArrayName;  /**< StoreArray with the NOT selected output shaperdigits */

    SelectSubset<SVDShaperDigit> m_selectedShaperDigits; /** all shaperdigits */
    SelectSubset<SVDShaperDigit> m_notSelectedShaperDigits; /** all shaperdigits from tracks */
  };
}

#endif /* SVDSHAPERDIGITSFROMTRACKSMODULE_H */
