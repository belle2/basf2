*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  Vesk1                                  //
*//                                                                          //
*//   Purpose: generate one-dimensional arbitrary distribution rho(x)        //
*//   where x is in the range (0,1).                                         //
*//   Now Denser grid!!!!                                                    //
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
*
*
      SUBROUTINE Vesk1_Initialize(funsko,Npeaks,Xpeaks,crude)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Same as original Vesk1_Initialize_ori, with the option                 //
*//   of using a priori knowledge of peaks in funsko.                        //
*//                                                                          //
*//   Maarten Boonekamp, sept. 2001                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../bornv/Vesk1.fi'
*
      DOUBLE PRECISION  funsko, crude
      EXTERNAL funsko
*
      DOUBLE PRECISION      drvec(10)
      DOUBLE PRECISION      wt,sum,zmx,ymax
      INTEGER    j,k,jdiv
      INTEGER    iniran
C (M.B.)
      DOUBLE PRECISION      Xpeaks(*)
      INTEGER               Npeaks
      INTEGER i
C end (M.B.)
*-----------------------------------------------------------------------------
* initialisation part, see vinsko for more comments
      wt   = 0.d0
      swt  = 0.d0
      sswt = 0.d0
      nevs=  -911119
* initialisation part, sampling distribution funsko
* and filling matrices xx,yy,zint etc.
C (M.B.) standard initialisation (one interval) or modified one if
C        peaks are given
      IF(Npeaks.LE.0) THEN
        jmax =1
        xx(1)=0.d0
        xx(2)=1.d0
        yy(1)=funsko(xx(1))
        yy(2)=funsko(xx(2))
        IF(yy(1) .LT. 0.d0 .OR. yy(2) .LT. 0.d0) go to 999
        zint(1)=.5d0*(yy(2)+yy(1))*(xx(2)-xx(1))
      ELSE
        jmax = Npeaks + 1
        xx(1)=0.d0
        yy(1)= funsko(xx(1))
        IF(yy(1) .LT. 0.d0) go to 999
        DO k = 1, Npeaks
          xx(k+1) = Xpeaks(k)
          yy(k+1) = funsko(xx(k+1))
          IF(yy(k+1) .LT. 0.d0) go to 999
        ENDDO
        xx(Npeaks+2)=1.d0
        yy(Npeaks+2)=funsko(xx(Npeaks+2))
        IF(yy(Npeaks+2) .LT. 0.d0) go to 999
        DO k = 1, jmax
           zint(k)=.5d0*(yy(k+1)+yy(k))*(xx(k+1)-xx(k))
        ENDDO
      ENDIF
C end (M.B.)
*
      jdiv=1
      DO k=1,jlim2-1-Npeaks ! respect vector bounds (M.B.)
         IF(jmax .LT. jlim1) THEN
*  note that Vesk1_Divide increments jmax=jmax+1 in every CALL
            CALL Vesk1_Divide(funsko,jdiv)
            jdiv=jdiv+2
            IF(jdiv .GT. jmax) jdiv=1
         ELSE
            jdiv=1
            zmx=zint(1)
            DO j=1,jmax
               IF(zmx .LT. zint(j)) THEN
                  zmx=zint(j)
                  jdiv=j
               ENDIF
            ENDDO
            CALL Vesk1_Divide(funsko,jdiv)
         ENDIF
      ENDDO
*     
*...  final administration, normalizing zint etc.
      zsum =0.d0
      DO j=1,jmax
         ymax= max( yy(j+1),yy(j))
         zint(j)=ymax*(xx(j+1)-xx(j))
         zsum=zsum+zint(j)
      ENDDO
      sum=0.
      DO j=1,jmax
         sum=sum+zint(j)
         zint(j)=sum/zsum
      ENDDO
      crude = zsum
      RETURN
*
 999  WRITE(*,'(a)') ' **** STOP in vesk01, negative value of funsko '
      STOP
      END       ! Vesk1_Init_Peaks


      SUBROUTINE Vesk1_Initialize_ori(funsko,crude)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   original version, NOT USED ANY MORE !!!!!!!!!!
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Vesk1.fi'
*
      DOUBLE PRECISION  funsko, crude
      EXTERNAL funsko
*
      DOUBLE PRECISION      drvec(10)
      DOUBLE PRECISION      wt,sum,zmx,ymax
      INTEGER    j,k,jdiv
      INTEGER    iniran
*-----------------------------------------------------------------------------
* initialisation part, see vinsko for more comments
      wt   = 0.d0
      swt  = 0.d0
      sswt = 0.d0
      nevs=  -911119
* initialisation part, sampling distribution funsko
* and filling matrices xx,yy,zint etc.
      jmax =1
      xx(1)=0.d0
      xx(2)=1.d0
      yy(1)=funsko(xx(1))
      yy(2)=funsko(xx(2))
      IF(yy(1) .LT. 0.d0 .OR. yy(2) .LT. 0.d0) go to 999
      zint(1)=.5d0*(yy(2)+yy(1))*(xx(2)-xx(1))
*
      jdiv=1
      DO k=1,jlim2-1
         IF(jmax .LT. jlim1) THEN
