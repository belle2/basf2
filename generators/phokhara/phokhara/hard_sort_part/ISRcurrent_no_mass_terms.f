      subroutine NLO_ISR_no_mass_terms(qq,mplus,mminus,
     1 mplus2,mminus2,rk1,eck1)
      include '../phokhara_10.0.inc'
      real*8 rk1(4),qq,qq2,m2
      complex*16 eck1(4),V1(15),ii
      complex*16 cdilog,Ly1,Ly2
      complex*16 logy1,logy2
      real*8 y1,y2,p1(4),p2(4),q1(4),q2(4),by1,by2,p1k1,p2k1,q1k1,q2k1
      complex*16 gam(4),gampl1(2,2),gammi1(2,2)
      complex*16 mplus(2,2),mminus(2,2),k1pl(2,2),k1mi(2,2),
     1 eck1pl(2,2),eck1mi(2,2),p1eck1,p2eck1,
     2 I(2,2)
      complex*16 mplus2(2,2),mminus2(2,2)
      real*8 pvecpl(4),pvecmi(4),deny1,deny2
      complex*16 avecpl(4),bvecpl(4),cvecpl(4),
     1 avecmi(4),bvecmi(4),cvecmi(4)
      complex*16 k1pleck1mi(2,2),eck1plk1mi(2,2),
     1 k1mieck1pl(2,2),eck1mik1pl(2,2)
      complex*16 plapl(2,2),plami(2,2),miapl(2,2),miami(2,2),mm1(2,2),
     1 mm2(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 spart1pl(2,2),part1(2,2),nom1(2,2)
      complex*16 const
      complex*16 spart2pl(2,2),part2(2,2),nom2(2,2)
      complex*16 spart1mi(2,2),spart2mi(2,2),sfull(2,2)
      complex*16 Spl1(2,2),Smi1(2,2),Spl2(2,2),Smi2(2,2),
     1 Spl5(2,2),Smi5(2,2),Spl8(2,2),Smi8(2,2),
     2 Spl9(2,2),Smi9(2,2),Spl10(2,2),Smi10(2,2),Spl11(2,2),Smi11(2,2)
      complex*16 Spl1t(2,2),Smi1t(2,2),Spl2t(2,2),Smi2t(2,2),
     1 Spl5t(2,2),Smi5t(2,2),Spl8t(2,2),Smi8t(2,2),
     2 Spl9t(2,2),Smi9t(2,2),Spl10t(2,2),Smi10t(2,2),Spl11t(2,2),
     3 Smi11t(2,2)
      complex*16 Spl3(2,2),Smi3(2,2),Spl4(2,2),Smi4(2,2),
     1 Spl6(2,2),Smi6(2,2),Spl7(2,2),Smi7(2,2),
     2 Spl12(2,2),Smi12(2,2),Spl13(2,2),Smi13(2,2),
     3 Spl14(2,2),Smi14(2,2),Spl15(2,2),Smi15(2,2)
      complex*16 Spl3t(2,2),Smi3t(2,2),Spl4t(2,2),Smi4t(2,2),
     1 Spl6t(2,2),Smi6t(2,2),Spl7t(2,2),Smi7t(2,2),
     2 Spl12t(2,2),Smi12t(2,2),Spl13t(2,2),Smi13t(2,2),
     3 Spl14t(2,2),Smi14t(2,2),Spl15t(2,2),Smi15t(2,2)
      complex*16 temppl(2,2),tempmi(2,2)
      integer ll,mu
      complex*16 pvecplgam,pvecmigam,eck1gam,bvecplgam,bvecmigam
     1 ,cvecplgam,cvecmigam,p1gam,p2gam,gamgamc,k1gam
      complex*16 nn,N0,N1,N2,W8,W9
      real*8 mnt1,mnt2,mnt3,mnt4,mnt5,mnt6,mnt7,mnt8,mnt9,
     1 mnt10,mnt11,mnt12,mnt13,mnt14
      real*8 y1mi1,y2mi1,m2miny1,m2miny2
      complex*16 PionFormFactor,f1
      common/ratios/qq2,m2
      common/matrix_check2/mnt1,mnt2,mnt3,mnt4,mnt5,mnt6,mnt7,mnt8,mnt9,
     1 mnt10,mnt11,mnt12,mnt13,mnt14
    
 
       
       call plus(rk1,k1pl)
       call minus(rk1,k1mi)
       call cplus(eck1,eck1pl)
       call cminus(eck1,eck1mi)

       I(1,1)=dcmplx(1.d0,0.d0)
       I(2,2)=dcmplx(1.d0,0.d0)
       I(1,2)=dcmplx(0.d0,0.d0)
       I(2,1)=dcmplx(0.d0,0.d0)

c hadronic current  
      f1=  PionFormFactor(qq)     
      do mu = 0,3
         gam(mu+1) = dcmplx((momenta(6,mu)-momenta(7,mu)),0.d0)*f1
      enddo
c
      do ll=0,3
          p1(ll+1)=momenta(1,ll)
          p2(ll+1)=momenta(2,ll)
      enddo

         p1eck1=p1(1)*eck1(1)
         p2eck1=p2(1)*eck1(1)

         p1k1=p1(1)*rk1(1)
         p2k1=p2(1)*rk1(1)
      do ll=2,4
         p1k1=p1k1-p1(ll)*rk1(ll)
         p2k1=p2k1-p2(ll)*rk1(ll)
         p1eck1=p1eck1-p1(ll)*eck1(ll)
         p2eck1=p2eck1-p2(ll)*eck1(ll)
      enddo

      y1=2.d0*p1k1/Sp
      y2=2.d0*p2k1/Sp

      do mu=1,4
          pvecpl(mu)=p1(mu)+p2(mu)
          pvecmi(mu)=p1(mu)-p2(mu)
      enddo


      pvecplgam=pvecpl(1)*gam(1)
      pvecmigam=pvecmi(1)*gam(1)
      eck1gam=eck1(1)*gam(1)
      p1gam=p1(1)*gam(1)
      p2gam=p2(1)*gam(1)
      gamgamc=gam(1)*CONJG(gam(1))
      k1gam=rk1(1)*gam(1)
      do ll=2,4
      pvecplgam=pvecplgam-pvecpl(ll)*gam(ll)
      pvecmigam=pvecmigam-pvecmi(ll)*gam(ll)
      eck1gam=eck1gam-eck1(ll)*gam(ll)
      p1gam=p1gam-p1(ll)*gam(ll)
      p2gam=p2gam-p2(ll)*gam(ll)
      k1gam=k1gam-rk1(ll)*gam(ll)
      gamgamc=gamgamc-gam(ll)*CONJG(gam(ll))
      enddo

      
      ii=dcmplx(0.d0,1.d0)
      qq2=qq/Sp
      m2=me**2/Sp
      logy1=log(y1/qq2)+ii*pi
      logy2=log(y2/qq2)+ii*pi

      deny1=m2*(1.d0-qq2)-y1
      deny2=m2*(1.d0-qq2)-y2
      y1mi1=y1-1.d0
      m2miny1=m2-y1
      y2mi1=y2-1.d0
      m2miny2=m2-y2
      Ly1=cdilog(dcmplx(-y1/qq2,0.d0))-
     1 cdilog(dcmplx(1.d0-1.d0/qq2,0.d0))+log(qq2+y1)*logy1
      Ly2=cdilog(dcmplx(-y2/qq2,0.d0))-
     1 cdilog(dcmplx(1.d0-1.d0/qq2,0.d0))+ log(qq2+y2)*logy2

c---------------------COEFFICIENTS-------------------------------------
      V1(1)=2.d0*log(4.d0*w**2)*(log(m2)+1.d0)
     1 -4.d0*log(qq2)+3.d0*log(m2)
     2 +4.d0-2.d0/3.d0*pi**2
     3 -((y1+2.d0*y2)/y2*Ly1+y1*(1.d0+2.d0*y2)/(1.d0-y2)*logy1+
     4 (y2+2.d0*y1)/y1*Ly2+y2*(1.d0+2.d0*y1)/(1.d0-y1)*logy2)

      V1(2)  =
     & y1**(-2)*y2*Ly2 + y1**(-1) + 3.D0*y1**(-1)*Ly2 + y1**(-1)*y2*
     & logy2 + y2**(-1) + 3.D0*y2**(-1)*Ly1 + 2.D0*logy2*y1mi1**(-1) - 
     & 2.D0*logy1 - 3.D0*y2*logy2*y1mi1**(-1) + y1*y2**(-2)*Ly1 + y1*
     & y2**(-1)*logy1 - 2.D0*y1*logy2*y1mi1**(-1) - 3.D0*y1*logy1*
     & y2mi1**(-1) + Log(qq2)*y1**(-1) + Log(qq2)*y2**(-1)

       V1(5)  =
     & 2.D0*y1**(-2)*Ly2 + y1**(-2)*y2*Ly2 + y1**(-1) + y1**(-1)*Ly2 + 
     & 2.D0*y1**(-1)*logy2 + y1**(-1)*y2*logy2 - 2.D0*y2**(-2)*Ly1 - 
     & y2**(-1) - y2**(-1)*Ly1 - 2.D0*y2**(-1)*logy1 + 2.D0*logy2 - 2.D0
     & *logy1 - 3.D0*y2*logy2*y1mi1**(-1) - y1*y2**(-2)*Ly1 - y1*
     & y2**(-1)*logy1 + 3.D0*y1*logy1*y2mi1**(-1) + 2.D0*Log(qq2)*
     & y1**(-1)*y2mi1**(-1) + Log(qq2)*y1**(-1) - 2.D0*Log(qq2)*
     & y2**(-1)*y1mi1**(-1) - Log(qq2)*y2**(-1) - 2.D0*Log(qq2)*
     & y1mi1**(-1)*y2mi1**(-1) + 2.D0/( - 1.D0 + qq2)*Log(qq2)*y1**(-1)
     & *y2mi1**(-1) - 2.D0/( - 1.D0 + qq2)*Log(qq2)*y2**(-1)*
     & y1mi1**(-1) - 4.D0/( - 1.D0 + qq2)*Log(qq2)*y1mi1**(-1)*
     & y2mi1**(-1) - 4.D0/( - 1.D0 + qq2)*Log(qq2)*y1mi1**(-1)

       V1(8)  =
     &  - 4.D0*y1**(-3)*Ly2 + 4.D0*y1**(-3)*y2*Ly2 + 4.D0*y1**(-2)*Ly2
     &  - 4.D0*y1**(-2)*logy2 + 4.D0*y1**(-2)*y2*logy2 + 2.D0*y1**(-1)
     &  + 2.D0*y1**(-1)*logy2 + 2.D0*y1**(-1)*y2*logy2 - 4.D0*
     & y1mi1**(-2) - 2.D0*y1mi1**(-1) - 2.D0*logy2*y1mi1**(-1) - 2.D0*
     & y2*logy2*y1mi1**(-2) - 2.D0*y2*logy2*y1mi1**(-1) + 4.D0*Log(qq2)
     & *y1**(-2) + 4.D0/( - 1.D0 + qq2)*y1**(-1) - 4.D0/( - 1.D0 + qq2)
     & *y1mi1**(-2) - 4.D0/( - 1.D0 + qq2)*y1mi1**(-1) - 4.D0/( - 1.D0
     &  + qq2)*y2*y1mi1**(-2) + 4.D0/( - 1.D0 + qq2)*Log(qq2)*y1**(-2)
     &  - 4.D0/( - 1.D0 + qq2)*Log(qq2)*y1**(-1) - 4.D0/( - 1.D0 + qq2)
     & /( - 1.D0 + qq2)*Log(qq2)*y1**(-1)

       V1(9)  =
     &  - 2.D0*y1**(-2)*Ly2 - 2.D0*y1**(-1)*logy2 + 4.D0*y2**(-3)*Ly1
     &  + 2.D0*y2**(-2)*Ly1 + 4.D0*y2**(-2)*logy1 + 4.D0*y2**(-1)*
     & y1mi1**(-1) + 4.D0*y2**(-1)*logy1 + 6.D0*logy2*y1mi1**(-1) + 4.D0
     & *Log(qq2)*y2**(-2)*y1mi1**(-1) - 6.D0*Log(qq2)*y2**(-1)*
     & y1mi1**(-1) + 4.D0/( - 1.D0 + qq2)*y2**(-1)*y1mi1**(-1) + 4.D0/(
     &  - 1.D0 + qq2)*y1mi1**(-1) + 2.D0/( - 1.D0 + qq2)*Log(qq2)*
     & y1**(-1) + 4.D0/( - 1.D0 + qq2)*Log(qq2)*y2**(-2)*y1mi1**(-1) - 
     & 6.D0/( - 1.D0 + qq2)*Log(qq2)*y2**(-1)*y1mi1**(-1) - 6.D0/( - 1.D
     & 0 + qq2)*Log(qq2)*y1mi1**(-1) - 4.D0/( - 1.D0 + qq2)/( - 1.D0 + 
     & qq2)*Log(qq2)*y2**(-1)*y1mi1**(-1) - 4.D0/( - 1.D0 + qq2)/( - 1.D
     & 0 + qq2)*Log(qq2)*y1mi1**(-1)

       V1(10)  =
     &  - 4.D0*y1**(-3)*Ly2 - 2.D0*y1**(-2)*Ly2 - 4.D0*y1**(-2)*logy2
     &  - 4.D0*y1**(-1)*y2mi1**(-1) - 4.D0*y1**(-1)*logy2 + 2.D0*
     & y2**(-2)*Ly1 + 2.D0*y2**(-1)*logy1 - 6.D0*logy1*y2mi1**(-1) - 4.D
     & 0*Log(qq2)*y1**(-2)*y2mi1**(-1) + 6.D0*Log(qq2)*y1**(-1)*
     & y2mi1**(-1) - 4.D0/( - 1.D0 + qq2)*y1**(-1)*y2mi1**(-1) - 4.D0/(
     &  - 1.D0 + qq2)*y2mi1**(-1) - 4.D0/( - 1.D0 + qq2)*Log(qq2)*
     & y1**(-2)*y2mi1**(-1) + 6.D0/( - 1.D0 + qq2)*Log(qq2)*y1**(-1)*
     & y2mi1**(-1) - 2.D0/( - 1.D0 + qq2)*Log(qq2)*y2**(-1) + 6.D0/( - 
     & 1.D0 + qq2)*Log(qq2)*y2mi1**(-1) + 4.D0/( - 1.D0 + qq2)/( - 1.D0
     &  + qq2)*Log(qq2)*y1**(-1)*y2mi1**(-1) + 4.D0/( - 1.D0 + qq2)/(
     &  - 1.D0 + qq2)*Log(qq2)*y2mi1**(-1)

       V1(11)  =
     & 4.D0*y2**(-3)*Ly1 - 4.D0*y2**(-2)*Ly1 + 4.D0*y2**(-2)*logy1 - 2.D
     & 0*y2**(-1) - 2.D0*y2**(-1)*logy1 + 2.D0*y2mi1**(-1) + 2.D0*logy1
     & *y2mi1**(-1) - 4.D0*y1*y2**(-3)*Ly1 - 4.D0*y1*y2**(-2)*logy1 - 2.
     & D0*y1*y2**(-1)*logy1 + 2.D0*y1*logy1*y2mi1**(-2) + 2.D0*y1*logy1
     & *y2mi1**(-1) - 4.D0*Log(qq2)*y2**(-2) - 4.D0/( - 1.D0 + qq2)*
     & y2**(-1) - 4.D0/( - 1.D0 + qq2)*Log(qq2)*y2**(-2) + 4.D0/( - 1.D0
     &  + qq2)*Log(qq2)*y2**(-1) + 4.D0/( - 1.D0 + qq2)/( - 1.D0 + qq2)
     & *Log(qq2)*y2**(-1) 
c


       V1(1)=(1.d0-2.d0*alpha/4.d0/pi*V1(1))/Sp!dcmplx(1.d0,0.d0)-alpha/4.d0/pi*V1(1)
       V1(2)=-2.d0*alpha/4.d0/pi*V1(2)/Sp
       V1(3)=0.d0!-2.d0*alpha/4.d0/pi*V1(3)/Sp/dsqrt(Sp)
       V1(4)=0.d0!-2.d0*alpha/4.d0/pi*V1(4)/Sp/dsqrt(Sp)
       V1(5)=-2.d0*alpha/4.d0/pi*V1(5)/Sp
       V1(6)=0.d0!-2.d0*alpha/4.d0/pi*V1(6)/dsqrt(Sp)
       V1(7)=0.d0!-2.d0*alpha/4.d0/pi*V1(7)/dsqrt(Sp)
       V1(8)=-2.d0*alpha/4.d0/pi*V1(8)/(Sp)
       V1(9)=-2.d0*alpha/4.d0/pi*V1(9)/(Sp)
       V1(10)=-2.d0*alpha/4.d0/pi*V1(10)/(Sp)
       V1(11)=-2.d0*alpha/4.d0/pi*V1(11)/(Sp)
       V1(12)=0.d0!-2.d0*alpha/4.d0/pi*V1(12)/dsqrt(Sp)
       V1(13)=0.d0!-2.d0*alpha/4.d0/pi*V1(13)/dsqrt(Sp)
       V1(14)=0.d0!-2.d0*alpha/4.d0/pi*V1(14)/Sp/dsqrt(Sp)
cST12
c----------------------------------------------------------------------
c                           Matrices
c ---------------------------------------------------------------------
c
       call matr(eck1pl,k1mi,eck1plk1mi)
       call matr(eck1mi,k1pl,eck1mik1pl)

       call matr(k1pl,eck1mi,k1pleck1mi)
       call matr(k1mi,eck1pl,k1mieck1pl)

       call conmat(2.d0*p1eck1,I,mm1)
       call conmat(2.d0*p2eck1,I,mm2)

       call cplus(gam,gampl1)
       call cminus(gam,gammi1)

c       S(1) 
c +         
        call minmat(mm1,eck1plk1mi,nom1)
        call matr(nom1,gampl1,part1)
        call conmat(dcmplx(1.d0/y1,0.d0),part1,spart1pl)

        call minmat(mm2,k1mieck1pl,nom2)
        call matr(gampl1,nom2,part2)
        call conmat(dcmplx(1.d0/y2,0.d0),part2,spart2pl)

        call minmat(spart1pl,spart2pl,Spl1t)
c -
        call minmat(mm1,eck1mik1pl,nom1)
        call matr(nom1,gammi1,part1)
        call conmat(dcmplx(1.d0/y1,0.d0),part1,spart1mi)

        call minmat(mm2,k1pleck1mi,nom2)
        call matr(gammi1,nom2,part2)
        call conmat(dcmplx(1.d0/y2,0.d0),part2,spart2mi)

        call minmat(spart1mi,spart2mi,Smi1t)

        
c       S(2)   
c+       
        call matr(k1pleck1mi,gampl1,part1)
        call matr(gampl1,eck1mik1pl,part2)

        call minmat(part1,part2,sfull)
        call conmat(dcmplx(1.d0/2.d0,0.d0),sfull,Spl2t)
c-       
        call matr(k1mieck1pl,gammi1,part1)
        call matr(gammi1,eck1plk1mi,part2)

        call minmat(part1,part2,sfull)
        call conmat(dcmplx(1.d0/2.d0,0.d0),sfull,Smi2t)

c     S(3)
c+
        call conmat(p1gam,k1pleck1mi,Spl3t)

c-
        call conmat(p1gam,k1mieck1pl,Smi3t)

c     S(4)
c+
        call conmat(p2gam,k1pleck1mi,Spl4t)

c-
        call conmat(p2gam,k1mieck1pl,Smi4t)

c   S(5)
c+
        call conmat(pvecplgam,eck1pl,part1)
        call conmat(eck1gam,k1pl,part2)
        call minmat(part2,part1,Spl5t)
c-
        call conmat(pvecplgam,eck1mi,part1)
        call conmat(eck1gam,k1mi,part2)
        call minmat(part2,part1,Smi5t)

c S(6)
        const=2.d0*p1eck1/Sp
        call conmat(const,k1pl,spart1pl)
        call conmat(dcmplx(y1,0.d0),eck1pl,spart2pl)
        call minmat(spart1pl,spart2pl,temppl)

        call conmat(const,k1mi,spart1mi)
        call conmat(dcmplx(y1,0.d0),eck1mi,spart2mi)        
        call minmat(spart1mi,spart2mi,tempmi)

c+
        call matr(temppl,gammi1,part1)
        call matr(gampl1,tempmi,part2)
        call minmat(part1,part2,Spl6t)
c-

        call matr(tempmi,gampl1,part1)
        call matr(gammi1,temppl,part2)
        call minmat(part1,part2,Smi6t)

c S(7)

        const=2.d0*p2eck1/Sp
        call conmat(const,k1pl,spart1pl)
        call conmat(dcmplx(y2,0.d0),eck1pl,spart2pl)
        call minmat(spart1pl,spart2pl,temppl)

        call conmat(const,k1mi,spart1mi)
        call conmat(dcmplx(y2,0.d0),eck1mi,spart2mi)        
        call minmat(spart1mi,spart2mi,tempmi)

c+
        call matr(temppl,gammi1,part1)
        call matr(gampl1,tempmi,part2)
        call minmat(part1,part2,Spl7t)
c-
        call matr(tempmi,gampl1,part1)
        call matr(gammi1,temppl,part2)
        call minmat(part1,part2,Smi7t)

c  S(8) 
c+       
         const=2.d0*p1eck1/Sp
        call conmat(const,k1pl,part1)
        call conmat(dcmplx(y1,0.d0),eck1pl,part2)
        call minmat(part1,part2,sfull)
        call conmat(p1gam,sfull,Spl8t)

 
c-

        call conmat(const,k1mi,part1)
        call conmat(dcmplx(y1,0.d0),eck1mi,part2)
        call minmat(part1,part2,sfull)
        call conmat(p1gam,sfull,Smi8t)

        
        

c  S(9) 
c+      
        const=2.d0*p2eck1/Sp
        call conmat(const,k1pl,part1)
        call conmat(dcmplx(y2,0.d0),eck1pl,part2)
        call minmat(part1,part2,sfull)
        call conmat(p1gam,sfull,Spl9t)
c-
        call conmat(const,k1mi,part1)
        call conmat(dcmplx(y2,0.d0),eck1mi,part2)
        call minmat(part1,part2,sfull)
        call conmat(p1gam,sfull,Smi9t)

c  S(10) 
c+
        const=2.d0*p1eck1/Sp
        call conmat(const,k1pl,part1)
        call conmat(dcmplx(y1,0.d0),eck1pl,part2)
        call minmat(part1,part2,sfull)
        call conmat(p2gam,sfull,Spl10t)
c-
        call conmat(const,k1mi,part1)
        call conmat(dcmplx(y1,0.d0),eck1mi,part2)
        call minmat(part1,part2,sfull)
        call conmat(p2gam,sfull,Smi10t)

c  S(11) 
c+
        const=2.d0*p2eck1/Sp
        call conmat(const,k1pl,part1)
        call conmat(dcmplx(y2,0.d0),eck1pl,part2)
        call minmat(part1,part2,sfull)
        call conmat(p2gam,sfull,Spl11t)
c-
        call conmat(const,k1mi,part1)
        call conmat(dcmplx(y2,0.d0),eck1mi,part2)
        call minmat(part1,part2,sfull)
        call conmat(p2gam,sfull,Smi11t)

c S(12)
c+
         const=2.d0*p1eck1*pvecplgam/Sp-y1*eck1gam
         call conmat(const,I,Spl12t)
c-
         call conmat(const,I,Smi12t)

c S(13)
c+
         const=2.d0*p2eck1*pvecplgam/Sp-y2*eck1gam
         call conmat(const,I,Spl13t)
c-
         call conmat(const,I,Smi13t)

c S(14)
c+
         const=2.d0*p1eck1*pvecmigam/y1-2.d0*p2eck1*pvecmigam/y2
         call conmat(const,I,Spl14t)
c-
         call conmat(const,I,Smi14t)
c---------------------------------------------------------------------
c---------------------------------------------------------------------        
        call conmat(V1(1),Spl1t,Spl1)
        call conmat(V1(2),Spl2t,Spl2)
        call conmat(V1(3),Spl3t,Spl3)
        call conmat(V1(4),Spl4t,Spl4)
        call conmat(V1(5),Spl5t,Spl5)
        call conmat(V1(6),Spl6t,Spl6)
        call conmat(V1(7),Spl7t,Spl7)
        call conmat(V1(8),Spl8t,Spl8)
        call conmat(V1(9),Spl9t,Spl9)
        call conmat(V1(10),Spl10t,Spl10)
        call conmat(V1(11),Spl11t,Spl11)
        call conmat(V1(12),Spl12t,Spl12)
        call conmat(V1(13),Spl13t,Spl13)
        call conmat(V1(14),Spl14t,Spl14)


        call conmat(V1(1),Smi1t,Smi1)
        call conmat(V1(2),Smi2t,Smi2)
        call conmat(V1(3),Smi3t,Smi3)
        call conmat(V1(4),Smi4t,Smi4)
        call conmat(V1(5),Smi5t,Smi5)
        call conmat(V1(6),Smi6t,Smi6)
        call conmat(V1(7),Smi7t,Smi7)
        call conmat(V1(8),Smi8t,Smi8)
        call conmat(V1(9),Smi9t,Smi9)
        call conmat(V1(10),Smi10t,Smi10)
        call conmat(V1(11),Smi11t,Smi11)
        call conmat(V1(12),Smi12t,Smi12)
        call conmat(V1(13),Smi13t,Smi13)
        call conmat(V1(14),Smi14t,Smi14)
         
       

        mplus(1,1)=-(Spl1(1,1)+Spl2(1,1)+Spl5(1,1)
     1 + Spl8(1,1)+Spl9(1,1)+Spl10(1,1)+Spl11(1,1))/qq!/Sp

       mplus(1,2)=-(Spl1(1,2)+Spl2(1,2)+Spl5(1,2)
     1 + Spl8(1,2)+Spl9(1,2)+Spl10(1,2)+Spl11(1,2))/qq!/Sp

       mplus(2,1)=-(Spl1(2,1)+Spl2(2,1)+Spl5(2,1)
     1 + Spl8(2,1)+Spl9(2,1)+Spl10(2,1)+Spl11(2,1))/qq!/Sp

       mplus(2,2)=-(Spl1(2,2)+Spl2(2,2)+Spl5(2,2)
     1 + Spl8(2,2)+Spl9(2,2)+Spl10(2,2)+Spl11(2,2))/qq!/Sp

       mminus(1,1)=-(Smi1(1,1)+Smi2(1,1)+Smi5(1,1)
     1 + Smi8(1,1)+Smi9(1,1)+Smi10(1,1)+Smi11(1,1))/qq!/Sp

       mminus(1,2)=-(Smi1(1,2)+Smi2(1,2)+Smi5(1,2)
     1 + Smi8(1,2)+Smi9(1,2)+Smi10(1,2)+Smi11(1,2))/qq!/Sp

       mminus(2,1)=-(Smi1(2,1)+Smi2(2,1)+Smi5(2,1)
     1 + Smi8(2,1)+Smi9(2,1)+Smi10(2,1)+Smi11(2,1))/qq!/Sp

       mminus(2,2)=-(Smi1(2,2)+Smi2(2,2)+Smi5(2,2)
     1 + Smi8(2,2)+Smi9(2,2)+Smi10(2,2)+Smi11(2,2))/qq!/Sp


       mplus2(1,1)=-(Spl3(1,1)+Spl4(1,1)+Spl6(1,1)+Spl7(1,1)+
     1 Spl12(1,1)+Spl13(1,1)+Spl14(1,1))/qq!/Sp

       mplus2(1,2)=-(Spl3(1,2)+Spl4(1,2)+Spl6(1,2)+Spl7(1,2)+
     1 Spl12(1,2)+Spl13(1,2)+Spl14(1,2))/qq!/Sp

       mplus2(2,1)=-(Spl3(2,1)+Spl4(2,1)+Spl6(2,1)+Spl7(2,1)+
     1 Spl12(2,1)+Spl13(2,1)+Spl14(2,1))/qq!/Sp

       mplus2(2,2)=-(Spl3(2,2)+Spl4(2,2)+Spl6(2,2)+Spl7(2,2)+
     1 Spl12(2,2)+Spl13(2,2)+Spl14(2,2))/qq!/Sp

       mminus2(1,1)=-(Smi3(1,1)+Smi4(1,1)+Smi6(1,1)+Smi7(1,1)+
     1 Smi12(1,1)+Smi13(1,1)+Smi14(1,1))/qq!/Sp

       mminus2(1,2)=-(Smi3(1,2)+Smi4(1,2)+Smi6(1,2)+Smi7(1,2)+
     1 Smi12(1,2)+Smi13(1,2)+Smi14(1,2))/qq!/Sp

       mminus2(2,1)=-(Smi3(2,1)+Smi4(2,1)+Smi6(2,1)+Smi7(2,1)+
     1 Smi12(2,1)+Smi13(2,1)+Smi14(2,1))/qq!/Sp

       mminus2(2,2)=-(Smi3(2,2)+Smi4(2,2)+Smi6(2,2)+Smi7(2,2)+
     1 Smi12(2,2)+Smi13(2,2)+Smi14(2,2))/qq!/Sp



cST12

      end

c      complex*16 function nn(yi,zz)
c      implicit none
c      real*8 yi,zz
c      real*8 m2,qq2
c      common/ratios/qq2,m2
c      
c      nn=m2/yi/(m2-yi)*(1.d0+zz*log(yi/m2))+m2/(m2-yi)**2*log(yi/m2)
c
c      end

c      complex*16 function N0(yi)
c      implicit none
c      real*8 yi,zz
c      real*8 m2,qq2,pi
c      complex*16 cdilog
c      common/ratios/qq2,m2
c
c      pi = 4.d0*dAtan(1.d0)
c
c      N0=log(qq2)*log(yi/m2)+cdilog(dcmplx(1.d0-qq2,0.d0))+
c     1 cdilog(dcmplx(1.d0-yi/m2,0.d0))-pi**2/6.d0
c
c      end

c      complex*16 function N1(yi)
c      implicit none
c      real*8 yi,zz
c      real*8 m2,qq2
c      complex*16 cdilog,N0
c      common/ratios/qq2,m2

c      N1=1.d0/qq2*log(yi/m2)+log(1.d0-qq2)/(1.d0-qq2)+
c     1 m2*N0(yi)/(m2*(1.d0-qq2)-yi)
c
c      end

c      complex*16 function N2(yi)
c      implicit none
c      real*8 yi,zz
c      real*8 m2,qq2
c      complex*16 cdilog,N1
c      common/ratios/qq2,m2

c      N2=1.d0/(2.d0*qq2*(1.d0-qq2))*(1.d0+(1.d0-qq2)/qq2*log(yi/m2)+
c     1 qq2*log(1.d0-qq2)/(1.d0-qq2))+m2*N1(yi)/(m2*(1.d0-qq2)-yi)

c      end

