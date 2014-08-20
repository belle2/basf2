# WARNING: if your module is in that example file, deactivate it for that run, otherwise, you wont get useful output for debugging!
valgrind --leak-check=yes --memcheck:leak-check=full --show-reachable=yes --error-limit=no --gen-suppressions=all --log-file=dryRun4suppressionFile.log --suppressions=$ROOTSYS/etc/valgrind-root.supp --suppressions=valgrind-python.supp basf2 part2_spCreator.py
cat ./dryRun4suppressionFile.log | ./parse_valgrind_suppressions.sh > noTBSPcreator.supp

# after that, execute:
# valgrind --leak-check=yes --memcheck:leak-check=full --show-reachable=yes --error-limit=no  --suppressions=noTBSPcreator.supp --suppressions=$ROOTSYS/etc/valgrind-root.supp --suppressions=valgrind-python.supp basf2 part2_spCreator.py