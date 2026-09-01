/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <EvtGenBase/EvtDecayAmp.hh>

class EvtParticle;

/** The class provides the form factors for orbitally excited semileptonic decays
 */
class EvtHNLScalarL: public  EvtDecayAmp  {

public:

  /** Default constructor */
  EvtHNLScalarL();

  /** virtual destructor */
  virtual ~EvtHNLScalarL();

  /** Returns name of module */
  std::string getName();

  /** Clones module */
  EvtDecayBase* clone();

  /** Creates a decay */
  void decay(EvtParticle* p);

  /** Sets maximal probab. */
  void initProbMax();

  /** Initializes module */
  void init();

private:

  /** Maximum probability used for accept-reject generation */
  static constexpr double m_ProbMax = 10000.0;

};



