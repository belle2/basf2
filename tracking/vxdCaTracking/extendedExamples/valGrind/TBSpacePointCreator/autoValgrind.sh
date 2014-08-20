date
for ((  i = 7 ;  i <= 50;  i++  ))
do
	echo "es folgt Durchgang nummer " $i
	durchGang="spLog"$i"Aug20"
	echo "neue datei = " "$durchGang"
	valgrind --leak-check=yes --memcheck:leak-check=full --show-reachable=yes --error-limit=no  --suppressions=noTBSPcreator.supp --suppressions=$ROOTSYS/etc/valgrind-root.supp --suppressions=valgrind-python.supp --log-file="$durchGang" basf2 part2_spCreator.py
	
	more "$durchGang" | grep "Invalid free"
done
date