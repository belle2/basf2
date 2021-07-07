*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                       Pseudo-Class  GLK                                  //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
*
*//////////////////////////////////////////////////////////////////////////////
*// =======================================================================  //
*// ==========================  _GLK_  ====================================  //
*// ========== General Library of histogramming/ploting utilities =========  //
*// ========== It is similar but not identical to HBOOK and HPLOT =========  //
*// =======================================================================  //
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      Version:    1.30                                    //
*//              Last correction:    January 1999                            //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*//
*//    ********************************************************************
*//    *  History of the package:                                         *
*//    *  MINI-HBOOK writen by S. Jadach, Rutherford Lab. 1976            *
*//    *  Rewritten December 1989 (S.J.)   and in 1997 (S.J.)             *
*//    ********************************************************************
*//
*//  Installation remarks:
*//  (1) printing backslash character depends on F77 compilator,
*//      user may need to modify definition of BS variable in HPLCAP
*//
*//  Usage of the program:
*//  (1) In many cases names and meanings of programs and their
*//      parameters is the same as in original CERN libraries HBOOK
*//  (2) Unlike to original HBOOK and HPLOT, all floating parameters
*//      of the programs are in DOUBLE PRECISION !
*//  (3) GLK stores histograms in DOUBLE PRECISION  and always with
*//      errors. DOUBLE PRECISION  storage is essential for 10**7 events statistics!
*//  (4) Output from GLK is a picture recorded as regular a LaTeX file
*//      with frame and curves/histograms, it is easy to change fonts
*//      add captions, merge plots, etc. by normal editing. Finally,
*//      picture may be inserted in any place into LaTeX source of the
*//      article.
*//  (5) WARNING: two-dimensional histograms are not active!!!
*//  
*//////////////////////////////////////////////////////////////////////////////
*//     List of procedures,  non-user subprograms in brackets                //
*//////////////////////////////////////////////////////////////////////////////
*    SUBR/FUNC       1 PAR. 2 PAR. 3 PAR. 4 PAR. 5 PAR. 6 PAR.
*  ====================================================================
*     (GLK_Initialize) ----   ----    ----   ----   ----   ----
*      GLK_hi          INT    INT     ----   ----   ----   ----
*      GLK_hie         INT    INT     ----   ----   ----   ----
*      GLK_Fil1        INT    DBL     DBL    ----   ----   ----
*      GLK_Fil2        INT    DBL     DBL    DBL    ----   ----
*      GLK_Book1       INT    CHR*80  INT    DBL    DBL    ----
*     (GLK_OptOut)     INT    INT     INT    INT    INT     INT
* (L.F. GLK_Exist)     INT    -----  ------  ----   ----   ----
*      GLK_Idopt       INT    CHR*4   -----  ----   ----   ----
*      GLK_BookFun1    INT    CHR*80   INT   DBL    DBL  DP-FUNC
*      GLK_Idopt       INT    CHR*4   -----  ----   ----   ----
*      GLK_Book2       INT    CHR*80   INT   DBL    DBL     INT  DBL DBL
*      GLK_PrintAll    ---    ----   ----   ----   ----   ----
*      GLK_SetNout     INT    ----   ----   ----   ----   ----
*      GLK_Print       INT    ----   ----   ----   ----   ----
*      GLK_Operat      INT    CHR*1   INT    INT    DBL    DBL
*      GLK_Hinbo1      INT    CHR*8   INT    DBL    DBL    ----
*      GLK_Unpak       INT    DBL(*) CHR*(*) INT    ---    ----
*      GLK_Pak         INT    DBL(*)  ----   ----   ---    ----
*      GLK_Pake        INT    DBL(*)  ----   ----   ---    ----
*      GLK_Range1      INT    DBL     DBL    ----   ---    ----
*      GLK_Hinbo2      INT    INT     DBL    DBL    INT    DBL   DBL
*      GLK_Ymaxim      INT    DBL     ----   ----   ---    ----
*      GLK_Yminim      INT    DBL     ----   ----   ---    ----
*      GLK_Reset       INT   CHR*(*)  ----   ----   ---    ----
*      GLK_Delet       INT     ----   ----   ----   ----   ----
*     (GLK_Copch)     CHR*80 CHR*80  ----   ----   ----   ----
*     (GLK_hadres)     INT    INT   ----   ----   ----   ----
*      GLK_Hrfile      INT   CHR*(*) CHR*(*) ----   ----   ----
*      GLK_Hrout       INT    INT    CHR*8   ----   ----   ----
*      GLK_Hrin        INT    INT     INT    ----   ----   ----
*      GLK_Hrend     CHR*(*) ----    ----   ----   ----   ----
*  *******************  HPLOT entries ******************
*      GLK_PlInt       INT    ----    ----   ----   ----   ----
*      GLK_PlCap       INT    ----    ----   ----   ----   ----
*      GLK_PlEnd       ----   ----    ----   ----   ----   ----
*      GLK_Plot        INT    CHR*1   CHR*1   INT   ----   ----
*     (GLK_Plfram1)    INT      INT     INT  ----   ----   ----
*     (GLK_SAxisX)     INT      DBL     DBL   INT    DBL   ----
*     (GLK_SAxisY)     INT      DBL     DBL   INT    DBL   ----
*     (GLK_PlHist)     INT      INT     DBL   DBL    INT    INT
*     (GLK_PlHis2)     INT      INT     DBL   DBL    INT    INT
*     (GLK_PlCirc)     INT      INT     INT   DBL    DBL    DBL
*     (GLK_aprof)      DBL      INT     DBL  ----   ----   ----
*      GLK_PlSet       INT      DBL    ----  ----   ----   ----
*      GLK_PlTitle     INT    CHR*80   ----  ----   ----   ----
*  *******************  WMONIT entries ******************
*      GLK_WtMon       INT ???
*  *******************************************************************
*                         END OF TABLE
*  *******************************************************************
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
*                     from previous history of calls (see hadres)
*          ----------------------------------
*              Binning size informations
*          ----------------------------------
*  One dimensional histogram            Two dimensional histog.
*  -------------------------            ----------------------
*  (8-11) Binning information           (8-15) Binning information
*  ist2 = ist+7
*  ist2 +1    NCHX                          ist2 +5   NCHY
*  ist2 +2      XL                          ist2 +6     YL
*  ist2 +3      XU                          ist2 +7     YU
*  ist2 +4   FACTX                          ist2 +8  FACTY
*
*          ----------------------------------
*             All kind of sums and  maxwt
*          ----------------------------------
*  One dimensional histogram            Two dimensional histog.
*  -------------------------            ----------------------
*  (12-24) Under/over-flow average x    (16-24)
*  ist3 = ist+11
*  ist3 +1   Underflow                     All nine combinations
*  ist3 +2   Normal                        (U,N,O) x (U,N,O)
*  ist3 +3   Overflow                      sum wt only (no errors)
*  ist3 +4   U  sum w**2
*  ist3 +5   N  sum w**2
*  ist3 +6   O  sum w**2
*  ist3 +7   Sum 1
*  ist3 +8   Sum wt*x
*  ist3 +9   Sum wt*x*x
*  ist3 +10  nevzer    (GLK_WtMon)
*  ist3 +11  nevove    (GLK_WtMon)
*  ist3 +12  nevacc    (GLK_WtMon)
*  ist3 +13  maxwt     (GLK_WtMon)
*          ----------------------------------
*           Content of bins including errors
*          ----------------------------------
*  (25 to 24+2*nchx)                     (25 to 24 +nchx*nchy)
*     sum wt and sum wt**2            sum wt only (no errors)
*  ----------------------------------------------------------------
*//////////////////////////////////////////////////////////////////////////////

      SUBROUTINE GLK_Initialize
*     *************************
* First Initialization called from may routines
*     *************************************
      IMPLICIT NONE
*----------------------------------------------------------------------
      INCLUDE 'GLK.fi'
      SAVE
*----------------------------------------------------------------------
* Note that backslash definition is varying from one
* instalation/compiler to another, you have to figure out by yourself
* how to fill backslash code into m_BS
ccc      CHARACTER*1 BBS1
ccc      DATA BBS1 /'\\'/    ! IBM or HP with 'f77 +B '
ccc      CHARACTER*1 BBS2
ccc      DATA BBS2 /1H\ /    ! HP  f77 with standard options
*-----------------------------------------------
      INTEGER init,i,j,k
      DATA init /0/
*-----------------------------------------------
      IF(init .NE. 0) RETURN
      init=1
* default output unit
      m_out=16
      m_length=0
* color
      m_KeyCol=0
* table range
      m_KeyTbr = 0
      DO k=1,3
         m_TabRan(k)=1
      ENDDO
* labels for 1-st col of table
      m_KeyTbl = 0
      DO j=1,m_MaxNlab
         DO k=1,32
            m_TabLab(j)(k:k)=' '
         ENDDO
      ENDDO
*
      DO k=1,80
         m_Color(k:k)=' '
      ENDDO
      m_Color(1:1)='%'
*
      DO i=1,m_idmax
         DO k=1,3
            m_index(i,k)=0
         ENDDO
         DO k=1,80
            m_titlc(i)(k:k)=' '
         ENDDO
      ENDDO
      DO k=1,m_LenmB
         m_b(k)=0d0
      ENDDO
*----------------------------------------------------------------------
**    m_BS = BBS1      ! IBM or HP with 'f77 +B '
**    m_BS = BBS2      ! HP standard options
      m_BS = '\\'      ! 
*----------------------------------------------------------------------
      END
 
      SUBROUTINE GLK_Flush
*     ********************
* FLUSH memory, all histos erased!
*     *************************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER i,k
*------------------------------------------------
      CALL GLK_Initialize
      m_length=0
      DO i=1,m_idmax
         DO k=1,3
            m_index(i,k)=0
         ENDDO
         DO k=1,80
            m_titlc(i)(k:k)=' '
         ENDDO
      ENDDO
      DO k=1,m_LenmB
         m_b(k)=0d0
      ENDDO
      END

      LOGICAL FUNCTION GLK_Exist(id)
*     ******************************
* this function is true when id  exists !!!!
*     ***************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id,lact
*------------------------------------------------
      CALL GLK_hadres(id,lact)
      GLK_Exist = lact .NE. 0
***   IF(GLK_Exist)      WRITE(6,*) 'GLK_Exist: does   ID,lact=',id,lact
***   IF(.not.GLK_Exist) write(6,*) 'GLK_Exist: doesnt ID,lact=',id,lact
      END


      SUBROUTINE GLK_SetBin(id,ib,xb)
*//////////////////////////////////////////////////
*//  Setting out bin content S.J. 20 apr 2000
*//////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER  id,ib
      DOUBLE PRECISION xb
      INTEGER    ist,ist2,ist3,iflag2,ityphi,nch,idmem,lact
      SAVE idmem
      DATA idmem / -1256765/
*------------------------------------------------------
      IF(id .EQ. idmem) goto 100
      idmem=id
      CALL GLK_hadres(id,lact) ! some checks, not repeated if id the same as previously
      IF(lact .EQ. 0) THEN
        CALL GLK_Stop1(' GLK_hi: nonexisting histo id=',id)
      ENDIF
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10 ! checking if histo is of proper type
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1 .AND. ityphi.NE.3) THEN
         CALL GLK_Stop1(' GLK_hi: 1-dim histos only !!! id=',id)
      ENDIF
  100 CONTINUE
      nch  = nint(m_b(ist2+1))
      IF(ib .EQ. 0) THEN                     ! underflow
         m_b(ist3 +1) = xb
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN ! normal bin
         m_b(ist +m_buf1+ib) = xb
      ELSEIF(ib .EQ. nch+1) THEN             ! overflow
         m_b(ist3 +3) = xb
      ELSE
         CALL GLK_Stop1('+++++ GLK_SetBin: wrong binning id=',id)
      ENDIF
      END

      SUBROUTINE  GLK_SetErr(id,ib,xe)
*//////////////////////////////////////////////////////
*//   getting out error of the bin, s.j. 20 apr 2000 //
*//////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      DOUBLE PRECISION xe
      INTEGER    ist,ist2,ist3,iflag2,ityphi,nch,lact,ib,id
      SAVE       idmem
      INTEGER    idmem
      DATA idmem / -1256765/
*---------------------------------------------------------
      IF(id .EQ. idmem) goto 100
      idmem=id
      CALL GLK_hadres(id,lact)  ! some checks, not repeated if id the same as previously
      IF(lact .EQ. 0) THEN
        CALL GLK_Stop1(' GLK_hie: nonexisting histo id=',id)
      ENDIF
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10 ! checking if histo is of proper type
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) THEN
        CALL GLK_Stop1(' GLK_hie: 1-dim histos only !!! id=',id)
      ENDIF
  100 CONTINUE
      nch  = m_b(ist2+1)
      IF(ib .EQ. 0) THEN                     ! underflow
         m_b(ist3 +4) = xe**2
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN ! normal bin, error content
         m_b(ist+m_buf1+nch+ib) = xe**2
      ELSEIF(ib .EQ. nch+1) THEN             ! overflow
         m_b(ist3 +6) = xe**2
      ELSE
         CALL GLK_Stop1('+++GLK_SetErr: wrong binning id= ',id)
      ENDIF
      END


      SUBROUTINE GLK_GetBin(id,ib,xb)
*//////////////////////////////////////////////////
*//  getting out bin content S.J. 20 apr 2000
*//////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER  id,ib
      DOUBLE PRECISION xb
      INTEGER    ist,ist2,ist3,iflag2,ityphi,nch,idmem,lact
      SAVE idmem
      DATA idmem / -1256765/
*------------------------------------------------------
      IF(id .EQ. idmem) goto 100
      idmem=id
      CALL GLK_hadres(id,lact) ! some checks, not repeated if id the same as previously
      IF(lact .EQ. 0) THEN
        CALL GLK_Stop1(' GLK_hi: nonexisting histo id=',id)
      ENDIF
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10 ! checking if histo is of proper type
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1 .AND. ityphi.NE.3) THEN
         CALL GLK_Stop1(' GLK_hi: 1-dim histos only !!! id=',id)
      ENDIF
  100 CONTINUE
      nch  = nint(m_b(ist2+1))
      IF(ib .EQ. 0) THEN                     ! underflow
         xb =   m_b(ist3 +1)
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN ! normal bin
         xb =   m_b(ist +m_buf1+ib)
      ELSEIF(ib .EQ. nch+1) THEN             ! overflow
         xb =   m_b(ist3 +3)
      ELSE
         CALL GLK_Stop1('+++++ GLK_GetBin: wrong binning id=',id)
      ENDIF
      END

      SUBROUTINE  GLK_GetErr(id,ib,xe)
*//////////////////////////////////////////////////////
*//   getting out error of the bin, s.j. 20 apr 2000 //
*//////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      DOUBLE PRECISION xe
      INTEGER    ist,ist2,ist3,iflag2,ityphi,nch,lact,ib,id
      SAVE       idmem
      INTEGER    idmem
      DATA idmem / -1256765/
*---------------------------------------------------------
      IF(id .EQ. idmem) goto 100
      idmem=id
      CALL GLK_hadres(id,lact)  ! some checks, not repeated if id the same as previously
      IF(lact .EQ. 0) THEN
        CALL GLK_Stop1(' GLK_hie: nonexisting histo id=',id)
      ENDIF
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10 ! checking if histo is of proper type
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) THEN
        CALL GLK_Stop1(' GLK_hie: 1-dim histos only !!! id=',id)
      ENDIF
  100 CONTINUE
      nch  = m_b(ist2+1)
      IF(ib .EQ. 0) THEN                     ! underflow
         xe=   DSQRT( DABS(m_b(ist3 +4)))
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN ! normal bin, error content
         xe=   DSQRT( DABS(m_b(ist+m_buf1+nch+ib)) )
      ELSEIF(ib .EQ. nch+1) THEN             ! overflow
         xe=   DSQRT( DABS(m_b(ist3 +6)))
      ELSE
         CALL GLK_Stop1('+++GLK_GetErr: wrong binning id= ',id)
      ENDIF
      END


      DOUBLE PRECISION FUNCTION GLK_hi(id,ib)
*     **********************
* getting out bin content S.J. 18-Nov. 90
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER  id,ib
* locals
      INTEGER    ist,ist2,ist3,iflag2,ityphi,nch,idmem,lact
      SAVE idmem
      DATA idmem / -1256765/
*------------------------------------------------------
      IF(id .EQ. idmem) goto 100
      idmem=id
      CALL GLK_hadres(id,lact) ! some checks, not repeated if id the same as previously
      IF(lact .EQ. 0) THEN
        CALL GLK_Stop1(' GLK_hi: nonexisting histo id=',id)
      ENDIF
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10 ! checking if histo is of proper type
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1 .AND. ityphi.NE.3) THEN
         CALL GLK_Stop1(' GLK_hi: 1-dim histos only !!! id=',id)
      ENDIF
  100 CONTINUE
      nch  = nint(m_b(ist2+1))
      IF(ib .EQ. 0) THEN        ! underflow
         GLK_hi=   m_b(ist3 +1)
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN ! normal bin
         GLK_hi=   m_b(ist +m_buf1+ib)
      ELSEIF(ib .EQ. nch+1) THEN ! overflow
         GLK_hi=   m_b(ist3 +3)
      ELSE
         CALL GLK_Stop1(' GLK_hi: wrong binning id=',id)
      ENDIF
      END

      DOUBLE PRECISION FUNCTION  GLK_hie(id,ib)
*     ************************
* getting out error of the bin, s.j. 18-nov. 90
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
* locals
      INTEGER    ist,ist2,ist3,iflag2,ityphi,nch,lact,ib,id
      SAVE       idmem
      INTEGER    idmem
      DATA idmem / -1256765/
*---------------------------------------------------------
      IF(id .EQ. idmem) goto 100
      idmem=id
      CALL GLK_hadres(id,lact)  ! some checks, not repeated if id the same as previously
      IF(lact .EQ. 0) THEN
        CALL GLK_Stop1(' GLK_hie: nonexisting histo id=',id)
      ENDIF
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10 ! checking if histo is of proper type
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) THEN
        CALL GLK_Stop1(' GLK_hie: 1-dim histos only !!! id=',id)
      ENDIF
  100 CONTINUE
      nch  = m_b(ist2+1)
      IF(ib .EQ. 0) THEN        ! underflow
         GLK_hie=   dsqrt( dabs(m_b(ist3 +4)))
      ELSEIF(ib .GE. 1.and.ib .LE. nch) THEN ! normal bin, error content
         GLK_hie=   dsqrt( dabs(m_b(ist+m_buf1+nch+ib)) )
      ELSEIF(ib .EQ. nch+1) THEN ! overflow
         GLK_hie=   dsqrt( dabs(m_b(ist3 +6)))
      ELSE
         CALL GLK_Stop1('+++GLK_hie: wrong binning id= ',id)
      ENDIF
      END

      SUBROUTINE GLK_Fil1(id,xx,wtx)
*     *****************************
* recommended fast filling 1-dim. histogram s.j. 18 nov. 90
* overflow/underflow corrected by Maciek and Zbyszek
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  xx,wtx
* local
      INTEGER           ist,ist2,ist3,iflag2,ityphi,ipose1,iposx1,kposx1,kpose1,kx,nchx,lact
      DOUBLE PRECISION  x1,wt1,xl,factx,xu
*-------------------------------------------------------------------------
      CALL GLK_hadres(id,lact)
* exit for non-existig histo
      IF(lact .EQ. 0)  RETURN
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
* one-dim. histo only
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1)  CALL GLK_Stop1('+++GLK_Fil1: wrong id=  ',id)
      x1= xx
      wt1= wtx
      m_index(lact,3)=m_index(lact,3)+1
* all entries
      m_b(ist3 +7)  =m_b(ist3 +7)   +1
* for average x
      m_b(ist3 +8)  =m_b(ist3 +8)  +wt1
      m_b(ist3 +9)  =m_b(ist3 +9)  +wt1*x1
* filling coordinates
      nchx  =m_b(ist2 +1)
      xl    =m_b(ist2 +2)
      xu    =m_b(ist2 +3)
      factx =m_b(ist2 +4)
      IF(x1 .LT. xl) THEN
* underflow
         iposx1 = ist3 +1
         ipose1 = ist3 +4
         kposx1 = 0
      ELSEIF(x1 .GT. xu) THEN
* or overflow
         iposx1 = ist3 +3
         ipose1 = ist3 +6
         kposx1 = 0
      ELSE
* or any normal bin
         iposx1 = ist3 +2
         ipose1 = ist3 +5
* or given normal bin
         kx = (x1-xl)*factx+1d0
         kx = MIN( MAX(kx,1) ,nchx)
         kposx1 = ist +m_buf1+kx
         kpose1 = ist +m_buf1+nchx+kx
      ENDIF
      m_b(iposx1) = m_b(iposx1)  +wt1
      m_b(ipose1) = m_b(ipose1)  +wt1*wt1
      IF( kposx1 .NE. 0) m_b(kposx1) = m_b(kposx1)  +wt1
      IF( kposx1 .NE. 0) m_b(kpose1) = m_b(kpose1)  +wt1*wt1
      END   !GLK_Fil1

      SUBROUTINE GLK_Fil1diff(id,xx,wtx,yy,wty)
*     *****************************************
* Special filling routine to fill the difference f(x)-g(y)
* in the case when f and g are very similar x and y are close for each event.
* In this case coherent filling is done if x and y fall into the same bin.
* Note that bin width starts to be important in this method.
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
*
      INTEGER           id
      DOUBLE PRECISION  xx,wtx,yy,wty
*    
      DOUBLE PRECISION  x1,x2,wt2,wt1,factx,xl,xu
      INTEGER           ist,ist2,ist3,iflag2,ityphi,kx,ke1,ie1,kx1,kx2,ke2,ix2,ie2,nchx,lact,ix1
*-----------------------------------------------------------------
      CALL GLK_hadres(id,lact)
* exit for non-existig histo
      IF(lact .EQ. 0)  RETURN
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
* one-dim. histo only
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) THEN
        CALL GLK_Stop1('GLK_Fil1diff: 1-dim histos only !!! id=',id)
      ENDIF
      x1= xx
      x2= yy
      wt1= wtx
      wt2= wty
      m_index(lact,3)=m_index(lact,3)+1
* all entries
      m_b(ist3 +7)  =m_b(ist3 +7)   +1
* for average x or y not very well defined yet
      m_b(ist3 +8)  =m_b(ist3 +8)  +wt1*x1 - wt2*x2
      m_b(ist3 +9)  =m_b(ist3 +9)  +wt1*x1*x1 - wt2*x2*x2
