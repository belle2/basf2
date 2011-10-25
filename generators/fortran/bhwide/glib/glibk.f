!======================================================================
!======================= G L I B K  ===================================
!==================General Library of utilities========================
!===========It is similar but not identical to HBOOK and HPLOT=========
!======================================================================
!   
!                      Version:    1.20
!              Last correction:    September 1996
!
!
!  Installation remarks: 
!  (1) printing backslash character depends on F77 compilator,
!      user may need to modify definition of BS variable in HPLCAP
!
!  Usage of the program:
!  (1) In most cases names and meanings of programs and their 
!      parameters is the same as in original CERN libraries HBOOK
!  (2) Unlike to original HBOOK and HPLOT, all floating parameters 
!      of the programs are in double precision!
!  (3) GLIBK stores histograms in double precision and always with
!      errors. REAL*8 storage is essential for 10**7 events statistics!
!  (4) Output from GLIBK is a picture recorded as regular a LaTeX file 
!      with frame and curves/histograms, it is easy to change fonts
!      add captions, merge plots, etc. by normal editing. Finally,
!      picture may be inserted in any place into LaTeX source of the
!      article.
!  (5) WARNING: two-dimensional histograms are not active!!!
!
!  ********************************************************************
!  *  History of the program:                                         *
!  *  MINI-HBOOK writen by S. Jadach, Rutherford Lab. 1976            *
!  *  Rewritten December 1989 (S.J.)                                  *
!  *  Version with DOUBLE PRECISION ARGUMENTS ONLY!  and SAVE         *
!  *  Subprogram names start with G instead of H letter!               *
!  *  Entries:   Obligatory:  GLIMIT                                  *
!  *             Optional: see table below                            *
!  *  non-user subprograms in brackets                                *
!  ********************************************************************
!    SUBR/FUNC  1 PAR. 2 PAR. 3 PAR. 4 PAR. 5 PAR. 6 PAR.       
!  ====================================================================
*     (G_INIT)  ----   ----    ----   ----   ----   ----        
*      GI       INT    INT     ----   ----   ----   ----        
*      GIE      INT    INT     ----   ----   ----   ----        
*      GF1      INT    DBL     DBL    ----   ----   ----        
*      GFILL    INT    DBL     DBL    DBL    ----   ----        
*      GBOOK1   INT    CHR*80  INT    DBL    DBL    ----  
*     (GOPTOU)  INT    INT     INT    INT    INT     INT
* (L.F. GEXIST) INT    -----  ------  ----   ----   ----        
*      GIDOPT   INT    CHR*4   -----  ----   ----   ----        
*      GBFUN1   INT    CHR*80   INT   DBL    DBL  DP-FUNC       
*      GIDOPT   INT    CHR*4   -----  ----   ----   ----        
*      GBOOK2   INT    CHR*80   INT   DBL    DBL     INT   DBL   DBL
*      GISTDO     ---   ----   ----   ----   ----   ----        
*      GOUTPU   INT     ----   ----   ----   ----   ----        
*      GPRINT   INT     ----   ----   ----   ----   ----        
*      GOPERA   INT    CHR*1   INT    INT    DBL    DBL         
*      GINBO1   INT    CHR*8   INT    DBL    DBL    ----        
*      GUNPAK   INT    DBL(*) CHR*(*) INT    ---    ----        
*      GPAK     INT    DBL(*)  ----   ----   ---    ----        
*      GPAKE    INT    DBL(*)  ----   ----   ---    ----       
*      GRANG1   INT    DBL     DBL    ----   ---    ----        
*      GINBO2   INT    INT     DBL    DBL    INT    DBL   DBL      
*      GMAXIM   INT    DBL     ----   ----   ---    ----        
*      GMINIM   INT    DBL     ----   ----   ---    ----        
*      GRESET   INT   CHR*(*)  ----   ----   ---    ----        
*      GDELET   INT     ----   ----   ----   ----   ----        
*      GLIMIT   INT     ----   ----   ----   ----   ----        
*     (COPCH)   CHR*80 CHR*80  ----   ----   ----   ----        
* (F. JADRES)   INT     ----   ----   ----   ----   ----        
*      GRFILE   INT   CHR*(*) CHR*(*) ----   ----   ----        
*      GROUT    INT    INT    CHR*8   ----   ----   ----        
*      GRIN     INT    INT     INT    ----   ----   ----        
*      GREND   CHR*(*) ----    ----   ----   ----   ----        
!  *******************  HPLOT entries ******************
*      GPLINT   INT    ----    ----   ----   ----   ----        
*      GPLCAP   INT    ----    ----   ----   ----   ----        
*      GPLEND   ----   ----    ----   ----   ----   ----        
*      GPLOT    INT    CHR*1   CHR*1   INT   ----   ----        
*     (LFRAM1)  INT      INT     INT  ----   ----   ----        
*     (SAXIX)   INT      DBL     DBL   INT    DBL   ----        
*     (SAXIY)   INT      DBL     DBL   INT    DBL   ----        
*     (PLHIST)  INT      INT     DBL   DBL    INT    INT        
*     (PLHIS2)  INT      INT     DBL   DBL    INT    INT        
*     (PLCIRC)  INT      INT     INT   DBL    DBL    DBL        
*     (APROF)   DBL      INT     DBL  ----   ----   ----        
*      GPLSET   INT      DBL    ----  ----   ----   ----        
*      GPLTIT   INT    CHR*80   ----  ----   ----   ----        
!  *******************  WMONIT entries ******************
*      GMONIT   INT ???
!  *******************************************************************
!                         END OF TABLE        
!  *******************************************************************
*          Map of memory for single histogram
*          ----------------------------------
*  (1-7) Header
*  ist +1   mark      9999999999999
*  ist +2   mark      9d12 + id*10 + 9
*  ist +3   iflag1    9d12 + iflag1*10 +9
*  ist +4   iflag2    9d12 + iflag2*10 +9
*  ist +5   scamin    minimum y-scale
*  ist +6   scamax    maximum y-scale
*  ist +7   jdlast    address of the next histogram 
*                     from previous history of calls (see jadres)
*          ----------------------------------
*              Binning size informations
*          ----------------------------------
*  One dimensional histogram            Two dimensional histog.
*  -------------------------            ----------------------
*  (8-11) Binning information           (8-15) Binning information
*  ist2 +1    NCHX                          ist2 +5   NCHY
*  ist2 +2      XL                          ist2 +6     YL
*  ist2 +3      XU                          ist2 +7     YU
*  ist2 +4   FACTX                          ist2 +8  FACTY
*
*          ----------------------------------
*           All kind of sums except of maxwt
*          ----------------------------------
*  (12-24) Under/over-flow average x    (16-24)
*  ist3 +1   Underflow                     All nine combinations
*  ist3 +2   Normal                        (U,N,O) x (U,N,O)
*  ist3 +3   Overerflow                    sum wt only (no errors)
*  ist3 +4   U  sum w**2
*  ist3 +5   N  sum w**2
*  ist3 +6   O  sum w**2
*  ist3 +7   Sum 1
*  ist3 +8   Sum wt*x
*  ist3 +9   Sum wt*x*x
*  ist3 +10  nevzer    (gmonit)
*  ist3 +11  nevove    (gmonit)
*  ist3 +12  nevacc    (gmonit)
*  ist3 +13  maxwt     (gmonit)
*          ----------------------------------
*           Content of bins including errors
*          ----------------------------------
*  (25 to 24+2*nchx)                     (25 to 24 +nchx*nchy)
*     sum wt and sum wt**2            sum wt only (no errors)
*  ----------------------------------------------------------------

      SUBROUTINE g_init
*     ****************
! First Initialization called from may routines
*     *************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      SAVE   / cglib /
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/
      DATA init /0/
      SAVE init
*
      IF(init .NE. 0) RETURN
      init=1
! this is version version number
      nvrs=111
! default output unit
      nout=16
      lenmax=0
      length=0
      DO i=1,idmx
         DO k=1,3
            index(i,k)=0
         ENDDO
         DO k=1,80
            titlc(i)(k:k)=' '
         ENDDO
      ENDDO
      DO k=1,50000
         b(k)=0d0
      ENDDO
      END

      SUBROUTINE gflush
*     ****************
! FLUSH memory, all histos erased!
*     *************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      SAVE   / cglib /
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/

      CALL g_init
      length=0
      DO i=1,idmx
         DO k=1,3
            index(i,k)=0
         ENDDO
         DO k=1,80
            titlc(i)(k:k)=' '
         ENDDO
      ENDDO
      DO k=1,50000
         b(k)=0d0
      ENDDO
      END

      LOGICAL FUNCTION gexist(id)
!     ***************************
! this function is true when id  exists !!!! 
!     ***************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      lact=jadres(id)
      gexist = lact .NE. 0
!###  IF(gexist)      WRITE(6,*) 'gexist: does   ID,lact= ',id,lact
!###  IF(.not.gexist) write(6,*) 'gexist: doesnt ID,lact= ',id,lact
      END

      FUNCTION gi(id,ib)
!     ******************
! getting out bin content
! S.J. 18-Nov. 90
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      SAVE idmem,nch,lact,ist,ist2,ist3
      DATA idmem / -1256765/
!
      IF(id .EQ. idmem) goto 100
      idmem=id
! some checks, not repeated if id the same as previously
      lact=jadres(id)
      IF(lact .EQ. 0) THEN
        WRITE(nout,*) ' gi: nonexisting histo id=',id
        WRITE(   6,*) ' gi: nonexisting histo id=',id
        gi= 0d0
        STOP
      ENDIF
      ist  = index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
! checking if histo is of proper type
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) THEN
        WRITE(nout,*) ' gi: 1-dim histos only !!! id=',id
        WRITE(   6,*) ' gi: 1-dim histos only !!! id=',id
        gi= 0d0
        STOP
      ENDIF
  100 continue
      nch  = nint(b(ist2+1))
      IF(ib .EQ. 0) THEN
! underflow
         gi=   b(ist3 +1)
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN
! normal bin
         gi=   b(ist +nbuf+ib)
      ELSEIF(ib .EQ. nch+1) THEN
! overflow
         gi=   b(ist3 +3)
      ELSE
! abnormal exit
         WRITE(nout,*) ' gi: wrong binning id,ib=',id,ib
         WRITE(   6,*) ' gi: wrong binning id,ib=',id,ib
         gi=0d0
         STOP
      ENDIF
      END

      FUNCTION  gie(id,ib)
!     ********************
! getting out error of the bin
! s.j. 18-nov. 90
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      SAVE idmem,nch,lact,ist,ist2,ist3
      DATA idmem / -1256765/
!
      IF(id .EQ. idmem) goto 100
      idmem=id
! some checks, not repeated if id the same as previously
      lact=jadres(id)
      IF(lact .EQ. 0) THEN
        WRITE(nout,*) ' gie: nonexisting histo id=',id
        WRITE(   6,*) ' gie: nonexisting histo id=',id
        gie= 0d0
        STOP
      ENDIF
      ist  = index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
! checking if histo is of proper type
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) THEN
        WRITE(nout,*) ' gie: 1-dim histos only !!! id=',id
        WRITE(   6,*) ' gie: 1-dim histos only !!! id=',id
        gie= 0d0
        STOP
      ENDIF
  100 continue
      nch  = b(ist2+1)
      IF(ib .EQ. 0) THEN
! underflow
         gie=   dsqrt( dabs(b(ist3 +4)))
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN
!...normal bin, error content
         gie=   dsqrt( dabs(b(ist+nbuf+nch+ib)) )
      ELSEIF(ib .EQ. nch+1) THEN
! overflow
         gie=   dsqrt( dabs(b(ist3 +6)))
      ELSE
! abnormal exit
         WRITE(nout,*) ' gie: wrong binning id, ib=',id,ib
         WRITE(   6,*) ' gie: wrong binning id, ib=',id,ib
         gie=0d0
         STOP
      ENDIF
      END

      SUBROUTINE gf1(id,xx,wtx)
!     ************************
! recommended fast filling 1-dim. histogram s.j. 18 nov. 90
! overflow/underflow corrected by Maciek and Zbyszek
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE
!
      lact=jadres(id)
! exit for non-existig histo
      IF(lact .EQ. 0)  RETURN
      ist  = index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
! one-dim. histo only
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) RETURN
      x1= xx
      wt1= wtx
      index(lact,3)=index(lact,3)+1
! all entries
      b(ist3 +7)  =b(ist3 +7)   +1
! for average x
      b(ist3 +8)  =b(ist3 +8)  +wt1*x1
      b(ist3 +9)  =b(ist3 +9)  +wt1*x1*x1
! filling coordinates
      nchx  =b(ist2 +1)
      xl    =b(ist2 +2)
      xu    =b(ist2 +3)
      factx =b(ist2 +4)
      IF(x1 .LT. xl) THEN
! underflow
         iposx1 = ist3 +1
         ipose1 = ist3 +4
         kposx1 = 0
      ELSEIF(x1 .GT. xu) THEN
! or overflow
         iposx1 = ist3 +3
         ipose1 = ist3 +6
         kposx1 = 0
      ELSE
! or any normal bin
         iposx1 = ist3 +2
         ipose1 = ist3 +5
! or given normal bin
         kx = (x1-xl)*factx+1d0
         kx = MIN( MAX(kx,1) ,nchx)
         kposx1 = ist +nbuf+kx
         kpose1 = ist +nbuf+nchx+kx
      ENDIF
      b(iposx1) = b(iposx1)  +wt1
      b(ipose1) = b(ipose1)  +wt1*wt1
      IF( kposx1 .NE. 0) b(kposx1) = b(kposx1)  +wt1
      IF( kposx1 .NE. 0) b(kpose1) = b(kpose1)  +wt1*wt1
      END   !gf1

      SUBROUTINE gf1diff(id,xx,wtx,yy,wty)
!     ************************************
! Special filling routine to fill the difference f(x)-g(y)
! in the case when f and g are very similar x and y are close for each event.
! In this case coherent filling is done if x and y fall into the same bin.
! Note that bin width starts to be important in this method.
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE
!
      lact=jadres(id)
! exit for non-existig histo
      IF(lact .EQ. 0)  RETURN
      ist  = index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
! one-dim. histo only
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) RETURN
      x1= xx
      x2= yy
      wt1= wtx
      wt2= wty
      index(lact,3)=index(lact,3)+1
! all entries
      b(ist3 +7)  =b(ist3 +7)   +1
! for average x or y not very well defined yet
      b(ist3 +8)  =b(ist3 +8)  +wt1*x1 - wt2*x2
      b(ist3 +9)  =b(ist3 +9)  +wt1*x1*x1 - wt2*x2*x2
! filling coordinates
      nchx  =b(ist2 +1)
      xl    =b(ist2 +2)
      xu    =b(ist2 +3)
      factx =b(ist2 +4)
* first variable
      IF(x1 .LT. xl) THEN       ! underflow
         ix1 = ist3 +1
         ie1 = ist3 +4
         kx1 = 0
      ELSEIF(x1 .GT. xu) THEN   ! or overflow
         ix1 = ist3 +3
         ie1 = ist3 +6
         kx1 = 0
      ELSE                      ! normal bin
         ix1 = ist3 +2
         ie1 = ist3 +5
         kx = (x1-xl)*factx+1d0
         kx = MIN( MAX(kx,1) ,nchx)
         kx1 = ist +nbuf+kx
         ke1 = ist +nbuf+nchx+kx
      ENDIF
* second variable
      IF(x2 .LT. xl) THEN       ! underflow
         ix2 = ist3 +1
         ie2 = ist3 +4
         kx2 = 0
      ELSEIF(x2 .GT. xu) THEN   ! or overflow
         ix2 = ist3 +3
         ie2 = ist3 +6
         kx2 = 0
      ELSE                      ! normal bin
         ix2 = ist3 +2
         ie2 = ist3 +5
         kx = (x2-xl)*factx+1d0
         kx = MIN( MAX(kx,1) ,nchx)
         kx2 = ist +nbuf+kx
         ke2 = ist +nbuf+nchx+kx
      ENDIF
