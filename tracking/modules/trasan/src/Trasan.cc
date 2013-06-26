//-----------------------------------------------------------------------------
// $Id: Trasan.cc 11152 2010-04-28 01:24:38Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : Trasan.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A tracking module.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// TTrack::appendStereo bug fixed,
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.231  2005/11/03 23:20:13  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.230  2005/04/18 23:41:47  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.229  2005/03/11 03:57:53  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.228  2005/01/14 00:55:40  katayama
// uninitialized variable
//
// Revision 1.227  2004/04/28 08:30:18  yiwasaki
// Trasan 3.14 : parameter doMCAnalysiss removed(automatical set up); Sakura 2.05 : protection for extra-tracks
//
// Revision 1.226  2004/04/26 06:29:12  yiwasaki
// Trasan 3.13 : TTrackManager MC analysis bug fixed
//
// Revision 1.225  2004/04/23 09:48:21  yiwasaki
// Trasan 3.12 : curlVersion=2 is default
//
// Revision 1.224  2004/04/15 05:37:27  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.223  2004/04/15 05:34:10  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.222  2004/04/08 22:02:48  yiwasaki
// MC quality check added
//
// Revision 1.221  2004/03/26 06:07:05  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.220  2004/02/18 04:07:27  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.219  2004/01/09 22:39:34  yiwasaki
// Trasan 3.08 : bug fix in curl version treatment
//
// Revision 1.218  2003/12/25 12:03:36  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.217  2003/12/19 07:36:03  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.216  2003/10/05 22:39:39  yiwasaki
// Trasan 3.05 : PMCurlFinder modification again : PMCurlFinder always off in exp>=30, default on in exp<30
//
// Revision 1.215  2003/10/05 21:27:36  yiwasaki
// PMCurlFinder turned off in default
//
// Revision 1.214  2003/09/15 22:53:53  yiwasaki
// Minor bug fix
//
// Revision 1.213  2003/09/10 01:18:31  yiwasaki
// Trasan for small cell CDC
//
// Revision 1.212  2002/04/18 02:15:58  yiwasaki
// minor bug fix
//
// Revision 1.211  2002/02/13 21:59:57  yiwasaki
// Trasan 3.03 : T0 reset test mode added
//
// Revision 1.210  2002/02/13 20:22:12  yiwasaki
// Trasan 3.02 : bug fixes in debug mode
//
// Revision 1.209  2001/12/23 09:58:50  katayama
// removed Strings.h
//
// Revision 1.208  2001/12/19 02:59:49  katayama
// Uss find,istring
//
// Revision 1.207  2001/06/18 00:16:37  yiwasaki
// Trasan 3.01 : perfect segment finder option, segv in term on linux fixed
//
// Revision 1.206  2001/04/27 01:28:42  yiwasaki
// Trasan 3.00 : official release, nothing changed from 3.00 RC6
//
// Revision 1.205  2001/04/25 02:36:00  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.204  2001/04/18 01:29:24  yiwasaki
// helix fitter speed up by jt
//
// Revision 1.203  2001/04/12 07:10:57  yiwasaki
// Trasan 3.00 RC4 : new stereo code for curl
//
// Revision 1.202  2001/04/11 23:20:24  yiwasaki
// Trasan 3.00 RC3 : a bug in stereo mode 1 and 2 is fixed
//
// Revision 1.201  2001/04/11 01:09:13  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.197  2001/02/07 22:25:04  yiwasaki
// Trasan 2.28 : conf minor change in shared memory
//
// Revision 1.196  2001/02/07 21:18:28  yiwasaki
// Trasan 2.27 : pivot of 2D track is moved to (x,y,0)
//
// Revision 1.195  2001/02/05 05:48:56  yiwasaki
// Trasan 2.26 : bug fix for data processing
//
// Revision 1.194  2001/02/01 06:15:50  yiwasaki
// Trasan 2.25 : conf bug fix for chisq=0
//
// Revision 1.193  2001/01/31 11:38:28  yiwasaki
// Trasan 2.24 : curl finder fix
//
// Revision 1.192  2001/01/30 22:40:13  yiwasaki
// Trasan 2.23 : curl fix
//
// Revision 1.191  2001/01/30 20:51:51  yiwasaki
// Trasan 2.22 : bug fixes
//
// Revision 1.190  2001/01/30 04:54:16  yiwasaki
// Trasan 2.21 release : bug fixes
//
// Revision 1.189  2001/01/29 09:27:47  yiwasaki
// Trasan 2.20 release : 2D & cosmic tracks are output, curler decision is made by dr and dz
//
// Revision 1.188  2001/01/11 04:40:24  yiwasaki
// minor changes
//
// Revision 1.187  2000/11/17 06:41:36  yiwasaki
// Trasan 2.18 : option for new CDC geometry
//
// Revision 1.186  2000/10/23 08:22:29  yiwasaki
// Trasan 2.17 : curl can output 2D tracks. Default parameters changed
//
// Revision 1.185  2000/10/13 11:23:16  yiwasaki
// Trasan 2.16 : PMF updates
//
// Revision 1.184  2000/10/13 00:28:08  yiwasaki
// Trasan 2.15 : trkmgr bug fix(YI)
//
// Revision 1.183  2000/10/12 21:48:51  yiwasaki
// Trasan 2.14 : trkmgr bug fix(YI) and updates(JT), new profiler for cdc hits
//
// Revision 1.182  2000/10/11 11:53:37  yiwasaki
// trasan 2.13 : PMF cdc seed off
//
// Revision 1.181  2000/10/11 11:30:46  yiwasaki
// trasan 2.12 : #hits protection, chi2 sorting modified
//
// Revision 1.180  2000/10/10 06:04:24  yiwasaki
// Trasan 2.11 : curl builder, helix fitter, perfect finder
//
// Revision 1.179  2000/10/07 01:44:52  yiwasaki
// Trasan 2.10 : updates of trkmgr from JT
//
// Revision 1.178  2000/10/05 23:54:28  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.177  2000/10/02 22:34:03  yiwasaki
// Trasan 2.08 : trkmgr update from JT
//
// Revision 1.176  2000/10/02 08:09:30  yiwasaki
// Trasan 2.07 : updates of pmf and trkmgr by JT
//
// Revision 1.175  2000/09/29 22:11:23  yiwasaki
// Trasan 2.06 : trkmgr updates from JT
//
// Revision 1.174  2000/09/27 07:45:20  yiwasaki
// Trasan 2.05 : updates of curl and pm finders by JT
//
// Revision 1.173  2000/08/31 23:51:48  yiwasaki
// Trasan 2.04 : pefect finder added
//
// Revision 1.172  2000/07/31 04:47:33  yiwasaki
// Minor changes
//
// Revision 1.171  2000/06/27 08:00:35  yiwasaki
// Trasan 2.03 : TTrackManager sorting bug fixed
//
// Revision 1.170  2000/06/13 05:19:03  yiwasaki
// Trasan 2.02 : RECCDC_MCTRK sorting
//
// Revision 1.169  2000/06/05 23:42:50  yiwasaki
// Trasan 2.01 : condition changed for 6dim t0 fitting, by H.Ozaki
//
// Revision 1.168  2000/05/03 11:13:59  yiwasaki
// Trasan 2.00 : version up without any change in trasan, trkmgr rectrk_tof fixed
//
// Revision 1.167  2000/04/29 03:45:58  yiwasaki
// old conf. fixed, new conf. unchanged
//
// Revision 1.166  2000/04/25 04:29:32  yiwasaki
// Trasan 2.00rc32 : fix again
//
// Revision 1.165  2000/04/25 02:53:26  yiwasaki
// Trasan 2.00rc31 : Definition of nhits, nster, and ndf in RECCDC_WIRHIT
//
// Revision 1.164  2000/04/14 05:20:46  yiwasaki
// Trasan 2.00rc30 : memory leak fixed, multi-track assignment to a hit fixed, helix parameter checks added
//
// Revision 1.163  2000/04/13 02:53:43  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.162  2000/04/11 13:05:49  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.161  2000/04/07 05:45:19  yiwasaki
// Trasan 2.00rc28 : curl finder bad point rejection, table clear, chisq info
//
// Revision 1.160  2000/04/04 12:14:17  yiwasaki
// Trasan 2.00RC27 : bad point rejection in conf., association check for dE/dx
//
// Revision 1.159  2000/04/04 07:52:38  yiwasaki
// Trasan 2.00RC26 : additions
//
// Revision 1.158  2000/04/04 07:40:08  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.157  2000/04/03 04:26:46  yiwasaki
// trkmgr fix again
//
// Revision 1.156  2000/03/31 08:33:24  yiwasaki
// Trasan 2.00RC24 : fix again
//
// Revision 1.155  2000/03/31 07:21:16  yiwasaki
// PM finder updates from J.Tanaka, trkmgr bug fixes
//
// Revision 1.154  2000/03/28 22:57:12  yiwasaki
// Trasan 2.00RC22 : PM finder from J.Tanaka
//
// Revision 1.153  2000/03/26 13:59:45  yiwasaki
// Trasan 2.00RC21 : track manager bug fix
//
// Revision 1.152  2000/03/24 10:22:58  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.151  2000/03/24 02:55:43  yiwasaki
// track manager
//
// Revision 1.150  2000/03/23 13:27:56  yiwasaki
// Trasan 2.00RC18 : bug fixes
//
// Revision 1.149  2000/03/21 11:55:18  yiwasaki
// Trasan 2.00RC17 : curler treatments
//
// Revision 1.148  2000/03/21 07:01:29  yiwasaki
// tmp updates
//
// Revision 1.147  2000/03/17 11:01:40  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.146  2000/03/07 03:00:56  yiwasaki
// Trasan 2.00RC15 : only debug info. changed
//
// Revision 1.145  2000/03/01 04:51:12  yiwasaki
// Trasan 2.00RC14 : stereo bug fix, curl updates
//
// Revision 1.144  2000/02/29 07:16:16  yiwasaki
// Trasan 2.00RC13 : default stereo param. changed
//
// Revision 1.143  2000/02/28 11:44:02  yiwasaki
// Trasan 2.00RC12 : t0 fit fix, salvage fix
//
// Revision 1.142  2000/02/28 01:59:00  yiwasaki
// Trasan 2.00RC11 : curl updates only
//
// Revision 1.141  2000/02/25 12:55:43  yiwasaki
// Trasan 2.00RC10 : stereo improved
//
// Revision 1.140  2000/02/25 08:09:58  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.139  2000/02/25 00:14:25  yiwasaki
// Trasan 2.00RC8 : robust fitter bug fix
//
// Revision 1.138  2000/02/24 06:19:30  yiwasaki
// Trasan 2.00RC7 : bug fix again
//
// Revision 1.137  2000/02/24 00:30:51  yiwasaki
// Trasan 2.00RC6 : quality2D bug fix
//
// Revision 1.136  2000/02/23 08:45:07  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.135  2000/02/17 13:24:20  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.134  2000/02/15 13:46:46  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.133  2000/02/10 13:11:40  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.132  2000/02/09 03:27:40  yiwasaki
// Trasan 1.68l : curl and new conf stereo updated
//
// Revision 1.131  2000/02/03 06:18:04  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.130  2000/02/01 11:24:44  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.129  2000/01/30 08:17:09  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.128  2000/01/28 06:30:26  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.127  2000/01/23 08:23:07  yiwasaki
// Trasan 1.67g : slow finder added
//
// Revision 1.126  2000/01/19 01:33:33  yiwasaki
// Trasan 1.67f : new conf modified
//
// Revision 1.125  2000/01/18 07:00:30  yiwasaki
// Trasan 1.67e : TWindow modified
//
// Revision 1.124  2000/01/12 14:25:01  yiwasaki
// Trasan 1.67d : tuned trasan, bug in masking fixed, sakura modified, mitsuo added
//
// Revision 1.123  2000/01/08 09:44:38  yiwasaki
// Trasan 1.66d : debug info. modified
//
// Revision 1.122  1999/11/19 09:13:12  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.121  1999/10/30 10:12:26  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.120  1999/10/21 15:45:16  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.119  1999/10/20 12:04:16  yiwasaki
// Trasan 1.64b : new T0 determination methode by H.Ozaki
//
// Revision 1.118  1999/10/15 04:28:05  yiwasaki
// TWindow is hidden, curl finder parameter
//
// Revision 1.117  1999/09/26 12:52:01  katayama
// Trivial fixes for seg. faults
//
// Revision 1.116  1999/09/21 02:01:35  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.115  1999/09/13 05:57:56  yiwasaki
// Trasan 1.62b release : calcdc_sag2 -> calcdc_sag3
//
// Revision 1.114  1999/09/10 09:19:58  yiwasaki
// Trasan 1.61b release : new parameters added, TTrackMC bug fixed
//
// Revision 1.113  1999/08/27 09:57:08  yiwasaki
// minor changes : version number corrected, parameter print-out
//
// Revision 1.112  1999/08/25 06:25:51  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.111  1999/08/04 01:01:41  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.110  1999/07/27 08:01:39  yiwasaki
// Trasan 1.58b release : protection for fpe error(not completed yet)
//
// Revision 1.109  1999/07/23 03:53:24  yiwasaki
// Trasan 1.57b release : minor changes only
//
// Revision 1.108  1999/07/17 06:42:59  yiwasaki
// THelixFitter has tof correction, Trasan time has option for tof correction
//
// Revision 1.107  1999/07/15 08:43:20  yiwasaki
// Trasan 1.55b release : Curl finder # of hits protection, bug in TManager for MC, helix fitter # of hits protection, fast finder improved
//
// Revision 1.106  1999/07/12 13:41:15  yiwasaki
// Trasan 1.54a release : cpu improvement in TTrack::approach, TTrackManager::saveTables checks # of hits
//
// Revision 1.105  1999/07/09 01:47:23  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.104  1999/07/06 10:41:11  yiwasaki
// Trasan 1.52a release : TRGCDC bug fix, associated hits output
//
// Revision 1.103  1999/07/01 08:15:24  yiwasaki
// Trasan 1.51a release : builder bug fix, TRGCDC bug fix again, T0 determination has more parameters
//
// Revision 1.102  1999/06/30 06:59:31  yiwasaki
// Trasan 1.50a release : TRGCDC::updateMC bug fix
//
// Revision 1.101  1999/06/29 05:14:40  yiwasaki
// Trasan 1.49a release : minor change : Support for invalid hits in TRGCDC
//
// Revision 1.100  1999/06/29 00:03:02  yiwasaki
// Trasan 1.48a release : TFastFinder added
//
// Revision 1.99  1999/06/26 07:05:43  yiwasaki
// Trasan 1.47a release : hit and tracking efficiency improved
//
// Revision 1.98  1999/06/17 09:45:01  yiwasaki
// Trasan 1.45 release : T0 determination by 2D fitting
//
// Revision 1.97  1999/06/17 01:39:18  yiwasaki
// Trasan 1.441 release : default mom. cut = 0. to recon. cosmics
//
// Revision 1.96  1999/06/16 08:29:53  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.95  1999/06/15 06:33:42  yiwasaki
// Trasan 1.43 release : minor changes in TRGCDCClust and TBuilder
//
// Revision 1.94  1999/06/14 12:40:22  yiwasaki
// Trasan 1.42 release : bug in findCloseHits fixed, sakura 1.06
//
// Revision 1.93  1999/06/14 05:51:05  yiwasaki
// Trasan 1.41 release : curl finder updates
//
// Revision 1.92  1999/06/10 09:44:53  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.91  1999/06/10 00:27:29  yiwasaki
// Trasan 1.39 release : TTrack::approach bug fix
//
// Revision 1.90  1999/06/09 15:09:54  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.89  1999/06/08 01:04:51  yiwasaki
// Trasan 1.37 release : TrkMgr::mask bug fix, reported by DST prod. group
//
// Revision 1.88  1999/06/06 09:33:02  yiwasaki
// Trasan 1.36 release : Trasan T0 output
//
// Revision 1.87  1999/05/28 18:17:57  yiwasaki
// Trasan 1.35 release : cathode and sakura updates
//
// Revision 1.86  1999/05/28 09:43:36  yiwasaki
// Trasan 1.35 beta : movePivot bug fix
//
// Revision 1.85  1999/05/28 07:11:12  yiwasaki
// Trasan 1.35 alpha release : cathdoe test version
//
// Revision 1.84  1999/05/26 05:03:49  yiwasaki
// Trasan 1.34 release : Track merge option added my T.Matsumoto, masking bug fixed, RecCDC_trk.stat is filled
//
// Revision 1.83  1999/05/18 04:44:31  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.82  1999/04/09 11:36:55  yiwasaki
// Trasan 1.32 release : TCosmicFitter minor change
//
// Revision 1.81  1999/04/09 09:26:54  yiwasaki
// Trasan 1.31 release : Cosmic fitter and curl finder updated
//
// Revision 1.80  1999/04/07 06:14:11  yiwasaki
// Trasan 1.30 release : curl finder mask updated
//
// Revision 1.79  1999/04/02 09:15:24  yiwasaki
// Trasan 1.29 release : tighter cuts for curl finder by J.Tanaka
//
// Revision 1.78  1999/03/21 15:45:44  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.77  1999/03/15 07:57:14  yiwasaki
// Trasan 1.27 release : curl finder update
//
// Revision 1.76  1999/03/12 13:12:02  yiwasaki
// Trasan 1.26 : bug fix in RECCDC_TRK output
//
// Revision 1.75  1999/03/12 06:56:46  yiwasaki
// Trasan 1.25 release : curl finder updated, salvage 2 is default, minor bug fixes
//
// Revision 1.74  1999/03/11 23:27:22  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.73  1999/03/10 12:55:08  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.72  1999/03/09 14:04:00  yiwasaki
// typo
//
// Revision 1.71  1999/03/09 13:43:03  yiwasaki
// Trasan 1.22 release : TrackManager bug fix
//
// Revision 1.70  1999/03/09 06:29:26  yiwasaki
// Trasan 1.21 release : conformal finder bug fix
//
// Revision 1.69  1999/03/08 05:47:53  yiwasaki
// Trasan 1.20 release : Fitter in TBuilder is modified
//
// Revision 1.68  1999/03/03 09:14:52  yiwasaki
// TRGCDCWireHit::WireHitValid -> WireHitFindingValid, WireHitFittingValid flag is checked in THelixFitter::fit
//
// Revision 1.67  1999/02/21 13:08:55  yiwasaki
// Trasan 1.18 : Cosmic builder loose cut
//
// Revision 1.66  1999/02/09 06:23:58  yiwasaki
// Trasan 1.17 release : cathode codes updated by T.Matsumoto, FPE error fixed by J.Tanaka
//
// Revision 1.65  1999/02/06 11:04:20  yiwasaki
// new approach calculation
//
// Revision 1.64  1999/02/04 02:12:26  yiwasaki
// Trasan 1.15 release : bug fix in cluster table output from S.Suzuki
//
// Revision 1.63  1999/02/03 06:23:14  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.62  1999/01/25 03:16:15  yiwasaki
// salvage improved, movePivot problem fixed again
//
// Revision 1.61  1999/01/20 01:02:50  yiwasaki
// Trasan 1.12 release : movePivot problem avoided temporary, new sakura
//
// Revision 1.60  1999/01/11 03:09:12  yiwasaki
// Trasan 1.11 release
//
// Revision 1.59  1998/12/26 03:43:05  yiwasaki
// cathode updates from S.Suzuki
//
// Revision 1.58  1998/12/04 15:11:04  yiwasaki
// TRGCDC creation timing changed, zero-division protection for TTrackMC
//
// Revision 1.57  1998/11/27 08:15:39  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.56  1998/11/12 12:27:33  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.55  1998/11/11 11:04:11  yiwasaki
// protection again
//
// Revision 1.54  1998/11/11 07:26:50  yiwasaki
// Trasan 1.1 beta 9 release : more protections for negative sqrt and zero division
//
// Revision 1.53  1998/11/10 09:09:14  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.52  1998/10/13 04:04:48  yiwasaki
// Trasan 1.1 beta 7 release : memory leak fixed by J.Tanaka, TCurlFinderParameters.h added by J.Tanaka
//
// Revision 1.51  1998/10/09 17:35:34  yiwasaki
// Trasan 1.1 beta 6 release : TBuilder::buildStereo bug, introduced by y.iwasaki, removed.
//
// Revision 1.50  1998/10/09 09:22:40  yiwasaki
// Trasan 1.1 beta 5 release : term() removes objects correctly
//
// Revision 1.49  1998/10/09 03:01:09  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.48  1998/10/06 02:30:08  yiwasaki
// Trasan 1.1 beta 3 relase : only minor change
//
// Revision 1.47  1998/09/30 02:34:36  yiwasaki
// Trasan 1.1 beta 2 release : minor change
//
// Revision 1.46  1998/09/29 01:24:28  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.45  1998/09/28 16:52:11  yiwasaki
// TBuilderCosmic added
//
// Revision 1.44  1998/09/28 16:11:14  yiwasaki
// fitter with cathode added
//
// Revision 1.43  1998/09/28 14:54:13  yiwasaki
// MC tables, TUpdater, oichan added
//
// Revision 1.42  1998/09/25 02:14:41  yiwasaki
// modification for cosmic
//
// Revision 1.41  1998/09/24 22:56:44  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.40  1998/09/17 16:05:26  yiwasaki
// Trasan 1.1 alpha 1 release : TTrackManager added to manage reconstructed tracks, TTrack::P() added, TTrack::_charge no longer constant
//
// Revision 1.39  1998/08/31 05:15:56  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.38  1998/08/12 16:32:52  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.37  1998/08/03 15:01:06  yiwasaki
// Trasan 1.07 release : cluster finder from S.Suzuki-san added
//
// Revision 1.36  1998/07/29 04:34:59  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.32  1998/07/07 08:31:09  yiwasaki
// Trasan::event bug fixed
//
// Revision 1.31  1998/07/06 15:48:52  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.30  1998/07/02 09:04:40  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.29  1998/06/30 00:53:50  yiwasaki
// Curl finder disabled by default
//
// Revision 1.28  1998/06/29 02:15:09  yiwasaki
// Trasan 1 release candidate 1
//
// Revision 1.27  1998/06/24 06:55:02  yiwasaki
// Trasan 1 beta 5.2 release, memory leaks fixed
//
// Revision 1.26  1998/06/24 04:14:24  yiwasaki
// fiTerm in RECCDC_TRK stored
//
// Revision 1.25  1998/06/23 02:00:24  yiwasaki
// stereo recon. is default
//
// Revision 1.24  1998/06/21 18:38:21  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.23  1998/06/17 20:23:01  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.22  1998/06/17 20:12:39  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.21  1998/06/14 11:09:53  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.20  1998/06/11 12:25:17  yiwasaki
// TConformalLink removed
//
// Revision 1.19  1998/06/11 08:14:11  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.18  1998/06/08 14:37:54  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.17  1998/06/03 17:17:40  yiwasaki
// const added to TRGCDC::hits,axialHits,stereoHits,hitsMC, symbols WireHitNeghborHit* added in TRGCDCWireHit, TCluster::innerWidth,outerWidth,innerMostLayer,outerMostLayer,type,split,split2,widht,outer,updateType added, TLink::conf added, TTrack::appendStereo3,refineStereo2,aa,bb,Zchisqr added
//
// Revision 1.16  1998/05/26 05:10:19  yiwasaki
// cvs repair
//
// Revision 1.15  1998/05/25 08:25:36  yiwasaki
// cvs test
//
// Revision 1.14  1998/05/25 08:21:41  yiwasaki
// cvs test again
//
// Revision 1.13  1998/05/25 04:10:53  yiwasaki
// cvs test, Trasan.cc has no change
//
// Revision 1.12  1998/05/24 15:00:01  yiwasaki
// Trasan 1 alpha 5 release, pivot is moved to the first hit
//
// Revision 1.11  1998/05/22 08:23:21  yiwasaki
// Trasan 1 alpha 4 release, TCluster added, TConformalLink no longer used
//
// Revision 1.10  1998/05/18 08:08:54  yiwasaki
// preparation for alpha 3
//
// Revision 1.9  1998/05/08 09:45:46  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.8  1998/04/23 17:21:47  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.7  1998/04/16 16:49:35  yiwasaki
// minor changes
//
// Revision 1.6  1998/04/10 09:36:28  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.5  1998/04/10 00:50:17  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#include <cmath>
#include <cfloat>
#include <time.h>

