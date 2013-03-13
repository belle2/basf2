*///////////////////////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                                           //
*//                                       KinLib                                                              //
*//                                                                                                           //
*//                   Library of kinematics operations on Lorentz vectors and matrices                        //
*//                                                                                                           //
*//  This class is just collection of tools for Lorentz vectors and transformation                            //
*//  Probably in future will be incorporated in the corresponding classes                                     //
*//                                                                                                           //
*// KinLib_ThetaD(PX,p1,p2,q1,q2,Svar,CosTheta)  : Provides Svar and single cos(theta) among p1,p2 and q1,q2  //
*// KinLib_SvarThet(p1,p2,q1,q2,Svar,CosTheta)   : The same as ThetaD but Energy conservation required        //
*// KinLib_ThetaR(Qtot,pp1,pp2,qq1,qq2,cth11,cth12,cth21,cth22) : cos(theta) of four scattering angles        //
*// KinLib_phspc2(qq,am1,am2,q1,q2,wt)                     : Generates q1, q2 with masses am1,am2             //
*// KinLib_givpair(cmsene,am1,am2,p1,p2,beta,eta1,eta2)    : Constructs  p1,p2 along z-axis                   //
*// KinLib_DefPair(cmsene,am1,am2,p1,p2)                   : Constructs  p1,p2 along z-axis                   //
*// KinLib_RotEul(the,phi,pvec,qvec)                  : Theta-phi rotation  y-rot(the) z-rot(phi)             //
*// KinLib_RotEulInv(the,phi,pvec,qvec)               : Inverse of KinLib_RotEul                              //
*// KinLib_RotEuler(alfa,beta,gamma,pvec,qvec)        : Full Euler rotation R_3(alpha)*R_2(beta)*R_3(gamma)   //
*// KinLib_RotEulerInv(alfa,beta,gamma,pvec,qvec)     : Inverse of KinLib_RotEuler                            //
*// KinLib_BostQ(Mode,QQ,pp,r)                        : Boost to rest frame of arbitrary timelike QQ          //
*// KinLib_BostQQ(Mode,QQ,pp,r)                       : Boost to rest frame of arbitrary timelike QQ          //
*// KinLib_DefBostQQ(Mode,QQ,Lor)                     : defines Lorenz transformation Lor of BostQQ           //
*// KinLib_Rotor(k1,k2,ph1,pvec,qvec)                 : Rotation in any of the three planes k1,k2             //
*// KinLib_DefRotor(k1,k2,phi,Lor)                    : Defines rotation matrix of  KinLib_Rotor              //
*// KinLib_Boost(k,exe,pvec,qvec)           : Boost along k-th axis, exe=exp(eta), eta= hiperbolic velocity   //
*// KinLib_DefBoost(kaxis,exe,Lor)          : Defines boost matrix the same as in  KinLib_Boost               //
*// KinLib_RotTranspose(Lor)                          : Transpose spacelike part of matrix Lor                //
*// KinLib_LorCopy(Lor1,Lor2)                         : Copy Lor1 --> Lor2                                    //
*// KinLib_RotColumn(e1,e2,e3,Rot)          : Column-wise construction of rotation using 3 versors            //
*// KinLib_LorMult(Lor1,Lor2,Lor3)          : Multiply matrices Lor3=Lor1*Lor2 for Lorenz transformations     //
*// KinLib_VecTrasform(Lor,p,q)             : Multiply vector and matrix, q=Lor*p (Lorentz transform)         //
*// FUNCTION KinLib_AngPhi(x,y)             : calculates angle in (0,2*pi) range out of (x,y) vector          //
*// KinLib_VecPrint(nunit,word,pp)          : prints single momentum 'pp' on unit 'nunit' with comment 'word' //
*// KinLib_LorPrint(nunit,word,Lor)         : Print transformation matrix Lor                                 //
*//                                                                                                           //
*///////////////////////////////////////////////////////////////////////////////////////////////////////////////

      SUBROUTINE KinLib_ThetaD(PX,p1,p2,q1,q2,Svar,CosTheta)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Provides Svar and  ONE cos(theta) among p1,p2 and q1,q2                       //