* coherent filling
      IF( ix1 .EQ. ix2 ) THEN
         b(ix1) = b(ix1)  +wt1-wt2
         b(ie1) = b(ie1)  +(wt1-wt2)**2
      ELSE
         b(ix1) = b(ix1)  +wt1
         b(ie1) = b(ie1)  +wt1*wt1
         b(ix2) = b(ix2)  -wt2
         b(ie2) = b(ie2)  +wt2*wt2
      ENDIF
      IF( kx1 .EQ. kx2 ) THEN
         IF( kx1 .NE. 0) THEN 
            b(kx1) = b(kx1)  +wt1-wt2
            b(ke1) = b(ke1)  +(wt1-wt2)**2
         ENDIF
      ELSE
         IF( kx1 .NE. 0) THEN 
            b(kx1) = b(kx1)  +wt1
            b(ke1) = b(ke1)  +wt1*wt1
         ENDIF
         IF( kx2 .NE. 0) THEN 
            b(kx2) = b(kx2)  -wt2
            b(ke2) = b(ke2)  +wt2*wt2
         ENDIF
      ENDIF
      END   !gf1diff

      SUBROUTINE gfill(id,x,y,wtw)
*     ****************************
! this routine not finished, 1-dim only!
*     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE
!
      lact=jadres(id)
      IF(lact .EQ. 0)  RETURN
      ist  = index(lact,2)
! one-dim. histo 
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 1) THEN
!...one-dim. histogram
        CALL gf1(id,x,wtw)
        RETURN
      ENDIF
!...two-dim. scattergram, no errors!
      ist2 = ist+7
      ist3 = ist+15
      xx= x
      yy= y
      wt= wtw
      index(lact,3)=index(lact,3)+1
! x-axis
      nchx  =b(ist2 +1)
      xl    =b(ist2 +2)
      factx =b(ist2 +4)
      kx=(xx-xl)*factx+1d0
      lx=2
      IF(kx .LT. 1)     lx=1
      IF(kx .GT. nchx)  lx=3
      l     = ist+34  +lx
      b(l)  = b(l)    +wt
      k     = ist+nbuf2  +kx
      IF(lx .EQ. 2) b(k)  =b(k)  +wt
      k2    = ist+nbuf2  +nchx+kx
      IF(lx .EQ. 2) b(k2) =b(k2) +wt**2
! y-axix
      nchy  =b(ist2 +5)
      yl    =b(ist2 +6)
      facty =b(ist2 +8)
      ky=(yy-yl)*facty+1d0
      ly=2
      IF(ky .LT. 1)    ly=1
      IF(ky .GT. nchy) ly=3
! under/over-flow
      l = ist3  +lx +3*(ly-1)
      b(l) =b(l)+wt
! regular bin
      k = ist+nbuf2 +kx +nchx*(ky-1)
      IF(lx .EQ. 2.and.ly .EQ. 2) b(k)=b(k)+wt
      END

      SUBROUTINE gbook1(id,title,nnchx,xxl,xxu)
*     *****************************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      CHARACTER*80 title
      LOGICAL gexist
!
      CALL g_init
      IF(gexist(id)) goto 900
      ist=length
      lact=jadres(0)
! the case of no free entry in the index
      IF(lact .EQ. 0) goto 901
      index(lact,1)=id
      index(lact,2)=length
      index(lact,3)=0
*----
!cc      WRITE(6,*) 'GBOOK1: ID= ',ID
! -------
      CALL copch(title,titlc(lact))
      nchx =nnchx
      xl   =xxl
      xu   =xxu
! ---------- title and bin content ----------
      lengt2 = length +2*nchx +nbuf+1
      IF(lengt2 .GE. lenmax) goto 902
      do 10 j=length+1,lengt2+1
  10  b(j) = 0d0
      length=lengt2
!... default flags
      ioplog   = 1
      iopsla   = 1
      ioperb   = 1
      iopsc1   = 1
      iopsc2   = 1
      iflag1   = 
     $ ioplog+10*iopsla+100*ioperb+1000*iopsc1+10000*iopsc2
      ityphi   = 1
      iflag2   = ityphi
! examples of decoding flags 
!      id       = nint(b(ist+2)-9d0-9d12)/10
!      iflag1   = nint(b(ist+3)-9d0-9d12)/10
!      ioplog = mod(iflag1,10)
!      iopsla = mod(iflag1,100)/10
!      ioperb = mod(iflag1,1000)/100
!      iopsc1 = mod(iflag1,10000)/1000
!      iopsc2 = mod(iflag1,100000)/10000
!      iflag2   = nint(b(ist+4)-9d0-9d12)/10
!      ityphi = mod(iflag2,10)
!--------- buffer -----------------
! header
      b(ist +1)  = 9999999999999d0
      b(ist +2)  = 9d12 +     id*10 +9d0
      b(ist +3)  = 9d12 + iflag1*10 +9d0
      b(ist +4)  = 9d12 + iflag2*10 +9d0
! dummy vertical scale
      b(ist +5)  =  -100d0
      b(ist +6)  =   100d0
! pointer used to speed up search of histogram address
      b(ist +7)  =   0d0
! information on binning
      ist2       = ist+7
      b(ist2 +1) = nchx
      b(ist2 +2) = xl
      b(ist2 +3) = xu
      ddx = xu-xl
      IF(ddx .EQ. 0d0) goto 903
      b(ist2 +4) = float(nchx)/ddx
! under/over-flow etc.
      ist3       = ist+11
      do 100  j=1,13
 100  b(ist3 +j)=0d0
!
      RETURN
 900  continue
      WRITE(6   ,*) ' WARNING gbook1: already exists id=  ', id
      WRITE(NOUT,*) ' WARNING gbook1: already exists id=  ', id
      RETURN      
 901  continue
      CALL gstop1(' gbook1: to many histos !!!!!,     id=  ',id)
 902  continue
      CALL gstop1(' gbook1: to litle storage!!!!,  lenmax= ',lenmax)
 903  continue
      CALL gstop1('  gbook1:    xl=xu,               id=   ',id)
      END

      SUBROUTINE gstop1(mesage,id)
*     *******************************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE   /gind/
      CHARACTER*40 mesage

      WRITE(nout,'(a)') 
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(nout,'(a,a,i10,a)')  
     $                          '+ ', mesage, id, ' +'
      WRITE(nout,'(a)') 
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6   ,'(a)') 
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6   ,'(a,a,i10,a)')  
     $                          '+ ', mesage, id, ' +'
      WRITE(6   ,'(a)') 
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      STOP
      END


      SUBROUTINE goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
!     ********************************************************
! decoding option flags
!     **********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/

      lact=jadres(id)
      IF(lact .EQ. 0) RETURN
      ist=index(lact,2)
! decoding flags 
      iflag1   = nint(b(ist+3)-9d0-9d12)/10
      ioplog = mod(iflag1,10)
      iopsla = mod(iflag1,100)/10
      ioperb = mod(iflag1,1000)/100
      iopsc1 = mod(iflag1,10000)/1000
      iopsc2 = mod(iflag1,100000)/10000
      END

      SUBROUTINE gidopt(id,ch)
!     ************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      CHARACTER*4 ch
!
      lact=jadres(id)
      IF(lact .EQ. 0) RETURN
      ist=index(lact,2)
! decoding flags 
      CALL goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      IF(ch .EQ.       'LOGY'  ) THEN
! log scale for print
        ioplog = 2 
      ELSEIF(ch .EQ.   'ERRO'  ) THEN
! errors in printing/plotting
       ioperb  = 2
      ELSEIF(ch .EQ.   'SLAN'  ) THEN
! slanted line in plotting
       iopsla  = 2
      ELSEIF(ch .EQ.   'YMIN'  ) THEN
       iopsc1  = 2
      ELSEIF(ch .EQ.   'YMAX'  ) THEN
       iopsc2  = 2
      ENDIF
! encoding back
      iflag1   = 
     $ ioplog+10*iopsla+100*ioperb+1000*iopsc1+10000*iopsc2
      b(ist+3) = 9d12 + iflag1*10 +9d0
      END


      SUBROUTINE gbfun1(id,title,nchx,xmin,xmax,func)
!     ***********************************************
! ...fills histogram with function func(x)
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/
      DIMENSION yy(200)
      EXTERNAL func
      CHARACTER*80 title
      LOGICAL gexist
!
      CALL g_init
      IF(gexist(id)) GOTO 900
 15   xl=xmin
      xu=xmax
      CALL gbook1(id,title,nchx,xl,xu)
!...slanted line in plotting
      CALL gidopt(id,'SLAN')
      IF(nchx .GT. 200) goto 901
      DO 20 ib=1,nchx
      x= xmin +(xmax-xmin)/nchx*(ib-0.5d0)
      yy(ib) = func(x)
   20 CONTINUE
      CALL gpak(id,yy)
      RETURN
 900  WRITE(nout,*) ' +++gbfun1: already exists id=',id
      WRITE(6   ,*) ' +++gbfun1: already exists id=',id      
      CALL gdelet(id)
      GO to 15
 901  WRITE(nout,*) ' +++gbfun1: to many bins'
      END

      SUBROUTINE gbfun2(id,title,nchx,xmin,xmax,func)
!     ***********************************************
! ...fills histogram with function func(x)
!.. three point fit used
!     ***********************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/
      DIMENSION yy(200),yy1(0:200)
      EXTERNAL func
      CHARACTER*80 title
      LOGICAL gexist
!
      CALL g_init
      IF( gexist(id) ) GOTO 900
 15   xl=xmin
      xu=xmax
      CALL gbook1(id,title,nchx,xl,xu)

!...slanted line in plotting
      CALL gidopt(id,'SLAN')
      IF(nchx.gt.200) GOTO 901

      yy1(0) = func(xmin)
      dx=(xmax-xmin)/nchx

      DO ib=1,nchx
         x2= xmin +dx*(ib-0.5d0)
         x3= x2 +dx*0.5d0
         yy(ib)  = func(x2)
         yy1(ib) = func(x3)
!..  simpson 
         yy(ib) = ( yy1(ib-1) +4*yy (ib) +yy1(ib))/6d0
      ENDDO

      CALL gpak(id,yy)
      RETURN
 900  WRITE(nout,*) ' +++gbfun2: already exists id=',id
      WRITE(6   ,*) ' +++gbfun2: already exists id=',id      
      CALL gdelet(id)
      GO TO 15
 901  WRITE(nout,*) ' +++gbfun2: to many bins'
      END



      SUBROUTINE GBOOK2(ID,TITLE,NCHX,XL,XU,NCHY,YL,YU)
*     *************************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      PARAMETER( IDMX=400,NBUF=24,NBUF2=24)
      COMMON / Cglib / B(50000)
      COMMON /GIND/ NVRS,NOUT,LENMAX,LENGTH,INDEX(IDMX,3),TITLC(IDMX)
      CHARACTER*80 TITLC
      SAVE /cglib/,/gind/
      CHARACTER*80 TITLE
      LOGICAL GEXIST
!
      CALL g_init
      IF(GEXIST(ID)) GOTO 900
      ist=length
      LACT=JADRES(0)
      IF(LACT .EQ. 0) GOTO 901
      index(LACT,1)=ID
      index(LACT,2)=length
      CALL COPCH(TITLE,TITLC(LACT))
      nnchx=NCHX
      nnchy=NCHY
      LENGT2 = LENGTH  +44+nnchx*nnchy
      IF(LENGT2 .GE. LENMAX) GOTO 902
      DO 10 J=LENGTH+1,LENGT2+1
   10 B(J) = 0D0
      LENGTH=LENGT2
      B(ist+1)=nnchx
      B(ist+2)=XL
      B(ist+3)=XU
      B(ist+4)=float(nnchx)/(b(ist+3)-b(ist+2))
      B(ist+5)=nnchy
      B(ist+6)=YL
      B(ist+7)=YU
      B(ist+8)=float(nnchy)/(b(ist+7)-b(ist+6))
      RETURN
  900 WRITE(NOUT,*) ' GBOOK2: HISTO ALREADY EXISTS!!!! ID=',ID
      RETURN
  901 WRITE(NOUT,*) ' GBOOK2: TO MANY HISTOS !!!!!',LACT
      STOP
  902 WRITE(NOUT,*) ' GBOOK2: TO LITLE STORAGE!!!!',LENMAX
      STOP
      END

      SUBROUTINE gistdo
*     *****************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/
      do 10 i=1,idmx
      id=index(i,1)
      IF(id .GT. 0) call gprint(id)
   10 continue
      END

      SUBROUTINE goutpu(ilun)
*     ***********************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/
      CALL g_init
      nout=ilun
      END


      SUBROUTINE gprint(id)
*     *********************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      CHARACTER*1 line(0:105),lchr(22),lb,lx,li,l0
      LOGICAL llg
      SAVE lb,lx,li,l0,lchr
      DATA lb,lx,li,l0 /' ','X','I','0'/
      DATA lchr/' ','1','2','3','4','5','6','7','8','9',
     $      'A','B','C','D','E','F','G','H','I','J','K','*'/

      lact=jadres(id)
      IF(lact .EQ. 0) goto 900
      ist  = index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      idec    = nint(b(ist+2)-9d0-9d12)/10
      IF(idec .NE. id) write(6,*) '++++GPRINT: PANIC! ID,IDEC= ',ID,IDEC

      CALL goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker    =  ioperb-1
      lmx = 67
      IF(ker .EQ. 1) lmx=54
      nent=index(lact,3)
      IF(nent  .EQ.  0)                          GOTO 901
      WRITE(nout,1000) id,titlc(lact)
 1000 FORMAT('1',/,1X,I9,10X,A)
!
! one-dim. histo 
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) goto 200
      nchx =   b(ist2 +1)
      xl   =   b(ist2 +2)
      dx   =  (  b(ist2 +3)-b(ist2 +2)  )/float(nchx)
! fixing vertical scale
      istr=ist+nbuf+1
      bmin = b(istr)
      bmax = b(istr)+1d-5*abs(b(istr))  ! problems for single bin case
      do 15 ibn=istr,istr+nchx-1
      bmax = max(bmax,b(ibn))
      bmin = min(bmin,b(ibn))
  15  continue
      IF(bmin  .EQ.  bmax)                       GOTO 903
      IF(iopsc1 .EQ. 2) bmin=b(ist +5)
      IF(iopsc2 .EQ. 2) bmax=b(ist +6)
!
      llg=ioplog .EQ. 2
      IF(llg.and.bmin .LE. 0d0) bmin=bmax/10000.d0
!
      deltb = bmax-bmin
      IF(deltb  .EQ.  0d0)                       GOTO 902
      fact  = (lmx-1)/deltb
      kzer  = -bmin*fact+1.00001d0
      IF(llg) fact=(lmx-1)/(log(bmax)-log(bmin))
      IF(llg) kzer=-log(bmin)*fact+1.00001d0
!
      undf = b(ist3 +1)
      ovef = b(ist3 +3)
      avex = 0d0
      sum  = b(ist3 +8)
      IF(nent .NE. 0) avex = sum/nent
      WRITE(nout,'(4a15      )')  'nent','sum','bmin','bmax'
      WRITE(nout,'(i15,3e15.5)')   nent,  sum,  bmin,  bmax
      WRITE(nout,'(4a15  )')      'undf','ovef','avex'
      WRITE(nout,'(4e15.5)')       undf,  ovef,  avex
!
      IF(llg) write(nout,1105)
 1105 format(35x,17hlogarithmic scale)
!
      kzer=max0(kzer,0)
      kzer=min0(kzer,lmx)
      xlow=xl
      do 100 k=1,nchx
! first fill with blanks
      do  45 j=1,105
   45 line(j)  =lb
