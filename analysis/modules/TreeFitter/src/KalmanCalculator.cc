/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Calculate Kalman gain matrix, update parameters and covariance matrix

#include <analysis/modules/TreeFitter/KalmanCalculator.h>

#include <iostream> //FT: gradually phase out in favor of basf2 messaging

#define SLOWBUTSAFE 1
//#undef SLOWBUTSAFE
#undef SKIPHIGHACCURACYCORRECTION

namespace TreeFitter {

  //  inline double fastsymmatrixaccess(double* m, int row, int col)
  //  {
  //    return *(m+(row*(row-1))/2+(col-1));
  //  }

  //  inline double symmatrixaccess(double* m, int row, int col)
  //  {
  //    return (row>=col? fastsymmatrixaccess(m,row,col) : fastsymmatrixaccess(m,col,row)) ;
  //  }

  ErrCode KalmanCalculator::init(const HepVector& value, const HepMatrix& G,
                                 const FitParams* fitparams, const HepSymMatrix* V, int weight)
  {
    ErrCode status ;
    m_nconstraints = value.num_row() ;  // dimension of the constraint
    m_nparameters  = fitparams->dim() ; // dimension of the state

#ifndef SLOWBUTSAFE
    int valdim  = value.num_row() ; // dimension of the constraint
    int statdim = fitparams->par().num_row() ; // dimension of the state
#endif

    //#ifdef VTK_BOUNDSCHECKING
    //    assert( G.num_row() == valdim && G.num_col() == statdim &&
    //      (!V || V->num_row()==valdim) ) ;
    //#endif
    m_value = &value ;
    m_matrixG     = &G ;
    const HepSymMatrix& C = fitparams->cov() ;
    // calculate C*G.T()
#ifdef SLOWBUTSAFE
    m_matrixCGT = C * G.T() ;
    //    m_matrixCGT = C * TMatrixD(TMatrixD::kTransposed, G); old Root implementation
#else
    //FT: not using this
    double tmp ;
    m_matrixCGT = HepMatrix(statdim, valdim, 0) ;
    for (int col = 1; col <= _nconstraints; ++col)
      for (int k = 1; k <= _nparameters; ++k)
        if ((tmp = G(col, k)) != 0) {
          for (int row = 1; row < k; ++row)
            _matrixCGT(row, col) += C.fast(k, row) * tmp ;
          for (int row = k; row <= statdim; ++row)
            _matrixCGT(row, col) += C.fast(row, k) * tmp ;
        }
#endif

    // calculate the error in the predicted residual R = G*C*GT + V
    // slow:
#ifdef SLOWBUTSAFE
    m_matrixRinv = fitparams->cov().similarity(G);
    if (V) m_matrixRinv += weight * (*V);
#else
    //FT: not using this
    if (V) {
      _matrixRinv = *V ;
      if (weight != 1) _matrixRinv *= weight ;
    } else _matrixRinv = HepSymMatrix(valdim, 0) ;

    for (int row = 1; row <= _nconstraints; ++row)
      for (int k = 1; k <= _nparameters; ++k)
        if ((tmp = G(row, k)) != 0)
          for (int col = 1; col <= row; ++col)
            _matrixRinv.fast(row, col) += tmp * _matrixCGT(k, col) ;
#endif
    m_matrixR = m_matrixRinv ;
    m_matrixRinv.invert(m_ierr) ;//could be InvertFast
    if (m_ierr) {
      status |= ErrCode::inversionerror;
      std::cout << "Error inverting matrix. Vertex fit fails." << std::endl;
    }

    // calculate the gain matrix
    m_matrixK = m_matrixCGT * m_matrixRinv ;
    m_chisq = -1 ;
//     // let's see if we get same results using sparce matrices
//     VtkSparseMatrix Gs(G) ;
//     VtkSparseMatrix CGT = Gs.transposeAndMultiplyRight(fitparams->cov()) ;
//     HepSymMatrix Rs(value.numrow()) ;
//     Gs.multiplyLeft(CGT,Rs) ;
//     if(V) Rs += (*V) ;
//     Rs.invert(_ierr) ;
//     VtkSparseMatrix Ks = CGT*Rs ;
    return status ;
  }

  void KalmanCalculator::updatePar(FitParams* fitparams)
  {
    //fitparams->par() -= fitparams->cov() * (G.T() * (R * value) ) ;
    fitparams->par() -= m_matrixK * (*m_value) ;
    m_chisq = m_matrixRinv.similarity(*m_value);
  }

  void KalmanCalculator::updatePar(const HepVector& pred, FitParams* fitparams)
  {
    // this is still very, very slow !
    HepVector valueprime = (*m_value) + (*m_matrixG) * (pred - fitparams->par()) ;
    fitparams->par() = pred - m_matrixK * valueprime ;
    m_chisq = m_matrixRinv.similarity(valueprime);
  }

  void KalmanCalculator::updateCov(FitParams* fitparams, double chisq)
  {
#ifdef SLOWBUTSAFE
    HepSymMatrix deltaCov = m_matrixRinv.similarityT(*m_matrixG).similarity(fitparams->cov()) ;
    fitparams->cov() -= deltaCov ;
#else

    // There are two expressions for updating the covariance
    // matrix.
    // slow: deltaCov = - 2*C*GT*KT +  K*R*KT
    // fast: deltaCov = - C*GT*KT
    // The fast expression is very sensitive to machine accuracy. The
    // following piece of code effectively invokes the slow
    // expression. I couldn't write it faster than this.

    double tmp ;
#ifndef SKIPHIGHACCURACYCORRECTION
    // substitute C*GT --> 2*C*GT - K*R. of course, this invalidates
    // C*GT, but we do not need it after this routine.

    // we use the fact that _in principle_ C*GT = K*R, such that
    // they have the same zero elements
    for (int row = 1; row <= m_nparameters; ++row)
      for (int col = 1; col <= m_nconstraints; ++col)
        if ((tmp = 2 * m_matrixCGT(row, col)) != 0) {
          for (int k = 1; k <= m_nconstraints; ++k)
            tmp -= m_matrixK(row, k) * m_matrixR(k, col) ;
          m_matrixCGT(row, col) = tmp ;
        }
#endif

//     HepMatrix KR = _matrixK*_matrixR ;
//     double tmp ;
//     for(int row=1; row<=_nparameters; ++row)
//       for(int k=1; k<=_nconstraints; ++k)
//  if( (tmp= (KR(row,k) - 2*_matrixCGT(row,k))) != 0 )
//    for(int col=1; col<=row; ++col)
//      fitparams->cov().fast(row,col) += tmp * _matrixK(col,k) ;

    // deltaCov = - C*GT*KT
    for (int row = 1; row <= m_nparameters; ++row)
      for (int k = 1; k <= m_nconstraints; ++k)
        if ((tmp = -(m_matrixCGT(row, k))) != 0)  // they have same size, and same 'emptiness'
          for (int col = 1; col <= row; ++col)
            fitparams->cov().fast(row, col) += tmp * m_matrixK(col, k) ;

#endif
    fitparams->addChiSquare(chisq > 0 ? chisq : m_chisq, m_value->num_row()) ;
    for (int col = 1; col <= m_nconstraints; ++col)
      for (int k = 1; k <= m_nparameters; ++k)
        if ((*m_matrixG)(col, k) != 0) ++(fitparams->nConstraintsVec(k)) ;
  }
}
