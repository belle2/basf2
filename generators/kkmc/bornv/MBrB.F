*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                Pseudo-CLASS  MBrB (Multi-Brancher)                       //
*//                                                                          //
*//   Purpose: Manage general multi-branched Monte Carlo algorithm           //
*//                                                                          //
*//   MBrB is capable to:                                                    //
*//    -register arbitrary number of branches                                //
*//    -keep track of elaborate MC statistics for each branch                //
*//    -generate randomly branch index                                       //
*//    -prowide useful averages and counts at the end of the run             //
*//    -print out statistics at the ned of the run                           //
*//   It is using tools from GLK library for weight monitoring               //
*//                                                                          //
*//  Xslist(i) = list of crude xsections without WTmax used in GenKF,        //
*//  Probablitity of branch(i) = XsList(i)*WTmax(i)*Normalization            //
*//                                                                          //
*//  Entries:                                                                //
*//      Global initialization:                                              //
*//  MBrB_Initialize(Nout, m_idMBrA, Nbin, WTmax, Title)                     //
*//  MBrB_AddBranch(KF,Nbin,WTmax,Title)    add branch i with name KF        //
*//      Event generation:                                                   //
*//  MBrB_SetXSList(XsList)                 set list of Xs Crude, see above  //
*//  MBrB_GenKF(KF,Wt_KF)                   generate KF                      //
*//  MBrB_GetKF(KF)                         get generated KF                 //
*//  MBrB_Fill(Wt,Rn)                       fill statistics                  //
*//      Global finalization:                                                //
*//  MBrB_MgetAve(AverWt, ErRela, WtSup)    get average total weight         //
*//  MBrB_Print0                            small final printout             //
*//  MBrB_Print1                            detailed final printout          //
*//      Tools:                                                              //
*//  MBrB_GetWMList(Ntot,WMList)            get list of WMlist(i)=WTmax(i)   //
*//  MBrB_GetKFlist(Ntot,KFlist)            get list of KF names             //
*//  MBrB_Stoper(mesage,id)                 miscelaneous                     //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
      SUBROUTINE MBrB_Initialize(Nout,idMBR,Nbin,WTmax,Title)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Basic initialization:                                                  //
*//   Nout  = output unit number                                             //
*//   idMBR = pointer for histograming                                       //
*//   Nbin  = number of bins for main weight                                 //
*//   WTmax = maximum weight for main weight                                 //
*//   Title = title of the branch system/main weight                         //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER idMBR,Nout,Nbin
      CHARACTER*(*) Title
      DOUBLE PRECISION  WTmax
      INCLUDE 'MBrB.fi'
      SAVE
      INTEGER i
*---------------------------------
      m_out   = Nout
      m_idMBR = idMBR
      m_Ntot  = 0
      DO i=1,m_MaxBra
         m_KFlist(i) =0
         m_XSlist(i) =0d0
         m_WMList(i) =0d0
      ENDDO
      CALL GLK_Mbook(m_idMBR,Title, Nbin, WTmax)
      END

      SUBROUTINE MBrB_AddBranch(KF,Nbin,WTmax,Title)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Register one branch in the system                                      //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER KF,Nbin
      CHARACTER*(*) Title
      DOUBLE PRECISION  WTmax
      INCLUDE 'MBrB.fi'
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      DO i=1,m_Ntot
         IF(KF .EQ. m_KFlist(i)) CALL MBrB_Stoper('MBrB_AddBranch: KF code already defined KF=',KF)
      ENDDO
*
      m_Ntot=m_Ntot+1
      IF(m_Ntot .GT. m_MaxBra) CALL MBrB_Stoper(
     $     'MBrB_AddBranch: Too many branches, MaxBra= ',m_MaxBra)
*-----------------------------------------------------------------------
      m_KFlist(m_Ntot)=KF
      m_WMList( m_Ntot)=WTmax
      CALL GLK_Mbook(m_idMBR+m_Ntot,Title, Nbin, 50d0)
      END                       ! MBrB_AddBranch

      SUBROUTINE MBrB_GetKFlist(Ntot,KFlist)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER Ntot, KFlist(*)
      INCLUDE 'MBrB.fi'
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      Ntot = m_Ntot
      DO i=1,m_Ntot
         KFlist(i) = m_KFlist(i)
      ENDDO
      END

      SUBROUTINE MBrB_GetWMList(Ntot,WMList)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER Ntot
      DOUBLE PRECISION   WMList(*)
      INCLUDE 'MBrB.fi'
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      Ntot = m_Ntot
      DO i=1,m_Ntot
         WMList(i) = m_WMList(i)
      ENDDO
      END

      SUBROUTINE MBrB_SetXSList(XsList)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrB.fi'
      DOUBLE PRECISION   XsList(*)
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      DO i=1,m_Ntot
         m_Xslist(i) = XsList(i)
      ENDDO
      END


      SUBROUTINE MBrB_GenKF(KF,Wt_KF)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Generate KF (and branch ID)                                             //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrB.fi'
      SAVE
      INTEGER  KF,ID
      DOUBLE PRECISION    Wt_KF
      DOUBLE PRECISION    cumuKF(m_MaxBra)
      DOUBLE PRECISION    sum,rnumb
      REAL                rvec(10)
      INTEGER  i