! THEN fill upper and lower boundry
      line(1)  =li
      line(lmx)=li
      ind=istr+k-1
      bind=b(ind)
      bind= max(bind,bmin)
      bind= min(bind,bmax)
      kros=(bind-bmin)*fact+1.0001d0
      IF(llg) kros=log(bind/bmin)*fact+1.0001d0
      k2=max0(kros,kzer)
      k2=min0(lmx,max0(1,k2))
      k1=min0(kros,kzer)
      k1=min0(lmx,max0(1,k1))
      do 50 j=k1,k2
   50 line(j)=lx
      line(kzer)=l0
      z=b(ind)
      IF(ker .NE. 1) THEN
        WRITE(nout,'(a, f7.4,  a, d14.6,  132a1)') 
     $             ' ', xlow,' ',     z,' ',(line(i),i=1,lmx)
      ELSE
        er=dsqrt(dabs(b(ind+nchx)))
        WRITE(nout,'(a,f7.4,  a,d14.6,  a,d14.6, 132a1 )') 
     $             ' ',xlow,' ',    z,' ',   er,' ',(line(i),i=1,lmx)
      ENDIF
      xlow=xlow+dx
  100 continue
      RETURN
!------------- two dimensional requires complete restoration!!!----------------
  200 continue
      nchx=B(ist+1)
      nchy=B(ist+5)
      WRITE(nout,2000) (lx,i=1,nchy)
 2000 format(1h ,10x,2hxx,100a1)
      do 300 kx=1,nchx
      do 250 ky=1,nchy
      k=ist +NBUF2 +kx+nchx*(ky-1)
      N=B(K)+1.99999D0
      n=max0(n,1)
      n=min0(n,22)
      IF(DABS(b(k)) .LT. 1D-20) n=1
      line(ky)=lchr(n)
  250 continue
      line(nchy+1)=lx
      i1=nchy+1
      WRITE(nout,2100) (line(i),i=1,i1)
 2100 format(1h ,10x,1hx,100a1)
  300 continue
      WRITE(nout,2000) (lx,i=1,nchy)
      RETURN
  900 WRITE(NOUT,*) ' +++GPRINT: NONEXISTING HISTO',ID
      WRITE(6   ,*) ' +++GPRINT: NONEXISTING HISTO',ID
      RETURN
 901  WRITE(NOUT,*) ' +++GPRINT: nent.eq.0',ID
      WRITE(   6,*) ' +++GPRINT: nent.eq.0',ID
      RETURN
 902  WRITE(NOUT,*) ' +++GPRINT: wrong plotting limits',ID,bmin,bmax
      WRITE(   6,*) ' +++GPRINT: wrong plotting limits',ID,bmin,bmax
      RETURN
 903  WRITE(NOUT,*) ' +++GPRINT: bmin.eq.bmax',ID,bmin
      WRITE(   6,*) ' +++GPRINT: bmin.eq.bmax',ID,bmin
      END

      SUBROUTINE gopera(ida,chr,idb,idc,coef1,coef2)
*     **********************************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      CHARACTER*80 title
      CHARACTER*1  chr
!
      lacta=jadres(ida)
      IF(lacta .EQ. 0) RETURN
      ista  = index(lacta,2)
      ista2 = ista+7
      ncha  = b(ista2+1)
!
      lactb =jadres(idb)
      IF(lactb .EQ. 0) RETURN
      istb  = index(lactb,2)
      istb2 = istb+7
      nchb  = b(istb2+1)
      IF(nchb .NE. ncha) goto 900
!
      lactc=jadres(idc)
      IF(lactc .EQ. 0) THEN
! ...if nonexistent, histo idc is here defined
        CALL ginbo1(ida,title,nchx,xl,xu)
        CALL gbook1(idc,title,nchx,xl,xu)
        lactc = jadres(idc)
        istc  = index(lactc,2)
!...option copied from ida
        b(istc+ 3)= b(ista +3)
      ENDIF
!...one nominal entry recorded
      index(lactc,3) = 1
!
      istc  =  index(lactc,2)
      istc2 =  istc+7
      nchc  =  b(istc2+1)
!
      IF(nchc .NE. ncha) goto 900
      IF(ncha .NE. nchb.or.nchb .NE. nchc) goto 900
      do 30 k=1,ncha
      i1 = ista+nbuf+k
      i2 = istb+nbuf+k
      i3 = istc+nbuf+k
      j1 = ista+nbuf+ncha+k
      j2 = istb+nbuf+ncha+k
      j3 = istc+nbuf+ncha+k
      if    (chr .EQ. '+')   THEN
        b(i3) =    coef1*b(i1) +    coef2*b(i2)
        b(j3) = coef1**2*b(j1) + coef2**2*b(j2)
      ELSEIF(chr .EQ. '-')   THEN
        b(i3) = coef1*b(i1) - coef2*b(i2)
        b(j3) = coef1**2*b(j1) + coef2**2*b(j2)
      ELSEIF(chr .EQ. '*')   THEN
        b(j3) = (coef1*coef2)**2
     $          *(b(j1)*b(i2)**2 + b(j2)*b(i1)**2)
        b(i3) = coef1*b(i1) * coef2*b(i2)
      ELSEIF(chr .EQ. '/')   THEN
        IF(b(i2) .EQ. 0d0) THEN
          b(i3) = 0d0
          b(j3) = 0d0
        ELSE
          b(j3) = (coef1/coef2)**2/b(i2)**4
     $          *(b(j1)*b(i2)**2 + b(j2)*b(i1)**2)
          b(i3) = (coef1*b(i1) )/( coef2*b(i2))
        ENDIF
      ELSE
        goto 901
      ENDIF
   30 continue
      RETURN
  900 write(nout,*) '+++++ gopera: non-equal no. bins ',ida,idb,idc
      WRITE(   6,*) '+++++ gopera: non-equal no. bins ',ida,idb,idc
      RETURN
  901 write(nout,*) '+++++ gopera: wrong chr=',chr
      END

      SUBROUTINE ginbo1(id,title,nchx,xl,xu)
!     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      CHARACTER*80 title
!
      lact=jadres(id)
      IF(lact .EQ. 0) THEN
         WRITE(6,*) '+++++ STOP in ginbo1: wrong id=',id
         STOP
      ENDIF
      ist=index(lact,2)
      ist2   = ist+7
      nchx   = b(ist2 +1)
      xl     = b(ist2 +2)
      xu     = b(ist2 +3)
      title  = titlc(lact)
      END

      SUBROUTINE gunpak(id,a,chd1,idum)
*     *********************************
! getting out histogram content (and error)
! chd1= 'ERRO' is nonstandard option (unpack errors)
*     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      CHARACTER*(*) chd1
      dimension a(*)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      lact=jadres(id)
      IF(lact .EQ. 0) goto 900
      ist   = index(lact,2)
      ist2  = ist+7
      nch   = b(ist2 +1)
      local = ist +nbuf
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 2) THEN
        nchy  = b(ist2+5)
        nch   = nch*nchy
        local = ist+ nbuf2
      ENDIF
      do 10 ib=1,nch
      IF(chd1 .NE. 'ERRO') THEN
! normal bin
        a(ib) = b(local+ib)
      ELSE
! error content
        IF(ityphi .EQ. 2) goto 901
        a(ib) = dsqrt( dabs(b(local+nch+ib) ))
      ENDIF
   10 continue
      RETURN
 900  write(nout,*) '+++gunpak: nonexisting id=',id
      WRITE(6   ,*) '+++gunpak: nonexisting id=',id
      RETURN
 901  write(nout,*) '+++gunpak: no errors, two-dim, id=',id
      END

      SUBROUTINE gpak(id,a)
!     *********************
! Loading in histogram content
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      DIMENSION  a(*)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      lact=jadres(id)
      IF(lact .EQ. 0) goto 900
      ist  = index(lact,2)
      ist2 = ist+7
      nch=b(ist2 +1)
      local = ist+nbuf
! 2-dimens histo alowed
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 2) THEN
        nchy  = b(ist2+5)
        nch   = nch*nchy
        local = ist+nbuf2
      ENDIF
      do 10 ib=1,nch
   10 b(local +ib) = a(ib)
! one nominal entry recorded
      index(lact,3)  = 1
      RETURN
  900 write(nout,*) '+++gpak: nonexisting id=',id
      WRITE(6   ,*) '+++gpak: nonexisting id=',id
      END

      SUBROUTINE gpake(id,a)
!     **********************
! Loading in error content
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      DIMENSION  a(*)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      lact=jadres(id)
      IF(lact .EQ. 0) goto 901
      ist  = index(lact,2)
      ist2 = ist+7
      nch=b(ist2+1)
! 2-dimens histo NOT alowed
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 2) goto 900
      do 10 ib=1,nch
   10 b(ist+nbuf+nch+ib) = a(ib)**2
      RETURN
  900 write(nout,*) ' +++++ gpake: only for one-dim histos'
      RETURN
  901 write(nout,*) '+++ gpake: nonexisting id=',id
      WRITE(6   ,*) '+++ gpake: nonexisting id=',id
      END


      SUBROUTINE grang1(id,ylr,yur)
*     *****************************
! provides y-scale for 1-dim plots
*     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      lact=jadres(id)
      IF(lact .EQ. 0) RETURN
      ist  = index(lact,2)
      ist2 = ist+7
      nch  = b(ist2 +1)
      yl   = b(ist+nbuf+1)
      yu   = b(ist+nbuf+1)
      do 10 ib=1,nch
      yl = min(yl,b(ist+nbuf+ib))
      yu = max(yu,b(ist+nbuf+ib))
   10 continue
      CALL goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      IF(iopsc1 .EQ. 2) yl= b( ist +5)
      IF(iopsc2 .EQ. 2) yu= b( ist +6)
      ylr = yl
      yur = yu
      END


      SUBROUTINE ginbo2(id,nchx,xl,xu,nchy,yl,yu)
*     *******************************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      lact=jadres(id)
      IF(lact .EQ. 0) goto 900
      ist  = index(lact,2)
      ist2 = ist+7
      nchx = b(ist2 +1)
      xl   = b(ist2 +2)
      xu   = b(ist2 +3)
      nchy = b(ist2 +5)
      yl   = b(ist2 +6)
      yu   = b(ist2 +7)
      RETURN
  900 write(nout,*) ' +++ginbo2: nonexisting histo id= ',id 
      WRITE(   6,*) ' +++ginbo2: nonexisting histo id= ',id
      END


      SUBROUTINE gmaxim(id,wmax)
*     **************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      IF(id .NE. 0) THEN
        lact=jadres(id)
        IF(lact .EQ. 0) RETURN
        ist= index(lact,2)
        b(ist+6) =wmax
        CALL gidopt(id,'YMAX')
      ELSE
        do 20 k=1,idmx
        IF(index(k,1) .EQ. 0) goto 20
        ist=index(k,2)
        jd =index(k,1)
        b(ist+6) =wmax
        CALL gidopt(jd,'YMAX')
   20   continue
      ENDIF
      END

      SUBROUTINE gminim(id,wmin)
*     **************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      IF(id .NE. 0) THEN
        lact=jadres(id)
        IF(lact .EQ. 0) RETURN
        ist =index(lact,2)
        b(ist+5) =wmin
        CALL gidopt(id,'YMIN')
      ELSE
        do 20 k=1,idmx
        IF(index(k,1) .EQ. 0) goto 20
        ist=index(k,2)
        jd =index(k,1)
        b(ist+5) =wmin
        CALL gidopt(jd,'YMIN')
   20   continue
      ENDIF
      END

      SUBROUTINE gmimax(id,wmin,wmax)
*     ******************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      CALL gminim(id,wmin)
      CALL gmaxim(id,wmax)
      END
    

      SUBROUTINE greset(id,chd1)
*     **************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      CHARACTER*(*) chd1
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
!
      lact=jadres(id)
      IF(lact .LE. 0) RETURN
      ist  =index(lact,2)
      ist2 = ist+7
! 
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 1) THEN
! one-dim.
        ist3  = ist+11
        nchx  = b(ist2 +1)
        nch   = 2*nchx
        local = ist + nbuf
      ELSEIF(ityphi .EQ. 2) THEN
! two-dim.
        ist3  = ist+15
        nchx  = b(ist2 +1)
        nchy  = b(ist2 +5)
        nch   = nchx*nchy
        local = ist +nbuf2
      ELSE
         WRITE(nout,*) '+++greset: wrong type id=',id
         WRITE(6   ,*) '+++greset: wrong type id=',id
        RETURN
      ENDIF
! reset miscaelaneous entries and bins
      do 10 j=ist3+1,local +nch
  10  b(j)    = 0d0
! and no. of entries in index
      index(lact,3) = 0
      END

      SUBROUTINE GDELET(ID1)
*     *********************
! Now it should work (stj Nov. 91) but watch out!
! should works for 2-dim histos, please check this!
*     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/
      LOGICAL gexist
!
      ID=ID1
      IF(id .EQ. 0) GOTO 300
      IF(.not.gexist(id)) GOTO 900
      lact = jadres(id)
      ist  = index(lact,2)
      ist2 = ist+7
