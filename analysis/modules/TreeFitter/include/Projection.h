/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Define a class to store projection matrix, covariance matrix and residuals

#ifndef PROJECTION_H
#define PROJECTION_H

#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/Matrix.h>

#include <string.h> //fix for memset (FT)

//using namespace CLHEP;
//using CLHEP::HepVector;
//using CLHEP::HepSymMatrix;
//using CLHEP::HepMatrix;

namespace TreeFitter {

  class Projection {
  public:
    // constructor
    Projection(int dimP, int dimC) : m_matrixH(dimC, dimP), m_r(dimC), m_matrixV(dimC), m_offset(0) {}

    // accessors to the projection matrix
    const CLHEP::HepMatrix& H() const { return m_matrixH;}
    //CLHEP::HepMatrix& H() { return _matrixH ; }
    double& H(int row, int col)
    {
      //#ifdef VTK_BOUNDSCHECKING
      //      assert( _offset+row >0 && col>0 && _offset+row <= _matrixH.num_row() && col <= _matrixH.num_col() ) ;
      //#endif
      return m_matrixH(m_offset + row, col) ;
    }

    // accessors to the residual (or the 'value' of the constraint)
    const CLHEP::HepVector& r() const { return m_r ; }
    CLHEP::HepVector& r() { return m_r ; }
    double& r(int row)
    {
      //#ifdef VTK_BOUNDSCHECKING
      //      assert( _offset+row >0  && _offset+row <= _r.num_row() ) ;
      //#endif
      return m_r(m_offset + row) ;
    }

    // accessors to the covariance matrix
    const CLHEP::HepSymMatrix& V() const { return m_matrixV ; }
    //CLHEP::HepSymMatrix& V() { return m_matrixV ; }
    double& V(int row, int col) { return m_matrixV(m_offset + row, m_offset + col); }

    double& Vfast(int row, int col)
    {
      //#ifdef VTK_BOUNDSCHECKING
      //      assert( _offset+row >0 && _offset+col>0 && _offset+row <= _matrixV.num_row() && _offset+col <= _matrixV.num_col() && row>=col ) ;
      //#endif
      return m_matrixV.fast(m_offset + row, m_offset + col) ;
    }

    // reset
    void reset()
    {
      // fill everything with '0'.  this implementation is kind of
      // tricky, but it is fast.

      int dimC = m_matrixH.num_row() ;
      int dimP = m_matrixH.num_col() ;

      //      std::cout << "Projection::reset(): resetting H(" << dimC << "," << dimP << ")" << std::endl << std::flush;
      memset(&(m_matrixH(1, 1)), 0, dimP * dimC * sizeof(double));
      memset(&(m_r(1))  , 0, dimC * sizeof(double));
      memset(&(m_matrixV(1, 1)), 0, dimC * (dimC + 1) / 2 * sizeof(double));
      m_offset = 0 ;
    }

    // globalChisq
    double chiSquare() const
    {
      CLHEP::HepSymMatrix W = m_matrixV ;
      int ierr;  W.inverse(ierr) ;
      return W.similarity(m_r) ;
    }

    void incrementOffset(unsigned int i) { m_offset += i ; }
    unsigned int offset() const { return m_offset ; }

  private:
    CLHEP::HepMatrix m_matrixH ;    // projection matrix
    CLHEP::HepVector m_r ;    // constraint residual
    CLHEP::HepSymMatrix m_matrixV ; // constraint variance (zero for lagrange constraints)
    unsigned int m_offset ; // offset for constraint index. only non-zero for merged constraints.
  } ;
}
#endif
