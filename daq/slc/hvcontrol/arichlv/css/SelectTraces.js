importPackage(Packages.org.csstudio.opibuilder.scriptUtil);

var index = PVUtil.getLong(triggerPV);
ConsoleUtil.writeString("Sector " + index + "\n");
plt = "vmon-Sector" + index + ".plt"  
var data=display.getWidget("VmonGraph");
data.setPropertyValue("filename", plt, true );


