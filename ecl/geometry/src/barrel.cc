#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include <G4VisAttributes.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4AssemblyVolume.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Trd.hh>
#include <G4TwoVector.hh>
#include <G4ExtrudedSolid.hh>
#include <G4PVReplica.hh>
#include "G4UserLimits.hh"

#include <iostream>
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/Matrix.h"
#include "G4Vector3D.hh"
#include "G4Point3D.hh"

#include "ecl/geometry/BelleLathe.h"
#include "ecl/geometry/BelleCrystal.h"
#include "ecl/geometry/shapes.h"

using namespace std;

double diff(double phi1, double phi0)  // dphi = phi1-phi0 signed difference -M_PI<dphi<M_PI
{
  const double twopi = 2 * M_PI;
  double dphi = fmod(phi1 - phi0, twopi);
  dphi = dphi < -M_PI ? dphi + twopi : dphi;
  dphi = dphi > M_PI ? dphi - twopi : dphi;
  return dphi;
}

cplacement_t barrel_placement[] = {
  {
    13, // wt = 0.17
    2.05359892664, 1.55625378854, -3.12205398143,
    1607.42073108, 2.08499142065, 1.59474635403
  }, //-79.058446619
  {
    13, // wt = 0.17
    2.05618820967, 1.57087763031, 3.14153703384,
    1607.32792577, 2.08501919138, 1.55137368786
  }, //-79.057768571
  {
    14, // wt = 0.17
    2.09169544982, 1.55689064049, -3.12205245943,
    1640.39851637, 2.12167595093, 1.59472275521
  }, //-85.8645159722
  {
    14, // wt = 0.17
    2.09434112695, 1.5708528645, 3.14153392592,
    1640.34457515, 2.12175823042, 1.55134572992
  }, //-85.873192232
  {
    15, // wt = 0.17
    4.15424499018, 1.58411542517, 0.0195392543083,
    1676.01480914, 2.15750785606, 1.59469292401
  }, //-92.7884529141
  {
    15, // wt = 0.17
    4.15156587372, 1.57073051516, -4.60430903438e-05,
    1676.01386964, 2.15764725254, 1.55131878997
  }, //-92.8078559426
  {
    16, // wt = 0.17
    4.11809116502, 1.58356135544, 0.019527867035,
    1714.35440421, 2.19247910632, 1.59466614377
  }, //-99.8446717088
  {
    16, // wt = 0.17
    4.11630229388, 1.57075346975, -3.67305842542e-05,
    1714.56335016, 2.19273453773, 1.55128307849
  }, //-99.892438751
  {
    17, // wt = 0.17
    4.0828824867, 1.58303348399, 0.0194914370644,
    1755.46008356, 2.22655839989, 1.59463288512
  }, //-107.041535629
  {
    17, // wt = 0.17
    4.08079428387, 1.57071609451, -5.49432023802e-05,
    1755.68788792, 2.22683172367, 1.55125938152
  }, //-107.093456109
// R=1251.6+2, d[17,0]=0.00252, d[15,0]=0.00164, d[16,0]=0.00136, d[14,0]=-0.000444, d[13,0]=-0.00138, d[15,1]=-0.0153, d[17,1]=-0.0157, d[16,1]=-0.0163, d[14,1]=-0.0168, d[13,1]=-0.0171,
  {
    9, // wt = 0.17
    1.89367133252, 1.55343379049, -3.12217281406,
    1500.22743721, 1.92910855512, 1.59481748202
  }, //-52.6120989103
  {
    9, // wt = 0.17
    1.89356114198, 1.57087547186, 3.14151807741,
    1500.17166217, 1.92899145922, 1.55144599277
  }, //-52.5936917979
  {
    10, // wt = 0.17
    1.93449451637, 1.55422819726, -3.12209599577,
    1523.09300714, 1.96898199935, 1.59480993094
  }, //-59.0574128978
  {
    10, // wt = 0.17
    1.93665216907, 1.57086308682, 3.1415162219,
    1523.07693414, 1.96904430603, 1.55143292318
  }, //-59.0655369212
  {
    11, // wt = 0.17
    1.97439555907, 1.55491202156, -3.12208157982,
    1548.44242762, 2.00813581692, 1.59479229049
  }, //-65.5813152571
  {
    11, // wt = 0.17
    1.97694153542, 1.57081351606, -3.14157389891,
    1548.41898432, 2.00821160454, 1.55139545495
  }, //-65.5909527841
  {
    12, // wt = 0.17
    2.01405940849, 1.55556184621, -3.12216360665,
    1576.3585222, 2.04647232365, 1.59477664808
  }, //-72.1876836196
  {
    12, // wt = 0.17
    2.01524148285, 1.57090375427, 3.14153888881,
    1576.38747893, 2.04670670391, 1.55139938011
  }, //-72.2218533117
// R=1251.6+2, d[12,0]=0.111, d[9,0]=0.000735, d[10,0]=-0.00112, d[11,0]=-0.0012, d[9,1]=-0.0146, d[10,1]=-0.0166, d[11,1]=-0.017, d[12,1]=-0.0171,
  {
    5, // wt = 0.17
    1.72422815657, 1.55216326074, -3.12056614908,
    1432.59273274, 1.76227806579, 1.59483570543
  }, //-27.2642111073
  {
    5, // wt = 0.17
    1.72385692443, 1.57098611689, 3.14142805875,
    1432.5624647, 1.76219017725, 1.55148811317
  }, //-27.2512744922
  {
    6, // wt = 0.17
    1.76779502163, 1.55232406866, -3.12108415358,
    1445.79297914, 1.80462713055, 1.5948400424
  }, //-33.4998570439
  {
    6, // wt = 0.17
    1.76738707782, 1.57097805651, 3.1414284696,
    1445.76373474, 1.80451554996, 1.55148912726
  }, //-33.4834863244
  {
    7, // wt = 0.17
    1.81060238145, 1.55294297142, -3.12123200395,
    1461.38371419, 1.84640996518, 1.59483622478
  }, //-39.7697262062
  {
    7, // wt = 0.17
    1.81030214954, 1.57096379233, 3.14145821809,
    1461.3403359, 1.84631099586, 1.55148128404
  }, //-39.7546285875
  {
    8, // wt = 0.17
    1.85285271941, 1.55283597829, -3.12199528815,
    1479.34794859, 1.88764540615, 1.59483445856
  }, //-46.0926414733
  {
    8, // wt = 0.17
    1.85351511476, 1.56923013424, 3.13936049898,
    1479.33931288, 1.88768698314, 1.55155835607
  }, //-46.0982168475
// R=1251.6+2, d[8,0]=-2.81e-05, d[8,1]=-0.000341, d[7,0]=-0.000952, d[7,1]=-0.0157, d[5,0]=-0.0287, d[6,0]=-0.0295, d[6,1]=-0.0299, d[5,1]=-0.0433,
  {
    1, // wt = 0.17
    1.57129310522, 1.54946426772, -3.12030107088,
    1404.28782557, 1.59057806619, 1.59480020574
  }, //-2.77774440799
  {
    1, // wt = 0.17
    1.57072370025, 1.5711959185, 3.14121700114,
    1404.28702853, 1.59057997487, 1.55149726912
  }, //-2.77801081309
  {
    2, // wt = 0.17
    1.59115919898, 1.5499183077, -3.1202668963,
    1407.17385681, 1.63211001286, 1.5948094703
  }, //-8.62249671528
  {
    2, // wt = 0.17
    1.59012284692, 1.57129547407, 3.14111481445,
    1407.15426268, 1.63216214152, 1.55152875311
  }, //-8.62969816347
  {
    3, // wt = 0.17
    1.63503148169, 1.55027906457, -3.11977654139,
    1413.2240973, 1.67552526562, 1.5949217358
  }, //-14.7735050667
  {
    3, // wt = 0.17
    1.63400173086, 1.57122908027, 3.14110245102,
    1413.18701434, 1.67557442135, 1.55153916982
  }, //-14.7800259564
  {
    4, // wt = 0.17
    1.67867841956, 1.55168314673, -3.12025595543,
    1421.59539571, 1.71863731277, 1.59482915612
  }, //-20.940528821
  {
    4, // wt = 0.17
    1.67770035961, 1.57128680388, 3.14113939009,
    1421.58938325, 1.7186846648, 1.55155046829
  }, //-20.947098315
// R=1251.6+2, d[2,0]=0.000259, d[3,1]=-0.000121, d[1,1]=-0.000167, d[4,0]=-0.000178, d[2,1]=-0.000198, d[4,1]=-0.000304, d[3,0]=-0.000502, d[1,0]=-0.00123,
  {
    1, // wt = 0.17
    3.17093271748, 0.785519051904, 1.59290749265,
    1404.29147017, 1.55098439015, 1.5947994423
  }, //2.78199136069
  {
    1, // wt = 0.17
    3.1400302684, 0.785407759371, 1.57111948239,
    1404.2876165, 1.55098381565, 1.55152541325
  }, //2.7820643874
  {
    2, // wt = 0.17
    3.17296626767, 0.774110946043, 1.59272676294,
    1407.28287436, 1.50956324209, 1.59471244755
  }, //8.61184311887
  {
    2, // wt = 0.17
    3.14164652791, 0.775554767341, 1.57078977313,
    1407.19480077, 1.5094011837, 1.5514205315
  }, //8.63406607306
  {
    3, // wt = 0.17
    3.1727273148, 0.75306184582, 1.59207573314,
    1413.23042432, 1.46595387499, 1.59481957347
  }, //14.7895252173
  {
    3, // wt = 0.17
    3.14087563292, 0.753506180328, 1.57026733762,
    1413.20538425, 1.46595893906, 1.55154462402
  }, //14.7885514442
  {
    4, // wt = 0.17
    3.17336784812, 0.731215705871, 1.59207121307,
    1421.62768013, 1.42283966042, 1.59482871048
  }, //20.9572702894
  {
    4, // wt = 0.17
    3.14089392369, 0.731530002676, 1.57024697713,
    1421.61659535, 1.42282238851, 1.5515526834
  }, //20.9595354537
// R=1251.6+2, d[2,0]=0.00968, d[1,1]=0.00201, d[3,1]=0.000978, d[4,1]=-3.76e-06, d[2,1]=-0.00172, d[1,0]=-0.00429, d[3,0]=-0.00449, d[4,0]=-0.00465,
  {
    5, // wt = 0.17
    3.17178776144, 0.709013934408, 1.59423135166,
    1432.57469967, 1.37937263677, 1.59484186986
  }, //27.2557039066
  {
    5, // wt = 0.17
    3.14133048546, 0.708865133307, 1.57058475256,
    1432.57702201, 1.37940203517, 1.55148837154
  }, //27.2516134584
  {
    6, // wt = 0.17
    3.17232277707, 0.686945879393, 1.59426742982,
    1445.78086755, 1.33698342114, 1.59484321967
  }, //33.4970591542
  {
    6, // wt = 0.17
    3.14142488742, 0.687150693106, 1.57062243028,
    1445.76452883, 1.33708370784, 1.55147755321
  }, //33.4825758606
  {
    7, // wt = 0.17
    3.1725773932, 0.665562353083, 1.59453054495,
    1461.36620292, 1.29520006024, 1.59483943709
  }, //39.7668068058
  {
    7, // wt = 0.17
    3.14158173722, 0.665194109437, 1.57078459662,
    1461.3934595, 1.29518859198, 1.55144691517
  }, //39.7691612284
  {
    8, // wt = 0.17
    3.17306085959, 0.644445251895, 1.59458559254,
    1479.31264412, 1.25399836012, 1.59483523215
  }, //46.084356635
  {
    8, // wt = 0.17
    3.13822115906, 0.644036809879, 1.5726707129,
    1479.34329738, 1.25389966177, 1.55155994114
  }, //46.0991856408
// R=1251.6+2, d[8,1]=0.000696, d[7,1]=-0.0028, d[8,0]=-0.00354, d[7,0]=-0.00486, d[5,1]=-0.0292, d[6,1]=-0.0303, d[5,0]=-0.0339, d[6,0]=-0.034,
  {
    9, // wt = 0.17
    3.17295883076, 0.624737551647, 1.5949973679,
    1500.3007717, 1.21239145828, 1.59481494772
  }, //52.6276865837
  {
    9, // wt = 0.17
    3.14116957338, 0.624465798241, 1.57079898973,
    1500.87615946, 1.21258314457, 1.55145608562
  }, //52.6209274829
  {
    10, // wt = 0.17
    3.17339325489, 0.60419860751, 1.59501178153,
    1523.14871593, 1.17255693882, 1.59480004347
  }, //59.0671144757
  {
    10, // wt = 0.17
    3.14063351464, 0.603919271564, 1.5713331912,
    1523.63768461, 1.17274108462, 1.55144520588
  }, //59.0602139485
  {
    11, // wt = 0.17
    3.17367271901, 0.583870714328, 1.59515456564,
    1549.10263596, 1.13345497511, 1.59479525806
  }, //65.6095383515
  {
    11, // wt = 0.17
    3.14015955687, 0.583735660249, 1.57154337407,
    1548.34003321, 1.13362555979, 1.55144449473
  }, //65.5533122828
  {
    12, // wt = 0.17
    3.17393024738, 0.564027170323, 1.59523900989,
    1576.24309753, 1.09510749245, 1.59475721322
  }, //72.1841967204
  {
    12, // wt = 0.17
    3.1414160022, 0.56403074359, 1.57080919127,
    1576.48134349, 1.09488481905, 1.55138799231
  }, //72.2263121309
// R=1251.6+2, d[9,1]=0.621, d[11,0]=0.602, d[10,1]=0.541, d[12,0]=0.00469, d[9,0]=0.00128, d[10,0]=0.00126, d[11,1]=-0.00214, d[12,1]=-0.00357,
  {
    13, // wt = 0.17
    3.17404683482, 0.544323282144, 1.59554773846,
    1609.26542286, 1.05667204165, 1.59475737012
  }, //79.1392531683
  {
    13, // wt = 0.17
    3.14163310881, 0.543173713211, 1.57068592533,
    1607.3208324, 1.05665758973, 1.55135788529
  }, //79.045646135
  {
    14, // wt = 0.17
    3.17455161869, 0.525414348377, 1.59526641567,
    1641.4972075, 1.01999334483, 1.5947321015
  }, //85.9113056272
  {
    14, // wt = 0.17
    3.14164871388, 0.52561697379, 1.57072895947,
    1640.78700026, 1.01971784062, 1.55132700285
  }, //85.9126509871
  {
    15, // wt = 0.17
    3.17491346373, 0.506802990402, 1.59530067232,
    1676.04283761, 0.984171396712, 1.59468050048
  }, //92.7779172034
  {
    15, // wt = 0.17
    3.14125199049, 0.506889414107, 1.57090126076,
    1676.33673461, 0.983898913725, 1.55129991428
  }, //92.8322232272
  {
    16, // wt = 0.17
    3.17526853775, 0.488597712991, 1.5953559203,
    1715.19923959, 0.949246935037, 1.59466533891
  }, //99.8752763114
  {
    16, // wt = 0.17
    3.1412991738, 0.488121620488, 1.57083958856,
    1714.67229014, 0.948873592909, 1.55128409274
  }, //99.8966288253
// R=1251.6+2, d[13,0]=1.67, d[14,0]=0.993, d[16,0]=0.82, d[15,0]=0.0918, d[14,1]=0.0521, d[15,1]=-0.00139, d[16,1]=-0.00259, d[13,1]=-0.00388,
  {
    17, // wt = 0.17
    3.17364923203, 0.470658004314, 1.59628486026,
    1756.79685883, 0.914146474334, 1.59457480356
  }, //107.246620188
  {
    17, // wt = 0.17
    3.1416175997, 0.472379427457, 1.57075192819,
    1759.67441221, 0.914161184018, 1.55123192532
  }, //107.420235123
  {
    18, // wt = 0.17
    3.17418632945, 0.453653976998, 1.59639598354,
    1806.24057851, 0.881133407221, 1.59464825167
  }, //114.926965109
  {
    18, // wt = 0.17
    3.14165935867, 0.456027695927, 1.57065132673,
    1801.27396478, 0.881002842756, 1.55120133723
  }, //114.629093229
  {
    19, // wt = 0.17
    3.17428464668, 0.436822615001, 1.59626381369,
    1849.90053609, 0.848965623958, 1.59456377009
  }, //122.234009162
  {
    19, // wt = 0.17
    3.14126519657, 0.43905710382, 1.57083219885,
    1848.79415679, 0.848925777686, 1.55117059978
  }, //122.166433425
  {
    20, // wt = 0.17
    3.17458995213, 0.420457150788, 1.59624975742,
    1899.98168483, 0.817719956395, 1.59453585104
  }, //129.937178065
  {
    20, // wt = 0.17
    3.14150111477, 0.421501749541, 1.57070952903,
    1898.40367486, 0.817578247801, 1.55112912686
  }, //129.84888619
// R=1251.6+2, d[18,0]=4.5, d[20,0]=2.28, d[17,1]=2.13, d[19,0]=2.06, d[19,1]=0.719, d[20,1]=0.64, d[17,0]=0.184, d[18,1]=0.0613,
  {
    21, // wt = 0.17
    3.17346176848, 0.404586464713, 1.59695622419,
    1951.95340652, 0.78628315919, 1.5944016865
  }, //137.901744381
  {
    21, // wt = 0.17
    3.14161687686, 0.40718269483, 1.57063042128,
    1959.21938611, 0.786186064699, 1.5510849678
  }, //138.428534326
  {
    22, // wt = 0.17
    3.17400151972, 0.38919854998, 1.59674003304,
    2011.6256917, 0.756922112419, 1.59444947462
  }, //146.235732508
  {
    22, // wt = 0.17
    3.14166903015, 0.391304360373, 1.57061616624,
    2010.32569947, 0.756843991188, 1.55105437808
  }, //146.1520131
  {
    23, // wt = 0.17
    3.1741500203, 0.374268111772, 1.5968319872,
    2070.48484306, 0.728445582269, 1.59440264792
  }, //154.50166928
  {
    23, // wt = 0.17
    3.14151407764, 0.376847996374, 1.57059528631,
    2069.74719022, 0.728432600774, 1.55101098439
  }, //154.448413526
  {
    24, // wt = 0.17
    3.1743296324, 0.359790838831, 1.59696157348,
    2136.31870415, 0.700901006733, 1.59441128282
  }, //163.270599297
  {
    24, // wt = 0.17
    3.14164528813, 0.360890298071, 1.57069718089,
    2130.87741844, 0.700745955143, 1.55097931546
  }, //162.876048308
// R=1251.6+2, d[21,1]=4.56, d[24,0]=4.44, d[22,0]=2.69, d[23,0]=2.36, d[23,1]=1.21, d[22,1]=1.15, d[24,1]=0.29, d[21,0]=0.159,
  {
    25, // wt = 0.17
    3.17559442703, 0.3456858562, 1.59625824869,
    2199.02068966, 0.673141340289, 1.59428993848
  }, //171.934187099
  {
    25, // wt = 0.17
    3.14124687169, 0.345348169841, 1.57095920643,
    2200.67398792, 0.673497779991, 1.55094695757
  }, //172.014538605
  {
    26, // wt = 0.17
    3.17583590037, 0.333809659838, 1.59628427856,
    2269.7569557, 0.647204581621, 1.59426074394
  }, //181.074953365
  {
    26, // wt = 0.17
    3.14153870212, 0.333048117328, 1.57072851308,
    2270.57930498, 0.647566310428, 1.55089798729
  }, //181.091023232
  {
    27, // wt = 0.17
    3.1763401877, 0.320756750932, 1.59610038283,
    2342.54934164, 0.622339676108, 1.59421375963
  }, //190.336068601
  {
    27, // wt = 0.17
    3.14155771347, 0.320076422791, 1.57080230998,
    2344.69607843, 0.622660940408, 1.550859438
  }, //190.466573953
  {
    28, // wt = 0.17
    3.17633973481, 0.308112590213, 1.59618381681,
    2419.70640958, 0.598314960679, 1.59417780465
  }, //199.936925817
  {
    28, // wt = 0.17
    3.14116300947, 0.307442128712, 1.57103630623,
    2421.05349583, 0.598591050989, 1.55081630492
  }, //200.010576679
  {
    29, // wt = 0.17
    3.17578222994, 0.295775884572, 1.59648901694,
    2505.0500088, 0.575136487403, 1.59419268557
  }, //210.203272365
  {
    29, // wt = 0.17
    3.1412887039, 0.295256239374, 1.57086022045,
    2500.02598307, 0.575314386614, 1.5507779983
  }, //209.757501678
// R=1251.6+2, d[29,0]=2.43, d[25,1]=1.95, d[27,1]=1.86, d[26,1]=1.5, d[28,1]=1.33, d[25,0]=0.387, d[26,0]=0.354, d[28,0]=0.0235, d[27,0]=0.0131, d[29,1]=0.00645,
};

