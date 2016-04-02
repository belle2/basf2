#!/usr/bin/env bash
# A script to do a fully automatic installation of basf2 and (optionally) externals (using git-svn for basf2)

# Get it via svn export https://belle2.cc.kek.jp/svn/trunk/software/framework/examples/git-svn/install_basf2.sh
# and run bash install_basf2.sh inside your install directory.

#TODO: could start cloning at beginning, while tools are being installed...

#abort on errors
set -e

BASF2_SVN="https://belle2.cc.kek.jp/svn/trunk/software"

echo "This script will install the basf2 software, including tools and externals, into `pwd`,"
echo "creating the directories tools/, externals/ and basf2/ if required."
echo "The basf2/ folder will be a git repository containing the entire history imported from SVN,"
echo "see https://belle2.cc.kek.jp/~twiki/bin/view/Software/GitSvn for details."
echo ""

if ! svn help >& /dev/null
then
  echo "subversion is not installed, please install it and try again."
  exit 1
fi

if ! git help svn >& /dev/null
then
  echo "git-svn is not installed, please install it and try again."
  exit 1
fi

if [ ! -d basf2 ]; then
  echo -n "Looking for current basf2 version..."
  CURRENT_REV=`svn log https://belle2.cc.kek.jp/svn/trunk/software  2> /dev/null | head -2 | tail -1 | awk '{print $1}'`
  echo " $CURRENT_REV"
else
  echo "basf2/ already exists."
fi

echo -n "Looking for current externals version..."
EXTERNALS_VERSION=`svn cat https://belle2.cc.kek.jp/svn/trunk/software/.externals 2> /dev/null`
echo " $EXTERNALS_VERSION."

echo ""
echo "Press return to continue, Ctrl-c to abort."
read

if [ ! -d basf2 ]; then
  echo "================================================================================"
  echo "How much of the SVN history do you want to import? Select $CURRENT_REV for a quick checkout with history starting from the current revision, or 'all' for the entire history which will take a few hours. .git requires about 1GB for storing the entire history, or 200MB for only the latest revision. Press Ctrl-c to abort."
  select SVN_CLONE_FROM in "all" $CURRENT_REV; do
    echo "Selected: $SVN_CLONE_FROM"
    break
  done
fi

echo "================================================================================"
echo "Select whether or not you will want to install the externals locally, or if you"
echo "want to use CVMFS externals."

while true
do
  read -r -p "Install Externals Locally? [Y/n] " input
  case $input in
      [nN][oO]|[nN])     echo  "No"; EXTERNALS_INSTALL=false; break;;
      "")                echo "Yes"; EXTERNALS_INSTALL=true;  break;;  #NOTE: ;& syntax doesn't work on SL5, so use copy&paste here.
      [yY][eE][sS]|[yY]) echo "Yes"; EXTERNALS_INSTALL=true;  break;;
      *)                 echo "Invalid input...";;
  esac
done

if [ "$EXTERNALS_INSTALL" = true ]
then
  if [ ! -d externals/$EXTERNALS_VERSION ]
  then
    if [ "$EXTERNALS_VERSION" == "development" ]; then
      BINARY_VARIANT=""
    else
      BINARY_OPTIONS=""
      ARCH=`uname -m`
      if [ "$ARCH" == "x86_64" ]; then
        BINARY_OPTIONS="sl5 sl6 ubuntu1404"
      elif [ "$ARCH" == "i686" ] || [ "$ARCH" == "i386" ]; then
        BINARY_OPTIONS="sl5_32bit ubuntu1404_32bit"
      fi
      echo "================================================================================"
      echo "Select an appropriate binary variant of the externals for your operating system, or 'source' to compile them from source. For your architecture ($ARCH), the following options are available. Press Ctrl-c to abort."
      select BINARY_VARIANT in "source" $BINARY_OPTIONS; do
        if [ "$BINARY_VARIANT" == "source" ]; then
          BINARY_VARIANT=""
        fi
        break
      done
    fi
  fi
  
  if [ ! -d tools/ ]
  then
    svn co https://belle2.cc.kek.jp/svn/trunk/tools
    ./tools/prepare_belle2.sh || true # optional packages still cause bad return value...
    ./tools/install.sh
  fi
  
  echo "Tools installed."
  source tools/setup_belle2
  
  
  echo "================================================================================"
  if [ ! -d externals/ ]
  then
    echo "Creating externals/ directory..."
    mkdir externals
  fi
  
  if [ ! -d externals/$EXTERNALS_VERSION ]
  then
    echo "Now installing the externals... (logging to get_externals.log)"
    get_externals.sh $EXTERNALS_VERSION $BINARY_VARIANT >& get_externals.log &
    EXTERNALS_PID=$!
  else
    echo "Externals $EXTERNALS_VERSION already installed, nothing to do."
    EXTERNALS_PID=$!
  fi
