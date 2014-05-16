JCC        = javac 
JAR        = jar
RM         = rm -rf
CP         = cp -f

TARGET     = b2gui

MANIFEST   = Belle2GUILauncher.mf

SRCDIR     = ./src
OUTDIR     = ./classes
JARDIR     = ./jars

JARFILE    = $(JARDIR)/b2gui.jar
JAREXE     = $(JARDIR)/b2gui-exe.jar

B2JAVA_PATH = $(BELLE2_LOCAL_DIR)/daq/slc/extra/javalib

CLASSPATH  = $(OUTDIR)
SOURCEPATH = $(SRCDIR)

JFLAG      = -classpath $(CLASSPATH) -d $(OUTDIR) -sourcepath $(SOURCEPATH)

LIB_OBJS = $(patsubst $(SRCDIR)/%.java, $(OUTDIR)/%.class, $(LIB_SRCS))

all :
	@ mkdir -p $(OUTDIR)
	@ $(JCC) $(JFLAG) src/b2gui/java/Belle2GUILauncherJavaGUI.java
	@ $(JCC) $(JFLAG) src/b2dqm/java/Belle2DQMBrowser.java
	@ $(JCC) $(JFLAG) src/b2dqm/java/Belle2EnvMonitor.java
	@ $(JCC) $(JFLAG) src/b2rc/java/Belle2RunController.java
	@ $(JCC) $(JFLAG) src/b2logger/java/Belle2LogViewer.java
	@ $(CP) $(SRCDIR)/b2gui/java/*.png $(OUTDIR)/b2gui/java/
	@ $(CP) $(SRCDIR)/b2dqm/java/ui/*.png $(OUTDIR)/b2dqm/java/ui
	@ $(CP) $(SRCDIR)/b2rc/java/ui/*.png $(OUTDIR)/b2rc/java/ui
	@ $(CP) $(SRCDIR)/b2logger/java/ui/*.png $(OUTDIR)/b2logger/java/ui
	@ echo '<< creating jar file $(JAREXE) >>'
	@ mkdir -p $(JARDIR)
	@ $(JAR) cfm $(JAREXE) $(MANIFEST) -C $(OUTDIR) $(TARGET)
	@ $(JAR) uf $(JAREXE) -C $(OUTDIR) com
	@ $(JAR) uf $(JAREXE) -C $(OUTDIR) b2daq
	@ $(JAR) uf $(JAREXE) -C $(OUTDIR) b2dqm
	@ $(JAR) uf $(JAREXE) -C $(OUTDIR) b2rc
	@ $(JAR) uf $(JAREXE) -C $(OUTDIR) b2logger
	@ $(RM) -rf com META-INF

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) *~ $(OUTDIR) $(BINDIR) $(JARDIR) $(CONDIR)

install: $(JARTARGET)
	@ echo '<< installing jar files to html directory >>'
	@ $(CP) $(JARTARGET) $(WEBJARDIR)

ant:
	@ ant