vector<cplacement_t> load_placements(const string& fname)
{
  vector<cplacement_t> res;
  ifstream IN(fname.c_str());
  while (IN) {
    double wt;
    cplacement_t t;
    IN >> t.nshape >> wt >> t.Rphi1 >> t.Rtheta >> t.Rphi2 >> t.Pr >> t.Ptheta >> t.Pphi;

    t.Rphi1  *= CLHEP::degree;
    t.Rtheta *= CLHEP::degree;
    t.Rphi2  *= CLHEP::degree;
    t.Pr     *= CLHEP::cm;
    t.Ptheta *= CLHEP::degree;
    t.Pphi   *= CLHEP::degree;
    res.push_back(t);

  }
  return res;
}

Point_t p1[]  = {{0.0, 0.0}, {263.4, 0.0}, {263.4, 197.2}, {248.6, 206.6}, {212.0, 152.6}, {145.2, 199.0}, {108.5, 146.6}, {46.4, 192.1}, {7.9, 140.5}, { -50.0, 185.2}, { -90.3, 134.2}, { -143.8, 178.3}, { -170.7, 145.0}};
Point_t p2[]  = {{0.0, 0.0}, {384.8, 0.0}, {181.9, 172.5}, {136.8, 122.3}, {90.7, 163.9}, {46.5, 115.6}, {4.1, 156.3}, { -42.5, 109.9}, { -80.9, 149.5}, { -110.9, 119.8}};
Point_t p3[]  = {{0.0, 0.0}, {343.2, 0.0}, {209.6, 144.2}, {158.0, 98.1}, {125.8, 135.0}, {74.3, 92.4}, {46.1, 128.4}, { -6.9, 87.3}, { -32.5, 121.3}, { -67.3, 95.8}};
Point_t p4[]  = {{0.0, 0.0}, {311.5, 0.0}, {229.5, 116.7}, {170.3, 76.5}, {150.6, 107.6}, {92.8, 71.9}, {75.0, 101.2}, {16.1, 67.5}, {1.0, 95.2}, { -37.8, 75.6}};
Point_t p5[]  = {{0.0, 0.0}, {289.2, 0.0}, {243.1, 92.2}, {177.4, 60.9}, {166.6, 85.0}, {103.3, 57.8}, {94.3, 79.8}, {30.0, 55.5}, {22.9, 75.3}, { -19.3, 61.8}};
Point_t p6[]  = {{0.0, 0.0}, {276.1, 0.0}, {253.3, 73.3}, {185.2, 53.4}, {180.7, 69.1}, {110.5, 51.9}, {107.5, 65.3}, {38.7, 51.8}, {36.7, 62.8}, { -7.5, 56.8}};
Point_t p7[]  = {{0.0, 0.0}, {253.1, 0.0}, {244.8, 62.7}, {171.0, 54.0}, {170.2, 60.3}, {100.8, 55.6}, {100.3, 59.6}, {38.2, 58.1}, {38.1, 59.5}, {0.0, 60.9}};
Point_t p11[] = {{0.0, 0.0}, {314.1, 0.0}, {351.9, 75.6}, {312.4, 96.9}, {297.5, 69.4}, {238.2, 102.9}, {221.4, 73.2}, {163.1, 108.8}, {143.5, 77.4}, {87.1, 114.8}, {64.3, 81.7}, {9.3, 121.5}, {0.0, 108.4}};
Point_t* pp[] = {p1, p2, p3, p4, p5, p6, p7, p11};
int npp[] = {13, 10, 10, 10, 10, 10, 10, 13};

