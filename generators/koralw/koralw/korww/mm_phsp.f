      SUBROUTINE make_phsp_point
     $      (msdump,label,ambeam,svar,sprim,fak_phsp,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
*     **************************************
* msdump=0 : generates the phase space point
* msdump=1 : point is red from the disk
* fak_phsp  : weight 
*     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'mm_phsp.inc'

      DIMENSION
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)
      DIMENSION
     $      qeffp1(4),qeffp2(4),qeffp3(4),qeffp4(4)
      DIMENSION drvec(100)

      CALL mm_brancher(sprim,itype,prob_mm)

      IF(msdump .EQ. 1) THEN
         CALL READer(itype,effbeam1,effbeam2,effp1,effp2,effp3,effp4)
         CALL mm_spacegen(1,itype,ambeam,svar,sprim,fak_phsp,
     $                     effp1,effp2,effp3,effp4)
      ELSE
         CALL mm_spacegen(0,itype,ambeam,svar,sprim,fak_phsp,
     $                     qeffp1,qeffp2,qeffp3,qeffp4)
         CALL set_eff_beams(sprim,ambeam,effbeam1,effbeam2)
! permutations in the case of eeee ZZ state
         CALL linear_to_WZ_label(1,label,iwm,iwp,if_z,if_w)
         IF(if_z.EQ.1 .AND. iwm.EQ.6 .AND. iwp.EQ.6) THEN
c extend to all         IF(if_z.EQ.1 .AND. iwm.EQ.iwp) THEN
c masses not equal         IF( (if_z.EQ.1 .AND. (iwm.EQ.6 .OR. iwp.EQ.6)) .OR.
c masses not equal     @       (if_w.EQ.1 .AND. (iwm.EQ.7 .OR. iwp.EQ.7)) ) THEN
           CALL varran(drvec,1)
           IF(drvec(1).LE. .25d0) THEN
             mode=0
           ELSEIF(drvec(1).LE. .5d0) THEN
             mode=1
           ELSEIF(drvec(1).LE. .75d0) THEN
             mode=2
           ELSE
             mode=3
           ENDIF
           CALL perm_em(mode,qeffp1,qeffp2,qeffp3,qeffp4,
     @                    effp1,effp2,effp3,effp4 )
           itype =itype +1000*mode
         ELSE
! no permutations for non-eeee states
           CALL perm_em(0,qeffp1,qeffp2,qeffp3,qeffp4,
     @                    effp1,effp2,effp3,effp4 )
         ENDIF
      ENDIF

      ikan_mon=itype
      faki_mon(itype)=fak_phsp

      END

      SUBROUTINE get_phsp_weight
     $      (label,ambeam,svar,sprim,fak_tot,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
*     **************************************
* calculates jacobians of all channels and does resummation
* fak_tot  : total weight - the only output
*     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'mm_phsp.inc'

      DIMENSION
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)
      DIMENSION
     $      qeffp1(4),qeffp2(4),qeffp3(4),qeffp4(4)

      fak=0d0
      DO i=1,mm_nrchan
         IF (prob_mm(i) .GT. 0d0) THEN
            IF (i .NE. ikan_mon) 
     $           CALL mm_spacegen(1,i,ambeam,svar,sprim,faki_mon(i),
     $                    effp1,effp2,effp3,effp4)
            IF (faki_mon(i) .GT. 0d0) THEN
               fak=fak+prob_mm(i)/faki_mon(i)
* m.s. 21.03, events with some faki_mon=0 should not be discarded
*            ELSE
*               fak=0d0
*               WRITE(6,*)'mm_phsp==> jacob=0 in chan.',i,' of ',ikan_mon
*               GOTO 150
* m.s. 21.03, events with some faki_mon=0 should not be discarded
            ENDIF
         ENDIF
      ENDDO
* 150  CONTINUE
! permutations in the case of eeee ZZ state
      CALL linear_to_WZ_label(1,label,iwm,iwp,if_z,if_w)
      IF(if_z.EQ.1 .AND. iwm.EQ.6 .AND. iwp.EQ.6) THEN
