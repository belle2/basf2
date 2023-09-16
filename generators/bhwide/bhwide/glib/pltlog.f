!==============================================================================
!==============================================================================
!= The package for plotting histograms in the logarithmic vertical scale      =
!= for histogramming/plotting library GLIBK version 1.20 of S. Jadach         =
!==============================================================================
!= Written by: Wieslaw Placzek                            CERN, November 1999 =
!= Last correction: 26.11.1999               by: WP                           =
!==============================================================================
!==============================================================================
!=          List of procedures,  non-user subprograms in brackets             =
!==============================================================================
!      SUBR/FUNC     1 PAR.  2 PAR.  3 PAR. 4 PAR. 5 PAR. 6 PAR. ...
!==============================================================================
!      GPlotLs         INT    CHR*1   CHR*1   INT   ----   ----
!     (LFram1Ls)       INT      INT     DBL   DBL    DBL    DBL
!     (sAxiYLs)        INT      INT     DBL   DBL   ----   ----
!     (PlHistLs)       INT      INT     INT   DBL    DBL    DBL  INT  DBL  DBL
!     (PlHis2Ls)       INT      INT     INT   DBL    DBL    DBL  INT  DBL  DBL
!      GPlot2Ls        INT    CHR*1   CHR*1 CHR*(*) CHR*4  CHR*4
!     (LFrameLs)       INT      INT     DBL   DBL    DBL    DBL CHR*1
!     (PlMarkLs)       INT      INT     INT   DBL    DBL    DBL  ...
!==============================================================================
!==============================================================================

      SUBROUTINE GPlotLs(id,ch1,ch2,kdum)
*     ***********************************
*======================================================================*
*= Plotting histograms in the logarithmic scale on the y-axis.        =*
*= Based on GPLOT from the glibk package by S. Jadach.                =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 23.11.1999     by: WP                           =*  
*======================================================================*
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
      DIMENSION yy(200),yer(200),yel(200),yeu(200)
      CHARACTER ch1,ch2,chr
      CHARACTER*80 title
      LOGICAL gexist
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
      DATA chr /' '/
!*
!* Return if histo non-existing
      IF (.NOT.gexist(id)) GOTO 900
