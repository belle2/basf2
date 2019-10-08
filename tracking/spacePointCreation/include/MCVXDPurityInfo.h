/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// stl:
#include <array>
#include <utility> // std::pair
#include <numeric>      // std::accumulate
#include <sstream>
#include <string>

namespace Belle2 {
  /** The MC VXD Purity info container class
   *
   * This class stores for a particleID the number of clusters found in a TrackCandidate container.
   *
   */
  class MCVXDPurityInfo {
  protected:
    /** stores the particleID*/
    int m_iD;

    /** stores the number of clusters the TrackCandidate container had.
     * (no matter which particleID was connected to the clusters)
     * for each ClusterType, PXD in [0], SVD-U in [1], and SVD-V in [2] */
    std::array<unsigned int, 3> m_nTotalClusters;

    /** stores the number for each ClusterType, PXD in [0], SVD-U in [1], and SVD-V in [2] */
    std::array<unsigned int, 3> m_nFoundClusters;

  public:

    /** empty constructor */
    MCVXDPurityInfo() :
      m_iD(-1),
      m_nTotalClusters( { {0, 0, 0} }),
    m_nFoundClusters({ {0, 0, 0} }) {}

    /** constructor
     * parameters:
     * iD, the particleID
     * nPXDClustersTotal, the number of PXDClusters the TrackCandidate container had (no matter which particleID was connected to the clusters)
     * nSVDUClustersTotal, the number of SVDClusters for u-strips the TrackCandidate container had (no matter which particleID was connected to the clusters)
     * nSVDVClustersTotal, the number of SVDClusters for v-strips the TrackCandidate container had (no matter which particleID was connected to the clusters)
     * nPXDClusters, the number of PXDClusters connected to iD
     * nSVDUClusters, the number of SVDClusters for u-strips connected to iD
     * nSVDVClusters, the number of SVDClusters for v-strips connected to iD
     * */
    MCVXDPurityInfo(int iD,
                    unsigned int nPXDClustersTotal,
                    unsigned int nSVDUClustersTotal,
                    unsigned int nSVDVClustersTotal,
                    unsigned int nPXDClusters,
                    unsigned int nSVDUClusters,
                    unsigned int nSVDVClusters) :
      m_iD(iD),
      m_nTotalClusters( { {nPXDClustersTotal, nSVDUClustersTotal, nSVDVClustersTotal} }),
    m_nFoundClusters({ {nPXDClusters, nSVDUClusters, nSVDVClusters} }) {}

    /**
     * constructor
     * @param iD: particle Id
     * @param nClustersTotal: the total number of Clusters in the container as array ([0] is PXDClusters, [1]/[2] are SVD U-/V- Clusters)
     * @param nMCClusters: the number of Clusters in the container connectoed to iD as array
     */
    MCVXDPurityInfo(int iD, std::array<unsigned int, 3> nClustersTotal, std::array<unsigned int, 3> nMCClusters) :
      m_iD(iD), m_nTotalClusters(nClustersTotal), m_nFoundClusters(nMCClusters) { }

    /** operator for sorting.
      *
      * will be compared using overal purity
      * NOTE: only the purity will be compared (i.e. .second), the iD is not used!
      * */
    inline bool operator > (const MCVXDPurityInfo& b) const
    {
      return getPurity().second > b.getPurity().second;
    }


    /** operator for sorting.
     *
     * will be compared using overal purity
     * NOTE: only the purity will be compared (i.e. .second), the iD is not used!
     * */
    inline bool operator < (const MCVXDPurityInfo& b) const
    {
      return getPurity().second < b.getPurity().second;
    }


    /** getter - returns overal purity (.second) for this particleID (.first).
    *
    * since PXD-Clusters are counting as two measurements, they count twice.
    */
    std::pair<int, float> getPurity() const
    {
      unsigned int nTotal = std::accumulate(m_nTotalClusters.begin() + 1, m_nTotalClusters.end(), 0) + 2 * m_nTotalClusters[0];
      unsigned int nFound = std::accumulate(m_nFoundClusters.begin() + 1, m_nFoundClusters.end(), 0) + 2 * m_nFoundClusters[0];
      return {m_iD, (nTotal == 0 ? 0.f : float(nFound) / float(nTotal)) };
    }


    /** getter - returns purity for PXDClusters (.second) for this particleID (.first) */
    std::pair<int, float> getPurityPXD() const
    {
      return {m_iD, (m_nTotalClusters[0] == 0 ? 0.f : float(m_nFoundClusters[0]) / float(m_nTotalClusters[0])) };
    }


    /** getter - returns purity for SVDClusters (.second) for this particleID (.first) */
    std::pair<int, float> getPuritySVD() const
    {
      unsigned int nTotal = std::accumulate(m_nTotalClusters.begin() + 1, m_nTotalClusters.end(), 0);
      unsigned int nFound = std::accumulate(m_nFoundClusters.begin() + 1, m_nFoundClusters.end(), 0);
      return {m_iD, (nTotal == 0 ? 0.f : float(nFound) / float(nTotal)) };
    }


