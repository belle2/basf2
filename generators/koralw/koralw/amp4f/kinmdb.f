C----- kinmdb.f ---------------------------------- 96/05/08 (TI) ---
      subroutine kinmdb
      implicit DOUBLE PRECISION(a-h,o-z)
      parameter (mproc=76)
      dimension  ngd(mproc),irs1d(mproc),ars1md(mproc),ars1gd(mproc),
     .           irs2d(mproc),ars2md(mproc),ars2gd(mproc),
     .           irs3d(mproc),ars3md(mproc),ars3gd(mproc),
     .           irs4d(mproc),ars4md(mproc),ars4gd(mproc),
     .           ics3d(mproc),ics5d(mproc),iph6d(mproc),
     .           icq3d(mproc),iswpd(mproc),swpmd(mproc),
     .           idntd(mproc),jtgmd(mproc)
C------ common block ----
      include 'inclk.f'
      common /grc4fs/ nthprc
      common /kmcntl/iresns(4),icos3,icosq3,icos5,isr,iswap,ident,iphi6
      common /cut001/coscut(2,4),engyct(2,4),amasct(2,6),aresns(2,4)
     .,opncut,swapm2
C------ common block ----
C 1  nu-e,positron,electron,nu-e-bar
      ngd( 1)    = 56
      irs1d( 1) = 1
      ars1md( 1) = amw
      ars1gd( 1) = agw
      irs2d( 1) = 1
      ars2md( 1) = amw
      ars2gd( 1) = agw
      irs3d( 1) = 1
      ars3md( 1) = amz
      ars3gd( 1) = agz
      irs4d( 1) = 3
      ars4md( 1) = amz
      ars4gd( 1) = agz
      ics3d( 1) = 0
      ics5d( 1) = 5
      iph6d( 1) = -999
      icq3d( 1) = 2
      iswpd( 1) = 1
      swpmd( 1) = -999.
      idntd( 1) = 0
      jtgmd( 1) = 1

C 2  electron,nu-e-bar,anti-muon,nu-mu
      ngd( 2)    = 22
      irs1d( 2) = 1
      ars1md( 2) = amw
      ars1gd( 2) = agw
      irs2d( 2) = 1
      ars2md( 2) = amw
      ars2gd( 2) = agw
      irs3d( 2) = -255
      ars3md( 2) = 0
      ars3gd( 2) = 0
      irs4d( 2) = -255
      ars4md( 2) = 0
      ars4gd( 2) = 0
      ics3d( 2) = 1
      ics5d( 2) = 0
      iph6d( 2) = 0
      icq3d( 2) = 2
      iswpd( 2) = 0
      swpmd( 2) = -999.
      idntd( 2) = 0
      jtgmd( 2) = 1

C 3  electron,nu-e-bar,anti-tau,nu-tau
      ngd( 3)    = 22
      irs1d( 3) = 1
      ars1md( 3) = amw
      ars1gd( 3) = agw
      irs2d( 3) = 1
      ars2md( 3) = amw
      ars2gd( 3) = agw
      irs3d( 3) = -255
      ars3md( 3) = 0
      ars3gd( 3) = 0
      irs4d( 3) = -255
      ars4md( 3) = 0
      ars4gd( 3) = 0
      ics3d( 3) = 1
      ics5d( 3) = 0
      iph6d( 3) = 0
      icq3d( 3) = 2
      iswpd( 3) = 0
      swpmd( 3) = -999.
      idntd( 3) = 0
      jtgmd( 3) = 1

C 4  nu-mu,anti-muon,muon,nu-mu-bar
      ngd( 4)    = 32
      irs1d( 4) = 1
      ars1md( 4) = amw
      ars1gd( 4) = agw
      irs2d( 4) = 1
      ars2md( 4) = amw
      ars2gd( 4) = agw
      irs3d( 4) = 1
      ars3md( 4) = amz
      ars3gd( 4) = agz
      irs4d( 4) = 3
      ars4md( 4) = amz
      ars4gd( 4) = agz
      ics3d( 4) = 0
      ics5d( 4) = 5
      iph6d( 4) = -999
      icq3d( 4) = 2
      iswpd( 4) = 1
      swpmd( 4) = -999.
      idntd( 4) = 0
      jtgmd( 4) = 0

C 5  nu-tau,anti-tau,tau,nu-tau-bar
      ngd( 5)    = 32
      irs1d( 5) = 1
      ars1md( 5) = amw
      ars1gd( 5) = agw
      irs2d( 5) = 1
      ars2md( 5) = amw
      ars2gd( 5) = agw
      irs3d( 5) = 1
      ars3md( 5) = amz
      ars3gd( 5) = agz
      irs4d( 5) = 3
      ars4md( 5) = amz
      ars4gd( 5) = agz
      ics3d( 5) = 0
      ics5d( 5) = 5
      iph6d( 5) = -999
      icq3d( 5) = 2
      iswpd( 5) = 1
      swpmd( 5) = -999.
      idntd( 5) = 0
      jtgmd( 5) = 0

C 6  muon,nu-mu-bar,anti-tau,nu-tau
      ngd( 6)    = 21
      irs1d( 6) = 1
      ars1md( 6) = amw
      ars1gd( 6) = agw
      irs2d( 6) = 1
      ars2md( 6) = amw
      ars2gd( 6) = agw
      irs3d( 6) = -255
      ars3md( 6) = 0
      ars3gd( 6) = 0
      irs4d( 6) = -255
      ars4md( 6) = 0
      ars4gd( 6) = 0
      ics3d( 6) = 0
      ics5d( 6) = 0
      iph6d( 6) = 0
      icq3d( 6) = 2
      iswpd( 6) = 0
      swpmd( 6) = -999.
      idntd( 6) = 0
      jtgmd( 6) = 0

C 7  electron,positron,electron,positron
      ngd( 7)    = 144
      irs1d( 7) = -1
      ars1md( 7) = amz
      ars1gd( 7) = agz
      irs2d( 7) = 3
      ars2md( 7) = amz
      ars2gd( 7) = agz
      irs3d( 7) = 3
      ars3md( 7) = amz
      ars3gd( 7) = agz
      irs4d( 7) = 3
      ars4md( 7) = amz
      ars4gd( 7) = agz
      ics3d( 7) = 2
      ics5d( 7) = 5
      iph6d( 7) = 0
      icq3d( 7) = 0
      iswpd( 7) = 0
      swpmd( 7) = -999.
      idntd( 7) = 3
      jtgmd( 7) = 1

C 8  electron,positron,muon,anti-muon
      ngd( 8)    = 50
      irs1d( 8) = 2
      ars1md( 8) = amz
      ars1gd( 8) = agz
      irs2d( 8) = 3
      ars2md( 8) = amz
      ars2gd( 8) = agz
      irs3d( 8) = -255
      ars3md( 8) = 0
      ars3gd( 8) = 0
      irs4d( 8) = -255
      ars4md( 8) = 0
      ars4gd( 8) = 0
      ics3d( 8) = 2
      ics5d( 8) = 0
      iph6d( 8) = 0
      icq3d( 8) = 2
      iswpd( 8) = 0
      swpmd( 8) = -999.
      idntd( 8) = 0
      jtgmd( 8) = 1