!* Unpack histogram
      CALL gunpak(id,yy ,'    ',idum)
      CALL gunpak(id,yer,'ERRO',idum)
      CALL ginbo1(id,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      CALL grang1(id,yl,yu)
!* Min and max values of y log scale
      IF (yu.GT.0d0) THEN
         yu = LOG10(yu)
      ELSE
         WRITE(6,*)' LOG scale impossible for negative values: ymax = ',
     &             yu
         STOP
      ENDIF
      IF (yl.GT.0d0) THEN
         yl = LOG10(yl)
      ELSE
         yl = yu - 10d0
      ENDIF
      dlty = yu - yl
      IF (dlty.LE.0d0) THEN
         WRITE(6,*)' Wrong values of: ymin, ymax = ',yl,yu
         STOP
      ELSEIF (dlty.GT.20d0) THEN
         yl = yu - 20d0
      ENDIF
!* Transform y values and their errors into log scale 
      DO i = 1,nchx
         yval = yy(i)
         IF (yval.GT.0d0) THEN
            yy(i) = MAX( LOG10(yval), yl )
            yerr = yer(i)
            yerl = yval - yerr 
            IF (yerl.GT.0d0) THEN
               yel(i) = MAX( LOG10(yerl), yl )
            ELSE
               yel(i) = yl
            ENDIF
            yerl = yval + yerr
            yeu(i) = MIN( MAX(LOG10(yerl),yl), yu )
         ELSE
            yy(i)  = yl 
            yel(i) = yl
            yeu(i) = yl
         ENDIF
      ENDDO
      kax = 1200
      kay = 1200
      IF (ch1 .EQ. 'S') THEN
!* Superimpose plot
        BACKSPACE(nouh1)
        BACKSPACE(nouh1)
      ELSE
!* New frame only
        chr = ch1
        CALL LFram1Ls(kax,kay,xl,xu,yl,yu)
      ENDIF
      WRITE(nouh1,'(A)')    '%========== next plot (line) =========='
      WRITE(nouh1,'(A,I10)') '%==== HISTOGRAM ID=',id
      WRITE(nouh1,'(A,A70 )') '% ',title
!* Continuous line for functions
      CALL goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker = ioperb - 1
      IF (iopsla .EQ. 2)  chr='C'
!* Suppress GPlotLs assignments
      IF (ch2 .EQ. 'B')   chr=' '
      IF (ch2 .EQ. '*')   chr='*'
      IF (ch2 .EQ. 'C')   chr='C'
!* Various types of lines
      IF     (chr .EQ. ' ') THEN
!* Contour line used for histogram
          CALL PlHistLs(kax,kay,nchx,yl,yu,yy,ker,yel,yeu)
      ELSE IF(chr .EQ. '*') THEN
!* Marks in the midle of the bin
          CALL PlHis2Ls(kax,kay,nchx,yl,yu,yy,ker,yel,yeu)
      ELSE IF(chr .EQ. 'C') THEN
!* Slanted (dotted) line in plotting non-MC functions
          CALL PlCirc(kax,kay,nchx,yl,yu,yy)
      ENDIF
!------------------------------------------------------------------
!* Ending ...
      WRITE(NOUH1,'(2A)') bs,'end{picture} % close entire picture '
      WRITE(NOUH1,'(2A)') bs,'end{figure}'
      RETURN

  900 WRITE(*,*) ' ++++ GPlotLs: NONEXISTIG HISTO ' ,id
      END

      SUBROUTINE LFram1Ls(kax,kay,xl,xu,yl,yu)
*     ****************************************
*======================================================================*
*= Frame for the logarithmic scale on the y-axis.                     =*
*= Based on LFRAM1 from the glibk package by S. Jadach.               =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 23.11.1999     by: WP                           =*  
*======================================================================*
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
!----------------------------------
!* Titles and captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 titch
      CHARACTER*80 title
      DIMENSION tipsx(20)
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
      DATA icont /0/
!*
      icont = icont + 1
      IF (icont.GT.1) WRITE(nouh1,'(2A)') bs,'newpage'
!------------------------------!
!           Header
!------------------------------!
      WRITE(nouh1,'(A)') ' '
      WRITE(nouh1,'(A)') ' '
      WRITE(nouh1,'(A)') '%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     $%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(nouh1,'(A)') '%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     $%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(nouh1,'(2A)') bs,'begin{figure}[!ht]'
      WRITE(nouh1,'(2A)') bs,'centering'
!------------------------------!
! General Caption
!------------------------------!
      WRITE(nouh1,'(4A)') bs,'caption{',bs,'small'
      IF (keytit.EQ.0) THEN
        WRITE(nouh1,'(A)')     title
      ELSE
        WRITE(nouh1,'(A)')     titch(1)
      ENDIF
      WRITE(nouh1,'(A)') '}'
!------------------------------!
! Frames and labels
!------------------------------!
      WRITE(nouh1,'(A)') '% =========== big frame, title etc. ======='
      WRITE(nouh1,'(4A)') bs,'setlength{',bs,'unitlength}{0.1mm}'
      WRITE(nouh1,'(2A)') bs,'begin{picture}(1600,1500)'
      WRITE(nouh1,'(4A)') bs,'put(0,0){',bs,'framebox(1600,1500){ }}'
      WRITE(nouh1,'(A)') '% =========== small frame, labeled axis ==='
      WRITE(nouh1,'(4A,I4,A,I4,A)')
     $    bs,'put(300,250){',bs,'begin{picture}( ',kax,',',kay,')'
      WRITE(nouh1,'(4A,I4,A,I4,A)')
     $    bs,'put(0,0){',bs,'framebox( ',kax,',',kay,'){ }}'
      WRITE(nouh1,'(A)') '% =========== x and y axis ================'
      CALL sAxiX(kax,xl,xu,ntipx,tipsx)
      CALL sAxiYLs(kax,kay,yl,yu)
      WRITE(nouh1,'(3A)') bs,'end{picture}}'
     $                ,'% end of plotting labeled axis'
      END

      SUBROUTINE sAxiYLs(kax,kay,yl,yu)
*     *********************************
*======================================================================*
*= Plotting the logarithmic scale on the y-axis.                      =*
*= Based on SAXIY from the glibk package by S. Jadach.                =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 23.11.1999     by: WP                           =*  
*======================================================================*
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
!*
!* Find number of long ticks
      nll = NINT(yl + 0.49999999d0)
      nlu = NINT(yu - 0.49999999d0)
      nlt = nlu - nll + 1
      WRITE(nouh1,'(A)') '% .......sAxiYLs........ '
      WRITE(nouh1,'(A,I4)') '%  NLT= ',nlt
!* Positions of first and last long ticks
      pll = kay*(nll - yl)/(yu - yl)
      plu = kay*(nlu - yl)/(yu - yl)
!* Distance between two long ticks
      pdl = (plu - pll)/MAX(nlt-1,1)
!* Plotting long tics on vertical axis
      WRITE(nouh1,1000)
     $     bs,'multiput(0,'  ,pll,  ')(0,'  ,pdl  ,'){'  ,nlt,  '}{',
     $     bs,'line(1,0){25}}'
      WRITE(nouh1,1001)
     $     bs,'multiput('  ,kax,  ','  ,pll,  ')(0,'  ,pdl,  '){'  ,nlt,
     $     '}{',bs,'line(-1,0){25}}'
!* Plotting short tics on vertical axis (a bit more complicated)
      DO its = 2,9
         nst = nlt - 1
         psl = pll + pdl*LOG10(DBLE(its))
         pds = pdl
         psb = psl - pds
         IF (psb.GE.0) THEN
            psl = psb
            nst = nst + 1
         ENDIF
         pst = psl + nst*pds
         IF (pst.LE.kay) nst = nst + 1
         WRITE(nouh1,1000)
     $        bs,'multiput(0,'  ,psl,  ')(0,'  ,pds,  '){'  ,nst,  '}{',
     $        bs,'line(1,0){10}}'
         WRITE(nouh1,1001)
     $        bs,'multiput('  ,kax,  ','  ,psl,  ')(0,'  ,pds,  '){'  ,
     $        nst,'}{',bs,'line(-1,0){10}}'
      ENDDO
 1000 FORMAT(2A,F8.2,A,F8.2,A,I4,3A)
 1001 FORMAT(2A,I4,A,F8.2,A,F8.2,A,I4,3A)
!* Labeling of axis
      DO n = nll,nlu
         k = NINT(pll + (n-nll)*pdl)
         WRITE(nouh1,'(2A,I4,5A,I4,A)')
     $        bs,'put(-25,'  ,k,  '){',bs,'makebox(0,0)[r]{',
     $        bs,'large $ 10^{'  ,n,  '} $}}'
      ENDDO
      END

      SUBROUTINE PlHistLs(kax,kay,nchx,yl,yu,yy,ker,yel,yeu)
*     ******************************************************
*======================================================================*
*= Plotting contour line for histogram.                               =*
*= Based on PLHIST from the glibk package by S. Jadach.               =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 23.11.1999     by: WP                           =*  
*======================================================================*
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION yy(*),yel(*),yeu(*)
      CHARACTER*80 fmt1
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
      SAVE
!*
      WRITE(nouh1,'(4A,I4,A,I4,A)')
     $     bs,'put(300,250){',bs,'begin{picture}( ',kax,',',kay,')'
      WRITE(nouh1,'(A)') '% ========== plotting primitives =========='
!* Various types of line
      IF(iline .EQ. 1) THEN
         WRITE(nouh1,'(2A)') bs,'thicklines '
      ELSE
         WRITE(nouh1,'(2A)') bs,'thinlines '
      ENDIF
!* Short macros for vertical/horizontal straight lines
      WRITE(nouh1,'(8A)')
     $     bs,'newcommand{',bs,'x}[3]{',bs,'put(#1,#2){',
     $     bs,'line(1,0){#3}}}'
      WRITE(nouh1,'(8A)')
     $     bs,'newcommand{',bs,'y}[3]{',bs,'put(#1,#2){',
     $     bs,'line(0,1){#3}}}'
      WRITE(nouh1,'(8A)')
     $     bs,'newcommand{',BS,'z}[3]{',bs,'put(#1,#2){',
     $     bs,'line(0,-1){#3}}}'
!* Error bars (slightly different for LOG scale)
      WRITE(nouh1,'(8A)')
     $     bs,'newcommand{',bs,'e}[3]{',
     $     bs,'put(#1,#2){',bs,'line(0,1){#3}}}'
      ix0 = 0
      iy0 = 0
      DO ib = 1,nchx
         ix1 = NINT( kax*(ib-0.00001d0)/nchx )
         iy1 = NINT( kay*(yy(ib)-yl)/(yu-yl) ) 
         idy = iy1-iy0
         idx = ix1-ix0
         fmt1 = '(2(2A,I4,A,I4,A,I4,A))'
         IF (idy .GE. 0) THEN  
            IF (iy1.GE.0 .AND. iy1.LE.kay)
     $           WRITE(nouh1,fmt1) bs,'y{',ix0,'}{',iy0,'}{',idy,'}',
     $           bs,'x{',ix0,'}{',iy1,'}{',idx,'}'
         ELSE
            IF (iy1.GE.0 .AND. iy1.LE.kay)
     $           WRITE(nouh1,fmt1) bs,'z{',ix0,'}{',iy0,'}{',-idy,'}',
     $           bs,'x{',ix0,'}{',iy1,'}{',idx,'}'
         ENDIF
         ix0 = ix1
         iy0 = iy1
         IF (ker.EQ.1) THEN
            ix2  = NINT(kax*(ib-0.5d0)/nchx)
            ierr = NINT( kay*(yel(ib) - yl)/(yu-yl) )
            ie   = NINT( kay*(yeu(ib) - yel(ib))/(yu-yl) )
            IF(iy1.GE.0 .AND. iy1.LE.kay .AND. ABS(ierr).LE.9999
     $           .AND. ie.LE.9999) WRITE(nouh1,8000) bs,ix2,ierr,ie  
         ENDIF
      ENDDO   
 8000 FORMAT(4(A1,2He{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(nouh1,'(3A)') bs,'end{picture}}',
     $       ' % end of plotting histogram'
!* Change line-style
      iline = iline + 1
      IF (iline.GT.2) iline = 1
      END

      SUBROUTINE PlHis2Ls(kax,kay,nchx,yl,yu,yy,ker,yel,yeu)
*     ******************************************************
*======================================================================*
*= Plotting marks in the midle if the bin for histogram.              =*
*= Based on PLHIS2 from the glibk package by S. Jadach.               =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 23.11.1999     by: WP                           =*  
*======================================================================*
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION yy(*),yel(*),yeu(*)
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
      SAVE 
!*
      WRITE(nouh1,'(4A,I4,A,I4,A)')
     $     bs,'put(300,250){',bs,'begin{picture}( ',kax,',',kay,')'
      WRITE(nouh1,'(A)') '% ========== plotting primitives =========='
!* Various types of mark
      irad1 = 6
      irad2 =10
      IF (iline.EQ.1) THEN
!*- Small filled circle
       WRITE(nouh1,'(8A,I3,A)')
     $        bs,'newcommand{',bs,'R}[2]{',
     $        bs,'put(#1,#2){',bs,'circle*{',irad1,'}}}'
      ELSEIF (iline.EQ.2) THEN
!*- Small open circle
       WRITE(NOUH1,'(8A,I3,A)')
     $        bs,'newcommand{',bs,'R}[2]{',
     $        bs,'put(#1,#2){',bs,'circle{',irad1,'}}}'
      ELSEIF (iline.EQ.3) THEN
!*- Big filled circle
       WRITE(nouh1,'(8A,I3,A)')
     $        bs,'newcommand{',bs,'R}[2]{',
     $        bs,'put(#1,#2){',bs,'circle*{',irad2,'}}}'
      ELSEIF (iline.EQ.4) THEN
!*- Big open circle
       WRITE(nouh1,'(8A,I3,A)')
     $        bs,'newcommand{',bs,'R}[2]{',
     $        bs,'put(#1,#2){',bs,'circle{',irad2,'}}}'
!*- Other symbols
      ELSEIF (iline.EQ.5) THEN
       WRITE(nouh1,'(10A)')
     $   bs,'newcommand{',bs,'R}[2]{',
     $   bs,'put(#1,#2){',bs,'makebox(0,0){$',bs,'diamond$}}}'
      ELSE
       WRITE(nouh1,'(10A)')
     $        bs,'newcommand{',bs,'R}[2]{',
     $        bs,'put(#1,#2){',bs,'makebox(0,0){$',bs,'star$}}}'
      ENDIF
!*- Error bars
      WRITE(nouh1,'(8A)')
     $   bs,'newcommand{',bs,'E}[3]{',
     $   bs,'put(#1,#2){',bs,'line(0,1){#3}}}'
      DO ib = 1,nchx
         ix1 = NINT(kax*(ib-0.5d0)/nchx)
         iy1 = NINT(kay*(yy(ib)-yl)/(yu-yl))
         IF(iy1.GT.0 .AND. iy1.LE.kay) WRITE(nouh1,7000) bs,ix1,iy1
         IF(ker.EQ.1) THEN
            ierr = NINT( kay*(yel(ib) - yl)/(yu-yl) )
            ie   = NINT( kay*(yeu(ib) - yel(ib))/(yu-yl) )
            IF(iy1.GT.0 .AND. iy1.LE.kay .AND. ABS(ierr).LE.9999
     $           .AND. ie.LE.9999) WRITE(nouh1,8000) bs,ix1,ierr,ie   
         ENDIF
      ENDDO
 7000 FORMAT(4(A1,2HR{,I4,2H}{,I4,1H}:1X ))
 8000 FORMAT(4(A1,2HE{,I4,2H}{,I5,2H}{,I4,1H}:1X ))
      WRITE(nouh1,'(3A)') BS,'end{picture}}',
     $    ' % end of plotting histogram'
!* Change line-style
      iline = iline+1
      IF(iline.GT.6) iline = 1
      END

      SUBROUTINE GPlot2Ls(id,ch1,ch2,chmark,chxfmt,chdumm)
*     ****************************************************
*======================================================================*
*= Plotting histograms in the logarithmic scale on the y-axis.        =*
*= Based on GPLOT2 from the glibk package by S. Jadach.               =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 26.11.1999     by: WP                           =*  
*======================================================================*
!* New version, more user-friendly, of GPlotLs
!* INPUT:
!*    ID          histogram identifier
!*    ch1 = ' '   normal new plot
!*        = 'S'   impose new plot on previous one
!*    ch2 = ' '   ploting line default, contour
!*        = '*'   error bars in midle of the bin
!*        = 'R'   error bars at Right edge of the bin
!*        = 'L'   error bars at Left  edge of the bin
!*        = 'C'   slanted continuous smooth line
!*    chmark =    TeX symbol for ploting points
!*    chxfmt =    format (string) for labeling x-axis
!*    chdumm =    dummy character (string) parameter 
!*                (kept only for compatibility with GPLOT2) 
!*----------------------------------------------------------------------
!* NOTE: For LOG scale the format for y-axis label is generated       
!*       automatically and does not have to be specified by the user. 
!*        This is why the parameter CHYFMT of GPLOT2 is not needed here.  
!*----------------------------------------------------------------------
!* Furthermore:
!* Captions are defined by means of 
!*    CALL gplcapt(capt) before CALL gplot2
!*    where CHARACTER*64 capt(50) is content of 
!*    caption, line by line, see also comments in gplcapt routine.
!* Additional text as a TeX source text can be appended by means of
!*    CALL gplabel(lines) after CALL gplot2
!*    where CHARACTER*64 lines(50) is the TeX add-on.
!*    this is used to decorate plot with
!*    any kind marks, special labels and text on the plot.
!**********************************************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      CHARACTER ch1,ch2,chmark*(*)
      CHARACTER*(*) chxfmt,chdumm
      SAVE
      DIMENSION yy(200),yer(200),yel(200),yeu(200)
      CHARACTER*80 title
!---------------------------------------------------
!* This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
      LOGICAL gexist
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
      CHARACTER chr
!* TeX Names of the error-bar command and of the point-mark command
      CHARACTER*1 chre, chrp1
      PARAMETER ( chre = 'E', chrp1= 'R' )
      CHARACTER*2 chrp
!* TeX Name of the point-mark command
      CHARACTER*1 chrx(12)
      DATA chrx /'a','b','c','d','f','g','h','i','j','k','l','m'/
      DATA chr /' '/
!*
!* Return if histo non-existing
      IF (.NOT.gexist(id)) GOTO 900
!* Unpack histogram
      CALL gunpak(id,yy ,'    ',idum)
      CALL gunpak(id,yer,'ERRO',idum)
      CALL ginbo1(id,title,nchx,dxl,dxu)
      xl = dxl
      xu = dxu
      CALL grang1(id,yl,yu)
!* Min and max values of y log scale
      IF (yu.GT.0d0) THEN
         yu = LOG10(yu)
      ELSE
         WRITE(6,*)' LOG scale impossible for negative values: ymax = ',
     &             yu
         STOP
      ENDIF
      IF (yl.GT.0d0) THEN
         yl = LOG10(yl)
      ELSE
         yl = yu - 10d0
      ENDIF
      dlty = yu - yl
      IF (dlty.LE.0d0) THEN
         WRITE(6,*)' Wrong values of: ymin, ymax = ',yl,yu
         STOP
      ELSEIF (dlty.GT.20d0) THEN
         yl = yu - 20d0
      ENDIF
!* Transform y values and their errors into log scale 
      DO i = 1,nchx
         yval = yy(i)
         IF (yval.GT.0d0) THEN
            yy(i) = MAX( LOG10(yval), yl )
            yerr = yer(i)
            yerl = yval - yerr 
            IF (yerl.GT.0d0) THEN
               yel(i) = MAX( LOG10(yerl), yl )
            ELSE
               yel(i) = yl
            ENDIF
            yerl = yval + yerr
            yeu(i) = MIN( MAX(LOG10(yerl),yl), yu )
         ELSE
            yy(i)  = yl 
            yel(i) = yl
            yeu(i) = yl
         ENDIF
      ENDDO
      kax = 1200
      kay = 1200
      IF (ch1 .EQ. 'S') THEN
!* Superimpose plot
        incr = incr + 1
        BACKSPACE(nouh1)
        BACKSPACE(nouh1)
      ELSE
!* New frame only
        incr = 1
        chr  = ch1
        CALL LFrameLs(kax,kay,xl,xu,yl,yu,chxfmt)
      ENDIF
      chrp= chrp1//chrx(incr)
      WRITE(nouh1,'(A)')    '%========== next plot (line) =========='
      WRITE(nouh1,'(A,I10)') '%==== HISTOGRAM ID=',id
      WRITE(nouh1,'(A,A70 )') '% ',title
!* Continuous line for functions
      CALL goptou(id,ioplog,iopsla,ioperb,iopsc1,iopsc2)
      ker = ioperb - 1
!* Default line type
      IF (iopsla .EQ. 2) THEN 
         chr = 'C'
      ELSE
         chr = ' '
      ENDIF
!* User defined line-type
      IF (ch2 .EQ. 'B')   chr=' '
!* Marks in the midle of the bin
      IF (ch2 .EQ. '*')   chr='*'
!* Marks on the right edge of the bin
      IF (ch2 .EQ. 'R')   chr='R'
!* Marks on the left edge of the bin
      IF (ch2 .EQ. 'L')   chr='L'
      IF (ch2 .EQ. 'C')   chr='C'
!* Various types of lines
      IF (chr.EQ.' ') THEN
!* Contour line used for histogram
          CALL PlHistLs(kax,kay,nchx,yl,yu,yy,ker,yel,yeu)
      ELSE IF(chr.EQ.'*' .OR. chr.EQ.'R' .OR. chr.EQ.'L') THEN
!* Marks on the right/left/midle of the bin
         CALL PlMarkLs(kax,kay,nchx,yl,yu,yy,ker,yel,yeu,
     &                 chmark,chr,chrp,chre)
      ELSE IF(chr .EQ. 'C') THEN
!* Slanted (dotted) line in plotting non-MC functions
          CALL PlCirc(kax,kay,nchx,yl,yu,yy)
       ELSE
          WRITE(6,*)' ++++ GPlot2Ls: Wrong mark position label: ',ch2
       ENDIF
!------------------------------------------------------------------
!* Ending ...
      WRITE(nouh1,'(2A)') bs,'end{picture} % close entire picture '
      IF(ABS(lint) .EQ. 2) THEN
         WRITE(nouh1,'(A)') '%======= GPlot2Ls:  end of plot =========='
      ELSE
         WRITE(nouh1,'(2A)') bs,'end{figure}'
      ENDIF
      RETURN
  900 WRITE(6,*) ' ++++ GPlot2Ls: NONEXISTIG HISTO ' ,id
      END

      SUBROUTINE LFrameLs(kax,kay,xl,xu,yl,yu,chxfmt)
*     ***********************************************
*======================================================================*
*= Frame for the logarithmic scale on the y-axis.                     =*
*= Based on LFRAME from the glibk package by S. Jadach.               =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 24.11.1999     by: WP                           =*  
*======================================================================*
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      CHARACTER chxfmt*(*)
      SAVE
!---------------------------------------------------
!* This COMMON connects gplint, gplcap and gplend
      COMMON / clint / lint
!---------------------------------------------------
!----------------------------------
!* Titles ang captions for plot
      COMMON / lpltit / titch(50),keytit
      CHARACTER*64 titch
      CHARACTER*80 title
      DIMENSION tipsx(20)
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
      DATA icont /0/
!*
      icont = icont + 1
      IF (icont.GT.1) WRITE(nouh1,'(2A)') bs,'newpage'
!------------------------------!
!           Header
!------------------------------!
      WRITE(nouh1,'(A)') ' '
      WRITE(nouh1,'(A)') ' '
      WRITE(nouh1,'(A)') 
     $'%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      WRITE(nouh1,'(A)') 
     $'%%%%%%%%%%%%%%%%%%%%%%%%%%LFrameLs%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%'
      IF (ABS(lint) .EQ. 2) THEN
         WRITE(nouh1,'(2A)') bs,'noindent'
      ELSE
         WRITE(nouh1,'(2A)') bs,'begin{figure}[!ht]'
         WRITE(nouh1,'(2A)') bs,'centering'
         WRITE(nouh1,'(2A)') bs,'htmlimage{scale=1.4}'
      ENDIF
!------------------------------!
! General Caption
!------------------------------!
      IF (ABS(lint) .NE. 2) THEN
         WRITE(nouh1,'(6A)') bs,'caption{',bs,'footnotesize',bs,'sf'
         DO i = 1,keytit
            WRITE(nouh1,'(A)')     titch(i)
         ENDDO
         WRITE(nouh1,'(A)') '}'
      ENDIF
!------------------------------!
! Frames and labels
!------------------------------!
      WRITE(nouh1,'(A)') '% =========== big frame, title etc. ======='
      WRITE(nouh1,'(4A)') bs,'setlength{',bs,'unitlength}{0.1mm}'
      WRITE(nouh1,'(2A)') bs,'begin{picture}(1600,1500)'
      IF( lint .LT. 0) THEN
! Big frame usefull for debuging 
         WRITE(nouh1,'(4A)') bs,'put(0,0){',bs,'framebox(1600,1500){ }}'
      ENDIF
      WRITE(nouh1,'(A)') '% =========== small frame, labeled axis ==='
      WRITE(nouh1,'(4A,I4,A,I4,A)')
     $    bs,'put(300,250){',bs,'begin{picture}( ',kax,',',kay,')'
      WRITE(nouh1,'(4A,I4,A,I4,A)')
     $    bs,'put(0,0){',bs,'framebox( ',kax,',',kay,'){ }}'
      WRITE(nouh1,'(A)') '% =========== x and y axis ================'
      CALL AxisX(kax,xl,xu,chxfmt)
      CALL sAxiYLs(kax,kay,yl,yu)
      WRITE(nouh1,'(3A)') bs,'end{picture}}'
     $                ,'% end of plotting labeled axis'
      END

      SUBROUTINE PlMarkLs(kax,kay,nchx,yl,yu,yy,ker,yel,yeu,
     &                    chmark,chr,chr2,chr3)
*     ******************************************************
*======================================================================*
*= Plotting marks in the midle if the bin for histogram.              =*
*= Based on PLHIS2 from the glibk package by S. Jadach.               =*
*----------------------------------------------------------------------*
*= Written by: Wieslaw Placzek                    CERN, November 1999 =*
*= Last modification: 24.11.1999     by: WP                           =*  
*======================================================================*
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      SAVE 
      DIMENSION yy(*),yel(*),yeu(*)
      CHARACTER*1 chr
      CHARACTER chmark*(*),chr2*(*),chr3*(*)
      COMMON / lpldat / nouh1,nouh2,iline
      COMMON / bslash / bs
      CHARACTER*1 bs
!*
      WRITE(nouh1,'(4A,I4,A,I4,A)')
     $     bs,'put(300,250){',bs,'begin{picture}( ',kax,',',kay,')'
      WRITE(nouh1,'(A)') '% ========== plotting primitives =========='
!* Plotting symbol
      WRITE(nouh1,'(10A)')
     $   bs,'newcommand{',bs,chr2  , '}[2]{',
     $   bs,'put(#1,#2){',chmark,'}}'
!* Error bar symbol
      WRITE(nouh1,'(10A)')
     $   bs,'newcommand{',bs,chr3  , '}[3]{',
     $   bs,'put(#1,#2){',bs,'line(0,1){#3}}}'
!* Print marks and values
      DO ib = 1,nchx
         IF (chr .EQ. '*') THEN
            ix1 = NINT(kax*(ib-0.5d0)/nchx)    ! Midle of bin
         ELSEIF (chr .EQ. 'R') THEN
            ix1 = NINT(kax*(ib*1d0)/nchx)      ! Right edge of bin
         ELSEIF (chr .EQ. 'L') THEN
            ix1 = NINT(kax*(ib-1d0)/nchx)      ! Left edge of bin
         ELSE
            WRITE(6,*) '+++++ PlMarkLs: Wrong line type: ',chr
            RETURN
         ENDIF
         iy1 = NINT(kay*(yy(ib)-yl)/(yu-yl))
         IF(iy1.GT.0 .AND. iy1.LE.kay)
     $   WRITE(nouh1,'(A,A,A,I4,A,I4,A)') 
     $               bs,chr2, '{' ,ix1, '}{' ,iy1, '}'
         IF(ker.EQ.1) THEN
            ierr = NINT( kay*(yel(ib) - yl)/(yu-yl) )
            ie   = NINT( kay*(yeu(ib) - yel(ib))/(yu-yl) )
            IF(iy1.GT.0 .AND. iy1.LE.KAY .AND. ABS(ierr).LE.9999
     $         .AND. ie.LE.9999) WRITE(nouh1,'(A,A,A,I4,A,I5,A,I4,A)') 
     $           bs, chr3,  '{'  ,ix1, '}{'  ,ierr, '}{'  ,ie,   '}'
         ENDIF
      ENDDO
      WRITE(nouh1,'(3A)') BS,'end{picture}}',
     $    ' % end of plotting histogram'
      END
