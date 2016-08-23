#include <ecl/geometry/GeoECLCreator.h>
#include "ecl/geometry/BelleLathe.h"
#include "ecl/geometry/BelleCrystal.h"
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
#include "ecl/geometry/shapes.h"

using namespace std;

cplacement_t backward_placement[] = {
  {
    1, // wt = 0.2
    2.07624340314, 0.53250095194, 0.970097841763,
    1297.68158264, 1.48196579296, 0.370690032275
  },
  {
    2, // wt = 0.2
    2.0388031483, 0.539184990092, 0.949254151216,
    1297.60347953, 1.4820034702, 0.327184270495
  },
  {
    2, // wt = 0.2
    2.00170177703, 0.546963090069, 0.927194683251,
    1297.16471336, 1.48207083048, 0.283638460357
  },
  {
    2, // wt = 0.2
    1.96426683008, 0.554013021564, 0.905396963472,
    1297.42336615, 1.48203002261, 0.240120668129
  },
  {
    2, // wt = 0.2
    1.92808740455, 0.561458165309, 0.884116204975,
    1297.57393822, 1.4820118483, 0.196489911317
  },
  {
    2, // wt = 0.2
    1.89162457855, 0.570148477622, 0.86222978399,
    1297.29041569, 1.48205444349, 0.152973491676
  },
  {
    2, // wt = 0.2
    1.85426050208, 0.57854339366, 0.840145829996,
    1297.54773743, 1.48201268589, 0.109108352425
  },
  {
    2, // wt = 0.2
    1.8172230627, 0.587838667536, 0.817968398486,
    1297.57684011, 1.48200915339, 0.0654925353315
  },
  {
    1, // wt = 0.2
    1.78190336481, 0.598360188135, 0.796304083127,
    1296.96141234, 1.48208864219, 0.0219438447705
  },
  {
    7, // wt = 0.2
    2.04092536827, 0.516010639874, 0.985330507827,
    1208.01223655, 1.47181321091, 0.370763983369
  },
  {
    4, // wt = 0.2
    2.02786906451, 0.518294792772, 0.948947697086,
    1208.01440706, 1.47181850817, 0.327341502118
  },
  {
    6, // wt = 0.2
    2.01545696506, 0.521314747164, 0.912705437584,
    1207.43426386, 1.47186749459, 0.283832937661
  },
  {
    5, // wt = 0.2
    1.92884442361, 0.537985303183, 0.920308784743,
    1207.5187511, 1.47186002241, 0.240018798679
  },
  {
    4, // wt = 0.2
    1.9157820065, 0.54006902317, 0.88416719802,
    1207.99563343, 1.4718162717, 0.196506781845
  },
  {
    6, // wt = 0.2
    1.90357398516, 0.543150845523, 0.847801307849,
    1207.63115239, 1.47184797339, 0.152996954105
  },
  {
    5, // wt = 0.2
    1.81715098894, 0.563236602221, 0.854784125105,
    1207.56690719, 1.47186641659, 0.10943490483
  },
  {
    4, // wt = 0.2
    1.80475438193, 0.565536500138, 0.820417696369,
    1208.02560476, 1.47179126854, 0.065725635975
  },
  {
    3, // wt = 0.2
    1.79166483366, 0.569924356681, 0.78167605586,
    1207.22690056, 1.47186859695, 0.0220014180666
  },
  {
    11, // wt = 0.2
    2.04590961532, 0.491511579453, 0.965366293189,
    1122.02602864, 1.46083486031, 0.359865433012
  },
  {
    10, // wt = 0.2
    1.98874326947, 0.502100281484, 0.93320138114,
    1121.32466394, 1.46088330158, 0.294648951957
  },
  {
    9, // wt = 0.2
    1.93307144995, 0.512402707477, 0.899592657949,
    1121.37093711, 1.46089630862, 0.229300871644
  },
  {
    10, // wt = 0.2
    1.87580071702, 0.523793154299, 0.868506622332,
    1121.54987877, 1.46086854952, 0.163851206034
  },
  {
    9, // wt = 0.2
    1.82105538864, 0.536096111301, 0.834473933151,
    1121.39642169, 1.46090389001, 0.0984691790646
  },
  {
    8, // wt = 0.2
    1.76296179019, 0.550363782802, 0.803925871352,
    1121.05394856, 1.46090216933, 0.0331226825904
  },
  {
    12, // wt = 0.2
    2.03534907108, 0.468476427882, 0.965485073067,
    1040.24891049, 1.44923573902, 0.359840271896
  },
  {
    13, // wt = 0.2
    1.97829847098, 0.478621962945, 0.933235454176,
    1039.40096205, 1.44925050081, 0.294584325859
  },
  {
    13, // wt = 0.2
    1.92117853582, 0.488764464498, 0.900221351566,
    1039.52241846, 1.44924419708, 0.229265591982
  },
  {
    13, // wt = 0.2
    1.86327775906, 0.499748249972, 0.868177824288,
    1039.69041045, 1.44924504069, 0.163924659018
  },
  {
    13, // wt = 0.2
    1.80745750145, 0.511679565542, 0.836232682576,
    1039.54098128, 1.44924121079, 0.0984612393759
  },
  {
    12, // wt = 0.2
    1.74784316209, 0.525820730466, 0.801821186346,
    1039.20369568, 1.44924171086, 0.032886813335
  },
  {
    14, // wt = 0.2
    2.02475341578, 0.444296710998, 0.965608752967,
    960.928400511, 1.43537049649, 0.359801101134
  },
  {
    15, // wt = 0.2
    1.96750308409, 0.454018424973, 0.933548329689,
    959.968689016, 1.43535192807, 0.294507024929
  },
  {
    15, // wt = 0.2
    1.90937943578, 0.463862198366, 0.901119940128,
    960.090207822, 1.43536669506, 0.229062055208
  },
  {
    15, // wt = 0.2
    1.85059259179, 0.474473526276, 0.868595910171,
    960.649789805, 1.43507846125, 0.163817684937
  },
  {
    15, // wt = 0.2
    1.79374690178, 0.48605568336, 0.836101342749,
    960.147435452, 1.43535011617, 0.098448604715
  },
  {
    14, // wt = 0.2
    1.73339092408, 0.499725983, 0.801834983526,
    959.780405021, 1.43533970375, 0.0329029469111
  },
  {
    16, // wt = 0.2
    2.01452110061, 0.418913579265, 0.967005355586,
    884.551476523, 1.41937840135, 0.35946312596
  },
  {
    17, // wt = 0.2
    1.95618003279, 0.428298973067, 0.9338091196,
    883.318335978, 1.41949888135, 0.294361351339
  },
  {
    17, // wt = 0.2
    1.89732279932, 0.437806802138, 0.901061648348,
    883.413638796, 1.41952838707, 0.229090097858
  },
  {
    17, // wt = 0.2
    1.83851100022, 0.448273104017, 0.868472501779,
    883.303324528, 1.41952879612, 0.163830691907
  },
  {
    17, // wt = 0.2
    1.78015393623, 0.459070891587, 0.836032125435,
    883.509888517, 1.41951260227, 0.0984270663735
  },
  {
    16, // wt = 0.2
    1.71899480449, 0.471573391078, 0.802000512439,
    883.054275578, 1.41939316765, 0.0329559118027
  },
  {
    22, // wt = 0.2
    1.956123303, 0.401157673051, 0.992050347426,
    808.364330934, 1.40014725547, 0.359537379228
  },
  {
    19, // wt = 0.2
    1.94506561576, 0.401376389052, 0.933430220969,
    808.181612113, 1.39994634613, 0.294250385949
  },
  {
    21, // wt = 0.2
    1.93401864684, 0.403035748451, 0.874557788915,
    808.247280078, 1.39993639693, 0.228884599549
  },
  {
    20, // wt = 0.2
    1.77788874349, 0.429024252176, 0.894947108193,
    808.129822936, 1.39993155022, 0.163661668076
  },
  {
    19, // wt = 0.2
    1.7668234845, 0.430694361503, 0.835408215259,
    808.414857654, 1.39994891952, 0.098362103743
  },
  {
    18, // wt = 0.2
    1.75242222281, 0.433081693047, 0.77533433488,
    807.92908996, 1.39979062966, 0.032866899801
  },
  {
    26, // wt = 0.2
    1.97924489767, 0.367499832068, 0.956997795309,
    735.700498225, 1.37838424952, 0.343426403275
  },
  {
    25, // wt = 0.2
    1.88843509076, 0.379892902523, 0.908359185521,
    735.780298535, 1.37839062322, 0.245666538632
  },
  {
    24, // wt = 0.2
    1.79815014604, 0.393862917283, 0.856574415288,
    735.710679893, 1.3784696462, 0.147456801003
  },
  {
    23, // wt = 0.2
    1.70600464684, 0.409511057469, 0.810936372899,
    735.545789332, 1.37828645465, 0.0493933411996
  },
  {
    27, // wt = 0.2
    1.97003752387, 0.338843506641, 0.957773913679,
    668.013458503, 1.35627029822, 0.343248966755
  },
  {
    28, // wt = 0.2
    1.87786837706, 0.350617297963, 0.908751521816,
    668.087075745, 1.35632685565, 0.245534537277
  },
  {
    28, // wt = 0.2
    1.78695709794, 0.36362787472, 0.860241302807,
    668.070392501, 1.35632038491, 0.147512446762
  },
  {
    27, // wt = 0.2
    1.69313753906, 0.378372377394, 0.809800132092,
    667.924565624, 1.35618177929, 0.0493783992994
  },
  {
    29, // wt = 0.2
    1.96113361723, 0.309340168716, 0.958804445559,
    602.688776512, 1.32795592691, 0.343079704494
  },
  {
    30, // wt = 0.2
    1.868009424, 0.320669438674, 0.9088063096,
    602.788132557, 1.32813011691, 0.24536282937
  },
  {
    30, // wt = 0.2
    1.77610594953, 0.332740916481, 0.860292519858,
    602.771986225, 1.32812222972, 0.147431606954
  },
  {
    29, // wt = 0.2
    1.68112166007, 0.346392476927, 0.809131861128,
    602.758941031, 1.32781304917, 0.0493839564419
  },
// zshift = 1 d[33]= 0.31, d[36]= 0.175, d[59]= 0.117, d[0]= 0.052, d[56]= 0.0149, d[46]= 0.00887, d[13]= 0.00878, d[16]= 0.00459, d[17]= 0.00219, d[32]= 0.00123, d[55]= 0.00114, d[23]= 0.00106, d[53]= 0.00105, d[30]= 0.00102, d[3]= 0.00101, d[43]= 0.001, d[24]= 0.001, d[41]= 0.001, d[39]= 0.001, d[49]= 0.001, d[37]= 0.001, d[35]= 0.001, d[45]= 0.001, d[26]= 0.001, d[48]= 0.001, d[22]= 0.001, d[34]= 0.001, d[27]= 0.000994, d[29]= 0.000992, d[42]= 0.000989, d[4]= 0.00098, d[58]= 0.000973, d[15]= 0.00097, d[28]= 0.000965, d[44]= 0.000958, d[6]= 0.000955, d[54]= 0.000952, d[31]= 0.00095, d[57]= 0.000943, d[50]= 0.000927, d[1]= 0.000919, d[38]= 0.000917, d[12]= 0.000892, d[21]= 0.000887, d[25]= 0.00087, d[2]= 0.00086, d[19]= 0.000856, d[20]= 0.000852, d[11]= 0.000845, d[9]= 0.00078, d[40]= 0.00075, d[14]= 0.000708, d[18]= 0.000701, d[47]= 0.000671, d[7]= 0.000566, d[10]= 0.000543, d[5]= 0.000367, d[8]= 0.000193, d[52]= 0.000192, d[51]= -7.66e-05,
};

