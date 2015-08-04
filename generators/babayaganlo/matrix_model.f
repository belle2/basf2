      subroutine squared_matrix(model,ng,ecms,p1,p2,pbeam1,pbeam2,qph,
     >           ie,imtx,summt2,prodotto,deltah)
! written by CMCC, last modified 5/3/2006
      implicit double precision (a-h,o-z)
      character*10 model
      character*6 eikng ! multip single
      parameter (imaxph = 40)
      dimension iclose(imaxph)
      real*4 csi(1)
      double precision mass,lambda
      dimension p1(0:3),p2(0:3),pin1(0:3),pin2(0:3),qmost(0:3)
      dimension p1r(0:3),p2r(0:3),p1b(0:3),p2b(0:3),q(0:3)
      dimension q1(0:3),q2(0:3),q3(0:3),q4(0:3),qq(0:3),qr(0:3)
      dimension p1tmp(0:3),p2tmp(0:3),p3tmp(0:3),p4tmp(0:3)
      dimension pin1bl(0:3),pin2bl(0:3),p1bl(0:3),p2bl(0:3)
      dimension ptmp(0:3),ptmp2(0:3)
      dimension pbeam1(0:3),pbeam2(0:3)
      dimension qmostpt(imaxph,0:3)
      dimension p1rr(0:3),p2rr(0:3),pin1rr(0:3),pin2rr(0:3),prif(0:3)
      dimension qph(imaxph,0:3),qphtmp(imaxph,0:3),randvec(3)
      character*2 fs
      common/finalstate/fs
      common/iclosest/iclose
      common/momentainitial/pin1,pin2
      common/reducedtoborn/p1b,p2b,iref
      common/momentainitialred/pin1b(0:3),pin2b(0:3),pin1r(0:3),
     >     pin2r(0:3)
      common/forborncrosssection/phsp2b,flux2b,bornme,bornmeq2,bornmez,
     > bornmenovp 
      common/radpattern/nph(4)
      common/parameters/ame,ammu,convfac,alpha,pi
      common/epssoft/eps
      common/fortests/prod
      common/for_debug/ppp(0:3),denphsp,dir3(0:3),dir4(0:3)
      common/singlefactors/singleeikfact(imaxph)
      common/idarkon/idarkon
      common/ialpharunning/iarun
      double complex vpolc
      external vpolc
!     imtx is to say that the matrix el. has been calculated..
! summt2 is the squared ME summed over all all spins

      integer iteubn
      common/teubner/iteubn
      
** VP UNC      
      dimension iswvperr(3),iswvperrt(6)
      common/iswitchvperror/iswvperr,iswvperrt
      parameter (nvps = 26)
      dimension remtx(nvps)
      common/rationsvpunc/remtx,invp
      common/rescaledhaderr/scal_err_dhad

      parameter (nvpst = 728)
      dimension remtxt(nvpst)
      integer ist(nvpst,6)
      common/rationsvpunct/remtxt,invpt,ist
***   
      common/firstsqmatrix/ifirst
      data ifirst /1/

      if (ifirst.eq.1) then
         iii = 0
         do i1 = -1,1
         do i2 = -1,1
         do i3 = -1,1
         do i4 = -1,1
         do i5 = -1,1
         do i6 = -1,1
            if (i1.ne.0.or.i2.ne.0.or.i3.ne.0.or.
     >           i4.ne.0.or.i5.ne.0.or.i6.ne.0) then
               iii = iii + 1
               ist(iii,1) = i1
               ist(iii,2) = i2
               ist(iii,3) = i3
               ist(iii,4) = i4
               ist(iii,5) = i5
               ist(iii,6) = i6
            endif
         enddo
         enddo
         enddo
         enddo
         enddo
         enddo

         ifirst = 0
      endif
      
      iarunbck = iarun
      
      imtx  = 0
      summt2   = 0.d0
      prodotto = 1.d0
      deltah   = 0.d0
      bornme   = 1.d-13
      bornmeq2 = 1.d-13
      bornmez  = 1.d-13

      
      if (ie.gt.0) return
      imtx = 1
      nspinconf = 2**(4+ng)

**********************************
*** DARK MATTER OFF TREE LEVEL ***
**********************************
      if(idarkon.eq.0) then
         if (ng.eq.0) then
            do k = 0,3
               pin1b(k) = pin1(k)
               pin2b(k) = pin2(k)
               p1b(k) = p1(k)
               p2b(k) = p2(k)
            enddo
         else
            call closest(ng,qph,pin1,pin2,p1,p2,q1,q2,q3,q4,iclose)
            do k = 0,3
               p1tmp(k) = pin1(k) - q1(k)
               p2tmp(k) = pin2(k) - q2(k)
               qq(k) = p1tmp(k) + p2tmp(k)
            enddo
            
            if(fs.eq.'ee') then
               call maptozero2(p1tmp,p2tmp,p1,p2,q1,q2,q3,q4,
     >              pin1b,pin2b,p1b,p2b,iemap)
            else
               call maptozero(p1tmp,p2tmp,p1,p2,
     >              pin1b,pin2b,p1b,p2b,iemap)
            endif
         endif
**  calcola elemento di matrice al tree level con i momenti mappati
** VP STUFF
         iq2   = 0
         iarun = 0              ! added ! ON OFF
         call born_me(pin1b,pin2b,p1b,p2b,bornmenovp,bornmeq2,vuoto,iq2)
         iarun = iarunbck

         iq2 = 1
         call born_me(pin1b,pin2b,p1b,p2b,bornme,bornmeq2,bornmez,iq2)

*[[[ START VP PARAMETRIC UNCERTAINTY         
         IF (abs(scal_err_dhad).gt.1d-5) THEN
            if (iteubn.eq.0) then
               remtx = 1.d0
               invp = 0
               iswvperr(1) = 0
               iswvperr(2) = 0
               iswvperr(3) = 0
               do i1 = -1,1
               do i2 = -1,1
               do i3 = -1,1
                  if (i1.ne.0.or.i2.ne.0.or.i3.ne.0) then
                     invp = invp + 1
                     iswvperr(1) = i1
                     iswvperr(2) = i2
                     iswvperr(3) = i3
                     call born_me(pin1b,pin2b,p1b,p2b,xr,bq2,vuoto,iq2)
                     remtx(invp) = xr/bornme
                  endif
               enddo   
               enddo
               enddo
            else  ! for hlmnt VP parametrization
               remtxt    = 1.d0
               invpt     = 0
               iswvperrt = 0
               do invpt=1,nvpst
                  iswvperrt(1) = ist(invpt,1)
                  iswvperrt(2) = ist(invpt,2)
                  iswvperrt(3) = ist(invpt,3)
                  iswvperrt(4) = ist(invpt,4)
                  iswvperrt(5) = ist(invpt,5)
                  iswvperrt(6) = ist(invpt,6)
                  call born_me(pin1b,pin2b,p1b,p2b,xr,bq2,vuoto,iq2)
                  remtxt(invpt) = xr/bornme
               enddo   
            endif
         ENDIF
*]]]  END VP PARAMETRIC UNCERTAINTY         

         
      endif
