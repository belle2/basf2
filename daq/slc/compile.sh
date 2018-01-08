CUR=${PWD}
SRC=${HOME}/externals/src
INS=${HOME}/slc/externals
mkdir -p ${SRC}

ncore=`cat /proc/cpuinfo | grep processor|tail -n1|awk '{print $3}'`

#mkdir -p ${INS}/python
#cd ${SRC}
#tar zxvf ${SRC}/python_src.tgz
#cd ${SRC}/Python-3.5.2
#./configure --prefix=${INS}/python --enable-shared LDFLAGS=-Wl,-rpath,${INS}/python/lib
#make
#make install
#cd ${INS}/python/bin
#ln -s python3 python
#ln -s pydoc3 pydoc
#cd ${INS}
#
#export PATH=${INS}/python/bin:${PATH}
#export PYTHONPATH=${INS}/python/lib:${PYTHONPATH}
#export C_INCLUDE_PATH=${INS}/python/include/python3.5m:${C_INCLUDE_PATH}
#export CPLUS_INCLUDE_PATH=${INS}/python/include/python3.5m:${CPLUS_INCLUDE_PATH}

#mkdir -p ${INS}/boost
#cd ${SRC}
#tar zxvf ${SRC}/boost_src.tgz
#cd boost_1_62_0
#./bootstrap.sh
#./b2 install -j${ncore} --prefix=${INS}/boost
##./b2 install --prefix=${INS}/boost

mkdir -p ${INS}/pgsql
cd ${SRC}
tar zxvf ${SRC}/pgsql_src.tgz
cd ${SRC}/postgresql-9.3.1
./configure --prefix=${INS}/pgsql
make
make install
cd ${INS}

cd ${CUR}