G4VSolid* get_crystal_support(int n)
{
  std::vector<G4TwoVector> p; p.reserve(npp[n]);
  for (int i = 0; i < npp[n]; i++)
    p.push_back(G4TwoVector(pp[n][i].x, pp[n][i].y));

  string name("barrel_crystal_support");
  name += to_string(n);

  G4TwoVector off(0, 0);
  return new G4ExtrudedSolid(name, p, 1.5, off, 1, off, 1);
}

namespace  Belle2 {
  namespace ECL {
    void barrel(G4LogicalVolume* top);
  };
};

void Belle2::ECL::barrel(G4LogicalVolume* top)
{
  // bool b_crystals = false;
  // bool b_forward_support_legs = false;
  // bool b_backward_support_legs = false;
  // bool b_forward_support_ring = true;
  // bool b_backward_support_ring = false;
  // bool b_forward_support_wall = false;
  // bool b_backward_support_wall = false;
  // bool b_crystal_support = false;
  // bool b_preamplifier = false;
  // bool b_septum_walls = false;
  // bool b_inner_support_wall = false;
  // bool b_support_ribs = true;
  // bool b_outer_plates = true;
  // bool b_forward_part5 = true;

  bool b_crystals = true;
  bool b_forward_support_legs = true;
  bool b_backward_support_legs = true;
  bool b_forward_support_ring = true;
  bool b_backward_support_ring = true;
  bool b_forward_support_wall = true;
  bool b_backward_support_wall = true;
  bool b_crystal_support = true;
  bool b_preamplifier = true;
  bool b_septum_walls = true;
  bool b_inner_support_wall = true;
  bool b_support_ribs = true;
  bool b_outer_plates = 1;
  bool b_forward_part5 = true;
  bool b_crystal_holder = true;
  bool b_pipe = true;

  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  G4Material* sus304 = new G4Material("SUS304", 8.00 * CLHEP::g / CLHEP::cm3, 3);
  sus304->AddElement(nist->FindOrBuildElement("Fe"), 0.74);
  sus304->AddElement(nist->FindOrBuildElement("Cr"), 0.18);
  sus304->AddElement(nist->FindOrBuildElement("Ni"), 0.08);
  cout << "SUS304 radiation length = " << sus304->GetRadlen() << " mm" << endl;

  G4Material* a5083 = new G4Material("A5083", 2.65 * CLHEP::g / CLHEP::cm3, 2);
  a5083->AddElement(nist->FindOrBuildElement("Al"), 0.955);
  a5083->AddElement(nist->FindOrBuildElement("Mg"), 0.045);
  cout << "A5083 radiation length = " << a5083->GetRadlen() << " mm" << endl;

  G4Material* a5052 = new G4Material("A5052", 2.68 * CLHEP::g / CLHEP::cm3, 2);
  a5052->AddElement(nist->FindOrBuildElement("Al"), 0.975);
  a5052->AddElement(nist->FindOrBuildElement("Mg"), 0.025);
  cout << "A5052 radiation length = " << a5052->GetRadlen() << " mm" << endl;

  G4Material* a6063 = new G4Material("A6063", 2.68 * CLHEP::g / CLHEP::cm3, 3);
  a6063->AddElement(nist->FindOrBuildElement("Al"), 0.98925);
  a6063->AddElement(nist->FindOrBuildElement("Mg"), 0.00675);
  a6063->AddElement(nist->FindOrBuildElement("Si"), 0.004);
  cout << "A6063 radiation length = " << a6063->GetRadlen() << " mm" << endl;

  G4Material* c1220 = new G4Material("C1220", 2.68 * CLHEP::g / CLHEP::cm3, 1); // c1200 contains >99.9% of copper
  c1220->AddElement(nist->FindOrBuildElement("Cu"), 1);
  cout << "Pure copper radiation length = " << c1220->GetRadlen() << " mm" << endl;

  double wrapm = 2.699 * 0.0025 + 1.4 * 0.0025 + 2.2 * 0.02;
  G4Material* medWrap = new G4Material("WRAP", wrapm / 0.025 * CLHEP::g / CLHEP::cm3, 3);
  medWrap->AddMaterial(nist->FindOrBuildMaterial("G4_Al"), 2.699 * 0.0025 / wrapm);
  medWrap->AddMaterial(nist->FindOrBuildMaterial("G4_MYLAR"), 1.4 * 0.0025 / wrapm);
  medWrap->AddMaterial(nist->FindOrBuildMaterial("G4_TEFLON"), 2.2 * 0.02 / wrapm);

  G4VisAttributes* att_iron = new G4VisAttributes(G4Colour(1., 0.1, 0.1));
  G4VisAttributes* att_iron2 = new G4VisAttributes(G4Colour(1., 0.5, 0.5));
  G4VisAttributes* att_alum = new G4VisAttributes(G4Colour(0.25, 0.25, 1.0, 0.5));
  G4VisAttributes* att_alum2 = new G4VisAttributes(G4Colour(0.5, 0.5, 1.0));
  G4VisAttributes* att_silv = new G4VisAttributes(G4Colour(0.9, 0., 0.9));
  G4VisAttributes* att_air = new G4VisAttributes(G4Colour(0., 1., 1.)); att_air->SetVisibility(false);

  double phi0 = M_PI / 2 - M_PI / 72, dphi = 2 * M_PI; //2*2*M_PI/72;

  int nseg = 72;

  int overlap = 0;

  G4LogicalVolume* sectorlogical; // tilted sector
  {
    double R = 1251.6;
    double zp = 4.9, zm = 2.5;
    //    zr_t bint[] = {{-1000.8-zm, 0}, {1982.6 + zp,0}, {2288.,198.2 - zp*tand(32.982)}, {2288.,404 + 12}, {-1219.,404 + 12}, {-1219., 404 -112.6-zm*tand(52.897)-3.2}};
    zr_t bint[] = {{ -1000.8 - zm, 0}, {1982.6 + zp, 0}, {2288., 198.2 - zp * tand(32.982)}, {2288., 1582 - R}, {2380, 1582 - R}, {2380, 404 + 12}, { -1330., 404 + 12}, { -1330., 1543 + 84 - R}, { -1219., 1543 + 84 - R}, { -1219., 404 - 112.6 - zm * tand(52.897) - 3.2}};
    for (unsigned int i = 0; i < sizeof(bint) / sizeof(zr_t); i++) bint[i].r += R;
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    G4VSolid* sect0 = new BelleLathe("sect0", phi0, 2 * M_PI, contourb);

    G4Vector3D n0(cos(phi0), sin(phi0), 0);
    // shift r0 by half of septum wall thickness so from one side of the sector the wall will be completely inside the sector volume
    G4Point3D r0 = R * n0 + 0.5 / 2 * (G4RotateZ3D(M_PI / 2) * n0), rt0 = r0 + G4RotateZ3D(M_PI / 144 - M_PI / 72) * (G4Vector3D(0, 417,
                   0)), r1 = G4RotateZ3D(2 * M_PI / 72) * r0, rt1 = G4RotateZ3D(2 * M_PI / 72) * rt0;
    G4Transform3D t0 = G4RotateZ3D(-M_PI / 144) * G4Translate3D(-r0);
    G4Point3D tr0 = t0 * r0, trt0 = t0 * rt0, tr1 = t0 * r1, trt1 = t0 * rt1;
    G4Vector3D tn1 = trt1 - tr1;
    double v0 = -(tr1.y() - tr0.y()) / tn1.y();
    double v1 = -(tr1.y() - trt0.y()) / tn1.y();

    G4Point3D u0 = tr1 + v0 * tn1, u1 = tr1 + v1 * tn1;
    G4VSolid* sect1 = new G4Trd("sect1", -u0.x() / 2, (trt0 - u1).x() / 2,  2000, 2000, trt0.y() / 2);
    G4IntersectionSolid* sector =  new G4IntersectionSolid("sector", sect0, sect1,
                                                           G4Translate3D(r0)*G4RotateZ3D(M_PI / 144)*G4Translate3D(u0.x() / 2, trt0.y() / 2, (2288 - 1219.) / 2)*G4RotateX3D(-M_PI / 2));
    sectorlogical = new G4LogicalVolume(sector, nist->FindOrBuildMaterial("G4_AIR"), "sectorlogical", 0, 0, 0);
    sectorlogical->SetVisAttributes(att_air);
    for (int i = 0; i < nseg; i++) {
      double phi = i * M_PI / 36;
      new G4PVPlacement(G4RotateZ3D(phi), sectorlogical, suf("sectorphysical", i), top, false, i, overlap);
    }
  }

  // phi septum wall
  if (b_septum_walls) {
    double zh = (0.5 - 0.001) / 2;
    G4ThreeVector psw[] = {G4ThreeVector(-1000.8, 0, -zh), G4ThreeVector(1982.6, 0, -zh), G4ThreeVector(2288, 198.2, -zh),
                           G4ThreeVector(2288, 404, -zh), G4ThreeVector(-1219., 404, -zh), G4ThreeVector(-1219., 404 - 112.6, -zh),
                           G4ThreeVector(-1000.8, 0, zh), G4ThreeVector(1982.6, 0, zh), G4ThreeVector(2288, 198.2, zh),
                           G4ThreeVector(2288, 404, zh), G4ThreeVector(-1219., 404, zh), G4ThreeVector(-1219., 404 - 112.6, zh)
                          };
    G4VSolid* pswsolid = new BelleCrystal("pswsolid", 6, psw);
    G4LogicalVolume* pswlogical = new G4LogicalVolume(pswsolid, nist->FindOrBuildMaterial("A5052"), "pswlogical", 0, 0, 0);
    pswlogical->SetVisAttributes(att_alum);
    new G4PVPlacement(G4RotateZ3D(2.5 / 180 * M_PI)*G4Translate3D(0, 1251.6 + 0.01,
                      0)*G4RotateZ3D(1.25 / 180 * M_PI)*G4RotateY3D(-M_PI / 2), pswlogical, "phiwallphysical", sectorlogical, false, 0, overlap);
  }

  if (b_forward_support_ring) {
    zr_t vc4[] = {{0, 1449.2}, {0, 1582}, {90, 1582}, {90, 1582 - 75.7}};
    std::vector<zr_t> contour4(vc4, vc4 + 4);
    G4VSolid* part4solid = new BelleLathe("part4solid", phi0, dphi, contour4);
    G4LogicalVolume* part4logical = new G4LogicalVolume(part4solid, nist->FindOrBuildMaterial("SUS304"), "part4logical", 0, 0, 0);
    part4logical->SetVisAttributes(att_iron);
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, 2290), part4logical, "part4physical", top, false, 0, overlap);
  }

  if (b_backward_support_ring) {
    zr_t vc6[] = {{0, 1543}, { -105, 1543}, { -105, 1543 + 84}, {0, 1543 + 84}};
    std::vector<zr_t> contour6(vc6, vc6 + 4);
    G4VSolid* part6solid = new BelleLathe("part6solid", phi0, dphi, contour6);
    G4LogicalVolume* part6logical = new G4LogicalVolume(part6solid, nist->FindOrBuildMaterial("SUS304"), "part6logical", 0, 0, 0);
    part6logical->SetVisAttributes(att_iron);
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, -1225), part6logical, "part6physical", top, false, 0, overlap);
  }

  if (b_forward_support_wall) {
    zr_t vc2[] = {{0, 1180.1}, {0, 1249.5}, {54.3, 1249.5}, {445.8, 1503.6}, {445.8 + 4 * sind(32.982), 1503.6 - 4 * cosd(32.982)}, {54.3 + 4 * sind(32.982 / 2), 1249.5 - 4}, {4, 1249.5 - 4}, {4, 1180.1}};
    std::vector<zr_t> contour2(vc2, vc2 + 8);
    G4VSolid* part2solid = new BelleLathe("part2solid", phi0, dphi, contour2);
    G4LogicalVolume* part2logical = new G4LogicalVolume(part2solid, nist->FindOrBuildMaterial("A5083"), "part2logical", 0, 0, 0);
    part2logical->SetVisAttributes(att_alum);
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, 1930), part2logical, "part2physical", top, false, 0, overlap);
  }

  if (b_backward_support_wall) {
    zr_t vc3[] = {{0, 1180.1}, { -4, 1180.1}, { -4, 1249.5 - 4}, { -61.9 - 4 * sind(52.897 / 2), 1249.5 - 4}, { -285, 1539.1 - 4 * sind(52.897 / 2)}, { -285, 1618.8}, { -285 + 4, 1618.8}, { -285 + 4, 1539.1}, { -61.9, 1249.5}, {0, 1249.5}};
    std::vector<zr_t> contour3(vc3, vc3 + 10);
    G4VSolid* part3solid = new BelleLathe("part3solid", phi0, dphi, contour3);
    G4LogicalVolume* part3logical = new G4LogicalVolume(part3solid, nist->FindOrBuildMaterial("A5083"), "part3logical", 0, 0, 0);
    part3logical->SetVisAttributes(att_alum);
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, -940), part3logical, "part3physical", top, false, 0, overlap);
  }

  if (b_inner_support_wall) {
    G4VSolid* part1solid = new G4Tubs("part1solid", 1250.1, 1250.1 + 1.5, (988.5 + 1972.5) / 2, phi0, dphi);
    G4LogicalVolume* part1logical = new G4LogicalVolume(part1solid, nist->FindOrBuildMaterial("A5083"), "part1logical", 0, 0, 0);
    part1logical->SetVisAttributes(att_alum2);
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, (-988.5 + 1972.5) / 2), part1logical, "part1physical", top, false, 0, overlap);
  }

  {
    //    double zh = (1219.+2288.)/2, dz = (-1219.+2288.)/2;
    double zh = (1330. + 2380.) / 2, dz = (-1330. + 2380.) / 2;
    if (b_support_ribs) {
      G4ThreeVector vc9[] = {G4ThreeVector(0, 0, -zh), G4ThreeVector(25, 0, -zh), G4ThreeVector(25, 24.5 - 25.0 * tand(2.5), -zh), G4ThreeVector(0, 24.5, -zh),
                             G4ThreeVector(0, 0, zh), G4ThreeVector(25, 0, zh), G4ThreeVector(25, 24.5 - 25.0 * tand(2.5), zh), G4ThreeVector(0, 24.5, zh)
                            };
      G4VSolid* part9solid = new BelleCrystal("part9solid", 4, vc9);
      G4LogicalVolume* part9logical = new G4LogicalVolume(part9solid, nist->FindOrBuildMaterial("SUS304"), "part9logical", 0, 0, 0);
      part9logical->SetVisAttributes(att_silv);
      G4Transform3D tpart9a = G4RotateZ3D(2.5 / 180 * M_PI) * G4Translate3D(0, 1251.6,
                              0) * G4RotateZ3D(1.25 / 180 * M_PI) * G4Translate3D(0.5 / 2, 404 - 25 - 3.7 + 0.2, dz);
      G4Transform3D tpart9b = G4RotateZ3D(-2.5 / 180 * M_PI) * G4Translate3D(0, 1251.6,
                              0) * G4RotateZ3D(1.25 / 180 * M_PI) * G4Translate3D(-0.5 / 2, 404 - 25 - 3.7 + 0.5, dz) * G4RotateY3D(M_PI);
      new G4PVPlacement(tpart9a, part9logical, "part9aphysical", sectorlogical, false, 0, overlap);
      new G4PVPlacement(tpart9b, part9logical, "part9bphysical", sectorlogical, false, 0, overlap);
    }

    if (b_outer_plates) {
      G4VSolid* part10solid = new G4Box("part10solid", 143. / 2, 8. / 2, zh);
      G4LogicalVolume* part10logical = new G4LogicalVolume(part10solid, nist->FindOrBuildMaterial("SUS304"), "part10logical", 0, 0, 0);
      part10logical->SetVisAttributes(att_iron2);
      G4Transform3D tpart10 = G4Translate3D(0, 1251.6, 0) * G4Translate3D(-404 * tand(1.25), 404 - 1.1,
                              dz) * G4RotateZ3D(0.3 / 180 * M_PI);
      new G4PVPlacement(tpart10, part10logical, "part10physical", sectorlogical, false, 0, overlap);
    }

    // G4VSolid *part5solid = new G4Box("part5solid", 60./2, 45./2, 90./2);
    // G4LogicalVolume* part5logical = new G4LogicalVolume(part5solid, nist->FindOrBuildMaterial("SUS304"), "part5logical", 0, 0, 0);
    // part5logical->SetVisAttributes(att_iron2);
    // new G4PVPlacement(G4RotateZ3D(2.796/180*M_PI)*G4Translate3D(0,1582+45./2,2380.-90./2), part5logical, "part5physical", sectorlogical, false, 1, overlap);


    if (b_forward_part5) {
      double R = 1251.6, phi1 = phi0 + 1.25 * M_PI / 180, phi2 = (2.796 + 90 - 5) / 180 * M_PI;
      G4Vector3D n0(cos(phi0), sin(phi0), 0), n1(cos(phi1), sin(phi1), 0), n2(cos(phi2), sin(phi2), 0);
      // shift r0 by half of septum wall thickness so from one side of the sector the wall will be completely inside the sector volume
      G4Point3D r0 = R * n0 + 0.5 / 2 * (G4RotateZ3D(M_PI / 2) * n0);

      //      n2*(r0 + t*n1 - n2*1582) = 0;
      double t0 = -n2 * (r0 - n2 * 1582) / (n2 * n1);
      double t1 = -n2 * (r0 - n2 * (1582 + 45)) / (n2 * n1);

      G4Point3D x0 = r0 + t0 * n1;
      G4Point3D x1 = (t1 - t0) * n1;

      Point_t c5a[] = {{0, 0}, {30 * cosd(5 - 2.796), -30 * sind(5 - 2.796)}, {30 * cosd(5 - 2.796) + 45 * cos(phi2), -30 * sind(5 - 2.796) + 45 * sin(phi2)}, {x1.x(), x1.y()}};
      G4ThreeVector vc5a[8];
      for (int i = 0; i < 4; i++) vc5a[i] = G4ThreeVector(c5a[i].x, c5a[i].y, -90. / 2);
      for (int i = 0; i < 4; i++) vc5a[i + 4] = G4ThreeVector(c5a[i].x, c5a[i].y, 90. / 2);
      G4VSolid* part5asolid = new BelleCrystal("part5asolid", 4, vc5a);
      G4LogicalVolume* part5alogical = new G4LogicalVolume(part5asolid, nist->FindOrBuildMaterial("SUS304"), "part5alogical", 0, 0, 0);
      part5alogical->SetVisAttributes(att_iron2);
      new G4PVPlacement(G4RotateZ3D(2 * M_PI / 72)*G4Translate3D(x0.x(), x0.y(), 2380. - 90. / 2), part5alogical, "part5aphysical",
                        sectorlogical, false, 0, overlap);

      Point_t c5b[] = {{ -30 * cosd(5 - 2.796), 30 * sind(5 - 2.796)}, {0, 0}, {x1.x(), x1.y()}, { -30 * cosd(5 - 2.796) + 44.8 * cos(phi2), 30 * sind(5 - 2.796) + 44.8 * sin(phi2)}};
      G4ThreeVector vc5b[8];
      for (int i = 0; i < 4; i++) vc5b[i] = G4ThreeVector(c5b[i].x, c5b[i].y, -90. / 2);
      for (int i = 0; i < 4; i++) vc5b[i + 4] = G4ThreeVector(c5b[i].x, c5b[i].y, 90. / 2);
      G4VSolid* part5bsolid = new BelleCrystal("part5bsolid", 4, vc5b);
      G4LogicalVolume* part5blogical = new G4LogicalVolume(part5bsolid, nist->FindOrBuildMaterial("SUS304"), "part5blogical", 0, 0, 0);
      part5blogical->SetVisAttributes(att_iron2);
      new G4PVPlacement(G4Translate3D(x0.x(), x0.y(), 2380. - 90. / 2), part5blogical, "part5bphysical", sectorlogical, false, 0,
                        overlap);
    }

  }
