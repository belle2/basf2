#include <tracking/trackFitting/measurementCreator/measurements/HMatrixQP.h>
#include <TMatrixT.h>                   // for TMatrixT
#include <TMatrixTSym.h>                // for TMatrixTSym
#include <TVectorT.h>                   // for TVectorT
#include <cassert>                      // for assert
#include <iostream>                     // for operator<<, basic_ostream, etc

using namespace genfit;

namespace Belle2 {

  const TMatrixD& HMatrixQP::getMatrix() const
  {
    static const double HMatrixContent[5] = {1, 0, 0, 0, 0};

    static const TMatrixD HMatrix(1, 5, HMatrixContent);

    return HMatrix;
  }


  TVectorD HMatrixQP::Hv(const TVectorD& v) const
  {
    assert(v.GetNrows() == 5);

    TVectorD returnValue(1);

    returnValue(0) = v(0);

    return returnValue;
  }


  TMatrixD HMatrixQP::MHt(const TMatrixDSym& M) const
  {
    assert(M.GetNcols() == 5);

    TMatrixD returnVector(5, 1);

    for (unsigned int i = 0; i < 5; ++i) {
      returnVector(i, 0) = M(0, i);
    }

    return returnVector;
  }


  TMatrixD HMatrixQP::MHt(const TMatrixD& M) const
  {
    assert(M.GetNcols() == 5);

    TMatrixD returnMatrix(M.GetNrows(), 1);

    for (int i = 0; i < M.GetNrows(); ++i) {
      returnMatrix(i, 0) = M(0, i);
    }

    return returnMatrix;
  }


  void HMatrixQP::HMHt(TMatrixDSym& M) const
  {
    assert(M.GetNrows() == 5);
    // Just use the 0,0 entry
    M.ResizeTo(1, 1);
  }


  void HMatrixQP::Print(const Option_t*) const
  {
    std::cout << "V" << std::endl;
  }

}
