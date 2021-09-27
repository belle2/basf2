      SUBROUTINE make_phsp_point_z
     $      (msdump,ambeam,svar,sprim,fak_phsp,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
*     **************************************
* msdump=0 : generates the phase space point
* msdump=1 : point is red from the disk
* fak_phsp  : weight 
*     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'zz_phsp.inc'

      DIMENSION
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)

      CALL brancher(sprim,itype)

      CALL spacegen(0,itype,svar,sprim,fak_phsp,
     $                       effp1,effp2,effp3,effp4)
      CALL set_eff_beams(sprim,ambeam,effbeam1,effbeam2)

      ikan=itype
      faki(itype)=fak_phsp

      END

      SUBROUTINE get_phsp_weight_z
     $      (svar,sprim,fak_tot,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
*     **************************************
* calculates jacobians of all channels and does resummation
* fak_tot  : total weight - the only output
*     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'zz_phsp.inc'

      DIMENSION
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)

      fak=0d0
*      DO i=mrchan,nrchan
      DO i=1,nrchan
         IF (i .NE. ikan .AND. prob(i) .GT. 0d0)
     $        CALL spacegen(1,i,svar,sprim,faki(i),
     $                       effp1,effp2,effp3,effp4)
*      IF (i .EQ. ikan) WRITE(*,*)'fakusie(',i,')=',faki(i)/faki(ikan)
         IF (prob(i) .GT. 0d0) THEN
            fak=fak+prob(i)/faki(i)
         ELSE
            faki(i)=0d0
         ENDIF
         IF(prob(i) .NE. 0d0 .AND. faki(i) .EQ. 0d0) THEN
            WRITE(*,*) 
     $           'karludw: jacobian(i)=0d0  ikan=',ikan,'i=',i
            WRITE(*,*) 
     $           'i from - to',mrchan,'  ',nrchan,'prob(i)',prob(i)
            WRITE(*,*) 'p1=',p1
            WRITE(*,*) 'p2=',p2
            WRITE(*,*) 'p3=',p3
            WRITE(*,*) 'p4=',p4
         ENDIF
      ENDDO

      fak=1d0/fak

      fak_tot=fak

      END