c extend to all      IF(if_z.EQ.1 .AND. iwm.EQ.iwp) THEN
c masses not equal      IF( (if_z.EQ.1 .AND. (iwm.EQ.6 .OR. iwp.EQ.6)) .OR.
c masses not equal     @    (if_w.EQ.1 .AND. (iwm.EQ.7 .OR. iwp.EQ.7)) ) THEN
       ip_act=ikan_mon/1000
       ikan_mon=mod(ikan_mon,1000)
       DO iper=1,3
        DO i=1,mm_nrchan
          IF (prob_mm(i) .GT. 0d0) THEN
            IF (i.NE.ikan_mon .OR. ip_act.NE.iper) THEN
! do permutations
              CALL perm_em(iper,effp1,effp2,effp3,effp4,
     @                    qeffp1,qeffp2,qeffp3,qeffp4 )
              CALL mm_spacegen(1,i,ambeam,svar,sprim,faki_mon(i),
     $                    qeffp1,qeffp2,qeffp3,qeffp4)
            ELSE
              faki_mon(i)=faki_mon(ikan_mon+ip_act*1000)
            ENDIF
            IF (faki_mon(i) .GT. 0d0) THEN
               fak=fak+prob_mm(i)/faki_mon(i)
            ENDIF
          ENDIF
        ENDDO
       ENDDO
       fak=fak/4d0
      ENDIF
      fak=1d0/fak

      fak_tot=fak

      END

      SUBROUTINE perm_em(mode,qeffp1,qeffp2,qeffp3,qeffp4,
     @                    effp1,effp2,effp3,effp4 )
!     *************************************************
! does very internal permutation for make_phsp_point /eeee state/
! qeffp --> effp
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION
     $      effp1(4),effp2(4),effp3(4),effp4(4)
      DIMENSION
     $      qeffp1(4),qeffp2(4),qeffp3(4),qeffp4(4)
      SAVE

      IF(mode .EQ. 0) THEN
! no permutation
        DO i=1,4
          effp1(i)=qeffp1(i)
          effp2(i)=qeffp2(i)
          effp3(i)=qeffp3(i)
          effp4(i)=qeffp4(i)
        ENDDO
      ELSEIF(mode .EQ. 1) THEN
! 1<->3
        DO i=1,4
          effp1(i)=qeffp3(i)
          effp2(i)=qeffp2(i)
          effp3(i)=qeffp1(i)
          effp4(i)=qeffp4(i)
        ENDDO
        itype=itype+1000
      ELSEIF(mode .EQ. 2) THEN
! 2<->4
        DO i=1,4
          effp1(i)=qeffp1(i)
          effp2(i)=qeffp4(i)
          effp3(i)=qeffp3(i)
          effp4(i)=qeffp2(i)
        ENDDO
        itype=itype+2000
      ELSEIF(mode .EQ. 3) THEN
! 1<->3, 2<->4
        DO i=1,4
          effp1(i)=qeffp3(i)
          effp2(i)=qeffp4(i)
          effp3(i)=qeffp1(i)
          effp4(i)=qeffp2(i)
        ENDDO
      ELSE
        WRITE(6,*)'perm_em=> wrong mode:',mode
        STOP
      ENDIF
      END

      SUBROUTINE pres_monit(mode,wtcrud,wtmod,wtset)
!     *************************************************
! monitors probabilities of different branches
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      SAVE

      INCLUDE 'mm_phsp.inc'

      DIMENSION densit(mm_nrchan),top(mm_nrchan),numb(mm_nrchan)
      DIMENSION densit_jac(mm_nrchan),top_jac(mm_nrchan)
      DIMENSION numb_jac(mm_nrchan)
      DIMENSION wtset(*)      

      DATA init /0/

      IF(init.eq.0)THEN
        init=1
        do i=1,mm_nrchan
          densit(i) = 0d0
          top(i) = 0d0
          numb(i) = 0
        enddo
      ENDIF

      IF(mode.EQ.0) THEN
        IF(wtmod.GT.0d0) THEN

        wtmod4f = wtset(40)
! 10/8/98 double counting of wtset(40)
!        wtt = wtmod*wtmod4f
        wtt = wtmod
! 10/8/98 ms

        densit(ikan_mon) = densit(ikan_mon) +wtt
        numb(ikan_mon) = numb(ikan_mon) +1
        IF(top(ikan_mon).LT.wtt) top(ikan_mon) = wtt 
