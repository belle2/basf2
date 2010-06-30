#Libraries SConscript
import os

Import('*')

#Loop over the library groups
basf2libs = []
groupdirs = os.listdir(os.getcwd())

for groupdir in groupdirs:
  if groupdir.find('.') > -1 or os.path.isfile(groupdir):
    continue

  #Loop over the libraries in each group
  libdirs = os.listdir(groupdir)
  for libdir in libdirs:
    if libdir.find('.') > -1 or os.path.isfile(libdir):
      continue

    #Set include and lib paths
    localenv = env.Clone()
    localenv.AppendUnique(CPPPATH=[os.path.join(groupdir,libdir)])

    #Search for all headers
    headdir = os.path.join(groupdir,libdir,libdir)
    header = Glob(os.path.join(headdir,'*.h'))

    #search for all source files
    srcdir = os.path.join(groupdir,libdir,'src')
    src = Glob(os.path.join(srcdir,'*.cc')) + \
          Glob(os.path.join(srcdir,'*.cxx')) + \
          Glob(os.path.join(srcdir,'*.cpp'))

    #Build a ROOT dictionary if a file "linkdef.h" exists
    rootdict = []
    for item in header:
      item = str(item)
      if item.lower().find('linkdef.h') >= 0:
        localenv.AppendUnique(CPPPATH=[os.path.join(groupdir,libdir,libdir)])
        rootdict = localenv.RootDict(libdir+'Dict.cxx',header)
        break

    #Build and install the shared library (if source files were found)
    if len(src) > 0:
      localenv.InstallInc(header, subdir=libdir)
      library = localenv.SharedLibrary(libdir, src + rootdict)
      basf2libs.append(libdir)
      localenv.InstallLib(library,env['ARCHSTRING'])
    else:
      print "Library", libdir, "has no source files. The library will not be built !"

#Return the found basf2 libraries
Return('basf2libs')