C 9  electron,positron,tau,anti-tau
      ngd( 9)    = 50
      irs1d( 9) = 2
      ars1md( 9) = amz
      ars1gd( 9) = agz
      irs2d( 9) = 3
      ars2md( 9) = amz
      ars2gd( 9) = agz
      irs3d( 9) = -255
      ars3md( 9) = 0
      ars3gd( 9) = 0
      irs4d( 9) = -255
      ars4md( 9) = 0
      ars4gd( 9) = 0
      ics3d( 9) = 2
      ics5d( 9) = 0
      iph6d( 9) = 0
      icq3d( 9) = 2
      iswpd( 9) = 0
      swpmd( 9) = -999.
      idntd( 9) = 0
      jtgmd( 9) = 1

C 10  muon,anti-muon,muon,anti-muon
      ngd(10)    = 88
      irs1d(10) = 3
      ars1md(10) = amz
      ars1gd(10) = agz
      irs2d(10) = 3
      ars2md(10) = amz
      ars2gd(10) = agz
      irs3d(10) = -1
      ars3md(10) = 0
      ars3gd(10) = 0
      irs4d(10) = 3
      ars4md(10) = amz
      ars4gd(10) = agz
      ics3d(10) = 0
      ics5d(10) = 0
      iph6d(10) = 0
      icq3d(10) = 2
      iswpd(10) = 1
      swpmd(10) = 20.
      idntd(10) = 2
      jtgmd(10) = 0

C 11  tau,anti-tau,tau,anti-tau
      ngd(11)    = 88
      irs1d(11) = 3
      ars1md(11) = amz
      ars1gd(11) = agz
      irs2d(11) = 3
      ars2md(11) = amz
      ars2gd(11) = agz
      irs3d(11) = -1
      ars3md(11) = 0
      ars3gd(11) = 0
      irs4d(11) = 3
      ars4md(11) = amz
      ars4gd(11) = agz
      ics3d(11) = 0
      ics5d(11) = 0
      iph6d(11) = 0
      icq3d(11) = 2
      iswpd(11) = 1
      swpmd(11) = 20.
      idntd(11) = 2
      jtgmd(11) = 0

C 12  muon,anti-muon,tau,anti-tau
      ngd(12)    = 44
      irs1d(12) = 3
      ars1md(12) = amz
      ars1gd(12) = agz
      irs2d(12) = 3
      ars2md(12) = amz
      ars2gd(12) = agz
      irs3d(12) = -255
      ars3md(12) = 0
      ars3gd(12) = 0
      irs4d(12) = -255
      ars4md(12) = 0
      ars4gd(12) = 0
      ics3d(12) = 0
      ics5d(12) = 0
      iph6d(12) = 0
      icq3d(12) = 2
      iswpd(12) = 0
      swpmd(12) = -999.
      idntd(12) = 0
      jtgmd(12) = 0

C 13  electron,positron,nu-mu,nu-mu-bar
      ngd(13)    = 20
      irs1d(13) = 2
      ars1md(13) = amz
      ars1gd(13) = agz
      irs2d(13) = 1
      ars2md(13) = amz
      ars2gd(13) = agz
      irs3d(13) = -255
      ars3md(13) = 0
      ars3gd(13) = 0
      irs4d(13) = -255
      ars4md(13) = 0
      ars4gd(13) = 0
      ics3d(13) = 1
      ics5d(13) = 0
      iph6d(13) = 0
      icq3d(13) = 2
      iswpd(13) = 0
      swpmd(13) = -999.
      idntd(13) = 0
      jtgmd(13) = 1

C 14  electron,positron,nu-tau,nu-tau-bar
      ngd(14)    = 20
      irs1d(14) = 2
      ars1md(14) = amz
      ars1gd(14) = agz
      irs2d(14) = 1
      ars2md(14) = amz
      ars2gd(14) = agz
      irs3d(14) = -255
      ars3md(14) = 0
      ars3gd(14) = 0
      irs4d(14) = -255
      ars4md(14) = 0
      ars4gd(14) = 0
      ics3d(14) = 1
      ics5d(14) = 0
      iph6d(14) = 0
      icq3d(14) = 2
      iswpd(14) = 0
      swpmd(14) = -999.
      idntd(14) = 0
      jtgmd(14) = 1

C 15  nu-e,nu-e-bar,muon,anti-muon
      ngd(15)    = 21
      irs1d(15) = 1
      ars1md(15) = amz
      ars1gd(15) = agz
      irs2d(15) = 3
      ars2md(15) = amz
      ars2gd(15) = agz
      irs3d(15) = -255
      ars3md(15) = 0
      ars3gd(15) = 0
      irs4d(15) = -255
      ars4md(15) = 0
      ars4gd(15) = 0
      ics3d(15) = 0
      ics5d(15) = 0
      iph6d(15) = 0
      icq3d(15) = 2
      iswpd(15) = 0
      swpmd(15) = -999.
      idntd(15) = 0
      jtgmd(15) = 0

C 16  nu-e,nu-e-bar,tau,anti-tau
      ngd(16)    = 21
      irs1d(16) = 1
      ars1md(16) = amz
      ars1gd(16) = agz
      irs2d(16) = 3
      ars2md(16) = amz
      ars2gd(16) = agz
      irs3d(16) = -255
      ars3md(16) = 0
      ars3gd(16) = 0
      irs4d(16) = -255
      ars4md(16) = 0
      ars4gd(16) = 0
      ics3d(16) = 0
      ics5d(16) = 0
      iph6d(16) = 0
      icq3d(16) = 2
      iswpd(16) = 0
      swpmd(16) = -999.
      idntd(16) = 0
      jtgmd(16) = 0

C 17  nu-tau,nu-tau-bar,muon,anti-muon
      ngd(17)    = 11
      irs1d(17) = 1
      ars1md(17) = amz
      ars1gd(17) = agz
      irs2d(17) = 3
      ars2md(17) = amz
      ars2gd(17) = agz
      irs3d(17) = -255
      ars3md(17) = 0
      ars3gd(17) = 0
      irs4d(17) = -255
      ars4md(17) = 0
      ars4gd(17) = 0
      ics3d(17) = 0
      ics5d(17) = 0
      iph6d(17) = 0
      icq3d(17) = 2
      iswpd(17) = 0
      swpmd(17) = -999.
      idntd(17) = 0
      jtgmd(17) = 0

C 18  nu-mu,nu-mu-bar,tau,anti-tau
      ngd(18)    = 11
      irs1d(18) = 1
      ars1md(18) = amz
      ars1gd(18) = agz
      irs2d(18) = 3
      ars2md(18) = amz
      ars2gd(18) = agz
      irs3d(18) = -255
      ars3md(18) = 0
      ars3gd(18) = 0
      irs4d(18) = -255
      ars4md(18) = 0
      ars4gd(18) = 0
      ics3d(18) = 0
      ics5d(18) = 0
      iph6d(18) = 0
      icq3d(18) = 2
      iswpd(18) = 0
      swpmd(18) = -999.
      idntd(18) = 0
      jtgmd(18) = 0

