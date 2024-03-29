! Valery Yundin 2011
!
! Process: 0 -> p1(e-) p2(e+) p3(mu-) p4(gamma) p5(mu+)
! contrubution of |ISRloop|*|ISRtree|^+
!
! OUTPUT:
! loop, tree
!
! INPUT:
! p1,p2,p3,p4,p5 particle momenta (all outgoing)
! mle2 - m_e^2 electrom mass squared
! mlm2 - m_mu^2 muon mass squared
! mu2  - mu_R^2 regularization scale
! eps 0 - finite part, 1 - pole
!
      subroutine eemmgsqampisr(loop,tree,p1,p2,p3,p4,p5,mle2,mlm2,mu2,
     #eps)
      implicit none
      double complex loop,tree
      double precision p1(0:3),p2(0:3),p3(0:3),p4(0:3),p5(0:3)
      double precision mle2,mlm2,mu2
      integer eps

      double precision x0,x1
      double precision s12,s23,s34,s45,s15
      double precision s13,s24,s35,s14,s25
      double complex I5s,I4s,I123s
      double precision Rs,I0s

      double precision Z(520)

      include 'pjfry.f'

      double precision s1,s2,s3,s4,s5,s6,s7,s8,s9

      double precision t3,t5,t19,t38,t48,t53,t63,t66,t84,t102,t116,t132,
     #t134,t141,t145,t147,t158,t166,t170,t175,t188,t194,t211,t229,t246,t
     #263,t280,t286,t303,t316,t319,t326,t335,t345,t354,t363,t375,t379,t3
     #89,t390,t393,t410,t419,t422,t424,t432,t447,t457,t480,t489,t492,t50
     #0,t523,t526,t528,t536,t540,t542,t545,t554,t567,t579,t581,t582,t586
     #,t598,t602,t616,t618,t625,t627,t653,t664,t699,t702,t703,t705,t721,
     #t723,t747,t765,t782,t808,t811,t813,t819,t835,t856,t861,t874,t877,t
     #887,t901,t951,t964,t966,t968,t985,t999,t1001,t1017,t1050,t1074,t11
     #06,t1134,t1159,t1185,t1213,t1238,t1240,t1263,t1285,t1299,t1320,t13
     #45,t1352,t1354,t1386,t1417,t1433,t1449,t1482,t1499,t1513,t1526,t15
     #40,t1555,t1556,t1558,t1560,t1574,t1588,t1596,t1608,t1613,t1645,t16
     #68,t1701,t1703
      x0=p1(0)+p2(0)
      x1=dsqrt((p1(1)+p2(1))**2+(p1(2)+p2(2))**2+(p1(3)+p2(3))**2)
      s12=(x0+x1)*(x0-x1)

      x0=p1(0)+p3(0)
      x1=dsqrt((p1(1)+p3(1))**2+(p1(2)+p3(2))**2+(p1(3)+p3(3))**2)
      s13=(x0+x1)*(x0-x1)

      x0=p1(0)+p4(0)
      x1=dsqrt((p1(1)+p4(1))**2+(p1(2)+p4(2))**2+(p1(3)+p4(3))**2)
      s14=(x0+x1)*(x0-x1)

      x0=p1(0)+p5(0)
      x1=dsqrt((p1(1)+p5(1))**2+(p1(2)+p5(2))**2+(p1(3)+p5(3))**2)
      s15=(x0+x1)*(x0-x1)

      x0=p2(0)+p3(0)
      x1=dsqrt((p2(1)+p3(1))**2+(p2(2)+p3(2))**2+(p2(3)+p3(3))**2)
      s23=(x0+x1)*(x0-x1)

      x0=p2(0)+p4(0)
      x1=dsqrt((p2(1)+p4(1))**2+(p2(2)+p4(2))**2+(p2(3)+p4(3))**2)
      s24=(x0+x1)*(x0-x1)

      x0=p2(0)+p5(0)
      x1=dsqrt((p2(1)+p5(1))**2+(p2(2)+p5(2))**2+(p2(3)+p5(3))**2)
      s25=(x0+x1)*(x0-x1)

      x0=p3(0)+p4(0)
      x1=dsqrt((p3(1)+p4(1))**2+(p3(2)+p4(2))**2+(p3(3)+p4(3))**2)
      s34=(x0+x1)*(x0-x1)

      x0=p3(0)+p5(0)
      x1=dsqrt((p3(1)+p5(1))**2+(p3(2)+p5(2))**2+(p3(3)+p5(3))**2)
      s35=(x0+x1)*(x0-x1)

      x0=p4(0)+p5(0)
      x1=dsqrt((p4(1)+p5(1))**2+(p4(2)+p5(2))**2+(p4(3)+p5(3))**2)
      s45=(x0+x1)*(x0-x1)

      call Zinit1(s12,s23,s34,s45,s15,s13,s24,s35,s14,s25,mle2,mlm2,Z)
      call psetmusq(mu2)
      call pclearcache

      s9=pd0i(1,0.D0,mle2,mle2,mle2,s24,s12,mle2,mle2,0.D0,0.D0,eps)
      s9=pd0i(1,0.D0,mle2,mle2,mle2,s14,s24,0.D0,0.D0,mle2,mle2,eps)
      I5s=0
      t3=Z(15)*Z(2)+Z(5)*Z(16)
      t5=Z(4)**2
      t19=pd0ij(1,2,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t38=pd0ij(1,2,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t48=Z(6)*Z(5)
      t53=pd0ij(1,3,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t63=Z(11)*Z(2)
      t66=pd0ij(2,2,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t84=pd0ij(2,2,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t102=pd0ij(2,3,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t116=pd0ij(0,0,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t132=pd0ij(0,0,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t134=-t3
      t141=pd0ij(1,1,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t145=Z(17)**2
      t147=Z(5)*t5
      t158=pd0ij(1,1,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t166=pd0ij(3,3,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      s1=(-32*Z(33)*t3*t5+(32*Z(35)*Z(2)+32*Z(34)*Z(5))*Z(4)-32*Z(24)*Z(
     #11)*Z(2)-32*Z(32)*Z(5))*t19+(16*Z(17)*(-Z(5)*Z(27)+Z(28)*Z(2))*t5+
     #(-16*Z(5)*Z(31)-16*Z(26)*Z(17)*Z(2))*Z(4)-16*Z(29)*Z(2)-16*Z(30)*Z
     #(5))*t38+16*Z(17)*((Z(5)*Z(39)+Z(40)*Z(2))*t5+(-Z(5)*Z(37)-Z(38)*Z
     #(2))*Z(4)-2*t48+Z(36)*Z(2))*t53-32*Z(48)*(Z(52)*t3*t5+(-Z(50)*Z(2)
     #-Z(5)*Z(51))*Z(4)+Z(5)*Z(49)+t63)*t66+((16*Z(46)*Z(2)+16*Z(47)*Z(5
     #))*t5+(-16*Z(5)*Z(44)-16*Z(45)*Z(2))*Z(4)-16*Z(42)*Z(2)-16*Z(43)*Z
     #(5))*t84
      t170=s1+((16*Z(57)*Z(5)+16*Z(56)*Z(2))*t5+(-16*Z(55)*Z(2)-16*Z(5)*
     #Z(54))*Z(4)+32*Z(23)*Z(6)*Z(5)-16*Z(53)*Z(2))*t102+(-128*Z(14)*t3*
     #t5+(64*Z(12)*Z(5)+64*Z(13)*Z(2))*Z(4)-64*Z(10)*Z(5)-64*t63)*t116+(
     #(32*Z(8)*Z(5)+32*Z(9)*Z(2))*t5+(-32*Z(5)*Z(3)-32*Z(7)*Z(2))*Z(4)+3
     #2*Z(2)*Z(1)-64*t48)*t132+32*Z(23)*(t134*t5+(t63+Z(5)*Z(25))*Z(4)-Z
     #(5))*Z(24)*t141+(-16*Z(18)*t145*t147-16*Z(17)*(Z(5)*Z(19)+Z(20)*Z(
     #2))*Z(4)+16*Z(22)*Z(2)+16*Z(21)*Z(5))*t158-32*Z(23)*(t3*t5+(-t48-Z
     #(61)*Z(2))*Z(4)+Z(2))*Z(24)*t166
      t175=pd0ij(1,3,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t188=pd0ij(2,3,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t194=pd0ij(3,3,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t211=pd0i(1,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t229=pd0i(1,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t246=pd0i(2,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t263=pd0i(2,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t280=pd0i(3,s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      t286=pd0i(3,0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t303=pd0(0.D0,mle2,mle2,s35,s14,s12,mle2,mle2,0.D0,mle2,eps)
      t316=pd0(s35,mle2,mle2,0.D0,s24,s12,mle2,mle2,0.D0,mle2,eps)
      s1=-32*Z(17)*(t134*Z(4)+Z(5)*Z(41)+t63)*t175+((32*Z(58)*Z(16)*Z(5)
     #+32*Z(15))*Z(4)+32*Z(59)*Z(5)+32*Z(60)*Z(2))*t188+(32*Z(62)*Z(2)+3
     #2*Z(63)*Z(5))*t194+((-16*Z(75)*Z(2)+16*Z(74)*Z(5))*t5+(16*Z(73)*Z(
     #2)-16*Z(5)*Z(72))*Z(4)-16*Z(70)*Z(2)-16*Z(71)*Z(5))*t211+(-16*Z(17
     #)*(-Z(5)*Z(64)+Z(65)*Z(2))*t5+(16*Z(68)*Z(2)-16*Z(5)*Z(69))*Z(4)+1
     #6*Z(66)*Z(5)-16*Z(67)*Z(2))*t229
      t319=s1+((16*Z(86)*Z(5)-16*Z(87)*Z(2))*t5+(-16*Z(5)*Z(84)+16*Z(85)
     #*Z(2))*Z(4)-16*Z(82)*Z(5)-16*Z(83)*Z(2))*t246+((-16*Z(80)*Z(2)+16*
     #Z(81)*Z(5))*t5+(16*Z(78)*Z(2)-16*Z(5)*Z(79))*Z(4)-16*Z(76)*Z(2)+16
     #*Z(77)*Z(5))*t263+((16*Z(93)*Z(2)-16*Z(92)*Z(5))*t5+(-16*Z(91)*Z(2
     #)+16*Z(5)*Z(90))*Z(4)+16*Z(89)*Z(2)+16*Z(88)*Z(5))*t280+(16*Z(95)*
     #Z(5)+16*Z(94)*Z(2))*t286+((-16*Z(106)*Z(2)+16*Z(107)*Z(5))*t5+(-16
     #*Z(105)*Z(5)+16*Z(104)*Z(2))*Z(4)+16*Z(103)*Z(2)-16*Z(102)*Z(5))*t
     #303-16*((Z(100)*Z(5)+Z(101)*Z(2))*t5+(-Z(98)*Z(2)+Z(99)*Z(5))*Z(4)
     #-Z(97)*Z(5)+Z(96)*Z(2))*t316
      I4s=t170+t319
      t326=pc0ij(2,2,0.D0,mle2,s14,mle2,mle2,0.D0,eps)
      t335=pc0ij(1,2,0.D0,mle2,s24,mle2,mle2,0.D0,eps)
      t345=pc0ij(0,0,0.D0,mle2,s24,mle2,mle2,0.D0,eps)
      t354=pc0ij(0,0,0.D0,mle2,s14,mle2,mle2,0.D0,eps)
      t363=pc0ij(1,1,0.D0,mle2,s24,mle2,mle2,0.D0,eps)
      t375=2*Z(119)*Z(5)
      t379=pb0(s24,0.D0,mle2,eps)
      t389=((2*Z(152)+Z(155)*Z(2))*Z(4)-Z(153)*Z(2)-Z(5)*Z(154))*Z(4)
      t390=pc0ij(1,1,0.D0,s12,s35,mle2,mle2,mle2,eps)
      t393=pc0ij(2,2,s35,s12,0.D0,mle2,mle2,mle2,eps)
      t410=pb0i(1,s24,0.D0,mle2,eps)
      t419=pc0ij(1,2,0.D0,mle2,s14,mle2,mle2,0.D0,eps)
      t422=Z(52)*Z(48)
      t424=Z(167)*Z(166)
      t432=pb0i(1,mle2,0.D0,mle2,eps)
      t447=pc0ij(0,0,s35,mle2,s24,mle2,mle2,0.D0,eps)
      t457=pc0ij(2,2,0.D0,mle2,s24,mle2,mle2,0.D0,eps)
      t480=pc0ij(2,2,s35,mle2,s24,mle2,mle2,0.D0,eps)
      t489=pc0ij(1,1,0.D0,mle2,s14,mle2,mle2,0.D0,eps)
      t492=Z(5)**2
      t500=Z(2)**2
      t523=32*Z(2)
      t526=pb0(mle2,0.D0,mle2,eps)
      s1=-16*((Z(210)-Z(211))*t5+(Z(208)-Z(209))*Z(4)+Z(24)+Z(207))*Z(5)
     #*t326+16*((-Z(185)+Z(186))*t5+(Z(183)-Z(184))*Z(4)+Z(23)+Z(182))*Z
     #(2)*t335+32*(-1+(Z(132)+2*Z(133))*t5+(-Z(130)+Z(131))*Z(4))*Z(2)*t
     #345-32*(1+(Z(136)-Z(137))*t5+(Z(134)-Z(135))*Z(4))*Z(5)*t354+16*((
     #Z(157)-Z(158))*Z(4)+Z(24)+Z(156)*Z(23)*Z(2))*Z(4)*t363+16*((2*Z(43
     #9)+Z(441)*Z(2))*t5+(Z(5)*Z(440)+Z(438)*Z(2))*Z(4)-t375-Z(437)*Z(2)
     #)*Z(5)*t379+16*t389*t390+16*t389*t393
      s2=s1-32*((Z(5)*Z(391)/2+Z(392)*Z(2))*t5+(-Z(388)*Z(2)/2+Z(5)*Z(39
     #0)/2)*Z(4)-t375-Z(389)*Z(2)/2)*Z(5)*t410-16*((Z(190)-Z(191))*t5+(-
     #Z(188)+Z(189))*Z(4)+Z(24)+Z(187))*Z(5)*t419+32*((t422*Z(2)-t424*Z(
     #5))*Z(4)-Z(52)*Z(2)-Z(5)*Z(167))*Z(4)*t432
      t528=s2+32*((Z(118)*Z(2)-Z(5)*Z(117)+2*Z(114))*t5+(-Z(5)*Z(115)-Z(
     #116)*Z(2)+Z(113))*Z(4)-t375-1+Z(112)*Z(2))*Z(5)*t447-16*((-Z(205)+
     #2*Z(206))*t5+(-Z(203)+Z(204))*Z(4)-Z(23)+Z(202))*Z(2)*t457-16*((2*
     #Z(5)*Z(192)*t424-Z(199)*Z(148)*Z(2)-2*Z(6)*Z(166)*Z(167))*t5+(-2*Z
     #(194)-Z(5)*Z(198)+Z(197)*Z(2))*Z(4)-Z(5)*Z(196)-2*Z(193)+Z(195)*Z(
     #2))*Z(5)*t480-16*Z(4)*((Z(33)-Z(160))*Z(4)+Z(23)-Z(5)*Z(24)*Z(159)
     #)*t489+((32*Z(486)*t492+(16*Z(481)-32*Z(2)*Z(487))*Z(5)+32*Z(488)*
     #t500-16*Z(484)*Z(2))*t5+(32*t492*Z(483)+(16*Z(480)+32*Z(482)*Z(2))
     #*Z(5)-32*Z(485)*t500+16*Z(478)*Z(2))*Z(4)-64*Z(479)*Z(5)*Z(2)-t523
     #+64*Z(5))*t526
      t536=pc0ij(1,1,s35,s12,0.D0,mle2,mle2,mle2,eps)
      t540=Z(404)*t500
      t542=16*Z(148)
      t545=Z(111)*Z(2)
      t554=Z(401)*t500
      t567=Z(400)*t500
      t579=pb0i(1,s14,mle2,0.D0,eps)
      t581=t492*Z(5)
      t582=Z(384)*t581
      t586=Z(385)*Z(2)
      t598=Z(386)*t581
      t602=Z(383)*Z(2)
      t616=Z(387)*Z(119)*t581
      t618=Z(381)*Z(2)
      t625=16*Z(2)
      t627=pb0i(1,s24,mle2,0.D0,eps)
      t653=pc0i(2,0.D0,mle2,s24,mle2,mle2,0.D0,eps)
      t664=Z(133)*Z(2)
      t699=t500*Z(2)
      t702=Z(119)*t500
      t703=32*t702
      t705=pb0(s14,mle2,0.D0,eps)
      t721=16*Z(248)*Z(5)
      t723=pc0i(1,0.D0,s12,s35,mle2,mle2,mle2,eps)
      t747=pc0i(1,0.D0,mle2,s24,mle2,mle2,0.D0,eps)
      t765=pc0i(1,mle2,mle2,s12,mle2,0.D0,mle2,eps)
      t782=pc0i(2,mle2,mle2,s12,mle2,0.D0,mle2,eps)
      t808=Z(17)*Z(5)
      t811=32*Z(5)
      t813=pb0i(1,0.D0,mle2,mle2,eps)
      t819=16*Z(140)
      t835=pb0i(1,s35,mle2,mle2,eps)
      t856=64*t702
      t861=pb0i(1,s14,0.D0,mle2,eps)
      t874=pc0ij(2,2,0.D0,s12,s35,mle2,mle2,mle2,eps)
      t877=Z(5)*Z(2)
      t887=Z(48)**2
      t901=pc0ij(1,1,s14,mle2,s35,mle2,0.D0,mle2,eps)
      t951=pb0(mle2,mle2,0.D0,eps)
      t964=64*Z(2)
      t966=pc0ij(0,0,0.D0,s12,s35,mle2,mle2,mle2,eps)
      s3=(16*Z(17)*Z(4)-16*Z(138)*Z(5)+32*Z(139)*Z(2))*t536+(((-32*Z(402
     #)*Z(2)-16*t540+t542)*Z(5)+32*Z(2)*(-t422+2*t545*Z(11)+t500*Z(405))
     #)*t5+((16*Z(395)+16*t554+16*Z(399)*Z(2))*Z(5)-32*Z(2)*(t500*Z(403)
     #-Z(396)+Z(2)*Z(397)*Z(11)))*Z(4)+(-32*t567+16-16*Z(398)*Z(2))*Z(5)
     #-32*t500*(-Z(11)+Z(2)*Z(406)*Z(119)))*t579
      s2=s3+((32*t582+(64*Z(6)*Z(379)-16*t586)*t492+(-32*Z(382)*Z(2)-32*
     #t424)*Z(5)+16*Z(148)*Z(2))*t5+(32*t598+(32*Z(6)*Z(380)+16*t602)*t4
     #92+(-32*Z(376)+16*Z(378)*Z(2))*Z(5)+16*Z(377)*Z(2))*Z(4)-32*t616+(
     #-32*t618+32*Z(6))*t492+16*Z(375)*Z(5)*Z(2)+t625)*t627+(((16*Z(309)
     #+16*Z(312)*Z(2))*Z(5)+16*Z(311)*Z(2)+64*Z(304))*t5+((-16*Z(310)*Z(
     #2)+16*Z(307))*Z(5)-16*Z(308)*Z(2)+32*Z(305))*Z(4)+32+16*Z(306)*Z(5
     #)*Z(2)+32*Z(303)*Z(2))*t653
      s1=s2+(((-64*Z(459)*Z(119)*t500-32*Z(452)*Z(2)+32*Z(114))*Z(5)-64*
     #(-Z(18)/4-t664*Z(119)+t500*Z(458)*Z(119))*Z(2))*t5+((64*Z(455)*Z(1
     #19)*t500+16*Z(450)*Z(2)+32*Z(449))*Z(5)+64*(Z(425)/4+t500*Z(457)*Z
     #(119)-Z(2)*Z(454)*Z(119))*Z(2))*Z(4)+(16*Z(451)*Z(2)-64*Z(453)*Z(1
     #19)*t500-32)*Z(5)-64*Z(456)*Z(119)*t699+t703-t625)*t705+((-16*Z(24
     #7)*Z(158)*Z(5)+16*Z(250)*Z(2))*t5+(-16*Z(246)*Z(2)-16*Z(5)*Z(249))
     #*Z(4)-16*Z(187)*Z(2)+t721)*t723+(((32*Z(258)*Z(2)+32*Z(255))*Z(5)+
     #32*Z(157)+32*Z(256)*Z(2))*t5+((16*Z(253)-16*Z(257))*Z(5)+32*Z(24)-
     #16*Z(254)*Z(2))*Z(4)+16*Z(251)*Z(5)-16*Z(252)*Z(2))*t747+((16*Z(24
     #3)*Z(158)*Z(5)+16*Z(245)*Z(2))*t5+(-16*Z(5)*Z(242)-16*Z(244)*Z(2))
     #*Z(4)-16*Z(240)*Z(5)-16*Z(241)*Z(2))*t765
      s2=s1+((16*Z(297)*Z(5)+16*Z(298)*Z(2))*t5+(-16*Z(5)*Z(294)-16*Z(29
     #6)*Z(2))*Z(4)+16*Z(293)*Z(2)-16*Z(295)*Z(5))*t782+(((16*Z(420)*Z(2
     #)+32*Z(18)+16*Z(158))*Z(5)+32*Z(2)*(Z(18)-Z(33)/2))*t5+((-16*Z(419
     #)*Z(2)+32*Z(416)+16*Z(417))*Z(5)+32*(Z(416)+Z(418)/2)*Z(2))*Z(4)+3
     #2*t808*Z(2)-t523-t811)*t813+(((-32*Z(119)*Z(17)*Z(2)+32*Z(17)+t819
     #)*Z(5)-16*Z(2)*(-Z(140)+Z(393)))*Z(4)-32*Z(119)*t492+(-16*Z(394)*Z
     #(2)-32)*Z(5)+48*Z(2)-t703)*t835
      s3=s2+(((-32*Z(412)*Z(2)-t542)*Z(5)-16*Z(411)*t500)*t5+((-16*Z(408
     #)-16*Z(409)*Z(2))*Z(5)+16*Z(2)*(-2*Z(11)+Z(2)*Z(410)))*Z(4)+t856+1
     #6*Z(407)*Z(5)*Z(2))*t861+((16*Z(17)-32*Z(58)*Z(17)*Z(5))*Z(4)+64*Z
     #(58)*Z(5)-16*Z(201)*Z(2))*t874
      t968=s3+((16*Z(148)*Z(147)*t877-32*t422*(Z(151)*t500+1))*t5+16*Z(2
     #)*(Z(5)*Z(149)+2*t887+Z(2)*Z(150))*Z(4)-16*Z(2)*(2*Z(48)+Z(2)*Z(14
     #6)+Z(5)*Z(145)))*t901+((-48*t582+(24*t586+64*Z(475))*t492+(24*t540
     #-32*Z(2)*Z(474))*Z(5)+64*Z(477)*t500-48*Z(405)*t699)*t5+(-48*t598+
     #(-24*t602+32*Z(476))*t492+(16*Z(470)*Z(2)-24*t554)*Z(5)+48*Z(403)*
     #t699-32*Z(473)*t500)*Z(4)+48*t616+(32*Z(472)+48*t618)*t492+(48*t56
     #7-16*Z(469)*Z(2))*Z(5)+32*Z(471)*t500+48*Z(406)*Z(119)*t699)*t951+
     #((64*Z(128)*Z(5)-32*Z(129)*Z(2))*t5+(32*Z(126)*Z(2)+32*Z(5)*Z(127)
     #)*Z(4)+t964+t811)*t966
      t985=pc0(mle2,mle2,s12,mle2,0.D0,mle2,eps)
      t999=16*Z(353)*Z(5)
      t1001=pc0(0.D0,s12,s35,mle2,mle2,mle2,eps)
      t1017=pc0(s35,s12,0.D0,mle2,mle2,mle2,eps)
      t1050=pc0i(1,s14,mle2,s35,mle2,0.D0,mle2,eps)
      t1074=pa0(mle2,eps)
      t1106=pc0i(2,s35,mle2,s24,mle2,mle2,0.D0,eps)
      t1134=pc0i(1,s35,mle2,s24,mle2,mle2,0.D0,eps)
      t1159=pc0i(2,0.D0,mle2,s14,mle2,mle2,0.D0,eps)
      t1185=pc0ij(1,2,s14,mle2,s35,mle2,0.D0,mle2,eps)
      t1213=pb0(s35,mle2,mle2,eps)
      t1238=16*Z(5)
      t1240=pb0(s14,0.D0,mle2,eps)
      t1263=pc0i(2,s14,mle2,s35,mle2,0.D0,mle2,eps)
      t1285=pc0i(1,0.D0,mle2,s14,mle2,mle2,0.D0,eps)
      t1299=pc0ij(0,0,s35,s12,0.D0,mle2,mle2,mle2,eps)
      t1320=pc0ij(0,0,s14,mle2,s35,mle2,0.D0,mle2,eps)
      t1345=32*Z(119)
      t1352=pc0(0.D0,mle2,s24,mle2,mle2,0.D0,eps)
      s2=((-16*Z(351)*Z(2)+16*Z(352)*Z(5))*t5+(16*Z(349)*Z(2)-16*Z(5)*Z(
     #350))*Z(4)+16*Z(347)*Z(5)-16*Z(348)*Z(2))*t985+((-16*Z(325)+16*Z(3
     #56)*Z(2))*t5+(-16*Z(354)*Z(2)-16*Z(5)*Z(355))*Z(4)-16*Z(23)*Z(2)-t
     #999)*t1001+((-16*Z(325)+16*Z(328)*Z(5))*t5+(-16*Z(5)*Z(326)-16*Z(3
     #27)*Z(2))*Z(4)+16*Z(323)*Z(2)+16*Z(324)*Z(5))*t1017+(((32*Z(230)*Z
     #(148)-16*Z(238)*Z(2))*Z(5)-16*Z(236)*Z(2)+32*Z(239)*t500)*t5+((16*
     #Z(235)*Z(2)-16*Z(233))*Z(5)-16*Z(2)*(-Z(232)+Z(2)*Z(237)))*Z(4)+(-
     #16*Z(229)-16*Z(231)*Z(2))*Z(5)+16*Z(2)*(-Z(228)+Z(2)*Z(234)))*t105
     #0
      s3=s2-64*((Z(498)*Z(5)*Z(2)/2+t492*Z(379)+Z(111)*t500)*t5+(t492*Z(
     #380)/2+(Z(2)*Z(497)/4-1.D0/2.D0)*Z(5)-Z(397)*t500/2+Z(2)/2)*Z(4)-t
     #877/2+t492/2+t500/2)*t1074
      s1=s3+((32*Z(283)*t492+(-16*Z(282)*Z(2)+16*Z(279))*Z(5)+16*Z(277)*
     #Z(2))*t5+(16*Z(281)*t492+(16*Z(274)+16*Z(278)*Z(2))*Z(5)-16*Z(275)
     #*Z(2))*Z(4)-16*Z(280)*t492+(16*Z(272)+16*Z(276)*Z(2))*Z(5)-16*Z(27
     #3)*Z(2))*t1106+(((16*Z(222)+16*Z(227))*Z(5)-16*Z(226)*Z(2))*t5+(16
     #*Z(221)*t492+(16*Z(223)*Z(2)+16*Z(225))*Z(5)+16*Z(224)*Z(2))*Z(4)+
     #16*Z(220)*t492+(16*Z(217)+16*Z(218)*Z(2))*Z(5)-16*Z(219)*Z(2))*t11
     #34+(((16*Z(319)+16*Z(321)*Z(2))*Z(5)-16*Z(322)*Z(2)+64*Z(316))*t5+
     #((16*Z(315)-16*Z(320)*Z(2))*Z(5)+16*Z(317)*Z(2)-32*Z(313))*Z(4)+16
     #+(-16*Z(318)*Z(2)-16*Z(314))*Z(5))*t1159
      s2=s1+((-32*Z(140)*Z(52)*Z(48)*Z(2)+16*Z(174)*Z(148)*Z(5))*t5+(32*
     #Z(58)+16*Z(177)*Z(5)*Z(2)+16*Z(178)*t500)*Z(4)-16*Z(2)*(Z(2)*Z(176
     #)+2*Z(179)+Z(5)*Z(175)))*t1185+((-16*t492*Z(222)-16*Z(292)*t500+32
     #*Z(14)+32*t877*Z(448))*t5+(-16*t492*Z(446)+(-16*Z(445)*Z(2)-32*Z(4
     #43))*Z(5)+16*Z(2)*(-Z(444)+Z(2)*Z(447)))*Z(4)+16*Z(442)*Z(5)*Z(2)+
     #t811+t523)*t1213+(((-16*Z(463)+16*Z(468)*Z(2))*Z(5)+32*(Z(465)-Z(4
     #66)/2)*Z(2))*t5+((-16*Z(461)-16*Z(462)*Z(2))*Z(5)-16*Z(2)*(Z(460)+
     #Z(2)*Z(467)))*Z(4)+t625+16*Z(464)*Z(5)*Z(2)-t703-t1238)*t1240
      s3=s2+((-16*Z(18)-16*Z(290)*Z(5)+16*Z(292)*Z(2))*t5+16*Z(2)*(-Z(5)
     #*Z(289)+Z(2)*Z(291)-Z(286))*Z(4)+(-32*Z(285)+16*Z(288)*Z(2))*Z(5)-
     #16*Z(2)*(Z(284)+Z(2)*Z(287)))*t1263+(((32*Z(265)*Z(2)+32*Z(262))*Z
     #(5)+32*Z(160)-32*Z(263)*Z(2))*t5+(16*Z(261)*Z(5)+(-16*Z(260)+16*Z(
     #264))*Z(2)-32*Z(23))*Z(4)-t721+16*Z(259)*Z(2))*t1285
      t1354=s3+((32*Z(110)*Z(5)-128*t545)*t5+(64*Z(108)*Z(2)+32*Z(5)*Z(1
     #09))*Z(4)+t964+t811)*t1299+((-128*Z(14)+32*Z(124)*Z(5)*Z(2)-32*Z(1
     #25)*t500)*t5+32*Z(2)*(Z(5)*Z(122)+Z(2)*Z(123)+2*Z(121))*Z(4)-t856-
     #32*Z(120)*Z(5)*Z(2)-t964)*t1320+((32*Z(363)*t492+(16*Z(364)*Z(2)+1
     #6*Z(360))*Z(5)-16*Z(362)*Z(2))*t5+(32*Z(365)*t492+(-16*Z(361)*Z(2)
     #-16*Z(359))*Z(5)+16*Z(358)*Z(2))*Z(4)-64*Z(119)*Z(6)*t492+(-t1345+
     #16*Z(6))*Z(5)-16*Z(357)*Z(2))*t1352
      t1386=pc0(s35,mle2,s24,mle2,mle2,0.D0,eps)
      t1417=pc0(0.D0,mle2,s14,mle2,mle2,0.D0,eps)
      t1433=pc0i(1,s35,s12,0.D0,mle2,mle2,mle2,eps)
      t1449=pc0i(2,0.D0,s12,s35,mle2,mle2,mle2,eps)
      t1482=pc0(s14,mle2,s35,mle2,0.D0,mle2,eps)
      t1499=pc0i(2,s35,s12,0.D0,mle2,mle2,mle2,eps)
      t1513=pb0(s12,mle2,mle2,eps)
      t1526=pb0i(1,s12,mle2,mle2,eps)
      t1540=pc0ij(1,1,s35,mle2,s24,mle2,mle2,0.D0,eps)
      t1555=Z(17)*Z(2)
      t1556=32*t1555
      t1558=16*Z(161)*Z(5)
      t1560=pc0ij(1,2,0.D0,s12,s35,mle2,mle2,mle2,eps)
      t1574=pc0ij(1,2,s35,s12,0.D0,mle2,mle2,mle2,eps)
      t1588=pc0ij(2,2,s14,mle2,s35,mle2,0.D0,mle2,eps)
      t1596=pb0i(1,mle2,mle2,0.D0,eps)
      t1608=16*Z(18)
      t1613=32*t664
      t1645=pb0(s24,mle2,0.D0,eps)
      t1668=pc0ij(1,2,s35,mle2,s24,mle2,mle2,0.D0,eps)
      t1701=pb0(0.D0,mle2,mle2,eps)
      s2=((16*Z(222)*Z(6)*t492+(-16*Z(226)*Z(6)*Z(2)-16*Z(334))*Z(5)+16*
     #Z(335)*Z(2))*t5+(-16*Z(333)*t492+(16*Z(337)*Z(2)+16*Z(336))*Z(5)-1
     #6*Z(332)*Z(2))*Z(4)-16*Z(331)*t492+(-16*Z(329)-16*Z(330)*Z(2))*Z(5
     #)+32*Z(338)*Z(2))*t1386+(((16*Z(371)*Z(2)+16*Z(368))*Z(5)-16*Z(370
     #)*Z(2)+32*Z(372)*t500)*t5+((-16*Z(367)*Z(2)+16*Z(366))*Z(5)+16*Z(3
     #73)*Z(119)*Z(2)-32*Z(369)*t500)*Z(4)+t999-64*(-Z(11)/4+Z(119)/2+Z(
     #2)*Z(119)*Z(11))*Z(2))*t1417+(-16*Z(215)*Z(17)*t147+(-16*t808*Z(21
     #4)+32*Z(216)*Z(2))*Z(4)-16*Z(213)*Z(5)-32*Z(212)*Z(2))*t1433+(16*Z
     #(128)*Z(17)*t5*Z(2)+(-16*Z(301)*Z(2)+16*Z(5)*Z(302))*Z(4)-16*Z(299
     #)*Z(5)-32*Z(300)*Z(2))*t1449
      s1=s2+((-16*Z(290)*Z(11)*t877-16*Z(215)-32*Z(119)*Z(18)*Z(2)+16*Z(
     #292)*Z(11)*t500)*t5+((-16*Z(343)+16*Z(345)*Z(2))*Z(5)-16*Z(2)*(-Z(
     #342)+Z(2)*Z(346)))*Z(4)+(-16*Z(339)+16*Z(341)*Z(2))*Z(5)+16*Z(2)*(
     #Z(2)*Z(344)+Z(340)))*t1482+((16*Z(270)*Z(5)-16*Z(271)*Z(2))*t5+(-1
     #6*Z(5)*Z(267)+16*Z(269)*Z(2))*Z(4)-16*Z(266)*Z(2)-16*Z(268)*Z(5))*
     #t1499+((-32*Z(423)*Z(5)+32*Z(424)*Z(2))*t5+(-16*Z(421)*Z(2)-16*Z(5
     #)*Z(422))*Z(4)-48*Z(5)+t625)*t1513+((16*Z(158)*Z(5)-16*Z(33)*Z(2))
     #*t5+(16*Z(5)*Z(162)+16*Z(374)*Z(2))*Z(4)-t625+t1238)*t1526
      s2=s1+((t819-16*Z(142)-32*Z(142)*Z(119)*Z(2))*Z(4)-16*Z(5)*(Z(144)
     #*Z(5)-Z(141)+Z(143)*Z(2)))*t1540+((32*Z(158)*Z(58)*Z(5)+16*Z(164)*
     #Z(33)*Z(2))*t5+(-16*Z(5)*Z(180)-16*Z(181)*Z(2))*Z(4)+t1556-t1558)*
     #t1560+((-32*Z(33)-16*Z(164)*Z(158)*Z(5))*t5+(32*Z(162)-16*Z(163)*Z
     #(5))*Z(4)+t1556-t1558)*t1574
      s3=s2+((t819-32*Z(119)*Z(58)*Z(5))*Z(4)+16*Z(2)*(-2*Z(139)+Z(2)*Z(
     #200)-Z(143)*Z(5)))*t1588-32*(Z(413)/2-Z(414)*Z(4)/2+Z(415)*t5)*Z(2
     #)*Z(5)*t1596
      t1703=s3+((64*Z(435)*Z(119)*t581+64*Z(119)*(Z(436)*Z(2)+Z(114))*t4
     #92+(t1608-32*Z(428)*Z(2))*Z(5)+t1613)*t5+(64*Z(434)*Z(119)*t581+64
     #*Z(119)*(Z(431)*Z(2)+Z(432))*t492+(16*Z(429)*Z(2)+16*Z(425))*Z(5)-
     #32*Z(427)*Z(2))*Z(4)-64*Z(433)*Z(119)*t581+(-64*Z(430)*Z(119)*Z(2)
     #+t1345)*t492+(-16*Z(426)*Z(2)-16)*Z(5)-t523)*t1645+((-32*t424*Z(14
     #0)*Z(5)+16*Z(170)*Z(148)*Z(2)+32*t424)*t5-16*Z(5)*(-Z(173)*Z(5)+Z(
     #172)*Z(2)-Z(169))*Z(4)-16*Z(5)*(-Z(171)*Z(5)-Z(165)+Z(168)*Z(2)))*
     #t1668+((-64*Z(494)*t492+(-t1608+32*Z(2)*Z(493))*Z(5)-t1613-64*Z(49
     #6)*t500)*t5+(-32*Z(495)*t492+(-32*Z(2)*Z(491)-16*Z(425))*Z(5)+32*Z
     #(2)*(Z(2)*Z(492)+Z(427)))*Z(4)-32*Z(490)*t492+(16+16*t1555)*Z(5)+t
     #523-32*Z(489)*t500)*t1701
      I123s=t528+t968+t1354+t1703
      Rs=(-16*Z(509)*Z(4)+16*Z(515)*t5-16*Z(516)*Z(119))*t699+((-8*Z(513
     #)*t5+8*Z(508)*Z(4)-16*Z(501))*Z(5)+16*Z(504)*Z(4)+96*Z(119)-16*Z(5
     #11)*t5)*t500+16*((-Z(512)*t5/2-Z(502)+Z(507)*Z(4)/2)*Z(5)-Z(500)*Z
     #(4)+Z(506)*t5+2*Z(499))*Z(5)*Z(2)-16*((-Z(514)*t5-Z(510)*Z(4)+Z(51
     #7)*Z(119))*Z(5)+Z(503)*Z(4)+Z(505)*t5-6*Z(119))*t492
      I0s=(-t1345+16*Z(488)*t5-16*Z(520)*Z(4))*t500-16*Z(5)*(Z(487)*t5-Z
     #(482)*Z(4)+2*Z(518))*Z(2)+16*t492*(-2*Z(119)+Z(486)*t5+Z(519)*Z(4)
     #)

      if (eps.eq.0) then
      loop=I5s+I4s+I123s+Rs
      tree=I0s
      else
      loop=I5s+I4s+I123s
      tree=0
      endif

      end