#if 0
  G4AssemblyVolume* pblogical = new G4AssemblyVolume();
  G4ThreeVector vc9[] = {G4ThreeVector(0, 0, -3710. / 2), G4ThreeVector(25, 0, -3710. / 2), G4ThreeVector(0, 24.5, -3710. / 2), G4ThreeVector(25, 24.5 - 25.0 * tand(2.5), -3710. / 2),
                         G4ThreeVector(0, 0, 3710. / 2), G4ThreeVector(25, 0, 3710. / 2), G4ThreeVector(0, 24.5, 3710. / 2), G4ThreeVector(25, 24.5 - 25.0 * tand(2.5), 3710. / 2)
                        };
  G4VSolid* part9solid = new BelleCrystal("part9solid", 4, vc9);
  G4LogicalVolume* part9logical = new G4LogicalVolume(part9solid, nist->FindOrBuildMaterial("SUS304"), "part9logical", 0, 0, 0);
  part9logical->SetVisAttributes(att_iron);

  G4VSolid* part10solid = new G4Box("part10solid", 143. / 2, 8. / 2, 3710. / 2);
  G4LogicalVolume* part10logical = new G4LogicalVolume(part10solid, nist->FindOrBuildMaterial("SUS304"), "part10logical", 0, 0, 0);
  part10logical->SetVisAttributes(att_iron2);

  G4Transform3D tpart9a(G4Translate3D(143. / 2 - 24.5 * sind(2.5) + 0.5, 24.5 * cosd(2.5), 0)*G4RotateZ3D(-(180 - 2.5)*M_PI / 180));
  G4Transform3D tpart9b(G4Translate3D(-143. / 2 + 24.5 * sind(2.5) - 0.5, 24.5 * cosd(2.5),
                                      0)*G4RotateZ3D(-(180 + 2.5)*M_PI / 180)*G4RotateY3D(M_PI));

  pblogical->AddPlacedVolume(part9logical, tpart9a);
  pblogical->AddPlacedVolume(part9logical, tpart9b);
  //  G4Transform3D tpart10 = G4Translate3D(0,-4.,0);
  //  pblogical->AddPlacedVolume(part10logical,tpart10);

  G4VSolid* part5solid = new G4Box("part5solid", 60. / 2, 45. / 2, 90. / 2);
  G4LogicalVolume* part5logical = new G4LogicalVolume(part5solid, nist->FindOrBuildMaterial("SUS304"), "part5logical", 0, 0, 0);
  part5logical->SetVisAttributes(att_iron2);

  for (int i = 0; i < nseg; i++) {
    double phi = (i * M_PI / 36 + 2.796 / 180 * M_PI);
    G4Transform3D tpb = G4RotateZ3D(phi + M_PI - 2.5 / 180 * M_PI) * G4Translate3D(0, -1668. + 12. + 5 + 1, (-1330. + 2380.) / 2);
    pblogical->MakeImprint(top, tpb, i, overlap);
    new G4PVPlacement(G4RotateZ3D(phi)*G4Translate3D(0, 1582 + 45. / 2, 2380. - 90. / 2), part5logical, "part5physical", top, false, 1,
                      overlap);
  }