*******************************************************
*** DARK MATTER ON - TREE LEVEL (cmq 1 g in uscita) ***
*******************************************************
      if(idarkon.eq.1) then

         do k=0,3
            q(k) = qph(1,k)
         enddo

*
** alpha running:
**  gli elementi di matrice li calcolo inizialmente tutti senza alpharun
**  1 --> fs=mm --> moltiplico l'elemento di matrice per
**    (alpha(s)/alpha(0))^2 = vpol(s)^2 xchè ci sono 3 vertici, ma
**    quello del fotone in uscita lo voglio con alpha(0)
**  
*
         do k=0,3
            ptmp(k)=pbeam1(k)+pbeam2(k)
            ptmp2(k)=p1(k)+p2(k)
         enddo
         s = (dot(ptmp,ptmp)+dot(ptmp2,ptmp2))/2.d0


         if(fs.eq.'mm') then
            call alpha_me('R',1,pbeam1,pbeam2,p1,p2,
     >                     qph,ialpha,bornme)
            bornme=bornme*abs(vpolc(s))**2
*
**  2 --> fs=ee --> all'elemento di matrice (M) sottraggo quello di canale
**    s (Ms). Poi faccio (M-Ms)*(alpha(t)/alpha(0))^2 =(M-Ms)*vpol(t)^2.
**    Poi Ms*alpha(s)^3/alpha(t)^3*alpha(t)^2/alpha(0)^2=
**    = Ms*vpol(s)^3/vpol(t). Elemento di matrice completo radiativo =
**    = M*vpol(t)^2 + Ms*(vpol(s)^2-vpol(t)^2)
*
         elseif(fs.eq.'ee') then

            call alpha_me('R',1,pbeam1,pbeam2,p1,p2,
     >                     qph,ialpha,bornme)

            if(iarun.eq.1) then
              do k=0,3
                ptmp(k)=pbeam1(k)-p1(k)
                ptmp2(k)=pbeam2(k)-p2(k)
              enddo
              t=(dot(ptmp,ptmp)+dot(ptmp2,ptmp2)/2.d0)
              bornme = bornme * abs(vpolc(t))**2
            endif
         endif
         bornmeq2=bornme

         if (iemap.eq.1) then
             ie = 1
             summt2 = 0.d0
             iarun = iarunbck
             return
        endif

        summt2 = bornme
        iref = 1
        imtx = 1
        iarun = iarunbck

        return

      endif
*
** calcola cose che servono x il matching
*
      flux2b = 8.d0*pin1b(0)*pin2b(0)
      mass = pin1b(0)+pin2b(0)
      amel = ame
      if (fs.eq.'gg') amel = 0.d0
      if (fs.eq.'mm') amel = ammu
      sqla = sqrt(lambda(mass**2,amel**2,amel**2))
      phsp2b = sqla/8.d0/mass**2 /(2.d0*pi) / 4.d0 ! spins...

****
*********************
**    MATCHING     **
*********************
! doing if only the k-th photon was present
      if (model.eq.'matched') then
         if (ng.eq.0) then
            iref = 1
            summt2 = bornme
            imtx = 1
            iarun = iarunbck
            return
         endif

c     * bornme already accounts for VP.
c     * I switch it off here for the matching to avoid jumps.
c     * It's anyway a higher order effect
         iarun = 0         ! added ! ON OFF
ccccc
         
         if (ng.eq.1) then
            do i=0,3
               q(i) = qph(1,i)
            enddo            
            call bk_me(pin1,pin2,p1,p2,q,summt2)

** VP STUFF
            summt2 = summt2/bornmenovp * bornme ! added
**
            imtx = 1
            iarun = iarunbck
            return
         endif

         eikng = 'multip'
         call eikonal_factor(eikng,ng,pin1,pin2,p1,p2,q,qph,eikfull)
         ei1 = pin1(0)
         ei2 = pin2(0)
         ef1 = ei1
         ef2 = ei2
         ef3 = p1(0)
         ef4 = p2(0)
         prodomega = 1.d0
         do k = 1,ng
            prodomega = prodomega*qph(k,0)
            if (iclose(k).eq.3) ef3 = ef3 + qph(k,0)
            if (iclose(k).eq.4) ef4 = ef4 + qph(k,0)
         enddo
         ei3 = ef3
         ei4 = ef4

         prodapx = 1.d0
         xjac    = 1.d0
         delta   = 0.d0
         prod    = 1.d0
         delta_eik = 0.d0
         prod_eik  = 1.d0
         do k = 1,ng
            do i=0,3
               q(i) = qph(k,i)
            enddo            
**** PS QUANTITIES

            if (iclose(k).eq.1) then
               xjac = xjac/ef1/2.d0
               xap  = qph(k,0)/ef1
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef1  = ef1 - qph(k,0)
            endif
            if (iclose(k).eq.2) then
               xjac = xjac/ef2/2.d0
               xap  = qph(k,0)/ef2
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef2  = ef2 - qph(k,0)
            endif
            if (iclose(k).eq.3) then
               xjac = xjac/ef3/2.d0
               xap  = qph(k,0)/ef3
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef3  = ef3 - qph(k,0)
            endif
            if (iclose(k).eq.4) then
               xjac = xjac/ef4/2.d0
               xap  = qph(k,0)/ef4
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef4    = ef4 - qph(k,0)
            endif

            prodapx = prodapx * apx
! killing the event!!
            if (prodapx.lt.0.d0) then
               ie = 1
               summt2 = 0.d0
               iarun = iarunbck
               return
            endif
***            
            is = 0
            if (iclose(k).eq.1.or.iclose(k).eq.2) is = 1

** doing as if only the k-th photon is present
            i1 = 0
            i2 = 0
            i3 = 0
            i4 = 0
            if (iclose(k).eq.1) i1 = 1
            if (iclose(k).eq.2) i2 = 1
            if (iclose(k).eq.3) i3 = 1
            if (iclose(k).eq.4) i4 = 1
            do ki = 0,3
               p1tmp(ki) = pin1(ki) - i1 * q(ki)
               p2tmp(ki) = pin2(ki) - i2 * q(ki)
               qq(ki) = p1tmp(ki) + p2tmp(ki)
            enddo

            call maptozero(p1tmp,p2tmp,p1,p2,pin1bl,pin2bl,
     .           p1bl,p2bl,iemap)