!.. by jacobians
        imin=1
        DO i=1,mm_nrchan
          IF(faki_mon(imin).eq.0d0 .or. 
     $       faki_mon(i).gt.0d0 .and. faki_mon(i).lt.faki_mon(imin))
     $          imin=i 
        ENDDO
        densit_jac(imin) = densit_jac(imin) +wtt
        numb_jac(imin) = numb_jac(imin) +1
        IF(top_jac(imin).LT.wtt) top_jac(imin) = wtt 
!.. by jacobians
        ENDIF
      ELSEIF(mode.EQ.1) THEN

        dtop=0d0
        dtot=0d0

        do i=1,mm_nrchan
          if(numb(i).gt.0) densit(i) = densit(i)/numb(i)
          dtot = dtot +densit(i)
          if(top(i).gt.dtop) dtop =top(i) 
        enddo

        do i=1,mm_nrchan
          densit(i) = densit(i)/dtot
          if(densit(i).gt.0.02d0) then
CC        write(6,*) '      prob(',i,')=',densit(i),' !max=',top(i)/dtop
          endif
        enddo

CC        write(6,*)'all channels'

        do i=1,mm_nrchan
CC        write(6,*) '      prob(',i,')=',densit(i),' !max=',top(i)/dtop
        enddo
! by jacobians
        dtop=0d0
        dtot=0d0

        do i=1,mm_nrchan
          if(numb_jac(i).gt.0) densit_jac(i)=densit_jac(i)/numb_jac(i)
          dtot = dtot +densit_jac(i)
          if(top_jac(i).gt.dtop) dtop =top_jac(i) 
        enddo

CC        write(6,*)'by jacobians........'
        do i=1,mm_nrchan
          densit_jac(i) = densit_jac(i)/dtot
          if(densit_jac(i).gt.0.02d0) then
CC        write(6,*) '      prob(',i,')=',densit_jac(i)
CC     $            ,' !max=',top_jac(i)/dtop
          endif
        enddo

CC        write(6,*)'all channels'

        do i=1,mm_nrchan
CC        write(6,*) '      prob(',i,')=',densit_jac(i)
CC     $           ,' !max=',top_jac(i)/dtop
        enddo

      ELSE
        write(6,*)'pres_monit=> wrong mode',mode
        stop
      ENDIF

      END


      subroutine mm_dumper(nout2,nout,nevtru
     $                     ,wtovr,wtu,wtmax,wtmod,wtmod4f,iflav)    
*     **************************************************************     
c overweighted events monitoring, MS version
c 
c (Re)Written by: M.Skrzypek        date: 
c Last update:             by:  
c
      implicit DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'mm_phsp.inc'

      COMMON / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot
      COMMON / cms_eff_momdec /
     $      effbeam1(4),effbeam2(4),p1(4),p2(4),p3(4),p4(4)
      SAVE   / momset /,/ cms_eff_momdec /

      dimension q12(4),q13(4),q14(4),q23(4),q24(4),q34(4),qtot(4)
      dimension q123(4),q124(4),q134(4),q234(4)
      dimension iflav(4)

CC      write(nout,*) '===OVERVEIGHTED EVT. NR: NEVTRU=',NEVTRU,'===='
CC      write(nout,*) '===generated with channel: ikan=',ikan_mon,'===='
      CALL DUMPL(nout,P1,P2,P3,P4,effbeam1,effbeam2,sphot,nphot)
