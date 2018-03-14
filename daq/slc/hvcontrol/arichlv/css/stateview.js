importPackage(Packages.org.csstudio.opibuilder.scriptUtil); 
var pv0 = PVUtil.getString(pvs[0]);
if(pv0=="READY") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,0));
} else if(pv0=="NOTREADY") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,0,0));
} else if(pv0=="RUNNING") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,255));
} else if(pv0=="LOADING" || pv0=="STARTING" || pv0=="STOPPING") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,128,0));
} else if(pv0=="ABORTING" || pv0=="RECOVERING") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,0,255));
} else if(pv0=="OFF") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(192,192,192));
} else {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,255,255));
}