*//   Energy conservation NOT required!!                                            //
*//   The angle is beteween 3-vectors (p1-p2) and (q1-q2) in PX rest frame.         //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION     PX(4),p1(4),p2(4),q1(4),q2(4)
      DOUBLE PRECISION     Svar,CosTheta
      DOUBLE PRECISION     pd(4),qd(4),a,b
      INTEGER   k
*
      DO k=1,4
         pd(k) = p1(k)-p2(k)
         qd(k) = q1(k)-q2(k)
      ENDDO
      Svar = PX(4)**2-PX(3)**2-PX(2)**2-PX(1)**2
      IF( Svar .LE. 0d0 ) THEN
         WRITE(*,*) '++++++++++ KinLib_ThetaDr: PX not timelike'
         STOP
      ENDIF
      a = PX(4)*pd(4) -PX(3)*pd(3) -PX(2)*pd(2) -PX(1)*pd(1)
      b = PX(4)*qd(4) -PX(3)*qd(3) -PX(2)*qd(2) -PX(1)*qd(1)
      DO k=1,4
         pd(k) = pd(k) - a*PX(k)/Svar
         qd(k) = qd(k) - b*PX(k)/Svar
      ENDDO
      a = pd(4)**2-pd(3)**2-pd(2)**2-pd(1)**2
      b = qd(4)**2-qd(3)**2-qd(2)**2-qd(1)**2
      IF( a*b .LE. 0d0 ) THEN
         WRITE(*,*) '++++++++++ KinLib_ThetaDr: a,b=',a,b
         STOP
      ENDIF
      CosTheta = -(qd(4)*pd(4) -qd(3)*pd(3) -qd(2)*pd(2) -qd(1)*pd(1))/DSQRT(ABS(a*b))
*
      IF( ABS(CosTheta) .GT. 1d0 ) THEN
         WRITE(*,*) '++++++++++ KinLib_ThetaDr: CosTheta= ',CosTheta
      ENDIF
*
      END

      SUBROUTINE KinLib_SvarThet(p1,p2,q1,q2,Svar,CosTheta)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Provides Svar and  ONE cos(theta) among p1,p2 and q1,q2                       //
*//   Energy conservation required!!                                                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION     p1(4),p2(4),q1(4),q2(4)
      DOUBLE PRECISION     Svar,CosTheta
*
      IF( ABS((p1(4)+p2(4)) - (q1(4)+q2(4))) .GT. 1d-50 ) THEN
         WRITE(*,*) '++++++++++ KinLib_ThetSvar: not conserved energy!!!!'
         STOP
      ENDIF
*
      Svar = (p1(4)+p2(4))**2-(p1(3)+p2(3))**2-(p1(2)+p2(2))**2-(p1(1)+p2(1))**2
      CosTheta=      (p1(3)*q1(3)+p1(2)*q1(2)+p1(1)*q1(1))
     $          /SQRT(p1(3)*p1(3)+p1(2)*p1(2)+p1(1)*p1(1))
     $          /SQRT(q1(3)*q1(3)+q1(2)*q1(2)+q1(1)*q1(1))
      END

      SUBROUTINE KinLib_ThetaR(Qtot,pp1,pp2,qq1,qq2,cth11,cth12,cth21,cth22)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Provides cos(theta) for four scattering angles among p1,p2 and q1,q2          //
*//   Angles are calculated in the rest frame of Qtot                               //
*//   Called in programs calculating Born(p1,p2,q1,q2) distribution.                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  cth11,cth12,cth21,cth22
      DOUBLE PRECISION  Qtot(*),pp1(*),pp2(*),qq1(*),qq2(*)
      DOUBLE PRECISION  p1(4),p2(4),q1(4),q2(4)
      DOUBLE PRECISION  q1d,q2d,p1d,p2d
*
* Boost to Z/gamma frame
      CALL KinLib_BostQ(1,Qtot,pp1,p1)
      CALL KinLib_BostQ(1,Qtot,pp2,p2)
      CALL KinLib_BostQ(1,Qtot,qq1,q1)
      CALL KinLib_BostQ(1,Qtot,qq2,q2)