c      write(nout,*) 'Note: this event should be outside your detector'
c      write(nout,*) 'if not, increase wtmax (?) and/or check manual ?'
c      write(nout,*) 'final state: ',IFLAV
CC      write(nout,*) 'wtu= wtmod*wtmod4f/wtmax=',wtovr/wtmax
CC      write(nout,*) 'wtu_max=                 ',wtu,'    ... so far'
CC      write(nout,*) '-----------'
CC      write(nout,*) ' wtmod4f=',wtmod4f,' wtmod=',wtmod

      CALL writer(nout2,ikan_mon,effbeam1,effbeam2,p1,p2,p3,p4)

      do k=1,4
        q12(k)=p1(k)+p2(k)-effbeam1(k)
        q13(k)=p1(k)+p3(k)-effbeam1(k)
        q14(k)=p1(k)+p4(k)-effbeam1(k)
        q23(k)=p2(k)+p3(k)-effbeam1(k)
        q24(k)=p2(k)+p4(k)-effbeam1(k)
        q34(k)=p3(k)+p4(k)-effbeam1(k)

        q123(k)=p1(k)+p2(k)+p3(k)-effbeam1(k)
        q124(k)=p1(k)+p2(k)+p4(k)-effbeam1(k)
        q134(k)=p1(k)+p3(k)+p4(k)-effbeam1(k)
        q234(k)=p2(k)+p3(k)+p4(k)-effbeam1(k)

      enddo
      xm12=sqrt(abs(dmas2(q12)))
      xm13=sqrt(abs(dmas2(q13)))
      xm14=sqrt(abs(dmas2(q14)))
      xm23=sqrt(abs(dmas2(q23)))
      xm24=sqrt(abs(dmas2(q24)))
      xm34=sqrt(abs(dmas2(q34)))

      xm123=sqrt(abs(dmas2(q123)))
      xm124=sqrt(abs(dmas2(q124)))
      xm134=sqrt(abs(dmas2(q134)))
      xm234=sqrt(abs(dmas2(q234)))

CC      write(nout,'(3(A,G14.4))') 't12 =',real(xm12),'t13 =',real(xm13)
CC     $              ,'t14 =',real(xm14)
CC      write(nout,'(3(A,G14.4))') 't23 =',real(xm23),'t24 =',real(xm24)
CC     $              ,'t34 =',real(xm34)
CC      write(nout,'(4(A,G14.4))')'t123=',real(xm123),'t124=',real(xm124)
CC     $              ,'t134=',real(xm134),'t234=',real(xm234) 
      do k=1,4

        q123(k)=p4(k)-effbeam1(k)
        q124(k)=p3(k)-effbeam1(k)
        q134(k)=p2(k)-effbeam1(k)
        q234(k)=p1(k)-effbeam1(k)

      enddo

      xm123=sqrt(abs(dmas2(q123)))
      xm124=sqrt(abs(dmas2(q124)))
      xm134=sqrt(abs(dmas2(q134)))
      xm234=sqrt(abs(dmas2(q234)))

CC      write(nout,'(4(A,G14.4))')'t4  =',real(xm123),'t3  =',real(xm124)
CC     $         ,'t2  =',real(xm134),'t1  =',real(xm234 )
      do k=1,4
        q12(k)=p1(k)+p2(k)
        q13(k)=p1(k)+p3(k)
        q14(k)=p1(k)+p4(k)
        q23(k)=p2(k)+p3(k)
        q24(k)=p2(k)+p4(k)
        q34(k)=p3(k)+p4(k)

        q123(k)=p1(k)+p2(k)+p3(k)
        q124(k)=p1(k)+p2(k)+p4(k)
        q134(k)=p1(k)+p3(k)+p4(k)
        q234(k)=p2(k)+p3(k)+p4(k)

        qtot(k)=p1(k)+p2(k)+p3(k)+p4(k)
      enddo
      xm12=sqrt(dmas2(q12))
      xm13=sqrt(dmas2(q13))
      xm14=sqrt(dmas2(q14))
      xm23=sqrt(dmas2(q23))
      xm24=sqrt(dmas2(q24))
      xm34=sqrt(dmas2(q34))

      xm123=sqrt(dmas2(q123))
      xm124=sqrt(dmas2(q124))
      xm134=sqrt(dmas2(q134))
      xm234=sqrt(dmas2(q234))

      xmtot=sqrt(dmas2(qtot))
CC      write(nout,'(3(A,G14.4))') ' m12 =',real(xm12)
CC     $       ,' m13 =',real(xm13),' m14 =',real(xm14)
CC      write(nout,'(3(A,G14.4))') ' m23 =',real(xm23)
CC     $       ,' m24 =',real(xm24),' m34 =',real(xm34)
CC      write(nout,'(4(A,G14.4))') ' m123=',real(xm123)
CC     $       ,' m124=',real(xm124),' m134=',real(xm134)
CC     $                 ,' m234=',real(xm234) 
CC      write(nout,'(A,G14.4)') ' m1234=',real(xmtot)

      cthe1=cos(angle(effbeam1,p1))
      cthe2=cos(angle(effbeam1,p2))
      cthe3=cos(angle(effbeam1,p3))
      cthe4=cos(angle(effbeam1,p4))
