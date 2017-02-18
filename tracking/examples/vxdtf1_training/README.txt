This following lines describe very basically the training of the sectormap. 


The scripts 
generateAndTestSecMapWithSPs.py
onlyExportMapWithSPs.py
are modified copies from tracking/vxdCaTracking/extendedExamples/secMapGen/svdL3displacement1mmNov2015/ 
which are the scripts (according to Jakob) used for the last training of the sectormap (the one 
currently used in production as of February 2017). 



Step 1: Trainings data generation
=================================
The script generateAndTestSecMapWithSPs.py generates the data used to train the sectormap. It has two 
input parameters, the random seed and the number of events to generate (in that order). To execute it do

> basf2 generateAndTestSecMapWithSPs.py <random seed> <number of events>

both input parameter are optional (default values are taken if not specified). The output is a root file
that contains the raw data for training the sectormap. The script can be executed several times with 
different random seeds which will add more events to the already existing root file. 



Step 2: creating the SectorMap
==============================
After the trainings data has been created you can run the sectormap generation:

> basf2 onlyExportMapWithSPs.py

This will create the sectormaps in xml - format. 


Step 3: setup
=============
After the sectormaps have been created you should move the sectormaps (xml files) to the folder where you 
want to store them. The default folder is "tracking/data/" (the following step will assume that your sectormaps 
are in that folder). Afterwards you have to register the sectormaps for the usage by modifying the following file:

> tracking/data/VXDTFindex.xml

Add the newly generated sectormaps to the list (see already existing entries) with its paths. Now 
they are ready to use. 

To use the sectormaps in the VXDTF you have to give their name to the VXDTF by setting the parameter
"sectorSetup". See for example line 294 and lines 98 - ... in generateAndTestSecMapWithSPs.py. For each 
given SectorMap one pass of VXDTF will be performed. 





Step 4: Testing:
===============
The generateAndTestSecMapWithSPs.py can also be used to test the generated sectormaps. To do 
so follow the steps above and change the parameter "generateSecMap" in line 22 from "True" to 
"False". This will generate a root file and debugging output which can be used to evaluate 
the performance of VXDTF. 




NOTE: the performance of the track finding is a function of the amount of data (number of events) used for the training! So 
      the steps 1 to 4 should be repeated iterated to find the optimal amount of data to achieve a maximal efficiency.
      The script tracking/vxdCaTracking/extendedExamples/secMapGen/svdL3displacement1mmNov2015/filterThis.sh
      shows one (not necessarily the optimal) example of how such an iteration can look like.



===========================================================================================================================

Find below a copy of the original email by Jakob explaining the scripts (in German). The line numbers correspond to 
the scripts found in tracking/vxdCaTracking/extendedExamples/secMapGen/svdL3displacement1mmNov2015/. 


Hiho,

geht leider nicht ganz so einfach wie für den beam test.
Die viel komplexere Geometrie braucht mehr Daten und daher ist ein Multiprocessor-setting für das Sammeln der Rohdaten ("FilterCalculator"), nicht aber für das Erstellen der secMap ("ExportSecMap") wichtig.

gibt im Ordner ../tracking/vxdCaTracking/extendedExamples/secMapGen/svdL3displacement1mmNov2015 (oder ähnlich) die relevanten files:
generateAndTestSecMapWithSPs.py <- zum Sammeln der Rohdaten, kann wiederholt mit verschiedenen Seeds laufen gelassen werden, um ne ausreichende Datenmenge zu bekommen.
onlyExportSecMapWithSPs.py <- zum Erstellen der SecMaps, einfach wenn alle Rohdaten gesammelt wurden einmal laufen lassen.

hier die Details:
generateAndTestSecMapWithSPs.py:
^^^^^^^^^^^^^^^^^^^^^^

Line 22: zum sammeln der Rohdaten: generateSecMap auf True setzen. Ist das auf false, kannst deine neue secMap testen, wenn du sie in tracking/data/VXDTFIndex.xml installiert hast (analog wie beim Beamtest, nur hier anderer Ordner)
Line 24 & 25: damit setzen, ob evtGen und/oder pGun benützt wird.
Line 31: usePXD: nur dann auf true setzen, wenn du 6 layer tracking testen willst. Ansonsten auf False setzen (ich glaub das gilt nur fürs secMap testen, nicht fürs training)
Line 32: Setze auf False, das war nur für den Run relevant, der damals gemacht wurde