* Calculate all four possible angles
      q1d=        sqrt(q1(1)**2 +q1(2)**2 +q1(3)**2)
      q2d=        sqrt(q2(1)**2 +q2(2)**2 +q2(3)**2)
      p1d=        sqrt(p1(1)**2 +p1(2)**2 +p1(3)**2)
      p2d=        sqrt(p2(1)**2 +p2(2)**2 +p2(3)**2)
      cth11 = (p1(1)*q1(1) +p1(2)*q1(2) +p1(3)*q1(3))/p1d/q1d
      cth12 =-(p1(1)*q2(1) +p1(2)*q2(2) +p1(3)*q2(3))/p1d/q2d
      cth21 =-(p2(1)*q1(1) +p2(2)*q1(2) +p2(3)*q1(3))/p2d/q1d
      cth22 = (p2(1)*q2(1) +p2(2)*q2(2) +p2(3)*q2(3))/p2d/q2d
c[[[
c      cth11 = (q1(1)*p1(1) +q1(2)*p1(2) +q1(3)*p1(3))/q1d/p1d
c      cth12 =-(q1(1)*p2(1) +q1(2)*p2(2) +q1(3)*p2(3))/q1d/p2d
c      cth21 =-(q2(1)*p1(1) +q2(2)*p1(2) +q2(3)*p1(3))/q2d/p1d
c      cth22 = (q2(1)*p2(1) +q2(2)*p2(2) +q2(3)*p2(3))/q2d/p2d
c]]]
      END

      SUBROUTINE KinLib_phspc2(qq,am1,am2,q1,q2,wt)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*// Generates q1, q2 with masses am1,am2, such that qq = q1+q2                   //
*// In the qq rest frame spherical density is flat                               //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  qq(*),q1(*),q2(*),am1,am2,wt
      DOUBLE PRECISION  pi
      PARAMETER( pi = 3.1415926535897932d0)
      DOUBLE PRECISION  cmsene,cth,phi,beta,eta1,eta2,the
      REAL              rvec(10)

      cmsene= sqrt(qq(4)**2-qq(3)**2-qq(2)**2-qq(1)**2)
      CALL PseuMar_MakeVec(rvec,2)
      cth= 1.d0 -2.d0*rvec(1)
      the= acos(cth)
      phi= 2.d0*pi*rvec(2)
      CALL KinLib_givpair(cmsene,am1,am2,q1,q2,beta,eta1,eta2)
      CALL KinLib_RotEul(the,phi,q1,q1)
      CALL KinLib_BostQ(  -1,qq,q1,q1)
      CALL KinLib_RotEul(the,phi,q2,q2)
      CALL KinLib_BostQ(  -1,qq,q2,q2)
      wt = beta/2d0
      END

      SUBROUTINE KinLib_givpair(cmsene,am1,am2,p1,p2,beta,eta1,eta2)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//  For CMS energy = cmsene it defines two 'decay' momenta p1,p2                //
*//  in their rest frame, along z-axix                                           //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  cmsene,am1,am2,beta,eta1,eta2
      DOUBLE PRECISION  p1(*),p2(*)
      DOUBLE PRECISION  ener,svar
*
      ener  =  cmsene/2d0
      svar  =  cmsene**2
      beta  =  sqrt((svar-(am1-am2)**2)
     $             *(svar-(am1+am2)**2))/svar
      eta1=    (svar+am1**2-am2**2)/svar
      eta2  =  (svar-am1**2+am2**2)/svar

      p1(1)  =  0d0
      p1(2)  =  0d0
      p1(3)  =  ener*beta
      p1(4)  =  ener*eta1

      p2(1)  =  0d0
      p2(2)  =  0d0
      p2(3)  = -ener*beta
      p2(4)  =  ener*eta2
      END

      SUBROUTINE KinLib_DefPair(cmsene,am1,am2,p1,p2)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//  For CMS energy = cmsene it defines two 'decay' momenta p1,p2                //
*//  in their rest frame, along z-axix                                           //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  cmsene,am1,am2,beta,eta1,eta2
      DOUBLE PRECISION  p1(*),p2(*)
      DOUBLE PRECISION  ener,svar