* filling coordinates
      nchx  =m_b(ist2 +1)
      xl    =m_b(ist2 +2)
      xu    =m_b(ist2 +3)
      factx =m_b(ist2 +4)
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
         kx1 = ist +m_buf1+kx
         ke1 = ist +m_buf1+nchx+kx
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
         kx2 = ist +m_buf1+kx
         ke2 = ist +m_buf1+nchx+kx
      ENDIF
* coherent filling
      IF( ix1 .EQ. ix2 ) THEN
         m_b(ix1) = m_b(ix1)  +wt1-wt2
         m_b(ie1) = m_b(ie1)  +(wt1-wt2)**2
      ELSE
         m_b(ix1) = m_b(ix1)  +wt1
         m_b(ie1) = m_b(ie1)  +wt1*wt1
         m_b(ix2) = m_b(ix2)  -wt2
         m_b(ie2) = m_b(ie2)  +wt2*wt2
      ENDIF
      IF( kx1 .EQ. kx2 ) THEN
         IF( kx1 .NE. 0) THEN
            m_b(kx1) = m_b(kx1)  +wt1-wt2
            m_b(ke1) = m_b(ke1)  +(wt1-wt2)**2
         ENDIF
      ELSE
         IF( kx1 .NE. 0) THEN
            m_b(kx1) = m_b(kx1)  +wt1
            m_b(ke1) = m_b(ke1)  +wt1*wt1
         ENDIF
         IF( kx2 .NE. 0) THEN
            m_b(kx2) = m_b(kx2)  -wt2
            m_b(ke2) = m_b(ke2)  +wt2*wt2
         ENDIF
      ENDIF
      END   !GLK_Fil1diff

      SUBROUTINE GLK_Fil2(id,x,y,wtw)
*     ****************************
* this routine not finished, 1-dim only!
*     ***********************************
      IMPLICIT NONE     
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  x,y,wtw
* local
      INTEGER           ist,iflag2,ityphi,ist2,ist3,nchx,nchy,ly,ky,k2,kx,lact,lx,k,l
      DOUBLE PRECISION  xx,yy,wt,factx,xl,yl,facty
*-------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0)  RETURN
      ist  = m_index(lact,2)
* one-dim. histo
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 2) THEN
        CALL GLK_Stop1('GLK_Fil2: 2-dim histos only !!! id=',id)
      ENDIF
*...two-dim. scattergram, no errors!
      ist2 = ist+7
      ist3 = ist+15
      xx= x
      yy= y
      wt= wtw
      m_index(lact,3)=m_index(lact,3)+1
* x-axis
      nchx  =m_b(ist2 +1)
      xl    =m_b(ist2 +2)
      factx =m_b(ist2 +4)
      kx=(xx-xl)*factx+1d0
      lx=2
      IF(kx .LT. 1)     lx=1
      IF(kx .GT. nchx)  lx=3
      l     = ist+34  +lx
      m_b(l)  = m_b(l)    +wt
      k     = ist+m_buf2  +kx
      IF(lx .EQ. 2) m_b(k)  =m_b(k)  +wt
      k2    = ist+m_buf2  +nchx+kx
      IF(lx .EQ. 2) m_b(k2) =m_b(k2) +wt**2
* y-axix
      nchy  =m_b(ist2 +5)
      yl    =m_b(ist2 +6)
      facty =m_b(ist2 +8)
      ky=(yy-yl)*facty+1d0
      ly=2
      IF(ky .LT. 1)    ly=1
      IF(ky .GT. nchy) ly=3
* under/over-flow
      l = ist3  +lx +3*(ly-1)
      m_b(l) =m_b(l)+wt
* regular bin
      k = ist+m_buf2 +kx +nchx*(ky-1)
      IF(lx .EQ. 2.and.ly .EQ. 2) m_b(k)=m_b(k)+wt
      END

      SUBROUTINE GLK_Book1(id,title,nnchx,xxl,xxu)
*     ********************************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER          id
      DOUBLE PRECISION xxl,xxu
      CHARACTER*80 title
* locals
      DOUBLE PRECISION xl,xu,ddx
      INTEGER          ist,nchx,ioplog,iopsla,ioperb,iflag2,ityphi,iflag1
      INTEGER          ist3,ist2,lengt2,lact,nnchx,iopsc2,iopsc1,j
      LOGICAL GLK_Exist
*-------------------------------------------------
      CALL GLK_Initialize
      IF(GLK_Exist(id)) goto 900
      ist=m_length
      CALL GLK_hadres(0,lact)
* Check if there is a free entry in the m_index
      IF(lact .EQ. 0)
     $     CALL GLK_Stop1('GLK_Book1: to many histos !!!!!,   id= ',id)
      m_index(lact,1)=id
      m_index(lact,2)=m_length
      m_index(lact,3)=0
* -------
      CALL GLK_Copch(title,m_titlc(lact))
      nchx =nnchx
      IF(nchx .GT. m_MaxNb)
     $     CALL GLK_Stop1(' GLK_Book1: To many bins requested,id= ',id)
      xl   =xxl
      xu   =xxu
* ---------- title and bin content ----------
      lengt2 = m_length +2*nchx +m_buf1+1
      IF(lengt2 .GE. m_LenmB)
     $  CALL GLK_Stop1('GLK_Book1:too litle storage, m_LenmB= ',m_LenmB)
*
      DO j=m_length+1,lengt2+1
         m_b(j) = 0d0
      ENDDO
      m_length=lengt2
*... default flags
      ioplog   = 1
      iopsla   = 1
      ioperb   = 1
      iopsc1   = 1
      iopsc2   = 1
      iflag1   =
     $ ioplog+10*iopsla+100*ioperb+1000*iopsc1+10000*iopsc2
      ityphi   = 1
      iflag2   = ityphi
* examples of decoding flags
*      id       = nint(m_b(ist+2)-9d0-9d12)/10
*      iflag1   = nint(m_b(ist+3)-9d0-9d12)/10
*      ioplog = mod(iflag1,10)
*      iopsla = mod(iflag1,100)/10
*      ioperb = mod(iflag1,1000)/100
*      iopsc1 = mod(iflag1,10000)/1000
*      iopsc2 = mod(iflag1,100000)/10000
*      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
*      ityphi = mod(iflag2,10)
*--------- buffer -----------------
* header
      m_b(ist +1)  = 9999999999999d0
      m_b(ist +2)  = 9d12 +     id*10 +9d0
      m_b(ist +3)  = 9d12 + iflag1*10 +9d0
      m_b(ist +4)  = 9d12 + iflag2*10 +9d0
* dummy vertical scale
      m_b(ist +5)  =  -100d0
      m_b(ist +6)  =   100d0
* pointer used to speed up search of histogram address
      m_b(ist +7)  =   0d0
* information on binning
      ist2         = ist+7
      m_b(ist2 +1) = nchx
      m_b(ist2 +2) = xl
      m_b(ist2 +3) = xu
      ddx = xu-xl
      IF(ddx .EQ. 0d0) CALL GLK_Stop1('+++GLK_Book1: xl=xu,  id= ',id)
      m_b(ist2 +4) = DFLOAT(nchx)/ddx
*
* under/over-flow etc.
      ist3       = ist+11
      DO j=1,13
         m_b(ist3 +j)=0d0
      ENDDO
      RETURN
*----------------
 900  CALL GLK_Retu1(' WARNING GLK_Book1: already exists id= ', id)
      END

      SUBROUTINE GLK_Retu1(mesage,id)
*     *******************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER id
      CHARACTER*(*) mesage
*-----------------------------
      WRITE(m_out,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(m_out,'(a,a,i10,a)')
     $                          '++ ', mesage, id, ' ++'
      WRITE(m_out,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6   ,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6   ,'(a,a,i10,a)')
     $                          '++ ', mesage, id, ' ++'
      WRITE(6   ,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      END

      SUBROUTINE GLK_Stop1(mesage,id)
*     *******************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      CHARACTER*(*) mesage
      INTEGER id
*-----------------------------
      WRITE(m_out,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(m_out,'(a,a,i10,a)')
     $                          '++ ', mesage, id, ' ++'
      WRITE(m_out,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6   ,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6   ,'(a,a,i10,a)')
     $                          '++ ', mesage, id, ' ++'
      WRITE(6   ,'(a)')
     $          '++++++++++++++++++++++++++++++++++++++++++++++++++++++'
      STOP
      END


      SUBROUTINE GLK_OptOut(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
*     ********************************************************
* decoding option flags
*     **********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER    id,ioplog,iopsla,ioperb,iopsc1,iopsc2
      INTEGER    ist,iflag1,lact
*----------------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) RETURN
      ist=m_index(lact,2)
* decoding flags
      iflag1   = nint(m_b(ist+3)-9d0-9d12)/10
      ioplog = mod(iflag1,10)
      iopsla = mod(iflag1,100)/10
      ioperb = mod(iflag1,1000)/100
      iopsc1 = mod(iflag1,10000)/1000
      iopsc2 = mod(iflag1,100000)/10000
      END

      SUBROUTINE GLK_idopt(id,ch)
*     ************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER     id
      CHARACTER*4 ch
*
      INTEGER     lact,ist,ioplog,ioperb,iopsla,iopsc1,iopsc2,iflag1
*----------------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) RETURN
      ist=m_index(lact,2)
* decoding flags
      CALL GLK_OptOut(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      IF(ch .EQ.       'LOGY'  ) THEN
* log scale for print
        ioplog = 2
      ELSEIF(ch .EQ.   'ERRO'  ) THEN
* errors in printing/plotting
       ioperb  = 2
      ELSEIF(ch .EQ.   'SLAN'  ) THEN
* slanted line in plotting
       iopsla  = 2
      ELSEIF(ch .EQ.   'YMIN'  ) THEN
       iopsc1  = 2
      ELSEIF(ch .EQ.   'YMAX'  ) THEN
       iopsc2  = 2
      ENDIF
* encoding back
      iflag1   = ioplog+10*iopsla+100*ioperb+1000*iopsc1+10000*iopsc2
      m_b(ist+3) = 9d12 + iflag1*10 +9d0
      END


      SUBROUTINE GLK_BookFun1(id,title,nchx,xmin,xmax,func)
*/////////////////////////////////////////////////////////////////////////
*//   fills histogram with function func(x)                             //
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  xmin,xmax,func
      CHARACTER*80 title
*
      DOUBLE PRECISION  yy(m_MaxNb)
      EXTERNAL func
      LOGICAL GLK_Exist
      INTEGER           ib,nchx
      DOUBLE PRECISION  xl,xu,x
*---------------------------------------------------------------------
      CALL GLK_Initialize
      IF(GLK_Exist(id)) GOTO 900
 15   xl=xmin
      xu=xmax
      CALL GLK_Book1(id,title,nchx,xl,xu)
*...slanted line in plotting
      CALL GLK_idopt(id,'SLAN')
      IF(nchx .GT. 200) goto 901
      DO ib=1,nchx
         x= xmin +(xmax-xmin)/nchx*(ib-0.5d0)
         yy(ib) = func(x)
      ENDDO
      CALL GLK_Pak(id,yy)
      RETURN
 900  CALL GLK_Retu1('+++GLK_BookFun1: already exists id=',id)
      CALL GLK_Delet(id)
      GOTO 15
 901  CALL GLK_Stop1('+++GLK_BookFun1: to many bins, id=',id)
      END

      SUBROUTINE GLK_BookFun1I(id,title,nchx,xmin,xmax,func)
*/////////////////////////////////////////////////////////////////////////
*//   Fills histogram with function func(x)                             //
*//   Gauss integration over each bin is done, can be slow.             //
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  xmin,xmax,func
      CHARACTER*80 title
*
      DOUBLE PRECISION  yy(m_MaxNb)
      EXTERNAL func
      LOGICAL GLK_Exist
      INTEGER           ib,nchx
      DOUBLE PRECISION  xl,xu,x
      DOUBLE PRECISION  GLK_Gauss,a,b,Eeps,dx
*---------------------------------------------------------------------
      CALL GLK_Initialize
      IF(GLK_Exist(id)) GOTO 900
 15   xl=xmin
      xu=xmax
      CALL GLK_Book1(id,title,nchx,xl,xu)
      IF(nchx .GT. 200) goto 901
      Eeps = -0.01d0             !!! relat. precision requirement not very demanding
      dx = (xmax-xmin)/nchx
      DO ib=1,nchx
         a= xmin +dx*(ib-1)
         b= xmin +dx*ib
         yy(ib) = GLK_Gauss(func,a,b,Eeps)/dx   !! 16-point Gauss integration over bin
      ENDDO
      CALL GLK_Pak(id,yy)
      RETURN
 900  CALL GLK_Retu1('+++GLK_BookFun1I: already exists id=',id)
      CALL GLK_Delet(id)
      GOTO 15
 901  CALL GLK_Stop1('+++GLK_BookFun1I: to many bins, id=',id)
      END

      SUBROUTINE GLK_BookFun1S(id,title,nchx,xmin,xmax,func)
*/////////////////////////////////////////////////////////////////////////
*// Fills histogram with function func(x)                               //
*// three point fit used                                                //
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      DOUBLE PRECISION  xmin,xmax,func
      EXTERNAL func
      INTEGER  id,nchx
      CHARACTER*80 title
* locals
      DOUBLE PRECISION  yy(m_MaxNb),yy1(0:m_MaxNb)
      LOGICAL GLK_Exist
      DOUBLE PRECISION  xl,xu,x3,x2,dx
      INTEGER           ib  
*--------------------------------------------------------
      CALL GLK_Initialize
      IF( GLK_Exist(id) ) GOTO 900
 15   xl=xmin
      xu=xmax
      CALL GLK_Book1(id,title,nchx,xl,xu)

*...slanted line in plotting
      CALL GLK_idopt(id,'SLAN')
      IF(nchx.gt.200) GOTO 901

      yy1(0) = func(xmin)
      dx=(xmax-xmin)/nchx

      DO ib=1,nchx
         x2= xmin +dx*(ib-0.5d0)
         x3= x2 +dx*0.5d0
         yy(ib)  = func(x2)
         yy1(ib) = func(x3)
*..  simpson
         yy(ib) = ( yy1(ib-1) +4*yy (ib) +yy1(ib))/6d0
      ENDDO

      CALL GLK_Pak(id,yy)
      RETURN
 900  CALL GLK_Retu1('+++GLK_BookFun1S: already exists, id=',id)
      CALL GLK_Delet(id)
      GOTO 15
 901  CALL GLK_Stop1(' +++GLK_BookFun1S: to many bins, id=',id)
      END

      DOUBLE PRECISION FUNCTION GLK_Gauss(f,a,b,Eeps)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// This is iterative integration procedure                                  //
*// originates  probably from CERN library                                   //
*// it subdivides inegration range until required precision is reached       //
*// precision is a difference from 8 and 16 point Gauss itegr. result        //
*// Eeps POSITIVE treated as absolute precision                              //
*// Eeps NEGATIVE treated as relative precision                              //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  f,a,b,Eeps
*
      DOUBLE PRECISION  c1,c2,bb,s8,s16,y,aa,const,delta,eps,u
      INTEGER           i
*
      DOUBLE PRECISION  w(12),x(12)
      EXTERNAL f
      DATA const /1.0d-19/
      DATA w
     1/0.10122 85362 90376, 0.22238 10344 53374, 0.31370 66458 77887,
     2 0.36268 37833 78362, 0.02715 24594 11754, 0.06225 35239 38648,
     3 0.09515 85116 82493, 0.12462 89712 55534, 0.14959 59888 16577,
     4 0.16915 65193 95003, 0.18260 34150 44924, 0.18945 06104 55069/
      DATA x
     1/0.96028 98564 97536, 0.79666 64774 13627, 0.52553 24099 16329,
     2 0.18343 46424 95650, 0.98940 09349 91650, 0.94457 50230 73233,
     3 0.86563 12023 87832, 0.75540 44083 55003, 0.61787 62444 02644,
     4 0.45801 67776 57227, 0.28160 35507 79259, 0.09501 25098 37637/
*-----------------------------------------------------------------------------
      eps=abs(Eeps)
      delta=const*abs(a-b)
      GLK_Gauss=0d0
      aa=a
    5 y=b-aa
      IF(abs(y)  .LE.  delta) RETURN
    2 bb=aa+y
      c1=0.5d0*(aa+bb)
      c2=c1-aa
      s8=0d0
      s16=0d0
      DO 1 i=1,4
      u=x(i)*c2
    1 s8=s8+w(i)*(f(c1+u)+f(c1-u))
      DO 3 i=5,12
      u=x(i)*c2
    3 s16=s16+w(i)*(f(c1+u)+f(c1-u))
      s8=s8*c2
      s16=s16*c2
      IF(Eeps .LT. 0d0) THEN
        IF(abs(s16-s8)  .GT.  eps*abs(s16)) GOTO 4
      ELSE
        IF(abs(s16-s8)  .GT.  eps) GOTO 4
      ENDIF
      GLK_Gauss=GLK_Gauss+s16
      aa=bb
      GOTO 5
    4 y=0.5d0*y
      IF(abs(y)  .GT.  delta) GOTO 2
      WRITE(*,7)
      GLK_Gauss=0d0
      RETURN
    7 FORMAT(1x,36hgaus  ... too high accuracy required)
      END



      SUBROUTINE GLK_Book2(ID,TITLE,NCHX,XL,XU,NCHY,YL,YU)
*     ***************************************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           ID,NCHX,NCHY
      DOUBLE PRECISION  XL,XU,YL,YU
      CHARACTER*80 TITLE
*
      INTEGER   ist,lact,lengt2,j,nnchx,nnchy
      LOGICAL GLK_EXIST
*-------------------------------------------------------------------------
      CALL GLK_Initialize
      IF(GLK_EXIST(ID)) GOTO 900
      ist=m_length
      CALL GLK_hadres(0,lact)
      IF(LACT .EQ. 0) GOTO 901
      m_index(LACT,1)=ID
      m_index(LACT,2)=m_length
      CALL GLK_COPCH(TITLE,M_TITLC(LACT))
      nnchx=NCHX
      nnchy=NCHY
      LENGT2 = M_LENGTH  +44+nnchx*nnchy
      IF(LENGT2 .GE. m_LenmB) GOTO 902
      DO 10 J=M_LENGTH+1,LENGT2+1
   10 m_b(J) = 0D0
      M_LENGTH=LENGT2
      m_b(ist+1)=nnchx
      m_b(ist+2)=XL
      m_b(ist+3)=XU
      m_b(ist+4)=float(nnchx)/(m_b(ist+3)-m_b(ist+2))
      m_b(ist+5)=nnchy
      m_b(ist+6)=YL
      m_b(ist+7)=YU
      m_b(ist+8)=float(nnchy)/(m_b(ist+7)-m_b(ist+6))
      RETURN
  900 CALL GLK_Retu1('GLK_Book2: histo already exists!!!! id=',id)
      RETURN
  901 CALL GLK_Stop1('GLK_Book2: too many histos !!!!! lact= ',LACT)
      RETURN
  902 CALL GLK_Stop1('GLK_Book2: too litle storage, m_LenmB=',m_LenmB)
      RETURN
      END

      SUBROUTINE GLK_PrintAll
*     ***********************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER i,id

      DO i=1,m_idmax
         id=m_index(i,1)
         IF(id .GT. 0) CALL GLK_Print(id)
      ENDDO
      END

      SUBROUTINE GLK_ListPrint(mout)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER i,id
      CHARACTER*80 title
      INTEGER             nb,mout
      DOUBLE PRECISION    xmin,xmax
*----------------------------------
      WRITE(mout,*) 
     $'============================================================================================'
      WRITE(mout,*) 
     $'     ID                 TITLE                                                nb, xmin, xmax'
      DO i=1,m_idmax
         id=m_index(i,1)
         IF(id .NE. 0) THEN
            CALL GLK_hinbo1(id,title,nb,xmin,xmax)
            WRITE(mout,'(i8,a,a,i8,2g14.6)') id, '  ', title, nb,xmin,xmax
         ENDIF
      ENDDO
      END



      SUBROUTINE GLK_Print(id)
*     ***********************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER  id
*
      DOUBLE PRECISION xl,bind,xlow,z,er,avex,dx,fact,ovef,undf,bmax,bmin,deltb
      DOUBLE PRECISION sum,sumw,sumx
      INTEGER          ist,ist2,ist3,idec,k2,k1,kros,j,ind,i,n,i1,ky,nchy,kx,nent,iflag2,lmx
      INTEGER          ioplog,iopsla,ioperb,iopsc1,iopsc2,lact,ker,ityphi,kzer,k,ibn,nchx,istr
      LOGICAL llg
      CHARACTER*1 line(0:105),lchr(22),lb,lx,li,l0
      SAVE lb,lx,li,l0,lchr
      DATA lb,lx,li,l0 /' ','X','I','0'/
      DATA lchr/' ','1','2','3','4','5','6','7','8','9',
     $      'A','B','C','D','E','F','G','H','I','J','K','*'/
*---------------------------------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) goto 900
      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
      idec    = nint(m_b(ist+2)-9d0-9d12)/10
      IF(idec .NE. id) WRITE(6,*) '++++GLK_PRINT: PANIC! ID,IDEC= ',ID,IDEC
      CALL GLK_OptOut(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker    =  ioperb-1
      lmx = 67
      IF(ker .EQ. 1) lmx=54
      nent=m_index(lact,3)
      IF(nent  .EQ.  0)                          GOTO 901
      WRITE(m_out,1000) id,m_titlc(lact)
 1000 FORMAT('1',/,1X,I9,10X,A)
*
* one-dim. histo
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 2) GOTO 200
      IF( (ityphi.NE.1) .AND. (ityphi.NE.3) )
     $   CALL GLK_Stop1(' GLK_PRINT: wrong histo type, id=',id)

      nchx =   m_b(ist2 +1)
      xl   =   m_b(ist2 +2)
      dx   =  (  m_b(ist2 +3)-m_b(ist2 +2)  )/float(nchx)
* fixing vertical scale
      istr=ist+m_buf1+1
      bmin = m_b(istr)
      bmax = m_b(istr)+1d-5*abs(m_b(istr))  ! problems for single bin case
      DO  ibn=istr,istr+nchx-1
         bmax = max(bmax,m_b(ibn))
         bmin = min(bmin,m_b(ibn))
      ENDDO
      IF(bmin  .EQ.  bmax)                       GOTO 903
      IF(iopsc1 .EQ. 2) bmin=m_b(ist +5)
      IF(iopsc2 .EQ. 2) bmax=m_b(ist +6)
*
      llg=ioplog .EQ. 2
      IF(llg.and.bmin .LE. 0d0) bmin=bmax/10000.d0
*
      deltb = bmax-bmin
      IF(deltb  .EQ.  0d0)                       GOTO 902
      fact  = (lmx-1)/deltb
      kzer  = -bmin*fact+1.00001d0
      IF(llg) fact=(lmx-1)/(log(bmax)-log(bmin))
      IF(llg) kzer=-log(bmin)*fact+1.00001d0
*
      undf = m_b(ist3 +1)
      ovef = m_b(ist3 +3)
      avex = 0d0
      sumw  = m_b(ist3 +8)
      sumx  = m_b(ist3 +9)
      IF(sumw .NE. 0d0) avex = sumx/sumw
      WRITE(m_out,'(4a15      )')  'nent',' sum','bmin','bmax'
      WRITE(m_out,'(i15,3e15.5)')   nent,   sum,  bmin,  bmax
      WRITE(m_out,'(4a15  )')      'undf','ovef','sumw','avex'
      WRITE(m_out,'(4e15.5)')       undf,  ovef,  sumw,  avex
*
      IF(llg) write(m_out,1105)
 1105 format(35x,17hlogarithmic scale)
*
      kzer=max0(kzer,0)
      kzer=min0(kzer,lmx)
      xlow=xl
      do 100 k=1,nchx
* first fill with blanks
      do  45 j=1,105
   45 line(j)  =lb
* THEN fill upper and lower boundry
      line(1)  =li
      line(lmx)=li
      ind=istr+k-1
      bind=m_b(ind)
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
      z=m_b(ind)
      IF(ker .NE. 1) THEN
        WRITE(m_out,'(a, f7.4,  a, d14.6,  132a1)')
     $             ' ', xlow,' ',     z,' ',(line(i),i=1,lmx)
      ELSE
        er=dsqrt(dabs(m_b(ind+nchx)))
        WRITE(m_out,'(a,f7.4,  a,d14.6,  a,d14.6, 132a1 )')
     $             ' ',xlow,' ',    z,' ',   er,' ',(line(i),i=1,lmx)
      ENDIF
      xlow=xlow+dx
  100 continue
      RETURN
*//////////////////////////////////////////////////////////////////////
*// two dimensional requires complete restoration and tests          //
*//////////////////////////////////////////////////////////////////////
  200 continue
      nchx=m_b(ist+1)
      nchy=m_b(ist+5)
      WRITE(m_out,2000) (lx,i=1,nchy)
 2000 format(1h ,10x,2hxx,100a1)
      do 300 kx=1,nchx
      do 250 ky=1,nchy
      k=ist +m_buf2 +kx+nchx*(ky-1)
      N=m_b(K)+1.99999D0
      n=max0(n,1)
      n=min0(n,22)
      IF(DABS(m_b(k)) .LT. 1D-20) n=1
      line(ky)=lchr(n)
  250 continue
      line(nchy+1)=lx
      i1=nchy+1
      WRITE(m_out,2100) (line(i),i=1,i1)
 2100 format(1h ,10x,1hx,100a1)
  300 continue
      WRITE(m_out,2000) (lx,i=1,nchy)
      RETURN
 900  CALL GLK_Retu1('GLK_PRINT: nonexisting histo',id)
      RETURN
 901  CALL GLK_Retu1('   GLK_PRINT: nent.eq.0',ID)
      RETURN
 902  CALL GLK_Retu1('   GLK_PRINT: wrong plotting limits, id=',id)
      RETURN
 903  CALL GLK_Retu1('   GLK_PRINT: bmin.eq.bmax, id=',id)
      END

      SUBROUTINE GLK_Operat(ida,chr,idb,idc,coef1,coef2)
*     **********************************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           ida,idb,idc
      DOUBLE PRECISION  coef1,coef2
      CHARACTER*80      title
      CHARACTER*1       chr
*
      DOUBLE PRECISION  xl,xu
      INTEGER           ista,ista2,ista3,ncha,iflag2a,ityphia,lactb
      INTEGER           k,j,nchc,istc2,istc3,i1,j2,j3,j1,i2,i3,istc,istb2,istb3,nchb
      INTEGER           lacta,id,istb,nchx,iflag2b,ityphib,lactc
*----------------------------------------------------------
      CALL GLK_hadres(ida,lacta)
      IF(lacta .EQ. 0) RETURN
      ista  = m_index(lacta,2)
      ista2 = ista+7
      ista3 = ista+11
      ncha  = m_b(ista2+1)
* check for type
      iflag2a   = nint(m_b(ista+4)-9d0-9d12)/10
      ityphia   = mod(iflag2a,10)
      IF(ityphia .NE. 1) CALL GLK_Stop1('GLK_Operat: 1-dim histos only, id=',id)
*------------------
      CALL GLK_hadres(idb,lactb)
      IF(lactb .EQ. 0) RETURN
      istb  = m_index(lactb,2)
      istb2 = istb+7
      istb3 = istb+11
      nchb  = m_b(istb2+1)
      IF(nchb .NE. ncha) goto 900
* check for type
      iflag2b   = nint(m_b(istb+4)-9d0-9d12)/10
      ityphib   = mod(iflag2b,10)
      IF(ityphib .NE. 1)  CALL GLK_Stop1('GLK_Operat: 1-dim histos only, id=',id)
*------------------
      CALL GLK_hadres(idc,lactc)
      IF(lactc .EQ. 0) THEN
* ...if nonexistent, histo idc is here defined
        CALL GLK_hinbo1(ida,title,nchx,xl,xu)
        CALL GLK_Book1(idc,title,nchx,xl,xu)
        CALL GLK_hadres(idc,lactc)
        istc  = m_index(lactc,2)
*...option copied from ida
        m_b(istc+ 3)= m_b(ista +3)
      ENDIF
*...one nominal entry recorded
      m_index(lactc,3) = 1
*
      istc  =  m_index(lactc,2)
      istc2 =  istc+7
      istc3 =  istc+11
      nchc  =  m_b(istc2+1)
*
      IF(nchc .NE. ncha) goto 900
      IF(ncha .NE. nchb .OR. nchb .NE. nchc) GOTO 900
      DO k=1,ncha+3
         IF(k .GT. ncha) THEN
*     underflow, overflow
            j=k-ncha
            i1 = ista3 +j
            i2 = istb3 +j
            i3 = istc3 +j
            j1 = ista3 +3+j
            j2 = istb3 +3+j
            j3 = istc3 +3+j
         ELSE
*     normal bins
            i1 = ista +m_buf1 +k
            i2 = istb +m_buf1 +k
            i3 = istc +m_buf1 +k
            j1 = ista +m_buf1 +ncha+k
            j2 = istb +m_buf1 +ncha+k
            j3 = istc +m_buf1 +ncha+k
         ENDIF
         IF    (chr .EQ. '+')   THEN
            m_b(i3) =    coef1*m_b(i1) +    coef2*m_b(i2)
            m_b(j3) = coef1**2*m_b(j1) + coef2**2*m_b(j2)
         ELSEIF(chr .EQ. '-')   THEN
            m_b(i3) = coef1*m_b(i1) - coef2*m_b(i2)
            m_b(j3) = coef1**2*m_b(j1) + coef2**2*m_b(j2)
         ELSEIF(chr .EQ. '*')   THEN
            m_b(j3) = (coef1*coef2)**2
     $           *(m_b(j1)*m_b(i2)**2 + m_b(j2)*m_b(i1)**2)
            m_b(i3) = coef1*m_b(i1) * coef2*m_b(i2)
         ELSEIF(chr .EQ. '/')   THEN
            IF(m_b(i2) .EQ. 0d0) THEN
               m_b(i3) = 0d0
               m_b(j3) = 0d0
            ELSE
***               m_b(j3) = (coef1/coef2)**2/m_b(i2)**4           ! problems with overflow
***     $              *(m_b(j1)*m_b(i2)**2 + m_b(j2)*m_b(i1)**2) ! problems with overflow
               m_b(j3) = (coef1/coef2)**2 *m_b(j1) /m_b(i2)**2
     $                  +(coef1/coef2)**2 *m_b(j2) *(m_b(i1)/m_b(i2)**2)**2
               m_b(i3) = (coef1*m_b(i1) )/( coef2*m_b(i2))
            ENDIF
         ELSE
            GOTO 901
         ENDIF
      ENDDO
      RETURN
 900  WRITE(m_out,*) '+++++ GLK_Operat: non-equal no. bins ',ida,idb,idc
      WRITE(    6,*) '+++++ GLK_Operat: non-equal no. bins ',ida,idb,idc
      STOP
 901  WRITE(m_out,*) '+++++ GLK_Operat: wrong chr=',chr
      WRITE(    6,*) '+++++ GLK_Operat: wrong chr=',chr
      STOP
      END

      SUBROUTINE GLK_hinbo1(id,title,nchx,xl,xu)
*     **************************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id,nchx
      DOUBLE PRECISION  xl,xu
      CHARACTER*80 title
      INTEGER           lact,ist,ist2
*----------------------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) THEN
         CALL GLK_Stop1('+++STOP in GLK_hinbo1: wrong id=',id)
      ENDIF
      ist=m_index(lact,2)
      ist2   = ist+7
      nchx   = m_b(ist2 +1)
      xl     = m_b(ist2 +2)
      xu     = m_b(ist2 +3)
      title  = m_titlc(lact)
      END

      SUBROUTINE GLK_UnPak(id,a,chd1,idum)
*     *********************************
* getting out histogram content (and error)
* chd1= 'ERRO' is nonstandard option (unpack errors)
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id,idum
      DOUBLE PRECISION  a(*)
      CHARACTER*(*) chd1
*
      INTEGER            lact,ist,ist2,iflag2,ityphi,local,nch,nchy,ib
*------------------------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) goto 900
      ist   = m_index(lact,2)
      ist2  = ist+7
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 1) THEN
         nch   = m_b(ist2 +1)
         local = ist +m_buf1
      ELSEIF(ityphi .EQ. 2) THEN
         nchy  = m_b(ist2+5)
         nch   = nch*nchy
         local = ist+ m_buf2
      ELSE
         CALL GLK_Stop1('+++GLK_UnPak: check type of histo id=',id)
      ENDIF
      do 10 ib=1,nch
      IF(chd1 .NE. 'ERRO') THEN