CC      write(nout,3100)
CC     @    'decay c1e-,c2-n,c3n,c4e+ ',cthe1,cthe2,cthe3,cthe4

      DO i = 1,mm_nrchan,5
CC        write(nout,'(A7,I3,A1,I3,A1,5g16.7)')'jacob. ',i,'-',i+4,'=',
CC     $  real(faki_mon(i)),real(faki_mon(i+1)),real(faki_mon(i+2))
CC     $               ,real(faki_mon(i+3)),real(faki_mon(i+4))
      ENDDO
CC       write(nout,*) '        ============================'

!ms        WLAMBD=SPRIM**2+S1**2+S2**2-2*SPRIM*S1-2*SPRIM*S2-2*S1*S2
cc        wlambd=max(0d0,(sprim-s1-s2)**2 -4*s1*s2)
cc        T=-(SPRIM-S1-S2-DSQRT(WLAMBD)*COSTHE)/2
c        write(nout,*) 'channel ',IFLAV(4)
c        write(nout,*) ' wtmod4f=',real(wtmod4f),'wtmod=',real(wtmod)
cc        write(nout,*) ' log t=',real(dlog(-t))
cc        write(nout,*) 'costhe=',real(costhe)
c     @             ,' sqrt(wlambd)/sprim=',real(dsqrt(wlambd)/sprim)
c      write(nout,*)'sqrt s1=',real(sqrt(s1)),'sqrt s2=',real(sqrt(s2))
c     @            ,' sqrt sprim=',real(dsqrt(sprim))
c        write(nout,*)
c     @            '                        ',cthe3,cthe4
 3100 format(a26,4g22.14)   

      end

      subroutine ww_dumper(nout,svar,amel,wtcort)    
*     *******************************************    
c overweighted events monitoring, MS version
c 
c (Re)Written by: M.Skrzypek        date: 
c Last update:             by:  
c
      implicit DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'mm_phsp.inc'

      COMMON / momdec / q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      COMMON / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot
      SAVE   / momset /,/ momdec /

      dimension q12(4),q13(4),q14(4),q23(4),q24(4),q34(4),qtot(4)
      dimension q123(4),q124(4),q134(4),q234(4)
      dimension effbeam1(4),effbeam2(4)

      CALL set_eff_beams(svar,amel,effbeam1,effbeam2)

CC      write(nout,*) '================ WW_DUMPER ==================='
CC      write(nout,*) '--> wtcort = ',wtcort
CC      write(nout,*) '-----------'
      CALL DUMPL(nout,P1,P2,P3,P4,effbeam1,effbeam2,sphot,nphot)

      do k=1,4
        q12(k)=p1(k)+p2(k)-effbeam1(k)
        q13(k)=p1(k)+p3(k)-effbeam1(k)
        q14(k)=p1(k)+p4(k)-effbeam1(k)
        q23(k)=p2(k)+p3(k)-effbeam1(k)
        q24(k)=p2(k)+p4(k)-effbeam1(k)
        q34(k)=p3(k)+p4(k)-effbeam1(k)

        q123(k)=p1(k)+p2(k)+p3(k)-effbeam1(k)
        q124(k)=p1(k)+p2(k)+p4(k)-effbeam1(k)
        q134(k)=p1(k)+p3(k)+p4(k)-effbeam1(k)
        q234(k)=p2(k)+p3(k)+p4(k)-effbeam1(k)

      enddo
      xm12=sqrt(abs(dmas2(q12)))
      xm13=sqrt(abs(dmas2(q13)))
      xm14=sqrt(abs(dmas2(q14)))
      xm23=sqrt(abs(dmas2(q23)))
      xm24=sqrt(abs(dmas2(q24)))
      xm34=sqrt(abs(dmas2(q34)))

      xm123=sqrt(abs(dmas2(q123)))
      xm124=sqrt(abs(dmas2(q124)))
      xm134=sqrt(abs(dmas2(q134)))
      xm234=sqrt(abs(dmas2(q234)))