*
      ener  =  cmsene/2d0
      svar  =  cmsene**2
      beta  =  sqrt((svar-(am1-am2)**2)
     $             *(svar-(am1+am2)**2))/svar
      eta1=    (svar+am1**2-am2**2)/svar
      eta2  =  (svar-am1**2+am2**2)/svar

      p1(1)  =  0d0
      p1(2)  =  0d0
      p1(3)  =  ener*beta
      p1(4)  =  ener*eta1

      p2(1)  =  0d0
      p2(2)  =  0d0
      p2(3)  = -ener*beta
      p2(4)  =  ener*eta2
      END

      SUBROUTINE KinLib_RotEul(the,phi,pvec,qvec)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//      Theta-phi rotation, it turns vector r along z-axis into r(theta,phi)    //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  the,phi,pvec(4),qvec(4)

      CALL KinLib_Rotor(3,1,the,pvec,qvec) ! y-rotation
      CALL KinLib_Rotor(1,2,phi,qvec,qvec) ! z-rotation
      END

      SUBROUTINE KinLib_RotEulInv(the,phi,pvec,qvec)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//      Inverse of KinLib_RotEul                                                //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  the,phi,pvec(4),qvec(4)

      CALL KinLib_Rotor(1,2,-phi,pvec,qvec) ! z-rotation
      CALL KinLib_Rotor(3,1,-the,qvec,qvec) ! y-rotation
      END

      SUBROUTINE KinLib_RotEuler(alfa,beta,gamma,pvec,qvec)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//      Full Euler rotation R_3(alpha)*R_2(beta)*R_3(gamma)                     //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  pvec(4),qvec(4),alfa,beta,gamma
*
      CALL KinLib_Rotor(1,2, alfa,  pvec,qvec) ! z-rotation
      CALL KinLib_Rotor(3,1, beta,  qvec,qvec) ! y-rotation
      CALL KinLib_Rotor(1,2, gamma, qvec,qvec) ! z-rotation
      END

      SUBROUTINE KinLib_RotEulerInv(alfa,beta,gamma,pvec,qvec)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//      Inverse of KinLib_RotEuler                                              //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  pvec(4),qvec(4),alfa,beta,gamma
*
      CALL KinLib_Rotor(1,2, -gamma, pvec,qvec) ! z-rotation
      CALL KinLib_Rotor(3,1, -beta,  qvec,qvec) ! y-rotation
      CALL KinLib_Rotor(1,2, -alfa,  qvec,qvec) ! z-rotation
      END

      SUBROUTINE KinLib_BostQ(Mode,QQ,pp,r)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*// Boost along arbitrary axis (as implemented by Ronald Kleiss).                //
*// The method is described in textbook of Jackson on electrodynamics.           //
*// p boosted into r  from actual frame to rest frame of Q                       //
*// forth (Mode = 1) or back (Mode = -1).                                        //
*// Q must be a timelike, p may be arbitrary.                                    //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER   Mode
      DOUBLE PRECISION     QQ(*),pp(*),r(*)
      DOUBLE PRECISION     Q(4),p(4),fac,amq
      INTEGER   k
*
      DO k=1,4
         p(k)=pp(k)
         Q(k)=QQ(k)
      ENDDO
      amQ =dsqrt(Q(4)**2-Q(1)**2-Q(2)**2-Q(3)**2)
      IF    (Mode .EQ. -1) THEN
         r(4) = (p(1)*Q(1)+p(2)*Q(2)+p(3)*Q(3)+p(4)*Q(4))/amQ
         fac  = (r(4)+p(4))/(Q(4)+amQ)
      ELSEIF(Mode .EQ.  1) THEN
         r(4) =(-p(1)*Q(1)-p(2)*Q(2)-p(3)*Q(3)+p(4)*Q(4))/amQ
         fac  =-(r(4)+p(4))/(Q(4)+amQ)
      ELSE
         WRITE(*,*) ' ++++++++ Wrong Mode in KinLib_BostQ ', Mode
         STOP
      ENDIF
      r(1)=p(1)+fac*Q(1)
      r(2)=p(2)+fac*Q(2)
      r(3)=p(3)+fac*Q(3)
      END

      SUBROUTINE KinLib_DefBostQQ(Mode,QQ,Lor)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   defines Lorenz transformation Lor, the same as in BostQQ                   //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      INTEGER  Mode
      DOUBLE PRECISION    QQ(4),Lor(4,4)
      INTEGER  k,l
      DOUBLE PRECISION    EQ,MQ
