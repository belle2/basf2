*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                Pseudo-CLASS  MBrA (Multi-Brancher)                       //
*//                                                                          //
*//   Purpose: Manage general multi-branched Monte Carlo algorithm           //
*//                                                                          //
*//   MBrA is capable to:                                                    //
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
*//  MBrA_Initialize(Nout, m_idMBrA, Nbin, WTmax, Title)                     //
*//  MBrA_AddBranch(KF,Nbin,WTmax,Title)    add branch i with name KF        //
*//      Event generation:                                                   //
*//  MBrA_SetXSList(XsList)                 set list of Xs Crude, see above  //
*//  MBrA_GenKF(KF,Wt_KF)                   generate KF                      //
*//  MBrA_GetKF(KF)                         get generated KF                 //
*//  MBrA_Fill(Wt,Rn)                       fill statistics                  //
*//      Global finalization:                                                //
*//  MBrA_MgetAve(AverWt, ErRela, WtSup)    get average total weight         //
*//  MBrA_Print0                            small final printout             //
*//  MBrA_Print1                            detailed final printout          //
*//      Tools:                                                              //
*//  MBrA_GetWMList(Ntot,WMList)            get list of WMlist(i)=WTmax(i)   //
*//  MBrA_GetKFlist(Ntot,KFlist)            get list of KF names             //
*//  MBrA_Stoper(mesage,id)                 miscelaneous                     //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
      SUBROUTINE MBrA_Initialize(Nout,idMBR,Nbin,WTmax,Title)
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
      INCLUDE 'MBrA.fi'
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

      SUBROUTINE MBrA_AddBranch(KF,Nbin,WTmax,Title)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   Register one branch in the system                                      //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER KF,Nbin
      CHARACTER*(*) Title
      DOUBLE PRECISION  WTmax
      INCLUDE 'MBrA.fi'
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      DO i=1,m_Ntot
         IF(KF .EQ. m_KFlist(i)) CALL MBrA_Stoper(
     $        'MBrA_AddBranch: KF code already defined KF=',KF)
      ENDDO
*
      m_Ntot=m_Ntot+1
      IF(m_Ntot .GT. m_MaxBra) CALL MBrA_Stoper(
     $     'MBrA_AddBranch: Too many branches, MaxBra= ',m_MaxBra)
*-----------------------------------------------------------------------
      m_KFlist(m_Ntot)=KF
      m_WMList( m_Ntot)=WTmax
      CALL GLK_Mbook(m_idMBR+m_Ntot,Title, Nbin, 1d0)
      END                       ! MBrA_AddBranch

      SUBROUTINE MBrA_GetKFlist(Ntot,KFlist)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER Ntot, KFlist(*)
      INCLUDE 'MBrA.fi'
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      Ntot = m_Ntot
      DO i=1,m_Ntot
         KFlist(i) = m_KFlist(i)
      ENDDO
      END

      SUBROUTINE MBrA_GetWMList(Ntot,WMList)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER Ntot
      DOUBLE PRECISION   WMList(*)
      INCLUDE 'MBrA.fi'
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      Ntot = m_Ntot
      DO i=1,m_Ntot
         WMList(i) = m_WMList(i)
      ENDDO
      END

      SUBROUTINE MBrA_SetXSList(XsList)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrA.fi'
      DOUBLE PRECISION   XsList(*)
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      DO i=1,m_Ntot
         m_Xslist(i) = XsList(i)
      ENDDO
      END


      SUBROUTINE MBrA_GenKF(KF,Wt_KF)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Generate KF (and branch ID)                                             //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrA.fi'
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
      CALL MBrA_Stoper('MBrA_GenKF: unable to define KF !!! ',-1)
*-----------------------------------------------------------------------
 500  CONTINUE
      Wt_KF = 1d0/m_WMList(ID)  ! compensating weight
      m_KFlast = KF             ! memorize generated KFcode
      m_IDlast = ID             ! memorize generated ID
      RETURN
*
 900  CONTINUE
      WRITE(*,*) '+++ MBrA_GenKF: STOP, sum=0'
      STOP
      END ! MBrA_GenKF

      SUBROUTINE MBrA_GetKF(KF)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER ID,KF
      INCLUDE 'MBrA.fi'
      SAVE
*-----------------------------------------------------------------------
      KF = m_KFlast
      END

      SUBROUTINE MBrA_Fill(Wt,Rn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  Wt,Rn
      INCLUDE 'MBrA.fi'
      SAVE
*-----------------------------------------------------------------------
      CALL GLK_Mfill(m_idMBR, Wt, Rn)
      CALL GLK_Mfill(m_idMBR +m_IDlast, Wt,   Rn)
      END

      SUBROUTINE MBrA_GetXCrude(XCrude)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//   get total crude integral, for normalization purpose                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrA.fi'
      DOUBLE PRECISION   XCrude
      SAVE
      INTEGER i
*-----------------------------------------------------------------------
      XCrude=0d0
      DO i=1,m_Ntot
         XCrude= XCrude + m_Xslist(i)*m_WMlist(i)
      ENDDO
      END

      SUBROUTINE MBrA_MgetAve(AverWt, ErRela, WtSup)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      INCLUDE 'MBrA.fi'
      DOUBLE PRECISION          AverWt, ErRela, WtSup
      CALL GLK_MgetAve(m_idMBR, AverWt, ErRela, WtSup)
      END

      SUBROUTINE MBrA_Print0
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BXformat.fi'
      INCLUDE 'MBrA.fi'
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
      WRITE(m_out,bxtxt) ' MBrA: report on the main Weight '
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

      SUBROUTINE MBrA_Print1
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BXformat.fi'
      INCLUDE 'MBrA.fi'
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
      WRITE(m_out,'(a)') '            MBrA:    Detailed statistics for all branches    '
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

      END



      SUBROUTINE MBrA_Stoper(mesage,id)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'MBrA.fi'
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

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  MBrA                                  //
*//////////////////////////////////////////////////////////////////////////////
