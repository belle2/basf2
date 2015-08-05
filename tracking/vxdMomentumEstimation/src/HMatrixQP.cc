#include <tracking/vxdMomentumEstimation/HMatrixQP.h>
#include <cassert>
#include <alloca.h>
#include <iostream>

namespace genfit {


// 1, 0, 0, 0, 0

  const TMatrixD& HMatrixQP::getMatrix() const
  {
    static const double HMatrixContent[5] = {1, 0, 0, 0, 0};

    static const TMatrixD HMatrix(1, 5, HMatrixContent);

    return HMatrix;
  }


  TVectorD HMatrixQP::Hv(const TVectorD& v) const
  {
    assert(v.GetNrows() == 5);

    double* retValArray = (double*)alloca(sizeof(double) * 1);

    retValArray[0] = v(0); // q/p

    return TVectorD(1, retValArray);
  }


  TMatrixD HMatrixQP::MHt(const TMatrixDSym& M) const
  {
    assert(M.GetNcols() == 5);

    double* retValArray = (double*)alloca(sizeof(double) * 5);
    const double* MatArray = M.GetMatrixArray();

    for (unsigned int i = 0; i < 5; ++i) {
      retValArray[i] = MatArray[i * 5 + 0];
    }

    return TMatrixD(5, 1, retValArray);
  }


  TMatrixD HMatrixQP::MHt(const TMatrixD& M) const
  {
    assert(M.GetNcols() == 5);

    double* retValArray = (double*)alloca(sizeof(double) * M.GetNrows());
    const double* MatArray = M.GetMatrixArray();

    for (int i = 0; i < M.GetNrows(); ++i) {
      retValArray[i] = MatArray[i * 5 + 0];
    }

    return TMatrixD(M.GetNrows(), 1, retValArray);
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

} /* End of namespace genfit */
