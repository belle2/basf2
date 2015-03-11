name=`echo ${1,,}`
bigname=`echo ${1^^}`
classname=`echo ${name^}`

dir=${BELLE2_LOCAL_DIR}/daq/slc/apps

mkdir $dir/${name}d/
mkdir $dir/${name}d/include
mkdir $dir/${name}d/src
mkdir $dir/${name}d/tools

sed "s/NSMTemplate/$classname/g" $dir/nsmtemplated/include/NSMTemplateCallback.h | \
sed "s/nsmtemplate/${name}/g" > $dir/${name}d/include/${classname}Callback.h 

sed "s/NSMTemplate/$classname/g" $dir/nsmtemplated/src/NSMTemplateCallback.cc | \
sed "s/nsmtemplate/${name}/g"  > $dir/${name}d/src/${classname}Callback.cc 

sed "s/NSMTemplate/$classname/g" $dir/nsmtemplated/tools/nsmtemplated.cc | \
sed "s/nsmtemplate/${name}/g" > $dir/${name}d/tools/${name}d.cc

sed "s/nsmtemplate/${name}/g" $dir/nsmtemplated/Makefile > $dir/${name}d/Makefile
sed "s/nsmtemplate/${name}/g" $dir/nsmtemplated/SConscript > $dir/${name}d/SConscript
sed "s/nsmtemplate/${name}/g" $dir/nsmtemplated/tools/Makefile > $dir/${name}d/tools/Makefile
sed "s/nsmtemplate/${name}/g" $dir/nsmtemplated/tools/SConscript > $dir/${name}d/tools/SConscript

dir=${BELLE2_LOCAL_DIR}/daq/slc/data/config

sed "s/nsmtemplate/${name}/g" $dir/nsmtemplate.conf | sed "s/NSMTEMPLATE/$bigname/g" > $dir/${name}.conf