*-----------------------------------------------------------------------
      Wt_KF = 1d0
      CALL PseuMar_MakeVec(rvec,1)
      rnumb = rvec(1)
*
      sum=0d0
      DO i=1,m_Ntot
         sum = sum +m_XsList(i)*m_WMList(i)
         cumuKF(i)= sum
      ENDDO
*
      IF(sum .EQ. 0d0 ) GOTO 900
      DO i=1,m_Ntot
         cumuKF(i)=cumuKF(i)/sum
      ENDDO
*
      DO i=1,m_Ntot
         IF(rnumb .LE. cumuKF(i)) THEN
            KF = m_KFList(i)
            ID = i
            GOTO 500
         ENDIF
      ENDDO
      CALL MBrB_Stoper('MBrA_GenKF: unable to define KF !!! ',-1)
*-----------------------------------------------------------------------
 500  CONTINUE
      Wt_KF = 1d0/m_WMList(ID)  ! compensating weight
      m_KFlast = KF             ! memorize generated KFcode
      m_IDlast = ID             ! memorize generated ID
      RETURN
*
 900  CONTINUE
      WRITE(*,*) '+++ MBrB_GenKF: STOP, sum=0'
      STOP
      END ! MBrB_GenKF

      SUBROUTINE MBrB_GetKF(KF)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER ID,KF
      INCLUDE 'MBrB.fi'
      SAVE