***   
! killing the event!!
            if (iemap.eq.1) then
               ie = 1
               summt2 = 0.d0
               iarun = iarunbck
               return
            endif
***
            iq2 = 0
            call born_me(pin1bl,pin2bl,p1bl,p2bl,bornmel,bornmelq2,
     .           bornmelz,iq2)
***      
            call maptoone(is,qq,p1,p2,q,pin1r,pin2r,p1r,p2r,qr)
            call eikonal_factor('single',ng,pin1r,pin2r,p1r,p2r,qr,
     >           qph,eiki)
            call bk_me(pin1r,pin2r,p1r,p2r,qr,oal)

            ei1r = pin1r(0)
            ei2r = pin2r(0)
            ef3r = p1r(0)
            ef4r = p2r(0)

            if (iclose(k).eq.1) then 
               ef1r = ei1r - qr(0)
               xjaci = 1.d0/ei1r/2.d0
               xapi = qr(0)/ei1r
               apxi = (1.d0 + (1.d0 - xapi)**2)/xapi
               apxi = apxi/(1.d0 - xapi) !!
            elseif (iclose(k).eq.2) then 
               ef2r = ei2r - qr(0)
               xjaci = 1.d0/ei2r/2.d0
               xapi = qr(0)/ei2r
               apxi = (1.d0 + (1.d0 - xapi)**2)/xapi
               apxi = apxi/(1.d0 - xapi) !!
            elseif (iclose(k).eq.3) then 
               ei3r = ef3r + qr(0)
               xjaci = 1.d0/ei3r/2.d0
               xapi = qr(0)/ei3r
               apxi = (1.d0 + (1.d0 - xapi)**2)/xapi
               apxi = apxi/(1.d0 - xapi) !!
            elseif (iclose(k).eq.4) then 
               ei4r = ef4r + qr(0)
               xjaci = 1.d0/ei4r/2.d0
               xapi = qr(0)/ei4r
               apxi = (1.d0 + (1.d0 - xapi)**2)/xapi
               apxi = apxi/(1.d0 - xapi) !!
            endif

            approxi =  apxi*eiki*qr(0)*xjaci
!! TO AVOID ROUND-OFFs
            if (approxi.gt.0.d0) then
               rescaletomatcheikfull=eiki/singleeikfact(k)
               deltai = (oal - approxi*bornmel)/approxi
*##
*##            deltai = deltai * rescaletomatcheikfull
*##
            else
               deltai = 0.d0
            endif
            
            delta  = delta + deltai

            prod = prod * (1.d0 + deltai/bornmel)
            
            deltah = deltah + deltai/bornmel
***   matched with eikonal approximation
c            deltai_eik = 0.d0
c            if (eiki.gt.0.d0) then
c               deltai_eik = (oal/eiki - bornmel)
c            endif
c            delta_eik  = delta_eik + deltai_eik
c            prod_eik = prod_eik * (1 + deltai_eik/bornmel)
***
         enddo

         
         summt2 = bornme*prodapx*eikfull*prodomega*xjac
c      summt2_eik = eikfull*bornme*prod_eik

!! MATCHED PARTON SHOWER !!!!!!!!!
! EUREKA !!! 
! Sun Aug 14 01:42:08 CEST 2005
         summt2 = summt2 * prod
         imtx = 1
         prodotto = prod
         
c         deltah   = delta

c         print*,'stop matrix_model.f line ~351'
c         stop

         iarun = iarunbck
         return
      endif
*******************
**      PS       **
*******************
      if (model.eq.'ps') then
         if (ng.eq.0) then
            iref = 1
            summt2 = bornme
            imtx = 1
            iarun = iarunbck
            return
         endif
*
         ei1 = pin1(0)
         ei2 = pin2(0)
         ef1 = ei1
         ef2 = ei2
         ef3 = p1(0)
         ef4 = p2(0)

         prodomega = 1.d0
         do k = 1,ng
            prodomega = prodomega*qph(k,0)
            if (iclose(k).eq.3) ef3 = ef3 + qph(k,0)
            if (iclose(k).eq.4) ef4 = ef4 + qph(k,0)
         enddo

         ei3 = ef3
         ei4 = ef4

         prodapx = 1.d0
         xjac    = 1.d0
         do k = 1,ng
** PS QUANTITIES
            if (iclose(k).eq.1) then
               xjac = xjac/ef1/2.d0
               xap  = qph(k,0)/ef1
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef1  = ef1 - qph(k,0)
            endif
            if (iclose(k).eq.2) then
               xjac = xjac/ef2/2.d0
               xap  = qph(k,0)/ef2
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef2  = ef2 - qph(k,0)
            endif
            if (iclose(k).eq.3) then
               xjac = xjac/ef3/2.d0
               xap  = qph(k,0)/ef3
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef3  = ef3 - qph(k,0)
            endif
            if (iclose(k).eq.4) then
               xjac = xjac/ef4/2.d0
               xap  = qph(k,0)/ef4
               apx  = (1.d0 + (1.d0 - xap)**2)/xap
               apx  = apx/(1.d0 - xap) !!
               ef4  = ef4 - qph(k,0)
            endif
            prodapx = prodapx * apx

            if (prodapx.lt.0.d0) then
               ie = 1
               summt2 = 0.d0
               iarun = iarunbck
               return
            endif
****  
         enddo
       
         if (ng.gt.1) then
            eikng = 'multip'
            call eikonal_factor(eikng,ng,pin1,pin2,p1,p2,q,qph,eikfull)
         else
            do i=0,3
               q(i) = qph(1,i)
            enddo
            eikng = 'single'
            call eikonal_factor(eikng,ng,pin1,pin2,p1,p2,q,qph,eikfull)
         endif

         summt2 = bornme * prodapx * eikfull * prodomega * xjac
         imtx = 1
         iarun = iarunbck
         return
      endif
******
      if (model.eq.'eikonal') then
         eikng = 'multip'
         call eikonal_factor(eikng,ng,pin1,pin2,p1,p2,q,qph,eikonal)
         summt2 = eikonal * bornme
         imtx = 1
         iarun = iarunbck
         return
      endif