else  
  echo "Sourcing tools from CVMFS directory. Choose which Scientific Linux directory you want:"
  select CVMFS_SL_ROOT in /cvmfs/belle.cern.ch/sl*;
  do
    echo "You picked $CVMFS_SL_ROOT"
    break
  done
  if [ -e "$CVMFS_SL_ROOT/tools/setup_belle2" ]; then
    source "$CVMFS_SL_ROOT/tools/setup_belle2"
    EXTERNALS_SETUP=true
  else
    echo "Missing $CVMFS_SL_ROOT/tools/setup_belle2 file"
    exit 1
  fi
fi

if [ ! -d basf2 ]; then
  echo "Cloning SVN repository into basf2/... (this will take a while)"
  if [ "$SVN_CLONE_FROM" == "all" ]; then
    GIT_SVN_CLONE_ARGS=""
  else
    GIT_SVN_CLONE_ARGS="-$SVN_CLONE_FROM:HEAD"
  fi

  git svn clone $GIT_SVN_CLONE_ARGS $BASF2_SVN basf2
  # remaining setup
  pushd basf2
  ln -s site_scons/SConstruct .
  echo "head" > .release
  echo "$EXTERNALS_VERSION" > .externals

  echo "Installing pre-commit hook"
  cp framework/examples/git-svn/pre-commit .git/hooks/
  chmod 755 .git/hooks/pre-commit

  echo "Installing hooks to deal with svn:externals (e.g. genfit2 package)"
  cp framework/examples/git-svn/svnexternals.sh .git/hooks/
  cp framework/examples/git-svn/svnexternals.py .git/hooks/
  pushd .git/hooks > /dev/null
  chmod 755 svnexternals.sh
  ln -s svnexternals.sh post-rewrite
  ln -s svnexternals.sh post-checkout
  popd > /dev/null

  # add initial gitignore
  cat > .gitignore <<EOT
# files we don't want to commit
*.py[cod]
*.orig
*.rej
*.bak
*.patch

# usual Belle2 stuff
/.gitignore
/.externals
/.release
/.scon*
/config.log
/setup.sh
/SConstruct
/bin/
/build/
/data/
/include/
/lib/
/modules/
EOT

  popd > /dev/null

  echo "git svn clone finished."
else
  echo "basf2/ already exists, skipping git svn clone."
fi

if [ "$EXTERNALS_INSTALL" = true ]; then
  echo "Waiting for externals install..."
  if wait $EXTERNALS_PID;  then
    echo "Externals install successful!"
    EXTERNALS_SETUP=true
  else
    echo "Something went wrong during installation of externals, please check get_externals.log!"
    exit 1
  fi
fi

if [ "$EXTERNALS_SETUP" = true ]; then
  pushd basf2
  setuprel
  echo "Fetching svn:externals..."
  ./.git/hooks/svnexternals.sh

  popd > /dev/null

  echo "================================================================================"
  echo "Installation finished. You can now set up your basf2 environment using"
  if [ "$EXTERNALS_INSTALL" = true ]; then
    echo "  source tools/setup_belle2"
  else
    echo "  source $CVMFS_SL_ROOT/tools/setup_belle2"
  fi
  echo "  cd basf2"
  echo "  setuprel"
  echo "and compile basf2 using 'scons' (use -j4 to limit scons to only use e.g. 4 cores)"
fi
