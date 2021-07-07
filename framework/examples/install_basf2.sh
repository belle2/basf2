#!/usr/bin/env bash
# A script to do a fully automatic installation of basf2 and (optionally) externals

# run bash install_basf2.sh inside your install directory.

#abort on errors
set -e
CVMFS_DIR="/cvmfs/belle.cern.ch"

echo "This script will install the basf2 software, including tools and externals, into `pwd`,"
echo "creating the directories tools/, externals/ and basf2/ if required."
echo "If these subdirectories exists, the corresponding installation steps will be skipped."
echo ""
echo "This installation procedure assumes SSH access to the git repository, so you should"
echo "upload a public key to stash. See the Git Stash introduction in confluence for instructions:"
echo "  https://confluence.desy.de/pages/viewpage.action?pageId=35819226"
echo ""

if ! git help >& /dev/null
then
  echo "git is not installed, please install it and try again."
  exit 1
fi


echo ""
echo "Press return to continue, Ctrl-c to abort."
read


if [ ! -d tools/ ]
then
  echo ""
  echo "Installing tools..."
  git clone ssh://git@stash.desy.de:7999/b2/tools.git
  ./tools/prepare_belle2.sh || true # optional packages still cause bad return value...
fi

echo "Tools installed."
#setting up tools somehow triggers an exit?
set +e
source tools/setup_belle2
set -e
echo "================================================================================"

echo -n "Looking for current externals version..."
EXTERNALS_VERSION=`git archive --remote=$BELLE2_SOFTWARE_REPOSITORY master .externals | tar xfO -`
echo " $EXTERNALS_VERSION."

if [ -d $CVMFS_DIR ]
then
  echo "Select whether or not you will want to install the externals locally, or if you"
  echo "want to use CVMFS externals (available at KEKCC and some other sites)."

  while true
  do
    read -r -p "Install externals locally? [Y/n] " input
    case $input in
        [nN][oO]|[nN])     echo  "No"; EXTERNALS_INSTALL=false; break;;
        "")                echo "Yes"; EXTERNALS_INSTALL=true;  break;;  #NOTE: ;& syntax doesn't work on SL5, so use copy&paste here.
        [yY][eE][sS]|[yY]) echo "Yes"; EXTERNALS_INSTALL=true;  break;;
        *)                 echo "Invalid input...";;
    esac
  done
else
  echo "No $CVMFS_DIR directory detected, selecting local install."
  EXTERNALS_INSTALL=true
fi

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
        BINARY_OPTIONS="sl6 el7 ubuntu1404 ubuntu1604"
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
  
  echo "================================================================================"
  if [ ! -d externals/ ]
  then
    echo "Creating externals/ directory..."
    mkdir externals
  fi
  
  if [ ! -d externals/$EXTERNALS_VERSION ]
  then
    echo "Now installing the externals in the background... (logging to get_externals.log)"
    get_externals.sh $EXTERNALS_VERSION $BINARY_VARIANT >& get_externals.log &
    EXTERNALS_PID=$!
  else
    echo "Externals $EXTERNALS_VERSION already installed, nothing to do."
    EXTERNALS_PID=$!
  fi
else  
  echo "Sourcing tools from CVMFS directory. Choose which Scientific Linux directory you want:"
  select CVMFS_SL_ROOT in $CVMFS_DIR/sl*;
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
  echo "Cloning repository into basf2/... (this may take a while)"

  newrel basf2
  echo "git clone finished."
else
  echo "basf2/ already exists, skipping git clone."
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