******
!! ALPHA model not released!
c$$$      if (model.eq.'alpha') then
c$$$! per ng = 0,1 e' inutile chiamare alpha... 
c$$$         if (ng.eq.0) then
c$$$            iref = 1
c$$$            summt2 = bornme
c$$$            imtx = 1
c$$$            return
c$$$         endif
c$$$         if (ng.eq.1) then
c$$$            do i=0,3
c$$$               q(i) = qph(1,i)
c$$$            enddo
c$$$c            call alpha_me('S',ng,p1,p2,qph,ialpha,emtxalpha)
c$$$
c$$$
c$$$            call bk_me(pin1,pin2,p1,p2,q,summt2)
c$$$ccc            call approxNg(ng,pin1,pin2,p1,p2,qph,summt2)
c$$$ccc            ialpha = 1
c$$$
c$$$            summt2 = summt2/factorial(ng)
c$$$            imtx = 1
c$$$            return
c$$$         endif
c$$$******
c$$$* for ALPHA, not released
c$$$c         call alpha_me('S',ng,p1,p2,qph,ialpha,emtxalpha) ! summed over spins
c$$$c         call alpha_me('R',ng,p1,p2,qph,ialpha,emtxalpha) ! random over spins
c$$$         emt1 = emtxalpha
c$$$         do k = 0,3
c$$$            ptmp(k)  = qph(1,k)
c$$$            qph(1,k) = qph(2,k)
c$$$            qph(2,k) = ptmp(k)
c$$$         enddo
c$$$         call alpha_me('S',ng,p1,p2,qph,ialpha,emtxalpha) ! random over spins
c$$$         emt2 = emtxalpha
c$$$
c$$$         print*,emt2/emt1
c$$$
c$$$
c$$$c$$$         call approxNg(ng,pin1,pin2,p1,p2,qph,emtxalpha)
c$$$c$$$         emtxalpha = emtxalpha/factorial(ng)
c$$$c$$$         ialpha = 1
c$$$
c$$$         imtx   = ialpha
c$$$         summt2 = emtxalpha
c$$$         return
c$$$      endif
******
      iarun = iarunbck
      return
      end
****************************************************************************
*
* e+e- --> e+e-g
****************************************************************************
      subroutine formme(p1,p2,p3,p4,q,elmat2)
! written by CMCC, last modified 19/10/2006
      implicit none
      double precision p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      double precision elmat2,ame,convfac,alpha,pi,e2,qph(40,0:3)
      double precision dot,elmat2cmn,e2approx,eikf
      double precision me,me2,me4,me6,ammu
      double precision p1p2,p1p3,p1p4,p2p3,p2p4,p3p4
      double precision p1p22,p1p32,p1p42,p2p32,p2p42,p3p42
      double precision p1q,p2q,p3q,p4q
      double precision p1qm1,p2qm1,p3qm1,p4qm1
      double precision p1qm2,p2qm2,p3qm2,p4qm2
      double precision s12,s34,t24,t13,u14,u23
      double precision s12m1,s34m1,t24m1,t13m1
      double precision s12m2,s34m2,t24m2,t13m2
      double precision vp24,vp13
      double precision vp122,vp342,vp242,vp132
      double precision ptmp(0:3)
      double precision vpol
      external vpol
      double complex vpolc,vp12,vp34
      external vpolc
      integer k
      character*2 fs
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      common/testsdebug/elmat2cmn,e2approx,s12,s34,t24,t13,u14,u23
      external dot

      e2 = alpha*4.d0*pi

      me  = ame
      me2 = me*me
      me4 = me2*me2
      me6 = me4*me2

      do k = 0,3
         ptmp(k) = p1(k)+p2(k)
      enddo
      s12 = dot(ptmp,ptmp)
      s12m1 = 1.d0/s12
      s12m2 = s12m1/s12
      vp12  = vpolc(s12)
      vp122 = abs(vp12)**2

      do k = 0,3
         ptmp(k) = p4(k)+p3(k)
      enddo
      s34 = dot(ptmp,ptmp)
      s34m1 = 1.d0/s34
      s34m2 = s34m1/s34
      vp34 = vpolc(s34)
      vp342 = abs(vp34)**2


      do k = 0,3
         ptmp(k) = p2(k)-p4(k)
      enddo
      t24 = dot(ptmp,ptmp)
      t24m1 = 1.d0/t24
      t24m2 = t24m1/t24
      vp24 = vpolc(t24)
      vp242 = vp24*vp24

      do k = 0,3
         ptmp(k) = p1(k)-p3(k)
      enddo
      t13= dot(ptmp,ptmp)
      t13m1 = 1.d0/t13
      t13m2 = t13m1/t13
      vp13 = vpolc(t13)
      vp132 = vp13*vp13

      p1p2  = dot(p1,p2)
      p1p22 = p1p2*p1p2

      p1p3  = dot(p1,p3)
      p1p32 = p1p3*p1p3

      p1p4  = dot(p1,p4)
      p1p42 = p1p4*p1p4

      p2p3  = dot(p2,p3)
      p2p32 = p2p3*p2p3

      p2p4  = dot(p2,p4)
      p2p42 = p2p4*p2p4

      p3p4  = dot(p3,p4)
      p3p42 = p3p4*p3p4

      p1q   = dot(p1,q)
      p2q   = dot(p2,q)
      p3q   = dot(p3,q)
      p4q   = dot(p4,q)

      p1qm1 =  1.d0/p1q
      p1qm2 = p1qm1/p1q
      p2qm1 =  1.d0/p2q
      p2qm2 = p2qm1/p2q
      p3qm1 =  1.d0/p3q
      p3qm2 = p3qm1/p3q
      p4qm1 =  1.d0/p4q
      p4qm2 = p4qm1/p4q

      include 'form/formme.include'
      elmat2 = elmat2 * e2**3
      return
      end
****************************************************************************
*
* e+e- --> mu+mu-g
****************************************************************************
      subroutine formmemm(p1,p2,p3,p4,q,elmat2)
