      SUBROUTINE betar(alfinv,wtborn,svar,amel,nphot,sphot,wtset)
!     *****************************************************************
! This routine defines weights for ISR QED matrix element up to O(alf3)
! to be implemented on top of basic distribution from karlud    
!     ***************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z) 
      PARAMETER( pi=3.1415926535897932d0)
      DIMENSION sphot(100,4),wtset(100)
      SAVE 

      DIMENSION bt10(100),bt11(100)
      DIMENSION bt20(100,100),bt21(100,100)
! ------------------ Inline functions ------------------
! Elements of single bremss. distribution
      chi(x)= (1d0+(1d0-x)**2)/2d0
      xni(x)= x*(-1+x/2d0 )
!
      DO i = 1,100
        wtset(i) = 0d0
      ENDDO
      IF (wtborn.EQ.0d0) RETURN

      beta00=0d0
      beta01=0d0
      beta02=0d0
      beta03=0d0

      beta10=0d0
      beta11=0d0
      beta12=0d0

      beta20=0d0
      beta21=0d0

      beta30=0d0

      gami=  2d0/alfinv/pi*(dlog(svar/amel**2)-1) 
!-- beta0
      beta00 = 1d0
      beta01 = 1d0 + gami/2d0     
      beta02 = 1d0 + gami/2d0 + gami**2/8d0 
      beta03 = 1d0 + gami/2d0 + gami**2/8d0 +gami**3/48d0

!-- beta1
!-- Contributions from beta1 
      ene=sqrt(svar/4d0)
      DO  jph=1,nphot
      CALL d_isr1(ene,gami,sphot,jph,dis10,dis11,dis12)
         vv=sphot(jph,4)/ene
         zz=1-vv
*     O(alf1) tree
         bt10(jph) = dis10 -beta00
         beta10 = beta10 +bt10(jph)
*     O(alf2) one loop
         bt11(jph) = dis11 -beta01
         beta11 = beta11 +bt11(jph)
*     O(alf3) two loop
         bt12   = dis12 -beta02
         beta12 = beta12 +bt12
      ENDDO

!-- beta2
      DO j2=2,nphot
         DO j1=1,j2-1
            dis20 = 0d0         !initialization
            dis21 = 0d0         !initialization
            CALL d_isr2(ene,gami,sphot,j1,j2,dis20,dis21)
            CALL d_isr2(ene,gami,sphot,j2,j1,dis20,dis21)
* O(alf2) Tree level
            bt20(j1,j2)  = dis20 -bt10(j1) -bt10(j2) -beta00
            beta20 = beta20 +bt20(j1,j2)  
* O(alf3) One loop level
            bt21(j1,j2)  = dis21 -bt11(j1) -bt11(j2) -beta01
            beta21 = beta21 +bt21(j1,j2)  
         ENDDO
      ENDDO

!-- beta3
* O(alf3) Tree level
         DO j3 = 3,nphot
            DO j2 = 2,j3-1
               DO j1 = 1,j2-1
                  dis30 = 0d0   !initialization
*     Sum over 6 fragmentation trees
                  CALL d_isr3(ene,gami,sphot,j1,j2,j3,dis30)
                  CALL d_isr3(ene,gami,sphot,j2,j1,j3,dis30)
                  CALL d_isr3(ene,gami,sphot,j1,j3,j2,dis30)
                  CALL d_isr3(ene,gami,sphot,j2,j3,j1,dis30)
                  CALL d_isr3(ene,gami,sphot,j3,j1,j2,dis30)
                  CALL d_isr3(ene,gami,sphot,j3,j2,j1,dis30)
                  bt30 = dis30
     $                 -beta00
     $                 -bt10(j1) -bt10(j2) -bt10(j3)
     $                 -bt20(j1,j2) -bt20(j1,j3) -bt20(j2,j3)
                  beta30 = beta30+bt30
               ENDDO
         ENDDO
      ENDDO

      wtx0=beta00
      wtx1=beta01 +beta10
      wtx2=beta02 +beta11 + beta20
      wtx3=beta03 +beta12 + beta21+ beta30
