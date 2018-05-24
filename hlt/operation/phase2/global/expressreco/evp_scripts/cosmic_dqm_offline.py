#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_expressreco_processing, finalize_hlt_path

components = ["CDC", "ECL", "TOP", "BKLM", "EKLM", "TRG"]

setup_basf2_and_db(dbfile='/home/usr/erecodaq/database/LocalDB.rel0101/database.txt')
path = create_hlt_path(inputfile='INPUTFILE', dqmfile='dqmfile.root')

add_expressreco_processing(path, run_type="cosmics", components=components, do_reconstruction=True)

finalize_hlt_path(path, outputfile='OUTPUTFILE')
basf2.print_path(path)
basf2.process(path)
