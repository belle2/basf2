#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::GeoComponent+; // checksum=0x53bd074b, version=1
#pragma link C++ class Belle2::GeoConfiguration+; // checksum=0xc8848c95, version=1
#pragma link C++ class Belle2::GeoMaterial+; // checksum=0x89602aa1, version=1
#pragma link C++ class Belle2::GeoMaterialComponent+; // checksum=0x6de50ca, version=1
#pragma link C++ class Belle2::GeoMaterialProperty+; // checksum=0x70f7e52d, version=1
#pragma link C++ class Belle2::GeoOpticalSurface+; // checksum=0x4d550110, version=1
#pragma link C++ class Belle2::MagneticFieldComponent3D+; // checksum=0xef5bdc9c, version=2

#pragma read                                               \
  sourceClass="Belle2::MagneticFieldComponent3D"           \
  source="std::vector<Belle2::B2Vector3<float>> m_bmap"    \
  version="[1]"                                            \
  targetClass="Belle2::MagneticFieldComponent3D"           \
  target="m_bmap"                                          \
  code="{for (const auto& magField : onfile.m_bmap)        \
           m_bmap.emplace_back(ROOT::Math::XYZVector(magField));                 \
        }"                                                 \

#endif