//#include "framework/core/ModuleManager.h"
#include "framework/core/Module.h"

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/THelix.h"
#include "tracking/modules/trasan/Trasan.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/TrackMC.h"
#include "tracking/modules/trasan/TPerfectFinder.h"
#include "tracking/modules/trasan/TConformalFinder0.h"
#include "tracking/modules/trasan/TConformalFinder.h"
#include "tracking/modules/trasan/TCurlFinder.h"
//cnv #include "tracking/modules/trasan/TPMCurlFinder.h"
#include "tracking/modules/trasan/THoughFinder.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TTrackMC.h"
#include "tracking/modules/trasan/TLink.h"
#include <framework/datastore/StoreArray.h>
#include <GFTrackCand.h>
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
#ifdef TRASAN_WINDOW_GTK
#include <gtkmm.h>
#include <pangomm/init.h>
#endif

#ifndef PANTHER_RECCDC_WIRHIT_
#define PANTHER_RECCDC_WIRHIT_
struct reccdc_wirhit {
    int m_panther_dummy_;
    int m_ID;
    float m_ddl;
    float m_ddr;
    float m_erddl;
    float m_erddr;
    float m_pChiSq;
    int m_lr;
    int m_stat;
    int m_geo;
    int m_dat;
    int m_trk;
    float m_tdc;
    float m_adc;
};
#else
struct reccdc_wirhit;
#endif
#ifndef PANTHER_GEOCDC_LAYER_
#define PANTHER_GEOCDC_LAYER_
struct geocdc_layer {
    int m_panther_dummy_;
    int m_ID;
    float m_slant;
    float m_r;
    float m_zf;
    float m_zb;
    float m_rcsiz1;
    float m_rcsiz2;
    float m_pcsiz;
    float m_div;
    float m_offset;
    float m_shift;
    int m_wirst;
    int m_sup;
    float m_fdist;
    float m_bdist;
    float m_ft;
    float m_bt;
    float m_fpin;
    float m_bpin;
    float m_ftwist;
    float m_btwist;
    float m_fxoff;
    float m_fyoff;
    float m_fzoff;
    float m_bxoff;
    float m_byoff;
    float m_bzoff;
};
#endif
#ifndef PANTHER_GEOCDC_WIRE_
#define PANTHER_GEOCDC_WIRE_
struct geocdc_wire {
    int m_panther_dummy_;
    int m_ID;
    float m_xwb;
    float m_ywb;
    float m_zwb;
    float m_xwf;
    float m_ywf;
    float m_zwf;
    float m_slant;
    int m_cell;
    int m_layer;
    int m_stat;
    int m_lyr;
};
#endif
#ifndef PANTHER_RECCDC_WIRHIT_
#define PANTHER_RECCDC_WIRHIT_
struct reccdc_wirhit {
    int m_panther_dummy_;
    int m_ID;
    float m_ddl;
    float m_ddr;
    float m_erddl;
    float m_erddr;
    float m_pChiSq;
    int m_lr;
    int m_stat;
    int m_geo;
    int m_dat;
    int m_trk;
    float m_tdc;
    float m_adc;
};
#endif
#ifndef PANTHER_DATRGCDC_MCWIRHIT_
#define PANTHER_DATRGCDC_MCWIRHIT_
struct datcdc_mcwirhit {
    int m_panther_dummy_;
    int m_ID;
    float m_xw;
    float m_yw;
    float m_zw;
    float m_dist;
    float m_chrg;
    float m_xin;
    float m_yin;
    float m_zin;
    float m_xout;
    float m_yout;
    float m_zout;
    int m_lr;
    int m_stat;
    int m_geo;
    int m_dat;
    int m_hep;
    int m_trk;
    float m_px;
    float m_py;
    float m_pz;
    float m_pid;
    float m_tof;
    float m_tdc;
};
#endif
#ifndef PANTHER_RECCDC_TRK_
#define PANTHER_RECCDC_TRK_
struct reccdc_trk {
    int m_panther_dummy_;
    int m_ID;
    float m_helix[5];
    float m_pivot[3];
    float m_error[15];
    float m_chiSq;
    float m_ndf;
    float m_fiTerm;
    int m_nhits;
    int m_nster;
    int m_nclus;
    int m_stat;
    float m_mass;
};
#endif
#ifndef PANTHER_RECCDC_TRK_ADD_
#define PANTHER_RECCDC_TRK_ADD_
struct reccdc_trk_add {
    int m_panther_dummy_;
    int m_ID;
    int m_quality;
    int m_kind;
    int m_mother;
    int m_daughter;
    int m_decision;
    float m_likelihood[3];
    int m_stat;
    int m_rectrk;
};
#endif
#ifndef PANTHER_RECCDC_MCTRK2HEP_
#define PANTHER_RECCDC_MCTRK2HEP_
struct reccdc_mctrk2hep {
    int m_panther_dummy_;
    int m_ID;
    int m_wir;
    int m_clust;
    int m_trk;
    int m_hep;
};
#endif
#ifndef PANTHER_RECCDC_MCTRK_
#define PANTHER_RECCDC_MCTRK_
struct reccdc_mctrk {
    int m_panther_dummy_;
    int m_ID;
    int m_hep;
    float m_wirFrac;
    float m_wirFracHep;
    int m_charge;
    float m_ptFrac;
    float m_pzFrac;
    int m_quality;
};
#endif
#ifndef PANTHER_RECCDC_TIMING_
#define PANTHER_RECCDC_TIMING_
struct reccdc_timing {
    int m_panther_dummy_;
    int m_ID;
    float m_time;
    int m_quality;
};
#endif