* Totals
      wtset(1) =wtborn*wtx0
      wtset(2) =wtborn*wtx1
      wtset(3) =wtborn*wtx2
      wtset(4) =wtborn*wtx3
* Betas
      wtset(10)=wtborn*beta00 ! O(alf0)
      wtset(11)=wtborn*beta01 !   O(alf1)
      wtset(12)=wtborn*beta10 !   O(alf1)
      wtset(13)=wtborn*beta02 ! O(alf2)
      wtset(14)=wtborn*beta11 ! O(alf2)
      wtset(15)=wtborn*beta20 ! O(alf2)
      wtset(16)=wtborn*beta03 !   O(alf3)
      wtset(17)=wtborn*beta12 !   O(alf3)
      wtset(18)=wtborn*beta21 !   O(alf3)
      wtset(19)=wtborn*beta30 !   O(alf3)

      END

      SUBROUTINE d_isr1(ene,gami,sphot,j1,dis10,dis11,dis12)
*     ******************************************************
*     ***********************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION sphot(100,4)
* inline functions
      chi2(a,b)=  0.5d0*   ((1d0-a)**2+(1d0-b)**2)
*
      a1 = (sphot(j1,4)-sphot(j1,3))/ene/2d0
      b1 = (sphot(j1,4)+sphot(j1,3))/ene/2d0
      zz = (1d0-a1)*(1d0-b1)
      IF(zz  .LE. 0d0) WRITE(*,*) '!!!! zz=',zz
      dels1 =  gami/2d0 -gami/4d0*dlog(zz)
      dels2 =  gami**2/8d0
     $        -gami**2/8d0  *dlog(zz)
     $        +gami**2/24d0 *dlog(zz)**2
* Exact O(alf1) matrix element for the hardest photon jhard
      dis10  = chi2(a1,b1)
      dis11  = dis10*(1+dels1)
      dis12  = dis10*(1+dels1+dels2)
      END


      SUBROUTINE d_isr2(ene,gami,sphot,j1,j2,dis20,dis21)
*     **************************************************
* dis20,dis21 has to be initialized in the calling program
*     ***********************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION sphot(100,4)
* inline functions
      chi2(a,b)=  0.5d0* ((1d0-a)**2+(1d0-b)**2)
*
      ntree = 2                 ! for 2 ISR fragmentation trees
      y1 = (sphot(j1,4)-sphot(j1,3))/ene/2d0
      z1 = (sphot(j1,4)+sphot(j1,3))/ene/2d0
      y2 = (sphot(j2,4)-sphot(j2,3))/ene/2d0
      z2 = (sphot(j2,4)+sphot(j2,3))/ene/2d0
      a1 = y1
      b1 = z1
      a2 = y2/(1d0-y1)
      b2 = z2/(1d0-z1)
      d20 = chi2(a1,b1)*chi2(a2,b2)/ntree
      zz1 =  (1-y1)*(1-z1)
      z1z2= (1-y1-y2)*(1-z1-z2)
* soft limit to d_isr1 OK! for 2 trees we get 3 terms gami/6d0*dlog(zz)
      delvir1 = gami/2d0 -gami/6d0*dlog(zz1) -gami/6d0*dlog(z1z2)
      dis20 = dis20 +d20
      dis21 = dis21 +d20*(1+delvir1)

      IF(z1  .le.0d0) WRITE(*,*) '!!!! z1=',z1
      IF(z1z2.le.0d0) WRITE(*,*) '!!!! z1z2=',z1z2

      END


      SUBROUTINE d_isr3(ene,gami,sphot,j1,j2,j3,dis30)
*     ************************************************
*     dis30 has to be initialized in the calling program
*     ***********************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION sphot(100,4)
* inline functions
      chi2(a,b)=  0.5d0*   ((1d0-a)**2+(1d0-b)**2)
