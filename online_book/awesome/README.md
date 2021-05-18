This is AWESOME: Awesome Working dEtector SimulatiOn exaMplE
============================================================

This directory contains a complete and working (but kind of useless) subdetector software package including geometry,
materials, sensitive detector, one dataobject and one (dummy) module to serve as an example on how to build your own
subdetector.

The contents are as follows:

* `data/`         : XML files containing the detector description;
* `dataobjects/`  : classes to be stored in the Datastore for usage by other modules or writing to file;
* `examples/`     : an example steering file to run the full simulation of the AWESOME detector;
* `geometry/`     : code responsible for creation of the Geant4 geometry;
* `modules/`      : modules which can be included in the processing like digitization or reconstruction modules;
* `simulation/`   : code responsible for the simulation of the detector response, in particular the sensitive detector
  implementation and probably routines needed for digitization.

The `SConscript` files are used to modify the build process, for example specifying the libraries needed for linking. 
The default built process will:

* install everything in `data/` into the global basf2 `data` directory under a subfolder `awesome`;
* install all header files in the global basf2 `include` directory: the "include" will be removed from the path but the
  directory structure will stay the same. Thus `awesome/geometry/include/AWESOMEGeometryCreator.h` will be installed as
  `awesome/geometry/AWESOMEGeometryCreator.h` and should be included from there;
* compile everything in the `dataobjects/` folder and put it in `libawesome_dataobjects.so` in the global basf2 `lib`
  directory;
* compile everything in the `modules/` folder and put it in `lib<module_name>.so` and `lib<module_name>.b2modmap` in 
  the global `modules` directory;
* compile everything under `tests/` as unittest test suites and create an executable to run all tests (would be called
  `test_awesome` in this case);
* compile everything else and link it as `libawesome.so` in the global basf2 `lib` directory.
