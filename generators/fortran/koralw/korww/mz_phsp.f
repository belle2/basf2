      SUBROUTINE make_phsp_point_mz
     $      (prob_ps_m,msdump,ambeam,svar,sprim,fak_phsp,i_m,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
*     **************************************
* msdump=0 : generates the phase space point
* msdump=1 : point is red from the disk
* fak_phsp  : weight 
*     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

! two commons, no way ....
      INCLUDE 'mm_phsp.inc'
      INCLUDE 'zz_phsp.inc'

      DIMENSION drvec(100)
      DIMENSION
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)

      IF(msdump .EQ. 1) THEN
         CALL mm_brancher(sprim,itype,prob_mm)
         CALL brancher(sprim,itype)
         CALL READer(itype,effbeam1,effbeam2,effp1,effp2,effp3,effp4)
         CALL mm_spacegen(1,itype,ambeam,svar,sprim,fak_phsp,
     $                  effp1,effp2,effp3,effp4)
      ELSE
        CALL varran(drvec,1)
        IF(drvec(1) .LE. prob_ps_m) THEN
          i_m=1
        ELSE
          i_m=0
        ENDIF
        IF( i_m .EQ. 1 ) THEN
          CALL mm_brancher(sprim,itype,prob_mm)
          CALL mm_spacegen(0,itype,ambeam,svar,sprim,fak_phsp,
     $                    effp1,effp2,effp3,effp4)
          ikan_mon=itype
          faki_mon(itype)=fak_phsp
        ELSEIF( i_m .EQ. 0 ) THEN
          CALL brancher(sprim,itype)
          CALL spacegen(0,itype,svar,sprim,fak_phsp,
     $                    effp1,effp2,effp3,effp4)
          ikan=itype
          faki(itype)=fak_phsp
        ENDIF
        CALL set_eff_beams(sprim,ambeam,effbeam1,effbeam2)
      ENDIF

      END

      SUBROUTINE get_phsp_weight_mz
     $      (prob_ps_m,ambeam,svar,sprim,fak_tot,i_m,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
*     **************************************
* calculates jacobians of all channels and does resummation
* fak_tot  : total weight - the only output
*     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

! two commons, no way ....
      INCLUDE 'mm_phsp.inc'
      INCLUDE 'zz_phsp.inc'

      DIMENSION
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)

      fakm=0d0
      DO i=1,mm_nrchan
         IF (prob_mm(i) .GT. 0d0) THEN
            IF (i .NE. ikan_mon  .OR.  i_m .NE. 1) THEN
               CALL mm_spacegen(1,i,ambeam,svar,sprim,faki_mon(i),
     $                    effp1,effp2,effp3,effp4)
            ENDIF
            IF (faki_mon(i) .GT. 0d0) THEN
               fakm=fakm+prob_mm(i)/faki_mon(i)
            ENDIF
         ENDIF
      ENDDO
*...  
      fakz=0d0
      DO i=1,nrchan
         IF (prob(i) .GT. 0d0) THEN
            IF (i .NE. ikan  .OR.  i_m .NE. 0) THEN
               CALL spacegen(1,i,svar,sprim,faki(i),
     $                    effp1,effp2,effp3,effp4)
            ENDIF
            IF (faki(i) .GT. 0d0) THEN
               fakz=fakz+prob(i)/faki(i)
            ENDIF
         ENDIF
      ENDDO
*...
      fak=1/(prob_ps_m*fakm +(1-prob_ps_m)*fakz)

      fak_tot=fak

      END