    /** getter - returns purity for SVDClusters of u-type (.second) for this particleID (.first) */
    std::pair<int, float> getPuritySVDU() const
    {
      return {m_iD, (m_nTotalClusters[1] == 0 ? 0.f : float(m_nFoundClusters[1]) / float(m_nTotalClusters[1])) };
    }


    /** getter - returns purity for SVDClusters of v-type (.second) for this particleID (.first) */
    std::pair<int, float> getPuritySVDV() const
    {
      return {m_iD, (m_nTotalClusters[2] == 0 ? 0.f : float(m_nFoundClusters[2]) / float(m_nTotalClusters[2])) };
    }


    /** getter - returns the ID of the particle, if value is -1 no particle has been able to be found for it */
    int getParticleID() const { return m_iD; }


    /** returns total number of clusters the trackCandidate had */
    unsigned int getNClustersTotal() const { return std::accumulate(m_nTotalClusters.begin(), m_nTotalClusters.end(), 0); }


    /** getter - returns total number of PXDClusters in the TrackCandidate */
    unsigned int getNPXDClustersTotal() const { return m_nTotalClusters[0]; }


    /** getter - returns total number of u-type SVDClustes in the TrackCandidate */
    unsigned int getNSVDUClustersTotal() const { return m_nTotalClusters[1]; }


    /** getter - returns total number of v-type SVDClustes in the TrackCandidate */
    unsigned int getNSVDVClustersTotal() const { return m_nTotalClusters[2]; }


    /** returns number of clusters the trackCandidate had assigned to this iD */
    unsigned int getNClustersFound() const { return std::accumulate(m_nFoundClusters.begin(), m_nFoundClusters.end(), 0); }


    /** getter - returns number of PXDClusters found to this iD */
    unsigned int getNPXDClusters() const { return m_nFoundClusters[0]; }


    /** getter - returns number of u-type SVDClustes found to this iD */
    unsigned int getNSVDUClusters() const { return m_nFoundClusters[1]; }


    /** getter - returns number of v-type SVDClustes found to this iD */
    unsigned int getNSVDVClusters() const { return m_nFoundClusters[2]; }


    /** getter - returns number of degrees of freedom for all Clusters in TC total */
    unsigned int getNDFTotal() const { return std::accumulate(m_nTotalClusters.begin() + 1, m_nTotalClusters.end(), 0) + 2 * m_nTotalClusters[0]; }


    /** getter - returns number of degrees of freedom for all Clusters in TC found */
    unsigned int getNDFFound() const { return std::accumulate(m_nFoundClusters.begin() + 1, m_nFoundClusters.end(), 0) + 2 * m_nFoundClusters[0]; }


    /** getter - returns number of degrees of freedom for PXD Clusters in TC total */
    unsigned int getNDFPXDTotal() const { return  2 * m_nTotalClusters[0]; }


    /** getter - returns number of degrees of freedom for PXD Clusters in TC found */
    unsigned int getNDFPXDFound() const { return  2 * m_nFoundClusters[0]; }


    /** getter - returns number of degrees of freedom for SVD Clusters in TC total */
    unsigned int getNDFSVDTotal() const { return std::accumulate(m_nTotalClusters.begin() + 1, m_nTotalClusters.end(), 0); }


    /** getter - returns number of degrees of freedom for SVD Clusters in TC found */
    unsigned int getNDFSVDFound() const { return std::accumulate(m_nFoundClusters.begin() + 1, m_nFoundClusters.end(), 0); }


    /** getter - returns number of degrees of freedom for u-type SVDClustes in the TrackCandidate */
    unsigned int getNDFSVDUTotal() const { return getNSVDUClustersTotal(); }


    /** getter - returns number of degrees of freedom for v-type SVDClustes in the TrackCandidate */
    unsigned int getNDFSVDVTotal() const { return getNSVDVClustersTotal(); }


    /** getter - returns number of degrees of freedom for u-type SVDClustes found to this iD */
    unsigned int getNDFSVDUFound() const { return getNSVDUClusters(); }


    /** getter - returns number of degrees of freedom for v-type SVDClustes found to this iD */
    unsigned int getNDFSVDVFound() const { return getNSVDVClusters(); }


    /** dump the contents to a string (for easier debugging) */
    std::string dumpToString() const
    {
      std::stringstream s;
      s << "iD: " << m_iD << "\n";
      for (size_t i = 0; i < m_nTotalClusters.size(); ++i) {
        s << "totalClusters[" << i << "]: " << m_nTotalClusters[i] << ", foundClusters[" << i << "]: " << m_nFoundClusters[i] << "\n";
      }
      return s.str();
    }
  };
}