namespace Belle {

//...Globals...
Trasan *
Trasan::_trasan = 0;

int
TrasanTest = 0;

float
TrasanTHelixFitterChisqMax = 0;

int
TrasanTHelixFitterNtrialMax = 20;

const Point3D
ORIGIN = Point3D(0., 0., 0.);

std::string
Trasan::version(void) const {
    return "5.17";
}

//...Definitions...
Trasan *
Trasan::getTrasan(void) {
    if (! _trasan) _trasan = new Trasan();
    return _trasan;
}

Trasan::~Trasan() {
}

Trasan::Trasan()
    : Belle2::Module::Module(),
      b_cdcVersion(0),       // 0:automatic, 1:normal cell, 2:small cell
      b_fudgeFactor(1.0),

      b_debugLevel(0),
      b_useAllHits(0),
      b_doT0Reset(0),
      b_nT0ResetMax(1),
      b_doMCAnalysis(0),
      b_mode(1),
      b_nEventsToSkip(0),
      b_helixFitterChisqMax(0.),
      b_helixFitterNtrialMax(20),

      b_doPerfectFinder(0),
      b_perfectFitting(0),

      b_conformalFinder(1),
      b_doConformalFinder(1),
      b_doConformalFastFinder(1),
      b_doConformalSlowFinder(1),
      b_conformalPerfectSegmentFinding(0),
      b_conformalUseSmallCells(1),
      b_conformalFittingFlag(0),
      b_conformalMaxSigma(30.),
      b_conformalMinNLinksForSegment(2),
      b_conformalMinNCores(2),
      b_conformalMinNSegments(3),
      b_salvageLevel(30.),
      b_conformalSalvageLoadWidth(1),
      b_conformalStereoMode(1),
      b_conformalStereoLoadWidth(5),
      b_conformalStereoMaxSigma(30.),
      b_conformalStereoSzSegmentDistance(10.),
      b_conformalStereoSzLinkDistance(15.),

      b_doConformalFinderStereo(1),
      b_doConformalFinderCosmic(0),
      b_conformalFraction(0.7),
      b_conformalStereoZ3(20.),
      b_conformalStereoZ4(20.),
//    b_conformalStereoChisq3(15.),
//    b_conformalStereoChisq4(9.),
      b_conformalStereoChisq3(30.),
      b_conformalStereoChisq4(30.),
      b_conformalFittingCorrections(0),

      b_momentumCut(0.0),
      b_ptCut(0.005),
      b_tanlCut(0.),
      b_fittingFlag(0),
      b_doSalvage(2),
      b_mergeTracks(1),
      b_mergeTrackDistance(0.33),
      b_mergeCurls(0),
      b_doT0Determination(7),
      b_nTracksForT0(2),
      b_sortMode(0),
      b_doAssociation(1),
      b_associateSigma(60),
      b_test(0),

      /* For Curl Finder --> */
      //ho      b_doCurlFinder(1),
      b_doCurlFinder(0),
      min_segment(5),
      min_salvage(10),
      bad_distance_for_salvage(1.0),
      good_distance_for_salvage(0.2),
      min_sequence(6),
      min_fullwire(7),
      range_for_axial_search(1.5),
      range_for_stereo_search(2.5),
      superlayer_for_stereo_search(3),
      range_for_axial_last2d_search(1.5),
      range_for_stereo_last2d_search(2.0),
      trace2d_distance(35.0),
      trace2d_first_distance(0.5),
      trace3d_distance(30.0),
      determine_one_track(0),
      selector_max_impact(4.0),
      selector_max_sigma(36.0),
      selector_strange_pz(10.0),
      selector_replace_dz(15.0),
      stereo_2dfind(0),
      merge_exe(1),
      merge_ratio(0.1),
      merge_z_diff(10.0),
      mask_distance(0.5),
      ratio_used_wire(0.3),
      range_for_stereo1(2.4),
      range_for_stereo2(2.7),
      range_for_stereo3(2.9),
      range_for_stereo4(3.4),
      range_for_stereo5(4.1),
      z_cut(50.0),
      z_diff_for_last_attend(1.5),
      svd_reconstruction(0),
      min_svd_electrons(20000.),
      //ho      on_correction(1),
      //ho      output_2dtracks(1),
      on_correction(0),
      output_2dtracks(0),
      curl_version(2),
      /* <-- For Curl Finder */

      b_pmCurlFinder(1),
      b_doPMCurlFinder(1),
      _doPMCurlFinder(0),
      min_svd_electrons_in_pmc(20000.),
      b_doSvdAssociator(0),

      b_doHoughFinder(1),
      b_doHoughFinderCurlSearch(1),
      b_houghAxialLoadWidth(1),
      b_houghAxialLoadWidthCurl(1),
      b_houghMaxSigma(30),
      b_houghStereoMaxSigma(30),
      b_houghSalvageLevel(30),
      b_houghMeshX(350),
      b_houghMeshY(100),
      b_houghPtBoundary(20),
      b_houghThreshold(0.8),
//b_houghThreshold(0.7),
//b_houghThreshold(0.72),
      b_houghMeshXLowPt(150),
      b_houghMeshYLowPt(100),
      b_houghPtBoundaryLowPt(200),
      b_houghThresholdLowPt(0.7),
      b_houghMode(0),

      b_simulateSmallCell(0),

      b_amin0(0.),
      b_amin1(0.),
      b_amin2(0.),
      b_amin3(0.),
      b_amin4(0.),
      b_amax0(DBL_MAX),
      b_amax1(DBL_MAX),
      b_amax2(DBL_MAX),
      b_amax3(DBL_MAX),
      b_amax4(DBL_MAX),

