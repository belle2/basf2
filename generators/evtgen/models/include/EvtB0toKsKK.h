#ifndef EvtB0toKsKK_H
#define EvtB0toKsKK_H

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtParticle.hh"

class EvtB0toKsKK : public  EvtDecayAmp {

public:

  EvtB0toKsKK() {}
  virtual ~EvtB0toKsKK();

  std::string getName();

  EvtDecayBase* clone();

  void init();

  void initProbMax();

  void decay(EvtParticle* p);

  EvtVector4R umu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                  const EvtVector4R& p4c);
  EvtVector4R Smu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                  const EvtVector4R& p4c);
  EvtVector4R Lmu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                  const EvtVector4R& p4c);
  EvtTensor4C gmunu_tilde(const EvtVector4R& p4a,
                          const EvtVector4R& p4b,
                          const EvtVector4R& p4c);
  EvtTensor4C Tmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                    const EvtVector4R& p4c);
  EvtTensor4C Multiply(const EvtTensor4C& t1,
                       const EvtTensor4C& t2);
  EvtTensor4C RaiseIndices(const EvtTensor4C& t);
  void RaiseIndex(EvtVector4R& vector);
  EvtTensor4C Mmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                    const EvtVector4R& p4c);
  double BWBF(const double& q, const unsigned int& L);
  double BWBF(const double& q, const double& q0,
              const unsigned int& L);
  EvtComplex BreitWigner(const double& m, const double& m0,
                         const double& Gamma0,
                         const double& q, const double& q0,
                         const unsigned int& L);
  EvtVector4R Boost(const EvtVector4R& p4,
                    const EvtVector4R& boost);
  double p(const double& mab, const double& M, const double& mc);
  double q(const double& mab, const double& ma, const double& mb);
  EvtComplex Flatte_k(const double& s, const double& m_h);
  EvtComplex Flatte(const double& m, const double& m0);

  EvtComplex A_f0ks(const EvtVector4R& p4ks,
                    const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_phiks(const EvtVector4R& p4ks,
                     const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_fxks(const EvtVector4R& p4ks,
                    const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_chic0ks(const EvtVector4R& p4ks,
                       const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_kknr(const EvtVector4R& p4k1, const EvtVector4R& p4k2,
                    const double& alpha_kk);

private:
  EvtComplex a_f0ks_;
  EvtComplex a_phiks_;
  EvtComplex a_fxks_;
  EvtComplex a_chic0ks_;
  EvtComplex a_kpkmnr_;
  EvtComplex a_kskpnr_;
  EvtComplex a_kskmnr_;

  EvtComplex abar_f0ks_;
  EvtComplex abar_phiks_;
  EvtComplex abar_fxks_;
  EvtComplex abar_chic0ks_;
  EvtComplex abar_kpkmnr_;
  EvtComplex abar_kskpnr_;
  EvtComplex abar_kskmnr_;

  double alpha_kpkmnr;
  double alpha_kskpnr;
  double alpha_kskmnr;

  std::ofstream debugfile_;
};

#endif