*---------------------------
      IF( IABS(MODE) .NE. 1 ) GOTO 900
      EQ = QQ(4)
      IF(EQ .LE. 0d0 ) GOTO 901
      MQ = QQ(4)**2-QQ(1)**2-QQ(2)**2-QQ(3)**2
      IF(MQ .LE. 0d0 ) GOTO 901
      MQ = DSQRT(MQ)
* Construct Lorenz transformation matrix
      DO k=1,3
         DO l=1,3
            Lor(k,l) = QQ(k)*QQ(l)/MQ/(MQ+EQ)
         ENDDO
      ENDDO
      DO k=1,3
         Lor(4,k) = -Mode*QQ(k)/MQ
         Lor(k,4) = -Mode*QQ(k)/MQ
         Lor(k,k) =  Lor(k,k) +1d0
      ENDDO
      Lor(4,4) = EQ/MQ
      RETURN
 900  WRITE(*,*) '++++++++ WRONG Mode in KinLib_BostQQ =',Mode
      STOP
 901  WRITE(*,*) '++++++++ WRONG QQ, not timelike'
      STOP
      END


      SUBROUTINE KinLib_BostQQ(Mode,QQ,pp,r)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*// Boost along arbitrary axis (as implemented by S.J.).                         //
*// The method is described in textbook of Jackson on electrodynamics.           //
*// p boosted into r  from actual frame to rest frame of Q                       //
*// forth (Mode = 1) or back (Mode = -1).                                        //
*// Q must be a timelike, p may be arbitrary.                                    //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      INTEGER  Mode
      DOUBLE PRECISION    QQ(*),pp(*),r(*)
      DOUBLE PRECISION    Lor(4,4),p(4)
      DOUBLE PRECISION    EQ,MQ,sum
      INTEGER  i,j,k,l
*---------------------------
      IF( IABS(MODE) .NE. 1 ) GOTO 900
      EQ = QQ(4)
      IF(EQ .LE. 0d0 ) GOTO 901
      MQ = QQ(4)**2-QQ(1)**2-QQ(2)**2-QQ(3)**2
      IF(MQ .LE. 0d0 ) GOTO 901
      MQ = DSQRT(MQ)
      DO i=1,4
         p(i)=pp(i)
      ENDDO
* Construct Lorenz transformation matrix
      DO k=1,3
         DO l=1,3
            Lor(k,l) = QQ(k)*QQ(l)/MQ/(MQ+EQ)
         ENDDO
      ENDDO
      DO k=1,3
         Lor(4,k) = -Mode*QQ(k)/MQ
         Lor(k,4) = -Mode*QQ(k)/MQ
         Lor(k,k) =  Lor(k,k) +1d0
      ENDDO
      Lor(4,4) = EQ/MQ

* Transform vector p, i.e. multiply by matrix Lor
      DO k=1,4
         sum = 0d0
         DO l=1,4
            sum=sum+ Lor(k,l)*p(l)
         ENDDO
         r(k) = sum
      ENDDO
*
      RETURN
 901  WRITE(*,*) '++++++++ KinLib_BostQQ: WRONG QQ, not timelike'
      STOP
 900  WRITE(*,*) '++++++++ KinLib_BostQQ: WRONG Mode = ',Mode
      STOP
      END

      SUBROUTINE KinLib_DefRotor(k1,k2,phi,Lor)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Defines rotation matrix the same as in  KinLib_Rotor                       //
*//   (k1,k2)= (1,2), x-y plane, around z axis                                   //
*//   (k1,k2)= (2,3), y-z plane, around x axis                                   //
*//   (k1,k2)= (3,1), x-z plane, around y axis                                   //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER k1,k2
      DOUBLE PRECISION  phi,Lor(4,4)
      INTEGER i,j,k