C 19  nu-e,nu-e-bar,nu-e,nu-e-bar
      ngd(19)    = 36
      irs1d(19) = 1
      ars1md(19) = amz
      ars1gd(19) = agz
      irs2d(19) = 1
      ars2md(19) = amz
      ars2gd(19) = agz
      irs3d(19) = 1
      ars3md(19) = amz
      ars3gd(19) = agz
      irs4d(19) = 1
      ars4md(19) = amz
      ars4gd(19) = agz
      ics3d(19) = 0
      ics5d(19) = 4
      iph6d(19) = 0
      icq3d(19) = 0
      iswpd(19) = 1
      swpmd(19) = -999.
      idntd(19) = 2
      jtgmd(19) = 0

C 20  nu-e,nu-e-bar,nu-mu,nu-mu-bar
      ngd(20)    = 12
      irs1d(20) = 1
      ars1md(20) = amz
      ars1gd(20) = agz
      irs2d(20) = 1
      ars2md(20) = amz
      ars2gd(20) = agz
      irs3d(20) = -255
      ars3md(20) = 0
      ars3gd(20) = 0
      irs4d(20) = -255
      ars4md(20) = 0
      ars4gd(20) = 0
      ics3d(20) = 0
      ics5d(20) = 0
      iph6d(20) = 0
      icq3d(20) = 0
      iswpd(20) = 0
      swpmd(20) = -999.
      idntd(20) = 0
      jtgmd(20) = 0

C 21  nu-e,nu-e-bar,nu-tau,nu-tau-bar
      ngd(21)    = 12
      irs1d(21) = 1
      ars1md(21) = amz
      ars1gd(21) = agz
      irs2d(21) = 1
      ars2md(21) = amz
      ars2gd(21) = agz
      irs3d(21) = -255
      ars3md(21) = 0
      ars3gd(21) = 0
      irs4d(21) = -255
      ars4md(21) = 0
      ars4gd(21) = 0
      ics3d(21) = 0
      ics5d(21) = 0
      iph6d(21) = 0
      icq3d(21) = 0
      iswpd(21) = 0
      swpmd(21) = -999.
      idntd(21) = 0
      jtgmd(21) = 0

C 22  nu-mu,nu-mu-bar,nu-mu,nu-mu-bar
      ngd(22)    = 12
      irs1d(22) = 1
      ars1md(22) = amz
      ars1gd(22) = agz
      irs2d(22) = 1
      ars2md(22) = amz
      ars2gd(22) = agz
      irs3d(22) = 1
      ars3md(22) = amz
      ars3gd(22) = agz
      irs4d(22) = 1
      ars4md(22) = amz
      ars4gd(22) = agz
      ics3d(22) = 0
      ics5d(22) = 4
      iph6d(22) = 0
      icq3d(22) = 0
      iswpd(22) = 1
      swpmd(22) = -999.
      idntd(22) = 2
      jtgmd(22) = 0

C 23  nu-tau,nu-tau-bar,nu-tau,nu-tau-bar
      ngd(23)    = 12
      irs1d(23) = 1
      ars1md(23) = amz
      ars1gd(23) = agz
      irs2d(23) = 1
      ars2md(23) = amz
      ars2gd(23) = agz
      irs3d(23) = 1
      ars3md(23) = amz
      ars3gd(23) = agz
      irs4d(23) = 1
      ars4md(23) = amz
      ars4gd(23) = agz
      ics3d(23) = 0
      ics5d(23) = 4
      iph6d(23) = 0
      icq3d(23) = 0
      iswpd(23) = 1
      swpmd(23) = -999.
      idntd(23) = 2
      jtgmd(23) = 0

C 24  nu-tau,nu-tau-bar,nu-mu,nu-mu-bar
      ngd(24)    = 6
      irs1d(24) = 1
      ars1md(24) = amz
      ars1gd(24) = agz
      irs2d(24) = 1
      ars2md(24) = amz
      ars2gd(24) = agz
      irs3d(24) = -255
      ars3md(24) = 0
      ars3gd(24) = 0
      irs4d(24) = -255
      ars4md(24) = 0
      ars4gd(24) = 0
      ics3d(24) = 0
      ics5d(24) = 0
      iph6d(24) = 0
      icq3d(24) = 0
      iswpd(24) = 0
      swpmd(24) = -999.
      idntd(24) = 0
      jtgmd(24) = 0

C 25  electron,nu-e-bar,u,d-bar
      ngd(25)    = 24
      irs1d(25) = 1
      ars1md(25) = amw
      ars1gd(25) = agw
      irs2d(25) = 1
      ars2md(25) = amw
      ars2gd(25) = agw
      irs3d(25) = -255
      ars3md(25) = 0
      ars3gd(25) = 0
      irs4d(25) = -255
      ars4md(25) = 0
      ars4gd(25) = 0
      ics3d(25) = 1
      ics5d(25) = 0
      iph6d(25) = 0
      icq3d(25) = 1
      iswpd(25) = 0
      swpmd(25) = -999.
      idntd(25) = 0
      jtgmd(25) = 1

C 26  electron,nu-e-bar,c,s-bar
      ngd(26)    = 24
      irs1d(26) = 1
      ars1md(26) = amw
      ars1gd(26) = agw
      irs2d(26) = 1
      ars2md(26) = amw
      ars2gd(26) = agw
      irs3d(26) = -255
      ars3md(26) = 0
      ars3gd(26) = 0
      irs4d(26) = -255
      ars4md(26) = 0
      ars4gd(26) = 0
      ics3d(26) = 1
      ics5d(26) = 0
      iph6d(26) = 0
      icq3d(26) = 1
      iswpd(26) = 0
      swpmd(26) = -999.
      idntd(26) = 0
      jtgmd(26) = 1

C 27  muon,nu-mu-bar,u,d-bar
      ngd(27)    = 23
      irs1d(27) = 1
      ars1md(27) = amw
      ars1gd(27) = agw
      irs2d(27) = 1
      ars2md(27) = amw
      ars2gd(27) = agw
      irs3d(27) = -255
      ars3md(27) = 0
      ars3gd(27) = 0
      irs4d(27) = -255
      ars4md(27) = 0
      ars4gd(27) = 0
      ics3d(27) = 0
      ics5d(27) = 0
      iph6d(27) = 0
      icq3d(27) = 1
      iswpd(27) = 0
      swpmd(27) = -999.
      idntd(27) = 0
      jtgmd(27) = 0