*-----------------------------------------------------------------------
      KF = m_KFlast
      END

      SUBROUTINE MBrB_Fill(Wt,Rn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  Wt,Rn
      INCLUDE 'MBrB.fi'
      SAVE
*-----------------------------------------------------------------------
      CALL GLK_Mfill(m_idMBR, Wt, Rn)
      CALL GLK_Mfill(m_idMBR +m_IDlast, Wt,   Rn)
      END

      SUBROUTINE MBrB_GetXCrude(XCrude)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   get total crude integral, for normalization purpose                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrB.fi'
      DOUBLE PRECISION   XCrude
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      XCrude=0d0
      DO i=1,m_Ntot
         XCrude= XCrude + m_Xslist(i)*m_WMlist(i)
      ENDDO
      END

      SUBROUTINE MBrB_MgetAve(AverWt, ErRela, WtSup)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      INCLUDE 'MBrB.fi'
      DOUBLE PRECISION          AverWt, ErRela, WtSup
      CALL GLK_MgetAve(m_idMBR, AverWt, ErRela, WtSup)
      END

      SUBROUTINE MBrB_Print0
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BXformat.fi'
      INCLUDE 'MBrB.fi'
      SAVE
      DOUBLE PRECISION    AveWt, ERela, WTsup, AvUnd, AvOve
      INTEGER  Ntot,Nacc,Nneg,Nove,Nzer
      DOUBLE PRECISION    ROverf, RUnder
      INTEGER  i
*-----------------------------------------------------------------------
      CALL GLK_Mprint( m_idMBR)
      CALL GLK_MgetAll(m_idMBR,
     $     AveWt, ERela, WtSup, AvUnd, AvOve,
     $     Ntot, Nacc, Nneg, Nove, Nzer)
      
      ROverf = AvOve/AveWt
      RUnder = AvUnd/AveWt

      WRITE(m_out,bxope)
      WRITE(m_out,bxtxt) ' MBrB: report on the main Weight '
      WRITE(m_out,bxl1i) Ntot,      'no of raw events   ','Ntot  ',' b1'
      WRITE(m_out,bxl1i) Nacc,      'accepted    events ','Nacc  ',' b2'
      WRITE(m_out,bxl1i) Nneg,      'wt<0        events ','Nneg  ',' b3'
      WRITE(m_out,bxl1i) Nove,      'wt>WTmax    events ','Nove  ',' b4'
      WRITE(m_out,bxl1f) WTsup ,    'WTsup, largest WT  ','WTsup ',' b5'
      WRITE(m_out,bxl1f) AvOve ,    '<Wt-WtMax>  Overfl.','AvOve ',' b6'
      WRITE(m_out,bxl1f) AvUnd ,    '<Wt> for Wt<0      ','AvUnd ',' b7'
      WRITE(m_out,bxl1f) ROverf,    'AvOve/<Wt>,WT>WtMax','ROverf',' b8'
      WRITE(m_out,bxl1f) RUnder,    'AvUnd/<Wt>,Wt<0    ','RUnder',' b9'
      WRITE(m_out,bxclo)
      END

      SUBROUTINE MBrB_Print1
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BXformat.fi'
      INCLUDE 'MBrB.fi'
      SAVE
      DOUBLE PRECISION  AveWt, ERela, WTsup, AvUnd, AvOve
      INTEGER  Ntot,Nacc,Nneg,Nove,Nzer
      INTEGER  i, KF
      DOUBLE PRECISION  Overf, Under
*-----------------------------------------------------------------------
      
***      DO i=1,m_Ntot
***         CALL GLK_Mprint( m_idMBR+i)
***      ENDDO

      WRITE(m_out,'(a)') ' '
      WRITE(m_out,'(2a)') '=============',
     $ '========================================================================================'
      WRITE(m_out,'(a)') '            MBrB:    Detailed statistics for all branches    '
      WRITE(m_out,'(2a)') '=============',
     $ '========================================================================================'

      WRITE(m_out,'(a4, 2a10,a10,2a10,2a11,3a7)') 
     $     'KF',
     $     'AveWt', 'ERela', 'WtSup', 'Wt<0', 'Wt>Wmax',
     $     'Ntot', 'Nacc', 'Nneg', 'Nove', 'Nzer'
*--------- chanel by chanel
      DO i= 1,m_Ntot
         KF=m_KFList(i)
         CALL GLK_MgetAll(m_idMBR+i,
     $        AveWt, ERela, WtSup, AvUnd, AvOve,
     $        Ntot, Nacc, Nneg, Nove, Nzer)
         Under = AvUnd/AveWt
         Overf = AvOve/AveWt
         WRITE(m_out,'(I4,2f10.6,g10.4,2f10.6,2i11,3i7)')
     $        KF,
     $        AveWt, ERela, WtSup, Under, Overf,
     $        Ntot, Nacc, Nneg, Nove, Nzer
      ENDDO
*-------- all chanels
      CALL GLK_MgetAll(m_idMBR,
     $     AveWt, ERela, WtSup, AvUnd, AvOve,
     $     Ntot, Nacc, Nneg, Nove, Nzer)
      Under = AvUnd/AveWt
      Overf = AvOve/AveWt
      WRITE(m_out,'(a4,2f10.6,g10.4,2f10.6,2i11,3i7)')
     $     'All:',
     $     AveWt, ERela, WtSup, Under, Overf,
     $     Ntot, Nacc, Nneg, Nove, Nzer

      WRITE(m_out,'(2a)')  '=============',
     $ '========================================================================================'

      DO i= 1,m_Ntot
***      CALL GLK_Print(   -(m_idMBR+i))
         CALL MBrB_WtLimit(-(m_idMBR+i))
      ENDDO

      END

      SUBROUTINE MBrB_Stoper(mesage,id)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrB.fi'
      SAVE
      CHARACTER*(*) mesage
      INTEGER id
*-----------------------------
      WRITE(m_out,'(a)')'++++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(m_out,'(a,a,i10,a)') '+++ ',   mesage, id,    ' +++'
      WRITE(m_out,'(a)') '+++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6    ,'(a)') '+++++++++++++++++++++++++++++++++++++++++++++'
      WRITE(6    ,'(a,a,i10,a)') '+++ ',   mesage, id,    ' +++'
      WRITE(6    ,'(a)') '+++++++++++++++++++++++++++++++++++++++++++++'
      STOP
      END

      SUBROUTINE MBrB_WtLimit(id)
*//////////////////////////////////////////////////////////////////////////////////
*// calculates wtmax for which overflow integral is below epsilon
*// the precision of the result is limited by beam size and statistics
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrB.fi'
      CHARACTER*80 title
      INTEGER           id,nchx
      DOUBLE PRECISION  xl,xu, Bin
      DOUBLE PRECISION  GLK_hi,sum,sumWt,WtLimit,tail,eps,AveWt,Wt
      INTEGER           ib,ibX
*     ------------------------------------------
      CALL GLK_hinbo1(id,title,nchx,xl,xu)
      eps=1d-4
      sum   = 0d0
      sumWt = 0d0
      DO ib=1,nchx
         Bin = GLK_hi(id,ib)
         Wt = xl+(ib-0.5d0)*(xu-xl)/nchx
         sum   = sum   +Bin
         sumWt = sumWt +Bin*Wt
      ENDDO
      AveWt = sumWt/sum

      DO ibX=nchx,1,-1
         WtLimit =xl+(ibX-0.5d0)*(xu-xl)/nchx
         tail=0d0
         DO ib=ibX,nchx
            Bin = GLK_hi(id,ib)
            Wt = xl+(ib-0.5d0)*(xu-xl)/nchx
            tail=tail+Bin*(Wt-WtLimit) ! deficit of xsection because Wt->WtLimit
         ENDDO
         IF(tail/sum .GT. eps) GOTO 100
      ENDDO
 100  CONTINUE
      WRITE(m_out,*) '-------------------------------------------------------------------------------'
      WRITE(m_out,*) '-->FindWtLimit: content, <Wt>,  WtLimit= ',sum,AveWt,WtLimit
      WRITE(m_out,*) '-->FindWtLimit: EFFICIENCY <Wt>/WtLimit= ',AveWt/WtLimit
      WRITE(    6,*) '-------------------------------------------------------------------------------'
      WRITE(    6,*) '-->FindWtLimit: content, <Wt>,  WtLimit= ',sum,AveWt,WtLimit
      WRITE(    6,*) '-->FindWtLimit: EFFICIENCY <Wt>/WtLimit= ',AveWt/WtLimit
      END                       !!!WtLimit

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  MBrB                                  //
*//////////////////////////////////////////////////////////////////////////////
