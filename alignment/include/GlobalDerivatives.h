/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <root/TMatrixD.h>

#include <vector>

namespace Belle2 {
  namespace alignment {
    /// Class for easier manipulation with global derivatives (and their labels)
    ///
    /// Allows to simply add, merge and pass global derivatives matrix and
    /// global label vector in RecoHit.
    /// For lables = 0 removes given columns from derivative matrix
    /// (e.g. save disc space for parameters you do not want to calibrate)
    class GlobalDerivatives {
    public:
      /// Constructor for empty derivative matrix and label vector
      /// @param dim number of matrix rows (number of local residuals in virtual plane)
      /// Usually 2 (also for 1 dim measurement, u/v selected via precision matrix)
      explicit GlobalDerivatives(int dim = 2) {m_globals.second.ResizeTo(dim, 0);}
      /// Constructor from pair of the vector and the matrix
      explicit GlobalDerivatives(const std::pair<std::vector<int>, TMatrixD>& globals);
      /// constructor from label vector and derivative matrix
      GlobalDerivatives(const std::vector<int>& labels, const TMatrixD& derivs);
      /// Convenient operator to allow to pass the object directly from RecoHit to globalDerivatives()
      /// WARNING: causes zero labels to be removed
      operator std::pair<std::vector<int>, TMatrixD>() {return passGlobals(m_globals);}
      /// Convenient operator to pass only labels (non-zero)
      operator std::vector<int>() {return passGlobals(m_globals).first;}
      /// Covenient operator to pass only derivatives (for non-zero labels)
      operator TMatrixD() {return passGlobals(m_globals).second;}
      /// Get stored lables (includes zeros)
      const std::vector<int>& getLabels() const {return m_globals.first;}
      /// Return the derivative matrix (includes columns with zero labels)
      const TMatrixD& getDerivatives() const {return m_globals.second;}
      /// Add another set of global labels and derivatives
      void add(const std::pair<std::vector<int>, TMatrixD>& globals);
      /// Add one parameter - label and the corresponding residual derivative
      /// @param paramLabel label of the global parameter to calibrate
      /// @param dResiduals_dParam vector od derivatives of local residual (U, v) versus global parameter
      void add(int paramLabel, std::vector<double> dResiduals_dParam);
      /// Add derivative of local U residual w.r.t. global parameter
      /// Global derivative versus V residual is set to zero.
      /// For (u, v) measurement use add(p, {dru/dp, drv/dp}.
      void add(int paramLabel, double drudp);
      /// Static convenient function to remove columns with zero labels (make error in Pede btw.)
      /// TODO: refactore interface: return value, params <->
      static std::pair<std::vector<int>, TMatrixD> passGlobals(std::pair<std::vector<int>, TMatrixD> globals);
    private:
      /// The global labels and derivatives matrix
      std::pair<std::vector<int>, TMatrixD> m_globals {{}, TMatrixD()};
    };
  }
}