* normal bin
        a(ib) = m_b(local+ib)
      ELSE
* error content
        IF(ityphi .EQ. 2) goto 901
        a(ib) = dsqrt( dabs(m_b(local+nch+ib) ))
      ENDIF
   10 continue
      RETURN
 900  CALL GLK_Retu1('+++GLK_UnPak: nonexisting id=',id)
      RETURN
 901  CALL GLK_Retu1('+++GLK_UnPak: no errors, two-dim, id=',id)
      END

      SUBROUTINE GLK_Pak(id,a)
*     ************************
* Loading in histogram content
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  a(*)
*
      INTEGER           lact,ist,ist2,iflag2,ityphi,nch,local,ib,nchy
*----------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) goto 900
      ist  = m_index(lact,2)
      ist2 = ist+7
* 2-dimens histo alowed
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 1) THEN
         nch   = m_b(ist2 +1)
         local = ist+m_buf1
      ELSEIF(ityphi .EQ. 2) THEN
         nchy  = m_b(ist2+5)
         nch   = nch*nchy
         local = ist+m_buf2
      ELSE
         CALL GLK_Stop1('+++GLK_Pak: wrong histo type, id=',id)
      ENDIF
      DO ib=1,nch
         m_b(local +ib) = a(ib)
      ENDDO
* one nominal entry recorded
      m_index(lact,3)  = 1
      RETURN
  900 CONTINUE
      CALL GLK_Stop1('+++GLK_Pak: nonexisting id=',id)
      END

      SUBROUTINE GLK_Pake(id,a)
*     **********************
* Loading in error content
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  a(*)
*
      INTEGER           lact,ist,ist2,nch,iflag2,ityphi
      INTEGER           nb,ib
*---------------------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) goto 901
      ist  = m_index(lact,2)
      ist2 = ist+7
      nch=m_b(ist2+1)
* 2-dimens histo NOT alowed
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 1) GOTO 900
      DO ib=1,nch
         m_b(ist+m_buf1+nch+ib) = a(ib)**2
      ENDDO
      CALL GLK_idopt( id,'ERRO')
      RETURN
  900 CALL GLK_Stop1('+++GLK_Pake: only for 1-dim hist, id=',id)
      RETURN
  901 CALL GLK_Stop1('+++GLK_Pake: nonexisting id=',id)
      END


      SUBROUTINE GLK_Range1(id,ylr,yur)
*     *****************************
* provides y-scale for 1-dim plots
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  ylr,yur
*
      INTEGER           lact,ist,ist2,nch,ib,ioplog,iopsla,ioperb,iopsc1,iopsc2
      DOUBLE PRECISION  yl,yu
*-------------------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) RETURN
      ist  = m_index(lact,2)
      ist2 = ist+7
      nch  = m_b(ist2 +1)
      yl   = m_b(ist+m_buf1+1)
      yu   = m_b(ist+m_buf1+1)
      DO ib=1,nch
         yl = min(yl,m_b(ist+m_buf1+ib))
         yu = max(yu,m_b(ist+m_buf1+ib))
      ENDDO
* For default range some safety range is added
      yu = yu + 0.05*ABS(yu-yl)
***   yl = yl - 0.05*ABS(yu-yl) ! to be decided later on

* If range was pre-defined then yl,yu are overwritten
      CALL GLK_OptOut(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      IF(iopsc1 .EQ. 2) yl= m_b( ist +5)
      IF(iopsc2 .EQ. 2) yu= m_b( ist +6)
      ylr = yl
      yur = yu
      END


      SUBROUTINE GLK_hinbo2(id,nchx,xl,xu,nchy,yl,yu)
*     *******************************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id,nchx,nchy
      DOUBLE PRECISION  xl,xu,yl,yu
      INTEGER           lact,ist,ist2
*--------------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) goto 900
      ist  = m_index(lact,2)
      ist2 = ist+7
      nchx = m_b(ist2 +1)
      xl   = m_b(ist2 +2)
      xu   = m_b(ist2 +3)
      nchy = m_b(ist2 +5)
      yl   = m_b(ist2 +6)
      yu   = m_b(ist2 +7)
      RETURN
  900 CALL GLK_Stop1(' +++GLK_hinbo2: nonexisting histo id= ',id)
      END


      SUBROUTINE GLK_Ymaxim(id,wmax)
*     **************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  wmax
      INTEGER           lact,ist,jd,k
*-------------------------------------------------------
      IF(id .NE. 0) THEN
         CALL GLK_hadres(id,lact)
         IF(lact .EQ. 0) RETURN
         ist= m_index(lact,2)
         m_b(ist+6) =wmax
         CALL GLK_idopt(id,'YMAX')
      ELSE
         DO k=1,m_idmax
            IF(m_index(k,1) .EQ. 0) GOTO 20
            ist=m_index(k,2)
            jd =m_index(k,1)
            m_b(ist+6) =wmax
            CALL GLK_idopt(jd,'YMAX')
         ENDDO
 20      CONTINUE
      ENDIF
      END

      SUBROUTINE GLK_Yminim(id,wmin)
*     ******************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id
      DOUBLE PRECISION  wmin
      INTEGER           lact,ist,k,jd
*---------------------------------------------
      IF(id .NE. 0) THEN
         CALL GLK_hadres(id,lact)
         IF(lact .EQ. 0) RETURN
         ist =m_index(lact,2)
         m_b(ist+5) =wmin
         CALL GLK_idopt(id,'YMIN')
      ELSE
         DO k=1,m_idmax
            IF(m_index(k,1) .EQ. 0) GOTO 20
            ist=m_index(k,2)
            jd =m_index(k,1)
            m_b(ist+5) =wmin
            CALL GLK_idopt(jd,'YMIN')
         ENDDO
 20      CONTINUE
      ENDIF
      END

      SUBROUTINE GLK_Reset(id,chd1)
*     **************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER       id
      CHARACTER*(*) chd1
      INTEGER  lact,ist,ist2,iflag2,ityphi,ist3,nchx,nch,local,nchy,j
*-------------------------------------------
      CALL GLK_hadres(id,lact)
      IF(lact .LE. 0) RETURN
      ist  =m_index(lact,2)
      ist2 = ist+7
*
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .EQ. 1) THEN
* one-dim.
        ist3  = ist+11
        nchx  = m_b(ist2 +1)
        nch   = 2*nchx
        local = ist + m_buf1
      ELSEIF(ityphi .EQ. 2) THEN
* two-dim.
        ist3  = ist+15
        nchx  = m_b(ist2 +1)
        nchy  = m_b(ist2 +5)
        nch   = nchx*nchy
        local = ist +m_buf2
      ELSE
         CALL GLK_Stop1('+++GLK_Reset: wrong type  id=',id)
      ENDIF
* reset miscaelaneous entries and bins
      DO j=ist3+1,local +nch
         m_b(j)    = 0d0
      ENDDO
* and no. of entries in m_index
      m_index(lact,3) = 0
      END

      SUBROUTINE GLK_Delet(id1)
*     *********************
* Now it should work (stj Nov. 91) but watch out!
* should works for 2-dim histos, please check this!
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           id1
*
      LOGICAL GLK_Exist
      INTEGER           id,lact,ist,ist2,nch,iflag2,ityphi,local,k,i,l,next,idec,nchx,nchy
*--------------------------------------------
      ID=ID1
      IF(id .EQ. 0) GOTO 300
      IF( .NOT. GLK_Exist(id)) GOTO 900
      CALL GLK_hadres(id,lact)
      ist  = m_index(lact,2)
      ist2 = ist+7
