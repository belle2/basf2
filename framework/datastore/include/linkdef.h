#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

//purely for PyROOT
#pragma link C++ class Belle2::RelationsObject;
#pragma link C++ class Belle2::RelationEntry;
#pragma link C++ class std::vector<Belle2::RelationEntry>;
#pragma link C++ class Belle2::RelationVector<TObject>;
#pragma link C++ class Belle2::DataStore::StoreEntry;

#endif