      _cdc(0),
//cnv  _cdccat(0),
      _perfectFinder(0),
      _confFinder(0),
      _curlFinder(0),
//cnv  _clustFinder(0),
//cnv  _pmCurlFinder(0),
      _houghFinder(0),
      _nEvents(0)
#ifdef TRASAN_WINDOW_GTK
      , _w(0),
      _hp(0),
      _hm(0),
      _hc(0),
      _hl(0)
#endif
{
    _trasan = this;
//    setDescription("Hello World");
    setDescription("Trasan test version");

    TrasanTHelixFitterChisqMax = b_helixFitterChisqMax;
    TrasanTHelixFitterNtrialMax = b_helixFitterNtrialMax;

//    addParam("testParamInt", b_doSalvage, 20);

//    int a = 10 / 0;

}

void
Trasan::initialize() {

#ifdef TRASAN_DEBUG_DETAIL
    b_debugLevel = 2;
    std::cout << "Trasan ... initializing" << std::endl;
#endif

    //...Create TRGCDC...
    _cdcVersion = "cdc2_test";
    _cdc = Belle2::TRGCDC::getTRGCDC(_cdcVersion);
    _cdc->debugLevel(b_debugLevel);
#if defined(TRASAN_DEBUG)
//  _cdc->dump("geometry superLayers layers wires detail");
    _cdc->dump("geometry superLayers layers detail");
#endif

    //...Prepare TLink...
    TLink::initializeBuffers();

    //...Prepare THelix related stuff...
    CLHEP::HepVector a_min(5, 0);
    CLHEP::HepVector a_max(5, 0);
    a_min[0] = b_amin0;
    a_min[1] = b_amin1;
    a_min[2] = b_amin2;     // a_min[2] = 1e-10;
    a_min[3] = b_amin3;
    a_min[4] = b_amin4;
//  a_max[0] = a_max[2] = 1e10;
    a_max[0] = b_amax0;
    a_max[1] = b_amax1;
    a_max[2] = b_amax2;
    a_max[3] = b_amax3;
    a_max[4] = b_amax4;
    THelix::set_limits(a_min, a_max);

    //...Trasan...
    TrasanTHelixFitterChisqMax = b_helixFitterChisqMax;
    TrasanTHelixFitterNtrialMax = b_helixFitterNtrialMax;

    //...Create rphi finder...
    _perfectFinder = new TPerfectFinder(b_perfectFitting,
                                        b_conformalMaxSigma,
                                        b_conformalStereoMaxSigma,
                                        b_conformalFittingCorrections);

    if ((! _confFinder) && (b_conformalFinder == 0)) {
	_confFinder = new TConformalFinder0(b_conformalMaxSigma,
					    b_conformalFraction,
					    b_conformalStereoZ3,
					    b_conformalStereoZ4,
					    b_conformalStereoChisq3,
					    b_conformalStereoChisq4,
					    b_conformalStereoMaxSigma,
					    b_conformalFittingCorrections,
					    b_salvageLevel,
					    b_doConformalFinderCosmic);
    } else if (! _confFinder) {
	_confFinder = new TConformalFinder(b_doConformalFastFinder,
					   b_doConformalSlowFinder,
					   b_conformalPerfectSegmentFinding,
					   b_conformalUseSmallCells,
					   b_conformalMaxSigma,
					   b_conformalStereoMaxSigma,
					   b_salvageLevel,
					   b_conformalMinNLinksForSegment,
					   b_conformalMinNCores,
					   b_conformalMinNSegments,
					   b_conformalSalvageLoadWidth,
					   b_conformalStereoMode,
					   b_conformalStereoLoadWidth,
					   b_conformalStereoSzSegmentDistance,
					   b_conformalStereoSzLinkDistance,
					   b_conformalFittingFlag);
    }
    _confFinder->debugLevel(b_debugLevel);
    _confFinder->doStereo(b_doConformalFinderStereo);

    //...Salvage flag is ignored in new conf. finder...
    if (b_doSalvage == 2) _confFinder->doSalvage(true);

    //...Create curl finder...
    if (! _curlFinder)
	_curlFinder = new TCurlFinder((unsigned)min_segment,
				      (unsigned)min_salvage,
				      bad_distance_for_salvage,
				      good_distance_for_salvage,
				      (unsigned)min_sequence,
				      (unsigned)min_fullwire,
				      range_for_axial_search,
				      range_for_stereo_search,
				      (unsigned)superlayer_for_stereo_search,
				      range_for_axial_last2d_search,
				      range_for_stereo_last2d_search,
				      trace2d_distance,
				      trace2d_first_distance,
				      trace3d_distance,
				      (unsigned)determine_one_track,
				      selector_max_impact,
				      selector_max_sigma,
				      selector_strange_pz,
				      selector_replace_dz,
				      (unsigned)stereo_2dfind,
				      (unsigned)merge_exe,
				      merge_ratio,
				      merge_z_diff,
				      mask_distance,
				      ratio_used_wire,
				      range_for_stereo1,
				      range_for_stereo2,
				      range_for_stereo3,
				      range_for_stereo4,
				      range_for_stereo5,
				      z_cut,
				      z_diff_for_last_attend,
				      (unsigned)svd_reconstruction,
				      min_svd_electrons,
				      (unsigned)on_correction,
				      (unsigned)output_2dtracks,
				      (unsigned)curl_version,
				      b_simulateSmallCell);
    _curlFinder->debugLevel(b_debugLevel);

    //...Create pm curl finder...
//cnv     if (! _pmCurlFinder)
//  _pmCurlFinder = new TPMCurlFinder(-min_svd_electrons_in_pmc,
//            min_svd_electrons_in_pmc);

    //...Create Hough finder...
//  b_conformalSalvageLoadWidth is not used
    if (b_doHoughFinder) {
	_houghFinder = new THoughFinder(b_doHoughFinderCurlSearch,
					b_houghAxialLoadWidth,
					b_houghAxialLoadWidthCurl,
					b_houghMaxSigma,
					b_houghStereoMaxSigma,
					b_houghSalvageLevel,
					5,
					b_conformalFittingFlag,
					b_conformalStereoLoadWidth,
					// b_conformalStereoLoadWidth * 2,
					b_conformalSalvageLoadWidth,
					b_houghMeshX,
					b_houghMeshY,
					b_houghPtBoundary,
					b_houghThreshold,
					b_houghMeshXLowPt,
					b_houghMeshYLowPt,
					b_houghPtBoundaryLowPt,
					b_houghThresholdLowPt,
					b_houghMode);
	_houghFinder->debugLevel(b_debugLevel);
	_houghFinder->init();
    }

    //...Create cathode cluster finder...
//cnv     if (_cdccat) {
// //   if (! _clustFinder) _clustFinder = new TRGCDCClustFinder(_cdccat);
// //   _clustFinder->debugLevel(b_debugLevel);
//     }

    //...Track manager setup...
    _trackManager.maxMomentum(b_momentumCut);
    _trackManager.minPt(b_ptCut);
    _trackManager.maxTanl(b_tanlCut);
    _trackManager.debugLevel(b_debugLevel);
    _trackManager.fittingFlag(b_fittingFlag);
    if (b_debugLevel)
	_trackManager.defineHistograms();

    //...Initialize...
    TUpdater::initialize();

    //...Save PMCurlFinder switch...
    _doPMCurlFinder = b_doPMCurlFinder;

#ifdef TRASAN_WINDOW_GTK

    //...Trasan window...
    std::cout << "TWindowGTK ... initializing GTK" << std::endl;
    int argc = 0;
    char** argv = 0;

    Gtk::Main main_instance(argc, argv);
    static TWindowGTKConformal w("Conformal", 160, 1137, 600);
    _w = & w;
    w.stage("Trasan : Tracking not started yet");
    w.move(0, 0);
    w.show();

#ifdef TRASAN_WINDOW_GTK_HOUGH
    //...Hough window...
    static TWindowGTKHough hp("Hough Plus");
    static TWindowGTKHough hm("Hough Minus");
    _hp = & hp;
    _hm = & hm;
    hp.show();
    hm.show();
    hp.move(600, 0);
    hm.move(1200, 0);

    //...Hough window for curl search...
    static TWindowGTKHough hc("Hough Curl");
    _hc = & hc;
    hc.show();
    hc.move(1800, 0);
#endif
#ifdef TRASAN_WINDOW_GTK_SZ
    //...Hough window for line searches...
    static TWindowGTKHough hl("Hough Line");
    _hl = & hl;
    hl.show();
    hl.move(0, 800);

    //...sz window for line searches...
    static TWindowGTKSZ sz;
    _sz = & sz;
    sz.show();
    sz.move(600, 800);
#endif
#endif

    //...For debug...
    banner();
    dump("parameter");

    TrasanTest = b_test;
    Belle2::StoreArray<GFTrackCand>::registerPersistent(_gfTrackCandsName);
}

void
Trasan::terminate() {

    //...Clear Trasan objects...
    clear(true);

    //...Summary...
    dump("summary");

    //...Delete Trasan objects...
//cnv    if (_cdccat) delete _cdccat;
    if (_perfectFinder) delete _perfectFinder;
    if (_confFinder) delete _confFinder;
    if (_curlFinder) delete _curlFinder;
//cnv    if (_clustFinder) delete _clustFinder;
//cnv    if (_pmCurlFinder) delete _pmCurlFinder;
    if (_houghFinder) delete _houghFinder;
}

void
Trasan::disp_stat(const char* m) {
    std::string msg = m;
    dump(msg);
}

void
Trasan::event() {

    static clock_t time_start = 0;
//cnv    static clock_t time_end = 0;
//cnv    static clock_t time_total = 0;

    ++_nEvents;
    if (int(_nEvents) < b_nEventsToSkip) {
	if (b_debugLevel) {
	    std::cout << "Trasan ... skipping ev# " << _nEvents << std::endl;
	}
	return;
    }
    if (b_debugLevel) {
	time_start = clock();
	std::cout << "Trasan ... processing ev# " << _nEvents << std::endl;
    }

#ifdef TRASAN_WINDOW_GTK
    std::cout << "Trasan ... opening GTK windows" << std::endl;
    TWindowGTKConformal& w = Trasan::getTrasan()->w();
    w.beginEvent();
#ifdef TRASAN_WINDOW_GTK_HOUGH
    _hp->clear();
    _hm->clear();
    _hc->clear();
#endif
#ifdef TRASAN_WINDOW_GTK_SZ
    _hl->clear();
    _sz->clear();
#endif
//debug
    w.skip(true);
#endif

    //...Starting point...
//cnv trasan_start:

    //...Clear myself...
    clear();

    //...Update TRGCDC...
    _cdc->update(b_doMCAnalysis);
///    _cdc->dump("detail hits");
//cnv    if (b_doClustFinder && _cdccat) _cdccat->update();

    //...Get lists of hits...
    unsigned mask = 0;
    if (! b_useAllHits) mask = CellHitFindingValid;
//  const AList<TRGCDCWireHit> & axialHits = _cdc->axialHits(mask);
//  const AList<TRGCDCWireHit> & stereoHits = _cdc->stereoHits(mask);
//  const AList<TRGCDCWireHit> & allHits = _cdc->hits(mask);
    CAList<Belle2::TRGCDCWireHit> axialHits;
    CAList<Belle2::TRGCDCWireHit> stereoHits;
    CAList<Belle2::TRGCDCWireHit> allHits;
    const std::vector<const Belle2::TRGCDCWireHit*> hits = _cdc->hits();
    for (unsigned i = 0; i < hits.size(); i++) {
	allHits.append(hits[i]);
    }
    allHits.sort(Belle2::TRGCDCWireHit::sortByWireId);
    for (unsigned i = 0; i < (unsigned) allHits.length(); i++) {
	const Belle2::TRGCDCWireHit* hit = allHits[i];
	if (hit->wire().axial())
	    axialHits.append(hit);
	else
	    stereoHits.append(hit);

    }

    //...Storage for tracks...
    AList<TTrack> tracks;
    AList<TTrack> tracks2D;

    //...Perfect finder...
    if (b_doPerfectFinder) {
	_perfectFinder->doit(axialHits, stereoHits, tracks, tracks2D);
	_trackManager.append(tracks);
    }

    //...Normal finders..
    else {

	//...Mode 0... (all finders on)
	if (b_mode == 0)
	    main0(axialHits, stereoHits, allHits, tracks, tracks2D);

	//...Mode 1... (all finders on)
	else
	    main1(axialHits, stereoHits, allHits, tracks, tracks2D);
    }

    //...Move a pivot...
    _trackManager.movePivot();

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "Track list after move pivot" << std::endl;
    _trackManager.dump("eventSummary helix", Tab(+1));
#endif

    //...Save Panther tables...
    // _trackManager.checkNumberOfHits();
    if (b_sortMode == 0) _trackManager.sortTracksByQuality();
    else                 _trackManager.sortTracksByPt();

    //...For Belle2...
    Belle2::StoreArray<GFTrackCand> trackCandidates(_gfTrackCandsName);
    _trackManager.saveBelle2(trackCandidates);

//cnv     _trackManager.saveTables();
//     if (b_doMCAnalysis) {
//  if (mcEvent()) {
//      mcInformation();
//      _trackManager.saveMCTables();
//  }
//     }

//     //...Statistics...
//     _trackManager.statistics(b_doMCAnalysis);

//     //...Calculate helix by associating SVD... jtanaka 000925
//     if (b_doSvdAssociator)_trackManager.addSvd(b_doMCAnalysis);

//     //...Associate cathode clusters and tracks...
// //cnv    if (b_doClustFinder && _cdccat) cathode(b_cathodeWindow);

//     //...T0 correction...
//     if (b_doT0Determination)
//  _trackManager.determineT0(b_doT0Determination, b_nTracksForT0);

    //...For debug...
    if (b_debugLevel) {
	std::cout
	    << "Trasan ... ev# " << _nEvents << " processed,"
	    << " #tracks found=" << _trackManager.allTracks().length()
	    << ", #good tracks=" << _trackManager.tracks().length()
	    << ", #2D tracks=" << _trackManager.tracks2D().length()
	    << std::endl;
	if (b_debugLevel > 1)
	    _trackManager.dump("eventSummary");
	else if (b_debugLevel > 2)
	    _trackManager.dump("eventSummary hits");

    }

    TUpdater::update();

//cnv     if (b_debugLevel) {
//  time_end = clock();
//  time_total += time_end - time_start;
//  const float time_used = float((time_end - time_start)) /
//      float(CLOCKS_PER_SEC);
//  const float time_total_used =
//      float(time_total) / float(CLOCKS_PER_SEC);

//  const belle_event & r = * (belle_event *) BsGetEnt(BELLE_EVENT,
//                     1,
//                     BBS_No_Index);
//  std::cout << "Trasan ... processed ev# " << _nEvents << std::endl;
//  std::cout << "Trasan ... time used = " << time_used
//         << " total time " << time_total_used
//         << " exp " << r.m_ExpNo << " run " << r.m_RunNo
//         << " evt " << r.m_EvtNo << std::endl;
//     }

#ifdef TRASAN_WINDOW_GTK
    AList<TLink> tmp;
    for (unsigned i = 0; i < (unsigned) _trackManager.tracks().length(); i++)
	tmp.append(_trackManager.tracks()[i]->links());
    w.endOfEvent();
    w.clear();
    w.stage("Trasan : all finders finished");
    w.information("blue:reconstructed, red:reconstructed(2D), pink:not used");
    w.append(allHits, Gdk::Color("lightpink"));
//  w.append(axialHits, Gdk::Color("lightpink"));
    w.append(tmp, Gdk::Color("blue"));
    w.append(_trackManager.tracks(), Gdk::Color("blue"));
    w.append(_trackManager.tracks2D(), Gdk::Color("red"));
//  w.skipEvent(true);
//  w.run(true);
    w.run(true);

//     Glib::RefPtr<Gtk::PrintOperation> op = Gtk::PrintOperation::create();
//     op->set_export_filename("test.pdf");
//     Gtk::PrintOperationResult res = op->run(Gtk::PRINT_OPERATION_ACTION_EXPORT);
#endif
}

void
Trasan::mcInformation(void) {

    //...Preparation...
//  const AList<TTrack> & allTracks = _trackManager.allTracks();
    const AList<TTrack> & allTracks = _trackManager.tracksFinal();

    unsigned nHep = Belle2::TRGCDCTrackMC::list().size();
    unsigned nTrk = allTracks.length();
    unsigned* N;
    if (NULL == (N = (unsigned*) malloc(nHep * sizeof(unsigned)))) {
	perror("$Id: Trasan.cc 11152 2010-04-28 01:24:38Z yiwasaki $:N:malloc");
	exit(1);
    }
    for (unsigned i = 0; i < nHep; i++) N[i] = 0;

    //...Loop over all tracks...
    for (unsigned i = 0; i < nTrk; i++) {
	TTrackMC* mc = allTracks[i]->_mc;
	if (! mc) {
	    mc = new TTrackMC(* allTracks[i]);
	    _mcTracks.append(mc);
	    allTracks[i]->_mc = mc;
	}

	mc->update();
	if (mc->hepId() != -1)
	    if (mc->charge())
		++N[mc->hepId()];
    }

    //...Check uniqueness...
    for (unsigned i = 0; i < nHep; i++) {
	if (N[i] < 2) {
	    for (unsigned j = 0; j < nTrk; j++)
		if ((unsigned) allTracks[j]->_mc->hepId() == i)
		    allTracks[j]->_mc->_quality += TTrackUnique;
	}
    }

    //...Good tracks...
    for (unsigned i = 0; i < nTrk; i++) {
	unsigned& quality = allTracks[i]->_mc->_quality;
	if ((quality & TTrackGhost) && (quality & TTrackUnique))
	    quality += TTrackGood;
    }

    //...Termination...
    free(N);

    //...Debug...
    if (b_debugLevel) {
	std::cout << "Belle2::TRGCDCTrackMC list ..." << std::endl;
	std::cout << "   id:ptype:mother:p:v" << std::endl;
	for (unsigned i = 0; i < nHep; i++)
	    (Belle2::TRGCDCTrackMC::list())[i]->dump("", "    ");
	std::cout << "TTrackMC list ..." << std::endl;
	for (unsigned i = 0; i < nTrk; i++)
	    allTracks[i]->_mc->dump("", "    ");
    }
}

void
Trasan::clear(bool termination) {

    //...Clear track candidates of the last event...
    HepAListDeleteAll(_mcTracks);

    //...Clear finders...
    if (b_doPerfectFinder) _perfectFinder->clear();
    if (b_doConformalFinder) _confFinder->clear();
    if (b_doCurlFinder) _curlFinder->clear();
//cnv    if (b_doClustFinder && _cdccat) _clustFinder->clear();
//cnv    if (b_doPMCurlFinder) _pmCurlFinder->clear();
    if (b_doHoughFinder) _houghFinder->clear();
    _trackManager.clear(termination);
    _trackManager.clearTables();

#ifdef TRASAN_DEBUG
    std::cout << "Trasan::clear ... nTLinks=" << TLink::nTLinks()
              << ", nTLinksMax=" << TLink::nTLinksMax() << std::endl;
    std::cout << "Trasan::clear ... nTTracks=" << TTrack::nTTracks()
              << ", nTTracksMax=" << TTrack::nTTracksMax() << std::endl;
#endif
}

void
Trasan::fastClear(void) {
    clear();
}

bool
Trasan::mcEvent(void) const {
//cnv     struct belle_event * ev =
//  (struct belle_event *) BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);

//     //...No BELLE_EVENT ???...
//     if (! ev) return false;
//     if (ev->m_ExpMC == 2) return true;
//     return false;
    return true;
}

void
Trasan::dump(const std::string& msg, const std::string& pre) const {
    bool def = (msg == "") ? true : false;

    std::cout << std::endl;
    if (msg.find("summary") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
	std::cout << "Trasan Summary (" << version() << ")" << std::endl;
	std::cout << "    Track Manager Summary" << std::endl;
	if (b_doMCAnalysis)
	    _trackManager.dump("summary MC", "        ");
	else
	    _trackManager.dump("summary", "        ");
	if (_confFinder) {
	    std::cout << "    Conformal Finder Summary" << std::endl;
	    _confFinder->dump("summary", "        ");
	}
    }
    if (def || msg.find("parameter") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
	std::string t0 = pre;
	std::string t1 = pre + "    ";
	std::string t2 = pre + "        ";

	std::cout
	    << t0 << name() << "(" << version() << ")" << " : debug level = "
	    << b_debugLevel << std::endl;
	if (b_doPerfectFinder != 0) {
	    std::cout
		<< t1 << "Perfect finder is active. ";
	    std::cout
		<< "Other finders will not called." << std::endl;
	}
	if (b_conformalFinder == 1) {
	    std::cout
		<< t1 << "New trasan(new conf. finder) is active."
		<< std::endl;
	}

	if (_cdc) {
	    std::cout
		<< t1 << _cdc->name() << "(" << _cdc->version() << ")"
		<< std::endl;
	    std::cout
		<< t2 << "cdc version           : " << _cdc->versionCDC()
		<< std::endl;
	    std::cout
		<< t2 << "fudge factor          : " << _cdc->fudgeFactor()
		<< std::endl;
//      std::cout
//    << t2 << "simulate small cell : "
//    << _cdc->simulateSmallCell() << std::endl;
	    std::cout
		<< t2 << "simulate small cell : no longer available"
		<< std::endl;
	} else {
	    std::cout
		<< t1 << "CDC is not created yet" << std::endl;
	}

//cnv   if (_cdccat)
//      std::cout
//    << t1 << _cdccat->name() << "(" << _cdccat->version() << ")"
//    << std::endl;

	std::cout
	    << t1 << "ignore hit quality      : " << b_useAllHits << std::endl;
	std::cout
	    << t1 << "do T0 reset             : " << b_doT0Reset << std::endl;
	std::cout
	    << t1 << "    max number of reset : " << b_nT0ResetMax
	    << std::endl;
	std::cout
	    << t1 << "MC analysis             : " << b_doMCAnalysis
	    << std::endl;
	std::cout
	    << t1 << "mode                    : " << b_mode << std::endl;
	std::cout
	    << t1 << "helix fitter chisq max  : " << b_helixFitterChisqMax
	    << std::endl;
	std::cout
	    << t1 << "helix fitter max. iterations  : "
	    << b_helixFitterNtrialMax
	    << std::endl;
	std::cout
	    << t1 << "test flag               : " << b_test << std::endl;

	std::cout
	    << t1 << _trackManager.name() << "(" << _trackManager.version()
	    << ") options" << std::endl;
	std::cout
	    << t2 << "T0 determination      : " << b_doT0Determination
	    << std::endl;
	std::cout
	    << t2 << "    # of tracks       : " << b_nTracksForT0 << std::endl;
	std::cout
	    << t2 << "bank sort             : " << b_sortMode << std::endl;
	std::cout
	    << t2 << "max. momentum allowed : " << b_momentumCut  << " GeV/c"
	    << std::endl;
	std::cout
	    << t2 << "max. pt allowed       : " << b_ptCut << " GeV/c"
	    << std::endl;
	std::cout
	    << t2 << "max. tanl allowed     : " << b_tanlCut << " GeV/c"
	    << std::endl;
	std::cout
	    << t2 << "salvage               : " << b_doSalvage << std::endl;
	std::cout
	    << t2 << "salvage level         : " << b_salvageLevel << std::endl;

	std::cout
	    << t1 << _perfectFinder->name() << "("
	    << _perfectFinder->version() << ") options" << std::endl;
	std::cout
	    << t2 << "do finding            : " << b_doPerfectFinder
	    << std::endl;
	std::cout
	    << t2 << "perfect fitting       : " << b_perfectFitting
	    << std::endl;

	std::cout
	    << t1 << _confFinder->name() << "(" << _confFinder->version()
	    << ") options" << std::endl;
	std::cout
	    << t2 << "do finding            : " << b_doConformalFinder
	    << std::endl;
	if (b_conformalFinder == 1) {
	    std::cout
		<< t2 << "fast finder           : " << b_doConformalFastFinder
		<< std::endl;
	    std::cout
		<< t2 << "slow finder           : " << b_doConformalSlowFinder
		<< std::endl;
	    std::cout
		<< t2 << "perfect segment find  : "
		<< b_conformalPerfectSegmentFinding << std::endl;
	    std::cout
		<< t2 << "max sigma             : " << b_conformalMaxSigma
		<< std::endl;
	    std::cout
		<< t2 << "min # hits for segment: "
		<< b_conformalMinNLinksForSegment << std::endl;
	    std::cout
		<< t2 << "min # cores in segment: " << b_conformalMinNCores
		<< std::endl;
	    std::cout
		<< t2 << "min # segments        : " << b_conformalMinNSegments
		<< std::endl;
	    std::cout
		<< t2 << "salvage level         : " << b_salvageLevel
		<< std::endl;
	    std::cout
		<< t2 << "salvage load width    : "
		<< b_conformalSalvageLoadWidth << std::endl;
	    std::cout
		<< t2 << "stereo mode           : " << b_conformalStereoMode
		<< std::endl;
	    std::cout
		<< t2 << "stereo load width     : "
		<< b_conformalStereoLoadWidth << std::endl;
	    std::cout
		<< t2 << "stereo max sigma      : "
		<< b_conformalStereoMaxSigma << std::endl;
	    std::cout
		<< t2 << "stereo segment dist.  : "
		<< b_conformalStereoSzSegmentDistance << std::endl;
	    std::cout
		<< t2 << "stereo link distance  : "
		<< b_conformalStereoSzLinkDistance << std::endl;
	}
	else {
	    std::cout
		<< t2 << "Old trasan(old conf. finder) is active."
		<< std::endl;
	    std::cout
		<< t2 << "do stereo finding     : "
		<< b_doConformalFinderStereo << std::endl;
	    std::cout
		<< t2 << "use cosmic builder    : "
		<< b_doConformalFinderCosmic << std::endl;
	    std::cout
		<< t2 << "max sigma             : " << b_conformalMaxSigma
		<< std::endl;
	    std::cout
		<< t2 << "fraction              : " << b_conformalFraction
		<< std::endl;
	    std::cout
		<< t2 << "fitting corrections   : "
		<< b_conformalFittingCorrections << std::endl;
	    std::cout
		<< t2 << "stereo max sigma      : "
		<< b_conformalStereoMaxSigma << std::endl;
	    std::cout
		<< t2 << "stereo z3             : " << b_conformalStereoZ3
		<< std::endl;
	    std::cout
		<< t2 << "stereo z4             : " << b_conformalStereoZ4
		<< std::endl;
	    std::cout
		<< t2 << "stereo chisq 3        : " << b_conformalStereoChisq3
		<< std::endl;
	    std::cout
		<< t2 << "stereo chisq 4        : " << b_conformalStereoChisq4
		<< std::endl;
	}
	std::cout << t1 << _curlFinder->name();
	std::cout << "(" << _curlFinder->version() << ") options" << std::endl;
	std::cout << t2 << "do finding            : " << b_doCurlFinder
		  << std::endl;
//cnv   if (_clustFinder) {
//      std::cout << t1 << _clustFinder->name();
//      std::cout << "(" << _clustFinder->version() << ") options"
//          << std::endl;
//      std::cout << t2 << "do finding            : " << b_doClustFinder
//          << std::endl;
//      std::cout << t2 << "cathode window        : " << b_cathodeWindow
//          << std::endl;
//      std::cout << t2 << "systematic correction : "
//          << b_cathodeSystematics<<std::endl;
//      std::cout << t2 << "cathode cosmic switch : " << b_cathodeCosmic
//          << std::endl;
//  }
	std::cout << t1 << "Patten Matching CurlFinder"
		  << "(0.2beta) options" << std::endl;
	std::cout << t2 << "do finding                    : "
		  << b_doPMCurlFinder << std::endl;
	std::cout << t2 << "mode(1=rec, 2=map, 3=plot)    : "
		  << b_pmCurlFinder << std::endl;
	std::cout << t2 << "min electrons of svd clusters : "
		  << min_svd_electrons_in_pmc << std::endl;
	std::cout << t1 << "SVD Associator" << "(0.2beta) options"
		  << std::endl;
	std::cout << t2 << "doing : " << b_doSvdAssociator << std::endl;
	if (_houghFinder) {
	    std::cout
		<< t1 << _houghFinder->name() << "("
		<< _houghFinder->version() << ") options" << std::endl;
	    std::cout
		<< t2 << "do finding            : " << b_doHoughFinder
		<< std::endl;
	    std::cout
		<< t2 << "do curl search        : "
		<< b_doHoughFinderCurlSearch << std::endl;
	    std::cout
		<< t2 << "mesh size x           : " << b_houghMeshX
		<< std::endl;
	    std::cout
		<< t2 << "mesh size y           : " << b_houghMeshY
		<< std::endl;
	    std::cout
		<< t2 << "Pt boundary           : " << b_houghPtBoundary
		<< std::endl;
	    std::cout
		<< t2 << "threshold             : " << b_houghThreshold
		<< std::endl;
	    std::cout
		<< t2 << "mesh size x (low Pt)  : " << b_houghMeshXLowPt
		<< std::endl;
	    std::cout
		<< t2 << "mesh size y (low Pt)  : " << b_houghMeshYLowPt
		<< std::endl;
	    std::cout
		<< t2 << "Pt boundary (low Pt)  : " << b_houghPtBoundaryLowPt
		<< std::endl;
	    std::cout
		<< t2 << "threshold  (low Pt)   : " << b_houghThresholdLowPt
		<< std::endl;
	    std::cout
		<< t2 << "axial load width      : " << b_houghAxialLoadWidth
		<< std::endl;
	    std::cout
		<< t2 << "axial load width curl : "
		<< b_houghAxialLoadWidthCurl
		<< std::endl;
	    std::cout
		<< t2 << "max sigma             : " << b_houghMaxSigma
		<< std::endl;
	    std::cout
		<< t2 << "max sigma stereo      : " << b_houghStereoMaxSigma
		<< std::endl;
	    std::cout
		<< t2 << "salvage level         : " << b_houghSalvageLevel
		<< std::endl;
	    std::cout
		<< t2 << "mode                  : " << b_houghMode
		<< std::endl;
	}
    }
    if (def
        || msg.find("tracks") != std::string::npos
        || msg.find("detail") != std::string::npos) {
	_trackManager.dump("eventSummary");
    }
    if (msg.find("cathode") != std::string::npos
        || msg.find("detail") != std::string::npos) {
//  if (b_doClustFinder && _cdccat) {
//      _cdccat->dump("hit");
// //     _clustFinder->dump("");
//  }
    }
}

void
Trasan::banner(void) const {
    std::cout << "\n";
    std::cout << "=========================================================\n";
    std::cout << "      Trasan : Belle Tracking Module                     \n";
    std::cout << "                                    Belle Tracking Group \n";
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << " 1.0 alpha  : 24-Feb-1998 : Y.Iwasaki : Creation            \n";
    std::cout << " 1.0 alpha 1: 24-Apr-1998 : Y.Iwasaki : TRK GRP internal use\n";
    std::cout << " 1.0 alpha 2: 02-May-1998 : Y.Iwasaki :                     \n";
    std::cout << " 1.0 alpha 3: 18-May-1998 : Y.Iwasaki : New Line Finder     \n";
    std::cout << " 1.0 alpha 4: 22-May-1998 : Y.Iwasaki :                     \n";
    std::cout << " 1.0 alpha 5: 25-May-1998 : Y.Iwasaki :                     \n";
    std::cout << " 1.0 alpha 6: 25-May-1998 : Y.Iwasaki : New appendStereo    \n";
    std::cout << " 1.0 alpha 7: 06-Jun-1998 : Y.Iwasaki : clust finder&stereo \n";
    std::cout << " 1.0 alpha 8: 08-Jun-1998 : Y.Iwasaki : NewStereo&curlFinder\n";
    std::cout << " 1.0 beta 1 : 11-Jun-1998 : Y.Iwasaki : code clean          \n";
    std::cout << " 1.0 beta 2 : 12-Jun-1998 : Y.Iwasaki : TTrack::fit bugfixed\n";
    std::cout << " 1.0 beta 3 : 12-Jun-1998 : Y.Iwasaki : code more clean     \n";
    std::cout << " 1.0 beta 4 : 18-Jun-1998 : Y.Iwasaki : rphi improved       \n";
    std::cout << " 1.0 beta 5 : 21-Jun-1998 : Y.Iwasaki : rphi improved       \n";
    std::cout << " 1.0 RC 1   : 29-Jun-1998 : Y.Iwasaki : Release Candidate 1 \n";
    std::cout << " 1.0  : 02-Jul-1998 : Y.Iwasaki : Officail release          \n";
    std::cout << " 1.01 : 06-Jul-1998 : Y.Iwasaki : CurlF on,TLine&fitterfixed\n";
    std::cout << " 1.02 : 07-Jul-1998 : Y.Iwasaki : Trasan bug fixed          \n";
    std::cout << " 1.06 : 29-Jul-1998 : J.Suzuki  : Stereo finder updated     \n";
    std::cout << " 1.07 : 03-Aug-1998 : S.Suzuki  : Cathode part installed    \n";
    std::cout << " 1.08 : 12-Aug-1998 : J.Suzuki  : Stereo finder updated     \n";
    std::cout << " 1.09 : 31-Aug-1998 : J.Tanaka, Y.Iwasaki : CurlF,MC classes\n";
    std::cout << " 1.1 alpha 1: 17-Sep-1998 : Y.Iwasaki : Super Mom. filter   \n";
    std::cout << " 1.1 alpha 2: 24-Sep-1998 : Y.Iwasaki : \n";
    std::cout << " 1.1 alpha 3: 25-Sep-1998 : Y.Iwasaki : cosmic modification\n";
    std::cout << " 1.1 alpha 4: 28-Sep-1998 : Y.Iwasaki : MC tables added\n";
    std::cout << " 1.1 alpha 5: 29-Sep-1998 : T.Matsumoto : cathode fitter\n";
    std::cout << " 1.1 alpha 6: 29-Sep-1998 : J.Suzuki : cosmic z finder\n";
    std::cout << " 1.1 beta 1: 29-Sep-1998 : J.Tanaka : curl finder\n";
    std::cout << " 1.1 beta 2: 30-Sep-1998 : Y.Iwasaki : minor change\n";
    std::cout << " 1.1 beta 3: 06-Oct-1998 : Y.Iwasaki : minor change\n";
    std::cout << " 1.1 beta 4: 09-Oct-1998 : J.Suzuki : memory leak stopped\n";
    std::cout << " 1.1 beta 5: 09-Oct-1998 : Y.Iwasaki : ::term() modified\n";
    std::cout << " 1.1 beta 6: 09-Oct-1998 : Y.Iwasaki : TBuilder bug fixed\n";
    std::cout << " 1.1 beta 7: 13-Oct-1998 : J.Tanaka : memoryleak fixed again\n";
    std::cout << " 1.1 beta 8: 10-Nov-1998 : Y.Iwasaki : negative sqrt fixed\n";
    std::cout << " 1.1 beta 9: 10-Nov-1998 : Y.Iwasaki : more protections\n";
    std::cout << " 1.1 RC 1  : 12-Nov-1998 : Y.Iwasaki : salvaging installed\n";
    std::cout << " 1.1 RC 2  : 19-Nov-1998 : Y.Iwasaki : salvaging improved\n";
    std::cout << " 1.1 RC 3  : 27-Nov-1998 : Y.Iwasaki : salvaging improved\n";
    std::cout << " 1.1 RC 4  : 04-Dec-1998 : Y.Iwasaki : TRGCDC\n";
    std::cout << " 1.1 : 26-Dec-1998 : J.Suzuki, S.Suzuki : stereo&cathode mod\n";
    std::cout << " 1.11: 11-Jan-1999 : Y.Iwasaki : fitter classes added\n";
    std::cout << " 1.12: 19-Jan-1999 : Y.Iwasaki : movePivot problem avoided\n";
    std::cout << " 1.13: 25-Jan-1999 : Y.Iwasaki : salvage improved\n";
    std::cout << " 1.14: 03-Feb-1999 : J.Tanaka, Y.Iwasaki : bug fix\n";
    std::cout << " 1.15: 04-Feb-1999 : S.Suzuki : bug fix in table output\n";
    std::cout << " 1.16: 06-Feb-1999 : Y.Iwasaki : new approach\n";
    std::cout << " 1.17: 09-Feb-1999 : T.Matsumoto, J.Tanaka : cathode, curl\n";;
    std::cout << " 1.18: 21-Feb-1999 : J.Suzuki : Cosmic builder loose cut\n";
    std::cout << " 1.19: 03-Mar-1999 : Y.Iwasaki : WireHitFittingValid added\n";
    std::cout << " 1.20: 07-Mar-1999 : Y.Iwasaki : THelixFitter bug fixed\n";
    std::cout << " 1.21: 09-Mar-1999 : Y.Iwasaki : Conformal finder bug fix\n";
    std::cout << " 1.22: 09-Mar-1999 : Y.Iwasaki : TrackManager bug fix\n";
    std::cout << " 1.23: 10-Mar-1999 : J.Tanaka : Curl finder update\n";
    std::cout << " 1.24: 11-Mar-1999 : Y.Iwasaki : Salvage improved\n";
    std::cout << " 1.25: 11-Mar-1999 : J.Tanaka : Curlfinder update,salvage on\n";
    std::cout << " 1.26: 12-Mar-1999 : Y.Iwasaki : output bug fix\n";
    std::cout << " 1.27: 15-Mar-1999 : J.Tanaka : Curl finder update\n";
    std::cout << " 1.28: 21-Mar-1999 : J.Suzuki, Y.Iwasaki :CosmicBuilder,tMgr\n";
    std::cout << " 1.29: 02-Apr-1999 : J.Tanaka : Curl finder tighter cuts\n";
    std::cout << " 1.30: 07-Apr-1999 : J.Tanaka : Curl finder mask update\n";
    std::cout << " 1.31: 09-Apr-1999 : J.Suzuki, J.Tanaka : CosmicF.and CurlF.\n";
    std::cout << " 1.32: 09-Apr-1999 : Y.Iwasaki : CosmicF. minor change\n";
    std::cout << " 1.33: 18-May-1999 : Y.Iwasaki : Conf.,mask bug fix,T0 calc.\n";
    std::cout << " 1.34: 26-May-1999 : T.Matsumoto, Y.Iwasaki :merge and state\n";
    std::cout << " 1.35 alpha: 28-May-1999 : T.Matsumoto : cathode updates\n";
    std::cout << " 1.35 beta : 28-May-1999 : Y.Iwasaki : movePivot fix\n";
    std::cout << " 1.35 : 28-May-1999 : Y.Iwasaki : minor modifictions\n";
    std::cout << " 1.36 : 06-Jun-1999 : Y.Iwasaki : Trasan T0 output\n";
    std::cout << " 1.37 : 08-Jun-1999 : Y.Iwasaki : TrkMgr bug fix\n";
    std::cout << " 1.38 : 09-Jun-1999 : Y.Iwasaki : update added\n";
    std::cout << " 1.39 : 10-Jun-1999 : H.Ozaki : approach fix\n";
    std::cout << " 1.40 : 10-Jun-1999 : Y.Iwasaki : minor modifications\n";
    std::cout << " 1.41 : 14-Jun-1999 : J.Tanaka : Curl finder update\n";
    std::cout << " 1.42 : 14-Jun-1999 : Y.Iwasaki : findCloseHits bug fix\n";
    std::cout << " 1.43 : 15-Jun-1999 : T.Matsumoto, Y.Iwasaki : cathode,confB\n";
    std::cout << " 1.44 : 16-Jun-1999 : Y.Iwasaki : CosmicF merged into THelixF\n";
    std::cout << " 1.441: 17-Jun-1999 : Y.Iwasaki : default Mom.cut=0.\n";
    std::cout << " 1.45 : 17-Jun-1999 : Y.Iwasaki : t0 determined by 2D fit\n";
    std::cout << " 1.46 : 25-Jun-1999 : Y.Iwasaki : test version\n";
    std::cout << " 1.46a: 25-Jun-1999 : Y.Iwasaki : test version upto 2D\n";
    std::cout << " 1.47a: 25-Jun-1999 : Y.Iwasaki : TRGCDC reads valid hits only\n";
    std::cout << " 1.48a: 25-Jun-1999 : Y.Iwasaki : tighter hit selection\n";
    std::cout << " 1.49a: 29-Jun-1999 : Y.Iwasaki : TRGCDC for invalid hits\n";
    std::cout << " 1.50a: 30-Jun-1999 : Y.Iwasaki : TRGCDC bug fix\n";
    std::cout << " 1.51a: 30-Jun-1999 : Y.Iwasaki : TRGCDC & Curl bug fix,T0 mod\n";
    std::cout << " 1.52a: 06-Jul-1999 : Y.Iwasaki : TRGCDC bug,output assc. hits\n";
    std::cout << " 1.53a: 07-Jul-1999 : T.Matsumoto : cathode updates \n";
    std::cout << " 1.54a: 12-Jul-1999 : N.Katayama : approch, hit #\n";
    std::cout << " 1.55b: 15-Jul-1999 : JT, YI : curl, fitter, mc, fast\n";
    std::cout << " 1.56b: 18-Jul-1999 : JS, YI : HFit tof, TraTime tof\n";
    std::cout << " 1.57b: 23-Jul-1999 : * : minor changes\n";
    std::cout << " 1.58b: 26-Jul-1999 : Y.Iwasaki : protection for bad tracks\n";
    std::cout << " 1.59b: 03-Aug-1999 : Y.Iwasaki : put_parameter added\n";
    std::cout << " 1.60b: 24-Aug-1999 : J.T, Y.I : curl updated, fitting flag\n";
    std::cout << " 1.61b: 10-Sep-1999 : J.S, Y.I : put_parameer, MC bug\n";
    std::cout << " 1.62b: 13-Sep-1999 : H.Ozaki : sag2 -> sag3\n";
    std::cout << " 1.63b: 15-Oct-1999 : Y.I, J.T : TWindow hidden, Curl param\n";
    std::cout << " 1.64b: 20-Oct-1999 : H.Ozaki : New methode for T0 det.\n";
    std::cout << " 1.65b: 21-Oct-1999 : Y.Iwasaki : conf version option added\n";
    std::cout << " 1.65c: 21-Oct-1999 : Y.Iwasaki : new fast conf with 3D\n";
    std::cout << " 1.65d: 04-Nov-1999 : Y.Iwasaki : stereo modified\n";
    std::cout << " 1.66d: 08-Jan-2000 : Y.Iwasaki : debug info. modified\n";
    std::cout << " 1.67d: 12-Jan-2000 : Y.Iwasaki : tuned,bug in masking fixed\n";
    std::cout << " 1.67e: 18-Jan-2000 : Y.Iwasaki : TWindow modified\n";
    std::cout << " 1.67f: 19-Jan-2000 : Y.Iwasaki : new conf modified\n";
    std::cout << " 1.67g: 23-Jan-2000 : Y.Iwasaki : new conf, slow finding\n";
    std::cout << " 1.67h: 28-Jan-2000 : Y.Iwasaki : new conf modified\n";
    std::cout << " 1.67i: 28-Jan-2000 : Y.Iwasaki : new conf modified\n";
    std::cout << " 1.68j: 28-Jan-2000 : JT, YI : curl and new conf modified\n";
    std::cout << " 1.68k: 03-Feb-2000 : Y.Iwasaki : slow in new conf off\n";
    std::cout << " 1.68l: 09-Feb-2000 : JT, YI : curl and conf stereo updated\n";
    std::cout << " 2.00RC1: 10-Feb-2000 : JT, YI : conf bug fix, param\n";
    std::cout << " 2.00RC2: 15-Feb-2000 : JT, YI : bug fixes\n";
    std::cout << " 2.00RC3: 17-Feb-2000 : YI : bug fixes\n";
    std::cout << " 2.00RC5: 23-Feb-2000 : YI : refineLinks\n";
    std::cout << " 2.00RC6: 24-Feb-2000 : YI : quality2D bug\n";
    std::cout << " 2.00RC7: 24-Feb-2000 : YI : quality2D bug again\n";
    std::cout << " 2.00RC8: 25-Feb-2000 : AI : robust fitter bug\n";
    std::cout << " 2.00RC9: 25-Feb-2000 : AI : stereo bug\n";
    std::cout << " 2.00RC10: 25-Feb-2000 : YI : stereo improved\n";
    std::cout << " 2.00RC11: 28-Feb-2000 : JT : curl updates\n";
    std::cout << " 2.00RC12: 28-Feb-2000 : YI : salvage and t0 fix \n";
    std::cout << " 2.00RC13: 29-Feb-2000 : YI : default stereo param changed \n";
    std::cout << " 2.00RC14: 01-Mar-2000 : YI, JT :stereo bug fix,curl update \n";
    std::cout << " 2.00RC15: 07-Mar-2000 : YI : debug info. changed \n";
    std::cout << " 2.00RC16: 16-Mar-2000 : YI : # of hits improved(slightly) \n";
    std::cout << " 2.00RC17: 21-Mar-2000 : JT, YI : curler treatments \n";
    std::cout << " 2.00RC18: 23-Mar-2000 : YI : bug fix \n";
    std::cout << " 2.00RC19: 24-Mar-2000 : YI : track manager \n";
    std::cout << " 2.00RC20: 24-Mar-2000 : YI : bug fix \n";
    std::cout << " 2.00RC21: 26-Mar-2000 : YI : track manager bug fix \n";
    std::cout << " 2.00RC22: 29-Mar-2000 : JT : pm finder included \n";
    std::cout << " 2.00RC23: 31-Mar-2000 : JT, YI :pmf updates,trkmgr bug fix \n";
    std::cout << " 2.00RC24: 31-Mar-2000 : YI : trkmgr another bug fix \n";
    std::cout << " 2.00RC25: 03-Apr-2000 : YI : trkmgr bug again \n";
    std::cout << " 2.00RC26: 04-Apr-2000 : JT, YI : pmf update, conf salv. mod\n";
    std::cout << " 2.00RC27: 04-Apr-2000 : YI :conf. bad pnt rej,mod. for dEdx\n";
    std::cout << " 2.00RC28: 07-Apr-2000 : JT, YI : curl bad pnt, table\n";
    std::cout << " 2.00RC29: 13-Apr-2000 : YI : minor changes\n";
    std::cout << " 2.00RC30: 14-Apr-2000 : YI : memory leak, resolve multih\n";
    std::cout << " 2.00RC31: 24-Apr-2000 : YI : nhits, nster, ndf\n";
    std::cout << " 2.00RC32: 24-Apr-2000 : YI : fix again\n";
    std::cout << " 2.00 : 03-May-2000 : YI : now 2.00 (no change from rc32)\n";
    std::cout << " 2.01 : 06-Jun-2000 : HO :";
    std::cout << " condition changed for 6dim t0 fitting\n";
    std::cout << " 2.02 : 09-Jun-2000 : YI : RECCDC_MCTRK sorting in trkmgr\n";
    std::cout << " 2.03 : 27-Jun-2000 : YI : buf fix of RECCDC_MCTRK sort\n";
    std::cout << " 2.04 : 22-Aug-2000 : YI : Perfect Finder added\n";
    std::cout << " 2.05 : 27-Sep-2000 : JT : updates of cf & pmf\n";
    std::cout << " 2.06 : 30-Sep-2000 : JT : updates of trkmgr\n";
    std::cout << " 2.07 : 02-Oct-2000 : JT : updates of pmf & trkmgr\n";
    std::cout << " 2.08 : 03-Oct-2000 : JT : updates of trkmgr\n";
    std::cout << " 2.09 : 06-Oct-2000 : YI : TLink has drift time\n";
    std::cout << " 2.10 : 07-Oct-2000 : JT : updates of trkmgr\n";
    std::cout << " 2.11 : 10-Oct-2000 : JT : updates of curl builder\n";
    std::cout << " 2.12 : 11-Oct-2000 : YI : #hits protection,chi2 sorting\n";
    std::cout << " 2.13 : 11-Oct-2000 : JT : PMF cdc seed off\n";
    std::cout << " 2.14 : 13-Oct-2000 : JT,YI : trkmgr\n";
    std::cout << " 2.15 : 13-Oct-2000 : YI : trkmgr bug fix\n";
    std::cout << " 2.16 : 13-Oct-2000 : JT : PMF updates\n";
    std::cout << " 2.17 : 23-Oct-2000 : JT : curl 2D output\n";
    std::cout << " 2.18 : 17-Nov-2000 : YI : option for new CDC\n";
    std::cout << " 2.19 : 09-Jan-2001 : YI : minor changes\n";
    std::cout << " 2.20 : 29-Jan-2001 : * : 2D&cosmic outputs, curler decision\n";
    std::cout << " 2.21 : 30-Jan-2001 : YI : still debugging\n";
    std::cout << " 2.22 : 31-Jan-2001 : JT,YI : still debugging\n";
    std::cout << " 2.23 : 31-Jan-2001 : JT : new output style finished\n";
    std::cout << " 2.24 : 31-Jan-2001 : JT : curl fix\n";
    std::cout << " 2.241: 31-Jan-2001 : JT : curl fix\n";
    std::cout << " 2.25 : 01-Feb-2001 : YI : conf fix\n";
    std::cout << " 2.26 : 05-Feb-2001 : YI : bug fix for data\n";
    std::cout << " 2.27 : 08-Feb-2001 : YI : pivot of 2D trk\n";
    std::cout << " 2.28 : 08-Feb-2001 : YI : conf shrdmem size adjusted\n";
    std::cout << " 2.29 : 09-Apr-2001 : YI : memory leak fixed\n";
    std::cout << " 3.00RC1: 28-Mar-2001 : YI : conf new stereo\n";
    std::cout << " 3.00RC2: 09-Apr-2001 : YI : test small changes\n";
    std::cout << " 3.00RC2.1: 11-Apr-2001 : YI : RC2 bugs, ML in PMF fixed\n";
    std::cout << " 3.00RC3: 12-Apr-2001 : YI : bugs in mode 1,2 fixed\n";
    std::cout << " 3.00RC4: 12-Apr-2001 : JT : curl new stereo\n";
    std::cout << " 3.00RC5: 18-Apr-2001 : JT : helix fitter speed up\n";
    std::cout << " 3.00RC6: 25-Apr-2001 : YI : helix fitter speed up\n";
    std::cout << " 3.00 : 27-Apr-2001 : new stereo finder\n";
    std::cout << " 3.01 : 18-May-2001 : perfect segment finder in conf.\n";
    std::cout << " 3.02 : 14-Feb-2002 : YI : bug fixes in debug mode\n";
    std::cout << " 3.03 : 14-Feb-2002 : YI : T0 reset test mode\n";
    std::cout << " 3.04 : 10-Sep-2003 : YI : for small cell CDC\n";
    std::cout << " 3.05 : 06-Oct-2003 : YI : PMCurl off in exp>=30\n";
    std::cout << " 3.06 : 19-Dec-2003 : YI : scdc included, Hough added\n";
    std::cout << " 3.07 : 25-Dec-2003 : YI : curl for scdc\n";
    std::cout << " 3.08 : 01-Jan-2004 : YI : buf fix in curl version\n";
    std::cout << " 3.09 : 18-Feb-2004 : YI : small cell simulation added\n";
    std::cout << " 3.10 : 26-Mar-2004 : YI : curl=taigr+junk rejection\n";
    std::cout << " 3.11 : 15-Apr-2004 : YI : trkmgr supports low-p tracks\n";
    std::cout << " 3.12 : 23-Apr-2004 : YI : curl version=2 is default\n";
    std::cout << " 3.13 : 26-Apr-2004 : YI : trkmgr MC analysis bug fixed\n";
    std::cout << " 3.14 : 28-Apr-2004 : YI : param doMCAnalysis removed\n";
    std::cout << " 3.23 : 28-Feb-2007 : YI : nan protection added\n";
    std::cout << " 3.24 : 02-Mar-2007 : YI : CurlFinder modified\n";
    std::cout << " 3.25 : 20-Apr-2007 : YI : TWindowGTK modified\n";
    std::cout << " 4.00 RC1 : 05-Dec-2007 : YI : HoughFinder release cand. \n";
    std::cout << " 4.00 RC2 : 25-Dec-2007 : YI : HoughFinder tuned         \n";
    std::cout << " 4.01     : 20-Jun-2008 : YI : Hough+Conf+Curl \n";
    std::cout << " 4.02     : 12-Sep-2008 : YI : Hough builder modified\n";
    std::cout << " 4.03     : 18-Sep-2008 : YI : houghMode=3 bug fix\n";
    std::cout << " 4.04     : 22-Sep-2008 : YI : valgrinded\n";
#endif
    std::cout << " 5.00     : 29-Sep-2010 : YI : for Belle2\n";
    std::cout << " 5.02     : 07-Dec-2012 : YI : Sorting wires correctly\n";
    std::cout << "=========================================================\n";
    std::cout << version() << " notices " << std::endl;
//  std::cout << "  This is a test version for the production 2001 summar.\n";
//  std::cout << "  Last purified : 25-Apr-2001 with 3.00RC6" << std::endl;
//  std::cout << "  Default finder is still Conformal + Curl Finders\n";
    std::cout << std::endl;
}

void
Trasan::beginRun() {

//cnv   Belle_version_Manager &bvMgr = Belle_version_Manager::get_manager();
//   if( bvMgr.count() > 0 ) {
//     bvMgr[0].CDC(b_doHoughFinder);
//   } else if( bvMgr.count() == 0 ) {
//     Belle_version & bv = bvMgr.add();
//     bv.CDC(b_doHoughFinder);
//   }

//     //...Access Belle_Runhead...
//     const belle_runhead & r = * (belle_runhead *) BsGetEnt(BELLE_RUNHEAD,
//                 1,
//                 BBS_No_Index);
//     if (! (& r))
//  std::cout << "Trasan::begin_run !!! BELLE_RUNHEAD not found"
//      << std::endl
//      << "           Normal cell CDC is selected" << std::endl;

//     //...MC analysis...
//     if (& r) {
//  if (r.m_ExpMC == 2)
//    //  if (r.m_ExpMC == 2 && r.m_ExpNo <= 1000)
//    // g4superb does not fill MC hits yet
//      b_doMCAnalysis = 1;
//  else
//      b_doMCAnalysis = 0;
//     }

//     //...Check CDC version...
//     _cdcVersion = "normal cell";
//     b_doPMCurlFinder = _doPMCurlFinder;
//     if (b_cdcVersion == 0) {
//  if (& r) {
//      if (r.m_ExpNo > 29) {
//    _cdcVersion = "small cell";
//    b_doPMCurlFinder = 0;
//      }
//  }
//     }
//     else if (b_cdcVersion == 2) {
//  _cdcVersion = "small cell";
//  b_doPMCurlFinder = 0;
//     }
//     else if (b_cdcVersion > 10) {
//  _cdcVersion = "superb";
//  b_doPMCurlFinder = 0;
//  //  b_doCurlFinder = 0;
//     }

//     //...Create TRGCDC...
//     _cdc = TRGCDC::getTRGCDC(_cdcVersion);
//     _cdc->debugLevel(b_debugLevel);
// //  _cdc->simulateSmallCell(b_simulateSmallCell);
//     TLink::initializeBuffers();
// #if defined(TRASAN_DEBUG)
//     _cdc->dump("geometry layers");
// #endif

//     //...Obtain fudge factor...
//     float fudgeFactor = b_fudgeFactor;
//     if (fudgeFactor == 0.) {
//    const calcdc_const4 & c = * (calcdc_const4 *) BsGetEnt(CALCDC_CONST4,
//                     1,
//                       BBS_No_Index);
//  if (!(& c)) fudgeFactor = 1.;
//    else        fudgeFactor = c.m_fudge;
//     }

//     //...Turn off fudge factor before checking performance...
//     fudgeFactor = 1.0;

//     _cdc->fudgeFactor(fudgeFactor);

//     //...Create Cathode...
//     _cdccat = NULL;
//     //    if (_cdcVersion.toFloat() > 0) {
// //     if (atof(_cdcVersion.c_str()) > 0) {
// //   if (! _cdccat) _cdccat = new TRGCDCCat();
// //   _cdccat->debugLevel(b_debugLevel);
// //     }
// //     else {
// //   b_doClustFinder = 0;
// //     }

//     //...Initialize finders...
//     if (_houghFinder)
//  _houghFinder->init();
}

void
Trasan::endRun() {
}

void
Trasan::selectUnusedHits(const CAList<Belle2::TRGCDCWireHit> & hits,
			 CAList<Belle2::TRGCDCWireHit> & unusedHits) const {

    //...Pick up used hits (3D tracks only)...
    CAList<Belle2::TRGCDCWireHit> used;
    const AList<TTrack> & tracks = _trackManager.tracks();
    for (unsigned i = 0; i < (unsigned) tracks.length(); i++) {
	const TTrack& t = * tracks[i];
	const AList<TLink> & links = t.links();
	const unsigned n = links.length();
	for (unsigned j = 0; j < n; j++)
	    used.append((Belle2::TRGCDCWireHit*) links[j]->hit());
    }

    unusedHits = hits;
    unusedHits.remove(used);
}

void
Trasan::main0(const CAList<Belle2::TRGCDCWireHit> & axialHits,
	      const CAList<Belle2::TRGCDCWireHit> & stereoHits,
	      const CAList<Belle2::TRGCDCWireHit> & allHits,
	      AList<TTrack> & tracks,
	      AList<TTrack> & tracks2D) {

start:
    unsigned nT0Reset = 0;

    //...Conformal finder...
    if (b_doConformalFinder) {

	//...T0 reset option...
	if (b_doT0Reset) {
	    if ((unsigned) b_nT0ResetMax > nT0Reset)
		((TConformalFinder*) _confFinder)->doT0Reset(true);
	    else
		((TConformalFinder*) _confFinder)->doT0Reset(false);
	}

//      AList<TRGCDCWireHit> unusedAxial;
//      AList<TRGCDCWireHit> unusedStereo;
//      selectUnusedHits(axialHits, unusedAxial);
//      selectUnusedHits(stereoHits, unusedStereo);
//      _confFinder->doit(unusedAxial, unusedStereo, tracks, tracks2D);
	_confFinder->doit(axialHits, stereoHits, tracks, tracks2D);

	//...T0 reset...
	if (b_doT0Reset) {
	    ++nT0Reset;
	    if (((TConformalFinder*) _confFinder)->T0ResetDone()) {
		clear();
		goto start;
	    }
	}

	//...Stores tracks...
	_trackManager.append(tracks);
	if (curl_version < 2)
	    _trackManager.append2D(tracks2D);
	if (b_conformalFinder == 0) {
	    if (b_doSalvage == 1) _trackManager.salvage(allHits);
	    if (b_doSalvage) _trackManager.mask();
	}
    }

    //...Hough finder...
    if (b_doHoughFinder) {
	((THoughFinder*) _houghFinder)->
	    doit(axialHits, stereoHits, tracks, tracks2D);
	_trackManager.append(tracks);
	_trackManager.append2D(tracks2D);
	_trackManager.refit();
    }

    //...Curl finder...
    if (b_doCurlFinder) {
	if ((! b_doSalvage) && (b_conformalFinder == 0)) {
	    _trackManager.maskCurlHits(axialHits,
				       stereoHits,
				       _trackManager.tracks());
	}

//      AList<TRGCDCWireHit> unusedAxial;
//      AList<TRGCDCWireHit> unusedStereo;
//      selectUnusedHits(axialHits, unusedAxial);
//      selectUnusedHits(stereoHits, unusedStereo);

	AList<TTrack> confTracks = _trackManager.tracks();
	tracks.append(confTracks);
//      _curlFinder->doit(unusedAxial, unusedStereo, tracks, tracks2D);
	_curlFinder->doit(axialHits, stereoHits, tracks, tracks2D);
	tracks.remove(confTracks);
	//_trackManager.append(tracks);
    }

    //...Finishes tracks...
    if ((b_doSalvage) && (b_conformalFinder == 0)) _trackManager.refit();

    //...Appends tracks which are reconstructed by CurlFinder...
    _trackManager.append(tracks);
    _trackManager.append2D(tracks2D);

    //...PM Curl finder...
    if (b_doPMCurlFinder) {
//cnv   _pmCurlFinder->doit(axialHits,
//          stereoHits,
//          _trackManager.tracks(),
//          b_pmCurlFinder);
//  _trackManager.append(_pmCurlFinder->getTracks());

// #ifdef TRASAN_DEBUG_DETAIL
//  std::cout << "Track list after PM Curl finder" << std::endl;
//  _trackManager.dump("eventSummary helix", Tab(+1));
// #endif
    }

    //...Merge & Mask ...
    if ((b_mergeCurls) && (b_conformalFinder == 0)) _trackManager.merge();
    if (b_conformalFinder != 0) _trackManager.setCurlerFlags();

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "main0 : Track list after merge and mask" << std::endl;
    _trackManager.dump("eventSummary helix", Tab(+1));
#endif

    //...Track merge...
    if (b_mergeTracks)
	_trackManager.mergeTracks(b_mergeTracks, b_mergeTrackDistance);

    //...Salvage for dE/dx...
    if (b_doAssociation)
	_trackManager.salvageAssociateHits(allHits, b_associateSigma);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "main0 : Track list after salvage" << std::endl;
    _trackManager.dump("eventSummary helix", Tab(+1));
#endif
}

void
Trasan::main1(const CAList<Belle2::TRGCDCWireHit> & axialHits,
	      const CAList<Belle2::TRGCDCWireHit> & stereoHits,
	      const CAList<Belle2::TRGCDCWireHit> & allHits,
	      AList<TTrack> & tracks,
	      AList<TTrack> & tracks2D) {

    //...Hough finder...
    if (b_doHoughFinder) {
	((THoughFinder*) _houghFinder)->
	    doit(axialHits, stereoHits, tracks, tracks2D);
	_trackManager.append(tracks);
	_trackManager.append2D(tracks2D);
	_trackManager.refit();
    }

    //...Conformal finder...
    if (b_doConformalFinder) {

	CAList<Belle2::TRGCDCWireHit> unusedAxial;
	CAList<Belle2::TRGCDCWireHit> unusedStereo;
	selectUnusedHits(axialHits, unusedAxial);
	selectUnusedHits(stereoHits, unusedStereo);

	_confFinder->doit(unusedAxial, unusedStereo, tracks, tracks2D);

	//...Stores tracks...
	_trackManager.append(tracks);
	if (curl_version < 2)
	    _trackManager.append2D(tracks2D);
	if (b_conformalFinder == 0) {
	    if (b_doSalvage == 1) _trackManager.salvage(allHits);
	    if (b_doSalvage) _trackManager.mask();
	}
    }

    //...Curl finder...
    if (b_doCurlFinder) {
	if ((! b_doSalvage) && (b_conformalFinder == 0)) {
	    _trackManager.maskCurlHits(axialHits,
				       stereoHits,
				       _trackManager.tracks());
	}

	AList<TTrack> confTracks = _trackManager.tracks();
	tracks.append(confTracks);
	_curlFinder->doit(axialHits, stereoHits, tracks, tracks2D);
	tracks.remove(confTracks);
    }

    //...Finishes tracks...
    if ((b_doSalvage) && (b_conformalFinder == 0)) _trackManager.refit();

    //...Appends tracks which are reconstructed by CurlFinder...
    _trackManager.append(tracks);
    _trackManager.append2D(tracks2D);

    //...Merge & Mask ...
    if ((b_mergeCurls) && (b_conformalFinder == 0)) _trackManager.merge();
    if (b_conformalFinder != 0) _trackManager.setCurlerFlags();

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "main1 : Track list after merge and mask" << std::endl;
    _trackManager.dump("eventSummary helix", Tab(+1));
#endif

    //...Track merge...
    if (b_mergeTracks)
	_trackManager.mergeTracks(b_mergeTracks, b_mergeTrackDistance);

    //...Salvage for dE/dx...
    if (b_doAssociation)
	_trackManager.salvageAssociateHits(allHits, b_associateSigma);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "main1 : Track list after salvage" << std::endl;
    _trackManager.dump("eventSummary helix", Tab(+1));
#endif
}

} // namespace Belle
