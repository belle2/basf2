#ifndef _pairs_h_included_
#define _pairs_h_included_

namespace Photospp {


  void trypar(bool* JESLI, double* STRENG, double AMCH, double AMEL, double PA[4], double PB[4], double PE[4], double PP[4],
              bool* sameflav);

  void PHOPAR(int IPARR, int NHEP0, int idlep, double masslep, double* STRENG);

  void partra(int IBRAN, double PHOT[4]);

} // namespace Photospp

#endif

