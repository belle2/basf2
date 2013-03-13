#!/bin/csh -f
if ( !( -r doth.list ) ) then
echo not found doth.list
exit
endif
foreach i ( `cat doth.list` )
set bn=`basename $i`
set fn=`echo $bn | sed -e 's/\.h/\.fi/g'`
set bns=`echo $bn | sed -e 's/\./\\./g'`
foreach j ( *.h *.F )
egrep $bn $j > /dev/null
if ( $status == 0 ) then
echo $j
mv $j ${j}.bak
sed -e s/${bns}/${fn}/g ${j}.bak > $j
diff ${j} ${j}.bak
endif
end
mv $bn $fn
end