! written by CMCC, last modified 18/9/2007
      implicit none
      double precision p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      double precision elmat2,ame,convfac,alpha,pi,e2,qph(40,0:3)
      double precision dot,elmat2cmn,e2approx,eikf,elmat2bck
      double precision me,me2,me4,me6,ammu
      double precision mm,mm2,mm4,mm6
      double precision p1p2,p1p3,p1p4,p2p3,p2p4,p3p4
      double precision p1p22,p1p32,p1p42,p2p32,p2p42,p3p42
      double precision p1q,p2q,p3q,p4q
      double precision p1qm1,p2qm1,p3qm1,p4qm1
      double precision p1qm2,p2qm2,p3qm2,p4qm2
      double precision s12,s34,t24,t13,u14,u23
      double precision s12m1,s34m1,t24m1,t13m1
      double precision s12m2,s34m2,t24m2,t13m2
      double precision vp24,vp13
      double precision vp122,vp342,vp242,vp132
      double precision ptmp(0:3)
      double precision vpol
      external vpol
      double complex vpolc,vp12,vp34
      external vpolc
      integer k
      character*2 fs
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      common/testsdebug/elmat2cmn,e2approx,s12,s34,t24,t13,u14,u23
      external dot

      e2 = alpha*4.d0*pi

      me  = ame
      me2 = me*me
      me4 = me2*me2
      me6 = me4*me2

      mm  = ammu
      mm2 = mm*mm
      mm4 = mm2*mm2
      mm6 = mm4*mm2
      

      do k = 0,3
         ptmp(k) = p1(k)+p2(k)
      enddo
      s12 = dot(ptmp,ptmp)
      s12m1 = 1.d0/s12
      s12m2 = s12m1/s12
      vp12 = vpolc(s12)
      vp122 = abs(vp12)**2

      do k = 0,3
         ptmp(k) = p4(k)+p3(k)
      enddo
      s34 = dot(ptmp,ptmp)
      s34m1 = 1.d0/s34 
      s34m2 = s34m1/s34!!  * 0.d0
      vp34 = vpolc(s34)
      vp342 = abs(vp34)**2
*
**
*
      p1p2  = dot(p1,p2)
      p1p22 = p1p2*p1p2

      p1p3  = dot(p1,p3)
      p1p32 = p1p3*p1p3

      p1p4  = dot(p1,p4)
      p1p42 = p1p4*p1p4

      p2p3  = dot(p2,p3)
      p2p32 = p2p3*p2p3

      p2p4  = dot(p2,p4)
      p2p42 = p2p4*p2p4

      p3p4  = dot(p3,p4)
      p3p42 = p3p4*p3p4

      p1q   = dot(p1,q)
      p2q   = dot(p2,q)
      p3q   = dot(p3,q)
      p4q   = dot(p4,q)
      p1qm1 =  1.d0/p1q
      p1qm2 = p1qm1/p1q
      p2qm1 =  1.d0/p2q
      p2qm2 = p2qm1/p2q
      p3qm1 =  1.d0/p3q
      p3qm2 = p3qm1/p3q
      p4qm1 =  1.d0/p4q
      p4qm2 = p4qm1/p4q
      include 'form/formmemm.include'

      elmat2 = elmat2 * e2**3
      return
      end
c
      subroutine born_me(p1,p2,p3,p4,smt,smtq2,smtz,iq2)
! written by CMCC, last modified 21/10/2005
      implicit double precision (a-h,l,m,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),ptmp(0:3)
      complex*16 chis,chit,im
      parameter (im = (0.d0,1.d0))
      double complex vpolc,ec2s
      external vpolc
      character*2 fs
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      common/zparameters/zm,gz,ve,ae,rv,ra,wm,s2th,gfermi,sqrt2,um

      integer ifirst
      common/ifirstborn/ifirst
      data ifirst /0/

      smt   = 0.d0
      smtq2 = 0.d0
      smtz  = 0.d0

      if (fs.eq.'gg') then
         me2 = ame*ame
         me4 = me2*me2
         p1p2 = dot(p1,p2)
         p1p3 = dot(p1,p3)
         p1p4 = dot(p1,p4)
         p2p3 = dot(p2,p3)
         p2p4 = dot(p2,p4)
         p3p4 = dot(p3,p4)
         p2p3m1 = 1.d0/p2p3
         p2p4m1 = 1.d0/p2p4
         include 'form/borngg.include'
         e2 = 4.d0*pi*alpha
         smt = elmat2*e2*e2
         smtq2 = smt         
         return
      endif

      do k=0,3
         ptmp(k) = p1(k)+p2(k)
      enddo
      s = dot(ptmp,ptmp)
      do k=0,3
         ptmp(k) = p1(k)-p3(k)
      enddo
      t = dot(ptmp,ptmp)
      do k=0,3
         ptmp(k) = p1(k)-p4(k)
      enddo
      u = dot(ptmp,ptmp)
      
      em2 = ame**2
      em4 = em2*em2
      ec2 = 4.d0*pi*alpha       ! electron charge^2
      ec4 = ec2*ec2

      s2 = s*s
      t2 = t*t
      u2 = u*u


*** NEW : always running !!!
      if (fs.eq.'ee') then

         if (ifirst.eq.0) then
c            print*,' ONLY INTERFERENCE!!'
            ifirst = 1
         endif

         ec2t  = ec2 * vpolc(t)
         ec2s  = ec2 * vpolc(s) ! * 0.d0

         sm2   = abs(ec2s)**2/s2*((t2+u2)/4.d0+2.d0*em2*s
     .        -2.d0*em4)
         tm2   = ec2t**2/t2*((s2+u2)/4.d0+2.d0*em2*t
     .        -2.d0*em4)
         st2   = ec2s*ec2t/s/t*(u2/2.d0-4.d0*em2*u+6.d0*em4 )

         smtq2 = 32.d0*(tm2 + sm2 + st2)
c         smtq2 = 32.d0*(st2)
         smt   = smtq2

      elseif (fs.eq.'mm') then
         !! WARNING::: NO Z EXCHANGE IN MU MU !!!
         mm2 = ammu*ammu
         ec2s  = ec2 * vpolc(s)

         me2   = ame*ame
         e4    = abs(ec2s)**2
         elmat2=32.d0*e4/s2*((t2+u2)/4.d0+s*(me2+mm2)-(me2+mm2)**2/2.d0)
         smtq2 = elmat2
         smt   = smtq2
         return
      endif
********************
***   with Z exchange  !
      if (iq2.eq.1) then

****
** with Z exchange, which gives a 0.1% effect at 10 GeV for wide acceptance.
** From Caffo & Remiddi in CERN Yellow Report 'Z phyisics at LEP1' Vol 1
         chis=zm**2/16.d0/um**2*(ve**2+ae**2)*s/(s-zm**2+im*gz*zm)
         chit=zm**2/16.d0/um**2*(ve**2+ae**2)*s/2.d0/(zm**2-t-im*gz*zm)

         z = p3(3)/sqrt(tridot(p3,p3))

         z2   = z**2
         upz2 = (1.d0+z)**2

         chi2   = (abs(chis))**2
         chit2  = (abs(chit))**2
         rechi  = (chis + dconjg(chis))/2.d0
         rechit = (chit + dconjg(chit))/2.d0
         rechichit = (chis*dconjg(chit) + dconjg(chis)*chit)/2.d0

         s4  = -2.d0*rechit*upz2
         s5  =  4.d0*rechit/(1.d0-z)*(upz2 + 4.d0*(rv-ra))
         s6  =  2.d0*chit2*
     .        (upz2*(1.d0+4.d0*rv*ra)+4.d0*(1.d0-4.d0*rv*ra))
         s7  =  2.d0*rechi*((1.d0+z2)*rv+2.d0*z*ra)
         s8  = -2.d0*rechi*upz2/(1.d0-z)
         s9  = -2.d0*rechichit*upz2*(1.d0+4.d0*rv*ra)
         s10 =  chi2*((1.d0+z2)+8.d0*z*rv*ra)
         
         sz = (s4+s5+s6+s7+s8+s9+s10)*alpha**2/4.d0/s
         smtz = sz*32.d0*pi**2 *2.d0*s * 4.d0 !summed, not averaged... 
      endif
