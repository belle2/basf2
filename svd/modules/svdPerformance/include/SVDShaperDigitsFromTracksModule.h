/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDSHAPERDIGITSFROMTRACKSMODULE_H
#define SVDSHAPERDIGITSFROMTRACKSMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <svd/dataobjects/SVDShaperDigit.h>

namespace Belle2 {
  /**
   * generates two new StoreArray from the input StoreArray. One contains all ShaperDigits related to Tracks and the other contains all SahperDigits not related to tracks
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
    std::string m_svdreco; /**<SVDRecoDigits StoreArray*/
    std::string m_svdcluster; /**<SVDCLuster store array*/
    std::string m_recotrack; /**<reco track store array*/
    std::string m_track; /**<Track store array*/

    bool m_inheritance = false; /**< if true all relations are inherited*/
    bool m_useWithRel5Reco = false; /**<if true uses SVDRecoDigits relations*/

    std::string m_outputINArrayName;  /**< StoreArray with the selected output shaperdigits*/
    std::string m_outputOUTArrayName;  /**< StoreArray with the NOT selected output shaperdigits */

    SelectSubset<SVDShaperDigit> m_selectedShaperDigits; /**< all shaperdigits */
    SelectSubset<SVDShaperDigit> m_notSelectedShaperDigits; /**< all shaperdigits from tracks */

    static bool isRelatedToTrack(const SVDShaperDigit* shaperdigit); /**< select the shaperdigits related to tracks **/
    static bool isRelatedToTrackRel5(const SVDShaperDigit*
                                     shaperdigit); /**< select the shaperdigits related to tracks using SVDRecoDigits**/

  };
}

#endif /* SVDSHAPERDIGITSFROMTRACKSMODULE_H */