#endif

#if 1
  if (b_forward_support_legs) {
    G4ThreeVector vc11a[] = {G4ThreeVector(0, 0, -63. / 2), G4ThreeVector(35, 24.5, -63. / 2), G4ThreeVector(35, 158 - 35, -63. / 2), G4ThreeVector(0, 158 - 35, -63. / 2),
                             G4ThreeVector(0, 0, 63. / 2), G4ThreeVector(35, 24.5, 63. / 2), G4ThreeVector(35, 158 - 35, 63. / 2), G4ThreeVector(0, 158 - 35, 63. / 2)
                            };
    G4VSolid* part11asolid = new BelleCrystal("part11asolid", 4, vc11a);
    G4VSolid* part11bsolid = new G4Box("part11bsolid", 63. / 2, 35. / 2, 100. / 2);
    G4LogicalVolume* part11alogical = new G4LogicalVolume(part11asolid, nist->FindOrBuildMaterial("SUS304"), "part11alogical", 0, 0, 0);
    part11alogical->SetVisAttributes(att_iron2);
    G4LogicalVolume* part11blogical = new G4LogicalVolume(part11bsolid, nist->FindOrBuildMaterial("SUS304"), "part11blogical", 0, 0, 0);
    part11blogical->SetVisAttributes(att_iron);

    double Ro = 158.4;
    zr_t vsf[] = {{0, 0}, {35, 24.5}, {35, 158 - 35}, {100, 158 - 35}, {100, Ro}, {0, Ro}};
    for (zr_t* it = vsf; it != vsf + 6; it++) {it->r += 1668 - 158;}
    std::vector<zr_t> csf(vsf, vsf + 6);
    G4VSolid* sf = new BelleLathe("sf", 0, 2 * M_PI, csf);
    G4LogicalVolume* sfl = new G4LogicalVolume(sf, nist->FindOrBuildMaterial("G4_AIR"), "supportfwd", 0, 0, 0);
    sfl->SetVisAttributes(att_air);
    new G4PVPlacement(G4Translate3D(0, 0, 2380), sfl, "supportfwdphysical", top, false, 0, overlap);

    G4VSolid* sfs = new BelleLathe("sfs", -M_PI / 72, 2 * M_PI / 72, csf);
    G4LogicalVolume* sfsl = new G4LogicalVolume(sfs, nist->FindOrBuildMaterial("G4_AIR"), "supportfwdsector", 0, 0, 0);
    sfsl->SetVisAttributes(att_air);

    new G4PVPlacement(G4Translate3D(1668 - 158, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2), part11alogical, "part11aphysical",
                      sfsl, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(1668 - 158, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4Translate3D(0, (158. - 35.) + 35. / 2, 100. / 2),
                      part11blogical, "part11bphysical", sfsl, false, 0, overlap);
    new G4PVReplica("supportfwdrep", sfsl, sfl, kPhi, nseg, 2 * M_PI / 72, (2.796 - 2.5)*M_PI / 180);
  }

  if (b_backward_support_legs) {
    G4VSolid* part12asolid = new G4Box("part12asolid", 90. / 2, (118. - 35.) / 2, 35. / 2);
    G4VSolid* part12bsolid = new G4Box("part12bsolid", 90. / 2, 35. / 2, 120. / 2);
    G4LogicalVolume* part12alogical = new G4LogicalVolume(part12asolid, nist->FindOrBuildMaterial("SUS304"), "part12alogical", 0, 0, 0);
    part12alogical->SetVisAttributes(att_iron2);
    G4LogicalVolume* part12blogical = new G4LogicalVolume(part12bsolid, nist->FindOrBuildMaterial("SUS304"), "part12blogical", 0, 0, 0);
    part12blogical->SetVisAttributes(att_iron);

    double Rb = 118.7;
    zr_t vsb[] = {{0, 0}, {0, Rb}, { -120, Rb}, { -120, 118 - 35}, { -35, 118 - 35}, { -35, 0}};
    for (zr_t* it = vsb; it != vsb + 6; it++) {it->r += 1668 - 118;}
    std::vector<zr_t> csb(vsb, vsb + 6);
    G4VSolid* sb = new BelleLathe("sb", 0, 2 * M_PI, csb);
    G4LogicalVolume* sbl = new G4LogicalVolume(sb, nist->FindOrBuildMaterial("G4_AIR"), "supportbkw", 0, 0, 0);
    sbl->SetVisAttributes(att_air);
    new G4PVPlacement(G4Translate3D(0, 0, -1330), sbl, "supportbkwphysical", top, false, 0, overlap);

    G4VSolid* sbs = new BelleLathe("sbs", -M_PI / 72, 2 * M_PI / 72, csb);
    G4LogicalVolume* sbsl = new G4LogicalVolume(sbs, nist->FindOrBuildMaterial("G4_AIR"), "supportbkwsector", 0, 0, 0);
    sbsl->SetVisAttributes(att_air);

    new G4PVPlacement(G4Translate3D(1668 - 118, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4Translate3D(0, (118. - 35.) / 2, -35. / 2),
                      part12alogical, "part12aphysical", sbsl, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(1668 - 118, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4Translate3D(0, (118. - 35.) + 35. / 2, -120. / 2),
                      part12blogical, "part12bphysical", sbsl, false, 0, overlap);

    new G4PVReplica("supportbkwrep", sbsl, sbl, kPhi, nseg, 2 * M_PI / 72, (2.796 - 2.5)*M_PI / 180);
  }

#endif
  if (b_septum_walls) {
    struct zwall_t { double zlow, zup;};
    zwall_t zs[] = {{ -682.1, -883.9}, { -445.9, -571.7}, { -220.8, -274.2}, {0, 0}, {220.8, 274.2}, {445.9, 571.7}, {682.1, 883.9}, {936.6, 1220.5}, {1217.1, 1591.2}, {1532., 2007.4}};

    G4Transform3D tf0 = G4RotateZ3D(2.5 / 180 * M_PI) * G4Translate3D(0, 1251.6,
                        0) * G4RotateZ3D(1.25 / 180 * M_PI) * G4RotateY3D(-M_PI / 2);
    G4Transform3D tf1 = G4RotateZ3D(-2.5 / 180 * M_PI) * G4Translate3D(0, 1251.6,
                        0) * G4RotateZ3D(1.25 / 180 * M_PI) * G4RotateY3D(-M_PI / 2);
    G4Vector3D n0(sind(90 + 2.5 + 1.25), -cosd(90 + 2.5 + 1.25), 0); G4Point3D r0 = tf0 * G4Point3D(0, 0, -0.26);
    G4Vector3D n1(-sind(90 - 2.5 + 1.25), cosd(90 - 2.5 + 1.25), 0); G4Point3D r1 = tf1 * G4Point3D(0, 0, 0.26);

    G4Vector3D n2(0, 1, 0); G4Point3D r2(0, 1251.6 + 0.2, 0), r3(0, 1251.6 + 310, 0);

    for (unsigned int i = 0; i < sizeof(zs) / sizeof(zwall_t); i++) {
      double th = -atan(404 / (zs[i].zlow - zs[i].zup));
      //    std::cout<<i<<" "<<th*180/M_PI<<std::endl;

      G4Vector3D nf(0, -cos(th), sin(th)); G4Point3D rf(0, 1251.6, zs[i].zlow);

      auto inter = [&nist, &top](const G4Vector3D & n0, const G4Point3D & r0,
                                 const G4Vector3D & n1, const G4Point3D & r1,
      const G4Vector3D & n2, const G4Point3D & r2) -> G4Point3D {
        CLHEP::HepMatrix A(3, 3);
        CLHEP::HepVector B(3);
        A[0][0] = n0.x(), A[0][1] = n0.y(), A[0][2] = n0.z();
        A[1][0] = n1.x(), A[1][1] = n1.y(), A[1][2] = n1.z();
        A[2][0] = n2.x(), A[2][1] = n2.y(), A[2][2] = n2.z();

        B[0] = r0 * n0;
        B[1] = r1 * n1;
        B[2] = r2 * n2;

        CLHEP::HepVector r = A.inverse() * B;
        G4Point3D res(r[0], r[1], r[2]);
        return  res;
      };

      G4Transform3D tfin(G4Translate3D(0, 1251.6, zs[i].zlow)*G4RotateX3D(-th + M_PI / 2)),
                    tfini(G4RotateX3D(th - M_PI / 2)*G4Translate3D(0, -1251.6, -zs[i].zlow));
      G4ThreeVector t0 = tfini * inter(n0, r0, n2, r2, nf, rf);
      G4ThreeVector t1 = tfini * inter(n1, r1, n2, r2, nf, rf);
      G4ThreeVector t2 = tfini * inter(n0, r0, n2, r3, nf, rf);
      G4ThreeVector t3 = tfini * inter(n1, r1, n2, r3, nf, rf);
      //    G4cout<<t0<<" "<<t1<<" "<<t2<<" "<<t3<<" "<<G4RotateX3D(th-M_PI/2)*G4Point3D(0,404,zs[i].zup-zs[i].zlow)<<G4endl;

      G4ThreeVector thfin[8];
      double dz = 0.25;
      thfin[0] = G4ThreeVector(t0.x(), t0.y(), -dz);
      thfin[1] = G4ThreeVector(t1.x(), t1.y(), -dz);
      thfin[3] = G4ThreeVector(t2.x(), t2.y(), -dz);
      thfin[2] = G4ThreeVector(t3.x(), t3.y(), -dz);
      thfin[4] = G4ThreeVector(t0.x(), t0.y(), +dz);
      thfin[5] = G4ThreeVector(t1.x(), t1.y(), +dz);
      thfin[7] = G4ThreeVector(t2.x(), t2.y(), +dz);
      thfin[6] = G4ThreeVector(t3.x(), t3.y(), +dz);

      G4VSolid* thfinsolid = new BelleCrystal("thfinsolid", 4, thfin);
      G4LogicalVolume* thfinlogical = new G4LogicalVolume(thfinsolid, nist->FindOrBuildMaterial("A5052"), "thfinlogical", 0, 0, 0);
      thfinlogical->SetVisAttributes(att_iron2);
      new G4PVPlacement(tfin, thfinlogical, "thfinphysical", sectorlogical, false, 0, overlap);
    }
  }

  if (b_pipe) {
    double zh = (1219. + 2288.) / 2, dz = (-1219. + 2288.) / 2;
    G4VSolid* tubec = new G4Tubs("tubec", 0, 12. / 2, zh, 0, 2 * M_PI);
    G4VSolid* tubew = new G4Tubs("tubew", 0, 10. / 2, zh, 0, 2 * M_PI);
    G4LogicalVolume* tubeclogical = new G4LogicalVolume(tubec, nist->FindOrBuildMaterial("C1220"), "tubec", 0, 0, 0);
    G4LogicalVolume* tubewlogical = new G4LogicalVolume(tubew, nist->FindOrBuildMaterial("G4_WATER"), "tubew", 0, 0, 0);
    new G4PVPlacement(G4Translate3D(0, 0, 0), tubewlogical, "tubewphysical", tubeclogical, false, 1, overlap);
    tubeclogical->SetVisAttributes(att_iron2);
    new G4PVPlacement(G4Translate3D(25, 1640, dz), tubeclogical, "tubecphysical", sectorlogical, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(-35, 1640, dz), tubeclogical, "tubecphysical", sectorlogical, false, 1, overlap);
  }

  double pa_box_height = 2;
  if (b_preamplifier) {
    G4VSolid* sv_preamplifier = new G4Box("sv_preamplifier", 58. / 2, 51. / 2, pa_box_height / 2);
    G4LogicalVolume* lv_preamplifier = new G4LogicalVolume(sv_preamplifier, nist->FindOrBuildMaterial("A5052"), "lv_preamplifier", 0, 0,
                                                           0);
    G4VSolid* sv_diode = new G4Box("sv_diode", 10. / 2, 20. / 2, 0.3 / 2);
    G4LogicalVolume* lv_diode = new G4LogicalVolume(sv_diode, nist->FindOrBuildMaterial("G4_Si"), "lv_diode", 0, 0, 0);
    lv_diode->SetUserLimits(new G4UserLimits(0.01));
    new G4PVPlacement(G4Translate3D(-5, 0, -pa_box_height / 2 + 0.3 / 2), lv_diode, "pv_diode1", lv_preamplifier, false, 1, overlap);
    new G4PVPlacement(G4Translate3D(5, 0, -pa_box_height / 2 + 0.3 / 2), lv_diode, "pv_diode2", lv_preamplifier, false, 2, overlap);

    G4VisAttributes* att_preampfifier = new G4VisAttributes(G4Colour(0.4, 0.4, 0.8));
    lv_preamplifier->SetVisAttributes(att_preampfifier);
    cplacement_t* bp = barrel_placement;
    for (cplacement_t* it = bp; it != bp + 46 * 2; it++) {
      G4Transform3D twc = get_transform(*it);
      int indx = it - bp;
      auto pv = new G4PVPlacement(twc * G4TranslateZ3D(300 / 2 + 0.250 + pa_box_height / 2), lv_preamplifier, "phys_preamplifier",
                                  sectorlogical, false, indx, 0);
      if (overlap)pv->CheckOverlaps(1000);
    }
  }

  if (b_crystal_holder) {
    G4VSolid* holder0 = new G4Box("holder0", 19. / 2, 38. / 2, 6. / 2);
    G4VSolid* holder1 = new G4Box("holder1", 7. / 2, 26. / 2, 7. / 2);
    G4VSolid* holder = new G4SubtractionSolid("holder", holder0, holder1);
    G4LogicalVolume* holderlogical = new G4LogicalVolume(holder, nist->FindOrBuildMaterial("A5052"), "holderlogical", 0, 0, 0);
    G4VisAttributes* att_holder = new G4VisAttributes(G4Colour(0.4, 0.8, 0.8));
    holderlogical->SetVisAttributes(att_holder);
    cplacement_t* bp = barrel_placement;
    for (cplacement_t* it = bp; it != bp + 46 * 2; it++) {
      G4Transform3D twc = get_transform(*it);
      int indx = it - bp;
      auto pv = new G4PVPlacement(twc * G4TranslateZ3D(300 / 2 + 0.250 + pa_box_height + 38. / 2)*G4RotateZ3D(M_PI / 2)*G4RotateX3D(
                                    M_PI / 2), holderlogical, "holderphysical", sectorlogical, false, indx, 0);
      if (overlap)pv->CheckOverlaps(1000);
    }
  }

  if (b_crystals) {
    vector<shape_t*> barcryst = load_shapes("crystal_barrel.txt");
    cplacement_t* bp = barrel_placement;
    for (cplacement_t* it = bp; it != bp + 46 * 2; it++) {
      const cplacement_t& t = *it;
      auto s = find_if(barcryst.begin(), barcryst.end(), [&t](const shape_t* shape) {return shape->nshape == t.nshape;});
      if (s == barcryst.end()) continue;

      G4LogicalVolume* wc = wrapped_crystal(*s, "barrel", 0.17 - 0.03);
      G4Transform3D twc = get_transform(t);
      int indx = it - bp;
      new G4PVPlacement(twc, wc, "wrapped_crystal_physical", sectorlogical, false, indx, overlap);
    }
  }

  if (b_crystal_support) {
    G4LogicalVolume* tl[8];
    for (int i = 0; i < 8; i++) {
      G4VSolid* t = get_crystal_support(i);
      tl[i] = new G4LogicalVolume(t, nist->FindOrBuildMaterial("SUS304"), "crystal_support_logical", 0, 0, 0);
      tl[i]->SetVisAttributes(att_iron2);
    }

    double offset[] = { -945, -607, -305, -45, 45, 305, 607, 909, 1240, 1628, 2040};
    int ns[] = {8, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1};
    double offsetb[] = {263.4 - 210 - 30, 27.4, 10.6, 16.1, 26.2, 21.7, 35, 22 + 240};
    bool flip[] = {1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};
    for (int i = 0; i < 11; i++) {
      G4Transform3D tflip = flip[i] ? G4RotateY3D(-M_PI / 2) : G4RotateY3D(M_PI / 2);
      G4Transform3D tr0(G4RotateZ3D(2.5 / 180 * M_PI)*G4Translate3D(0, 1251.6,
                        0)*G4RotateZ3D(1.25 / 180 * M_PI)*G4Translate3D(25 + 3. / 2 + 0.5 / 2, 404 - 25 - 3.7 + 24.5 - 25.0 * tand(2.5) - 0.5,
                            offset[i])*tflip * G4Translate3D(-offsetb[ns[i] - 1], 0, 0)*G4RotateX3D(M_PI));
      new G4PVPlacement(tr0, tl[ns[i] - 1], "crystal_support_physical", sectorlogical, false, 0, overlap);
      G4Transform3D tr1(G4RotateZ3D(-2.5 / 180 * M_PI)*G4Translate3D(0, 1251.6,
                        0)*G4RotateZ3D(1.25 / 180 * M_PI)*G4Translate3D(-25 - 3. / 2 - 0.5 / 2, 404 - 25 - 3.7 + 24.5 - 25.0 * tand(2.5) - 0.5,
                            offset[i])*tflip * G4Translate3D(-offsetb[ns[i] - 1], 0, 0)*G4RotateX3D(M_PI));
      new G4PVPlacement(tr1, tl[ns[i] - 1], "crystal_support_physical", sectorlogical, false, 1, overlap);
    }
  }

}