*----
![[[      WRITE(6,*) 'GDELET-ing ID= ',ID
      idec    = nint(b(ist+2)-9d0-9d12)/10
      IF(idec .NE. id) WRITE(6,*) '++++GDELET: ALARM! ID,IDEC= ',ID,IDEC
*----
      nch  = b(ist2 +1)
      iflag2   = nint(b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 1) THEN
! one-dim.
        nchx  = b(ist2 +1)
        nch   = 2*nchx
! lenght of local histo to be removed
        local = nch+nbuf+1
      ELSEIF(ityphi .EQ. 2) THEN
! two-dim.
        nchx  = b(ist2 +1)
        nchy  = b(ist2 +5)
        nch   = nchx*nchy
! lenght of local histo to be removed
        local = nch+nbuf2+1
      ELSE
         WRITE(nout,*) '+++gdelet: wrong type id=',id
         WRITE(6   ,*) '+++gdelet: wrong type id=',id
        RETURN
      ENDIF
! starting position of next histo in storage b
      next = ist+1 +local
! move down all histos above this one 
      DO 15 k =next,length
      b(k-local)=b(k)
   15 CONTINUE  
! define new end of storage
      length=length-local
! clean free space at the end of storage b
      DO 20 k=length+1, length+local
   20 b(k)=0d0 
! shift adresses of all displaced histos 
      DO 25 l=lact+1,idmx
      IF(index(l,1) .NE. 0) index(l,2)=index(l,2)-local
   25 CONTINUE
! move entries in index down by one and remove id=lact entry
      DO 30 l=lact+1,idmx
      index(l-1,1)=index(l,1)
      index(l-1,2)=index(l,2)
      index(l-1,3)=index(l,3)
      titlc(l-1)=titlc(l)
   30 CONTINUE
! last entry should be always empty
      index(idmx,1)=0
      index(idmx,2)=0
      index(idmx,3)=0 
      do 50 k=1,80
   50 titlc(idmx)(k:k)=' '
      RETURN
! -----------------------------------
! Deleting all histos at once!!!
  300 length=0
      DO 400 i=1,idmx
      DO 340 k=1,3
  340 index(i,k)=0
      DO 350 k=1,80
  350 titlc(i)(k:k)=' '
 400  CONTINUE
      RETURN
! -----------------------------------
 900  CONTINUE
      WRITE(nout,*) ' +++GDELET: nonexisting histo id= ',id 
      WRITE(   6,*) ' +++GDELET: nonexisting histo id= ',id 
      END


      SUBROUTINE glimit(lenmx)
*     ************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/
      CALL g_init
      IF(lenmx .GE. lenmax) THEN
         lenmax=lenmx
      ELSE
         CALL gstop1('glimit: cant decrease storage lenmx  =',lenmx)
      ENDIF
      END

      SUBROUTINE copch(ch1,ch2)
*     *************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
* copies CHARACTER*80 ch1 into ch2 up to a first $ sign
      CHARACTER*80 ch1,ch2
      LOGICAL met
      met = .false.
      do 10 i=1,80
      IF( ch1(i:i) .EQ. '$' .or. met )   THEN
        ch2(i:i)=' '
        met=.true.
      ELSE
        ch2(i:i)=ch1(i:i)
      ENDIF
  10  continue
      END

      FUNCTION jadre2(id)
*     *********************
*------------------------------------------------
* Good old version -- but it is very very slow!!!
* In the case of 100 histograms or more.
*------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      jadre2=0
      DO 1 i=1,idmx
      IF(index(i,1) .EQ. id) goto 2
    1 CONTINUE
* Nothing found.
      RETURN
* Found: id=0 is also legitimate find!!!
    2 jadre2=i
      END

      FUNCTION jadres(id1)
*     *********************
*--------------------------------------------------------------------
* Educated guess based on past history is used to find quickly
* location of the histogram in the matrix index.
* This is based on observation that subsequent histogram calls 
* are linked into loops (so one can predict easily which histo will
* be called next time).
*--------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE /gind/
      DATA iguess,jdlast,idlast /-2141593,-3141593,-3141593/
      SAVE iguess,jdlast,idlast

      id=id1
! --- The case of ID=0 treated separately, it is used to find out
! --- last entry in the index (it is marked with zero)
      IF(id .EQ. 0) THEN
         DO i=1,idmx
            IF(index(i,1) .EQ. 0) goto 4
         ENDDO
         WRITE(6,*) '+++++jadres: STOP index to short'
         STOP
 4       CONTINUE
         jadres = i
         RETURN
      ENDIF

! --- Omit sophistications if lack of initialization
      IF(jdlast .EQ. -3141593) GOTO 10
      IF(iguess .EQ. -2141593) GOTO 10
      IF(iguess .EQ. 0) GOTO 10
      IF(jdlast .EQ. 0) GOTO 10

! --- Try first previous histo (for repeated calls)
      IF(jdlast .LT. 1 .OR. jdlast .GT. idmx) THEN
         WRITE(6,*) '+++++ jadres: jdlast=',jdlast
      ENDIF
      IF(index(jdlast,1) .EQ. id) THEN
         jadres = jdlast
!##   write(6,*) 
!##   $   'found, guess based on previous call to jadres ',jdlast
         GOTO 20
      ENDIF

! --- Try current guess based on previous call
      IF(iguess .LT. 1 .OR. iguess .GT. idmx)  THEN
         WRITE(6,*)'+++++ jadres: iguess=',iguess
      ENDIF
      IF(index(iguess,1) .EQ. id) THEN
         jadres = iguess
!##   write(6,*) 
!##   $   'found, guess on previous calls recorded in b(ist+7)',jdlast
         GOTO 20
      ENDIF

! ================================================
!    Do it HARD WAY, Search all matrix index
! ================================================
 10   CONTINUE
!##   write(6,*) 'trying HARD WAY'
      DO i=1,idmx
         jadres=i
         IF(index(i,1) .EQ. id) GOTO 20
      ENDDO
! -------------------------------------
!     Nothing found: jadres=0
! -------------------------------------
      jadres=0
      RETURN
! =====================================
!     Found: Set new guess for next call
! =====================================
 20   CONTINUE
! --- and store result as a new guess in previous histo 
! --- but only if it existed!!!!
      DO i=1,idmx
         IF(index(i,1) .EQ. 0) GOTO 40
         IF(index(i,1) .EQ. idlast) THEN
            ist=index(i,2)
            IF(ist .GT. 0 .AND. ist .LT. 50000) b(ist +7) = jadres
!##   write(6,*) 'STORED     id=',id
            GOTO 40
         ENDIF 
      ENDDO
 40   CONTINUE
!##   write(6,*)  'found, hard way searching all of index)', jdlast
      iguess = b( index(jadres,2) +7)
      jdlast = jadres
      idlast = id
      END


!--------------------------------------------------------------
! ----------- storing histograms in the disk file -------------
!--------------------------------------------------------------
      SUBROUTINE grfile(nhruni,dname,chd2)
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      CHARACTER*(*) chd2, dname
      COMMON / hruni / nhist
      SAVE /hruni/
      nhist=nhruni
      END

      SUBROUTINE grout(idum1,idum2,chdum)
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      CHARACTER*8 chdum
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      COMMON / hruni / nhist
      CHARACTER*80 titlc
      SAVE /cglib/,/gind/, /hruni/
!
      CALL g_init
      nouth=nhist
      WRITE(nouth,'(6i10)')   nvrs,nout,lenmax,length
      WRITE(nouth,'(6i10)')   ((index(i,k),k=1,3),i=1,idmx)
      WRITE(nouth,'(a80)')    titlc
      WRITE(nouth,'(3d24.16)') (b(i),i=1,length)
      END


      SUBROUTINE GRIN(IDUM1,IDUM2,IDUM3)
!     **********************************
! New version which has a possibility to 
!            MERGE histograms
! If given ID already exists then it is modified by adding 1000000 !!!!
! Mergigng is done simply by appending new histograms at the 
! very end of the index and bin matrices.
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      COMMON / hruni / nhist
      SAVE /cglib/,/gind/, /hruni/
! Copy of the new index from the disk
      DIMENSION lndex(idmx,3),titld(idmx)
      CHARACTER*80 titld
      LOGICAL gexist

      CALL g_init 
      nouth=nhist
! Read basic params
      READ(nouth,'(6i10)')   nvrs3,nout3,lenma3,lengt3
      IF(length+lengt3 .GE. lenmax) GOTO 900
! Check version
      IF(nvrs .NE. nvrs3) WRITE(nout,*)
     $ '  +++++ warning (grin): histos produced by older version',nvrs3
      IF(nvrs .NE. nvrs3) WRITE(6,*)
     $ '  +++++ warning (grin): histos produced by older version',nvrs3
! Read new index  from the disk
      READ(nouth,'(6i10)')  ((lndex(i,k),k=1,3),i=1,idmx)
      READ(nouth,'(a80)')    titld

      lenold=length
! Append content of new histos AT ONCE  at the end of storage b
      length=length+lengt3
      READ(nouth,'(3d24.16)') (b(i),i=lenold+1,length)

! Append index and titlc with new histos one by one
      lact = jadres(0)
      DO 100 l=1,idmx
      IF(lact .EQ. 0) GOTO 901
      idn= lndex(l,1)
      IF(idn .EQ. 0) GOTO 100
! Identical id's are changed by adding big number = 1000000
 10   CONTINUE
      IF( gexist(idn) ) THEN
         idn = idn +1000000*(idn/iabs(idn))
         GOTO 10 
      ENDIF
      index(lact,1)=idn
      index(lact,2)=lndex(l,2)+lenold
      index(lact,3)=lndex(l,3)
      titlc(lact)  =titld(l)
!
! Still one small correction in the newly appended histo
      istn  = index(lact,2)
      b(istn +2)  = 9d12 +     idn*10 +9d0
!
      lact=lact+1
  100 CONTINUE

!
      RETURN

 900  CONTINUE
      CALL gstop1('++++ grin: to litle space, lenmax=  ',lenmax)
 901  CONTINUE
      CALL gstop1('++++ grin: to many histos, idmx=    ',idmx)
      END




      SUBROUTINE GRIN2(IDUM1,IDUM2,IDUM3)
!     **********************************
! New version which has a possibility to 
!            ADD histograms
! If ID is not existing already then no action is taken
!     ***********************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      COMMON / hruni / nhist
      SAVE /cglib/,/gind/, /hruni/
! Copy of the histos from the disk
      DIMENSION bz(50000)
      DIMENSION indez(idmx,3),titlz(idmx)
      CHARACTER*80 titlz
      LOGICAL gexist

      CALL g_init 
      nouth=nhist
! Read basic params
      READ(nouth,'(6i10)')   nvrsz,noutz,lenmaz,lengtz
! Check version
      IF(nvrs .NE. nvrsz) WRITE(nout,*)
     $ '  +++++ warning (grin2): histos produced by older version',nvrsz
      IF(nvrs .NE. nvrsz) WRITE(6,*)
     $ '  +++++ warning (grin2): histos produced by older version',nvrsz
! Read new index, title and bins from the disk
      READ(nouth,'(6i10)')    ((indez(i,k),k=1,3),i=1,idmx)
      READ(nouth,'(a80)')     titlz
      READ(nouth,'(3d24.16)') (bz(i),i=1,lengtz)

! Add new histos from disk to existing ones one by one
      DO 100 lz=1,idmx
      id= indez(lz,1)
      IF(id .EQ. 0) GOTO 200
      IF(.not.gexist(id)) THEN
        WRITE(6,*) ' Grin2: unmached histo ID=', id, '  Skipped'
        goto 100
      ENDIF
! parameters of existing histo
      lact = jadres(id)
      ist  = index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      nchx = b(ist2 +1)
! parameters of the histo from the disk
      istz   = indez(lz,2)
      ist2z  = istz+7
      ist3z  = istz+11
      nchxz  = bz(ist2z +1)
      IF(nchx .NE. nchxz) THEN
        WRITE(6,*) ' Grin2: non-equal binning ID=', id, '  Skipped' 
        goto 100
      ENDIF
! Add/Merge all additive entries of the two histos
! No of entries in index
      index(lact,3) = index(lact,3)+indez(lact,3)
! Overflows, underflows etc.
      DO i=1,12
        b(ist3+i)=b(ist3+i) +bz(ist3z+i)
      ENDDO
! Except of this one non-additive entry 
      b(ist3+13)=max(b(ist3+13),b(ist3z+13))
! Regular bin content added now!
      DO i= 1, 2*nchx
        b(ist+nbuf+i)=b(ist+nbuf+i) +bz(istz+nbuf+i)
      ENDDO
  100 CONTINUE
  200 CONTINUE

      END

      SUBROUTINE grend(chdum)
!     ***********************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / hruni / nhist
      SAVE   /hruni/
      CHARACTER*(*) chdum
      CLOSE(nhist)
!======================================================================
!======================end of gbook====================================
!======================================================================
      END

!======================================================================
!======================Mini-GPLOT======================================
!======================================================================
!... Plotting using LaTeX
      SUBROUTINE gplint(ICOD)
!     ***********************
      SAVE
!---------------------------------------------------
! This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
!
      lint = icod
!
      END

      SUBROUTINE gplcap(IFILE)
!     ***********************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
!---------------------------------------------------
! This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
!----------------------------------
! Titles and captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 titch
!----------------
! Note that backslash definition is varying from one 
! instalation/compiler to another, you have to figure out by yourself 
! how to fill backslash code into BS
      COMMON / BSLASH / BS
      CHARACTER*1 BS,BBS
!     DATA BBS / 1H\ /
      DATA BBS / '\\' /
      BS = BBS
!c      BS = '\\'
!---------
      KEYTIT= 0
      DO i=1,50
         DO k=1,64
            titch(i)(k:k)=' '
         ENDDO
      ENDDO
!---------
      ILINE = 1
      NOUH1=IABS(IFILE)
      NOUH2=NOUH1+1
      IF( ABS(lint) .EQ. 0) THEN
! Normal mode
!WP         WRITE(NOUH1,'(A,A)') BS,'documentstyle[12pt,html]{article}'
         WRITE(NOUH1,'(A,A)') BS,'documentstyle[12pt]{article}'
         WRITE(NOUH1,'(A,A)') BS,'textwidth  = 16cm'
         WRITE(NOUH1,'(A,A)') BS,'textheight = 18cm'
         WRITE(NOUH1,'(A,A)') BS,'begin{document}'
         WRITE(NOUH1,'(A)') '  '
      ELSEIF( ABS(lint) .EQ. 1) THEN
! For TeX file is used in \input 
         WRITE(NOUH1,'(A)') '  '
      ELSEIF( ABS(lint) .EQ. 2) THEN
! For one-page plot being input for postrscript
         WRITE(NOUH1,'(A,A)') BS,'documentclass[12pt]{article}'
         WRITE(NOUH1,'(A,A)') BS,'usepackage{amstex}'
         WRITE(NOUH1,'(A,A)') BS,'usepackage{amssymb}'
!WP         WRITE(NOUH1,'(A,A)') BS,'usepackage{html}'
         WRITE(NOUH1,'(A,A)') BS,'usepackage{epsfig}'
         WRITE(NOUH1,'(A,A)') BS,'usepackage{epic}'
         WRITE(NOUH1,'(A,A)') BS,'usepackage{eepic}'
!!!         WRITE(NOUH1,'(A,A)') BS,'hoffset    = -1in'
!!!         WRITE(NOUH1,'(A,A)') BS,'voffset    = -1in'
!!!         WRITE(NOUH1,'(A,A)') BS,'textwidth  = 16cm'
!!!         WRITE(NOUH1,'(A,A)') BS,'textheight = 16cm'
!!!         WRITE(NOUH1,'(A,A)') BS,'oddsidemargin = 0cm'
!!!         WRITE(NOUH1,'(A,A)') BS,'topmargin     = 0cm'
!!!         WRITE(NOUH1,'(A,A)') BS,'headheight    = 0cm'
!!!         WRITE(NOUH1,'(A,A)') BS,'headsep       = 0cm'
         WRITE(NOUH1,'(A,A)') BS,'begin{document}'
         WRITE(NOUH1,'(A,A)') BS,'pagestyle{empty}'
         WRITE(NOUH1,'(A)') '  '
      ELSE
         WRITE(6,*) ' +++++++ STOP in gplint, wrong lint=',lint
      ENDIF

      END

      SUBROUTINE gplend
!     *****************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
!---------------------------------------------------
! This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS

! Note that TeX file is used in \input then you may not want
! to have header and \end{document}
      IF( ABS(lint) .NE. 1) THEN
         WRITE(NOUH1,'(2A)') BS,'end{document}'
      ENDIF

      CLOSE(NOUH1)
      END

      SUBROUTINE GPLOT(ID,CH1,CH2,KDUM)
!     *********************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
      DIMENSION YY(200),YER(200)
      CHARACTER CH1,CH2,CHR
      CHARACTER*80 TITLE
      LOGICAL GEXIST
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      DATA CHR /' '/
! RETURN if histo non-existing
      IF(.NOT.GEXIST(ID)) GOTO 900
! ...unpack histogram
      CALL GUNPAK(ID,YY ,'    ',IDUM)
      CALL GUNPAK(ID,YER,'ERRO',IDUM)
      CALL GINBO1(ID,TITLE,NCHX,DXL,DXU)
      XL = DXL
      XU = DXU
      CALL GRANG1(ID,YL,YU)
      KAX=1200
      KAY=1200
      IF(CH1 .EQ. 'S') THEN
! ...superimpose plot
        BACKSPACE(NOUH1)
        BACKSPACE(NOUH1)
      ELSE
! ...new frame only
        CHR=CH1
        CALL LFRAM1(ID,KAX,KAY)
      ENDIF
      WRITE(NOUH1,'(A)')    '%========== next plot (line) =========='
      WRITE(NOUH1,'(A,I10)') '%==== HISTOGRAM ID=',ID
      WRITE(NOUH1,'(A,A70 )') '% ',TITLE
!...cont. line for functions
      CALL goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker = ioperb-1
      IF (iopsla .EQ. 2)  CHR='C'
!...suppress GPLOT assignments
      IF (CH2 .EQ. 'B')   CHR=' '
      IF (CH2 .EQ. '*')   CHR='*'
      IF (CH2 .EQ. 'C')   CHR='C'
!...various types of lines
      IF     (CHR .EQ. ' ') THEN
!...contour line used for histogram
          CALL PLHIST(KAX,KAY,NCHX,YL,YU,YY,KER,YER)
      ELSE IF(CHR .EQ. '*') THEN
!...marks in the midle of the bin
          CALL PLHIS2(KAX,KAY,NCHX,YL,YU,YY,KER,YER)
      ELSE IF(CHR .EQ. 'C') THEN
!...slanted (dotted) line in plotting non-MC functions
          CALL PLCIRC(KAX,KAY,NCHX,YL,YU,YY)
      ENDIF
!------------------------------!
! Ending
!------------------------------!
      WRITE(NOUH1,'(2A)') BS,'end{picture} % close entire picture '
      WRITE(NOUH1,'(2A)') BS,'end{figure}'

      RETURN
  900 WRITE(*,*) ' ++++ GPLOT: NONEXISTIG HISTO ' ,ID
      END

      SUBROUTINE LFRAM1(ID,KAX,KAY)
!     *****************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
!----------------------------------
! Titles ans captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 TITCH
      CHARACTER*80 title
!----------------
      DIMENSION TIPSY(20),TIPSX(20)
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      DOUBLE PRECISION DXL,DXU
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      DATA ICONT/0/

      ICONT=ICONT+1
      CALL GINBO1(ID,TITLE,NCHX,DXL,DXU)
      XL = DXL
      XU = DXU
      CALL GRANG1(ID,YL,YU)

      IF(ICONT .GT. 1) WRITE(NOUH1,'(2A)') BS,'newpage'
!------------------------------!
!           Header
!------------------------------!
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') '%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     $%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(NOUH1,'(A)') '%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     $%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(NOUH1,'(2A)') BS,'begin{figure}[!ht]'
      WRITE(NOUH1,'(2A)') BS,'centering'
!------------------------------!
! General Caption
!------------------------------!
      WRITE(NOUH1,'(4A)') BS,'caption{',BS,'small'
      IF(KEYTIT.EQ.0) THEN
        WRITE(NOUH1,'(A)')     TITLE
      ELSE
        WRITE(NOUH1,'(A)')     TITCH(1)
      ENDIF
      WRITE(NOUH1,'(A)') '}'
!------------------------------!
! Frames and labels
!------------------------------!
      WRITE(NOUH1,'(A)') '% =========== big frame, title etc. ======='
      WRITE(NOUH1,'(4A)') BS,'setlength{',BS,'unitlength}{0.1mm}'
      WRITE(NOUH1,'(2A)') BS,'begin{picture}(1600,1500)'
      WRITE(NOUH1,'(4A)') BS,'put(0,0){',BS,'framebox(1600,1500){ }}'
      WRITE(NOUH1,'(A)') '% =========== small frame, labeled axis ==='
      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $    BS,'put(300,250){',BS,'begin{picture}( ',KAX,',',KAY,')'
      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $    BS,'put(0,0){',BS,'framebox( ',KAX,',',KAY,'){ }}'
      WRITE(NOUH1,'(A)') '% =========== x and y axis ================'
      CALL SAXIX(KAX,XL,XU,NTIPX,TIPSX)
      CALL SAXIY(KAY,YL,YU,NTIPY,TIPSY)
      WRITE(NOUH1,'(3A)') BS,'end{picture}}'
     $                ,'% end of plotting labeled axis'
      END

      SUBROUTINE SAXIX(KAY,YL,YU,NLT,TIPSY)
!     ***************************************
! plotting x-axis with long and short tips
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION TIPSY(20)
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/

      DY= ABS(YU-YL)
      LY = NINT( LOG10(DY) -0.4999999d0 )
      JY = NINT(DY/10d0**LY)
      DDYL = DY*10d0**(-LY)
      IF( JY .EQ. 1)             DDYL = 10d0**LY*0.25d0
      IF( JY .GE. 2.AND.JY .LE. 3) DDYL = 10d0**LY*0.5d0
      IF( JY .GE. 4.AND.JY .LE. 6) DDYL = 10d0**LY*1.0d0
      IF( JY .GE. 7)             DDYL = 10d0**LY*2.0d0
      WRITE(NOUH1,'(A)') '% .......SAXIX........ '
      WRITE(NOUH1,'(A,I4)') '%  JY= ',JY
!-------
      NLT = INT(DY/DDYL)
      NLT = MAX0(MIN0(NLT,20),1)+1
      YY0L = NINT(YL/DDYL+0.5d0)*DDYL
      DDYS = DDYL/10d0
      YY0S = NINT(YL/DDYS+0.4999999d0)*DDYS
      P0L = KAY*(YY0L-YL)/(YU-YL)
      PDL = KAY*DDYL/(YU-YL)
      P0S = KAY*(YY0S-YL)/(YU-YL)
      PDS = KAY*DDYS/(YU-YL)
      NLT = INT(ABS(YU-YY0L)/DDYL+0.0000001d0)+1
      NTS = INT(ABS(YU-YY0S)/DDYS+0.0000001d0)+1
      DO 41 N=1,NLT
      TIPSY(N) =YY0L+ DDYL*(N-1)
  41  CONTINUE
      WRITE(NOUH1,1000)
     $ BS,'multiput('  ,P0L,  ',0)('  ,PDL,  ',0){'  ,NLT,  '}{',
     $ BS,'line(0,1){25}}',
     $ BS,'multiput('  ,P0S,  ',0)('  ,PDS,  ',0){'  ,NTS,  '}{',
     $ BS,'line(0,1){10}}'
      WRITE(NOUH1,1001)
     $ BS,'multiput('  ,P0L,  ','  ,KAY,  ')('  ,PDL,  ',0){'  ,NLT,
     $ '}{'  ,BS,  'line(0,-1){25}}',
     $ BS,'multiput('  ,P0S,  ','  ,KAY,  ')('  ,PDS,  ',0){'  ,NTS,
     $ '}{'  ,BS,  'line(0,-1){10}}'
 1000 FORMAT(2A,F8.2,A,F8.2,A,I4,3A)
 1001 FORMAT(2A,F8.2,A,I4,A,F8.2,A,I4,3A)
! ...labeling of axis
      SCMX = DMAX1(DABS(YL),DABS(YU))
      LEX  = NINT( LOG10(SCMX) -0.50001)
      DO 45 N=1,NLT
      K = NINT(KAY*(TIPSY(N)-YL)/(YU-YL))
      IF(LEX .LT. 2.AND.LEX .GT. -1) THEN
! ...without exponent
      WRITE(NOUH1,'(2A,I4,5A,F8.3,A)')
     $ BS,'put(',K,',-25){',BS,'makebox(0,0)[t]{',BS,'large $ ',
     $ TIPSY(N), ' $}}'
      ELSE
! ...with exponent
      WRITE(NOUH1,'(2A,I4,5A,F8.3,2A,I4,A)')
     $ BS,'put('  ,K,  ',-25){',BS,'makebox(0,0)[t]{',BS,'large $ ',
     $ TIPSY(N)/(10d0**LEX),BS,'cdot 10^{',LEX,'} $}}'
      ENDIF
  45  CONTINUE
      END

      SUBROUTINE SAXIY(KAY,YL,YU,NLT,TIPSY)
!     ***************************************
! plotting y-axis with long and short tips
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION TIPSY(20)
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/

      DY= ABS(YU-YL)
      LY = NINT( LOG10(DY) -0.49999999d0 )
      JY = NINT(DY/10d0**LY)
      DDYL = DY*10d0**(-LY)
      IF( JY .EQ. 1)             DDYL = 10d0**LY*0.25d0
      IF( JY .GE. 2.AND.JY .LE. 3) DDYL = 10d0**LY*0.5d0
      IF( JY .GE. 4.AND.JY .LE. 6) DDYL = 10d0**LY*1.0d0
      IF( JY .GE. 7)             DDYL = 10d0**LY*2.0d0
      WRITE(NOUH1,'(A)') '% .......SAXIY........ '
      WRITE(NOUH1,'(A,I4)') '%  JY= ',JY
!-------
      NLT = INT(DY/DDYL)
      NLT = MAX0(MIN0(NLT,20),1)+1
      YY0L = NINT(YL/DDYL+0.4999999d0)*DDYL
      DDYS = DDYL/10d0
      YY0S = NINT(YL/DDYS+0.5d0)*DDYS
      P0L = KAY*(YY0L-YL)/(YU-YL)
      PDL = KAY*DDYL/(YU-YL)
      P0S = KAY*(YY0S-YL)/(YU-YL)
      PDS = KAY*DDYS/(YU-YL)
      NLT= INT(ABS(YU-YY0L)/DDYL+0.0000001d0) +1
      NTS= INT(ABS(YU-YY0S)/DDYS+0.0000001d0) +1
      DO 41 N=1,NLT
      TIPSY(N) =YY0L+ DDYL*(N-1)
  41  CONTINUE
! plotting tics on vertical axis
      WRITE(NOUH1,1000)
     $ BS,'multiput(0,'  ,P0L,  ')(0,'  ,PDL  ,'){'  ,NLT,  '}{',
     $ BS,'line(1,0){25}}',
     $ BS,'multiput(0,'  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,  '}{',
     $ BS,'line(1,0){10}}'
      WRITE(NOUH1,1001)
     $ BS,'multiput('  ,KAY,  ','  ,P0L,  ')(0,'  ,PDL,  '){'  ,NLT,
     $ '}{',BS,'line(-1,0){25}}',
     $ BS,'multiput('  ,KAY,  ','  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,
     $ '}{',BS,'line(-1,0){10}}'
 1000 FORMAT(2A,F8.2,A,F8.2,A,I4,3A)
 1001 FORMAT(2A,I4,A,F8.2,A,F8.2,A,I4,3A)
! ...Zero line if necessary
      Z0L = KAY*(-YL)/(YU-YL)
      IF(Z0L .GT. 0D0.AND.Z0L .LT. FLOAT(KAY))
     $      WRITE(NOUH1,'(2A,F8.2,3A,I4,A)')
     $       BS,'put(0,'  ,Z0L,  '){',BS,'line(1,0){'  ,KAY,  '}}'
! ...labeling of axis
      SCMX = DMAX1(DABS(YL),DABS(YU))
      LEX  = NINT( LOG10(SCMX) -0.50001d0)
      DO 45 N=1,NLT
      K = NINT(KAY*(TIPSY(N)-YL)/(YU-YL))
      IF(LEX .LT. 2.AND.LEX .GT. -1) THEN
! ...without exponent
      WRITE(NOUH1,'(2A,I4,5A,F8.3,A)')
     $  BS,'put(-25,'  ,K,  '){',BS,'makebox(0,0)[r]{',
     $  BS,'large $ '  ,TIPSY(N),  ' $}}'
      ELSE
! ...with exponent
      WRITE(NOUH1,'(2A,I4,5A,F8.3,2A,I4,A)')
     $ BS,'put(-25,'  ,K,  '){',BS,'makebox(0,0)[r]{',
     $ BS,'large $ '
     $ ,TIPSY(N)/(10d0**LEX),  BS,'cdot 10^{'  ,LEX,  '} $}}'
      ENDIF
  45  CONTINUE
      END
      SUBROUTINE PLHIST(KAX,KAY,NCHX,YL,YU,YY,KER,YER)
!     ************************************************
! plotting contour line for histogram
!     ***********************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION YY(*),YER(*)
      CHARACTER*80 FMT1
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/
      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $  BS,'put(300,250){',BS,'begin{picture}( ',KAX,',',KAY,')'
      WRITE(NOUH1,'(A)') '% ========== plotting primitives =========='
!...various types of line
      IF(ILINE .EQ. 1) THEN
         WRITE(NOUH1,'(2A)') BS,'thicklines '
      ELSE
         WRITE(NOUH1,'(2A)') BS,'thinlines '
      ENDIF
!...short macros for vertical/horizontal straight lines
      WRITE(NOUH1,'(8A)')
     $ BS,'newcommand{',BS,'x}[3]{',BS,'put(#1,#2){',
     $ BS,'line(1,0){#3}}}'
      WRITE(NOUH1,'(8A)')
     $ BS,'newcommand{',BS,'y}[3]{',BS,'put(#1,#2){',
     $ BS,'line(0,1){#3}}}'
      WRITE(NOUH1,'(8A)')
     $ BS,'newcommand{',BS,'z}[3]{',BS,'put(#1,#2){',
     $ BS,'line(0,-1){#3}}}'
!   error bars
      WRITE(NOUH1,'(8A)')
     $   BS,'newcommand{',BS,'e}[3]{',
     $   BS,'put(#1,#2){',BS,'line(0,1){#3}}}'
      IX0=0
      IY0=0
      DO 100 IB=1,NCHX
      IX1 = NINT(KAX*(IB-0.00001)/NCHX)
      IY1 = NINT(KAY*(YY(IB)-YL)/(YU-YL))
      IDY = IY1-IY0
      IDX = IX1-IX0
      FMT1 = '(2(2A,I4,A,I4,A,I4,A))'
      IF( IDY .GE. 0) THEN  
         IF(IY1 .GE. 0.AND.IY1 .LE. KAY)
     $   WRITE(NOUH1,FMT1) BS,'y{',IX0,'}{',IY0,'}{',IDY,'}',
     $                     BS,'x{',IX0,'}{',IY1,'}{',IDX,'}'
      ELSE
         IF(IY1 .GE. 0.AND.IY1 .LE. KAY)
     $   WRITE(NOUH1,FMT1) BS,'z{',IX0,'}{',IY0,'}{',-IDY,'}',
     $                     BS,'x{',IX0,'}{',IY1,'}{',IDX,'}'
      ENDIF
      IX0=IX1
      IY0=IY1
      IF(KER .EQ. 1) THEN
        IX2  = NINT(KAX*(IB-0.5000d0)/NCHX)
        IERR = NINT(KAY*((YY(IB)-YER(IB))-YL)/(YU-YL))
        IE = NINT(KAY*YER(IB)/(YU-YL))
        IF(IY1 .GE. 0.AND.IY1 .LE. KAY.and.abs(ierr) .LE. 9999
     $     .and.2*ie .LE. 9999) WRITE(NOUH1,8000) BS,IX2,IERR,IE*2
      ENDIF
 100  CONTINUE
8000  FORMAT(4(A1,2He{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(NOUH1,'(3A)') BS,'end{picture}}',
     $       ' % end of plotting histogram'
! change line-style
      ILINE= ILINE+1
      IF(ILINE .GT. 2) ILINE=1
      END
      SUBROUTINE PLHIS2(KAX,KAY,NCHX,YL,YU,YY,KER,YER)
!     ************************************************
! marks in the midle of the bin
!     **********************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION YY(*),YER(*)
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/

      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $ BS,'put(300,250){',BS,'begin{picture}( ',KAX,',',KAY,')'
      WRITE(NOUH1,'(A)') '% ========== plotting primitives =========='
!...various types of mark
      IRAD1= 6
      IRAD2=10
      IF(ILINE .EQ. 1) THEN
!   small filled circle
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle*{',IRAD1,'}}}'
      ELSEIF(ILINE .EQ. 2) THEN
!   small open circle
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle{',IRAD1,'}}}'
      ELSEIF(ILINE .EQ. 3) THEN
!   big filled circle
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle*{',IRAD2,'}}}'
      ELSEIF(ILINE .EQ. 4) THEN