******
** TEST 2 LOOP!
c      esoft = 1.d-4*p1(0)
c      call twoloopvirtualsoft(p1,p2,p3,p4,esoft,twoloopoverborn,pair)
c      smt = pair*smtq2 !!!*pi*pi*p1(0)**2*1024.d0
**** END TEST 2 LOOP
** TEST SOFT V+R PAIR CORRECTIONS
** virtual -- FORMULA 6b jadach
c$$$      ammu = ame
c$$$      L   = log(-t/ammu/ammu)
c$$$      L2  = L*L
c$$$      L3  = L2*L
c$$$      cf  = 383.d0/108.d0-11.d0/6.d0*pi*pi/6.d0
c$$$      z3  = 1.2020569032d0
c$$$      cfp = -z3/3.d0+3355.d0/1296.d0-19.d0/18.d0*pi**2/6.d0 
c$$$      
c$$$      fmu = -1.d0/36.d0*L3      ! * 0.d0
c$$$     .     +19.d0/72.d0*L2      !* 0.d0
c$$$     .     +(-1.d0/36.d0*pi*pi-265.d0/216.d0)*L+
c$$$     .     cf + 19.d0/75.d0*pi*pi
c$$$
c$$$      v = 4.d0*(alpha/pi)**2*fmu * 32.d0*tm2 ! full virtual for t channel only
c$$$      smt   = v
c$$$      smtq2 = smt
c$$$** real -- Arbuzov et al., formula 43 only t-channel
c$$$      c = p3(3)/sqrt(tridot(p3,p3))
c$$$      L = log(-t/ame/ame) ! - 1.d0 ! ?????
c$$$      L2 = L**2
c$$$      L3 = L2*L
c$$$      delta = 1d0
c$$$c      delta = 1.d0
c$$$      ld = log(delta)
c$$$cccccccccccccccccccccccc      ld = -ld
c$$$      ddlog = ddilog((1.d0+c)/2.d0)
c$$$      at = 56.d0/9.d0 -2.d0/3.d0*pi*pi+2.d0*ddlog
c$$$      r  = 1.d0/3.d0*L3
c$$$     .     +L2*(2.d0*ld-5.d0/3.d0)
c$$$     .     +L*(4.d0*ld**2-20.d0/3.d0*ld+at)
c$$$      r  = r * 1.d0/3.d0*alpha**2/pi/pi
c$$$      r  = r *32.d0*tm2
c$$$      r1 = r
c$$$* tesi Oreste
c$$$      L  = L ! + 1.d0
c$$$      z2 = pi**2/6.d0
c$$$      r  =  1.d0/18.d0*(L-2.d0*ld)**3
c$$$     .     -5.d0/18.d0*(L-2.d0*ld)**2
c$$$     .     +1.d0/9.d0*3.d0/2.d0*(56.d0/9.d0-4.d0*z2 + 0.*2*ddlog) ! Oreste
c$$$     .     *(L-2.d0*ld)
c$$$      r = r*2.d0
c$$$      r = r*alpha**2/pi/pi *32.d0*tm2
c$$$
c$$$ccc ATTENZIONE: QUELLO DI ORESTE AUMENTA AL DIMINUIRE DI DELTA!!!
c$$$ccc A ME SEMBRA POCO FISICO (INTEGRO SU UNO SPAZIO FASI PIU' AMPIO!!)
c$$$
c$$$c      print*,'TEST COPPIE!!!!'
c$$$c      print*,r-r1,L
c$$$c      r = r1
c$$$      smt = r
c$$$      smtq2 = smt
c$$$      return
************ END TEST COPPIE
      return
      end
**************************************************************************
*
*
**************************************************************************
      subroutine eikonal_factor(eikng,ng,p1,p2,p3,p4,q,qph,eikonal)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      character*6 eikng
      parameter (imaxph = 40)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      dimension qph(imaxph,0:3),q(0:3),peik(0:3)
      character*2 fs
      common/finalstate/fs
      common/singlefactors/singleeikfact(imaxph)
      common/parameters/ame,ammu,convfac,alpha,pi

      eikonal = 1.d0
      test = 1.d0
      if (ng.eq.0) return

      nglocal = ng
      if (eikng.eq.'single') nglocal = 1

      ec2 = 4.d0*pi*alpha ! electron charge^2
      do k = 1,nglocal
         if (eikng.eq.'multip') then
            q(0) = qph(k,0)
            q(1) = qph(k,1)
            q(2) = qph(k,2)
            q(3) = qph(k,3)
         endif
         oop1dk = 1.d0/dot(p1,q)
         oop2dk = 1.d0/dot(p2,q)
         if (fs.eq.'ee'.or.fs.eq.'mm') then
            oop3dk = 1.d0/dot(p3,q)
            oop4dk = 1.d0/dot(p4,q)
         elseif (fs.eq.'gg') then
            oop3dk = 0.d0
            oop4dk = 0.d0
         endif
c         do i = 0,3
c            peik(i)=p1(i)*oop1dk-p2(i)*oop2dk-p3(i)*oop3dk+p4(i)*oop4dk
c         enddo
c         singlephoton = -ec2*dot(peik,peik)
         singlephoton = -(
     .          dot(p1,p1)*oop1dk**2
     .        + dot(p2,p2)*oop2dk**2
     .        + dot(p3,p3)*oop3dk**2
     .        + dot(p4,p4)*oop4dk**2
     .        - 2.d0*dot(p1,p2)*oop1dk*oop2dk
     .        - 2.d0*dot(p3,p4)*oop3dk*oop4dk
     .        - 2.d0*dot(p1,p3)*oop1dk*oop3dk
     .        + 2.d0*dot(p1,p4)*oop1dk*oop4dk
     .        + 2.d0*dot(p2,p3)*oop2dk*oop3dk
     .        - 2.d0*dot(p2,p4)*oop2dk*oop4dk)*ec2               
         
         eikonal = eikonal * singlephoton
         if (eikng.eq.'multip') singleeikfact(k) = singlephoton
      enddo

      eikonal = eikonal/factorial(nglocal)      
      return
      end
**************************************************************************
*
**************************************************************************
      subroutine eikonal_factor_BCK(eikng,ng,p1,p2,p3,p4,q,qph,eikonal)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      character*6 eikng
      parameter (imaxph = 40)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      dimension qph(imaxph,0:3),q(0:3),peik(0:3)
      character*2 fs
      common/finalstate/fs
      common/singlefactors/singleeikfact(imaxph)
      common/parameters/ame,ammu,convfac,alpha,pi

      eikonal = 1.d0
      if (ng.eq.0) return

      nglocal = ng
      if (eikng.eq.'single') nglocal = 1

      ec2 = 4.d0*pi*alpha ! electron charge^2
      do k = 1,nglocal
         if (eikng.eq.'multip') then
            q(0) = qph(k,0)
            q(1) = qph(k,1)
            q(2) = qph(k,2)
            q(3) = qph(k,3)
         endif
         oop1dk = 1.d0/dot(p1,q)
         oop2dk = 1.d0/dot(p2,q)
         if (fs.eq.'ee'.or.fs.eq.'mm') then
            oop3dk = 1.d0/dot(p3,q)
            oop4dk = 1.d0/dot(p4,q)
         elseif (fs.eq.'gg') then
            oop3dk = 0.d0
            oop4dk = 0.d0
         endif
         do i = 0,3
            peik(i)=p1(i)*oop1dk-p2(i)*oop2dk-p3(i)*oop3dk+p4(i)*oop4dk
         enddo
         singlephoton = -ec2*dot(peik,peik)
         eikonal = eikonal * singlephoton
         if (eikng.eq.'multip') singleeikfact(k) = singlephoton
      enddo
      eikonal = eikonal/factorial(nglocal)      
      return
      end
**************************************************************************
*     
**************************************************************************
      subroutine eikonal_noint(eikng,ng,p1,p2,p3,p4,q,qph,eikonal)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      character*6 eikng
      parameter (imaxph = 40)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),pm(4,0:3)
      dimension qph(imaxph,0:3),q(0:3),peik(0:3),ptmp1(0:3),ptmp2(0:3)
      common/parameters/ame,ammu,convfac,alpha,pi

      eikonal = 1.d0
      if (ng.eq.0) return

      do k = 0,3
         pm(1,k) = p1(k)
         pm(2,k) = p2(k)
         pm(3,k) = p3(k)
         pm(4,k) = p4(k)
      enddo

      nglocal = ng
      if (eikng.eq.'single') nglocal = 1

      ec2 = 4.d0*pi*alpha ! electron charge^2
      do k = 1,nglocal
         singlephoton = 0.d0
         if (eikng.eq.'multip') then
            do i = 0,3
               q(i) = qph(k,i)
            enddo
         endif
         
         do i = 1,3
            do j = i+1,4
               do jj = 0,3
                  ptmp1(jj) = pm(i,jj)
                  ptmp2(jj) = pm(j,jj)
               enddo

               if (i.eq.1.and.j.eq.2) etaij = -1.d0
               if (i.eq.1.and.j.eq.3) etaij = -1.d0
               if (i.eq.1.and.j.eq.4) etaij =  1.d0
               if (i.eq.2.and.j.eq.3) etaij =  1.d0
               if (i.eq.2.and.j.eq.4) etaij = -1.d0
               if (i.eq.3.and.j.eq.4) etaij = -1.d0

               if (i.eq.1.and.j.eq.2) etaij = -1.d0
               if (i.eq.1.and.j.eq.3) etaij =  0.d0
               if (i.eq.1.and.j.eq.4) etaij =  0.d0
               if (i.eq.2.and.j.eq.3) etaij =  0.d0
               if (i.eq.2.and.j.eq.4) etaij =  0.d0
               if (i.eq.3.and.j.eq.4) etaij = -1.d0


               pipj = dot(ptmp1,ptmp2)
               singlephoton = singlephoton -
     .              etaij*2.d0*pipj/dot(ptmp1,q)/dot(ptmp2,q)
            enddo
         enddo
         singlephoton = singlephoton - ame**2*
     .        (1.d0/dot(p1,q)**2+1.d0/dot(p2,q)**2+
     .         1.d0/dot(p3,q)**2+1.d0/dot(p4,q)**2)
         eikonal = eikonal * singlephoton * ec2
      enddo
      eikonal = eikonal/factorial(nglocal)
      return
      end