C 28  muon,nu-mu-bar,c,s-bar
      ngd(28)    = 23
      irs1d(28) = 1
      ars1md(28) = amw
      ars1gd(28) = agw
      irs2d(28) = 1
      ars2md(28) = amw
      ars2gd(28) = agw
      irs3d(28) = -255
      ars3md(28) = 0
      ars3gd(28) = 0
      irs4d(28) = -255
      ars4md(28) = 0
      ars4gd(28) = 0
      ics3d(28) = 0
      ics5d(28) = 0
      iph6d(28) = 0
      icq3d(28) = 1
      iswpd(28) = 0
      swpmd(28) = -999.
      idntd(28) = 0
      jtgmd(28) = 0

C 29  tau,nu-tau-bar,u,d-bar
      ngd(29)    = 23
      irs1d(29) = 1
      ars1md(29) = amw
      ars1gd(29) = agw
      irs2d(29) = 1
      ars2md(29) = amw
      ars2gd(29) = agw
      irs3d(29) = -255
      ars3md(29) = 0
      ars3gd(29) = 0
      irs4d(29) = -255
      ars4md(29) = 0
      ars4gd(29) = 0
      ics3d(29) = 0
      ics5d(29) = 0
      iph6d(29) = 0
      icq3d(29) = 1
      iswpd(29) = 0
      swpmd(29) = -999.
      idntd(29) = 0
      jtgmd(29) = 0

C 30  tau,nu-tau-bar,c,S-bar
      ngd(30)    = 23
      irs1d(30) = 1
      ars1md(30) = amw
      ars1gd(30) = agw
      irs2d(30) = 1
      ars2md(30) = amw
      ars2gd(30) = agw
      irs3d(30) = -255
      ars3md(30) = 0
      ars3gd(30) = 0
      irs4d(30) = -255
      ars4md(30) = 0
      ars4gd(30) = 0
      ics3d(30) = 0
      ics5d(30) = 0
      iph6d(30) = 0
      icq3d(30) = 1
      iswpd(30) = 0
      swpmd(30) = -999.
      idntd(30) = 0
      jtgmd(30) = 0

C 31  electron,positron,u,u-bar
      ngd(31)    = 50
      irs1d(31) = 2
      ars1md(31) = amz
      ars1gd(31) = agz
      irs2d(31) = 3
      ars2md(31) = amz
      ars2gd(31) = agz
      irs3d(31) = -255
      ars3md(31) = 0
      ars3gd(31) = 0
      irs4d(31) = -255
      ars4md(31) = 0
      ars4gd(31) = 0
      ics3d(31) = 2
      ics5d(31) = 0
      iph6d(31) = 0
      icq3d(31) = 2
      iswpd(31) = 0
      swpmd(31) = -999.
      idntd(31) = 0
      jtgmd(31) = 1

C 32  electron,positron,c,c-bar
      ngd(32)    = 50
      irs1d(32) = 2
      ars1md(32) = amz
      ars1gd(32) = agz
      irs2d(32) = 3
      ars2md(32) = amz
      ars2gd(32) = agz
      irs3d(32) = -255
      ars3md(32) = 0
      ars3gd(32) = 0
      irs4d(32) = -255
      ars4md(32) = 0
      ars4gd(32) = 0
      ics3d(32) = 2
      ics5d(32) = 0
      iph6d(32) = 0
      icq3d(32) = 2
      iswpd(32) = 0
      swpmd(32) = -999.
      idntd(32) = 0
      jtgmd(32) = 1

C 33  electron,positron,d,d-bar
      ngd(33)    = 50
      irs1d(33) = 3
      ars1md(33) = amz
      ars1gd(33) = agz
      irs2d(33) = 3
      ars2md(33) = amz
      ars2gd(33) = agz
      irs3d(33) = -255
      ars3md(33) = 0
      ars3gd(33) = 0
      irs4d(33) = -255
      ars4md(33) = 0
      ars4gd(33) = 0
      ics3d(33) = 2
      ics5d(33) = 0
      iph6d(33) = 0
      icq3d(33) = 2
      iswpd(33) = 0
      swpmd(33) = -999.
      idntd(33) = 0
      jtgmd(33) = 1

C 34  electron,positron,s,s-bar
      ngd(34)    = 50
      irs1d(34) = 3
      ars1md(34) = amz
      ars1gd(34) = agz
      irs2d(34) = 3
      ars2md(34) = amz
      ars2gd(34) = agz
      irs3d(34) = -255
      ars3md(34) = 0
      ars3gd(34) = 0
      irs4d(34) = -255
      ars4md(34) = 0
      ars4gd(34) = 0
      ics3d(34) = 2
      ics5d(34) = 0
      iph6d(34) = 0
      icq3d(34) = 2
      iswpd(34) = 0
      swpmd(34) = -999.
      idntd(34) = 0
      jtgmd(34) = 1

C 35  electron,positron,b,b-bar
      ngd(35)    = 50
      irs1d(35) = 3
      ars1md(35) = amz
      ars1gd(35) = agz
      irs2d(35) = 3
      ars2md(35) = amz
      ars2gd(35) = agz
      irs3d(35) = -255
      ars3md(35) = 0
      ars3gd(35) = 0
      irs4d(35) = -255
      ars4md(35) = 0
      ars4gd(35) = 0
      ics3d(35) = 2
      ics5d(35) = 0
      iph6d(35) = 0
      icq3d(35) = 2
      iswpd(35) = 0
      swpmd(35) = -999.
      idntd(35) = 0
      jtgmd(35) = 1

C 36  muon,anti-muon,u,u-bar
      ngd(36)    = 44
      irs1d(36) = 1
      ars1md(36) = amz
      ars1gd(36) = agz
      irs2d(36) = 3
      ars2md(36) = amz
      ars2gd(36) = agz
      irs3d(36) = -255
      ars3md(36) = 0
      ars3gd(36) = 0
      irs4d(36) = -255
      ars4md(36) = 0
      ars4gd(36) = 0
      ics3d(36) = 0
      ics5d(36) = 0
      iph6d(36) = 0
      icq3d(36) = 2
      iswpd(36) = 0
      swpmd(36) = -999.
      idntd(36) = 0
      jtgmd(36) = 0

C 37  muon,anti-muon,c,c-bar
      ngd(37)    = 44
      irs1d(37) = 1
      ars1md(37) = amz
      ars1gd(37) = agz
      irs2d(37) = 3
      ars2md(37) = amz
      ars2gd(37) = agz
      irs3d(37) = -255
      ars3md(37) = 0
      ars3gd(37) = 0
      irs4d(37) = -255
      ars4md(37) = 0
      ars4gd(37) = 0
      ics3d(37) = 0
      ics5d(37) = 0
      iph6d(37) = 0
      icq3d(37) = 2
      iswpd(37) = 0
      swpmd(37) = -999.
      idntd(37) = 0
      jtgmd(37) = 0