!   big open circle
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle{',IRAD2,'}}}'
! Other symbols
      ELSEIF(ILINE .EQ. 5) THEN
       WRITE(NOUH1,'(10A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'makebox(0,0){$',BS,'diamond$}}}'
      ELSE
       WRITE(NOUH1,'(10A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'makebox(0,0){$',BS,'star$}}}'
      ENDIF
!   error bars
      WRITE(NOUH1,'(8A)')
     $   BS,'newcommand{',BS,'E}[3]{',
     $   BS,'put(#1,#2){',BS,'line(0,1){#3}}}'
      DO 100 IB=1,NCHX
      IX1 = NINT(KAX*(IB-0.5000d0)/NCHX)
      IY1 = NINT(KAY*(YY(IB)-YL)/(YU-YL))
      IF(IY1 .GE. 0.AND.IY1 .LE. KAY) WRITE(NOUH1,7000) BS,IX1,IY1
      IF(KER .EQ. 1) THEN
        IERR = NINT(KAY*((YY(IB)-YER(IB))-YL)/(YU-YL))
        IE   = NINT(KAY*YER(IB)/(YU-YL))
        IF(IY1 .GE. 0.AND.IY1 .LE. KAY.and.abs(ierr) .LE. 9999
     $       .and.2*ie .LE. 9999) WRITE(NOUH1,8000) BS,IX1,IERR,IE*2
      ENDIF
 100  CONTINUE