****
      subroutine bk_me(p1,p2,p3,p4,q,bkme)
! written by CMCC, last modified 13/10/2005
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3),ptmp(0:3)
      dimension qph(40,0:3)
      character*2 fs
      common/finalstate/fs
      common/forborncrosssection/phsp2b,flux2b,bornme,bornmeq2,bornmez,
     > bornmenovp
! using always FORM !!!!!!!!!
      if (fs.eq.'ee') then
         call formme(p1,p2,p3,p4,q,bkme)
      elseif (fs.eq.'mm') then
         call formmemm(p1,p2,p3,p4,q,bkme)
      elseif (fs.eq.'gg') then
         ppdk1 = dot(p2,p3)
         pmdk1 = dot(p1,p3)
         ppdk2 = dot(p2,p4)
         pmdk2 = dot(p1,p4)
         ppdk3 = dot(p2,q)
         pmdk3 = dot(p1,q)
         ppdpm = dot(p1,p2)
         call EE3G(PPDK1,PMDK1,PPDK2,PMDK2,PPDK3,PMDK3,PPDPM,
     #        bkme)
      endif
      return
      end
*****************
      SUBROUTINE EE3G(PPDK1,PMDK1,PPDK2,PMDK2,PPDK3,PMDK3,PPDPM,
     #             FUN)
      IMPLICIT REAL*8 (A-H,O-Z)
      common/funvalues/funa,funb,func,funm
      common/parameters/ame,ammu,convfac,alpha,pi
*
      AME2 = AME**2
      AME4 = AME2*ame2
      E2=ALPHA*4.D0*PI
      FUN0=0.D0
      FUNM=0.D0
*
*---- WHITHOUT MASS TERMS
c      FUN0=FUN0+(PPDK1**2+PMDK1**2)/PPDK2/PMDK2/PPDK3/PMDK3
c      FUN0=FUN0+(PPDK2**2+PMDK2**2)/PPDK1/PMDK1/PPDK3/PMDK3
c      FUN0=FUN0+(PPDK3**2+PMDK3**2)/PPDK1/PMDK1/PPDK2/PMDK2
      
      funa = (PPDK1**2+PMDK1**2)/PPDK2/PMDK2/PPDK3/PMDK3
      funb = (PPDK2**2+PMDK2**2)/PPDK1/PMDK1/PPDK3/PMDK3
      func = (PPDK3**2+PMDK3**2)/PPDK1/PMDK1/PPDK2/PMDK2
      fun0 = funa + funb + func

***************!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c      fun0=(PPDK3**2+PMDK3**2)/PPDK1/PMDK1/PPDK2/PMDK2  
***************!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      FUN0=2.D0*FUN0*PPDPM*(E2**3)
*----- MASS TERMS
      FUNM=FUNM+(PMDK2/PMDK3+PMDK3/PMDK2)/PPDK1/PPDK1
      FUNM=FUNM+(PMDK1/PMDK3+PMDK3/PMDK1)/PPDK2/PPDK2
      FUNM=FUNM+(PMDK1/PMDK2+PMDK2/PMDK1)/PPDK3/PPDK3
      FUNM=FUNM+(PPDK2/PPDK3+PPDK3/PPDK2)/PMDK1/PMDK1
      FUNM=FUNM+(PPDK1/PPDK3+PPDK3/PPDK1)/PMDK2/PMDK2
      FUNM=FUNM+(PPDK1/PPDK2+PPDK2/PPDK1)/PMDK3/PMDK3
      FUNM=-2.D0*AME2*(E2)**3*FUNM

*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!**!
c      FUNM=(PPDK1/PPDK3+PPDK3/PPDK1)/PMDK2/PMDK2
c      FUNM=FUNM+(PMDK1/PMDK3+PMDK3/PMDK1)/PPDK2/PPDK2
c      FUNM=-2.D0*AME2*(E2)**3*FUNM
*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!*!**!
*----- FULL MATRIX ELEMENT
      FUN=FUN0 + FUNM
      fun = fun * 4.d0
      RETURN
      END


*****************
      SUBROUTINE EEG0(PPDQM,PMDQM,QMDK,PPDPM,PPDK,
     #                PMDK,QPDK,S,T,U,SP,TP,UP,FLUX,SQLUMI,
     #                elm)
! essentialy from LABSPV
      implicit double precision (A-H,O-Z)
      common/parameters/ame,ammu,convfac,alpha,pi   

      AME2 = AME**2
!      AME4 = AME2**2
*-----FOR ELECTRON RADIATION
      PPDK2  = PPDK**2
      PMDKM1 = 1.D0/PMDK
      PMDKM2 = PMDKM1/PMDK
      QMDKM1 = 1.D0/QMDK
      QMDKM2 = QMDKM1/QMDK
*-----FOR POSITRON RADIATION
      PMDK2  = PMDK**2
      QPDKM1 = 1.D0/QPDK
      QPDKM2 = QPDKM1/QPDK
      PPDKM1 = 1.D0/PPDK
      PPDKM2 = PPDKM1/PPDK
*
      S2  = S**2
      SP2 = SP**2
      T2  = T**2
      TP2 = TP**2
      U2  = U**2
      UP2 = UP**2
*
      W1 = S/PPDK/PMDK
      W2 = SP/QPDK/QMDK
      W3 = -T/PPDK/QPDK-TP/PMDK/QMDK+U/PPDK/QMDK+UP/PMDK/QPDK    
*
      SQMBER0 = (W1+W2+W3)*(S*SP*(S*S+SP*SP)
     #                     +T*TP*(T*T+TP*TP)
     #                     +U*UP*(U*U+UP*UP))/S/SP/T/TP
      SQMBERM = 
     #          PMDKM2*(SP/T  + T/SP  + 1.D0)**2 
     #        + PPDKM2*(SP/TP + TP/SP + 1.D0)**2 
     #        + QMDKM2*(S/T   + T/S   + 1.D0)**2 
     #        + QPDKM2*(S/TP  + TP/S  + 1.D0)**2 
      SQMBERM = -4.D0*AME2*SQMBERM
*
      SQMBER = (SQMBER0 + SQMBERM)*(4.D0*PI*ALPHA)**3
      sqmber = 4.d0 * sqmber ! it has to be summed over all spins! (also init.)
      elm    = sqmber
      SQLUMI = SQMBER*FLUX
      RETURN
      END
*******
      subroutine approxNg(ng,p1,p2,p3,p4,qph,me)
      implicit double precision (a-h,m,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),qph(40,0:3),qt(40,0:3)
      dimension tmp(0:3)
      common/parameters/ame,ammu,convfac,alpha,pi
      e2 = alpha*4.d0*pi
      nph = ng + 2
      me = 0.d0
      do i = 0,3
         tmp(i)  = p1(i) + p2(i)
         qt(1,i) = p3(i)
         qt(2,i) = p4(i)
         if (ng.gt.0) then
            do k = 1,ng
               qt(k+2,i) = qph(k,i)
            enddo
         endif
      enddo
      s = dot(tmp,tmp)

      sum  = 0.d0
      prod = 1.d0
      do k = 1,nph
         do i = 0,3
            tmp(i) = qt(k,i)
         enddo
         xi = dot(tmp,p1)
         yi = dot(tmp,p2)
         sum = sum + xi*yi * (xi**2 + yi**2)
         prod = prod * xi*yi
      enddo
      me = sum/prod * s**(nph-2) * e2**nph
      return
      end

      subroutine approxNg_SINGLE(ng,p1,p2,p3,p4,qph,me)
! only one term
      implicit double precision (a-h,m,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),qph(40,0:3),qt(40,0:3)
      dimension tmp(0:3)
      common/parameters/ame,ammu,convfac,alpha,pi
      e2 = alpha*4.d0*pi
      nph = ng + 2
      me = 0.d0
      do i = 0,3
         tmp(i)  = p1(i) + p2(i)
         qt(1,i) = p3(i)
         qt(2,i) = p4(i)
         if (ng.gt.0) then
            do k = 1,ng
               qt(k+2,i) = qph(k,i)
            enddo
         endif
      enddo
      s = dot(tmp,tmp)

      sum  = 0.d0
      prod = 1.d0
      do k = 2,2
         do i = 0,3
            tmp(i) = qt(k,i)
         enddo
         xi = dot(tmp,p1)
         yi = dot(tmp,p2)
         sum = sum + xi*yi * (xi**2 + yi**2)
      enddo
      do k = 1,nph
         do i = 0,3
            tmp(i) = qt(k,i)
         enddo
         xi = dot(tmp,p1)
         yi = dot(tmp,p2)
         prod = prod * xi*yi
      enddo
      me = sum/prod * s**(nph-2) * e2**nph  * 8.d0
      return
      end
