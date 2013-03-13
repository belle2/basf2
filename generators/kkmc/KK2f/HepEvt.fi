*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  HepEvt                                 //
*//                                                                          //
*//  Purpose:  keep and serve event in HEPEVT format                         //
*//                                                                          //
*//  Output of KK2f   is encoded in double precission /hepevt/               //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////

* ----------------------------------------------------------------------
      INTEGER nmxhep         ! maximum number of particles
      PARAMETER (nmxhep=4000)
      DOUBLE PRECISION   phep, vhep
      INTEGER nevhep, nhep, isthep, idhep, jmohep, jdahep
      COMMON /hepevt/
     $     nevhep,           ! serial number
     $     nhep,             ! number of particles
     $     isthep(nmxhep),   ! status code
     $     idhep(nmxhep),    ! particle ident KF
     $     jmohep(2,nmxhep), ! parent particles
     $     jdahep(2,nmxhep), ! childreen particles
     $     phep(5,nmxhep),   ! four-momentum, mass [GeV]
     $     vhep(4,nmxhep)    ! vertex [mm]
      SAVE  /hepevt/
* ----------------------------------------------------------------------
      LOGICAL qedrad
      COMMON /phoqed/ 
     $     qedrad(nmxhep)    ! Photos flag
      SAVE   /phoqed/
* ----------------------------------------------------------------------
      INTEGER m_PhotStart ! start position of photons (1st phot at m_PhotStart+1)
      INTEGER m_PhotEnd   ! end position photons
      INTEGER m_PosnF     ! position of final state fermion
      INTEGER m_PosnFbar  ! position of final state anti-fermion

      COMMON/HepEvt_posn/m_PhotStart,m_PhotEnd,m_PosnF,m_PosnFbar

      SAVE/HepEvt_posn/

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  HepEvt                                //
*//////////////////////////////////////////////////////////////////////////////