*  note that Vesk1_Divide increments jmax=jmax+1 in every CALL
            CALL Vesk1_Divide(funsko,jdiv)
            jdiv=jdiv+2
            IF(jdiv .GT. jmax) jdiv=1
         ELSE
            jdiv=1
            zmx=zint(1)
            DO j=1,jmax
               IF(zmx .LT. zint(j)) THEN
                  zmx=zint(j)
                  jdiv=j
               ENDIF
            ENDDO
            CALL Vesk1_Divide(funsko,jdiv)
         ENDIF
      ENDDO
*     
*...  final administration, normalizing zint etc.
      zsum =0.d0
      DO j=1,jmax
         ymax= max( yy(j+1),yy(j))
         zint(j)=ymax*(xx(j+1)-xx(j))
         zsum=zsum+zint(j)
      ENDDO
      sum=0.
      DO j=1,jmax
         sum=sum+zint(j)
         zint(j)=sum/zsum
      ENDDO
      crude = zsum
      RETURN
*
 999  WRITE(*,'(a)') ' **** STOP in vesk01, negative value of funsko '
      STOP
      END       ! Vesk1_Initialize


      SUBROUTINE Vesk1_Divide(funsko,jdiv)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// this routine belongs to vesko1 package                                   //
*// it sudivides into two equal parts the interval                           //
*// (xx(jdiv),xx(jdiv+1))  in the 1-dim. latice                              //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Vesk1.fi'
*
      DOUBLE PRECISION  funsko
      EXTERNAL          funsko
      DOUBLE PRECISION  xnew
      INTEGER           j,jdiv
*----------------------------------------------------------------------------
      xnew=.5d0*(xx(jdiv) +xx(jdiv+1))
      DO j=jmax,jdiv,-1
         xx(j+2)  =xx(j+1)
         yy(j+2)  =yy(j+1)
         zint(j+1)=zint(j)
      ENDDO
      xx(jdiv+1)= xnew
      yy(jdiv+1)= funsko(xnew)
      IF(yy(jdiv+1) .LT. 0.) GOTO 999
      zint(jdiv)  =.5d0*(yy(jdiv+1)+yy(jdiv)  )*(xx(jdiv+1)-xx(jdiv)  )
      zint(jdiv+1)=.5d0*(yy(jdiv+2)+yy(jdiv+1))*(xx(jdiv+2)-xx(jdiv+1))
      jmax=jmax+1
      RETURN
 999  CONTINUE
      WRITE(*,'(a)') ' *** STOP Vesk1_Divide, negative value of funsko '
      STOP
      END


      SUBROUTINE Vesk1_Make(funsko,x,fn,wt)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Vesk1.fi'
*
      DOUBLE PRECISION  funsko
      EXTERNAL funsko
*
      DOUBLE PRECISION      fn,x,d,yymax,wt,rnumb
      REAL                  rvec(10)
      INTEGER    j,jstop,iwarm
      INTEGER    klower,krange,kurrent
*
      DATA iwarm /0/

* check if initialization was done (not trivial in old poor fortran)
      IF(iwarm .EQ. 0) THEN
         iwarm=1
         IF(nevs .EQ.  -911119 ) THEN
            nevs = 0
         ELSE
            GOTO 901
         ENDIF
      ENDIF
      
      CALL PseuMar_MakeVec(rvec,1)
      rnumb = rvec(1)
****----------------------------------------------
**   Old simple search method
****----------------------------------------------
**      DO j=1,jmax
**         jstop=j
**         IF(zint(j) .GT. rnumb) GOTO 216
**      ENDDO
** 216  CONTINUE
*====================================================
**  Equivalent faster Weierstrass-type search method
*====================================================
      klower   = 0
      krange   = jmax
 330  CONTINUE
      krange   = krange/2
      kurrent  = klower +krange
      IF(kurrent.EQ.0) GOTO 350
      IF(zint(kurrent) .LE. rnumb) THEN
         klower = kurrent
      ENDIF
      IF(krange.EQ.0) GOTO 350
      GOTO 330
 350  CONTINUE
      jstop = kurrent+1
*====================================================
      IF(jstop .EQ. 1) THEN
         d=rnumb/zint(1)
      ELSE
         d =(rnumb-zint(jstop-1))/(zint(jstop)-zint(jstop-1))
      ENDIF
      x=xx(jstop)*(1.d0 -d )+xx(jstop+1)*d
      fn=funsko(x)
      IF(fn .LT. 0.d0) GOTO 999
      yymax=max(yy(jstop+1),yy(jstop))
      wt=fn/yymax
      nevs=nevs+1
      swt=swt+wt
      sswt=sswt+wt*wt
*
      RETURN
 901  WRITE(*,'(a)') ' **** STOP in vesko1, lack of initialisation'
      STOP
 999  WRITE(*,'(a)') ' **** STOP in vesk01, negative value of funsko '
      STOP
      END       ! Vesk1_Make



      SUBROUTINE Vesk1_Finalize(cinteg,errint,zcrude)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Vesk1.fi'
*
      DOUBLE PRECISION     cinteg,errint,zcrude
*-----------------------------------------------------------------------------
      cinteg =0d0
      errint =0d0
      zcrude =0d0
      IF(nevs .GT. 0) cinteg=zsum*swt/float(nevs)
      IF(nevs .GT. 0) errint=sqrt(sswt/swt**2-1.d0/float(nevs))
      IF(nevs .GT. 0) zcrude=  zsum
      END       ! Vesk1_Finalize
*
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  Vesk1                                 //
*//////////////////////////////////////////////////////////////////////////////