*----
*[[[      WRITE(6,*) 'GLK_DELET-ing ID= ',ID
      idec    = nint(m_b(ist+2)-9d0-9d12)/10
      IF(idec .NE. id) WRITE(6,*)
     $     '++++GLK_DELET: ALARM! ID,IDEC= ',ID,IDEC
*----
      nch  = m_b(ist2 +1)
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = MOD(iflag2,10)
      IF(ityphi .EQ. 1) THEN
* one-dim.
        nchx  = m_b(ist2 +1)
        nch   = 2*nchx
* lenght of local histo to be removed
        local = nch+m_buf1+1
      ELSEIF(ityphi .EQ. 2) THEN
* two-dim.
        nchx  = m_b(ist2 +1)
        nchy  = m_b(ist2 +5)
        nch   = nchx*nchy
* lenght of local histo to be removed
        local = nch+m_buf2+1
      ELSE
         CALL GLK_Stop1('+++GLK_Delet: wrong type id=',id)
      ENDIF
* starting position of next histo in storage b
      next = ist+1 +local
* move down all histos above this one
      DO 15 k =next,m_length
      m_b(k-local)=m_b(k)
   15 CONTINUE
* define new end of storage
      m_length=m_length-local
* clean free space at the end of storage b
      DO 20 k=m_length+1, m_length+local
   20 m_b(k)=0d0
* shift adresses of all displaced histos
      DO 25 l=lact+1,m_idmax
      IF(m_index(l,1) .NE. 0) m_index(l,2)=m_index(l,2)-local
   25 CONTINUE
* move entries in m_index down by one and remove id=lact entry
      DO 30 l=lact+1,m_idmax
      m_index(l-1,1)=m_index(l,1)
      m_index(l-1,2)=m_index(l,2)
      m_index(l-1,3)=m_index(l,3)
      m_titlc(l-1)=m_titlc(l)
   30 CONTINUE
* last entry should be always empty
      m_index(m_idmax,1)=0
      m_index(m_idmax,2)=0
      m_index(m_idmax,3)=0
      do 50 k=1,80
   50 m_titlc(m_idmax)(k:k)=' '
      RETURN
* -----------------------------------
* Deleting all histos at once!!!
  300 m_length=0
      DO 400 i=1,m_idmax
      DO 340 k=1,3
  340 m_index(i,k)=0
      DO 350 k=1,80
  350 m_titlc(i)(k:k)=' '
 400  CONTINUE
      RETURN
* -----------------------------------
 900  CONTINUE
      CALL GLK_Retu1(' +++GLK_DELET: nonexisting histo id= ',id)
      END


      SUBROUTINE GLK_Copch(ch1,ch2)
*     *****************************
      IMPLICIT NONE
* copies CHARACTER*80 ch1 into ch2 up to a first $ sign
      CHARACTER*80 ch1,ch2
      LOGICAL met
      INTEGER      i
*----------------------------
      met = .FALSE.
      DO i=1,80
         IF( ch1(i:i) .EQ. '$' .or. met )   THEN
            ch2(i:i)=' '
            met=.TRUE.
         ELSE
            ch2(i:i)=ch1(i:i)
         ENDIF
      ENDDO
      END

      INTEGER FUNCTION GLK_jadre2(id)
*------------------------------------------------
* Good old version -- but it is very very slow!!!
* In the case of 100 histograms or more.
*------------------------------------------------
      IMPLICIT NONE 
      INCLUDE 'GLK.fi'
      INTEGER           id,i
*---------------------------------------
      GLK_jadre2=0
      DO 1 i=1,m_idmax
      IF(m_index(i,1) .EQ. id) goto 2
    1 CONTINUE
* Nothing found.
      RETURN
* Found: id=0 is also legitimate find!!!
    2 GLK_jadre2=i
      END

      SUBROUTINE GLK_hadres(id1,jadres)
*     *********************************
*--------------------------------------------------------------------
* Educated guess based on past history is used to find quickly
* location of the histogram in the matrix m_index.
* This is based on observation that subsequent histogram calls
* are linked into loops (so one can predict easily which histo will
* be called next time).
*--------------------------------------------------------------------
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER       id1,jadres
      INTEGER       ist,i,id
*----------------------------------------------------------------------
      INTEGER iguess,jdlast,idlast
      DATA    iguess,jdlast,idlast /-2141593,-3141593,-3141593/
      SAVE    iguess,jdlast,idlast
*----------------------------------------------------------------------
      id=id1
* --- The case of ID=0 treated separately, it is used to find out
* --- last entry in the m_index (it is marked with zero)
      IF(id .EQ. 0) THEN
         DO i=1,m_idmax
            IF(m_index(i,1) .EQ. 0) GOTO 44
         ENDDO
         CALL GLK_Stop1('+++GLK_hadres: Too short m_index=',m_index)
 44      CONTINUE
         jadres = i
         RETURN
      ENDIF

* --- Omit sophistications if lack of initialization
      IF(jdlast .EQ. -3141593) GOTO 10
      IF(iguess .EQ. -2141593) GOTO 10
      IF(iguess .EQ. 0) GOTO 10
      IF(jdlast .EQ. 0) GOTO 10

* --- Try first previous histo (for repeated calls)
      IF(jdlast .LT. 1 .OR. jdlast .GT. m_idmax) THEN
         WRITE(6,*) '+++++ hadres: jdlast=',jdlast
      ENDIF
      IF(m_index(jdlast,1) .EQ. id) THEN
         jadres = jdlast
*##   WRITE(6,*)
*##   $   'found, guess based on previous call to jadres ',jdlast
         GOTO 20
      ENDIF

* --- Try current guess based on previous call
      IF(iguess .LT. 1 .OR. iguess .GT. m_idmax)  THEN
         WRITE(6,*)'+++++ hadres: iguess=',iguess
      ENDIF
      IF(m_index(iguess,1) .EQ. id) THEN
         jadres = iguess
*##   WRITE(6,*)
*##   $   'found, guess on previous calls recorded in m_b(ist+7)',jdlast
         GOTO 20
      ENDIF

* ================================================
*    Do it HARD WAY, Search all matrix m_index
* ================================================
 10   CONTINUE
*##   WRITE(6,*) 'trying HARD WAY'
      DO i=1,m_idmax
         jadres=i
         IF(m_index(i,1) .EQ. id) GOTO 20
      ENDDO
* -------------------------------------
*     Nothing found: jadres=0
* -------------------------------------
      jadres=0
      RETURN
* =====================================
*     Found: Set new guess for next call
* =====================================
 20   CONTINUE
* --- and store result as a new guess in previous histo
* --- but only if it existed!!!!
      DO i=1,m_idmax
         IF(m_index(i,1) .EQ. 0) GOTO 40
         IF(m_index(i,1) .EQ. idlast) THEN
            ist=m_index(i,2)
            IF(ist .GT. 0 .AND. ist .LT. m_LenmB) m_b(ist +7) = jadres
*##   WRITE(6,*) 'STORED     id=',id
            GOTO 40
         ENDIF
      ENDDO
 40   CONTINUE
*##   WRITE(6,*)  'found, hard way searching all of m_index)', jdlast
      iguess = m_b( m_index(jadres,2) +7)
      jdlast = jadres
      idlast = id
      END


*--------------------------------------------------------------
* ----------- storing histograms in the disk file -------------
*--------------------------------------------------------------

      SUBROUTINE GLK_ReadFile(Dname)
*     ******************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER ninph
      CHARACTER*60 Dname
*-------------------------------------------------
      CALL GLK_Initialize
* Read histograms
      WRITE(    *,*) 'GLK_ReadFile: Reading histos from ', Dname
      WRITE(m_out,*) 'GLK_ReadFile: Reading histos from ', Dname
      ninph = 21
      OPEN(ninph,file=Dname)           ! Open dump-file
      CALL GLK_hrfile(ninph,' ',' ')   ! Define dump-file unit in GKL
      CALL GLK_hrin(0,0,0)             ! Read histos from dump-file
      CALL GLK_hrend(' ')              ! Close dump-file
      END

      SUBROUTINE GLK_WriteFile(Dname)
*     ******************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER nouth
      CHARACTER*60 Dname
*-------------------------------------------------
      CALL GLK_Initialize
* Write all histograms into disk file
      WRITE(    *,*) 'GLK_WriteFile: Writing histos into ', Dname
      WRITE(m_out,*) 'GLK_WriteFile: Writing histos into ', Dname
      nouth=22
      OPEN(nouth,file=Dname)           ! Open dump-file
      CALL GLK_hrfile(nouth,' ',' ')   ! Define dump-file in GLK
      CALL GLK_hrout(0,0,' ')          ! Dump all histos on disk
      CALL GLK_hrend(' ')              ! Close dump-file
      END

      SUBROUTINE GLK_hrfile(nhruni,chd1,chd2)
*     ***************************************
      IMPLICIT NONE
      CHARACTER*(*) chd1,chd2
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER nhruni
*-------------------------------------------------
      CALL GLK_Initialize
      m_huni=nhruni
      END

      SUBROUTINE GLK_hrout(idum1,idum2,chdum)
*     ***************************************
      IMPLICIT NONE
      CHARACTER*8 chdum
*
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER i,k,last,idum1,idum2
*-----------------------------------------------------------------------
      CALL GLK_Initialize
      CALL GLK_hadres(0,last)
      IF(last.EQ.0) GOTO 900
      m_LenInd = last -1 
      WRITE(m_huni,'(6i10)')    m_version, m_LenInd, m_LenmB, m_Length
      WRITE(m_huni,'(6i15)')    ((m_index(i,k),k=1,3),i=1,m_LenInd)
      WRITE(m_huni,'(a80)')     (m_titlc(i),          i=1,m_LenInd)
      WRITE(m_huni,'(3d24.16)') (m_b(i), i=1,m_length)
      RETURN
 900  CONTINUE
      WRITE(m_out,*) '+++ GLK_hrout: no space in index'
      WRITE(    *,*) '+++ GLK_hrout: no space in index'
      END


      SUBROUTINE GLK_hrin(idum1,idum2,idum3)
*     **************************************
* New version which has a possibility to
*            MERGE histograms
* If given ID already exists then it is modified by adding 1000000 !!!!
* Mergigng is done simply by appending new histograms at the
* very end of the m_index and bin matrices.
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER        idum1,idum2,idum3
      INTEGER        l,lact,lenold,istn,idn,k,lenind3,nvrs3,nouth
      INTEGER        i,lengt3,lenma3
* Copy of the new m_index from the disk
      INTEGER        lndex(m_idmax,3)
      CHARACTER*80   titld(m_idmax)
      LOGICAL GLK_Exist
*-----------------------------------------------------------
      CALL GLK_Initialize
      nouth=m_huni
* Read basic params
      READ(nouth,'(6i10)')   nvrs3,lenind3,lenma3,lengt3
      IF(m_length+lengt3 .GE. m_LenmB) GOTO 900
* Check version
      IF(m_version .NE. nvrs3) WRITE(m_out,*)
     $ '+++++WARNING (GLK_hrin): histos produced by older version',nvrs3
      IF(m_version .NE. nvrs3) WRITE(6,*)
     $ '+++++WARNING (GLK_hrin): histos produced by older version',nvrs3
      DO i=1,m_idmax
         DO k=1,3
            lndex(i,k)=0
         ENDDO
      ENDDO
* Read new index and title from the disk
* We keep backward compatibility for reading disk files
      IF(nvrs3. LT. 130) lenind3 = m_idmax
      IF(nvrs3. LT. 134) THEN
         READ(nouth,'(6i10)')  ((lndex(i,k),k=1,3),i=1,lenind3)
      ELSE
         READ(nouth,'(6i15)')  ((lndex(i,k),k=1,3),i=1,lenind3)
      ENDIF
      READ(nouth,'(a80)')   (titld(i),          i=1,lenind3)
      lenold=m_length
* Append AT ONCE content of new histos at the end of storage m_b
      m_length=m_length+lengt3
      READ(nouth,'(3d24.16)') (m_b(i),i=lenold+1,m_length)

* Append ONE BY ONE m_index and m_titlc with new histos
      CALL GLK_hadres(0,lact)
      DO l=1,lenind3
         IF(lact .EQ. 0) GOTO 901
         idn= lndex(l,1)
         IF(idn .EQ. 0) GOTO 100
* Identical id's are changed by adding big number = 1000000
 10      CONTINUE
         IF( GLK_Exist(idn) ) THEN
            idn = idn +1000000*(idn/iabs(idn))
            GOTO 10
         ENDIF
         m_index(lact,1)=idn
         m_index(lact,2)=lndex(l,2)+lenold
         m_index(lact,3)=lndex(l,3)
         m_titlc(lact)  =titld(l)
* Still one small correction in the newly appended histo
         istn  = m_index(lact,2)
         m_b(istn +2)  = 9d12 +     idn*10 +9d0
         lact=lact+1
      ENDDO
  100 CONTINUE
      RETURN

 900  CONTINUE
      CALL GLK_Stop1('++++ GLK_hrin: to litle space, m_LenmB= ',m_LenmB)
 901  CONTINUE
      CALL GLK_Stop1('++++ GLK_hrin: to many histos, m_idmax= ',m_idmax)
      END


      SUBROUTINE GLK_hrin2(idum1,idum2,idum3)
*     **********************************
* New version which has a possibility to
*            ADD histograms
* If ID is not existing already then no action is taken
*     ***********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER         idum1,idum2,idum3
* Copy of the histos from the disk
      DOUBLE PRECISION  bz(m_LenmB)
      INTEGER           indez(m_idmax,3)
      CHARACTER*80      titlz(m_idmax)
      LOGICAL           GLK_Exist
      INTEGER           nouth,ist3,nchx,ist,ist2,ist3z,nchxz,istz
      INTEGER           ist2z,lact,lenmaz,lengtz,nvrsz,lenindz,lz,id,i,k
*-------------------------------------------------------------
      CALL GLK_Initialize
      nouth=m_huni
* Read basic params
      READ(nouth,'(6i10)')   nvrsz,lenindz,lenmaz,lengtz
* Check version
      IF(m_version .NE. nvrsz) WRITE(m_out,*)
     $ '++++WARNING (GLK_hrin2): histos produced by older version',nvrsz
      IF(m_version .NE. nvrsz) WRITE(*,*)
     $ '++++WARNING (GLK_hrin2): histos produced by older version',nvrsz

* We keep backward compatibility for reading disk files
      IF(nvrsz. LT. 130) lenindz = m_idmax
      DO i=1,m_idmax
         DO k=1,3
            indez(i,k)=0
         ENDDO
      ENDDO
* Read new m_index, title and bins from the disk
      IF(nvrsz. LT. 140) THEN
         READ(nouth,'(6i10)')    ((indez(i,k),k=1,3),i=1,lenindz)
      ELSE
         READ(nouth,'(6i15)')    ((indez(i,k),k=1,3),i=1,lenindz)
      ENDIF
      READ(nouth,'(a80)')     (titlz(i) ,         i=1,lenindz)
      READ(nouth,'(3d24.16)') (bz(i),i=1,lengtz)

* Add new histos from disk to existing ones one by one
      DO lz=1,lenindz
         id= indez(lz,1)
         IF(id .EQ. 0) GOTO 200
         IF( .NOT. GLK_Exist(id)) THEN
            CALL GLK_Retu1('GLK_hrin2: unmached skipped histo ID=', id)
            GOTO 100
         ENDIF
* parameters of existing histo
         CALL GLK_hadres(id,lact)
         ist  = m_index(lact,2)
         ist2 = ist+7
         ist3 = ist+11
         nchx = m_b(ist2 +1)
* parameters of the histo from the disk
         istz   = indez(lz,2)
         ist2z  = istz+7
         ist3z  = istz+11
         nchxz  = bz(ist2z +1)
         IF(nchx .NE. nchxz) THEN
            CALL GLK_Retu1('GLK_hrin2: non-equal bins, skipped ID=', id)
            GOTO 100
         ENDIF
* Add/Merge all additive entries of the two histos
* No of entries in m_index
         m_index(lact,3) = m_index(lact,3)+indez(lact,3)
* Overflows, underflows etc.
         DO i=1,12
            m_b(ist3+i)=m_b(ist3+i) +bz(ist3z+i)
         ENDDO
* Except of this one non-additive entry
         m_b(ist3+13)=max(m_b(ist3+13),m_b(ist3z+13))
* Regular bin content added now!
         DO i= 1, 2*nchx
            m_b(ist+m_buf1+i)=m_b(ist+m_buf1+i) +bz(istz+m_buf1+i)
         ENDDO
 100     CONTINUE
      ENDDO
 200  CONTINUE
      END

      SUBROUTINE GLK_hrend(chdum)
*     ***********************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      CHARACTER*(*) chdum
*---------------------------
      CLOSE(m_huni)
      END
*======================================================================
*                End of histograming procedures
*======================================================================



*======================================================================
*               Ploting procedures using LaTeX
*======================================================================

      SUBROUTINE GLK_PlInitialize(Lint,TeXfile)
*----------------------------------------------------------------------
* Lint =0 
*     Normal mode, full LaTeX header
* Lint =1
*     For TeX file is used in \input, no  LaTeX header
* Lint =2
*     LaTeX header for one-page plot used as input for postscript
*
* Negative Lint only for debug, big frame around plot is added.
*----------------------------------------------------------------------
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER Lint,noufig
      CHARACTER*60  TeXfile
*---------------------------------
* Initialize GLK_Plot
      CALL GLK_PlInt(Lint)             ! Define header style
      noufig=11                        ! 
      OPEN(noufig,file=TeXfile)        ! Open LaTeX file
      CALL GLK_PlCap(noufig)           ! Initialize GLK_Plot
      END

      SUBROUTINE GLK_PlEnd
*     ********************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
*---------------------------------------------------
* Note that TeX file is used in \input then you may not want
* to have header and \end{document}
      IF( ABS(m_lint) .NE. 1) THEN
         WRITE(m_ltx,'(2A)') m_BS,'end{document}'
      ENDIF
      CLOSE(m_ltx)
      END

      SUBROUTINE GLK_PlInt(Lint)
*     **************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER Lint
*---------------------------------
      m_lint = Lint
      END

      SUBROUTINE GLK_PlCap(LtxUnit)
*     ****************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER   LtxUnit,i,k
*---------
      CALL GLK_Initialize
      m_KeyTit= 0
      DO i=1,m_titlen
         DO k=1,80
            m_titch(i)(k:k)=' '
         ENDDO
      ENDDO
*---------
      m_tline = 1
      m_ltx=IABS(LtxUnit)

      IF( ABS(m_lint) .EQ. 0) THEN
* Normal mode, no colors!!!
         WRITE(m_ltx,'(A,A)') m_BS,'documentclass[12pt]{article}'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{html}'
         WRITE(m_ltx,'(A,A)') m_BS,'textwidth  = 16cm'
         WRITE(m_ltx,'(A,A)') m_BS,'textheight = 18cm'
         WRITE(m_ltx,'(A,A)') m_BS,'begin{document}'
         WRITE(m_ltx,'(A)') '  '
      ELSEIF( ABS(m_lint) .EQ. 1) THEN
* For TeX file is used in \input
         WRITE(m_ltx,'(A)') '  '
      ELSEIF( ABS(m_lint) .EQ. 2) THEN
* For one-page plot being input for postrscript
*!!         WRITE(m_ltx,'(A,A)') m_BS,'documentclass[12pt,html]{article}'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'documentclass[12pt,dvips]{seminar}' !<-for colors!!!
         WRITE(m_ltx,'(A,A)') m_BS,'documentclass[12pt,dvips]{article}'
         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{amsmath}'
         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{amssymb}'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{html}'
         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{epsfig}'
         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{epic}'
         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{eepic}'
         WRITE(m_ltx,'(A,A)') m_BS,'usepackage{color}' !<-for colors!!!
*!!         WRITE(m_ltx,'(A,A)') m_BS,'hoffset    = -1in'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'voffset    = -1in'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'textwidth  = 16cm'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'textheight = 16cm'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'oddsidemargin = 0cm'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'topmargin     = 0cm'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'headheight    = 0cm'
*!!         WRITE(m_ltx,'(A,A)') m_BS,'headsep       = 0cm'
         WRITE(m_ltx,'(A,A)') m_BS,'begin{document}'
         WRITE(m_ltx,'(A,A)') m_BS,'pagestyle{empty}'
         WRITE(m_ltx,'(A)') '  '
      ELSE
         CALL GLK_Stop1('+++STOP in GLK_PlInt, wrong m_lint=',m_lint)
      ENDIF
      END


      SUBROUTINE GLK_Plot(id,ch1,ch2,kdum)
*     ************************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      CHARACTER CH1,CH2,CHR
      CHARACTER*80 TITLE
      INTEGER          id,kdum
      DOUBLE PRECISION YY(m_MaxNb),YER(m_MaxNb)
      LOGICAL GLK_EXIST
      INTEGER          idum,kax,kay,ioplog,iopsla,ioperb,iopsc1,iopsc2
      INTEGER          ker,nchx
      DOUBLE PRECISION XL,XU,DXL,DXU,yl,yu
*--------------------------------------------
      DATA CHR /' '/
* RETURN if histo non-existing
      IF(.NOT.GLK_EXIST(ID)) GOTO 900
* ...unpack histogram
      CALL GLK_UNPAK(ID,YY ,'    ',IDUM)
      CALL GLK_UNPAK(ID,YER,'ERRO',IDUM)
      CALL GLK_HINBO1(ID,TITLE,NCHX,DXL,DXU)
      XL = DXL
      XU = DXU
      CALL GLK_RANGE1(ID,YL,YU)
      kax=1200
      kay=1200
      IF(CH1 .EQ. 'S') THEN
* ...superimpose plot
        BACKSPACE(m_ltx)
        BACKSPACE(m_ltx)
      ELSE
* ...new frame only
        CHR=CH1
        CALL GLK_Plfram1(ID,kax,kay)
      ENDIF
      WRITE(m_ltx,'(A)')     '%========== next plot (line) =========='
      WRITE(m_ltx,'(A,I10)') '%==== HISTOGRAM ID=',ID
      WRITE(m_ltx,'(A,A70 )')'% ',TITLE
*...cont. line for functions
      CALL GLK_OptOut(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker = ioperb-1
      IF (iopsla .EQ. 2)  CHR='C'
*...suppress GLK_PLOT assignments
      IF (CH2 .EQ. 'B')   CHR=' '
      IF (CH2 .EQ. '*')   CHR='*'
      IF (CH2 .EQ. 'C')   CHR='C'
*...various types of lines
      IF     (CHR .EQ. ' ') THEN
*...contour line used for histogram
          CALL GLK_PlHist(kax,kay,NCHX,YL,YU,YY,KER,YER)
      ELSE IF(CHR .EQ. '*') THEN
*...marks in the midle of the bin
          CALL GLK_PlHis2(kax,kay,NCHX,YL,YU,YY,KER,YER)
      ELSE IF(CHR .EQ. 'C') THEN
*...slanted (dotted) line in plotting non-MC functions
          CALL GLK_PlCirc(kax,kay,NCHX,YL,YU,YY)
      ENDIF
*------------------------------!
* Ending
*------------------------------!
      WRITE(m_ltx,'(2A)') m_BS,'end{picture} % close entire picture '
      WRITE(m_ltx,'(2A)') m_BS,'end{figure}'

      RETURN
  900 CALL GLK_Retu1('+++GLK_PLOT: Nonexistig histo, skipped, id=' ,ID)
      END

      SUBROUTINE GLK_Plfram1(ID,kax,kay)
*     **********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           ID,kax,kay
      CHARACTER*80 title
      DOUBLE PRECISION   TIPSY(20),TIPSX(20)
      DOUBLE PRECISION   XL,DXL,XU,DXU
      INTEGER            ntipy,ntipx,nchx,icont
      DOUBLE PRECISION   yu,yl
      DATA ICONT/0/
*----------------
      ICONT=ICONT+1
      CALL GLK_HINBO1(ID,TITLE,NCHX,DXL,DXU)
      XL = DXL
      XU = DXU
      CALL GLK_RANGE1(ID,YL,YU)

      IF(ICONT .GT. 1) WRITE(m_ltx,'(2A)') m_BS,'newpage'
*------------------------------!
*           Header
*------------------------------!
      WRITE(m_ltx,'(A)') ' '
      WRITE(m_ltx,'(A)') ' '
      WRITE(m_ltx,'(A)') '%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     $%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(m_ltx,'(A)') '%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     $%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(m_ltx,'(2A)') m_BS,'begin{figure}[!ht]'
      WRITE(m_ltx,'(2A)') m_BS,'centering'
*------------------------------!
* General Caption
*------------------------------!
      WRITE(m_ltx,'(4A)') m_BS,'caption{',m_BS,'small'
      IF(M_KEYTIT.EQ.0) THEN
        WRITE(m_ltx,'(A)')     TITLE
      ELSE
        WRITE(m_ltx,'(A)')     m_titch(1)
      ENDIF
      WRITE(m_ltx,'(A)') '}'
*------------------------------!
* Frames and labels
*------------------------------!
      WRITE(m_ltx,'(A)') '% =========== big frame, title etc. ======='
      WRITE(m_ltx,'(4A)') m_BS,'setlength{',m_BS,'unitlength}{0.1mm}'
      WRITE(m_ltx,'(2A)') m_BS,'begin{picture}(1600,1500)'
      WRITE(m_ltx,'(4A)')
     $     m_BS,'put(0,0){',m_BS,'framebox(1600,1500){ }}'
      WRITE(m_ltx,'(A)') '% =========== small frame, labeled axis ==='
      WRITE(m_ltx,'(4A,I4,A,I4,A)')
     $    m_BS,'put(300,250){',m_BS,'begin{picture}( ',kax,',',kay,')'
      WRITE(m_ltx,'(4A,I4,A,I4,A)')
     $    m_BS,'put(0,0){',m_BS,'framebox( ',kax,',',kay,'){ }}'
      WRITE(m_ltx,'(A)') '% =========== x and y axis ================'
      CALL GLK_SAxisX(kax,XL,XU,NTIPX,TIPSX)
      CALL GLK_SAxisY(kay,YL,YU,NTIPY,TIPSY)
      WRITE(m_ltx,'(3A)') m_BS,'end{picture}}'
     $                ,'% end of plotting labeled axis'
      END

      SUBROUTINE GLK_SAxisX(kay,YL,YU,NLT,TIPSY)
*     ***************************************
* plotting x-axis with long and short tips
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           kay,NLT
      DOUBLE PRECISION  YL,YU,TIPSY(20)
*
      INTEGER           LY,JY,n,nts,k,lex
      DOUBLE PRECISION  DY,pds,scmx,p0s,ddys,yy0l,ddyl,pdl,p0l,yy0s
*---------------------------------------------------
      DY= ABS(YU-YL)
      LY = NINT( LOG10(DY) -0.4999999d0 )
      JY = NINT(DY/10d0**LY)
      DDYL = DY*10d0**(-LY)
      IF( JY .EQ. 1)             DDYL = 10d0**LY*0.25d0
      IF( JY .GE. 2.AND.JY .LE. 3) DDYL = 10d0**LY*0.5d0
      IF( JY .GE. 4.AND.JY .LE. 6) DDYL = 10d0**LY*1.0d0
      IF( JY .GE. 7)             DDYL = 10d0**LY*2.0d0
      WRITE(m_ltx,'(A)') '% .......GLK_SAxisX........ '
      WRITE(m_ltx,'(A,I4)') '%  JY= ',JY
*-------
      NLT = INT(DY/DDYL)
      NLT = MAX0(MIN0(NLT,20),1)+1
      YY0L = NINT(YL/DDYL+0.5d0)*DDYL
      DDYS = DDYL/10d0
      YY0S = NINT(YL/DDYS+0.4999999d0)*DDYS
      P0L = kay*(YY0L-YL)/(YU-YL)
      PDL = kay*DDYL/(YU-YL)
      P0S = kay*(YY0S-YL)/(YU-YL)
      PDS = kay*DDYS/(YU-YL)
      NLT = INT(ABS(YU-YY0L)/DDYL+0.0000001d0)+1
      NTS = INT(ABS(YU-YY0S)/DDYS+0.0000001d0)+1
      DO 41 N=1,NLT
      TIPSY(N) =YY0L+ DDYL*(N-1)
  41  CONTINUE
      WRITE(m_ltx,1000)
     $ m_BS,'multiput('  ,P0L,  ',0)('  ,PDL,  ',0){'  ,NLT,  '}{',
     $ m_BS,'line(0,1){25}}',
     $ m_BS,'multiput('  ,P0S,  ',0)('  ,PDS,  ',0){'  ,NTS,  '}{',
     $ m_BS,'line(0,1){10}}'
      WRITE(m_ltx,1001)
     $ m_BS,'multiput('  ,P0L,  ','  ,kay,  ')('  ,PDL,  ',0){'  ,NLT,
     $ '}{'  ,m_BS,  'line(0,-1){25}}',
     $ m_BS,'multiput('  ,P0S,  ','  ,kay,  ')('  ,PDS,  ',0){'  ,NTS,
     $ '}{'  ,m_BS,  'line(0,-1){10}}'
 1000 FORMAT(2A,F8.2,A,F8.2,A,I4,3A)
 1001 FORMAT(2A,F8.2,A,I4,A,F8.2,A,I4,3A)
* ...labeling of axis
      SCMX = DMAX1(DABS(YL),DABS(YU))
      LEX  = NINT( LOG10(SCMX) -0.50001)
      DO 45 N=1,NLT
      K = NINT(kay*(TIPSY(N)-YL)/(YU-YL))
      IF(LEX .LT. 2.AND.LEX .GT. -1) THEN
* ...without exponent
      WRITE(m_ltx,'(2A,I4,5A,F8.3,A)')
     $ m_BS,'put(',K,',-25){',m_BS,'makebox(0,0)[t]{',m_BS,'large $ ',
     $ TIPSY(N), ' $}}'
      ELSE
* ...with exponent
      WRITE(m_ltx,'(2A,I4,5A,F8.3,2A,I4,A)')
     $ m_BS,'put(' ,K, ',-25){',m_BS,'makebox(0,0)[t]{',m_BS,'large $ ',
     $ TIPSY(N)/(10d0**LEX),m_BS,'cdot 10^{',LEX,'} $}}'
      ENDIF
  45  CONTINUE
      END

      SUBROUTINE GLK_SAxisY(kay,yl,yu,nlt,tipsy)
*     ******************************************
* plotting y-axis with long and short tips
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER          kay,nlt
      DOUBLE PRECISION yl,yu,tipsy(20)
*
      DOUBLE PRECISION dy,ddyl,z0l,scmx,yy0s,ddys,yy0l,p0l,pds,p0s,pdl
      INTEGER          ly,jy,n,nts,k,lex
*---------------------------------------------------
      DY= ABS(YU-YL)
      LY = NINT( LOG10(DY) -0.49999999d0 )
      JY = NINT(DY/10d0**LY)
      DDYL = DY*10d0**(-LY)
      IF( JY .EQ. 1)             DDYL = 10d0**LY*0.25d0
      IF( JY .GE. 2.AND.JY .LE. 3) DDYL = 10d0**LY*0.5d0
      IF( JY .GE. 4.AND.JY .LE. 6) DDYL = 10d0**LY*1.0d0
      IF( JY .GE. 7)             DDYL = 10d0**LY*2.0d0
      WRITE(m_ltx,'(A)') '% .......GLK_SAxisY........ '
      WRITE(m_ltx,'(A,I4)') '%  JY= ',JY
*-------
      NLT = INT(DY/DDYL)
      NLT = MAX0(MIN0(NLT,20),1)+1
      YY0L = NINT(YL/DDYL+0.4999999d0)*DDYL
      DDYS = DDYL/10d0
      YY0S = NINT(YL/DDYS+0.5d0)*DDYS
      P0L = kay*(YY0L-YL)/(YU-YL)
      PDL = kay*DDYL/(YU-YL)
      P0S = kay*(YY0S-YL)/(YU-YL)
      PDS = kay*DDYS/(YU-YL)
      NLT= INT(ABS(YU-YY0L)/DDYL+0.0000001d0) +1
      NTS= INT(ABS(YU-YY0S)/DDYS+0.0000001d0) +1
      DO 41 N=1,NLT
      TIPSY(N) =YY0L+ DDYL*(N-1)
  41  CONTINUE
* plotting tics on vertical axis
      WRITE(m_ltx,1000)
     $ m_BS,'multiput(0,'  ,P0L,  ')(0,'  ,PDL  ,'){'  ,NLT,  '}{',
     $ m_BS,'line(1,0){25}}',
     $ m_BS,'multiput(0,'  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,  '}{',
     $ m_BS,'line(1,0){10}}'
      WRITE(m_ltx,1001)
     $ m_BS,'multiput('  ,kay,  ','  ,P0L,  ')(0,'  ,PDL,  '){'  ,NLT,
     $ '}{',m_BS,'line(-1,0){25}}',
     $ m_BS,'multiput('  ,kay,  ','  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,
     $ '}{',m_BS,'line(-1,0){10}}'
 1000 FORMAT(2A,F8.2,A,F8.2,A,I4,3A)
 1001 FORMAT(2A,I4,A,F8.2,A,F8.2,A,I4,3A)
* ...Zero line if necessary
      Z0L = kay*(-YL)/(YU-YL)
      IF(Z0L .GT. 0D0.AND.Z0L .LT. FLOAT(kay))
     $      WRITE(m_ltx,'(2A,F8.2,3A,I4,A)')
     $       m_BS,'put(0,'  ,Z0L,  '){',m_BS,'line(1,0){'  ,kay,  '}}'
* ...labeling of axis
      SCMX = DMAX1(DABS(YL),DABS(YU))
      LEX  = NINT( LOG10(SCMX) -0.50001d0)
      DO 45 N=1,NLT
      K = NINT(kay*(TIPSY(N)-YL)/(YU-YL))
      IF(LEX .LT. 2.AND.LEX .GT. -1) THEN
* ...without exponent
      WRITE(m_ltx,'(2A,I4,5A,F8.3,A)')
     $  m_BS,'put(-25,'  ,K,  '){',m_BS,'makebox(0,0)[r]{',
     $  m_BS,'large $ '  ,TIPSY(N),  ' $}}'
      ELSE
* ...with exponent
      WRITE(m_ltx,'(2A,I4,5A,F8.3,2A,I4,A)')
     $ m_BS,'put(-25,'  ,K,  '){',m_BS,'makebox(0,0)[r]{',
     $ m_BS,'large $ '
     $ ,TIPSY(N)/(10d0**LEX),  m_BS,'cdot 10^{'  ,LEX,  '} $}}'
      ENDIF
  45  CONTINUE
      END

      SUBROUTINE GLK_PlHist(kax,kay,nchx,yl,yu,yy,ker,yer)
*     ************************************************
* plotting contour line for histogram
*     ***********************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           kax,kay,nchx,ker
      DOUBLE PRECISION  yl,yu,yy(*),yer(*)
      CHARACTER*80 FMT1
*
      INTEGER           IX0,ix2,idx,ie,ierr,idy,ib,iy0,iy1,ix1
*---------------------------------------------------
      WRITE(m_ltx,'(4A,I4,A,I4,A)')
     $  m_BS,'put(300,250){',m_BS,'begin{picture}( ',kax,',',kay,')'
      WRITE(m_ltx,'(A)') '% ========== plotting primitives =========='
*...various types of line
      IF(m_tline .EQ. 1) THEN
         WRITE(m_ltx,'(2A)') m_BS,'thicklines '
      ELSE
         WRITE(m_ltx,'(2A)') m_BS,'thinlines '
      ENDIF
*...short macros for vertical/horizontal straight lines
      WRITE(m_ltx,'(8A)')
     $ m_BS,'newcommand{',m_BS,'x}[3]{',m_BS,'put(#1,#2){',
     $ m_BS,'line(1,0){#3}}}'
      WRITE(m_ltx,'(8A)')
     $ m_BS,'newcommand{',m_BS,'y}[3]{',m_BS,'put(#1,#2){',
     $ m_BS,'line(0,1){#3}}}'
      WRITE(m_ltx,'(8A)')
     $ m_BS,'newcommand{',m_BS,'z}[3]{',m_BS,'put(#1,#2){',
     $ m_BS,'line(0,-1){#3}}}'
*   error bars
      WRITE(m_ltx,'(8A)')
     $   m_BS,'newcommand{',m_BS,'e}[3]{',
     $   m_BS,'put(#1,#2){',m_BS,'line(0,1){#3}}}'
      IX0=0
      IY0=0
      DO 100 IB=1,NCHX
      IX1 = NINT(kax*(IB-0.00001)/NCHX)   !ib=7
      IY1 = NINT(kay*(YY(IB)-YL)/(YU-YL)) !iy1=775,while ix0=168,iy0=770
      IDY = IY1-IY0
      IDX = IX1-IX0
      FMT1 = '(2(2A,I4,A,I4,A,I4,A))'
      IF( IDY .GE. 0) THEN
         IF(IY1 .GE. 0.AND.IY1 .LE. kay)
     $   WRITE(m_ltx,FMT1) m_BS,'y{',IX0,'}{',IY0,'}{',IDY,'}',
     $                     m_BS,'x{',IX0,'}{',IY1,'}{',IDX,'}'
      ELSE
         IF(IY1 .GE. 0.AND.IY1 .LE. kay)
     $   WRITE(m_ltx,FMT1) m_BS,'z{',IX0,'}{',IY0,'}{',-IDY,'}',
     $                     m_BS,'x{',IX0,'}{',IY1,'}{',IDX,'}'
      ENDIF
      IX0=IX1
      IY0=IY1
      IF(KER .EQ. 1) THEN
        IX2  = NINT(kax*(IB-0.5000d0)/NCHX)
        IERR = NINT(kay*((YY(IB)-YER(IB))-YL)/(YU-YL))
        IE = NINT(kay*YER(IB)/(YU-YL))
        IF(IY1 .GE. 0.AND.IY1 .LE. kay.and.abs(ierr) .LE. 9999
     $     .and.2*ie .LE. 9999) WRITE(m_ltx,8000) m_BS,IX2,IERR,IE*2
      ENDIF
 100  CONTINUE
8000  FORMAT(4(A1,2He{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(m_ltx,'(3A)') m_BS,'end{picture}}',
     $       ' % end of plotting histogram'
* change line-style
      m_tline= m_tline+1
      IF(m_tline .GT. 2) m_tline=1
      END

      SUBROUTINE GLK_PlHis2(kax,kay,nchx,yl,yu,yy,ker,yer)
*     ************************************************
* marks in the midle of the bin
*     **********************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      DOUBLE PRECISION  yl,yu,yy(*),yer(*)
      INTEGER           kax,kay,nchx,ker
*
      INTEGER           iy1,ierr,ie,ix1,irad1,irad2,ib
*---------------------------------------------------

      WRITE(m_ltx,'(4A,I4,A,I4,A)')
     $ m_BS,'put(300,250){',m_BS,'begin{picture}( ',kax,',',kay,')'
      WRITE(m_ltx,'(A)') '% ========== plotting primitives =========='
*...various types of mark
      IRAD1= 6
      IRAD2=10
      IF(m_tline .EQ. 1) THEN
*   small filled circle
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle*{',IRAD1,'}}}'
      ELSEIF(m_tline .EQ. 2) THEN
*   small open circle
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle{',IRAD1,'}}}'
      ELSEIF(m_tline .EQ. 3) THEN
*   big filled circle
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle*{',IRAD2,'}}}'
      ELSEIF(m_tline .EQ. 4) THEN
*   big open circle
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle{',IRAD2,'}}}'
* Other symbols
      ELSEIF(m_tline .EQ. 5) THEN
       WRITE(m_ltx,'(10A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'makebox(0,0){$',m_BS,'diamond$}}}'
      ELSE
       WRITE(m_ltx,'(10A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'makebox(0,0){$',m_BS,'star$}}}'
      ENDIF