7000  FORMAT(4(A1,2HR{,I4,2H}{,I4,1H}:1X ))
8000  FORMAT(4(A1,2HE{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(NOUH1,'(3A)') BS,'end{picture}}',
     $    ' % end of plotting histogram'
! change line-style
      ILINE= ILINE+1
      IF(ILINE .GT. 6) ILINE=1
      END
      SUBROUTINE PLCIRC(KAX,KAY,NCHX,YL,YU,YY)
!     ****************************************
! plots equidistant points, four-point interpolation,
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION YY(*),IX(3000),IY(3000)
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/
      SAVE DS

! ...various types of line
! ...distance between points is DS, radius of a point is IRAD
      IRAD2=6
      IRAD1=3
! .............
      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $  BS,'put(300,250){',BS,'begin{picture}( ',KAX,',',KAY,')'
      WRITE(NOUH1,'(A)') '% ========== plotting primitives =========='
      IF(ILINE .EQ. 1) THEN
!   small filled circle
       DS = 10
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle*{',IRAD1,'}}}'
      ELSEIF(ILINE .EQ. 2) THEN
!   small open circle
       DS = 10
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle{',IRAD1,'}}}'
      ELSEIF(ILINE .EQ. 3) THEN
!   big filled circle
       DS = 20
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle*{',IRAD2,'}}}'
      ELSEIF(ILINE .EQ. 4) THEN
!   big open circle
       DS = 20
       WRITE(NOUH1,'(8A,I3,A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'circle{',IRAD2,'}}}'
! Other symbols
      ELSEIF(ILINE .EQ. 5) THEN
       DS = 20
       WRITE(NOUH1,'(10A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'makebox(0,0){$',BS,'diamond$}}}'
      ELSE
       DS = 20
       WRITE(NOUH1,'(10A)')
     $   BS,'newcommand{',BS,'R}[2]{',
     $   BS,'put(#1,#2){',BS,'makebox(0,0){$',BS,'star$}}}'
      ENDIF
      FACY = KAY/(YU-YL)
! plot first point
      AI  = 0.
      AJ  = (APROF( (AI/KAX)*NCHX+0.5d0, NCHX, YY) -YL)*FACY
      IPNT =1
      IX(IPNT) = INT(AI)
      IY(IPNT) = INT(AJ)
      DX =  DS
      AI0 = AI
      AJ0 = AJ
! plot next points
      DO 100 IPOIN=2,3000
! iteration to get (approximately) equal distance among ploted points
      DO  50 ITER=1,3
      AI  = AI0+DX
      AJ  = (APROF( (AI/KAX)*NCHX+0.5d0, NCHX, YY) -YL)*FACY
      DX  = DX *DS/SQRT(DX**2 + (AJ-AJ0)**2)
  50  CONTINUE
      IF(INT(AJ) .GE. 0.AND.INT(AJ) .LE. KAY.AND.INT(AI) .LE. KAX) THEN
         IPNT = IPNT+1
         IX(IPNT) = INT(AI)
         IY(IPNT) = INT(AJ)
      ENDIF
      AI0 = AI
      AJ0 = AJ
      IF(INT(AI) .GT. KAX) GOTO 101
 100  CONTINUE
 101  CONTINUE
      WRITE(NOUH1,7000) (BS,IX(I),IY(I), I=1,IPNT)
7000  FORMAT(4(A1,2HR{,I4,2H}{,I4,1H}:1X ))
      WRITE(NOUH1,'(2A)') BS,'end{picture}} % end of plotting line'
! change line-style
      ILINE= ILINE+1
      IF(ILINE .GT. 2) ILINE=1
      END
      FUNCTION APROF(PX,NCH,YY)
!     *************************
! PX is a continuous extension of the index in array YY
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION YY(*)
      X=PX
      IF(X .LT. 0.0.OR.X .GT. FLOAT(NCH+1)) THEN
        APROF= -1E-20
        RETURN
      ENDIF
      IP=INT(X)
      IF(IP .LT. 2)     IP=2
      IF(IP .GT. NCH-2) IP=NCH-2
      P=X-IP
      APROF = -(1./6.)*P*(P-1)*(P-2)  *YY(IP-1)
     $        +(1./2.)*(P*P-1)*(P-2)  *YY(IP  )
     $        -(1./2.)*P*(P+1)*(P-2)  *YY(IP+1)
     $        +(1./6.)*P*(P*P-1)      *YY(IP+2)
      END
      SUBROUTINE GPLSET(CH,XX)
*     ************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      SAVE / LPLDAT /
      CHARACTER*4 CH
      KTY=NINT(XX)
      IF(CH .EQ. 'DMOD') THEN
        ILINE=KTY
      ENDIF
      END

      SUBROUTINE gpltit(title)
*     ************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
      CHARACTER*64 title
!----------------------------------
! Titles ans captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 titch
!----------------
      keytit=1
      CALL copch(title,titch(1))
      END

      SUBROUTINE gplcapt(lines)
!     ************************
! This routine defines caption and should be called
! before CALL gplot2, gpltab or bpltab2
! The matrix CHARACTER*64 lines containes text of the caption ended
! with the last line '% end-of-caption'
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
      CHARACTER*64 lines(*)
!----------------------------------
! Titles ans captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 titch
!----------------
      keytit=0
      DO i=1,50
         titch(i)=lines(i)
         keytit= keytit+1
         IF(lines(i) .EQ. '% end-of-caption' ) GOTO 100
      ENDDO
 100  CONTINUE
      END

      SUBROUTINE gplabel(lines)
*     ************************
! This should be envoked after "CALL gplot2" 
! to add lines of TeX to a given plot
*     ***********************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
      CHARACTER*64 lines(*)
!---------------------------------------------------
! This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
!----------------------------------
! Titles ans captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 titch
!----------------
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 BS
!
      keytit=0
      DO i=1,50
         titch(i)=lines(i)
         keytit= keytit+1
         IF(lines(i) .EQ. '% end-of-label' ) GOTO 100
      ENDDO
 100  CONTINUE
!------------------------------!
!   erase Ending               !
!------------------------------!
      BACKSPACE(NOUH1)
      BACKSPACE(NOUH1)
!
      DO i=1,keytit
        WRITE(NOUH1,'(A)')     TITCH(i)
      ENDDO
!------------------------------!
!   restore Ending             !
!------------------------------!
      WRITE(NOUH1,'(2A)') BS,'end{picture} % close entire picture '
      IF(ABS(lint) .EQ. 2) THEN
         WRITE(NOUH1,'(A)') '%====== end of gplabel =========='
      ELSE
         WRITE(NOUH1,'(2A)') BS,'end{figure}'
      ENDIF
      END

 
      SUBROUTINE gplot2(id,ch1,ch2,chmark,chxfmt,chyfmt)
!     **************************************************
! New version, more user-friendly of gplot
! INPUT:
!    ID          histogram identifier
!    ch1 = ' '   normal new plot
!        = 'S'   impose new plot on previous one
!    ch2 = ' '   ploting line default, contour
!        = '*'   error bars in midle of the bin
!        = 'R'   error bars at Right edge of the bin
!        = 'L'   error bars at Left  edge of the bin
!        = 'C'   slanted continuous smooth line
!    chmark =    TeX symbol for ploting points
!    chxfmt =    format (string) for labeling x-axis
!    chyfmt =    format (string) for labeling y-axis
! Furthermore:
! Captions are defined by means of 
!    CALL gplcapt(capt) before CALL gplot2
!    where CHARACTER*64 capt(50) is content of 
!    caption, line by line, see also comments in gplcapt routine.
! Additional text as a TeX source text can be appended by means of
!    CALL gplabel(lines) after CALL gplot2
!    where CHARACTER*64 lines(50) is the TeX add-on.
!    this is used to decorate plot with
!    any kind marks, special labels and text on the plot.
!
!     ************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      CHARACTER ch1,ch2,chmark*(*)
!WP:      CHARACTER*8 chxfmt,chyfmt
      CHARACTER*4 chxfmt,chyfmt
      SAVE
      DIMENSION yy(200),yer(200)
      CHARACTER*80 title
!---------------------------------------------------
! This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
      LOGICAL gexist
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 BS
      CHARACTER chr
! TeX Names of the error-bar command and of the point-mark command
      CHARACTER*1 chre, chrp1
      PARAMETER ( chre = 'E', chrp1= 'R' )
      CHARACTER*2 chrp
! TeX Name of the point-mark command
      CHARACTER*1 chrx(12)
      DATA  chrx /'a','b','c','d','f','g','h','i','j','k','l','m'/
      DATA CHR /' '/
!!
! RETURN if histo non-existing
      IF(.NOT.gexist(id)) GOTO 900