C 38  tau,anti-tau,u,u-bar
      ngd(38)    = 44
      irs1d(38) = 1
      ars1md(38) = amz
      ars1gd(38) = agz
      irs2d(38) = 3
      ars2md(38) = amz
      ars2gd(38) = agz
      irs3d(38) = -255
      ars3md(38) = 0
      ars3gd(38) = 0
      irs4d(38) = -255
      ars4md(38) = 0
      ars4gd(38) = 0
      ics3d(38) = 0
      ics5d(38) = 0
      iph6d(38) = 0
      icq3d(38) = 2
      iswpd(38) = 0
      swpmd(38) = -999.
      idntd(38) = 0
      jtgmd(38) = 0

C 39  tau,anti-tau,c,c-bar
      ngd(39)    = 44
      irs1d(39) = 1
      ars1md(39) = amz
      ars1gd(39) = agz
      irs2d(39) = 3
      ars2md(39) = amz
      ars2gd(39) = agz
      irs3d(39) = -255
      ars3md(39) = 0
      ars3gd(39) = 0
      irs4d(39) = -255
      ars4md(39) = 0
      ars4gd(39) = 0
      ics3d(39) = 0
      ics5d(39) = 0
      iph6d(39) = 0
      icq3d(39) = 2
      iswpd(39) = 0
      swpmd(39) = -999.
      idntd(39) = 0
      jtgmd(39) = 0

C 40  muon,anti-muon,d,d-bar
      ngd(40)    = 44
      irs1d(40) = 1
      ars1md(40) = amz
      ars1gd(40) = agz
      irs2d(40) = 3
      ars2md(40) = amz
      ars2gd(40) = agz
      irs3d(40) = -255
      ars3md(40) = 0
      ars3gd(40) = 0
      irs4d(40) = -255
      ars4md(40) = 0
      ars4gd(40) = 0
      ics3d(40) = 0
      ics5d(40) = 0
      iph6d(40) = 0
      icq3d(40) = 2
      iswpd(40) = 0
      swpmd(40) = -999.
      idntd(40) = 0
      jtgmd(40) = 0

C 41  muon,anti-muon,s,s-bar
      ngd(41)    = 44
      irs1d(41) = 1
      ars1md(41) = amz
      ars1gd(41) = agz
      irs2d(41) = 3
      ars2md(41) = amz
      ars2gd(41) = agz
      irs3d(41) = -255
      ars3md(41) = 0
      ars3gd(41) = 0
      irs4d(41) = -255
      ars4md(41) = 0
      ars4gd(41) = 0
      ics3d(41) = 0
      ics5d(41) = 0
      iph6d(41) = 0
      icq3d(41) = 2
      iswpd(41) = 0
      swpmd(41) = -999.
      idntd(41) = 0
      jtgmd(41) = 0

C 42  muon,anti-muon,b,b-bar
      ngd(42)    = 44
      irs1d(42) = 1
      ars1md(42) = amz
      ars1gd(42) = agz
      irs2d(42) = 3
      ars2md(42) = amz
      ars2gd(42) = agz
      irs3d(42) = -255
      ars3md(42) = 0
      ars3gd(42) = 0
      irs4d(42) = -255
      ars4md(42) = 0
      ars4gd(42) = 0
      ics3d(42) = 0
      ics5d(42) = 0
      iph6d(42) = 0
      icq3d(42) = 2
      iswpd(42) = 0
      swpmd(42) = -999.
      idntd(42) = 0
      jtgmd(42) = 0

C 43  tau,anti-tau,d,d-bar
      ngd(43)    = 44
      irs1d(43) = 1
      ars1md(43) = amz
      ars1gd(43) = agz
      irs2d(43) = 3
      ars2md(43) = amz
      ars2gd(43) = agz
      irs3d(43) = -255
      ars3md(43) = 0
      ars3gd(43) = 0
      irs4d(43) = -255
      ars4md(43) = 0
      ars4gd(43) = 0
      ics3d(43) = 0
      ics5d(43) = 0
      iph6d(43) = 0
      icq3d(43) = 2
      iswpd(43) = 0
      swpmd(43) = -999.
      idntd(43) = 0
      jtgmd(43) = 0

C 44  tau,anti-tau,s,s-bar
      ngd(44)    = 44
      irs1d(44) = 1
      ars1md(44) = amz
      ars1gd(44) = agz
      irs2d(44) = 3
      ars2md(44) = amz
      ars2gd(44) = agz
      irs3d(44) = -255
      ars3md(44) = 0
      ars3gd(44) = 0
      irs4d(44) = -255
      ars4md(44) = 0
      ars4gd(44) = 0
      ics3d(44) = 0
      ics5d(44) = 0
      iph6d(44) = 0
      icq3d(44) = 2
      iswpd(44) = 0
      swpmd(44) = -999.
      idntd(44) = 0
      jtgmd(44) = 0

C 45  tau,anti-tau,b,b-bar
      ngd(45)    = 44
      irs1d(45) = 1
      ars1md(45) = amz
      ars1gd(45) = agz
      irs2d(45) = 3
      ars2md(45) = amz
      ars2gd(45) = agz
      irs3d(45) = -255
      ars3md(45) = 0
      ars3gd(45) = 0
      irs4d(45) = -255
      ars4md(45) = 0
      ars4gd(45) = 0
      ics3d(45) = 0
      ics5d(45) = 0
      iph6d(45) = 0
      icq3d(45) = 2
      iswpd(45) = 0
      swpmd(45) = -999.
      idntd(45) = 0
      jtgmd(45) = 0

C 46  nu-e,nu-e-bar,u,u-bar
      ngd(46)    = 21
      irs1d(46) = 1
      ars1md(46) = amz
      ars1gd(46) = agz
      irs2d(46) = 3
      ars2md(46) = amz
      ars2gd(46) = agz
      irs3d(46) = -255
      ars3md(46) = 0
      ars3gd(46) = 0
      irs4d(46) = -255
      ars4md(46) = 0
      ars4gd(46) = 0
      ics3d(46) = 0
      ics5d(46) = 0
      iph6d(46) = 0
      icq3d(46) = 0
      iswpd(46) = 0
      swpmd(46) = -999.
      idntd(46) = 0
      jtgmd(46) = 0

C 47  nu-e,nu-e-bar,c,c-bar
      ngd(47)    = 21
      irs1d(47) = 1
      ars1md(47) = amz
      ars1gd(47) = agz
      irs2d(47) = 3
      ars2md(47) = amz
      ars2gd(47) = agz
      irs3d(47) = -255
      ars3md(47) = 0
      ars3gd(47) = 0
      irs4d(47) = -255
      ars4md(47) = 0
      ars4gd(47) = 0
      ics3d(47) = 0
      ics5d(47) = 0
      iph6d(47) = 0
      icq3d(47) = 0
      iswpd(47) = 0
      swpmd(47) = -999.
      idntd(47) = 0
      jtgmd(47) = 0

