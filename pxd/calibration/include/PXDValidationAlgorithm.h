/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vxd/dataobjects/VxdID.h>
#include <vector>

#include <TH1F.h>
#include <TH2F.h>


namespace Belle2 {
  /**
   * Class implementing the PXD calibration validation algorithm
   */
  class  PXDValidationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDValidationAlgorithm
    PXDValidationAlgorithm();

    /// Destructor
    virtual ~PXDValidationAlgorithm() override;

    /// Minimum number of track points per sensor
    int minTrackPoints;

    /// Flag to save 2D histograms for efficiency;
    bool save2DHists;

    /// Setter for boundaries
    void setBoundaries(const std::vector<Calibration::ExpRun>& boundaries) {m_requestedBoundaries = boundaries;}

    /// getter for current boundaries
    std::vector<Calibration::ExpRun> getBoundaries() const {return m_requestedBoundaries;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

    /// Decide if a run should be a payload boundary. Only used in certain Python Algorithm Starategies.
    virtual bool isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/) override;

    /// What we do before isBoundaryRequired gets called.
    //  We reset the temporary boundaries list to be the same as the requested one.
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /* iteration = 0*/) override
    {
      m_boundaries = m_requestedBoundaries;
    };

  private:

    /// During findPayloadBoundaries and isBoundaryRequired this is used to define the boundaries we want.
    std::vector<Calibration::ExpRun> m_requestedBoundaries;

    /// Pointer for TFile
    std::shared_ptr<TFile> m_file;

    /// Pointer for TTree of the validation info
    std::shared_ptr<TTree> m_tree;

    /// Experiment number
    int m_exp;

    /// Run number
    int m_run;

    /// Vector of PXD module id (DHE id)
    std::vector<unsigned short> m_pxdid;

    /// Vecotr of uBin
    std::vector<unsigned short> m_uBin;

    /// Vecotr of vBin
    std::vector<unsigned short> m_vBin;

    /// Vecotr of number of track points
    std::vector<unsigned long> m_nTrackPoints;

    /// Vecotr of number of track matched clusters
    std::vector<unsigned long> m_nTrackClusters;

    /// Histogram of corrected d0 for each 2-track event
    TH1F* m_hD0;

    /// Histogram of corrected z0 for each 2-track event
    TH1F* m_hZ0;

    /// Histogram of intersection points for layer 1
    TH2F* m_hTrackPointsLayer1;

    /// Histogram of track matched clusters for layer 1
    TH2F* m_hTrackClustersLayer1;

    /// Histogram of intersection points for layer 2
    TH2F* m_hTrackPointsLayer2;

    /// Histogram of track matched clusters for layer 2
    TH2F* m_hTrackClustersLayer2;

  };
} // namespace Belle2