*   error bars
      WRITE(m_ltx,'(8A)')
     $   m_BS,'newcommand{',m_BS,'E}[3]{',
     $   m_BS,'put(#1,#2){',m_BS,'line(0,1){#3}}}'
      DO 100 IB=1,NCHX
      IX1 = NINT(kax*(IB-0.5000d0)/NCHX)
      IY1 = NINT(kay*(YY(IB)-YL)/(YU-YL))
      IF(IY1 .GE. 0.AND.IY1 .LE. kay) WRITE(m_ltx,7000) m_BS,IX1,IY1
      IF(KER .EQ. 1) THEN
        IERR = NINT(kay*((YY(IB)-YER(IB))-YL)/(YU-YL))
        IE   = NINT(kay*YER(IB)/(YU-YL))
        IF(IY1 .GE. 0.AND.IY1 .LE. kay.and.abs(ierr) .LE. 9999
     $       .and.2*ie .LE. 9999) WRITE(m_ltx,8000) m_BS,IX1,IERR,IE*2
      ENDIF
 100  CONTINUE
7000  FORMAT(4(A1,2HR{,I4,2H}{,I4,1H}:1X ))
8000  FORMAT(4(A1,2HE{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(m_ltx,'(3A)') m_BS,'end{picture}}',
     $    ' % end of plotting histogram'
* change line-style
      m_tline= m_tline+1
      IF(m_tline .GT. 6) m_tline=1
      END

      SUBROUTINE GLK_PlCirc(kax,kay,nchx,yl,yu,yy)
*     ****************************************
* plots equidistant points, four-point interpolation,
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           kax,kay,nchx
      DOUBLE PRECISION  yl,yu,yy(*)
*
      INTEGER           IX(m_MaxNb),IY(m_MaxNb)
      DOUBLE PRECISION  ai0,dx,aj0,ds,facy,aj,ai
      INTEGER           ipnt,i,iter,ipoin,irad1,irad2
      DOUBLE PRECISION  GLK_AproF
*---------------------------------------------------

* ...various types of line
* ...distance between points is DS, radius of a point is IRAD
      IRAD2=6
      IRAD1=3
* .............
      WRITE(m_ltx,'(4A,I4,A,I4,A)')
     $  m_BS,'put(300,250){',m_BS,'begin{picture}( ',kax,',',kay,')'
      WRITE(m_ltx,'(A)') '% ========== plotting primitives =========='
      IF(m_tline .EQ. 1) THEN
*   small filled circle
       DS = 10
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle*{',IRAD1,'}}}'
      ELSEIF(m_tline .EQ. 2) THEN
*   small open circle
       DS = 10
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle{',IRAD1,'}}}'
      ELSEIF(m_tline .EQ. 3) THEN
*   big filled circle
       DS = 20
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle*{',IRAD2,'}}}'
      ELSEIF(m_tline .EQ. 4) THEN
*   big open circle
       DS = 20
       WRITE(m_ltx,'(8A,I3,A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'circle{',IRAD2,'}}}'
* Other symbols
      ELSEIF(m_tline .EQ. 5) THEN
       DS = 20
       WRITE(m_ltx,'(10A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'makebox(0,0){$',m_BS,'diamond$}}}'
      ELSE
       DS = 20
       WRITE(m_ltx,'(10A)')
     $   m_BS,'newcommand{',m_BS,'R}[2]{',
     $   m_BS,'put(#1,#2){',m_BS,'makebox(0,0){$',m_BS,'star$}}}'
      ENDIF
      FACY = kay/(YU-YL)
* plot first point
      AI  = 0.
      AJ  = (GLK_AproF( (AI/kax)*NCHX+0.5d0, NCHX, YY) -YL)*FACY
      IPNT =1
      IX(IPNT) = INT(AI)
      IY(IPNT) = INT(AJ)
      DX =  DS
      AI0 = AI
      AJ0 = AJ
* plot next points
      DO 100 IPOIN=2,3000
* iteration to get (approximately) equal distance among ploted points
      DO  50 ITER=1,3
      AI  = AI0+DX
      AJ  = (GLK_AproF( (AI/kax)*NCHX+0.5d0, NCHX, YY) -YL)*FACY
      DX  = DX *DS/SQRT(DX**2 + (AJ-AJ0)**2)
  50  CONTINUE
      IF(INT(AJ) .GE. 0.AND.INT(AJ) .LE. kay.AND.INT(AI) .LE. kax) THEN
         IPNT = IPNT+1
         IX(IPNT) = INT(AI)
         IY(IPNT) = INT(AJ)
      ENDIF
      AI0 = AI
      AJ0 = AJ
      IF(INT(AI) .GT. kax) GOTO 101
 100  CONTINUE
 101  CONTINUE
      WRITE(m_ltx,7000) (m_BS,IX(I),IY(I), I=1,IPNT)
7000  FORMAT(4(A1,2HR{,I4,2H}{,I4,1H}:1X ))
      WRITE(m_ltx,'(2A)') m_BS,'end{picture}} % end of plotting line'
* change line-style
      m_tline= m_tline+1
      IF(m_tline .GT. 2) m_tline=1
      END

      DOUBLE PRECISION  FUNCTION GLK_AproF(px,nch,yy)
*     ************************************************
* PX is a continuous extension of the m_index in array YY
      IMPLICIT NONE
      INTEGER           nch,ip
      DOUBLE PRECISION  px,yy(*),X,p
*-----------------------------------------------------
      X=PX
      IF(X .LT. 0.0.OR.X .GT. FLOAT(NCH+1)) THEN
        GLK_AproF= -1E-20
        RETURN
      ENDIF
      IP=INT(X)
      IF(IP .LT. 2)     IP=2
      IF(IP .GT. NCH-2) IP=NCH-2
      P=X-IP
      GLK_AproF =
     $     -(1./6.)*P*(P-1)*(P-2)  *YY(IP-1)
     $     +(1./2.)*(P*P-1)*(P-2)  *YY(IP  )
     $     -(1./2.)*P*(P+1)*(P-2)  *YY(IP+1)
     $     +(1./6.)*P*(P*P-1)      *YY(IP+2)
      END

      SUBROUTINE GLK_PlTitle(title)
*     *****************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      CHARACTER*80 title
*----------------------------------------
      m_KeyTit=1
      CALL GLK_Copch(title,m_titch(1))
      END

      SUBROUTINE GLK_PlCapt(lines)
*     ****************************
* This routine defines caption and should be called
* before CALL GLK_Plot2, GLK_PlTable or bpltab2
* The matrix CHARACTER*80 lines containes text of the caption ended
* with the last line '% end-of-caption'
      IMPLICIT NONE
      CHARACTER*80 lines(*)
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER i
*----------------------------------
      m_KeyTit=0
      DO i=1,m_titlen
         m_titch(i)=lines(i)
         m_KeyTit= m_KeyTit+1
         IF(lines(i) .EQ. '% end-of-caption' ) GOTO 100
      ENDDO
      CALL GLK_Retu1(' WARNING from GLK_PlCapt: to many lines =',m_titlen)
 100  CONTINUE
      END

      SUBROUTINE GLK_PlLabel(lines)
*     *****************************
* This should be envoked after 'CALL GLK_Plot2'
* to add lines of TeX to a given plot
*     ***********************************
      IMPLICIT NONE
      CHARACTER*80 lines(*)
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER i
*----------------------------------
      m_KeyTit=0
      DO i=1,m_titlen
         m_titch(i)=lines(i)
         m_KeyTit= m_KeyTit+1
         IF(lines(i) .EQ. '% end-of-label' ) GOTO 100
      ENDDO
      CALL GLK_Retu1(' WARNING from GLK_PlLabel: to many lines =',m_titlen)
 100  CONTINUE
*------------------------------!
*   erase Ending               !
*------------------------------!
      BACKSPACE(m_ltx)
      BACKSPACE(m_ltx)
*
      DO i=1,m_KeyTit
        WRITE(m_ltx,'(A)')     m_titch(i)
      ENDDO
*------------------------------!
*   restore Ending             !
*------------------------------!
      WRITE(m_ltx,'(2A)') m_BS,'end{picture} % close entire picture '
      IF(ABS(m_lint) .EQ. 2) THEN
         WRITE(m_ltx,'(A)') '%====== end of GLK_PlLabel =========='
      ELSE
         WRITE(m_ltx,'(2A)') m_BS,'end{figure}'
      ENDIF
      END


      SUBROUTINE GLK_Plot2(id,ch1,ch2,chmark,chxfmt,chyfmt)
*     *****************************************************
* The new, more user-friendly, version of older GLK_Plot
* INPUT:
*    ID          histogram identifier
*    ch1 = ' '   normal new plot
*        = 'S'   impose new plot on previous one
*    ch2 = ' '   ploting line default, contour
*        = '*'   error bars in midle of the bin
*        = 'R'   error bars at Right edge of the bin
*        = 'L'   error bars at Left  edge of the bin
*        = 'C'   slanted continuous smooth line
*    chmark =    TeX symbol for ploting points
*    chxfmt =    format (string) for labeling x-axis
*    chyfmt =    format (string) for labeling y-axis
* Furthermore:
* Captions are defined by means of
*    CALL GLK_PlCapt(capt) before CALL GLK_Plot2
*    where CHARACTER*80 capt(50) is content of
*    caption, line by line, see also comments in GLK_PlCapt routine.
* Additional text as a TeX source text can be appended by means of
*    CALL GLK_PlLabel(lines) after CALL GLK_Plot2
*    where CHARACTER*80 lines(50) is the TeX add-on.
*    This is to be used to decorate plot with
*    any kind marks, special labels and text on the plot.
*
*     ************************************
      IMPLICIT NONE
      INTEGER id
      CHARACTER ch1,ch2,chmark*(*)
      CHARACTER*8 chxfmt,chyfmt
      INCLUDE 'GLK.fi'
      SAVE
      DOUBLE PRECISION  yy(m_MaxNb),yer(m_MaxNb)
      CHARACTER*80 title
*---------------------------------------------------------------------
      LOGICAL GLK_Exist
      INTEGER kax,kay,incr,ker,nchx
      INTEGER iopsla,ioplog,ioperb,iopsc1,iopsc2,idum
      DOUBLE PRECISION   dxl,dxu,xu,xl,yu,yl
      CHARACTER chr
      DATA CHR /' '/
* TeX Names of the error-bar command and of the point-mark command
      CHARACTER*1 chre, chrp1
      PARAMETER ( chre = 'E', chrp1= 'R' )
      CHARACTER*2 chrp
* TeX Name of the point-mark command
      CHARACTER*1 chrx(12)
      DATA  chrx /'a','b','c','d','f','g','h','i','j','k','l','m'/
*---------------------------------------------------------------------
* RETURN if histo non-existing
      IF(.NOT.GLK_Exist(id)) GOTO 900
* ...unpack histogram
      CALL GLK_UnPak(id,yy ,'    ',idum)
      CALL GLK_UnPak(id,yer,'ERRO',idum)
      CALL GLK_hinbo1(id,title,nchx,dxl,dxu)