*
      DO i=1,4
         DO j=1,4
            Lor(i,j) = 0d0
         ENDDO
         Lor(i,i) = 1d0
      ENDDO
      Lor(k1,k1) =  cos(phi)
      Lor(k2,k2) =  cos(phi)
      Lor(k1,k2) = -sin(phi)
      Lor(k2,k1) =  sin(phi)  
      END

      SUBROUTINE KinLib_Rotor(k1,k2,ph1,pvec,qvec)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//  This is for rotation in any of the three planes                             //
*//  Rotation of (k1,k2) components, with copying of other                       //
*//   (k1,k2)= (1,2), x-y plane, around z axis                                   //
*//   (k1,k2)= (2,3), y-z plane, around x axis                                   //
*//   (k1,k2)= (3,1), x-z plane, around y axis                                   //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER k1,k2
      DOUBLE PRECISION   ph1,pvec(4),qvec(4)
      INTEGER k
      DOUBLE PRECISION   cs,sn,rvec1,rvec2,phi

      phi=ph1
      cs=cos(phi)
      sn=sin(phi)
      DO k=1,4
         qvec(k)=pvec(k)
      ENDDO
      rvec1  = pvec(k1)
      rvec2  = pvec(k2)
      qvec(k1)= cs*rvec1-sn*rvec2
      qvec(k2)= sn*rvec1+cs*rvec2
      END

      SUBROUTINE KinLib_DefBoost(kaxis,exe,Lor)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Defines boost matrix the same as in  KinLib_Boost                          //
*//   boost along k-th axis, exe=exp(eta), eta= hiperbolic velocity.             //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER kaxis
      DOUBLE PRECISION   exe,Lor(4,4)
      INTEGER i,j,k
      DOUBLE PRECISION   ch,sh
*
      IF(exe .LT. 0d0 ) THEN
         WRITE(*,*) '+++++++ KinLib_DefBoost: Wrong exe= ',exe
         STOP
      ENDIF
      DO i=1,4
         DO j=1,4
            Lor(i,j) = 0d0
         ENDDO
         Lor(i,i) = 1d0
      ENDDO
      ch = (exe +1d0/exe)/2d0
      sh = (exe -1d0/exe)/2d0
      k=kaxis
      Lor(4,4) = ch
      Lor(k,k) = ch
      Lor(4,k) = sh
      Lor(k,4) = sh
      END

      SUBROUTINE KinLib_Boost(k,exe,pvec,qvec)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*// boost along k-th axis, exe=exp(eta), eta= hiperbolic velocity.               //
*// unboosted components are copied                                              //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER  k
      DOUBLE PRECISION    exe
      DOUBLE PRECISION    pvec(4),qvec(4)
      DOUBLE PRECISION    rpl,rmi,qpl,qmi
      INTEGER  i
*
      IF(exe .LT. 0d0 ) THEN
         WRITE(*,*) '+++++++ KinLib_Boost: Wrong exe= ',exe
         STOP
      ENDIF
      DO i=1,4
         qvec(i)=pvec(i)
      ENDDO
      rpl=pvec(4)+pvec(k)
      rmi=pvec(4)-pvec(k)
      qpl=rpl*exe
      qmi=rmi/exe
      qvec(k)=(qpl-qmi)/2
      qvec(4)=(qpl+qmi)/2
      END


      SUBROUTINE KinLib_RotTranspose(Lor)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Transpose spacelike part of matrix Lor                                     //
*//   Used for getting inverse of the rotation matrix                            //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  Lor(4,4),x
      INTEGER  k,l
*-------
      DO k=1,2
         DO l=k+1,3
            x= Lor(k,l)
            Lor(k,l) = Lor(l,k)
            Lor(l,k) = x
         ENDDO
      ENDDO
      END

      SUBROUTINE KinLib_LorCopy(Lor1,Lor2)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Copy Lor1 --> Lor2                                                         //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  Lor1(4,4),Lor2(4,4)
      INTEGER  k,l
*-------
      DO k=1,4
         DO l=1,4
