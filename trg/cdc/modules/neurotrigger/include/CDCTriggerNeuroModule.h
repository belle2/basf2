/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGERNEUROMODULE_H
#define CDCTRIGGERNEUROMODULE_H

#include <framework/core/Module.h>
#include <trg/cdc/NeuroTrigger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPInput.h>

namespace Belle2 {

  /** The neural network module of the CDC trigger.
   * Select one of several trained neural networks
   * and run it to estimate the z-vertex of a track
   * from track segment hits and 2D track estimates.
   */
  class CDCTriggerNeuroModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerNeuroModule();

    /** Destructor. */
    virtual ~CDCTriggerNeuroModule() {}

    /** Initialize the module.
     * Load the network weights and register datastore objects.
     */
    virtual void initialize() override;

    /** Called once for each event.
     * Run the network for all CDCTriggerTracks and store output z-vertex
     * in the same CDCTriggerTrack.
     */
    virtual void event() override;
    /** shuffle the input ids in the input vector to match the hardware*/
    float hwInputIdShuffle(float tsid, int sl);

  protected:
    /** Name of file where network weights etc. are stored. */
    std::string m_filename;
    /** Name of the TObjArray holding the networks. */
    std::string m_arrayname;
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_EventTimeName;
    /** Name of the StoreArray containing the input 2D tracks or neurotracks. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray containing the real input 2D tracks. */
    std::string m_realinputCollectionName;
    /** Name of the StoreArray containing the resulting NN tracks. */
    std::string m_outputCollectionName;
    /** Instance of the NeuroTrigger. */
    NeuroTrigger m_NeuroTrigger;
    /** Switch to execute the network with fixed point calculation.
     * (for FPGA simulation) */
    bool m_fixedPoint;
    /** Fixed point precision in bit after radix point.
     *  8 values:
     *  - 2D track parameters: omega, phi
     *  - geometrical values derived from track: crossing angle, reference wire ID
     *  - scale factor: radian to wire ID
     *  - MLP values: nodes, weights, activation function LUT input (LUT output = nodes)
     */
    std::vector<unsigned> m_precision;
    /** way to obtain the event time, possible values are:
     *   "etf_only"                 :   only ETF info is used, otherwise an error
     *                                  is thrown.
     *   "fastestpriority"          :   event time is estimated by fastest priority
     *                                  time in selected track segments. if something
     *                                  fails, it is set to 0.
     *   "zero"                     :   the event time is set to 0.
     *   "etf_or_fastestpriority"   :   the event time is obtained by the ETF, if
     *                                  not possible, the flag
     *                                  "fastestppriority" is used.
     *   "etf_or_zero"              :   the event time is obtained by the ETF, if
     *                                  not possible, it es set to 0
     */
    std::string m_et_option;
    /** Switch for writing out the input vector for each track (off by default). */
    bool m_writeMLPinput;
    /** Switch to mimic an apparent bug in the hardware preprocessing. */
    bool m_hardwareCompatibilityMode;
    /** use Neurotracks as InputTracks */
    bool m_neuroTrackInputMode;
    /** list of input 2D tracks or neurotracks */
    StoreArray<CDCTriggerTrack> m_tracks2D;
    /** list of input real 2D tracks */
    StoreArray<CDCTriggerTrack> m_realtracks2D;
    /** list of output NN tracks */
    StoreArray<CDCTriggerTrack> m_tracksNN;
    /** list of track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    /** list of input vectors for each NN track */
    StoreArray<CDCTriggerMLPInput> m_mlpInput;
    /** get NNT payload from database. */
    DBObjPtr<CDCTriggerNeuroConfig> m_cdctriggerneuroconfig;
  };
}
#endif