CC      write(nout,'(3(A,G14.4))') 't12 =',real(xm12),'t13 =',real(xm13)
CC     $              ,'t14 =',real(xm14)
CC      write(nout,'(3(A,G14.4))') 't23 =',real(xm23),'t24 =',real(xm24)
CC     $              ,'t34 =',real(xm34)
CC      write(nout,'(4(A,G14.4))')'t123=',real(xm123),'t124=',real(xm124)
CC     $              ,'t134=',real(xm134),'t234=',real(xm234) 
      do k=1,4

        q123(k)=p4(k)-effbeam1(k)
        q124(k)=p3(k)-effbeam1(k)
        q134(k)=p2(k)-effbeam1(k)
        q234(k)=p1(k)-effbeam1(k)

      enddo

      xm123=sqrt(abs(dmas2(q123)))
      xm124=sqrt(abs(dmas2(q124)))
      xm134=sqrt(abs(dmas2(q134)))
      xm234=sqrt(abs(dmas2(q234)))

CC      write(nout,'(4(A,G14.4))')'t4- =',real(xm123),'t3- =',real(xm124)
CC     $         ,'t2- =',real(xm134),'t1- =',real(xm234 )

      do k=1,4

        q123(k)=p4(k)-effbeam2(k)
        q124(k)=p3(k)-effbeam2(k)
        q134(k)=p2(k)-effbeam2(k)
        q234(k)=p1(k)-effbeam2(k)

      enddo

      xm123=sqrt(abs(dmas2(q123)))
      xm124=sqrt(abs(dmas2(q124)))
      xm134=sqrt(abs(dmas2(q134)))
      xm234=sqrt(abs(dmas2(q234)))

CC      write(nout,'(4(A,G14.4))')'t4+ =',real(xm123),'t3+ =',real(xm124)
CC     $         ,'t2+  =',real(xm134),'t1+  =',real(xm234 )

      do k=1,4
        q12(k)=p1(k)+p2(k)
        q13(k)=p1(k)+p3(k)
        q14(k)=p1(k)+p4(k)
        q23(k)=p2(k)+p3(k)
        q24(k)=p2(k)+p4(k)
        q34(k)=p3(k)+p4(k)

        q123(k)=p1(k)+p2(k)+p3(k)
        q124(k)=p1(k)+p2(k)+p4(k)
        q134(k)=p1(k)+p3(k)+p4(k)
        q234(k)=p2(k)+p3(k)+p4(k)

        qtot(k)=p1(k)+p2(k)+p3(k)+p4(k)
      enddo
      xm12=sqrt(dmas2(q12))
      xm13=sqrt(dmas2(q13))
      xm14=sqrt(dmas2(q14))
      xm23=sqrt(dmas2(q23))
      xm24=sqrt(dmas2(q24))
      xm34=sqrt(dmas2(q34))

      xm123=sqrt(dmas2(q123))
      xm124=sqrt(dmas2(q124))
      xm134=sqrt(dmas2(q134))
      xm234=sqrt(dmas2(q234))

      xmtot=sqrt(dmas2(qtot))
CC      write(nout,'(3(A,G14.4))') ' m12 =',real(xm12)
CC     $       ,' m13 =',real(xm13),' m14 =',real(xm14)
CC      write(nout,'(3(A,G14.4))') ' m23 =',real(xm23)
CC     $       ,' m24 =',real(xm24),' m34 =',real(xm34)
CC      write(nout,'(4(A,G14.4))') ' m123=',real(xm123)
CC     $       ,' m124=',real(xm124),' m134=',real(xm134)
CC     $                 ,' m234=',real(xm234) 
CC      write(nout,'(A,G14.4)') ' m1234=',real(xmtot)

      cthe1=cos(angle(effbeam1,p1))
      cthe2=cos(angle(effbeam1,p2))
      cthe3=cos(angle(effbeam1,p3))
      cthe4=cos(angle(effbeam1,p4))
CC      write(nout,3100)
CC     @    'decay c1e-,c2-n,c3n,c4e+ ',cthe1,cthe2,cthe3,cthe4

CC       write(nout,*) '        ============================'

 3100 format(a26,4g22.14)   

      end

