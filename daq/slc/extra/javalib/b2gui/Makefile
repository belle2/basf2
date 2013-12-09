JCC        = javac 
JAR        = jar
RM         = rm -rf
CP         = cp -f

TARGET     = b2gui

MANIFEST   = Belle2GUILauncher.mf

SRCDIR     = ./src
OUTDIR     = ./classes
JARDIR     = ./jars

MAIN       = src/b2gui/java/Belle2GUILauncherJavaGUI.java

JARFILE    = $(JARDIR)/b2gui.jar
JAREXE     = $(JARDIR)/b2gui-exe.jar

B2JAVA_PATH = $(BELLE2_LOCAL_DIR)/daq/slc/extra/javalib

CLASSPATH  = $(B2JAVA_PATH)/b2daq/classes:$(B2JAVA_PATH)/b2dqm/classes:$(B2JAVA_PATH)/b2rc/classes:$(B2JAVA_PATH)/b2logger/classes
SOURCEPATH = $(SRCDIR)

JFLAG      = -classpath $(CLASSPATH) -d $(OUTDIR) -sourcepath $(SOURCEPATH)

WEBJARDIR = $(DCONLINE_WEB_HOME)/java/jars

LIB_SRCS  = $(wildcard $(SRCDIR)/com/*.java)
LIB_SRCS += $(wildcard $(SRCDIR)/com/*/*.java)
LIB_SRCS += $(wildcard $(SRCDIR)/com/*/*/*.java)
LIB_SRCS += $(wildcard $(SRCDIR)/com/*/*/*/*.java)
LIB_SRCS += $(wildcard $(SRCDIR)/com/*/*/*/*/*.java)

LIB_OBJS = $(patsubst $(SRCDIR)/%.java, $(OUTDIR)/%.class, $(LIB_SRCS))

all : $(LIB_OBJS)
	@ echo "<< compiling $(MAIN) >>"
	@ if [ ! -d $(OUTDIR) ];then mkdir $(OUTDIR); fi
	@ $(JCC) $(JFLAG) $(MAIN)
	@ $(CP) $(SRCDIR)/b2gui/java/*.png $(OUTDIR)/b2gui/java/
	@ echo '<< creating jar file $(JAREXE) >>'
	@ if [ ! -d $(JARDIR) ];then mkdir $(JARDIR); fi
	@ $(JAR) cfm $(JAREXE) $(MANIFEST) -C $(OUTDIR) $(TARGET)
	@ $(JAR) uf $(JAREXE) -C $(OUTDIR) com
	@ $(JAR) uf $(JAREXE) -C $(B2JAVA_PATH)/b2daq/classes b2daq
	@ $(JAR) uf $(JAREXE) -C $(B2JAVA_PATH)/b2dqm/classes b2dqm
	@ $(JAR) uf $(JAREXE) -C $(B2JAVA_PATH)/b2rc/classes b2rc
	@ $(JAR) uf $(JAREXE) -C $(B2JAVA_PATH)/b2logger/classes b2logger
	@ $(RM) -rf com META-INF

$(LIB_OBJS) : $(OUTDIR)/%.class: $(SRCDIR)/%.java
	@ echo '<< compiling $@ >>'
	@ if [ ! -d $(OUTDIR) ];then mkdir $(OUTDIR); fi
	@ $(JCC) $(JFLAG) $<

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) *~ $(OUTDIR) $(BINDIR) $(JARDIR) $(CONDIR)

install: $(JARTARGET)
	@ echo '<< installing jar files to html directory >>'
	@ $(CP) $(JARTARGET) $(WEBJARDIR)

ant:
	@ ant