C 48  nu-e,nu-e-bar,d,d-bar
      ngd(48)    = 21
      irs1d(48) = 1
      ars1md(48) = amz
      ars1gd(48) = agz
      irs2d(48) = 2
      ars2md(48) = amz
      ars2gd(48) = agz
      irs3d(48) = -255
      ars3md(48) = 0
      ars3gd(48) = 0
      irs4d(48) = -255
      ars4md(48) = 0
      ars4gd(48) = 0
      ics3d(48) = 0
      ics5d(48) = 0
      iph6d(48) = 0
      icq3d(48) = 0
      iswpd(48) = 0
      swpmd(48) = -999.
      idntd(48) = 0
      jtgmd(48) = 0

C 49  nu-e,nu-e-bar,s,s-bar
      ngd(49)    = 21
      irs1d(49) = 1
      ars1md(49) = amz
      ars1gd(49) = agz
      irs2d(49) = 2
      ars2md(49) = amz
      ars2gd(49) = agz
      irs3d(49) = -255
      ars3md(49) = 0
      ars3gd(49) = 0
      irs4d(49) = -255
      ars4md(49) = 0
      ars4gd(49) = 0
      ics3d(49) = 0
      ics5d(49) = 0
      iph6d(49) = 0
      icq3d(49) = 0
      iswpd(49) = 0
      swpmd(49) = -999.
      idntd(49) = 0
      jtgmd(49) = 0

C 50  nu-e,nu-e-bar,b,b-bar
      ngd(50)    = 21
      irs1d(50) = 1
      ars1md(50) = amz
      ars1gd(50) = agz
      irs2d(50) = 2
      ars2md(50) = amz
      ars2gd(50) = agz
      irs3d(50) = -255
      ars3md(50) = 0
      ars3gd(50) = 0
      irs4d(50) = -255
      ars4md(50) = 0
      ars4gd(50) = 0
      ics3d(50) = 0
      ics5d(50) = 0
      iph6d(50) = 0
      icq3d(50) = 0
      iswpd(50) = 0
      swpmd(50) = -999.
      idntd(50) = 0
      jtgmd(50) = 0

C 51  nu-mu,nu-mu-bar,u,u-bar
      ngd(51)    = 11
      irs1d(51) = 1
      ars1md(51) = amz
      ars1gd(51) = agz
      irs2d(51) = 3
      ars2md(51) = amz
      ars2gd(51) = agz
      irs3d(51) = -255
      ars3md(51) = 0
      ars3gd(51) = 0
      irs4d(51) = -255
      ars4md(51) = 0
      ars4gd(51) = 0
      ics3d(51) = 0
      ics5d(51) = 0
      iph6d(51) = 0
      icq3d(51) = 0
      iswpd(51) = 0
      swpmd(51) = -999.
      idntd(51) = 0
      jtgmd(51) = 0

C 52  nu-mu,nu-mu-bar,c,c-bar
      ngd(52)    = 11
      irs1d(52) = 1
      ars1md(52) = amz
      ars1gd(52) = agz
      irs2d(52) = 3
      ars2md(52) = amz
      ars2gd(52) = agz
      irs3d(52) = -255
      ars3md(52) = 0
      ars3gd(52) = 0
      irs4d(52) = -255
      ars4md(52) = 0
      ars4gd(52) = 0
      ics3d(52) = 0
      ics5d(52) = 0
      iph6d(52) = 0
      icq3d(52) = 0
      iswpd(52) = 0
      swpmd(52) = -999.
      idntd(52) = 0
      jtgmd(52) = 0

C 53  nu-tau,nu-tau-bar,u,u-bar
      ngd(53)    = 11
      irs1d(53) = 1
      ars1md(53) = amz
      ars1gd(53) = agz
      irs2d(53) = 3
      ars2md(53) = amz
      ars2gd(53) = agz
      irs3d(53) = -255
      ars3md(53) = 0
      ars3gd(53) = 0
      irs4d(53) = -255
      ars4md(53) = 0
      ars4gd(53) = 0
      ics3d(53) = 0
      ics5d(53) = 0
      iph6d(53) = 0
      icq3d(53) = 0
      iswpd(53) = 0
      swpmd(53) = -999.
      idntd(53) = 0
      jtgmd(53) = 0

C 54  nu-tau,nu-tau-bar,c,c-bar
      ngd(54)    = 11
      irs1d(54) = 1
      ars1md(54) = amz
      ars1gd(54) = agz
      irs2d(54) = 3
      ars2md(54) = amz
      ars2gd(54) = agz
      irs3d(54) = -255
      ars3md(54) = 0
      ars3gd(54) = 0
      irs4d(54) = -255
      ars4md(54) = 0
      ars4gd(54) = 0
      ics3d(54) = 0
      ics5d(54) = 0
      iph6d(54) = 0
      icq3d(54) = 0
      iswpd(54) = 0
      swpmd(54) = -999.
      idntd(54) = 0
      jtgmd(54) = 0

C 55  nu-mu,nu-mu-bar,d,d-bar
      ngd(55)    = 11
      irs1d(55) = 1
      ars1md(55) = amz
      ars1gd(55) = agz
      irs2d(55) = 3
      ars2md(55) = amz
      ars2gd(55) = agz
      irs3d(55) = -255
      ars3md(55) = 0
      ars3gd(55) = 0
      irs4d(55) = -255
      ars4md(55) = 0
      ars4gd(55) = 0
      ics3d(55) = 0
      ics5d(55) = 0
      iph6d(55) = 0
      icq3d(55) = 0
      iswpd(55) = 0
      swpmd(55) = -999.
      idntd(55) = 0
      jtgmd(55) = 0

C 56  nu-mu,nu-mu-bar,s,s-bar
      ngd(56)    = 11
      irs1d(56) = 1
      ars1md(56) = amz
      ars1gd(56) = agz
      irs2d(56) = 3
      ars2md(56) = amz
      ars2gd(56) = agz
      irs3d(56) = -255
      ars3md(56) = 0
      ars3gd(56) = 0
      irs4d(56) = -255
      ars4md(56) = 0
      ars4gd(56) = 0
      ics3d(56) = 0
      ics5d(56) = 0
      iph6d(56) = 0
      icq3d(56) = 0
      iswpd(56) = 0
      swpmd(56) = -999.
      idntd(56) = 0
      jtgmd(56) = 0

C 57  nu-mu,nu-mu-bar,b,b-bar
      ngd(57)    = 11
      irs1d(57) = 1
      ars1md(57) = amz
      ars1gd(57) = agz
      irs2d(57) = 3
      ars2md(57) = amz
      ars2gd(57) = agz
      irs3d(57) = -255
      ars3md(57) = 0
      ars3gd(57) = 0
      irs4d(57) = -255
      ars4md(57) = 0
      ars4gd(57) = 0
      ics3d(57) = 0
      ics5d(57) = 0
      iph6d(57) = 0
      icq3d(57) = 0
      iswpd(57) = 0
      swpmd(57) = -999.
      idntd(57) = 0
      jtgmd(57) = 0

