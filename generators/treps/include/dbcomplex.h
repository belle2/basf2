/*
  dbcomplex:

  Author : S.Uehara, KEK-IPNS
  Date :  Feb.1, 1999
*/
// Class definition



#if !defined(PACKAGE_DBCOMPLEX_H_INCLUDED)
#define PACKAGE_DBCOMPLEX_H_INCLUDED

namespace Belle2 {

  class dbcomplex {
    double re, im;

  public:
    dbcomplex(double r = 0, double i = 0) { re = r; im = i; };
    ~dbcomplex() {};

    friend dbcomplex operator +(dbcomplex a , dbcomplex b)
    {
      return dbcomplex(a.re + b.re, a.im + b.im);
    };

    friend dbcomplex operator +(double a , dbcomplex b)
    {
      return dbcomplex(a + b.re, b.im);
    };

    friend dbcomplex operator +(dbcomplex a , double b)
    {
      return dbcomplex(a.re + b, a.im);
    };

    friend dbcomplex operator -(dbcomplex a , dbcomplex b)
    {
      return dbcomplex(a.re - b.re, a.im - b.im);
    };

    friend dbcomplex operator -(double a , dbcomplex b)
    {
      return dbcomplex(a - b.re, b.im);
    };

    friend dbcomplex operator -(dbcomplex a , double b)
    {
      return dbcomplex(a.re - b, a.im);
    };

    friend dbcomplex operator *(dbcomplex a , dbcomplex b)
    {
      return dbcomplex(a.re * b.re -  a.im * b.im, a.re * b.im + a.im * b.re);
    };

    friend dbcomplex operator *(double a , dbcomplex b)
    {
      return dbcomplex(a * b.re, a * b.im);
    };

    friend dbcomplex operator *(dbcomplex a , double b)
    {
      return dbcomplex(a.re * b, a.im * b);
    };


    friend dbcomplex operator /(dbcomplex a , double b)
    {
      return dbcomplex(a.re / b, a.im / b);
    };

    friend dbcomplex operator /(double a , dbcomplex b)
    {
      return dbcomplex(a * b.re / (b.re * b.re + b.im * b.im),
                       -a * b.im / (b.re * b.re + b.im * b.im));
    };



    friend dbcomplex operator /(dbcomplex a , dbcomplex b)
    {
      return a * (1. / b);
    };

    double real(void)
    {
      return re ;
    };

    double imag(void)
    {
      return im ;
    };

    double abs(void)
    {
      return sqrt(re * re + im * im) ;
    }

    dbcomplex conjg(void)
    {
      return dbcomplex(re, -im) ;
    }
  };

#endif /* PACKAGE_DBCOMPLEX_H_INCLUDED */


