/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : CosmicRayHLTDQMModule.h
// Description : Module to monitor the reconstruction of cosmic ray on hlt
//-

#pragma once

#include <string>

#include <TH1F.h>

#include <framework/core/HistoModule.h>

namespace Belle2 {
  namespace SoftwareTrigger {

    /*! A class definition of an input module for Sequential ROOT I/O */

    class CosmicRayHLTDQMModule : public HistoModule {

      // Public functions
    public:

      //! Constructor / Destructor
      CosmicRayHLTDQMModule();
      virtual ~CosmicRayHLTDQMModule() override;

      //! Module functions to be called from main process
      virtual void initialize() override;

      //! Module functions to be called from event process
      virtual void event() override;

      //! Histogram definition
      virtual void defineHisto() override;


    private:
      /// Directory to put the generated histograms
      std::string m_param_histogramDirectoryName;

      //CDC
      /**Signed distance to the POCA in the r-phi plane*/
      TH1F* h_d0 = nullptr;
      /**z coordinate of the POCA*/
      TH1F* h_z0 = nullptr;
      /**number of traks*/
      TH1F* h_ntrk = nullptr;
      /**momentum information of track*/
      TH1F* h_p[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
      /**chi2 probalility of the track fit*/
      TH1F* h_pValue = nullptr;
      /**Number of CDC hits associated to CDC track*/
      TH1F* h_ncdchits = nullptr;
      /**Angle of the transverse momentum in the r-phi plane*/
      TH1F* h_phi0 = nullptr;
      /**charge of track*/
      TH1F* h_charge = nullptr;

      //ECL
      /**number of ecl clusters*/
      TH1F* h_ncluster = nullptr;
      /**phi angle of ECLCluster position*/
      TH1F* h_phi_eclcluster = nullptr;
      /**theta angle of ECLCluster position*/
      TH1F* h_theta_eclcluster = nullptr;
      /**the ecl cluster time*/
      TH1F* h_Time_eclcluster = nullptr;
      /**the E1/E9 energy ratio*/
      TH1F* h_E1oE9_eclcluster = nullptr;
      /**energy of ECL cluster*/
      TH1F* h_e_eclcluster = nullptr;

      /**number of ECL showers*/
      TH1F* h_nshower = nullptr;
      /**time of  ECL shower*/
      TH1F* h_time_eclshower = nullptr;
      /**energy of ECL shower*/
      TH1F* h_e_eclshower = nullptr;

      //KLM
      /**number of bKLM Hit*/
      TH1F* h_nbklmhit = nullptr;
      /**layer ID of hits in bKLM*/
      TH1F* h_layerId_bklmhit = nullptr;
      /**sector ID of hits in bKLM*/
      TH1F* h_sectorId_bklmhit = nullptr;
      /**number of eKLM Hit*/
      TH1F* h_neklmhit = nullptr;
      /**layer ID of hits in eKLM*/
      TH1F* h_layerId_eklmhit = nullptr;
      /**sector ID of hits in eKLM*/
      TH1F* h_sectorId_eklmhit = nullptr;


    };

  } // end namespace SoftwareTrigger
} // end namespace Belle2