c[[[            Lor2(k,l) = Lor1(k,l)
            Lor2(k,l) = Lor1(k,l)
         ENDDO
      ENDDO
      END

      SUBROUTINE KinLib_RotColumn(e1,e2,e3,Rot)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Column-wise construction of rotation using 3 versors                       //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  Rot(4,4)
      DOUBLE PRECISION  e1(4),e2(4),e3(4)
      INTEGER  k,l
*-------
      Rot(4,4) = 1d0
      DO k=1,3
         Rot(k,4) =0d0
         Rot(4,k) =0d0
      ENDDO
      Rot(1,1) = e1(1)
      Rot(2,1) = e1(2)
      Rot(3,1) = e1(3)
      Rot(1,2) = e2(1)
      Rot(2,2) = e2(2)
      Rot(3,2) = e2(3)
      Rot(1,3) = e3(1)
      Rot(2,3) = e3(2)
      Rot(3,3) = e3(3)
      END


      SUBROUTINE KinLib_LorMult(Lor1,Lor2,Lor3)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Multiply matrices Lor3=Lor1*Lor2 for Lorenz transformations                //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION    Lor1(4,4),Lor2(4,4),Lor3(4,4)
      DOUBLE PRECISION    Lor(4,4)
      INTEGER  i,j,k,l
      DOUBLE PRECISION    sum
*-------
      DO i=1,4
         DO j=1,4
            sum=0d0
            DO k=1,4
               sum=sum+Lor1(i,k)*Lor2(k,j)
            ENDDO
            Lor(i,j)=sum
         ENDDO
      ENDDO
      DO i=1,4
         DO j=1,4
            Lor3(i,j) = Lor(i,j)
         ENDDO
      ENDDO
      END


      SUBROUTINE KinLib_VecTrasform(Lor,p,q)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Multiply vector and matrix, q=Lor*p                                        //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION    Lor(4,4),p(4),q(4)
      INTEGER  k,l
      DOUBLE PRECISION    pp(4),sum
*-------
      DO k=1,4
         pp(k)=p(k)
      ENDDO
      DO k=1,4
         sum=0d0
         DO l=1,4
            sum=sum+Lor(k,l)*pp(l)
         ENDDO
         q(k)=sum
      ENDDO
      END


      DOUBLE PRECISION  FUNCTION KinLib_AngPhi(x,y)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//     calculates angle in (0,2*pi) range out of (x,y) vector                   //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  x,y
      DOUBLE PRECISION  pi
      PARAMETER( pi =3.1415926535897932d0)
      DOUBLE PRECISION  the

      IF(ABS(y) .LT. ABS(x)) THEN
        the = ATAN(abs(y/x))
        IF(x .LE. 0d0) the=pi-the
      ELSE
        the = ACOS(x/sqrt(x**2+y**2))
      ENDIF
      IF(y .LT. 0d0) the = 2d0*pi-the
      KinLib_AngPhi = the
      END

      SUBROUTINE KinLib_VecPrint(nunit,word,pp)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   prints single momentum 'pp' on unit 'nunit' with comment 'word'         //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER      nunit
      CHARACTER*8  word
      DOUBLE PRECISION        pp(4),ams
      INTEGER      i
*----
      ams = pp(4)**2 -pp(3)**2 -pp(2)**2 -pp(1)**2
      IF(ams .GT. 0.0) ams = SQRT(ams)
      WRITE(nunit,'(a8,5(1x,f20.13))') word,(pp(i),i=1,4),ams
      END

      SUBROUTINE KinLib_LorPrint(nunit,word,Lor)
*//////////////////////////////////////////////////////////////////////////////////
*//                                                                              //
*//   Print transformation matrix Lor                                            //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER      nunit
      CHARACTER*8  word
      DOUBLE PRECISION  Lor(4,4)
      INTEGER k,j

      WRITE(nunit,'(4a)') '::::::::::::::::::::::::::::',
     $                    ' Lorenz Transformation ',word,
     $                    ' :::::::::::::::::::::::::::'
      DO k=1,4
         WRITE(nunit,'(6f20.14)') (Lor(k,j), j=1,4)
      ENDDO
      END
*///////////////////////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                                           //
*//                          End of CLASS  KinLib                                                             //
*///////////////////////////////////////////////////////////////////////////////////////////////////////////////