void Belle2::ECL::GeoECLCreator::backward(const GearDir& content, G4LogicalVolume& _top)
{
  G4LogicalVolume* top = &_top;
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  G4VisAttributes* att_iron = new G4VisAttributes(G4Colour(1., 0.1, 0.1));
  G4VisAttributes* att_iron2 = new G4VisAttributes(G4Colour(1., 0.5, 0.5));
  G4VisAttributes* att_alum = new G4VisAttributes(G4Colour(0.25, 0.25, 1.0, 0.5));
  G4VisAttributes* att_alum2 = new G4VisAttributes(G4Colour(0.5, 0.5, 1.0));
  G4VisAttributes* att_silv = new G4VisAttributes(G4Colour(0.9, 0., 0.9));
  G4VisAttributes* att_air = new G4VisAttributes(G4Colour(0., 1., 1.)); att_air->SetVisibility(false);

  bool sec = 0;
  double phi0 = 0, dphi = (sec) ? M_PI / 16 : 2 * M_PI;

  bool b_inner_support_ring = 1;
  bool b_outer_support_ring = 1;
  bool b_support_wall = 1;
  bool b_ribs = 1;
  bool b_septum_wall = 1;
  bool b_crystals = 1;
  bool b_preamplifier = 1;
  bool b_support_leg = 1;
  int overlap = 1;

  if (b_inner_support_ring) {
    zr_t vc1[] = {{0., 452.3 + 3}, {0., 452.3}, {3., 474.9 - 20 / cosd(27.81)}, {434., 702.27 - 20 / cosd(27.81)}, {434., 702.27}, {3., 474.9}, {3., 452.3 + 3}};
    std::vector<zr_t> contour1(vc1, vc1 + sizeof(vc1) / sizeof(zr_t));
    G4VSolid* part1solid = new BelleLathe("part1solid", phi0, dphi, contour1);
    G4LogicalVolume* part1logical = new G4LogicalVolume(part1solid, nist->FindOrBuildMaterial("SUS304"), "part1logical", 0, 0, 0);
    part1logical->SetVisAttributes(att_iron);
    new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), part1logical, "part1physical", top, false, 0, overlap);
  }

  if (b_support_wall) {
    // solveing equation to get L : 3+L*cosd(52.90)+1.6*cosd(52.90+90) = 435 - 202.67
    double L = (435 - 202.67 - 3 - 1.6 * cosd(52.90 + 90)) / cosd(52.90);
    zr_t vc23[] = {{0, 452.3 + 3}, {3, 452.3 + 3}, {3, 1190.2}, {3 + L * cosd(52.90), 1190.2 + L * sind(52.90)},
      {3 + L * cosd(52.90) + 1.6 * cosd(52.90 + 90), 1190.2 + L * sind(52.90) + 1.6 * sind(52.90 + 90)}, {3 + 1.6 * cosd(52.90 + 90), 1190.2 + 1.6 * sind(52.90 + 90)}, {0, 1190.2}
    };
    std::vector<zr_t> contour23(vc23, vc23 + sizeof(vc23) / sizeof(zr_t));
    G4VSolid* part23solid = new BelleLathe("part23solid", phi0, dphi, contour23);
    G4LogicalVolume* part23logical = new G4LogicalVolume(part23solid, nist->FindOrBuildMaterial("A5052"), "part23logical", 0, 0, 0);
    part23logical->SetVisAttributes(att_alum);
    new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), part23logical, "part23physical", top, false, 0, overlap);
  }

  if (b_outer_support_ring) {
    zr_t vc4[] = {{434 - 214.8, 1496 - 20}, {434, 1496 - 20}, {434, 1496 - 5}, {434 + 5, 1496 - 5}, {434 + 5, 1496}, {434 - 199.66, 1496}};
    std::vector<zr_t> contour4(vc4, vc4 + sizeof(vc4) / sizeof(zr_t));
    G4VSolid* part4solid = new BelleLathe("part4solid", phi0, dphi, contour4);
    G4LogicalVolume* part4logical = new G4LogicalVolume(part4solid, nist->FindOrBuildMaterial("SUS304"), "part4logical", 0, 0, 0);
    part4logical->SetVisAttributes(att_iron);
    new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), part4logical, "part4physical", top, false, 0, overlap);
  }

  zr_t vin[] = {{3., 474.9}, {434., 702.27}, {434, 1496 - 20}, {434 - 214.8, 1496 - 20}, {3, 1190.2}};
  std::vector<zr_t> cin(vin, vin + sizeof(vin) / sizeof(zr_t));
  G4VSolid* innervolume_solid = new BelleLathe("innervolume_solid", 0, 2 * M_PI, cin);
  G4LogicalVolume* innervolume_logical = new G4LogicalVolume(innervolume_solid, nist->FindOrBuildMaterial("G4_AIR"),
                                                             "innervolume_logical", 0, 0, 0);
  innervolume_logical->SetVisAttributes(att_air);
  new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), innervolume_logical, "ECLBackwardPhysical", top, false, 0,
                    overlap);

  G4VSolid* innervolumesector_solid = new BelleLathe("innervolumesector_solid", -M_PI / 8, M_PI / 4, cin);
  G4LogicalVolume* innervolumesector_logical = new G4LogicalVolume(innervolumesector_solid, nist->FindOrBuildMaterial("G4_AIR"),
      "innervolumesector_logical", 0, 0, 0);
  innervolumesector_logical->SetVisAttributes(att_air);
  new G4PVReplica("ECLBackwardSectorPhysical", innervolumesector_logical, innervolume_logical, kPhi, 8, M_PI / 4, 0);

  if (b_ribs) {
    double H = 60, W = 20;
    double X0 = 702.27, X1 = 1496 - 20;
    G4TwoVector r0o(X1, 0), r1o(X1 * sqrt(1 - pow(W / X1, 2)), W);
    double beta = asin(W / X0);
    G4TwoVector r0i(X0 / cos(beta / 2), 0), r1i(X0 * cos(beta / 2) - tan(beta / 2) * (W - X0 * sin(beta / 2)), W);
    double dxymzp = (r0o - r0i).x(), dxypzp = (r1o - r1i).x();
    double theta = atan(tand(27.81) / 2);
    double dxymzm = dxymzp + tand(27.81) * H, dxypzm = dxypzp + tand(27.81) * H;

    G4TwoVector m0 = (r0i + r0o) * 0.5, m1 = (r1i + r1o) * 0.5, dm = m1 - m0;
    double alpha = atan(dm.x() / dm.y());

    G4VSolid* solid6_p1 = new G4Trap("solid6_p1", H / 2, theta, 0, W / 2, dxymzm / 2, dxypzm / 2, alpha, W / 2, dxymzp / 2, dxypzp / 2,
                                     alpha);
    G4LogicalVolume* lsolid6_p1 = new G4LogicalVolume(solid6_p1, nist->FindOrBuildMaterial("SUS304"), "lsolid6", 0, 0, 0);
    G4VisAttributes* asolid6 = new G4VisAttributes(G4Colour(1., 0.3, 0.2));
    lsolid6_p1->SetVisAttributes(asolid6);
    G4Transform3D tsolid6_p1(G4Translate3D(X0 * cos(beta / 2) + (dxymzp / 2 + dxypzp / 2) / 2 - tan(theta)*H / 2, W / 2, 434 - H / 2));
    new G4PVPlacement(G4RotateZ3D(-M_PI / 8)*tsolid6_p1, lsolid6_p1, "psolid6_p1", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(0)*tsolid6_p1, lsolid6_p1, "psolid6_p2", innervolumesector_logical, false, 0, overlap);

    H = 40;
    dxymzm = dxymzp + tand(27.81) * H, dxypzm = dxypzp + tand(27.81) * H;
    G4VSolid* solid6_p2 = new G4Trap("solid6_p2", H / 2, theta, 0, W / 2, dxypzm / 2, dxymzm / 2, -alpha, W / 2, dxypzp / 2, dxymzp / 2,
                                     -alpha);
    G4LogicalVolume* lsolid6_p2 = new G4LogicalVolume(solid6_p2, nist->FindOrBuildMaterial("SUS304"), "lsolid6", 0, 0, 0);
    lsolid6_p2->SetVisAttributes(asolid6);
    G4Transform3D tsolid6_p2(G4Translate3D(X0 * cos(beta / 2) + (dxymzp / 2 + dxypzp / 2) / 2 - tan(theta)*H / 2, -W / 2, 434 - H / 2));
    new G4PVPlacement(G4RotateZ3D(0)*tsolid6_p2, lsolid6_p2, "psolid6_p3", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*tsolid6_p2, lsolid6_p2, "psolid6_p4", innervolumesector_logical, false, 0, overlap);

    G4VSolid* solid7_p8 = new G4Box("solid7_p8", 171. / 2, (140. - 40) / 2 / 2, 40. / 2);
    G4LogicalVolume* lsolid7 = new G4LogicalVolume(solid7_p8, nist->FindOrBuildMaterial("SUS304"), "lsolid7", 0, 0, 0);
    G4VisAttributes* asolid7 = new G4VisAttributes(G4Colour(1., 0.3, 0.2));
    lsolid7->SetVisAttributes(asolid7);
    double dx = sqrt(X1 * X1 - 70 * 70) - 171. / 2;
    G4Transform3D tsolid7_p1(G4Translate3D(dx, -20 - 25, 434 - 40. / 2));
    new G4PVPlacement(tsolid7_p1, lsolid7, "psolid7_p1", innervolumesector_logical, false, 0, overlap);
    G4Transform3D tsolid7_p2(G4Translate3D(dx, 20 + 25, 434 - 40. / 2));
    new G4PVPlacement(tsolid7_p2, lsolid7, "psolid7_p2", innervolumesector_logical, false, 0, overlap);

    double L = X1 - (X0 - tand(27.81) * 40) - 10;
    G4VSolid* solid13 = new G4Box("solid13", L / 2, 5. / 2, 18. / 2);
    G4LogicalVolume* lsolid13 = new G4LogicalVolume(solid13, nist->FindOrBuildMaterial("SUS304"), "lsolid13", 0, 0, 0);
    G4VisAttributes* asolid13 = new G4VisAttributes(G4Colour(1., 0.5, 0.5));
    lsolid13->SetVisAttributes(asolid13);
    G4Transform3D tsolid13(G4TranslateZ3D(434 - 60 + 18. / 2)*G4TranslateY3D(-5. / 2 - 0.5 / 2)*G4TranslateX3D(X0 - tand(
                             27.81) * 40 + L / 2 + 5));
    new G4PVPlacement(tsolid13, lsolid13, "psolid13_p1", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*tsolid13, lsolid13, "psolid13_p2", innervolumesector_logical, false, 0, overlap);
  }


  double zsep = 135;
  if (b_septum_wall) {
    double d = 5;
    Point_t vin[] = {{434. - zsep, 702.27 - tand(27.81)* zsep}, {434. - 60, 702.27 - tand(27.81) * 60}, {434. - 60, 1496 - 20 - d}, {434. - zsep, 1496 - 20 - d}};
    const int n = sizeof(vin) / sizeof(Point_t);
    Point_t c = centerofgravity(vin, vin + n);
    G4ThreeVector cin[n * 2];
    for (int i = 0; i < n; i++) cin[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2);
    for (int i = 0; i < n; i++) cin[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2);

    G4VSolid* septumwall_solid = new BelleCrystal("septumwall_solid", n, cin);

    G4LogicalVolume* septumwall_logical = new G4LogicalVolume(septumwall_solid, nist->FindOrBuildMaterial("A5052"),
                                                              "septumwall_logical", 0, 0, 0);
    septumwall_logical->SetVisAttributes(att_alum2);
    new G4PVPlacement(G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0), septumwall_logical,
                      "septumwall_physical", innervolumesector_logical, false, 0, overlap);

    for (int i = 0; i < n; i++) cin[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2 / 2);
    for (int i = 0; i < n; i++) cin[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2 / 2);

    G4VSolid* septumwall2_solid = new BelleCrystal("septumwall2_solid", n, cin);

    G4LogicalVolume* septumwall2_logical = new G4LogicalVolume(septumwall2_solid, nist->FindOrBuildMaterial("A5052"),
                                                               "septumwall2_logical", 0, 0, 0);
    septumwall2_logical->SetVisAttributes(att_alum2);
    new G4PVPlacement(G4RotateZ3D(-M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0.5 / 2 / 2),
                      septumwall2_logical, "septumwall2_physical", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, -0.5 / 2 / 2),
                      septumwall2_logical, "septumwall2_physical", innervolumesector_logical, false, 1, overlap);
  }

  zr_t vcr[] = {{3., 474.9}, {434. - zsep, 702.27 - tand(27.81)* zsep}, {434 - zsep, 1496 - 20}, {434 - 214.8, 1496 - 20}, {3, 1190.2}};
  std::vector<zr_t> ccr(vcr, vcr + sizeof(vcr) / sizeof(zr_t));
  G4VSolid* crystalvolume_solid = new BelleLathe("crystalvolume_solid", 0, M_PI / 8, ccr);
  G4LogicalVolume* crystalvolume_logical = new G4LogicalVolume(crystalvolume_solid, nist->FindOrBuildMaterial("G4_AIR"),
      "crystalvolume_logical", 0, 0, 0);
  crystalvolume_logical->SetVisAttributes(att_air);
  new G4PVPlacement(G4RotateZ3D(-M_PI / 8), crystalvolume_logical, "ECLBackwardCrystalSectorPhysical_0", innervolumesector_logical,
                    false, 0, overlap);
  new G4PVPlacement(G4RotateZ3D(0), crystalvolume_logical, "ECLBackwardCrystalSectorPhysical_1", innervolumesector_logical, false, 1,
                    overlap);

  if (b_septum_wall) {
    double d = 5;
    Point_t vin[] = {{3., 474.9}, {434. - zsep, 702.27 - tand(27.81)* zsep}, {434 - zsep, 1496 - 20 - d}, {434 - 214.8 - d / tand(52.90), 1496 - 20 - d}, {3, 1190.2}};
    const int n = sizeof(vin) / sizeof(Point_t);
    Point_t c = centerofgravity(vin, vin + n);
    G4ThreeVector cin[n * 2];

    for (int i = 0; i < n; i++) cin[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2 / 2);
    for (int i = 0; i < n; i++) cin[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2 / 2);

    G4VSolid* septumwall3_solid = new BelleCrystal("septumwall3_solid", n, cin);

    G4LogicalVolume* septumwall3_logical = new G4LogicalVolume(septumwall3_solid, nist->FindOrBuildMaterial("A5052"),
                                                               "septumwall3_logical", 0, 0, 0);
    septumwall3_logical->SetVisAttributes(att_alum2);
    new G4PVPlacement(G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0.5 / 2 / 2), septumwall3_logical,
                      "septumwall3_physical_0", crystalvolume_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, -0.5 / 2 / 2),
                      septumwall3_logical, "septumwall3_physical_1", crystalvolume_logical, false, 1, overlap);
  }

  if (b_crystals) {
    vector<shape_t*> cryst = load_shapes("/ecl/data/crystal_shape_backward.dat");
    vector<G4LogicalVolume*> wrapped_crystals;
    for (auto it = cryst.begin(); it != cryst.end(); it++) {
      shape_t* s = *it;
      wrapped_crystals.push_back(wrapped_crystal(s, "backward", 0.20 - 0.02));
    }
    cplacement_t* bp = backward_placement;
    for (cplacement_t* it = bp; it != bp + 60; it++) {
      const cplacement_t& t = *it;
      auto s = find_if(cryst.begin(), cryst.end(), [&t](const shape_t* shape) {return shape->nshape == t.nshape;});
      if (s == cryst.end()) continue;

      G4Transform3D twc = G4Translate3D(0, 0, 3) * get_transform(t);
      int indx = it - bp;
      new G4PVPlacement(twc, wrapped_crystals[s - cryst.begin()], suf("ECLBackwardWrappedCrystal_Physical", indx), crystalvolume_logical,
                        false, (1152 + 6624) / 16 + indx, overlap);
    }
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
    cplacement_t* bp = backward_placement;
    for (cplacement_t* it = bp; it != bp + 60; it++) {
      G4Transform3D twc = G4Translate3D(0, 0, 3) * get_transform(*it);
      int indx = it - bp;
      auto pv = new G4PVPlacement(twc * G4TranslateZ3D(300 / 2 + 0.20 + pa_box_height / 2)*G4RotateZ3D(-M_PI / 2), lv_preamplifier,
                                  "phys_preamplifier", crystalvolume_logical, false, indx, 0);
      if (overlap)pv->CheckOverlaps(1000);
    }
  }

  if (b_support_leg) {
    G4VisAttributes* batt = att_iron;

    G4VSolid* s1 = new G4Box("leg_p1", 130. / 2, 185. / 2, (40. - 5) / 2);
    G4LogicalVolume* l1 = new G4LogicalVolume(s1, nist->FindOrBuildMaterial("SUS304"), "l1", 0, 0, 0);
    G4Transform3D t1 = G4Translate3D(0, 185. / 2, (40. - 5) / 2);
    l1->SetVisAttributes(batt);

    Point_t v3[] = {{ -212. / 2, -135. / 2}, {212. / 2 - 30, -135. / 2}, {212. / 2, -135. / 2 + 30}, {212. / 2, 135. / 2} , { -212. / 2, 135. / 2}};
    const int n3 = sizeof(v3) / sizeof(Point_t);
    G4ThreeVector c3[n3 * 2];

    for (int i = 0; i < n3; i++) c3[i + 0] = G4ThreeVector(v3[i].x, v3[i].y, -60. / 2);
    for (int i = 0; i < n3; i++) c3[i + n3] = G4ThreeVector(v3[i].x, v3[i].y, 60. / 2);

    G4VSolid* s3 = new BelleCrystal("leg_p3", n3, c3);
    G4LogicalVolume* l3 = new G4LogicalVolume(s3, nist->FindOrBuildMaterial("SUS304"), "l3", 0, 0, 0);
    G4Transform3D t3 = G4Translate3D(0, 135. / 2 + 35, 40. - 5. + 212. / 2) * G4RotateY3D(-M_PI / 2);
    l3->SetVisAttributes(batt);

    G4VSolid* s6 = new G4Box("leg_p6", 140. / 2, 189. / 2, 160. / 2);
    G4LogicalVolume* l6 = new G4LogicalVolume(s6, nist->FindOrBuildMaterial("G4_AIR"), "l6", 0, 0, 0);
    G4Transform3D t6 = G4Translate3D(0, 170. + 189. / 2, 57. + 35. + 160. / 2);
    l6->SetVisAttributes(att_air);

    G4VSolid* s6a = new G4Box("leg_p6a", 140. / 2, (189. - 45.) / 2, 160. / 2);
    G4LogicalVolume* l6a = new G4LogicalVolume(s6a, nist->FindOrBuildMaterial("SUS304"), "l6a", 0, 0, 0);
    l6a->SetVisAttributes(batt);
    new G4PVPlacement(G4TranslateY3D(-45. / 2), l6a, "l6a_physical", l6, false, 0, overlap);

    G4VSolid* s6b = new G4Box("leg_p6b", 60. / 2, 45. / 2, 160. / 2);
    G4LogicalVolume* l6b = new G4LogicalVolume(s6b, nist->FindOrBuildMaterial("SUS304"), "l6b", 0, 0, 0);
    l6b->SetVisAttributes(batt);
    double dy = 189. / 2 - 45 + 45. / 2;
    new G4PVPlacement(G4TranslateY3D(dy), l6b, "l6b_physical", l6, false, 0, overlap);

    G4VSolid* s6c = new G4Box("leg_p6c", 40. / 2, 45. / 2, 22.5 / 2);
    G4LogicalVolume* l6c = new G4LogicalVolume(s6c, nist->FindOrBuildMaterial("SUS304"), "l6c", 0, 0, 0);
    l6c->SetVisAttributes(batt);
    new G4PVPlacement(G4Translate3D(30 + 20, dy, 20 + 22.5 / 2), l6c, "l6c_physical", l6, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(30 + 20, dy, -20 - 22.5 / 2), l6c, "l6c_physical", l6, false, 1, overlap);
    new G4PVPlacement(G4Translate3D(-30 - 20, dy, 20 + 22.5 / 2), l6c, "l6c_physical", l6, false, 2, overlap);
    new G4PVPlacement(G4Translate3D(-30 - 20, dy, -20 - 22.5 / 2), l6c, "l6c_physical", l6, false, 3, overlap);

    G4AssemblyVolume* support_leg = new G4AssemblyVolume();

    support_leg->AddPlacedVolume(l1, t1);
    //    support_leg->AddPlacedVolume(l2,t2);
    support_leg->AddPlacedVolume(l3, t3);
    //    support_leg->AddPlacedVolume(l4,t4);
    //    support_leg->AddPlacedVolume(l5,t5);
    support_leg->AddPlacedVolume(l6, t6);

    G4VSolid* s_all = new G4Box("leg_all", 140. / 2, 359. / 2, (257. - 5.) / 2);
    G4LogicalVolume* l_all = new G4LogicalVolume(s_all, nist->FindOrBuildMaterial("G4_AIR"), "l_all", 0, 0, 0);
    l_all->SetVisAttributes(att_air);
    G4Transform3D tp = G4Translate3D(0, -359. / 2, -(257. - 5.) / 2);
    support_leg->MakeImprint(l_all, tp, 0, overlap);


    for (int i = 0; i < 8; i++)
      new G4PVPlacement(G4RotateX3D(M_PI)*G4RotateZ3D(-M_PI / 2 + M_PI / 8 + i * M_PI / 4)*G4Translate3D(0, 1496 - 185 + 359. / 2,
                        1020 + 434 + 5 + (257. - 5.) / 2), l_all, "lall_physical", top, false, i, overlap);


    G4VSolid* s1a = new G4Box("leg_p1a", 130. / 2, 178. / 2, 5. / 2);
    G4LogicalVolume* l1a = new G4LogicalVolume(s1a, nist->FindOrBuildMaterial("SUS304"), "l1a", 0, 0, 0);
    l1a->SetVisAttributes(batt);
    for (int i = 0; i < 8; i++)
      new G4PVPlacement(G4RotateX3D(M_PI)*G4RotateZ3D(-M_PI / 2 + M_PI / 8 + i * M_PI / 4)*G4Translate3D(0, 1496 - 185 + 178. / 2,
                        1020 + 434 + 5 - 5. / 2), l1a, "l1a_physical", top, false, i, overlap);

  }

}
