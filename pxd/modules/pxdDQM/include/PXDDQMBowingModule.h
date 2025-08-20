/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/ParticleList.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <vxd/dataobjects/VxdID.h>

#include "TH1F.h"

namespace Belle2 {
  /**
   * Create basic histograms for PXD bowing monitoring
   */
  class PXDDQMBowingModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDDQMBowingModule();

    /// Virtual destructor (base class)
    virtual ~PXDDQMBowingModule() {}

    /** initializes the needed store arrays and histograms*/
    void initialize() override final;

    /** Resets the histograms for each run and print ROI size in v */
    void beginRun() override final;

    /** Refits tracks without PXD hits and calculate the residuals for the PXD layers */
    void event() override final;

    /** actually defines the histograms*/
    void defineHisto() override final;

  private:

    std::string m_histogramDirectoryName;  /**< Name of the directory where the histogram will be placed */
    std::string m_outputRecoTracksArrayName{"NoPXDRecoTracks"};  /**< Name of the StoreArray with the recoTracks without PXD hits */
    std::string m_particleListName{""};  /**< Name of the particle list to which the module is applied */


    StoreObjPtr<ParticleList>
    m_ParticleList; /**< particle list on which run the module (need to be charged particles to have some results)*/
    StoreArray<RecoTrack> m_recoTracks;/**< store array of the recoTracks related to the particles selected with m_ParticleList*/
    StoreArray<RecoTrack> m_outputRecoTracks; /**< tracks without the PDX hits*/

    Double_t m_cutResU; /**<value fot the cut on absolute value of the u residuals*/
    Double_t m_cutP; /**< value fot the cut on the momentum of the particle*/
    Double_t m_cutD0; /**< value fot the cut on absolute value of the track parameter d0*/
    Double_t m_cutZ0; /**< value fot the cut on absolute value of the track parameter z0*/

    Double_t m_rangeV; /**< range for the histos of the v residuls*/
    Double_t m_rangeS; /**< range for the histos of the sagitta*/
    int m_binsV; /**< number of bins for the histograms of the v residuals*/
    int m_binsS; /**< number of bins for the histograms of the sagitta*/

    std::map<VxdID, Double_t> m_dwAlignment; /**< Bowing amplitude of the PXD modules inside the alignment */

    std::map<VxdID, TH1F*> m_hResV;/**< histos for the residual of the PXD hits' v component for each PXD module*/
    std::map<VxdID, TH1F*> m_hSagitta;/**< histos of the sagitta for each PXD module*/
  };
}
