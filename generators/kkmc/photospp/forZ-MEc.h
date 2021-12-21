#ifndef _forZ_MEc_h_included_
#define _forZ_MEc_h_included_

namespace Photospp {

  class PhotosMEforZ {
  public:
    static double phwtnlo();
    static void   set_VakPol(double (*fun)(double[4], double[4], double[4], double[4], double[4], int, int));

  private:
    static void   GIVIZO(int idferm, int ihelic, double* sizo3, double* charge, int* kolor);
    static double PHBORNM(double svar, double costhe, double T3e, double qe, double T3f, double qf, int Ncf);
    static double AFBCALC(double SVAR, int IDEE, int IDFF);
    static int    GETIDEE(int IDE);
    static double PHASYZ(double SVAR, int IDE, int IDF);
    static double Zphwtnlo(double svar, double xk, int IDHEP3, int IREP, double qp[4], double qm[4], double ph[4], double pp[4],
                           double pm[4], double COSTHG, double BETA, double th1, int IDE, int IDF);

    static double         VakPol(double qp[4], double qm[4], double ph[4], double pp[4], double pm[4], int IDE, int IDF);
    static double default_VakPol(double qp[4], double qm[4], double ph[4], double pp[4], double pm[4], int IDE, int IDF);

  private:
    static double (*currentVakPol)(double[4], double[4], double[4], double[4], double[4], int, int);
  };

} // namespace Photospp

#endif
