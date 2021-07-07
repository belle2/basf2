*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                     Pseudo-CLASS  QED3                                          //
*//                                                                                 //
*//   Purpose:  QED bremsstrahlung matrix element up to O(alfa^3)                   //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
*     Version 2-nd August 1997, by S. Jadach.
*     Matrix element for fermion pair production, 
*     ISR O(alf3) + FSR O(alf2).
*     New features:
*     1. Kinematical reduction procedure kicked out.
*     2. Numerical stability for collinear photons improved up to BHLUMI level.
*     3. O(alf3)LL for ISR
*****
*     To be done:
*     All that should be organized as separate Class
*     1. wtset should be accessible through getter
*     2. ISR+FSR and ISR parts beter separated, if possible.
*/////////////////////////////////////////////////////////////////////////////////////
* The auxiliary weights for variable-weight runs are:
*     wt=wtcru1*wtcru2*wtset(i) where
*     wtset(i)  = ypar(500+i) are auxiliary weights
*
*     wtset(71) =   zero-th order initial+final
*     wtset(72) =   first order   initial+final
*     wtset(73) =   second order  initial+final
*     wtset(74) =   third order   initial+final
* and the following provide the corresponding components of x-section.
*     wtset(80-84)   =   first order beta's
*     wtset(90-97)   =   second order beta's
*     wtset(100-111) =   third order beta's
* furthermore, for the initial state alone we provide:
*     wtset( 1) =   zero-th order initial
*     wtset( 2) =   first order   initial
*     wtset( 3) =   second order  initial
*     wtset( 4) =   third order   initial
* and the corresponding components
*     wtset(10-11) =   first order beta's
*     wtset(20-22) =   second order beta's
*     wtset(30-33) =   third order beta's
*/////////////////////////////////////////////////////////////////////////////////////
      DOUBLE PRECISION      pi
      PARAMETER( pi=3.1415926535897932d0 )
*     Below limits m_vlim1/2 the contribution from beta1/2 is not calculated,
      DOUBLE PRECISION     m_vlim1,m_vlim2
      PARAMETER(m_vlim1= 1.d-9, m_vlim2= 1.d-9)
      INTEGER    npmx                                  ! maximum number of photons
      PARAMETER( npmx=100)
      INTEGER    m_lenwt                               ! max. no. of aux weights
      PARAMETER (m_lenwt=200)
*/////////////////////////////////////////////////////////////////////////////////////
* amplitude components
      DOUBLE PRECISION  
     $  m_xSfac,m_ySfac                                           ! Soft factors
* ISR+FSR
      DOUBLE PRECISION   m_Beta00                                 ! O(alf0)
      DOUBLE PRECISION   m_Beta01                                 ! O(alf1)
      DOUBLE PRECISION   m_xBet10,  m_yBet10                      ! O(alf1)
      DOUBLE PRECISION   m_Beta02,  m_xBet11,  m_yBet11           ! O(alf2)
      DOUBLE PRECISION   m_xxBet20, m_xyBet20, m_yyBet20          ! O(alf2)
      DOUBLE PRECISION   m_Beta03,  m_xBet12,  m_yBet12           ! O(alf3)
      DOUBLE PRECISION   m_xxBet21, m_xyBet21, m_yyBet21          ! O(alf3)
      DOUBLE PRECISION   m_xxxBet30,m_xxyBet30,m_xyyBet30         ! O(alf3)
* pure ISR
      DOUBLE PRECISION   m_beti00                                 ! O(alf0)
      DOUBLE PRECISION   m_beti01,m_sbti10                        ! O(alf1)
      DOUBLE PRECISION   m_beti02,m_sbti11,m_sbti20               ! O(alf2)
      DOUBLE PRECISION   m_beti03,m_sbti12,m_sbti21,m_sbti30      ! O(alf2)
* UNEXP
      DOUBLE PRECISION   m_dis0,m_dis1,m_dis2
      DOUBLE PRECISION   m_dig1
*
      DOUBLE PRECISION   m_WtSet,m_WtBest,m_alfinv,m_vvmin
      INTEGER m_IdeWgt,m_KeyISR
*
        COMMON /c_QED3/
     $  m_WtSet(m_lenwt),
     $  m_WtBest,
     $  m_xSfac(npmx),                                 ! ISR soft factors
     $  m_ySfac(npmx),                                 ! FSR soft factors
     $  m_Beta00,                                      ! beta0 O(alf0) ISR+FSR
     $  m_Beta01,  m_xBet10,  m_yBet10,                ! beta1 O(alf1) ISR+FSR
     $  m_Beta02,                                      ! beta0 O(alf2) ISR+FSR
     $  m_xBet11,  m_yBet11,                           ! beta1 O(alf2) ISR+FSR
     $  m_xxBet20, m_xyBet20, m_yyBet20,               ! beta2 O(alf2) ISR+FSR
     $  m_Beta03,                                      ! beta0 O(alf3) ISR+FSR
     $  m_xBet12,  m_yBet12,                           ! beta1 O(alf3) ISR+FSR
     $  m_xxBet21, m_xyBet21, m_yyBet21,               ! beta2 O(alf3) ISR+FSR
     $  m_xxxBet30,m_xxyBet30,m_xyyBet30,              ! beta3 O(alf3) ISR+FSR
     $  m_beti00,                                      ! O(alf0) pure ISR
     $  m_beti01,m_sbti10,                             ! O(alf1) pure ISR
     $  m_beti02,m_sbti11,m_sbti20,                    ! O(alf2) pure ISR
     $  m_beti03,m_sbti12,m_sbti21,m_sbti30,           ! O(alf2) pure ISR
     $  m_dis0,m_dis1,m_dis2,                          ! UNEXP O(alf2) ISR
     $  m_dig1,                                        ! UNEXP auxiliary
     $  m_alfinv,
     $  m_vvmin,
     $  m_IdeWgt,
     $  m_KeyISR
*
        SAVE  /c_QED3/
*
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//              End of Pseudo-CLASS  QED3                                          //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
