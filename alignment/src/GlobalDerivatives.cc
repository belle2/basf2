/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/GlobalDerivatives.h>

namespace Belle2 {
  namespace alignment {
    GlobalDerivatives::GlobalDerivatives(std::pair< std::vector< int >, TMatrixD > globals)
    {
      m_globals.second.ResizeTo(globals.second);
      m_globals.first = globals.first;
      m_globals.second = globals.second;
    }
    GlobalDerivatives::GlobalDerivatives(std::vector< int > labels, TMatrixD derivs)
    {
      m_globals.second.ResizeTo(derivs);
      m_globals.first = labels;
      m_globals.second = derivs;
    }
    void GlobalDerivatives::add(const std::pair<std::vector<int>, TMatrixD>& globals)
    {
      if (globals.first.empty())
        return;

      auto& main = m_globals;

      // Create composed matrix of derivatives
      //TODO: check main and globals matrix has the same number of rows
      TMatrixD allDerivatives(main.second.GetNrows(), main.second.GetNcols() + globals.second.GetNcols());
      allDerivatives.Zero();
      allDerivatives.SetSub(0, 0, main.second);
      allDerivatives.SetSub(0, main.second.GetNcols(), globals.second);

      // Merge labels
      main.first.insert(main.first.end(), globals.first.begin(), globals.first.end());
      // Update matrix
      main.second.ResizeTo(allDerivatives);
      main.second = allDerivatives;

    }
    void GlobalDerivatives::add(int paramLabel, std::vector< double > dResiduals_dParam)
    {
      int nRows = m_globals.second.GetNrows();
      int nCols = m_globals.second.GetNcols();

      m_globals.first.push_back(paramLabel);
      m_globals.second.ResizeTo(nRows, nCols + 1);
      for (int iRow = 0; iRow < nRows; ++iRow) {
        m_globals.second(iRow, nCols) = dResiduals_dParam.at(iRow);
      }
    }
    void GlobalDerivatives::add(int paramLabel, double drudp)
    {
      std::vector<double> dResiduals_dParam(m_globals.second.GetNrows(), 0.);
      dResiduals_dParam.at(0) = drudp;
      add(paramLabel, dResiduals_dParam);
    }
    std::pair< std::vector< int >, TMatrixD > GlobalDerivatives::passGlobals(std::pair< std::vector< int >, TMatrixD > globals)
    {
      TMatrixD newMatrix(globals.second.GetNrows(), 0);
      std::vector<int> newLabels;

      for (unsigned int iOldCol = 0; iOldCol < globals.first.size(); ++iOldCol) {
        auto label = globals.first.at(iOldCol);
        if (label == 0)
          continue;

        newLabels.push_back(label);
        newMatrix.ResizeTo(globals.second.GetNrows(), newMatrix.GetNcols() + 1);
        for (int iRow = 0; iRow < globals.second.GetNrows(); ++iRow) {
          newMatrix(iRow, newMatrix.GetNcols() - 1) = globals.second(iRow, iOldCol);
        }
      }
      return {newLabels, newMatrix};
    }
  }
}