C 58  nu-tau,nu-tau-bar,d,d-bar
      ngd(58)    = 11
      irs1d(58) = 1
      ars1md(58) = amz
      ars1gd(58) = agz
      irs2d(58) = 3
      ars2md(58) = amz
      ars2gd(58) = agz
      irs3d(58) = -255
      ars3md(58) = 0
      ars3gd(58) = 0
      irs4d(58) = -255
      ars4md(58) = 0
      ars4gd(58) = 0
      ics3d(58) = 0
      ics5d(58) = 0
      iph6d(58) = 0
      icq3d(58) = 0
      iswpd(58) = 0
      swpmd(58) = -999.
      idntd(58) = 0
      jtgmd(58) = 0

C 59  nu-tau,nu-tau-bar,s,s-bar
      ngd(59)    = 11
      irs1d(59) = 1
      ars1md(59) = amz
      ars1gd(59) = agz
      irs2d(59) = 3
      ars2md(59) = amz
      ars2gd(59) = agz
      irs3d(59) = -255
      ars3md(59) = 0
      ars3gd(59) = 0
      irs4d(59) = -255
      ars4md(59) = 0
      ars4gd(59) = 0
      ics3d(59) = 0
      ics5d(59) = 0
      iph6d(59) = 0
      icq3d(59) = 0
      iswpd(59) = 0
      swpmd(59) = -999.
      idntd(59) = 0
      jtgmd(59) = 0

C 60  nu-tau,nu-tau-bar,b,b-bar
      ngd(60)    = 11
      irs1d(60) = 1
      ars1md(60) = amz
      ars1gd(60) = agz
      irs2d(60) = 3
      ars2md(60) = amz
      ars2gd(60) = agz
      irs3d(60) = -255
      ars3md(60) = 0
      ars3gd(60) = 0
      irs4d(60) = -255
      ars4md(60) = 0
      ars4gd(60) = 0
      ics3d(60) = 0
      ics5d(60) = 0
      iph6d(60) = 0
      icq3d(60) = 0
      iswpd(60) = 0
      swpmd(60) = -999.
      idntd(60) = 0
      jtgmd(60) = 0

C 61  u,d-bar,d,u-bar
      ngd(61)    = 77
      irs1d(61) = 1
      ars1md(61) = amw
      ars1gd(61) = agw
      irs2d(61) = 1
      ars2md(61) = amw
      ars2gd(61) = agw
      irs3d(61) = 3
      ars3md(61) = amz
      ars3gd(61) = agz
      irs4d(61) = 3
      ars4md(61) = amz
      ars4gd(61) = agz
      ics3d(61) = 0
      ics5d(61) = 5
      iph6d(61) = -999
      icq3d(61) = 2
      iswpd(61) = 0
      swpmd(61) = -999.
      idntd(61) = 0
      jtgmd(61) = 0

C 62  c,s-bar,s,c-bar
      ngd(62)    = 77
      irs1d(62) = 1
      ars1md(62) = amw
      ars1gd(62) = agw
      irs2d(62) = 1
      ars2md(62) = amw
      ars2gd(62) = agw
      irs3d(62) = 3
      ars3md(62) = amz
      ars3gd(62) = agz
      irs4d(62) = 3
      ars4md(62) = amz
      ars4gd(62) = agz
      ics3d(62) = 0
      ics5d(62) = 5
      iph6d(62) = -999
      icq3d(62) = 2
      iswpd(62) = 0
      swpmd(62) = -999.
      idntd(62) = 0
      jtgmd(62) = 0

C 63  u,d-bar,s,c-bar
      ngd(63)    = 25
      irs1d(63) = 1
      ars1md(63) = amw
      ars1gd(63) = agw
      irs2d(63) = 1
      ars2md(63) = amw
      ars2gd(63) = agw
      irs3d(63) = -255
      ars3md(63) = 0
      ars3gd(63) = 0
      irs4d(63) = -255
      ars4md(63) = 0
      ars4gd(63) = 0
      ics3d(63) = 0
      ics5d(63) = 0
      iph6d(63) = 0
      icq3d(63) = 2
      iswpd(63) = 0
      swpmd(63) = -999.
      idntd(63) = 0
      jtgmd(63) = 0

C 64  u,u-bar,u,u-bar
      ngd(64)    = 104
      irs1d(64) = 3
      ars1md(64) = amz
      ars1gd(64) = agz
      irs2d(64) = 3
      ars2md(64) = amz
      ars2gd(64) = agz
      irs3d(64) = 1
      ars3md(64) = amz
      ars3gd(64) = agz
      irs4d(64) = -1
      ars4md(64) = 0
      ars4gd(64) = 0
      ics3d(64) = 0
      ics5d(64) = 3
      iph6d(64) = -999
      icq3d(64) = 2
      iswpd(64) = 1
      swpmd(64) = 5.
      idntd(64) = 4
      jtgmd(64) = 0

C 65  c,c-bar,c,c-bar
      ngd(65)    = 104
      irs1d(65) = 3
      ars1md(65) = amz
      ars1gd(65) = agz
      irs2d(65) = 3
      ars2md(65) = amz
      ars2gd(65) = agz
      irs3d(65) = 1
      ars3md(65) = amz
      ars3gd(65) = agz
      irs4d(65) = -1
      ars4md(65) = 0
      ars4gd(65) = 0
      ics3d(65) = 0
      ics5d(65) = 3
      iph6d(65) = -999
      icq3d(65) = 2
      iswpd(65) = 1
      swpmd(65) = 5.
      idntd(65) = 4
      jtgmd(65) = 0

C 66  d,d-bar,d,d-bar
      ngd(66)    = 104
      irs1d(66) = 3
      ars1md(66) = amz
      ars1gd(66) = agz
      irs2d(66) = 3
      ars2md(66) = amz
      ars2gd(66) = agz
      irs3d(66) = 1
      ars3md(66) = amz
      ars3gd(66) = agz
      irs4d(66) = -1
      ars4md(66) = 0
      ars4gd(66) = 0
      ics3d(66) = 0
      ics5d(66) = 3
      iph6d(66) = -999
      icq3d(66) = 2
      iswpd(66) = 1
      swpmd(66) = 5.
      idntd(66) = 4
      jtgmd(66) = 0

C 67  s,s-bar,s,s-bar
      ngd(67)    = 104
      irs1d(67) = 3
      ars1md(67) = amz
      ars1gd(67) = agz
      irs2d(67) = 3
      ars2md(67) = amz
      ars2gd(67) = agz
      irs3d(67) = 1
      ars3md(67) = amz
      ars3gd(67) = agz
      irs4d(67) = -1
      ars4md(67) = 0
      ars4gd(67) = 0
      ics3d(67) = 0
      ics5d(67) = 3
      iph6d(67) = -999
      icq3d(67) = 2
      iswpd(67) = 1
      swpmd(67) = 5.
      idntd(67) = 4
      jtgmd(67) = 0

