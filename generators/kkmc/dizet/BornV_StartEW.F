      SUBROUTINE BornV_StartEW(xpar_input)
*///////////////////////////////////////////////////////////////////
*//                                                               //
*//   Slow-start EW correction library                            //
*//   Here it is done by making forfactors from the scratch       //
*//                                                               //
*///////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../bornv/BornV.fi'
      DOUBLE PRECISION  xpar_input(*)
*------------------------------------------------------------------
      CALL BornV_StartDZ(xpar_input)
      END                       !BornV_StartEW

      SUBROUTINE BornV_StartDZ(xpar)
*///////////////////////////////////////////////////////////////////
*//                                                               //
*//   xpar goes as an input to DZface                             //
*//                                                               //
*///////////////////////////////////////////////////////////////////
      INCLUDE '../bornv/BornV.fi'
      DOUBLE PRECISION  xpar(*)
*------------------------------------------------------------------
      INTEGER KFdown, KFup, KFstran, KFcharm, KFbotom, KFtop
      PARAMETER( 
     $     KFdown  = 1,   KFup    = 2,
     $     KFstran = 3,   KFcharm = 4,
     $     KFbotom = 5,   KFtop   = 6)
      INTEGER KFel,KFelnu,KFmu,KFmunu,KFtau,KFtaunu
      PARAMETER(
     $     KFel    = 11,  KFelnu  = 12,
     $     KFmu    = 13,  KFmunu  = 14,
     $     KFtau   = 15,  KFtaunu = 16)
*------------------------------------------------------------------
* Find active chanels
      DO i=401,416
         IF( xpar(i) .EQ. 1d0 ) THEN
            KFfin= i-400
            IF(    KFfin .EQ. KFdown  ) THEN
               WRITE(    *,*) '=========== BornV_StartDZ: DOWN quark ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: DOWN quark ==========='
               CALL DZface_Initialize( KFfin, xpar) ! Set EW params and run Dizet
               CALL DZface_Tabluj                   ! Calculate formfactor and store in tables
            ELSEIF(KFfin .EQ. KFup    )  THEN 
               WRITE(    *,*) '=========== BornV_StartDZ: UP quark ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: UP quark ==========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFstran )  THEN
               WRITE(    *,*) '=========== BornV_StartDZ: STRAN quark ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: STRAN quark ==========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFcharm )  THEN
               WRITE(    *,*) '=========== BornV_StartDZ: CHARM quark ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: CHARM quark ==========='
                CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFbotom )  THEN 
               WRITE(    *,*) '=========== BornV_StartDZ: BOTTOM quark ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: BOTTOM quark ==========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFelnu)  THEN 
               WRITE(    *,*) '=========== BornV_StartDZ: e   neutrino ========='
               WRITE(m_out,*) '=========== BornV_StartDZ: e   neutrino ========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFmu)  THEN 
               WRITE(    *,*) '=========== BornV_StartDZ: MU lepton ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: MU lepton ==========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFmunu)  THEN 
               WRITE(    *,*) '=========== BornV_StartDZ: MU neutrino =========='
               WRITE(m_out,*) '=========== BornV_StartDZ: MU neutrino =========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFtau)  THEN 
               WRITE(    *,*) '=========== BornV_StartDZ: TAU lepton ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: TAU lepton ==========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSEIF(KFfin .EQ. KFtaunu)  THEN 
               WRITE(    *,*) '=========== BornV_StartDZ: TAU lepton ==========='
               WRITE(m_out,*) '=========== BornV_StartDZ: TAU lepton ==========='
               CALL DZface_Initialize( KFfin, xpar)
               CALL DZface_Tabluj
            ELSE
               WRITE(*,   *) '#### STOP in dizet/BornV_StartEW, wrong KFfin=',KFfin
               WRITE(iout,*) '#### STOP in dizet/BornV_StartEW, wrong KFfin=',KFfin
               STOP
            ENDIF
         ENDIF
      ENDDO
      END                       ! BornV_StartDZ
