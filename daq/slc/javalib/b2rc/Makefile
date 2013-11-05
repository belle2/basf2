JCC        = javac
JAR        = jar
RM         = rm -rf
CP         = cp -f

TARGET     = b2rc

MANIFEST  = Belle2RunController.mf

SRCDIR     = ./src
OUTDIR     = ./classes
JARDIR     = ./jars

MAIN       = src/b2rc/java/Belle2RunController.java

JARFILE  = $(JARDIR)/b2rc.jar
JAREXE  = $(JARDIR)/b2rc-exe.jar

JAVALIB_PATH = $(BELLE2_DAQ_SLC_DIR)/javalib

CLASSPATH  = $(JAVALIB_PATH)/b2daq/classes:$(SRCDIR)
SOURCEPATH = $(JAVALIB_PATH)/b2daq/src:$(SRCDIR)
JFLAG      = -classpath $(CLASSPATH) -d $(OUTDIR) -sourcepath $(SOURCEPATH)

MYSQL_JAR = $(JAVALIB_PATH)/mysql-connector-java-5.1.26-bin.jar

all:
	@ echo "<< compiling $(MAIN) >>"
	@ if [ ! -d $(OUTDIR) ];then mkdir $(OUTDIR); fi
	@ $(JCC) $(JFLAG) $(MAIN)
	@ cp $(SRCDIR)/b2rc/java/ui/*.png $(OUTDIR)/b2rc/java/ui

jar : all 
	@ echo '<< creating jar file $(JARFILE) >>'
	@ if [ ! -d $(JARDIR) ];then mkdir $(JARDIR); fi
	@ $(JAR) cf $(JARFILE) -C $(OUTDIR) $(TARGET)
	@ $(JAR) uf $(JARFILE) -C $(JAVALIB_PATH)/b2daq/classes b2daq
	@ $(RM) $(OUTDIR)/com  $(OUTDIR)/org $(OUTDIR)/META-INF
	@ jar -xf $(MYSQL_JAR)
	@ mv com org META-INF $(OUTDIR)
	@ $(JAR) uf $(JAREXE) -C classes org
	@ $(JAR) uf $(JAREXE) -C classes com

exe : all 
	@ echo '<< creating jar file $(JAREXE) >>'
	@ if [ ! -d $(JARDIR) ];then mkdir $(JARDIR); fi
	@ $(JAR) cfm $(JAREXE) $(MANIFEST) -C $(OUTDIR) $(TARGET)
	@ $(JAR) uf $(JAREXE) -C $(JAVALIB_PATH)/b2daq/classes b2daq
	@ $(RM) $(OUTDIR)/com  $(OUTDIR)/org $(OUTDIR)/META-INF
	@ jar -xf $(MYSQL_JAR)
	@ mv com org META-INF $(OUTDIR)
	@ $(JAR) uf $(JAREXE) -C classes org
	@ $(JAR) uf $(JAREXE) -C classes com

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) *~ $(OUTDIR) $(BINDIR) $(JARDIR) $(CONDIR)

install: $(JARTARGET)
	@ echo '<< installing jar files to html directory >>'
	@ $(CP) $(JARTARGET) $(WEBJARDIR)

ant:
	@ ant