#ifndef BPARTICLE_H
#define BPARTICLE_H
//+
// File : BParticle.h
// Description : class to contain particle info.
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Jan - 2004
//-

#include "TObject.h"
#include "TClonesArray.h"

enum SIMPLEPID {PHOTON, ELECTRON, PION, MUON, KAON, PROTON, JPSI, D, DSTAR, B, PHI, LAMBDA0, ALL };

class BParticle : public TObject {

private:
  float m_px;
  float m_py;
  float m_pz;
  float m_e;
  float m_charge;
  SIMPLEPID m_pid;

public:
  BParticle() {};
  BParticle(float px, float py, float pz, float e, float c, SIMPLEPID pid);
  ~BParticle() {};

  float px() const { return m_px; };
  float py() const { return m_py; };
  float pz() const { return m_pz; };
  float e() const { return m_e; };
  float GetMomentum() const { return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz); };
  float GetTransverseMomentum() const { return sqrt(m_px * m_px + m_py * m_py); };
  float charge() const { return m_charge; };
  SIMPLEPID pid() const { return m_pid; };
  float GetMass(SIMPLEPID pid);
  float GetMass();
  void SetEnergyFromMass(float mass);
  void SetEnergyFromPid();
  void SetPid(SIMPLEPID pid) { m_pid = pid; };
  int  InMassRange(float mlower, float mupper) { float m = GetMass(); if (m >= mlower && m <= mupper) return 1; else return 0; };
  BParticle operator+(const BParticle& b)
  {

    BParticle particle(
      px() + b.px(),
      py() + b.py(),
      pz() + b.pz(),
      e()  + b.e(),
      charge() + b.charge(),
      PHOTON // wrong
    );

    return particle;
  };

  BParticle& operator=(const BParticle&  p)
  {
    //if ( this != &p){
    m_px = p.px();
    m_py = p.py();
    m_pz = p.pz();
    m_e  = p.e();
    m_charge  = p.charge();
    m_pid  =  p.pid();
    //}
    return *this;
  };


  ClassDef(BParticle, 1)    // Simple particle class
};

int SelectParticles(TClonesArray* pin , int charge, SIMPLEPID type, TClonesArray* pout);
int CombineParticles(TClonesArray* plist1 , TClonesArray* plist2 , int same, float masslow, float massup, SIMPLEPID pid,
                     TClonesArray* pout);


#endif