*
      ntree = 6                 ! for 6 ISR fragmentation trees
      y1 = (sphot(j1,4)-sphot(j1,3))/ene/2d0
      z1 = (sphot(j1,4)+sphot(j1,3))/ene/2d0
      y2 = (sphot(j2,4)-sphot(j2,3))/ene/2d0
      z2 = (sphot(j2,4)+sphot(j2,3))/ene/2d0
      y3 = (sphot(j3,4)-sphot(j3,3))/ene/2d0
      z3 = (sphot(j3,4)+sphot(j3,3))/ene/2d0
      a1 = y1
      b1 = z1
      a2 = y2/(1d0-y1)
      b2 = z2/(1d0-z1)
      a3 = y3/(1d0-y2-y1)
      b3 = z3/(1d0-z2-z1)

      d30= chi2(a1,b1) *chi2(a2,b2) *chi2(a3,b3)/ntree

      dis30= dis30 +d30

      IF(a2  .GT. 1d0) WRITE(*,*) '!!!! a2=',a2
      IF(b2  .GT. 1d0) WRITE(*,*) '!!!! b2=',b2
      IF(a3  .GT. 1d0) WRITE(*,*) '!!!! a3=',a3
      IF(b3  .GT. 1d0) WRITE(*,*) '!!!! b3=',b3

      END

**************************************************************
! !!!! OBSOLETE !!!!! !!!! OBSOLETE !!!!! !!!! OBSOLETE !!!!
! !!!! OBSOLETE !!!!! !!!! OBSOLETE !!!!! !!!! OBSOLETE !!!!
**************************************************************
      SUBROUTINE betax(alfinv,wtborn,svar,amel,nphot,sphot,wtset)
!     *****************************************************************
! O(alf2)LL weights for the beta0, beta1 and beta2,
! to be implemented on top of basic distribution from karlud    
!     ***************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      PARAMETER( pi=3.1415926535897932d0)
      DIMENSION sphot(100,4),wtset(100)
      SAVE 
! ------------------ Inline functions ------------------
! Elements of single bremss. distribution
      chi(x)= (1d0+(1d0-x)**2)/2d0
      xni(x)= x*(-1+x/2d0 )

      beta00=0d0
      beta01=0d0
      beta02=0d0
      beta10=0d0
      beta11=0d0
      beta20=0d0

      gami=  2d0/alfinv/pi*(dlog(svar/amel**2)-1) 
!-- beta0
      beta00 = 1d0
      beta01 = 1d0 + gami/2d0     
      beta02 = 1d0 + gami/2d0 + gami**2/8d0 

!-- beta1
!-- Contributions from beta1            
      ene=sqrt(svar/4d0)    
      DO  jph=1,nphot 
         vv=sphot(jph,4)/ene            
         b10 = xni(vv)
         b11 = xni(vv)*(1d0 +gami/2d0)
     $        -chi(vv)*(gami/4d0)*dlog(1d0-vv)
         beta10  =  beta10 +b10             
         beta11  =  beta11 +b11             
      ENDDO

!-- beta2
      DO i=1,nphot
         DO j=i+1,nphot
            v1=sphot(i,4)/ene
            v2=sphot(j,4)/ene
            v1st = v1/(1-v2)
            v2st = v2/(1-v1)
            IF ( sphot(i,3)*sphot(j,3) .LT. 0d0) THEN
!           OPPOSITE directions two photons
               dis2= chi(v1)*chi(v2)
            ELSE
!           SAME directions two photons
               dis2= 0.5d0*( chi(v1)*chi(v2st) + chi(v1st)*chi(v2) )
            ENDIF
            beta1i = xni(v1)
            beta1j = xni(v2)
            bt20  = dis2 -beta1i -beta1j -beta00
            beta20 = beta20 +bt20  
         ENDDO
      ENDDO

      wtx0=beta00
      wtx1=beta01 +beta10
      wtx2=beta02 +beta11 + beta20
* Totals OLD version
      wtset(5) =wtborn*wtx0
      wtset(6) =wtborn*wtx1
      wtset(7) =wtborn*wtx2

      END