C 68  b,b-bar,b,b-bar
      ngd(68)    = 104
      irs1d(68) = 3
      ars1md(68) = amz
      ars1gd(68) = agz
      irs2d(68) = 3
      ars2md(68) = amz
      ars2gd(68) = agz
      irs3d(68) = 1
      ars3md(68) = amz
      ars3gd(68) = agz
      irs4d(68) = -1
      ars4md(68) = 0
      ars4gd(68) = 0
      ics3d(68) = 0
      ics5d(68) = 3
      iph6d(68) = -999
      icq3d(68) = 2
      iswpd(68) = 1
      swpmd(68) = 5.
      idntd(68) = 4
      jtgmd(68) = 0

C 69  u,u-bar,c,c-bar
      ngd(69)    = 52
      irs1d(69) = 3
      ars1md(69) = amz
      ars1gd(69) = agz
      irs2d(69) = 3
      ars2md(69) = amz
      ars2gd(69) = agz
      irs3d(69) = -255
      ars3md(69) = 0
      ars3gd(69) = 0
      irs4d(69) = -255
      ars4md(69) = 0
      ars4gd(69) = 0
      ics3d(69) = 0
      ics5d(69) = 0
      iph6d(69) = 0
      icq3d(69) = 2
      iswpd(69) = 0
      swpmd(69) = -999.
      idntd(69) = 0
      jtgmd(69) = 0

C 70  u,u-bar,s,s-bar
      ngd(70)    = 52
      irs1d(70) = 3
      ars1md(70) = amz
      ars1gd(70) = agz
      irs2d(70) = 3
      ars2md(70) = amz
      ars2gd(70) = agz
      irs3d(70) = -255
      ars3md(70) = 0
      ars3gd(70) = 0
      irs4d(70) = -255
      ars4md(70) = 0
      ars4gd(70) = 0
      ics3d(70) = 0
      ics5d(70) = 0
      iph6d(70) = 0
      icq3d(70) = 2
      iswpd(70) = 0
      swpmd(70) = -999.
      idntd(70) = 0
      jtgmd(70) = 0

C 71  u,u-bar,b,b-bar
      ngd(71)    = 52
      irs1d(71) = 3
      ars1md(71) = amz
      ars1gd(71) = agz
      irs2d(71) = 3
      ars2md(71) = amz
      ars2gd(71) = agz
      irs3d(71) = -255
      ars3md(71) = 0
      ars3gd(71) = 0
      irs4d(71) = -255
      ars4md(71) = 0
      ars4gd(71) = 0
      ics3d(71) = 0
      ics5d(71) = 0
      iph6d(71) = 0
      icq3d(71) = 2
      iswpd(71) = 0
      swpmd(71) = -999.
      idntd(71) = 0
      jtgmd(71) = 0

C 72  c,c-bar,d,d-bar
      ngd(72)    = 52
      irs1d(72) = 3
      ars1md(72) = amz
      ars1gd(72) = agz
      irs2d(72) = 3
      ars2md(72) = amz
      ars2gd(72) = agz
      irs3d(72) = -255
      ars3md(72) = 0
      ars3gd(72) = 0
      irs4d(72) = -255
      ars4md(72) = 0
      ars4gd(72) = 0
      ics3d(72) = 0
      ics5d(72) = 0
      iph6d(72) = 0
      icq3d(72) = 2
      iswpd(72) = 0
      swpmd(72) = -999.
      idntd(72) = 0
      jtgmd(72) = 0

C 73  c,c-bar,b,b-bar
      ngd(73)    = 52
      irs1d(73) = 3
      ars1md(73) = amz
      ars1gd(73) = agz
      irs2d(73) = 3
      ars2md(73) = amz
      ars2gd(73) = agz
      irs3d(73) = -255
      ars3md(73) = 0
      ars3gd(73) = 0
      irs4d(73) = -255
      ars4md(73) = 0
      ars4gd(73) = 0
      ics3d(73) = 0
      ics5d(73) = 0
      iph6d(73) = 0
      icq3d(73) = 2
      iswpd(73) = 0
      swpmd(73) = -999.
      idntd(73) = 0
      jtgmd(73) = 0

C 74  d,d-bar,s,s-bar
      ngd(74)    = 52
      irs1d(74) = 3
      ars1md(74) = amz
      ars1gd(74) = agz
      irs2d(74) = 3
      ars2md(74) = amz
      ars2gd(74) = agz
      irs3d(74) = -255
      ars3md(74) = 0
      ars3gd(74) = 0
      irs4d(74) = -255
      ars4md(74) = 0
      ars4gd(74) = 0
      ics3d(74) = 0
      ics5d(74) = 0
      iph6d(74) = 0
      icq3d(74) = 2
      iswpd(74) = 0
      swpmd(74) = -999.
      idntd(74) = 0
      jtgmd(74) = 0

C 75  d,d-bar,b,b-bar
      ngd(75)    = 52
      irs1d(75) = 3
      ars1md(75) = amz
      ars1gd(75) = agz
      irs2d(75) = 3
      ars2md(75) = amz
      ars2gd(75) = agz
      irs3d(75) = -255
      ars3md(75) = 0
      ars3gd(75) = 0
      irs4d(75) = -255
      ars4md(75) = 0
      ars4gd(75) = 0
      ics3d(75) = 0
      ics5d(75) = 0
      iph6d(75) = 0
      icq3d(75) = 2
      iswpd(75) = 0
      swpmd(75) = -999.
      idntd(75) = 0
      jtgmd(75) = 0

C 76  s,s-bar,b,b-bar
      ngd(76)    = 52
      irs1d(76) = 3
      ars1md(76) = amz
      ars1gd(76) = agz
      irs2d(76) = 3
      ars2md(76) = amz
      ars2gd(76) = agz
      irs3d(76) = -255
      ars3md(76) = 0
      ars3gd(76) = 0
      irs4d(76) = -255
      ars4md(76) = 0
      ars4gd(76) = 0
      ics3d(76) = 0
      ics5d(76) = 0
      iph6d(76) = 0
      icq3d(76) = 2
      iswpd(76) = 0
      swpmd(76) = -999.
      idntd(76) = 0
      jtgmd(76) = 0

C--------------------
C---- end of creation
C--------------------
      ncgrph      = ngd(nthprc)
      iresns(1)   = irs1d(nthprc)
      aresns(1,1) = ars1md(nthprc)
      aresns(2,1) = ars1gd(nthprc)
      iresns(2)   = irs2d(nthprc)
      aresns(1,2) = ars2md(nthprc)
      aresns(2,2) = ars2gd(nthprc)
      iresns(3)   = irs3d(nthprc)
      aresns(1,3) = ars3md(nthprc)
      aresns(2,3) = ars3gd(nthprc)
      iresns(4)   = irs4d(nthprc)
      aresns(1,4) = ars4md(nthprc)
      aresns(2,4) = ars4gd(nthprc)
      icos3       = ics3d(nthprc)
      icos5       = ics5d(nthprc)
      iphi6       = iph6d(nthprc)
      icosq3      = icq3d(nthprc)
      iswap       = iswpd(nthprc)
      swapm2      = swpmd(nthprc)
      ident       = idntd(nthprc)
      jtgamm      = jtgmd(nthprc)

      return
      end

