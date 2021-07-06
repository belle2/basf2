/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>

namespace Belle2 {

  /*! Class to store mapping between cell id and store array positions
  */

  class ECLCellIdMapping : public TObject {
  public:

    /** Number of ECL CellId */
    static constexpr int c_nECLCellIds = 8736;

    /**
    * Default constructor.
    */
    ECLCellIdMapping() :
      m_CellIdToStoreArrPosition(c_nECLCellIds + 1),
      m_CellIdToPhi(c_nECLCellIds + 1),
      m_CellIdToTheta(c_nECLCellIds + 1),
      m_CellIdToPhiId(c_nECLCellIds + 1),
      m_CellIdToThetaId(c_nECLCellIds + 1),
      m_CellIdToNeighbours5(c_nECLCellIds + 1),
      m_CellIdToNeighbours7(c_nECLCellIds + 1)
    {
      for (unsigned idx = 0; idx < c_nECLCellIds + 1; idx++) {
        m_CellIdToStoreArrPosition[idx] = -1;
      }
    }

    /** Set celld id to store array*/
    void setCellIdToStoreArray(const int& cellid, const int& idx)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        m_CellIdToStoreArrPosition[cellid] = idx;
      } else {
        B2ERROR("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Set celld id to neighbour5*/
    void setCellIdToNeighbour5(const int& cellid, const std::vector<short int>& neighbours)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        m_CellIdToNeighbours5[cellid] = neighbours;
      } else {
        B2ERROR("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Set celld id to neighbour7*/
    void setCellIdToNeighbour7(const int& cellid, const std::vector<short int>& neighbours)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        m_CellIdToNeighbours7[cellid] = neighbours;
      } else {
        B2ERROR("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Set celld id to phi*/
    void setCellIdToPhi(const int& cellid, const double& phi)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        m_CellIdToPhi[cellid] = phi;
      } else {
        B2ERROR("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Set celld id to theta*/
    void setCellIdToTheta(const int& cellid, const double& theta)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        m_CellIdToTheta[cellid] = theta;
      } else {
        B2ERROR("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Set celld id to phi*/
    void setCellIdToPhiId(const int& cellid, const int& phiid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        m_CellIdToPhiId[cellid] = phiid;
      } else {
        B2ERROR("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Set celld id to theta*/
    void setCellIdToThetaId(const int& cellid, const int& thetaid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        m_CellIdToThetaId[cellid] = thetaid;
      } else {
        B2ERROR("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Reset store array*/
    void reset()
    {
      memset(&m_CellIdToStoreArrPosition[0], -1, m_CellIdToStoreArrPosition.size() * sizeof m_CellIdToStoreArrPosition[0]);
    }

    /** Get store array from cell id */
    int getCellIdToStoreArray(const int& cellid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        return m_CellIdToStoreArrPosition[cellid];
      } else {
        B2FATAL("Cell Id " << cellid << " does not exist.");
        return -1;
      }
    }

    /** Get store array from cell id */
    std::vector<short int>& getCellIdToNeighbour5(const int& cellid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        return m_CellIdToNeighbours5[cellid];
      } else {
        B2FATAL("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Get store array from cell id */
    std::vector<short int>& getCellIdToNeighbour7(const int& cellid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        return m_CellIdToNeighbours7[cellid];
      } else {
        B2FATAL("Cell Id " << cellid << " does not exist.");
      }
    }

    /** Get phi from cell id */
    double getCellIdToPhi(const int& cellid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        return m_CellIdToPhi[cellid];
      } else {
        B2FATAL("Cell Id " << cellid << " does not exist.");
        return -1;
      }
    }

    /** Get theta from cell id */
    double getCellIdToTheta(const int& cellid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        return m_CellIdToTheta[cellid];
      } else {
        B2FATAL("Cell Id " << cellid << " does not exist.");
        return -1;
      }
    }


    /** Get phi from cell id */
    int getCellIdToPhiId(const int& cellid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        return m_CellIdToPhiId[cellid];
      } else {
        B2FATAL("Cell Id " << cellid << " does not exist.");
        return -1;
      }
    }

    /** Get theta from cell id */
    int getCellIdToThetaId(const int& cellid)
    {
      if (cellid > 0 and cellid < c_nECLCellIds + 1) {
        return m_CellIdToThetaId[cellid];
      } else {
        B2FATAL("Cell Id " << cellid << " does not exist.");
        return -1;
      }
    }


  private:

    /** vector (8736+1 entries) with cell id to store array positions */
    std::vector<int> m_CellIdToStoreArrPosition;

    /** vector (8736+1 entries) with cell id to phi values  */
    std::vector<double> m_CellIdToPhi;

    /** vector (8736+1 entries) with cell id to phi values  */
    std::vector<double> m_CellIdToTheta;

    /** vector (8736+1 entries) with cell id to phi values  */
    std::vector<int> m_CellIdToPhiId;

    /** vector (8736+1 entries) with cell id to phi values  */
    std::vector<int> m_CellIdToThetaId;

    /** vector (8736+1 entries) with cell id to 5x5 neighbour vector */
    std::vector<std::vector<short int>> m_CellIdToNeighbours5;

    /** vector (8736+1 entries) with cell id to 7x7 neighbour vector */
    std::vector<std::vector<short int>> m_CellIdToNeighbours7;

    ClassDef(ECLCellIdMapping, 1); /**< ClassDef */
  };

} // end namespace Belle2
