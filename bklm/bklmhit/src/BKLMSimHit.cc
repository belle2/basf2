/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/bklmhit/BKLMSimHit.h>
#include <framework/logging/Logger.h>
#include<iostream>
#include<fstream>

using namespace Belle2;

ClassImp(Belle2::BKLMSimHit)

G4Allocator<BKLMSimHit> BKLMSimHitAllocator;

void BKLMSimHit::Save(char* filename)
{
  std::ofstream save_hit(filename, std::fstream::app);
  save_hit << '\n';
  save_hit << "BKLM Hit: \n" ;
  save_hit << "Position: " << m_hitPos << '\n'  ;
  save_hit << "Time: " << m_hitTime << '\n' ;
  save_hit << "Energy Deposition: " <<  m_deltaE << '\n' ;
  save_hit << "PDG code: " << m_primaryPID << '\n';
  save_hit.close();
}



//} //end of Belle2 namespace