Line 39 & 40: die setze ich normalerweise per shell script, wichtig ist hier aber: wenn du das File nicht für 100k+ events auf einmal laufen lassen willst, lass das teil mit kleineren nEvents laufen, aber mit verschiedenen initialValues, weil sonst immer die selben Events erzeugt werden.

Line 48-70: settings für die bis zu 2 pGuns, die du mit line 25 eingeschaltet hast (hat ne pGun 0 tracks, wird auch kein Track erzeugt (da gabs irgendwann mal ein Problem, weiß nicht, ob dieses File schon die gefixte Version ist. Solltest du also tracks bekommen, die du nicht erwartest, dann wäre hier eine mögliche PRoblemquelle)).

Line 73: in GeV/c: die transverse momentum cuts für die secMaps. Die Anzahl der Einträge bestimmt die Anzahl der secMap-passes, die erstellt werden. Der erste cut ist lower limit, tracks darunter werden nicht gesammelt

Line 76: der name deiner SecMaps: der muss natürlich immer für dich passen und möglichst aussagekräftig sein (meine waren das selten/nie)

Line 82&83: die cuts für die sectors auf nem sensor in relativen (element [0;1]) coords: die ersten und letzten Einträge sind lower und upper limits. Eugenio wollte das im Redesign anders haben, also sind die nicht 1:1 übertragbar -> vorsicht!

Line 101-106: wenn du generateSecMap == false gesetzt hast, musst hier natürlich die zu testenden secMaps eintragen.

Line 150-192: settings für das sammeln der Rohdaten, bitte ggf basf2 -m FilterCalculatorWithSPs ausführen.

Line 202 & 207: falls du spezielle geometries und so testen willst...

Line564: set_nprocesses: hast nen 8-Kerner, lass es auf dem setting. nCores (+HyperThreadingstuff) und evtl -1 wäre mein empfehlenswertes setting

Line 567: lass nprocesses hier auf 0, weil sonst der VXDTF undeterministisch wegen multithreading wird.

die restlichen lines: wenn dir langweilig ist, kannst die natürlich gerne durchschauen, müsste aber größtenteils für alle usecases passen


onlyExportMapWithSPs.py:
^^^^^^^^^^^^^^^^^^^^^^

Line 15: der selbe Name für die zu erstellende secMap wie im anderen File. Hätt ich mal automatisieren sollen, hab ich net, sorry...

Line 44: stretchfactor: dehnt die ermittelten cuts um gegebene prozente. Da es manchmal komische effekte hat, wenn überhaupt, nur im <1.0-Bereich benützen

Line 45: für den Exporter gibt es effektiv 2 Fälle: ein Sample wo nicht viel drin ist (weil es selten getriggert wurde) und welche, wo viel Daten da sind. Der erste Eintrag sagt, ab wann man samples überhaupt ernst nehmen sollte (dh hat man weniger als den ersten wert in einem sample, wird diese sector-combi verworfen), der 2. sagt, ab wann man "viel" hat. Sind beides absolute Zahlen, wurde afaik mit dem Redesign geändert.

Line 46 & 47 : welche quantile wollen wir für die cuts? 46 ist der quantil-Wert für "große" samples, bei 47 ist der für kleine. 0, 1 bedeutet min und max, 0.001, 0.999 meint 0.1% & 99.9%

Line 49: wenn der äußere sector x mal vorkommt und der innere sector zum äußeren seltener als 0.5% vorkommt, dann verwirf die combi.

Line 55-60, das selbe wie oben, nur für die VXD-secMap.

Allgemein: nEvents und initialValues sind egal, alles von dem Modul wird im Terminate ausgeführt. Muss aber >0 sein, weil sich basf2 sonst aufregt. nProcesses _muss_ auf 0 (also single-threaded) bleiben, weil das in der programmierten Weise nicht multi-threaded machbar ist.


ich hoffe, die Mail reicht zum ausreichenden "zurechtkommen". Good Luck!

cheers,
Jakob 