! ...unpack histogram
      CALL gunpak(id,yy ,'    ',idum)
      CALL gunpak(id,yer,'ERRO',idum)
      CALL ginbo1(id,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      CALL grang1(id,yl,yu)
      KAX=1200
      KAY=1200
      IF(CH1 .EQ. 'S') THEN
! ...superimpose plot
        incr=incr+1
        BACKSPACE(NOUH1)
        BACKSPACE(NOUH1)
      ELSE
! ...new frame only
        incr=1
        CHR=CH1
        CALL lframe(id,kax,kay,chxfmt,chyfmt)
      ENDIF
      chrp= chrp1//chrx(incr)
      WRITE(NOUH1,'(A)')    '%====gplot2:  next plot (line) =========='
      WRITE(NOUH1,'(A,I10)')'%====HISTOGRAM ID=',ID
      WRITE(NOUH1,'(A,A70 )') '% ',TITLE
      CALL goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker = ioperb-1
! Default line type
      IF (iopsla .EQ. 2) THEN 
         CHR='C'
      ELSE
         CHR=' '
      ENDIF
! User defined line-type
      IF (CH2 .EQ. 'B')   CHR=' '
!...marks in the midle of the bin
      IF (CH2 .EQ. '*')   CHR='*'
!...marks on the right edge of the bin
      IF (CH2 .EQ. 'R')   CHR='R'
!...marks on the left edge of the bin
      IF (CH2 .EQ. 'L')   CHR='L'
      IF (CH2 .EQ. 'C')   CHR='C'
!...various types of lines
      IF     (CHR .EQ. ' ') THEN
!...contour line used for histogram
          CALL plkont(kax,kay,nchx,yl,yu,yy,ker,yer)
      ELSE IF(CHR .EQ. '*' .OR. CHR .EQ. 'R'.OR. CHR .EQ. 'L') THEN
!...marks on the right/left/midle of the bin
         CALL plmark(kax,kay,nchx,yl,yu,yy,ker,yer,chmark,chr,chrp,chre)
      ELSE IF(CHR .EQ. 'C') THEN
!...slanted (dotted) line in plotting non-MC functions
          CALL plcirc(kax,kay,nchx,yl,yu,yy)
      ENDIF
!------------------------------!
!        ENDing                !
!------------------------------!
      WRITE(NOUH1,'(2A)') BS,'end{picture} % close entire picture '
      IF(ABS(lint) .EQ. 2) THEN
         WRITE(NOUH1,'(A)') '%======= gplot2:  end of plot  =========='
      ELSE
         WRITE(NOUH1,'(2A)') BS,'end{figure}'
      ENDIF
      RETURN
  900 WRITE(*,*) ' ++++ GPLOT: NONEXISTIG HISTO ' ,ID
      END

      SUBROUTINE lframe(id,kax,kay,chxfmt,chyfmt)
!     *******************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      CHARACTER chxfmt*(*),chyfmt*(*)
      SAVE
!---------------------------------------------------
! This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
!----------------------------------
! Titles ans captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 TITCH
      CHARACTER*80 title
!----------------
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      DOUBLE PRECISION DXL,DXU
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      DATA ICONT/0/

      ICONT=ICONT+1
      CALL ginbo1(id,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      CALL grang1(id,yl,yu)

      IF(ICONT .GT. 1) WRITE(NOUH1,'(2A)') BS,'newpage'
!------------------------------!
!           Header
!------------------------------!
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') 
     $'%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(NOUH1,'(A)') 
     $'%%%%%%%%%%%%%%%%%%%%%%%%%%lframe%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      IF(ABS(lint) .EQ. 2) THEN
         WRITE(NOUH1,'(2A)') BS,'noindent'
      ELSE
         WRITE(NOUH1,'(2A)') BS,'begin{figure}[!ht]'
         WRITE(NOUH1,'(2A)') BS,'centering'
!WP         WRITE(NOUH1,'(2A)') BS,'htmlimage{scale=1.4}'
      ENDIF
!------------------------------!
! General Caption
!------------------------------!
      IF(ABS(lint) .NE. 2) THEN
         WRITE(NOUH1,'(6A)') BS,'caption{',BS,'footnotesize',BS,'sf'
         DO i=1,keytit
            WRITE(NOUH1,'(A)')     TITCH(i)
         ENDDO
         WRITE(NOUH1,'(A)') '}'
      ENDIF
!------------------------------!
! Frames and labels
!------------------------------!
      WRITE(NOUH1,'(A)') '% =========== big frame, title etc. ======='
      WRITE(NOUH1,'(4A)') BS,'setlength{',BS,'unitlength}{0.1mm}'
      WRITE(NOUH1,'(2A)') BS,'begin{picture}(1600,1500)'
      IF( lint .LT. 0) THEN
! Big frame usefull for debuging 
         WRITE(NOUH1,'(4A)') BS,'put(0,0){',BS,'framebox(1600,1500){ }}'
      ENDIF
      WRITE(NOUH1,'(A)') '% =========== small frame, labeled axis ==='
      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $    BS,'put(300,250){',BS,'begin{picture}( ',KAX,',',KAY,')'
      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $    BS,'put(0,0){',BS,'framebox( ',KAX,',',KAY,'){ }}'
      WRITE(NOUH1,'(A)') '% =========== x and y axis ================'
      CALL axisx(kax,xl,xu,chxfmt)
      CALL axisy(kay,yl,yu,chyfmt)
      WRITE(NOUH1,'(3A)') BS,'end{picture}}'
     $                ,'% end of plotting labeled axis'
      END

      SUBROUTINE axisx(kay,yl,yu,chxfmt)
!     ***************************************
! plotting x-axis with long and short tips
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
!WP: Problems on Linux
!WP      CHARACTER chxfmt*(*)
      CHARACTER*4 chxfmt
      DIMENSION tipsy(20)
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
      SAVE /lpldat/, /bslash/
!
      CHARACTER*64 fmt1,fmt2
      PARAMETER (fmt1 = '(2A,F8.2,A,F8.2,A,I4,3A)')
      PARAMETER (fmt2 = '(2A,F8.2,A,I4,A,F8.2,A,I4,3A)')

      DY= ABS(YU-YL)
      LY = NINT( LOG10(DY) -0.4999999d0 )
      JY = NINT(DY/10d0**LY)
      DDYL = DY*10d0**(-LY)
      IF( JY .EQ. 1)               DDYL = 10d0**LY*0.25d0
      IF( JY .GE. 2.AND.JY .LE. 3) DDYL = 10d0**LY*0.5d0
      IF( JY .GE. 4.AND.JY .LE. 6) DDYL = 10d0**LY*1.0d0
      IF( JY .GE. 7)               DDYL = 10d0**LY*2.0d0
      WRITE(NOUH1,'(A)') '% -------axisX---- '
      WRITE(NOUH1,'(A,I4)') '%  JY= ',JY
!-------
      NLT = INT(DY/DDYL)
      NLT = MAX0(MIN0(NLT,20),1)+1
      YY0L = NINT(YL/DDYL+0.5d0)*DDYL
      DDYS = DDYL/10d0
      YY0S = NINT(YL/DDYS+0.4999999d0)*DDYS
      P0L = KAY*(YY0L-YL)/(YU-YL)
      PDL = KAY*DDYL/(YU-YL)
      P0S = KAY*(YY0S-YL)/(YU-YL)
      PDS = KAY*DDYS/(YU-YL)
      NLT = INT(ABS(YU-YY0L)/DDYL+0.0000001d0)+1
      NTS = INT(ABS(YU-YY0S)/DDYS+0.0000001d0)+1
      DO n=1,nlt
         tipsy(n) =yy0l+ ddyl*(n-1)
      ENDDO
      WRITE(NOUH1,fmt1)
     $ BS,'multiput('  ,P0L,  ',0)('  ,PDL,  ',0){'  ,NLT,  '}{',
     $ BS,'line(0,1){25}}',
     $ BS,'multiput('  ,P0S,  ',0)('  ,PDS,  ',0){'  ,NTS,  '}{',
     $ BS,'line(0,1){10}}'
      WRITE(NOUH1,fmt2)
     $ BS,'multiput('  ,P0L,  ','  ,KAY,  ')('  ,PDL,  ',0){'  ,NLT,
     $ '}{'  ,BS,  'line(0,-1){25}}',
     $ BS,'multiput('  ,P0S,  ','  ,KAY,  ')('  ,PDS,  ',0){'  ,NTS,
     $ '}{'  ,BS,  'line(0,-1){10}}'
! ...labeling of axis
      scmx = DMAX1(DABS(yl),DABS(YU))
      lex  = NINT( LOG10(scmx) -0.50001)
      DO n=1,nlt
         k = nint(kay*(tipsy(n)-yl)/(yu-yl))
         IF(lex .LE. 3 .AND. lex .GE. -3) THEN
! ...without exponent
           WRITE(NOUH1,'(2A,I4,5A,'//chxfmt//',A)')
     $     BS,'put(',K,',-25){',BS,'makebox(0,0)[t]{',BS,'Large $ ',
     $     TIPSY(N), ' $}}'
         ELSE
! ...with exponent
           WRITE(NOUH1,'(2A,I4,5A,'//chxfmt//',2A,I4,A)')
     $     BS,'put('  ,K,  ',-25){',BS,'makebox(0,0)[t]{',BS,'Large $ ',
     $     TIPSY(N)/(10d0**LEX),BS,'cdot 10^{',LEX,'} $}}'
         ENDIF
      ENDDO
      END

      SUBROUTINE axisy(kay,yl,yu,chyfmt)
!     ***************************************
! plotting y-axis with long and short tips
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
!WP: Problems on Linux
!WP      CHARACTER chyfmt*(*)
      CHARACTER*4 chyfmt
      DIMENSION tipsy(20)
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / BS
      CHARACTER*1 bs
      SAVE /lpldat/, /bslash/
!
      CHARACTER*64 fmt1,fmt2
      PARAMETER (fmt1 = '(2A,F8.2,A,F8.2,A,I4,3A)')
      PARAMETER (fmt2 = '(2A,I4,A,F8.2,A,F8.2,A,I4,3A)')

      DY= ABS(YU-YL)
      LY = NINT( LOG10(DY) -0.49999999d0 )
      JY = NINT(DY/10d0**LY)
      DDYL = DY*10d0**(-LY)
      IF( JY .EQ. 1)               DDYL = 10d0**LY*0.25d0
      IF( JY .GE. 2.AND.JY .LE. 3) DDYL = 10d0**LY*0.5d0
      IF( JY .GE. 4.AND.JY .LE. 6) DDYL = 10d0**LY*1.0d0
      IF( JY .GE. 7)               DDYL = 10d0**LY*2.0d0
      WRITE(NOUH1,'(A)') '% --------saxiY------- '
      WRITE(NOUH1,'(A,I4)') '%  JY= ',JY
!-------
      NLT = INT(DY/DDYL)
      NLT = MAX0(MIN0(NLT,20),1)+1
      YY0L = NINT(YL/DDYL+0.4999999d0)*DDYL
      DDYS = DDYL/10d0
      YY0S = NINT(YL/DDYS+0.5d0)*DDYS
      P0L = KAY*(YY0L-YL)/(YU-YL)
      PDL = KAY*DDYL/(YU-YL)
      P0S = KAY*(YY0S-YL)/(YU-YL)
      PDS = KAY*DDYS/(YU-YL)
      NLT= INT(ABS(YU-YY0L)/DDYL+0.0000001d0) +1
      NTS= INT(ABS(YU-YY0S)/DDYS+0.0000001d0) +1
      DO N=1,NLT
         tipsy(n) =yy0l+ ddyl*(n-1)
      ENDDO
! plotting tics on vertical axis
      WRITE(NOUH1,fmt1)
     $ BS,'multiput(0,'  ,P0L,  ')(0,'  ,PDL  ,'){'  ,NLT,  '}{',
     $ BS,'line(1,0){25}}',
     $ BS,'multiput(0,'  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,  '}{',
     $ BS,'line(1,0){10}}'
      WRITE(NOUH1,fmt2)
     $ BS,'multiput('  ,KAY,  ','  ,P0L,  ')(0,'  ,PDL,  '){'  ,NLT,
     $ '}{',BS,'line(-1,0){25}}',
     $ BS,'multiput('  ,KAY,  ','  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,
     $ '}{',BS,'line(-1,0){10}}'
! ...Zero line if necessary
      Z0L = KAY*(-YL)/(YU-YL)
      IF(Z0L .GT. 0D0.AND.Z0L .LT. FLOAT(KAY))
     $      WRITE(NOUH1,'(2A,F8.2,3A,I4,A)')
     $       BS,'put(0,'  ,Z0L,  '){',BS,'line(1,0){'  ,KAY,  '}}'
! ...labeling of axis
      SCMX = DMAX1(DABS(YL),DABS(YU))
      LEX  = NINT( LOG10(SCMX) -0.50001d0)
      DO n=1,nlt
         k = nint(kay*(tipsy(n)-yl)/(yu-yl))
         IF(lex .LE. 3 .AND. lex .GE. -3) THEN
! ...without exponent
            WRITE(NOUH1,'(2A,I4,5A,'//chyfmt//',A)')
     $           BS,'put(-25,'  ,K,  '){',BS,'makebox(0,0)[r]{',
     $           BS,'Large $ '  ,TIPSY(N),  ' $}}'
         ELSE
! ...with exponent
            WRITE(NOUH1,'(2A,I4,5A,'//chyfmt//',2A,I4,A)')
     $           BS,'put(-25,'  ,K,  '){',BS,'makebox(0,0)[r]{',
     $           BS,'Large $ '
     $           ,TIPSY(N)/(10d0**LEX),  BS,'cdot 10^{'  ,LEX,  '} $}}'
      ENDIF
      ENDDO
      END

      SUBROUTINE plkont(kax,kay,nchx,yl,yu,yy,ker,yer)
!     ************************************************
! For the moment unchanged
!     ************************************************
! plotting contour line for histogram
!     ***********************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION YY(*),YER(*)
      CHARACTER*80 FMT1
      COMMON / LPLDAT / NOUH1,NOUH2,ILINE
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/
      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $  BS,'put(300,250){',BS,'begin{picture}( ',KAX,',',KAY,')'
      WRITE(NOUH1,'(A)') '% ========== plotting primitives =========='
!...various types of line
      IF(ILINE .EQ. 1) THEN
         WRITE(NOUH1,'(2A)') BS,'thicklines '
      ELSE
         WRITE(NOUH1,'(2A)') BS,'thinlines '
      ENDIF
!...short macros for vertical/horizontal straight lines
      WRITE(NOUH1,'(8A)')
     $ BS,'newcommand{',BS,'x}[3]{',BS,'put(#1,#2){',
     $ BS,'line(1,0){#3}}}'
      WRITE(NOUH1,'(8A)')
     $ BS,'newcommand{',BS,'y}[3]{',BS,'put(#1,#2){',
     $ BS,'line(0,1){#3}}}'
      WRITE(NOUH1,'(8A)')
     $ BS,'newcommand{',BS,'z}[3]{',BS,'put(#1,#2){',
     $ BS,'line(0,-1){#3}}}'
!   error bars
      WRITE(NOUH1,'(8A)')
     $   BS,'newcommand{',BS,'e}[3]{',
     $   BS,'put(#1,#2){',BS,'line(0,1){#3}}}'
      IX0=0
      IY0=0
      DO 100 IB=1,NCHX
      IX1 = NINT(KAX*(IB-0.00001)/NCHX)
      IY1 = NINT(KAY*(YY(IB)-YL)/(YU-YL))
      IDY = IY1-IY0
      IDX = IX1-IX0
      FMT1 = '(2(2A,I4,A,I4,A,I4,A))'
      IF( IDY .GE. 0) THEN  
         IF(IY1 .GE. 0.AND.IY1 .LE. KAY)
     $   WRITE(NOUH1,FMT1) BS,'y{',IX0,'}{',IY0,'}{',IDY,'}',
     $                     BS,'x{',IX0,'}{',IY1,'}{',IDX,'}'
      ELSE
         IF(IY1 .GE. 0.AND.IY1 .LE. KAY)
     $   WRITE(NOUH1,FMT1) BS,'z{',IX0,'}{',IY0,'}{',-IDY,'}',
     $                     BS,'x{',IX0,'}{',IY1,'}{',IDX,'}'
      ENDIF
      IX0=IX1
      IY0=IY1
      IF(KER .EQ. 1) THEN
        IX2  = NINT(KAX*(IB-0.5000d0)/NCHX)
        IERR = NINT(KAY*((YY(IB)-YER(IB))-YL)/(YU-YL))
        IE = NINT(KAY*YER(IB)/(YU-YL))
        IF(IY1 .GE. 0.AND.IY1 .LE. KAY.and.abs(ierr) .LE. 9999
     $     .and.2*ie .LE. 9999) WRITE(NOUH1,8000) BS,IX2,IERR,IE*2
      ENDIF
 100  CONTINUE
8000  FORMAT(4(A1,2He{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(NOUH1,'(3A)') BS,'end{picture}}',
     $       ' % end of plotting histogram'
! change line-style
      ILINE= ILINE+1
      IF(ILINE .GT. 2) ILINE=1
      END

      SUBROUTINE 
     $     plmark(kax,kay,nchx,yl,yu,yy,ker,yer,chmark,chr,chr2,chr3)
!     ***************************************************************
! marks in the midle of the bin
!     **********************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE 
      CHARACTER*1 chr
      DIMENSION yy(*),yer(*)
      CHARACTER chmark*(*),chr2*(*),chr3*(*)
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs

      WRITE(NOUH1,'(4A,I4,A,I4,A)')
     $ BS,'put(300,250){',BS,'begin{picture}( ',KAX,',',KAY,')'
      WRITE(NOUH1,'(A)') '% ======= plmark: plotting primitives ======='

! Plotting symbol
      WRITE(NOUH1,'(10A)')
     $   BS,'newcommand{',BS,chr2  , '}[2]{',
     $   BS,'put(#1,#2){',chmark,'}}'
! Error bar symbol
      WRITE(NOUH1,'(10A)')
     $   BS,'newcommand{',BS,chr3  , '}[3]{',
     $   BS,'put(#1,#2){',BS,'line(0,1){#3}}}'

      DO ib=1,nchx
         IF(chr .EQ. '*') THEN
            IX1 = NINT(KAX*(IB-0.5000d0)/NCHX) ! Midle of bin
         ELSEIF(chr .EQ. 'R') THEN
            IX1 = NINT(KAX*(IB*1d0)/NCHX)      ! Right edge of bin
         ELSEIF(chr .EQ. 'L') THEN
            IX1 = NINT(KAX*(IB-1D0)/NCHX)      ! Left edge of bin
         ELSE
            WRITE(6,*) '+++++ plamark: wrong line type:',chr
            RETURN
         ENDIF
         IY1 = NINT(KAY*(YY(IB)-YL)/(YU-YL))
         IF(IY1 .GE. 0.AND.IY1 .LE. KAY) 
     $   WRITE(NOUH1,'(A,A,A,I4,A,I4,A)') 
     $               BS,chr2, '{' ,IX1, '}{' ,IY1, '}'
         IF(KER .EQ. 1) THEN
            IERR = NINT(KAY*((YY(IB)-YER(IB))-YL)/(YU-YL))
            IE   = NINT(KAY*YER(IB)/(YU-YL))
            IF(iy1 .GE. 0 .AND. iy1 .LE. kay 
     $         .AND. ABS(ierr) .LE. 9999 .AND. 2*ie .LE. 9999) 
     $      WRITE(NOUH1,'(A,A,A,I4,A,I5,A,I4,A)') 
     $          BS, chr3,  '{'  ,IX1, '}{'  ,IERR, '}{'  ,IE*2,   '}'
         ENDIF
      ENDDO
      WRITE(NOUH1,'(3A)') BS,'end{picture}}',
     $    ' % end of plotting histogram'
      END


      SUBROUTINE gpltab(Npl,idl,capt,fmt,nch1,incr,npag)
!     ******************************************************
! Tables in TeX, up to 9 columns
! Npl           = numbers of columns/histograms
! idl(1:Npl)    = list of histo id's
! capt(1:Npl+1) = list of captions above each column
! fmt(1:1)      = format to print x(i) in first columb, 
!                 h(i) and error he(i) in further columns
! nch1,incr     = raws are printet in the sequence
!                 (h(i),he(i),i=nch1,nbin,incr), nbin is no. of bins.
! npag          = 0 no page eject, =1 with page eject
!     ******************************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE 
!------- parameters
      DIMENSION    idl(*)
      CHARACTER*16 capt(*)
      CHARACTER*8  fmt(3)
!----------------------------------
! Titles and captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 TITCH
!----------------
      COMMON / LPLDAT / nouh1,nouh2,iline
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/
!-----------------
      LOGICAL gexist
      DIMENSION yyy(200),yer(200),bi(200,9),er(200,9)
      CHARACTER*80 title
      CHARACTER*1 Cn(9)
      DATA Cn /'1','2','3','4','5','6','7','8','9'/
!----------

! RETURN if histo non-existing or to many columns
      IF(.NOT.GEXIST(ID)) GOTO 900
      IF(Npl .GT. 9 )     GOTO 901
!
! npack histograms
      id1=idl(1)
      CALL ginbo1( id1,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      DO n=1,Npl
        CALL gunpak( idl(n),yyy ,'    ',idum)
        CALL gunpak( idl(n),yer ,'ERRO',idum)
        DO k=1,nchx
           bi(k,n)=yyy(k)
           er(k,n)=yer(k)
        ENDDO
      ENDDO
!------------------------------!
!           Header
!------------------------------!
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') '% ========================================='
      WRITE(NOUH1,'(A)') '% ============= begin table ==============='
      WRITE(NOUH1,'(2A)') BS,'begin{table}[!ht]'
      WRITE(NOUH1,'(2A)') BS,'centering'
!------------------------------!
! Central Caption
!------------------------------!
      WRITE(NOUH1,'(4A)') BS,'caption{',BS,'small'
      DO i=1,keytit
        WRITE(NOUH1,'(A)')     TITCH(i)
      ENDDO
      WRITE(NOUH1,'(A)') '}'
!------------------------------!
! Tabular header
!------------------------------!
      WRITE(NOUH1,'(20A)') BS,'begin{tabular}
     $ {|',  ('|c',j=1,Npl+1),  '||}'
!
      WRITE(NOUH1,'(4A)') BS,'hline',BS,'hline'
!------------------------------!
! Captions in columns
!------------------------------!
      WRITE(NOUH1,'(2A)') capt(1),('&',capt(j+1),j=1,Npl)
!
      WRITE(NOUH1,'(2A)') BS,BS
      WRITE(NOUH1,'(2A)') BS,'hline'
!----------------------------------------!
! Table content
! Note that by default RIGHT EDGE of bin is printed, as necessary for
! cumulative distributions, this can be changed with SLAN option
!----------------------------------------!
      CALL goptou(idl(1),ioplog,iopsla,ioperb,iopsc1,iopsc2)
      DO k=nch1,nchx,incr
        xi= dxl + (dxu-dxl)*k/(1d0*nchx)
        IF(iopsla.eq.2) xi= dxl + (dxu-dxl)*(k-0.5d0)/(1d0*nchx)
        IF(ioperb.eq.2) THEN
        WRITE(NOUH1,
     $  '(A,'//fmt(1)//'
     $     ,      '//Cn(Npl)//'(A,'//fmt(2)//',A,A,'//fmt(3)//'),  A)')
     $   '$', xi, ('$ & $', bi(k,j), BS, 'pm', er(k,j), j=1,Npl), '$'
        WRITE(NOUH1,'(2A)') BS,BS
        ELSE
        WRITE(NOUH1,
     $  '(A,'//fmt(1)//'
     $     ,      '//Cn(Npl)//'(A,'//fmt(2)//'),  A)')
     $   '$', xi, ('$ & $', bi(k,j), j=1,Npl), '$'
        WRITE(NOUH1,'(2A)') BS,BS
        ENDIF
      ENDDO
!------------------------------!
! Ending
!------------------------------!
      WRITE(NOUH1,'(4A)') BS,'hline',BS,'hline'
      WRITE(NOUH1,'(2A)') BS,'end{tabular}'
      WRITE(NOUH1,'(2A)') BS,'end{table}'
      WRITE(NOUH1,'(A)') '% ============= end   table ==============='
      WRITE(NOUH1,'(A)') '% ========================================='
      IF(npag .NE. 0) WRITE(NOUH1,'(2A)') BS,'newpage'

      RETURN
  900 WRITE(*,*) ' ++++ gpltab: NONEXISTIG HISTO ' ,ID
      RETURN
 901  WRITE(*,*) ' ++++ gpltab: TO MANY COLUMNS  ' ,Nplt
      END

      SUBROUTINE gpltab2(Npl,idl,ccapt,mcapt,fmt,chr1,chr2,chr3)
!     **********************************************************
! Tables in TeX, up to 9 columns
! Npl           = numbers of columns/histograms
! idl(1:Npl)    = list of histo id's
! ccapt(1:Npl+1)= list of column-captions above each column
! mcapt         = multicolumn header, none if mcapt=' ',
! fmt(1:1)      = format to print x(i) in first columb, 
!                 h(i) and error he(i) in further columns
! chr1          = ' ' normal default, ='S' the Same table continued
! chr2          = ' ' midle of the bin for x(i) in the first column
!               = 'R' right edge,     = 'L' left edge of the bin
! chr3          = ' ' no page eject,  ='E' with page eject at the end.
! Furthermore:
! Captions are defined by means of 
!    CALL gplcapt(capt) before CALL gpltab2
!    where CHARACTER*64 capt(50) is content of 
!    caption, line by line, see also comments in gplcapt routine.
!
!     ******************************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE 
!------- parameters
      DIMENSION    idl(*)
      CHARACTER*16 ccapt(*)
      CHARACTER*8  fmt(3)
      CHARACTER*1  chr1,chr2,chr3
      CHARACTER*64 mcapt
!---------------------------------------------------
! This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
!----------------------------------
! Titles and captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 TITCH
!----------------
      COMMON / LPLDAT / nouh1,nouh2,iline
      COMMON / BSLASH / BS
      CHARACTER*1 BS
      SAVE /LPLDAT/, /BSLASH/
!-----------------
      LOGICAL gexist
      DIMENSION yyy(200),yer(200),bi(200,9),er(200,9)
      CHARACTER*80 title
      CHARACTER*1 Cn(9)
      DATA Cn /'1','2','3','4','5','6','7','8','9'/
!----------

! RETURN if histo non-existing or to many columns
      IF(.NOT.GEXIST(ID)) GOTO 900
      IF(Npl .GT. 9 )     GOTO 901
!
! npack histograms
      id1 = idl(1)
      CALL ginbo1( id1,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      DO n=1,Npl
        CALL gunpak( idl(n),yyy ,'    ',idum)
        CALL gunpak( idl(n),yer ,'ERRO',idum)
        DO k=1,nchx
           bi(k,n)=yyy(k)
           er(k,n)=yer(k)
        ENDDO
      ENDDO

      IF(chr1 .EQ. ' ' ) THEN
!------------------------------!
!           Header
!------------------------------!
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') ' '
      WRITE(NOUH1,'(A)') '% ========================================'
      WRITE(NOUH1,'(A)') '% ============ begin table ==============='
      IF(ABS(lint) .EQ. 2 ) THEN
         WRITE(NOUH1,'(2A)') BS,'noindent'
      ELSE
        WRITE(NOUH1,'(2A)') BS,'begin{table}[!ht]'
        WRITE(NOUH1,'(2A)') BS,'centering'
      ENDIF
!------------------------------!
! Central Caption
!------------------------------!
      IF(ABS(lint) .NE. 2 ) THEN
         WRITE(NOUH1,'(6A)') BS,'caption{',BS,'footnotesize',BS,'sf'
         DO i=1,keytit
            WRITE(NOUH1,'(A)')     TITCH(i)
         ENDDO
         WRITE(NOUH1,'(A)') '}'
      ENDIF
!------------------------------!
! Tabular header
!------------------------------!
      WRITE(NOUH1,'(20A)') BS,'begin{tabular}
     $ {|',  ('|c',j=1,Npl+1),  '||}'
!
      WRITE(NOUH1,'(4A)') BS,'hline',BS,'hline'
!------------------------------!
! Captions in columns
!------------------------------!
      WRITE(NOUH1,'(2A)') ccapt(1),('&',ccapt(j+1),j=1,Npl)
!
      ELSEIF(chr1 .EQ. 'S' ) THEN
         DO i=1,6
            BACKSPACE(NOUH1)
         ENDDO
      ELSE
         WRITE(*,*) ' ++++ gpltab2: WRONG chr1 ' ,chr1
      ENDIF

      WRITE(NOUH1,'(2A)') BS,BS
      WRITE(NOUH1,'(2A)') BS,'hline'

!------------------------------!
! Optional multicolumn caption
!------------------------------!
      IF(mcapt .NE. ' ') THEN
         WRITE(NOUH1,'(3A,I2,A)') '& ',BS,'multicolumn{',Npl,'}{c||}{'
         WRITE(NOUH1,'(3A)') '     ',mcapt, ' }'
         WRITE(NOUH1,'(2A)') BS,BS
         WRITE(NOUH1,'(2A)') BS,'hline'
      ENDIF

!----------------------------------------!
! Table content
! Note that by default RIGHT EDGE of bin is printed, as necessary for
! cumulative distributions, this can be changed with SLAN option
!----------------------------------------!
      CALL goptou(idl(1),ioplog,iopsla,ioperb,iopsc1,iopsc2)
      DO k=1,nchx
         IF(chr2 .EQ. 'R') THEN
! right
            xi= dxl + (dxu-dxl)*k/(1d0*nchx)
         ELSEIF(chr2 .EQ. 'L') THEN
! left
            xi= dxl + (dxu-dxl)*(k-1d0)/(1d0*nchx)
         ELSE
! midle
            xi= dxl + (dxu-dxl)*(k-0.5d0)/(1d0*nchx)
         ENDIF
         IF(ioperb.eq.2) THEN
          WRITE(NOUH1,
     $    '(A,'//fmt(1)//'
     $     ,      '//Cn(Npl)//'(A,'//fmt(2)//',A,A,'//fmt(3)//'),  A)')
     $    '$', xi, ('$ & $', bi(k,j), BS, 'pm', er(k,j), j=1,Npl), '$'
          WRITE(NOUH1,'(2A)') BS,BS
         ELSE
          WRITE(NOUH1,
     $    '(A,'//fmt(1)//'
     $     ,      '//Cn(Npl)//'(A,'//fmt(2)//'),  A)')
     $    '$', xi, ('$ & $', bi(k,j), j=1,Npl), '$'
          WRITE(NOUH1,'(2A)') BS,BS
         ENDIF
      ENDDO
!------------------------------!
! Ending
!------------------------------!
      WRITE(NOUH1,'(4A)') BS,'hline',BS,'hline'
      WRITE(NOUH1,'(2A)') BS,'end{tabular}'
      IF(ABS(lint) .EQ. 2 ) THEN
         WRITE(NOUH1,'(A)') '% ========================================'
      ELSE
         WRITE(NOUH1,'(2A)') BS,'end{table}'
      ENDIF
      WRITE(NOUH1,'(A)') '% ============= end   table =============='
      WRITE(NOUH1,'(A)') '% ========================================'
      IF(chr3 .EQ. 'E') WRITE(NOUH1,'(2A)') BS,'newpage'

      RETURN
  900 WRITE(*,*) ' ++++ gpltab2: NONEXISTIG HISTO ' ,ID
      RETURN
 901  WRITE(*,*) ' ++++ gpltab2: TO MANY COLUMNS  ' ,Nplt
      END


      SUBROUTINE gmonit(mode,id,par1,par2,par3)
!     *****************************************
! Utility program for monitoring m.c. rejection weights.
! ---------------------------------------------------------
! It is backward compatible with WMONIT except:
!  (1) for id=-1 one  should call as follows:
!      gmonit(-1,id,0d0,1d0,1d0) or skip initialisation completely!
!  (2) maximum absolute weight is looked for,
!  (3) gprint(-id) prints weight distribution, net profit!
!  (4) no restriction id<100 any more!
! ---------------------------------------------------------
! wt is weight, wtmax is maximum weight and rn is random number.
! IF(mode .EQ. -1) then
!          initalization if entry id, 
!        - wtmax is maximum weight used for couting overweighted
!          other arguments are ignored
! ELSEIF(mode .EQ. 0) then
!          summing up weights etc. for a given event for entry id
!        - wt is current weight.
!        - wtmax is maximum weight used for couting overweighted
!          events with wt>wtmax.
!        - rn is random number used in rejection, it is used to
!          count no. of accepted (rn < wt/wtmax) and rejected
!          (wt > wt/wtmax) events,
!          if ro rejection then put rn=0d0.
! ELSEIF(mode .EQ. 1) THEN
!          in this mode wmonit repports on accumulated statistics
!          and the information is stored in COMMON /cmonit/
!        - averwt= average weight wt counting all event
!        - errela= relative error of averwt
!        - nevtot= total number of accounted events
!        - nevacc= no. of accepted events (rn < wt/wtmax)
!        - nevneg= no. of events with negative weight (wt < 0)
!        - nevzer= no. of events with zero weight (wt = 0d0)
!        - nevove= no. of overweghted events (wt > wtmax)
!          and if you do not want to use cmonit then the value
!          the value of averwt is assigned to wt,
!          the value of errela is assigned to wtmax and
!          the value of wtmax  is assigned to rn in this mode.
! ELSEIF(mode .EQ. 2) THEN
!          all information defined for entry id defined above
!          for mode=2 is just printed of unit nout
! ENDIF
! note that output repport (mode=1,2) is done dynamically just for a
! given entry id only and it may be repeated many times for one id and
! for various id's as well.
!     ************************
      IMPLICIT double precision (a-h,o-z)
      PARAMETER( idmx=400,nbuf=24,nbuf2=24)
      COMMON / cglib / b(50000)
      COMMON /gind/ nvrs,nout,lenmax,length,index(idmx,3),titlc(idmx)
      CHARACTER*80 titlc
      SAVE
!
      idg = -id
      IF(id .LE. 0) THEN
           WRITE(nout,*) ' =====> Gmonit: wrong id= ',id
           WRITE(   6,*) ' =====> Gmonit: wrong id= ',id
           STOP
      ENDIF
      IF(mode .EQ. -1) THEN
!     *******************
           nbin = nint(dabs(par3))
           IF(nbin .GT. 100) nbin =100 
           IF(nbin .EQ. 0)   nbin =1
           xl   =  par1
           xu   =  par2
           IF(xu .LE. xl) THEN
             xl = 0d0
             xu = 1d0
           ENDIF
           lact=jadres(idg)
           IF(lact .EQ. 0) THEN
              CALL gbook1(idg,' gmonit $',nbin,xl,xu)
           ELSE
              WRITE(nout,*) ' WARNING gmonit: exists, id= ',id
              WRITE(   6,*) ' WARNING gmonit: exists, id= ',id
           ENDIF
      ELSEIF(mode .EQ. 0) THEN
!     **********************
           lact=jadres(idg)
           IF(lact .EQ. 0) THEN
              WRITE(nout,*) ' *****> Gmonit: uninitialized, id= ',id
              WRITE(   6,*) ' *****> Gmonit: uninitialized, id= ',id
              CALL gbook1(idg,' gmonit $',1,0d0,1d0)
              lact=jadres(idg)
           ENDIF
           wt   =par1
           wtmax=par2
           rn   =par3
!     standard entries
           CALL gf1(idg,wt,1d0)
!     additional goodies
           ist  = index(lact,2)
           ist2 = ist+7
           ist3 = ist+11
!    maximum weight -- maximum by absolute value but keeping sign
           b(ist3+13)    = max( dabs(b(ist3+13)) ,dabs(wt))
           IF(wt .NE. 0d0) b(ist3+13)=b(ist3+13) *wt/dabs(wt)
!    nevzer,nevove,nevacc
           IF(wt .EQ. 0d0)        b(ist3+10) =b(ist3+10) +1d0
           IF(wt .GT. wtmax)      b(ist3+11) =b(ist3+11) +1d0
           IF(rn*wtmax .LE. wt)   b(ist3+12) =b(ist3+12) +1d0
      ELSEIF(mode .GE. 1 .OR. mode .LE. 10) THEN
!     *************************************
           lact=jadres(idg)
           IF(lact .EQ. 0) THEN
              WRITE(nout,*) ' +++++++++ STOP in  wmonit ++++++++++++++'
              WRITE(   6,*) ' +++++++++ STOP in  wmonit ++++++++++++++'
              WRITE(nout,*) ' lack of initialization, id=',id
              WRITE(   6,*) ' lack of initialization, id=',id
              STOP
           ENDIF
           ist    = index(lact,2)
           ist2   = ist+7
           ist3   = ist+11
           ntot   = nint(b(ist3 +7))
           swt    =      b(ist3 +8)
           sswt   =      b(ist3 +9)
           IF(ntot .LE. 0 .or. swt  .EQ.  0d0 )  THEN
              averwt=0d0
              errela=0d0
           ELSE
              averwt=swt/float(ntot)
              errela=sqrt(abs(sswt/swt**2-1d0/float(ntot)))
           ENDIF
           nevacc = b(ist3 +12)
           nevneg = b(ist3  +1)
           nevove = b(ist3 +11)
           nevzer = b(ist3 +10)
           wwmax  = b(ist3 +13)
           nevtot = ntot
!  output through parameters
           par1   = averwt
           par2   = errela
           par3   = nevtot
           IF(mode .EQ. 2) THEN
              par1   = nevacc
              par2   = nevneg
              par3   = nevove
           ELSEIF(mode .EQ. 3) THEN
              par1   = nevneg
              par2   = nevzer
              par3   = wwmax
           ENDIF
!  no printout for mode > 1
!  ************************
           IF(mode .LE. 9) RETURN
           WRITE(nout,1003) id, averwt, errela, wwmax
           WRITE(nout,1004) nevtot,nevacc,nevneg,nevove,nevzer
           IF(mode .EQ. 2) RETURN
           CALL gprint(idg)
      ELSE
!     ****
           WRITE(nout,*) ' =====wmonit: wrong mode',mode
           WRITE(   6,*) ' =====wmonit: wrong mode',mode
           STOP
      ENDIF
!     *****
 1003 format(
     $  ' =======================gmonit========================'
     $/,'   id           averwt         errela            wwmax'
     $/,    i5,           e17.7,         f15.9,           e17.7)
 1004 format(
     $  ' -----------------------------------------------------------'
     $/,'      nevtot      nevacc      nevneg      nevove      nevzer'
     $/,   5i12)
      END
