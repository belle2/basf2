//+
// File : BEvent.h
// Description : class to contain an event
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Jan - 2004
//-

#pragma once

#include "TObject.h"
#include "TClonesArray.h"

#include "BParticle.h"

class BEvent : public TObject {
private:
  int m_evno;
  int m_nprt;
  TClonesArray* m_particles;

public:
  BEvent();
  ~BEvent();

  virtual void Clear(Option_t* /*option*/ = "");

  void EventNo(int evtno);
  int EventNo();

  void AddTrack(float px, float py, float pz, float e,
                float charge, SIMPLEPID pid);

  int NParticles();
  TClonesArray* GetParticleList();

  ClassDef(BEvent, 1)
};