* Header
      kax=1200
      kay=1200
      IF(CH1 .EQ. 'S') THEN
* Superimpose plot
        incr=incr+1
        BACKSPACE(m_ltx)
        BACKSPACE(m_ltx)
      ELSE
* New frame only
        incr=1
        CHR=CH1
        CALL GLK_PlFrame(id,kax,kay,chxfmt,chyfmt)
* The Y-range from first plot is preserved
        CALL GLK_Range1(id,yl,yu)
      ENDIF
* The X-range as in histo
      xl = dxl
      xu = dxu
*
      chrp= chrp1//chrx(incr)
      WRITE(m_ltx,'(A)')    '%=GLK_Plot2:  next plot (line) =========='
      WRITE(m_ltx,'(A,I10)')'%====HISTOGRAM ID=',ID
      WRITE(m_ltx,'(A,A70 )') '% ',TITLE
      CALL GLK_OptOut(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker = ioperb-1
* Default line type
      IF (iopsla .EQ. 2) THEN
         CHR='C'
      ELSE
         CHR=' '
      ENDIF
* User defined line-type
      IF (CH2 .EQ. 'B')   CHR=' '
*...marks in the midle of the bin
      IF (CH2 .EQ. '*')   CHR='*'
*...marks on the right edge of the bin
      IF (CH2 .EQ. 'R')   CHR='R'
*...marks on the left edge of the bin
      IF (CH2 .EQ. 'L')   CHR='L'
      IF (CH2 .EQ. 'C')   CHR='C'
*...various types of lines
      IF     (CHR .EQ. ' ') THEN
*...contour line used for histogram
          CALL GLK_PlKont(kax,kay,nchx,yl,yu,yy,ker,yer)
      ELSE IF(CHR .EQ. '*' .OR. CHR .EQ. 'R'.OR. CHR .EQ. 'L') THEN
*...marks on the right/left/midle of the bin
         CALL GLK_PlMark(kax,kay,nchx,yl,yu,yy,ker,yer,chmark,chr,chrp,chre)
      ELSE IF(CHR .EQ. 'C') THEN
*...slanted (dotted) line in plotting non-MC functions
          CALL GLK_PlCirc(kax,kay,nchx,yl,yu,yy)
      ENDIF
*------------------------------!
*        ENDing                !
*------------------------------!
      WRITE(m_ltx,'(2A)') m_BS,'end{picture} % close entire picture '
      IF(ABS(m_lint) .EQ. 2) THEN
         WRITE(m_ltx,'(A)') '%== GLK_Plot2:  end of plot  =========='
      ELSE
         WRITE(m_ltx,'(2A)') m_BS,'end{figure}'
      ENDIF
      RETURN
  900 CALL GLK_Stop1('+++GLK_Plot2: Nonexistig histo, skipped, id= ',ID)
      END

      SUBROUTINE GLK_PlFrame(id,kax,kay,chxfmt,chyfmt)
*     ************************************************
      IMPLICIT NONE
      INTEGER id,kax,kay
      CHARACTER chxfmt*(*),chyfmt*(*)
      INCLUDE 'GLK.fi'
      SAVE
*---------------------------------------------------
      CHARACTER*80 title
      DOUBLE PRECISION    dxl,dxu,xl,xu,yl,yu
      INTEGER  icont,i,nchx
      DATA icont/0/
*---------------------------------------------------
      icont=icont+1
      CALL GLK_hinbo1(id,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      CALL GLK_Range1(id,yl,yu)
*
      IF(icont .GT. 1) WRITE(m_ltx,'(2A)') m_BS,'newpage'
*------------------------------!
*           Header
*------------------------------!
      WRITE(m_ltx,'(A)') ' '
      WRITE(m_ltx,'(A)') ' '
      WRITE(m_ltx,'(A)')
     $'%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(m_ltx,'(A)')
     $'%%%%%%%%%%%%%%%%%%%%%%GLK_PlFrame%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      IF(ABS(m_lint) .EQ. 2) THEN
         WRITE(m_ltx,'(2A)') m_BS,'noindent'
      ELSE
         WRITE(m_ltx,'(2A)') m_BS,'begin{figure}[!ht]'
         WRITE(m_ltx,'(2A)') m_BS,'centering'
*         WRITE(m_ltx,'(2A)') m_BS,'htmlimage{scale=1.4}'
      ENDIF
*------------------------------!
* General Caption
*------------------------------!
      IF(ABS(m_lint) .NE. 2) THEN
         WRITE(m_ltx,'(6A)')
     $        m_BS,'caption{',m_BS,'footnotesize',m_BS,'sf'
         DO i=1,m_KeyTit
            WRITE(m_ltx,'(A)')     m_titch(i)
         ENDDO
         WRITE(m_ltx,'(A)') '}'
      ENDIF
*------------------------------!
* Frames and labels
*------------------------------!
      WRITE(m_ltx,'(A)') '% =========== big frame, title etc. ======='
      WRITE(m_ltx,'(4A)') m_BS,'setlength{',m_BS,'unitlength}{0.1mm}'
      WRITE(m_ltx,'(2A)') m_BS,'begin{picture}(1600,1500)'
      IF( m_lint .LT. 0) THEN
* Big frame usefull for debuging
         WRITE(m_ltx,'(4A)')
     $        m_BS,'put(0,0){',m_BS,'framebox(1600,1500){ }}'
      ENDIF
      WRITE(m_ltx,'(A)') '% =========== small frame, labeled axis ==='
      WRITE(m_ltx,'(4A,I4,A,I4,A)')
     $    m_BS,'put(300,250){',m_BS,'begin{picture}( ',kax,',',kay,')'
      WRITE(m_ltx,'(4A,I4,A,I4,A)')
     $    m_BS,'put(0,0){',m_BS,'framebox( ',kax,',',kay,'){ }}'
      WRITE(m_ltx,'(A)') '% =========== x and y axis ================'
      CALL GLK_AxisX(kax,xl,xu,chxfmt)
      CALL GLK_AxisY(kay,yl,yu,chyfmt)
      WRITE(m_ltx,'(3A)') m_BS,'end{picture}}'
     $                ,'% end of plotting labeled axis'
      END

      SUBROUTINE GLK_AxisX(kay,yl,yu,chxfmt)
*     ***************************************
* plotting x-axis with long and short tips
      IMPLICIT NONE
      INTEGER  kay
      DOUBLE PRECISION    yl,yu
      CHARACTER chxfmt*16
      INCLUDE 'GLK.fi'
      SAVE
*-------------------------------------------------------
      CHARACTER*64 fmt1,fmt2
      PARAMETER (fmt1 = '(2A,F8.2,A,F8.2,A,I4,3A)')
      PARAMETER (fmt2 = '(2A,F8.2,A,I4,A,F8.2,A,I4,3A)')
      DOUBLE PRECISION   dy,ddy,ddyl,yy0l,ddys,yy0s,p0s,pds,scmx,p0l,pdl
      INTEGER ly,jy,nlt,nts,lex,k,n
      DOUBLE PRECISION  tipsy(20)
*-------------------------------------------------------
      dy= ABS(yu-yl)
      ly = NINT( LOG10(dy) -0.4999999d0 )
      jy = NINT(dy/10d0**ly)
      ddyl = dy*10d0**(-ly)
      IF( jy .EQ. 1)                 ddyl = 10d0**ly*0.25d0
      IF( jy .GE. 2 .AND. jy .LE. 3) ddyl = 10d0**ly*0.5d0
      IF( jy .GE. 4 .AND. jy .LE. 6) ddyl = 10d0**ly*1.0d0
      IF( jy .GE. 7)                 ddyl = 10d0**ly*2.0d0
      WRITE(m_ltx,'(A)') '% -------GLK_AxisX---- '
      WRITE(m_ltx,'(A,I4)') '%  JY= ',JY
*-------
      nlt = INT(dy/ddyl)
      nlt = MAX0(MIN0(nlt,20),1)+1
      yy0l = NINT(yl/ddyl+0.5d0)*ddyl
      ddys = ddyl/10d0
      yy0s = NINT(yl/ddys+0.4999999d0)*ddys
      p0l = kay*(yy0l-yl)/(yu-yl)
      pdl = kay*ddyl/(yu-yl)
      p0s = kay*(yy0s-yl)/(yu-yl)
      pds = kay*ddys/(yu-yl)
      nlt = INT(ABS(yu-yy0l)/ddyl+0.0000001d0)+1
      nts = INT(abs(yu-yy0s)/ddys+0.0000001d0)+1
      DO n=1,nlt
         tipsy(n) =yy0l+ ddyl*(n-1)
      ENDDO
      WRITE(m_ltx,fmt1)
     $ m_BS,'multiput('  ,P0L,  ',0)('  ,PDL,  ',0){'  ,NLT,  '}{',
     $ m_BS,'line(0,1){25}}',
     $ m_BS,'multiput('  ,P0S,  ',0)('  ,PDS,  ',0){'  ,NTS,  '}{',
     $ m_BS,'line(0,1){10}}'
      WRITE(m_ltx,fmt2)
     $ m_BS,'multiput('  ,P0L,  ','  ,kay,  ')('  ,PDL,  ',0){'  ,NLT,
     $ '}{'  ,m_BS,  'line(0,-1){25}}',
     $ m_BS,'multiput('  ,P0S,  ','  ,kay,  ')('  ,PDS,  ',0){'  ,NTS,
     $ '}{'  ,m_BS,  'line(0,-1){10}}'
* ...labeling of axis
      scmx = DMAX1(DABS(yl),DABS(YU))
      lex  = NINT( LOG10(scmx) -0.50001)
      DO n=1,nlt
         k = nint(kay*(tipsy(n)-yl)/(yu-yl))
         IF(lex .LE. 3 .AND. lex .GE. -3) THEN
* ...without exponent
           WRITE(m_ltx,'(2A,I4,5A,'//chxfmt//',A)')
     $     m_BS,'put(',K,',-25){',m_BS,'makebox(0,0)[t]{',
     $           m_BS,'Large $ ', TIPSY(N), ' $}}'
         ELSE
* ...with exponent
           WRITE(m_ltx,'(2A,I4,5A,'//chxfmt//',2A,I4,A)')
     $     m_BS,'put('  ,K,  ',-25){',m_BS,'makebox(0,0)[t]{',
     $     m_BS,'Large $ ',
     $     TIPSY(N)/(10d0**LEX),m_BS,'cdot 10^{',LEX,'} $}}'
         ENDIF
      ENDDO
      END

      SUBROUTINE GLK_AxisY(kay,yl,yu,chyfmt)
*     ***************************************
* plotting y-axis with long and short tips
      IMPLICIT NONE
      INTEGER  kay
      DOUBLE PRECISION    yl,yu
      CHARACTER chyfmt*16
      INCLUDE 'GLK.fi'
      SAVE
      DOUBLE PRECISION  tipsy(20)
*------------------------------------------------------------------
      CHARACTER*64 fmt1,fmt2
      PARAMETER (fmt1 = '(2A,F8.2,A,F8.2,A,I4,3A)')
      PARAMETER (fmt2 = '(2A,I4,A,F8.2,A,F8.2,A,I4,3A)')
      INTEGER ly,jy,nlt,nts,lex,n,k
      DOUBLE PRECISION   ddyl,dy,yy0l,p0l,pdl,pds,scmx,z0l,p0s,yy0s,ddys
*------------------------------------------------------------------
      dy= ABS(yu-yl)
      ly = NINT( log10(dy) -0.49999999d0 )
      jy = NINT(dy/10d0**ly)
      ddyl = dy*10d0**(-ly)
      IF( jy .EQ. 1)                 ddyl = 10d0**ly*0.25d0
      IF( jy .GE. 2 .AND. jy .LE. 3) ddyl = 10d0**ly*0.5d0
      IF( jy .GE. 4 .AND. jy .LE. 6) ddyl = 10d0**ly*1.0d0
      IF( jy .GE. 7)                 ddyl = 10d0**ly*2.0d0
      WRITE(m_ltx,'(A)') '% --------GLK_SAxisY------- '
      WRITE(m_ltx,'(A,I4)') '%  JY= ',JY
*-------
      nlt = INT(dy/ddyl)
      nlt = MAX0(MIN0(nlt,20),1)+1
      yy0l = NINT(yl/ddyl+0.4999999d0)*ddyl
      ddys = ddyl/10d0
      yy0s = nint(yl/ddys+0.5d0)*ddys
      p0l = kay*(yy0l-yl)/(yu-yl)
      pdl = kay*ddyl/(yu-yl)
      p0s = kay*(yy0s-yl)/(yu-yl)
      pds = kay*ddys/(yu-yl)
      nlt= INT(ABS(yu-yy0l)/ddyl+0.0000001d0) +1
      nts= INT(ABS(yu-yy0s)/ddys+0.0000001d0) +1
      DO N=1,NLT
         tipsy(n) =yy0l+ ddyl*(n-1)
      ENDDO
* plotting tics on vertical axis
      WRITE(m_ltx,fmt1)
     $ m_BS,'multiput(0,'  ,P0L,  ')(0,'  ,PDL  ,'){'  ,NLT,  '}{', m_BS,'line(1,0){25}}',
     $ m_BS,'multiput(0,'  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,  '}{', m_BS,'line(1,0){10}}'
      WRITE(m_ltx,fmt2)
     $ m_BS,'multiput('  ,kay,  ','  ,P0L,  ')(0,'  ,PDL,  '){'  ,NLT,
     $ '}{',m_BS,'line(-1,0){25}}',
     $ m_BS,'multiput('  ,kay,  ','  ,P0S,  ')(0,'  ,PDS,  '){'  ,NTS,
     $ '}{',m_BS,'line(-1,0){10}}'
* ...Zero line if necessary
      Z0L = kay*(-YL)/(YU-YL)
      IF( (Z0L .GT. 0D0) .AND. (Z0L .LT. FLOAT(kay)) )
     $ WRITE(m_ltx,'(2A,F8.2,3A,I4,A)') m_BS,'put(0,'  ,Z0L,  '){',m_BS,'line(1,0){'  ,kay,  '}}'
* ...labeling of axis
      SCMX = DMAX1(DABS(YL),DABS(YU))
      LEX  = NINT( LOG10(SCMX) -0.50001d0)
      DO n=1,nlt
         k = nint(kay*(tipsy(n)-yl)/(yu-yl))
         IF(lex .LE. 3 .AND. lex .GE. -3) THEN
* ...without exponent
            WRITE(m_ltx,'(2A,I4,5A,'//chyfmt//',A)')
     $           m_BS,'put(-25,'  ,K,  '){',m_BS,'makebox(0,0)[r]{',
     $           m_BS,'Large $ '  ,TIPSY(N),  ' $}}'
         ELSE
* ...with exponent
            WRITE(m_ltx,'(2A,I4,5A,'//chyfmt//',2A,I4,A)')
     $           m_BS,'put(-25,'  ,K,  '){',m_BS,'makebox(0,0)[r]{',
     $           m_BS,'Large $ ',
     $           TIPSY(N)/(10d0**LEX),  m_BS,'cdot 10^{'  ,LEX,  '} $}}'
      ENDIF
      ENDDO
      END

      SUBROUTINE GLK_PlKont(kax,kay,nchx,yl,yu,yy,ker,yer)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//             Plotting contour line for histogram (formely PlHis)                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER kax,kay,nchx,ker
      DOUBLE PRECISION   yl, yu, yy(*),yer(*),z0l
      INCLUDE 'GLK.fi'
      SAVE
*---------------------------------------------------
      CHARACTER*80 fmt1
      INTEGER  ix0,iy0,ib,ix1,iy1,ie,ierr,ix2,idy,idx
      DOUBLE PRECISION    yib
*---------------------------------------------------
      WRITE(m_ltx,'(4A,I4,A,I4,A)') m_BS,'put(300,250){',m_BS,'begin{picture}( ',kax,',',kay,')'
      WRITE(m_ltx,'(A)') '% ========== plotting primitives =========='
* Color string, optionaly
      IF(m_KeyCol .EQ. 1) THEN
         WRITE(m_ltx,'(A)') m_Color
         m_KeyCol = 0
      ENDIF
*...short macros for vertical/horizontal straight lines
      WRITE(m_ltx,'(8A)')
     $     m_BS,'newcommand{',m_BS,'x}[3]{',m_BS,'put(#1,#2){', m_BS,'line(1,0){#3}}}'
      WRITE(m_ltx,'(8A)')
     $     m_BS,'newcommand{',m_BS,'y}[3]{',m_BS,'put(#1,#2){', m_BS,'line(0,1){#3}}}'
      WRITE(m_ltx,'(8A)')
     $     m_BS,'newcommand{',m_BS,'z}[3]{',m_BS,'put(#1,#2){', m_BS,'line(0,-1){#3}}}'
*   error bars
      WRITE(m_ltx,'(8A)')
     $     m_BS,'newcommand{',m_BS,'e}[3]{', m_BS,'put(#1,#2){',m_BS,'line(0,1){#3}}}'
* Starting point for the line
      ix0=0
      iy0=0
* Start at Zero line if possible
      z0l = kay*(-yl)/(yu-yl)
      IF( (z0l .GT. 0d0) .AND. (z0l .LT. FLOAT(kay)) )  iy0=z0l
      DO ib=1,nchx
         yib = yy(ib)
         ix1 = NINT(kax*(ib-0.00001d0)/nchx) ! new x
         iy1 = NINT(kay*(yib-yl)/(yu-yl))    ! new y
         iy1 = MIN(MAX(iy1,-1),kay+1)        ! cosmetics
         idx = ix1-ix0                       ! delta x
         idy = iy1-iy0                       ! delta y
         fmt1 = '(2(2a,i4,a,i4,a,i4,a))'
         IF(iy1 .GE. 0 .AND. iy1 .LE. kay) THEN
            IF( idy .GE. 0) THEN             ! up
               WRITE(m_ltx,fmt1) m_BS,'y{',ix0,'}{',iy0,'}{',idy,'}',
     $                           m_BS,'x{',ix0,'}{',iy1,'}{',idx,'}'
            ELSE                             ! down
               WRITE(m_ltx,fmt1) m_BS,'z{',IX0,'}{',IY0,'}{',-idy,'}',
     $                           m_BS,'x{',IX0,'}{',IY1,'}{',idx,'}'
            ENDIF
         ENDIF
         ix0=ix1
         iy0=iy1
         IF(ker .EQ. 1) THEN
            ix2  = NINT(kax*(ib-0.5000d0)/nchx)
            ierr = NINT(kay*((yy(ib)-yer(ib))-yl)/(yu-yl))  ! bottom of error bar
            ie = NINT(kay*yer(ib)/(yu-yl))                  ! total length of error bar
*        Cosmetics
            IF(ierr .LT. 0) THEN
               ie= ie+ierr
               ierr = 0
            ENDIF
            IF( (ierr+2*ie) .GT. kay) THEN
               ie= IABS(kay-ierr)/2
            ENDIF
            IF( (iy1.GE.0).AND.(iy1.LE. kay).AND.(ABS(1d0*ierr).LE.9999d0).AND.(2d0*ie.LE.9999d0) ) 
     $           WRITE(m_ltx,8000) m_BS,ix2,ierr,2*ie
         ENDIF
      ENDDO
8000  FORMAT(4(A1,2He{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(m_ltx,'(3A)') m_BS,'end{picture}}', ' % end of plotting histogram'
* change line-style
      m_tline= m_tline+1
      IF(m_tline .GT. 2) m_tline=1
      END

      SUBROUTINE GLK_PlMark(kax,kay,nchx,yl,yu,yy,ker,yer,chmark,chr,chr2,chr3)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                       marks in the midle of the bin                             //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     kax,kay,nchx,ker
      DOUBLE PRECISION       yl,yu, yy(*),yer(*)
      CHARACTER*1 chr
      CHARACTER   chmark*(*),chr2*(*),chr3*(*)
*---------------------------------------------------
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER    ib,ix1,iy1,ierr,ie
*---------------------------------------------------
      WRITE(m_ltx,'(4A,I4,A,I4,A)') m_BS,'put(300,250){',m_BS,'begin{picture}( ',kax,',',kay,')'
      WRITE(m_ltx,'(A)') '% ===GLK_PlMark: plotting primitives ======'
* Color string, optionaly
      IF(m_KeyCol .EQ. 1) THEN
         WRITE(m_ltx,'(A)') m_Color
         m_KeyCol = 0
      ENDIF
* Plotting symbol
      WRITE(m_ltx,'(10A)') m_BS,'newcommand{',m_BS,chr2  , '}[2]{', m_BS,'put(#1,#2){',chmark,'}}'
* Error bar symbol
      WRITE(m_ltx,'(10A)')
     $   m_BS,'newcommand{',m_BS,chr3  , '}[3]{', m_BS,'put(#1,#2){',m_BS,'line(0,1){#3}}}'

      DO ib=1,nchx
         IF(chr .EQ. '*') THEN
            ix1 = NINT(kax*(ib-0.5000d0)/nchx) ! Midle of bin
         ELSEIF(chr .EQ. 'R') THEN
            ix1 = NINT(kax*(ib*1d0)/nchx)      ! Right edge of bin
         ELSEIF(chr .EQ. 'L') THEN
            ix1 = NINT(kax*(ib-1d0)/nchx)      ! Left edge of bin
         ELSE
            WRITE(6,*) '+++++ plamark: wrong line type:',chr
            RETURN
         ENDIF
         iy1 = NINT(kay*(yy(ib)-yl)/(yu-yl))
         IF(iy1 .GE. 0 .AND. iy1 .LE. kay)
     $   WRITE(m_ltx,'(A,A,A,I4,A,I4,A)')
     $               m_BS,chr2, '{' ,IX1, '}{' ,IY1, '}'
         IF(ker .EQ. 1) THEN
            ierr = NINT(kay*((yy(ib)-yer(ib))-yl)/(yu-yl)) ! bottom of error bar
            ie   = NINT(kay*yer(ib)/(yu-yl))               ! total length of error bar
*        Cosmetics
            IF(ierr .LT. 0) THEN
               ie= ie+ierr
               ierr = 0
            ENDIF
            IF( (ierr+2*ie) .GT. kay) THEN
               ie= IABS(kay-ierr)/2
            ENDIF
            IF((iy1.GE.0) .AND.(iy1.LE.kay) .AND.(ABS(1d0*ierr).LE.9999d0) .AND.(2d0*ie.LE.9999d0))
     $      WRITE(m_ltx,'(A,A,A,I4,A,I5,A,I4,A)')
     $          m_BS, chr3,  '{'  ,IX1, '}{'  ,ierr, '}{'  ,2*ie,   '}'
         ENDIF
      ENDDO
      WRITE(m_ltx,'(3A)') m_BS,'end{picture}}',
     $    ' % end of plotting histogram'
      END


      SUBROUTINE GLK_PlTable(Npl,idl,capt,fmt,nch1,incr,npag)
*     ******************************************************
* Tables in TeX, up to 9 columns
* Npl           = numbers of columns/histograms
* idl(1:Npl)    = list of histo id's
* capt(1:Npl+1) = list of captions above each column
* fmt(1:1)      = format to print x(i) in first columb,
*                 h(i) and error he(i) in further columns
* nch1,incr     = raws are printet in the sequence
*                 (h(i),he(i),i=nch1,nbin,incr), nbin is no. of bins.
* npag          = 0 no page eject, =1 with page eject
*     ******************************************************
      IMPLICIT NONE
*--------------- parameters ------------
      INTEGER        Npl,idl(*),nch1,incr,npag
      CHARACTER*(*)  capt(*)
      CHARACTER*(*)  fmt(3)
*-------------------------------------------
      INCLUDE 'GLK.fi'
      SAVE
*---------------------------------------------------
      CHARACTER*16 fmt1,fmt2,fmt3
      LOGICAL GLK_Exist
      INTEGER   i,j,k,n,nchx,nplt,idum,id1,id
      INTEGER   iopsc1,ioperb,iopsla,iopsc2,ioplog
      DOUBLE PRECISION     xl,xu,dxl,dxu,xi
      DOUBLE PRECISION     yyy(m_MaxNb),yer(m_MaxNb),bi(m_MaxNb,9),er(m_MaxNb,9)
      CHARACTER*80 title
      CHARACTER*1 Cn(9)
      DATA Cn /'1','2','3','4','5','6','7','8','9'/
*-----------------------------------------------------------------------------
* Return if histo non-existing or to many columns
      IF(.NOT.GLK_EXIST(ID)) GOTO 900
      IF(Npl .GT. 9 )     GOTO 901
      fmt1 = fmt(1)
      fmt2 = fmt(2)
      fmt3 = fmt(3)
*
* npack histograms
      id1=idl(1)
      CALL GLK_hinbo1( id1,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      DO n=1,Npl
        CALL GLK_UnPak( idl(n),yyy ,'    ',idum)
        CALL GLK_UnPak( idl(n),yer ,'ERRO',idum)
        DO k=1,nchx
           bi(k,n)=yyy(k)
           er(k,n)=yer(k)
        ENDDO
      ENDDO
*------------------------------!
*           Header
*------------------------------!
      WRITE(m_ltx,'(A)') ' '
      WRITE(m_ltx,'(A)') ' '
      WRITE(m_ltx,'(A)') '% ========================================='
      WRITE(m_ltx,'(A)') '% ============= begin table ==============='
      WRITE(m_ltx,'(2A)') m_BS,'begin{table}[!ht]'
      WRITE(m_ltx,'(2A)') m_BS,'centering'
*------------------------------!
* Central Caption
*------------------------------!
      WRITE(m_ltx,'(4A)') m_BS,'caption{',m_BS,'small'
      DO i=1,m_KeyTit
        WRITE(m_ltx,'(A)')     m_titch(i)
      ENDDO
      WRITE(m_ltx,'(A)') '}'
*------------------------------!
* Tabular header
*------------------------------!
      WRITE(m_ltx,'(20A)') m_BS,'begin{tabular}
     $ {|',  ('|r',j=1,Npl+1),  '||}'
*
      WRITE(m_ltx,'(4A)') m_BS,'hline',m_BS,'hline'
*------------------------------!
* Captions in columns
*------------------------------!
      WRITE(m_ltx,'(2A)') capt(1),('&',capt(j+1),j=1,Npl)
*
      WRITE(m_ltx,'(2A)') m_BS,m_BS
      WRITE(m_ltx,'(2A)') m_BS,'hline'
*----------------------------------------!
* Table content
* Note that by default RIGHT EDGE of bin is printed, as necessary for
* cumulative distributions, this can be changed with SLAN option
*----------------------------------------!
      CALL GLK_OptOut(idl(1),ioplog,iopsla,ioperb,iopsc1,iopsc2)
      DO k=nch1,nchx,incr
        xi= dxl + (dxu-dxl)*k/(1d0*nchx)
        IF(iopsla.eq.2) xi= dxl + (dxu-dxl)*(k-0.5d0)/(1d0*nchx)
        IF(ioperb.eq.2) THEN
        WRITE(m_ltx,'(A,'//fmt1//','//Cn(Npl)//'(A,'//fmt2//',A,A,'//fmt3//'),  A)')
     $               '$', xi, ('$ & $', bi(k,j), m_BS, 'pm', er(k,j), j=1,Npl), '$'
        WRITE(m_ltx,'(2A)') m_BS,m_BS
        ELSE
        WRITE(m_ltx,'(A,'//fmt1//','//Cn(Npl)//'(A,'//fmt2//'),  A)')
     $               '$', xi, ('$ & $', bi(k,j), j=1,Npl), '$'
        WRITE(m_ltx,'(2A)') m_BS,m_BS
        ENDIF
      ENDDO
*------------------------------!
* Ending
*------------------------------!
      WRITE(m_ltx,'(4A)') m_BS,'hline',m_BS,'hline'
      WRITE(m_ltx,'(2A)') m_BS,'end{tabular}'
      WRITE(m_ltx,'(2A)') m_BS,'end{table}'
      WRITE(m_ltx,'(A)') '% ============= end   table ==============='
      WRITE(m_ltx,'(A)') '% ========================================='
      IF(npag .NE. 0) WRITE(m_ltx,'(2A)') m_BS,'newpage'

      RETURN
 900  CALL GLK_Retu1('++++ GLK_PlTable: Nonexistig histo id=',ID)
      RETURN
 901  CALL GLK_Retu1('++++ GLK_PlTable: To many columns Nplt=',Nplt)
      END

      SUBROUTINE GLK_PlTable2(Npl,idl,ccapt,mcapt,fmt,chr1,chr2,chr3)
*     ***************************************************************
* Tables in TeX, up to 9 columns
* Npl           = numbers of columns/histograms
* idl(1:Npl)    = list of histo id's
* ccapt(1:Npl+1)= list of column-captions above each column
* mcapt         = multicolumn header, none if mcapt=' ',
* fmt(1:1)      = format to print x(i) in first columb,
*                 h(i) and error he(i) in further columns
* chr1          = ' ' normal default, ='S' the Same table continued
* chr2          = ' ' midle of the bin for x(i) in the first column
*               = 'R' right edge,     ='L' left edge of the bin
* chr3          = ' ' no page eject,  ='E' with page eject at the end.
* Furthermore:
* Captions are defined by means of
*    CALL GLK_PlCapt(capt) before CALL GLK_PlTable2
*    where CHARACTER*80 capt(50) is content of
*    caption, line by line, see also comments in GLK_PlCapt routine.
*
*     ******************************************************
      IMPLICIT NONE
*-------------- parameters--------------
      INTEGER       Npl,idl(*)
      CHARACTER*(*) ccapt(*)
      CHARACTER*(*) fmt(3)
      CHARACTER*1   chr1,chr2,chr3
      CHARACTER*(*) mcapt
*----------------------------------------------------------------------
      INCLUDE 'GLK.fi'
      SAVE
*----------------------------------------------------------------------
      CHARACTER*16 fmt1,fmt2,fmt3
      LOGICAL GLK_Exist
      INTEGER   iopsc1,ioperb,iopsla,iopsc2,ioplog
      INTEGER   i,j,k,n,idum,id1,id,nchx,Nplt
      DOUBLE PRECISION     xl,xu,xi,dxu,dxl
      DOUBLE PRECISION     yyy(m_MaxNb),yer(m_MaxNb),bi(m_MaxNb,9),er(m_MaxNb,9)
      CHARACTER*80 title
      CHARACTER*1 Cn(9)
      CHARACTER*32 TabLab(11)        ! label in 1-st column of table
      INTEGER   k1,k2,k3,iraw
      DATA Cn /'1','2','3','4','5','6','7','8','9'/
*----------------------------------------------------------------------
* RETURN if histo non-existing or to many columns
      IF(.NOT.GLK_EXIST(ID)) GOTO 900
      IF(Npl .GT. 9 )     GOTO 901
      fmt1 = fmt(1)
      fmt2 = fmt(2)
      fmt3 = fmt(3)
*
* unpack histograms
      id1 = idl(1)
      CALL GLK_hinbo1( id1,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      DO n=1,Npl
         CALL GLK_UnPak( idl(n),yyy ,'    ',idum)
         CALL GLK_UnPak( idl(n),yer ,'ERRO',idum)
         DO k=1,nchx
            bi(k,n)=yyy(k)
            er(k,n)=yer(k)
         ENDDO
      ENDDO

      IF(chr1 .EQ. ' ' ) THEN
*------------------------------!
*           Header
*------------------------------!
         WRITE(m_ltx,'(A)') ' '
         WRITE(m_ltx,'(A)') ' '
         WRITE(m_ltx,'(A)') '% ========================================'
         WRITE(m_ltx,'(A)') '% ============ begin table ==============='
*
         IF(ABS(m_lint) .EQ. 2 ) THEN
            WRITE(m_ltx,'(2A)') m_BS,'noindent'
         ELSE
            WRITE(m_ltx,'(2A)') m_BS,'begin{table}[!ht]'
            WRITE(m_ltx,'(2A)') m_BS,'centering'
         ENDIF
*------------------------------!
* Central Caption
*------------------------------!
         IF(ABS(m_lint) .NE. 2 ) THEN
            WRITE(m_ltx,'(6A)')
     $           m_BS,'caption{',m_BS,'footnotesize',m_BS,'sf'
            DO i=1,m_KeyTit
               WRITE(m_ltx,'(A)')     m_titch(i)
            ENDDO
            WRITE(m_ltx,'(A)') '}'
         ENDIF
*------------------------------!
* Tabular header
*------------------------------!
         WRITE(m_ltx,'(20A)') m_BS,'begin{tabular}
     $        {|',  ('|r',j=1,Npl+1),  '||}'
         WRITE(m_ltx,'(4A)') m_BS,'hline',m_BS,'hline'
*------------------------------!
* Captions in columns
*------------------------------!
         WRITE(m_ltx,'(2A)') ccapt(1),('&',ccapt(j+1),j=1,Npl)
*------------------------------!
* Append previous table
*------------------------------!
      ELSEIF(chr1 .EQ. 'S' ) THEN
         DO i=1,7
            BACKSPACE(m_ltx)
         ENDDO
      ELSE
         WRITE(*,*) ' ++++ GLK_PlTable2: WRONG chr1 ' ,chr1
      ENDIF

      WRITE(m_ltx,'(2A)') m_BS,m_BS
      WRITE(m_ltx,'(2A)') m_BS,'hline'

*------------------------------!
* Optional multicolumn caption
*------------------------------!
      IF(mcapt .NE. ' ') THEN
         WRITE(m_ltx,'(3A,I2,A)') '& ',m_BS,'multicolumn{',Npl,'}{c||}{'
         WRITE(m_ltx,'(3A)') '     ',mcapt, ' }'
         WRITE(m_ltx,'(2A)') m_BS,m_BS
         WRITE(m_ltx,'(2A)') m_BS,'hline'
      ENDIF

*----------------------------------------!
* Table content
* Note that by default RIGHT EDGE of bin is printed, as necessary for
* cumulative distributions, this can be changed with SLAN option
*----------------------------------------!
      CALL GLK_OptOut(idl(1),ioplog,iopsla,ioperb,iopsc1,iopsc2)
*
* table printout can be controlled by  GLK_SetTabRan(i1,i2,i3)
      k1=1
      k2=nchx
      k3=1
      IF( m_KeyTbr .EQ. 1 ) THEN
         k1 = MAX(k1,m_TabRan(1))
         k2 = MIN(k2,m_TabRan(2))
         k3 = MAX(k3,m_TabRan(3))
         m_KeyTbr = 0
      ENDIF
      IF( m_KeyTbl .EQ. 0 ) THEN
* In the FIRST COLUMN the position of the bin edge/midle is ploted
         DO k=k1,k2,k3
            IF(chr2 .EQ. 'R') THEN
               xi= dxl + (dxu-dxl)*k/(1d0*nchx)         ! right
            ELSEIF(chr2 .EQ. 'L') THEN
               xi= dxl + (dxu-dxl)*(k-1d0)/(1d0*nchx)   ! left
            ELSE
               xi= dxl + (dxu-dxl)*(k-0.5d0)/(1d0*nchx) ! middle
            ENDIF
            IF(ioperb.eq.2) THEN
               WRITE(m_ltx,'(A,'//fmt1//','//Cn(Npl)//'(A,'//fmt2//',A,A,'//fmt3//'),  A)') !
     $                       '$', xi, ('$ & $', bi(k,j), m_BS, 'pm', er(k,j), j=1,Npl), '$' !
               WRITE(m_ltx,'(2A)') m_BS,m_BS
            ELSE
               WRITE(m_ltx,'(A,'//fmt1//','//Cn(Npl)//'(A,'//fmt2//'),  A)') !
     $                      '$', xi, ('$ & $', bi(k,j), j=1,Npl), '$'        !
               WRITE(m_ltx,'(2A)') m_BS,m_BS
            ENDIF
         ENDDO
      ELSE
         m_KeyTbl = 0
         iraw=0
         DO k=k1,k2,k3
            iraw=iraw+1
            IF(ioperb.eq.2) THEN
               WRITE(m_ltx,'(A, '//Cn(Npl)//'(A,'//fmt2//',A,A,'//fmt3//',A) )' ) !
     $                  m_TabLab(iraw), (' & $', bi(k,j), m_BS, 'pm', er(k,j),'$', j=1,Npl) !
               WRITE(m_ltx,'(2A)') m_BS,m_BS
            ELSE
               WRITE(m_ltx,'(A, '//Cn(Npl)//'(A,'//fmt2//',A) )' ) !
     $                  m_TabLab(iraw), (' & $', bi(k,j),'$', j=1,Npl) !
               WRITE(m_ltx,'(2A)') m_BS,m_BS
            ENDIF
         ENDDO
      ENDIF
*------------------------------!
* Ending
*------------------------------!
      WRITE(m_ltx,'(4A)') m_BS,'hline',m_BS,'hline'
      WRITE(m_ltx,'(2A)') m_BS,'end{tabular}'
      IF(ABS(m_lint) .EQ. 2 ) THEN
         WRITE(m_ltx,'(A)') '% ========================================'
      ELSE
         WRITE(m_ltx,'(2A)') m_BS,'end{table}'
      ENDIF
      WRITE(m_ltx,'(A)') '% ============= end   table =============='
      WRITE(m_ltx,'(A)') '% ========================================'
      IF(chr3 .EQ. 'E') THEN
         WRITE(m_ltx,'(2A)') m_BS,'newpage'
      ELSE
         WRITE(m_ltx,'(A)') '% ========================================'
      ENDIF
      RETURN
 900  CALL GLK_Retu1(' ++++ GLK_PlTable2: Nonexistig histo,id= ',ID)
      RETURN
 901  CALL GLK_Retu1(' ++++ GLK_PlTable2: To many columns Nplt= ',Nplt)
      END


      SUBROUTINE GLK_WtMon(mode,id,par1,par2,par3)
*     ********************************************
* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* !!!!  It is now replaces by GKL_M package, see below  !!!
* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*
* Utility program for monitoring M.C. rejection weights.
* ---------------------------------------------------------
* It is backward compatible with WMONIT except:
*  (1) for id=-1 one  should call as follows:
*      GLK_WtMon(-1,id,0d0,1d0,1d0) or skip initialisation completely!
*  (2) maximum absolute weight is looked for,
*  (3) GLK_Print(-id) prints weight distribution, net profit!
*  (4) no restriction id<100 any more!
* ---------------------------------------------------------
* wt is weight, wtmax is maximum weight and rn is random number.
* IF(mode .EQ. -1) then
*          initalization if entry id,
*        - wtmax is maximum weight used for couting overweighted
*          other arguments are ignored
* ELSEIF(mode .EQ. 0) then
*          summing up weights etc. for a given event for entry id
*        - wt is current weight.
*        - wtmax is maximum weight used for couting overweighted
*          events with wt>wtmax.
*        - rn is random number used in rejection, it is used to
*          count no. of accepted (rn < wt/wtmax) and rejected
*          (wt > wt/wtmax) events,
*          if ro rejection then put rn=0d0.
* ELSEIF(mode .EQ. 1) THEN
*          in this mode wmonit repports on accumulated statistics
*        - averwt= average weight wt counting all event
*        - errela= relative error of averwt
*        - nevtot= total number of accounted events
*        - nevacc= no. of accepted events (rn < wt/wtmax)
*        - nevneg= no. of events with negative weight (wt < 0)
*        - nevzer= no. of events with zero weight (wt = 0d0)
*        - nevove= no. of overweghted events (wt > wtmax)
*          and if you do not want to use cmonit then the value
*          the value of averwt is assigned to wt,
*          the value of errela is assigned to wtmax and
*          the value of wtmax  is assigned to rn in this mode.
* ELSEIF(mode .EQ. 2) THEN
*          all information defined for entry id defined above
*          for mode=2 is just printed of unit nout
* ENDIF
* note that output repport (mode=1,2) is done dynamically just for a
* given entry id only and it may be repeated many times for one id and
* for various id's as well.
*     ************************
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER           mode,id
      DOUBLE PRECISION  par1,par2,par3
* locals
      INTEGER           idg,nevneg,nevzer,nevtot,nevove,nevacc,nbin,lact,ist3,ntot,ist,ist2
      DOUBLE PRECISION  xl,xu,errela,sswt,averwt,wwmax,swt,wt,wtmax,rn
*---------------------------------------------------------------------------
      idg = -id
      IF(id .LE. 0) THEN
           CALL GLK_Stop1(' =====> GLK_WtMon: wrong id= ',id)
      ENDIF
      IF(mode .EQ. -1) THEN
*     *******************
           nbin = nint(dabs(par3))
           IF(nbin .GT. 100) nbin =100
           IF(nbin .EQ. 0)   nbin =1
           xl   =  par1
           xu   =  par2
           IF(xu .LE. xl) THEN
             xl = 0d0
             xu = 1d0
           ENDIF
           CALL GLK_hadres(idg,lact)
           IF(lact .EQ. 0) THEN
              CALL GLK_Book1(idg,' GLK_WtMon $',nbin,xl,xu)
           ELSE
              WRITE(m_out,*) ' WARNING GLK_WtMon: exists, id= ',id
              WRITE(    6,*) ' WARNING GLK_WtMon: exists, id= ',id
           ENDIF
      ELSEIF(mode .EQ. 0) THEN
*     **********************
         CALL GLK_hadres(idg,lact)
           IF(lact .EQ. 0) THEN
              WRITE(m_out,*) ' *****> GLK_WtMon: uninitialized, id= ',id
              WRITE(    6,*) ' *****> GLK_WtMon: uninitialized, id= ',id
              CALL GLK_Book1(idg,' GLK_WtMon $',1,0d0,1d0)
              CALL GLK_hadres(idg,lact)
           ENDIF
           wt   =par1
           wtmax=par2
           rn   =par3
*     standard entries
           CALL GLK_Fil1(idg,wt,1d0)  !!!! <-- principal filling!!!!
*     additional goodies
           ist  = m_index(lact,2)
           ist2 = ist+7
           ist3 = ist+11
*    maximum weight -- maximum by absolute value but keeping sign
           m_b(ist3+13)    = max( dabs(m_b(ist3+13)) ,dabs(wt))
           IF(wt .NE. 0d0) m_b(ist3+13)=m_b(ist3+13) *wt/dabs(wt)
*    nevzer,nevove,nevacc
           IF(wt .EQ. 0d0)        m_b(ist3+10) =m_b(ist3+10) +1d0
           IF(wt .GT. wtmax)      m_b(ist3+11) =m_b(ist3+11) +1d0
           IF(rn*wtmax .LE. wt)   m_b(ist3+12) =m_b(ist3+12) +1d0
      ELSEIF(mode .GE. 1 .OR. mode .LE. 10) THEN
*     *************************************
         CALL GLK_hadres(idg,lact)
           IF(lact .EQ. 0) THEN
              CALL GLK_Stop1(' lack of initialization, id=',id)
           ENDIF
           ist    = m_index(lact,2)
           ist2   = ist+7
           ist3   = ist+11
           ntot   = nint(m_b(ist3 +7))
           swt    =      m_b(ist3 +8)
           sswt   =      m_b(ist3 +9)
           IF(ntot.LE.0 .OR. swt.EQ.0d0 )  THEN
              averwt=0d0
              errela=0d0
           ELSE
              averwt=swt/float(ntot)
              errela=sqrt(abs(sswt/swt**2-1d0/float(ntot)))
           ENDIF
           nevneg = m_b(ist3  +1) !!! it us underflow, xlow=0 assumed!!!
           nevzer = m_b(ist3 +10)
           nevove = m_b(ist3 +11)
           nevacc = m_b(ist3 +12)
           wwmax  = m_b(ist3 +13)
           nevtot = ntot
* Output through parameters
           par1   = averwt
           par2   = errela
           par3   = nevtot
           IF(mode .EQ. 2) THEN
              par1   = nevacc
              par2   = nevneg
              par3   = nevove
           ELSEIF(mode .EQ. 3) THEN
              par1   = nevneg
              par2   = wwmax
           ENDIF
*  no printout for mode <10
*  ************************
           IF(mode .LE. 9) RETURN
           WRITE(m_out,1003) id, averwt, errela, wwmax
           WRITE(m_out,1004) nevtot,nevacc,nevneg,nevove,nevzer
           IF(mode .LE. 10) RETURN
           CALL GLK_Print(idg)
      ELSE
*     ****
           CALL GLK_Stop1('+++GLK_WtMon: wrong mode=',mode)
      ENDIF
*     *****
 1003 FORMAT(
     $  ' ======================= GLK_WtMon ========================='
     $/,'   id           averwt         errela            wwmax'
     $/,    i5,           e17.7,         f15.9,           e17.7)
 1004 FORMAT(
     $  ' -----------------------------------------------------------'
     $/,'      nevtot      nevacc      nevneg      nevove      nevzer'
     $/,   5i12)
      END

      SUBROUTINE GLK_CumHis(IdGen,id1,id2)
*     ************************************
*///////////////////////////////////////////////////////////////////////////
*//   Cumulates histogram content starting from UNDERFLOW                 //
*//   and normalizes to the total x-section in NANOBARNS                  //
*//   IdGen is ID of special histogram written by M.C. generator itself   //
*//   id2. NE. id1 required!!!                                            //
*///////////////////////////////////////////////////////////////////////////
*     ***********************************
      IMPLICIT NONE
      INTEGER  IdGen,id1,id2
*----------------------------------------------------------------------
      INCLUDE 'GLK.fi'
      SAVE
*----------------------------------------------------------------------
      CHARACTER*80 TITLE
      DOUBLE PRECISION    X(m_MaxNb),ER(m_MaxNb)
      LOGICAL GLK_EXIST
      DOUBLE PRECISION    swt,sswt,xsec,errel,tmin,tmax
      DOUBLE PRECISION    xscrnb,ERela,WtSup
      INTEGER  i,nbt,nevt
      DOUBLE PRECISION    GLK_hi,GLK_hie
*----------------------------------------------------------------------
      IF (GLK_Exist(id2)) GOTO 900
*
      CALL GLK_MgetNtot(IdGen,nevt)
      CALL GLK_MgetAve( IdGen,xscrnb,ERela,WtSup)
*
      IF(nevt .EQ. 0) GOTO 901
      CALL GLK_hinbo1(id1,title,nbt,tmin,tmax)
      swt  = GLK_hi( id1,0)     ! UNDERFLOW
      sswt = GLK_hie(id1,0)**2  ! UNDERFLOW
      DO i=1,nbt
         swt   = swt + GLK_hi( id1,i)
         sswt  = sswt+ GLK_hie(id1,i)**2
* note NEVT in error calc. is for the entire sample related
* to the crude x-section XCRU including !!! zero weight events !!!!
         xsec  = 0d0
         errel = 0d0
         IF(swt .NE. 0d0 .AND. nevt .NE. 0) THEN
            xsec  = swt*(xscrnb/nevt)
            errel = SQRT(ABS(sswt/swt**2-1d0/FLOAT(nevt)))
         ENDIF
         x(i)  = xsec
         er(i) = xsec*errel
      ENDDO
*! store result in id2
      CALL GLK_Book1(id2,title,nbt,tmin,tmax)
      CALL GLK_Pak(  id2,x)
      CALL GLK_Pake( id2,er)
      CALL GLK_idopt(id2,'ERRO')
      RETURN
 900  WRITE(6,*) '+++++ CUMHIS: ID2 exixsts!!',ID2
      RETURN
 901  WRITE(6,*) '+++++ CUMHIS: EMPTY HISTO ID=',ID1
      END




      SUBROUTINE GLK_RenHst(chak,IdGen,id1,id2)
*     *****************************************
*///////////////////////////////////////////////////////////////////////////
*//   IdGen is ID of special histogram written by M.C. generator itself   //
*//   This routine RE-NORMALIZES to  NANOBARNS or to UNITY                //
*//   CHAK = 'NB  '    normal case [nb]                                   //
*//   CHAK = 'NB10'    log10 x-scale assumed [nb]                         //
*//   CHAK = 'UNIT'    normalization to unity                             //
*//   id2 .NE. id1 required !!!                                           //
*///////////////////////////////////////////////////////////////////////////
*     ***********************************
      IMPLICIT NONE
      CHARACTER*4 CHAK
      INTEGER     IdGen,id1,id2
*----------------------------------------------------------------------
      INCLUDE 'GLK.fi'
      SAVE
      CHARACTER*80 TITLE
      DOUBLE PRECISION        xscrnb,ERela,WtSup,tmin,tmax
      DOUBLE PRECISION        swt,fln10,fact
      INTEGER      i,nbt,nevt
      DOUBLE PRECISION    GLK_hi,GLK_hie
*----------------------------------------------------------------------
      IF( id2 .eq. id1) GOTO 900

      CALL GLK_MgetNtot(IdGen,nevt)
      CALL GLK_MgetAve( IdGen,xscrnb,ERela,WtSup)
*
      CALL GLK_hinbo1(id1,title,nbt,tmin,tmax)
      IF(     chak .EQ. 'NB  ') THEN
         fact = nbt*xscrnb/(nevt*(tmax-tmin))
         CALL GLK_Operat(id1,'+',id1,id2, fact, 0d0)
      ELSEIF( chak .EQ. 'NB10') THEN
         fln10 = log(10.)
         fact = nbt*xscrnb/(nevt*(tmax-tmin)*fln10)
         CALL GLK_Operat(id1,'+',id1,id2, fact, 0d0)
      ELSEIF( chak .EQ. 'UNIT') THEN
         swt  = GLK_hi(id1,0)
         DO i=1,nbt+1
            swt   = swt + GLK_hi(id1,i)
         ENDDO
         fact = nbt/((tmax-tmin))/swt
         CALL GLK_Operat(id1,'+',id1,id2, fact, 0d0)
      ELSEIF( chak .EQ. 'UN10') THEN
         swt  = GLK_hi(id1,0)
         DO i=1,nbt+1
            swt   = swt + GLK_hi(id1,i)
         ENDDO
         fact = nbt/((tmax-tmin)*log(10.))/swt
         CALL GLK_Operat(id1,'+',id1,id2, fact, 0d0)
      ELSEIF( chak .EQ. '    ') THEN
         CALL GLK_Operat(id1,'+',id1,id2, 1d0, 0d0)
      ELSE
         WRITE(6,*) '+++++ RENHST: wrong chak=',chak
      ENDIF
*
      RETURN
 900  WRITE(6,*) '+++++ RENHST: ID1=ID2=',ID1
      END


*///////////////////////////////////////////////////////////////////////
*//                New Weight Motoring ToolBox
*//               (replacement for WTmonit etc.)
*//
*//  The tool to monitor very precisely the average weigh
*//  and other features of the weight distribution.
*//  Note that in principle we are vitaly interested in three parts
*//  of the weight distribution:
*//     Underflow (-infty,0)
*//     Regular   (0, WTmax)
*//     Overflow  (WTmax,+infty)
*//  with special emphasis on events with exactly zero weight WT=0d0.
*//  Nevertheless, we split (0, WTmax) range into several bins
*//  in order to be able to visualise the weight distribution.
*//  (Using stardard tools for histogram)
*//
*//
*///////////////////////////////////////////////////////////////////////
      SUBROUTINE GLK_Mbook(idm,title,nnchx,WTmax)
*     ******************************************
*///////////////////////////////////////////////////////////////////////
*//
*//   Booking one entry. Note it is not an ordinary histogram!!!
*//   It works just like GLK_Book1 except that it
*//   has internaly negative id and x_minimum is always zero.
*//
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER idm
      CHARACTER*80 title
      DOUBLE PRECISION   WTmax
*
      LOGICAL GLK_Exist
      INTEGER j,id,nnchx,nchx,lact,lengt2,ist,ist2,ist3
      INTEGER iopsc1, iopsc2, ioperb, ioplog, iopsla
      INTEGER iflag1, iflag2
      INTEGER ityphi
      DOUBLE PRECISION   xl,xu,ddx
*-------------------------------------------------
      CALL GLK_Initialize
      id = -idm
      IF(GLK_Exist(id)) goto 900
      ist=m_length
      CALL GLK_hadres(0,lact)
* Check if there is a free entry in the m_index
      IF(lact .EQ. 0) CALL GLK_Stop1('GLK_Mbook: no space left,id= ',id)
      m_index(lact,1)=id
      m_index(lact,2)=m_length
      m_index(lact,3)=0
* ---------- limits 
      CALL GLK_Copch(title,m_titlc(lact))
      nchx =nnchx
      IF(nchx .GT. m_MaxNb)
     $     CALL GLK_Stop1(' GLK_Mbook: Too many bins ,id= ',id)
      xl   = 0d0
      xu   = WTmax
* ---------- title and bin content ----------
      lengt2 = m_length +2*nchx +m_buf1+1
      IF(lengt2 .GE. m_LenmB)
     $  CALL GLK_Stop1('GLK_Mbook:too litle storage, m_LenmB= ',m_LenmB)
*
      DO j=m_length+1,lengt2+1
         m_b(j) = 0d0
      ENDDO
      m_length=lengt2
*... default flags
      ioplog   = 1
      iopsla   = 1
      ioperb   = 1
      iopsc1   = 1
      iopsc2   = 1
      iflag1   =
     $ ioplog+10*iopsla+100*ioperb+1000*iopsc1+10000*iopsc2
      ityphi   = 3  !!!! <-- new type of histo !!!!
      iflag2   = ityphi
* examples of decoding flags
*      id       = nint(m_b(ist+2)-9d0-9d12)/10
*      iflag1   = nint(m_b(ist+3)-9d0-9d12)/10
*      ioplog = mod(iflag1,10)
*      iopsla = mod(iflag1,100)/10
*      ioperb = mod(iflag1,1000)/100
*      iopsc1 = mod(iflag1,10000)/1000
*      iopsc2 = mod(iflag1,100000)/10000
*      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
*      ityphi = mod(iflag2,10)
*--------- buffer -----------------
* header
      m_b(ist +1)  = 9999999999999d0
      m_b(ist +2)  = 9d12 +     id*10 +9d0
      m_b(ist +3)  = 9d12 + iflag1*10 +9d0
      m_b(ist +4)  = 9d12 + iflag2*10 +9d0
* dummy vertical scale
      m_b(ist +5)  =  -100d0
      m_b(ist +6)  =   100d0
* pointer used to speed up search of histogram address
      m_b(ist +7)  =   0d0
* information on binning
      ist2         = ist+7
      m_b(ist2 +1) = nchx
      m_b(ist2 +2) = xl
      m_b(ist2 +3) = xu
      ddx = xu-xl
      IF(ddx .EQ. 0d0)
     $     CALL GLK_Stop1(' GLK_Mbook:    xl=xu,              id= ',id)
      m_b(ist2 +4) = float(nchx)/ddx
*
* under/over-flow etc.
      ist3       = ist+11
      DO j=1,13
         m_b(ist3 +j)=0d0
      ENDDO
      RETURN
*----------------
 900  CALL GLK_Retu1(' WARNING GLK_Mbook: already exists id= ', id)
      END


      SUBROUTINE GLK_Mfill(idm,Wtm,rn)
*     ********************************
*///////////////////////////////////////////////////////////////////////
*//
*//    filling of M-subpackage entry
*//    simillar as fil1 for 1-dim histo but the storage for error
*//    is now used to store sum for 'partial averages' <wt-xlowedge>
*//
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER idm
      DOUBLE PRECISION  Wtm,rn
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER id
      INTEGER lact, ist, ist2, ist3, iflag2, nchx, ityphi
      INTEGER iposx1,ipose1, kposx1, kpose1, kx
      DOUBLE PRECISION   Wt, deltx, factx, xlowedge
      DOUBLE PRECISION   xu, xl, x1, wtmax
*---------------------------------
      id = -idm
      Wt = Wtm
      CALL GLK_hadres(id,lact)
* exit for non-existig histo
      IF(lact .EQ. 0)  
     $     CALL GLK_Stop1('+++GLK_Mfill: nonexisting id= ',id)

      ist  = m_index(lact,2)
      ist2 = ist+7
      ist3 = ist+11
* one-dim. histo only
      iflag2   = nint(m_b(ist+4)-9d0-9d12)/10
      ityphi   = mod(iflag2,10)
      IF(ityphi .NE. 3) CALL GLK_Stop1('+++GLK_Mfill: wrong id=  ',id)
      x1 =  Wt
      m_index(lact,3)=m_index(lact,3)+1
* for standard average of x=Wt and its error
      m_b(ist3 +7)  =m_b(ist3 +7)  +1
      m_b(ist3 +8)  =m_b(ist3 +8)  +x1
      m_b(ist3 +9)  =m_b(ist3 +9)  +x1*x1
* filling coordinates
      nchx  = m_b(ist2 +1)
      xl    = m_b(ist2 +2)      !!<--- It was set to zero in book!!!
      xu    = m_b(ist2 +3)
      WtMax = xu
      factx = m_b(ist2 +4)      ! (fact=nchx/(xu-xl)
      deltx = 1d0/factx
      IF(x1 .LT. xl) THEN
* (U)nderflow
         iposx1 = ist3 +1
         ipose1 = ist3 +4
         m_b(iposx1) = m_b(iposx1)  +1d0
         m_b(ipose1) = m_b(ipose1)  +Wt
      ELSEIF(x1 .GT. xu) THEN
* (O)verflow
         iposx1 = ist3 +3
         ipose1 = ist3 +6
         kposx1 = 0
         m_b(iposx1) = m_b(iposx1)  +1d0
         m_b(ipose1) = m_b(ipose1)  +(Wt- WtMax)
      ELSE
* All of (R)egular range (0,WtMax) in one bin
         iposx1 = ist3 +2
         ipose1 = ist3 +5
         m_b(iposx1) = m_b(iposx1)  +1d0
         m_b(ipose1) = m_b(ipose1)  +Wt
* (R)egular bin, the ACTUAL one
         kx = (x1-xl)*factx+1d0
         kx = MIN( MAX(kx,1) ,nchx)
         kposx1 = ist +m_buf1+kx
         kpose1 = ist +m_buf1+nchx+kx
         xlowedge = deltx*(kx-1)
         m_b(kposx1) = m_b(kposx1)  +1d0
         m_b(kpose1) = m_b(kpose1)  +(Wt-xlowedge)
      ENDIF
*--------------------------------
* Additional goodies:
* maximum weight -- maximum by absolute value but keeping sign
      m_b(ist3+13) = MAX( DABS(m_b(ist3+13)) ,DABS(wt))
      IF(wt .NE. 0d0) m_b(ist3+13)=m_b(ist3+13) *wt/dabs(wt)
* nevzer,nevove,nevacc
      IF(wt .EQ. 0d0)        m_b(ist3+10) =m_b(ist3+10) +1d0
      IF(wt .GT. wtmax)      m_b(ist3+11) =m_b(ist3+11) +1d0
      IF(rn*wtmax .LE. wt)   m_b(ist3+12) =m_b(ist3+12) +1d0
*---
      END   !GLK_Mfill


      SUBROUTINE GLK_MgetAll(idm,
     $     AveWt,ERela, WtSup, AvUnd, AvOve,
     $     Ntot,Nacc,Nneg,Nove,Nzer)
*     ***************************************************************
*///////////////////////////////////////////////////////////////////////
*//
*//   Get all statistics out
*//
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER idm
      DOUBLE PRECISION   AveWt,ERela, WtSup, AvUnd, AvOve
      INTEGER Ntot,Nacc,Nneg,Nove,Nzer
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER id,ist,ist2,ist3,lact
      DOUBLE PRECISION   swt,sswt
*--------------------
      id= -idm
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0)
     $     CALL GLK_Stop1('GLK_MgetAll:lack of initialization, id=',id)
      ist    = m_index(lact,2)
      ist2   = ist+7
      ist3   = ist+11
      Ntot   = nint(m_b(ist3 +7))
      swt    =      m_b(ist3 +8)
      sswt   =      m_b(ist3 +9)
      IF(Ntot.LE.0 .OR. swt.EQ.0d0 )  THEN
         AveWt=0d0
         ERela=0d0
      ELSE
         AveWt=swt/DFLOAT(Ntot)
         ERela=sqrt(abs(sswt/swt**2-1d0/float(Ntot)))
      ENDIF
      WtSup  = m_b(ist3 +13)
      AvUnd  = m_b(ist3 +4)/Ntot
      AvOve  = m_b(ist3 +6)/Ntot
      Nneg = m_b(ist3  +1)    ! NB. it is underflow
      Nzer = m_b(ist3 +10)
      Nove = m_b(ist3 +11)
      Nacc = m_b(ist3 +12)
*-----------------------------
*      WRITE(m_out,1003) idm, AveWt, ERela, WtSup
*      WRITE(m_out,1004) Ntot,Nacc,Nneg,Nove,Nzer
* 1003 FORMAT(
*     $  ' ======================= GLK_Mget =========================='
*     $/,'   id            AveWt          ERela            WtSup'
*     $/,    i5,           e17.7,         f15.9,           e17.7)
* 1004 FORMAT(
*     $  ' -----------------------------------------------------------'
*     $/,'        Ntot        Nacc        Nneg        Nove        Nzer'
*     $/,   5i12)
*------------------------------
      END

      SUBROUTINE GLK_MgetNtot(id,Ntot)
*///////////////////////////////////////////////////////////////////////
*//
*//   Get Ntotal only
*//
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER idm,id
      DOUBLE PRECISION   AveWt, ERela, WtSup, AvUnd, AvOve
      INTEGER Ntot, Nacc, Nneg, Nove, Nzer
*--------------------
      CALL GLK_MgetAll(id,
     $     AveWt,ERela, WtSup, AvUnd, AvOve,
     $     Ntot,Nacc,Nneg,Nove,Nzer)
      END

      SUBROUTINE GLK_MgetAve(id,AveWt,ERela,WtSup)
*///////////////////////////////////////////////////////////////////////
*//
*//   Get averages only and highest weight
*//
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER idm,id
      DOUBLE PRECISION   AveWt, ERela, WtSup, AvUnd, AvOve
      INTEGER Ntot, Nacc, Nneg, Nove, Nzer
*--------------------
      CALL GLK_MgetAll(id,
     $     AveWt,ERela, WtSup, AvUnd, AvOve,
     $     Ntot,Nacc,Nneg,Nove,Nzer)
      END

      SUBROUTINE GLK_Mprint(idm)
*///////////////////////////////////////////////////////////////////////
*//
*//   Printout
*//   Note that bin errors have now changed meaning
*//
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER idm,id
      id= -idm
      CALL GLK_Print(id)
      END

*//////////////////////////////////////////////////////////////////////////////
*//////////////////////////////////////////////////////////////////////////////
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Setters and Getters                                  //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////

      SUBROUTINE GLK_Clone1(id1,id2,title2)
*//////////////////////////////////////////////////////////////////////////////
*//   Clone 1-dimensional histo with onl bining and new title                //
*//////////////////////////////////////////////////////////////////////////////
      CHARACTER*80 title1, title2, title3
      INTEGER      i,nb
      DOUBLE PRECISION        xmin,xmax

      CALL GLK_hinbo1(id1,title1,nb,xmin,xmax)
      CALL GLK_Copch(title2,title3)
      CALL GLK_Book1(id2,title3,nb,xmin,xmax)

      END

      SUBROUTINE GLK_Ymimax(id,wmin,wmax)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER id
      DOUBLE PRECISION  wmin,wmax

      CALL GLK_Yminim(id,wmin)
      CALL GLK_Ymaxim(id,wmax)
      END


      SUBROUTINE GLK_PLset(ch,xx)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Old style seter, sets type of the ploting mark                         //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      CHARACTER*4 CH
      DOUBLE PRECISION  xx
      INCLUDE 'GLK.fi'
      SAVE
*----------------------------------
      IF(CH .EQ. 'DMOD') THEN
        m_tline = NINT(xx)
      ENDIF
      END

      SUBROUTINE GLK_SetNout(ilun)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Sets output unit number                                                //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      SAVE
      INTEGER ilun

      CALL GLK_Initialize
      m_out=ilun
      END

      SUBROUTINE GLK_GetYmin(id,ymin)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id
      DOUBLE PRECISION   ymin
      INTEGER lact,ist
*
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) RETURN
      ist= m_index(lact,2)
      ymin = m_b(ist+5)
      END

      SUBROUTINE GLK_GetYmax(id,ymax)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id
      DOUBLE PRECISION   ymax
      INTEGER lact,ist
*
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) RETURN
      ist= m_index(lact,2)
      ymax = m_b(ist+6)
      END

      SUBROUTINE GLK_SetYmin(id,ymin)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id
      DOUBLE PRECISION   ymin
      INTEGER lact,ist
*
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) RETURN
      ist= m_index(lact,2)
      m_b(ist+5) = ymin
      CALL GLK_idopt(id,'YMIN')
      END

      SUBROUTINE GLK_SetYmax(id,ymax)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id
      DOUBLE PRECISION   ymax
      INTEGER lact,ist
*
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) RETURN
      ist= m_index(lact,2)
      m_b(ist+6) = ymax
      CALL GLK_idopt(id,'YMAX')
      END


      SUBROUTINE GLK_GetYminYmax(id,ymin,ymax)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id
      DOUBLE PRECISION   ymin,ymax
*
      CALL GLK_GetYmin(id,ymin)
      CALL GLK_GetYmax(id,ymax)
      END

      SUBROUTINE GLK_SetYminYmax(id,ymin,ymax)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id
      DOUBLE PRECISION   ymin,ymax
*
      CALL GLK_SetYmin(id,ymin)
      CALL GLK_SetYmax(id,ymax)
      END

      SUBROUTINE GLK_CopyYmin(id1,id2)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id1,id2
      DOUBLE PRECISION   ymin
*
      CALL GLK_GetYmin(id1,ymin)
      CALL GLK_SetYmin(id2,ymin)
      END

      SUBROUTINE GLK_CopyYmax(id1,id2)
*//////////////////////////////////////////////////////////////////////////////
*//   Sets vertical scale                                                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER id1,id2
      DOUBLE PRECISION   ymax
*
      CALL GLK_GetYmax(id1,ymax)
      CALL GLK_SetYmax(id2,ymax)
      END

      SUBROUTINE GLK_SetColor(Color)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Sets output unit number                                                //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      CHARACTER*(*) Color
*
      CALL GLK_Copch(Color,m_Color)
*
      m_KeyCol = 1              !flag set up
      END

      SUBROUTINE GLK_SetTabRan(i1,i2,i3)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Sets table range for taple printout in GKL_PlTable2                    //
*//   i1,i2,i3 are lower limit, upper limit and increment                    //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER i1,i2,i3
*
      m_KeyTbr = 1              !flag set up
      m_TabRan(1) = i1
      m_TabRan(2) = i2
      m_TabRan(3) = i3
      END

      SUBROUTINE GLK_SetTabLab(Nent,TabLab)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Sets table range for taple printout in GKL_PlTable2                    //
*//   i1,i2,i3 are lower limit, upper limit and increment                    //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      CHARACTER*32 TabLab(*)
      INTEGER Nent,j
      m_KeyTbl = 1
      IF( Nent.LE.0 .OR. Nent.GT.m_MaxNlab ) THEN
         CALL GLK_Stop1('+++STOP in GLK_SetTabLab: wrong Nent=',Nent)
      ENDIF
      DO j=1,m_MaxNlab
         m_TabLab(j)=TabLab(j)
      ENDDO
      END

      SUBROUTINE GLK_GetNb(id,Nb)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GLK.fi'
      INTEGER  id,Nb
* local
      CHARACTER*80 title
      INTEGER lact,ist,ist2
*-------------
      CALL GLK_hadres(id,lact)
      IF(lact .EQ. 0) THEN
         CALL GLK_Stop1('+++STOP in GLK_GetNb: wrong id=',id)
      ENDIF
      ist    = m_index(lact,2)
      ist2   = ist+7
      Nb     = m_b(ist2 +1)
      END

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  GLK                                   //
*//////////////////////////////////////////////////////////////////////////////

