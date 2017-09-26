importPackage(Packages.org.csstudio.opibuilder.scriptUtil);
var pv0 = PVUtil.getString(triggerPV);
widget.setPropertyValue("label",pv0);
if(pv0=="ON") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,0));
  //widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  //widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(0,255,255));
} else if(pv0=="RAMPINGUP" || pv0=="RAMPINGDOWN") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,128,0));
} else if(pv0=="TRIP" || pv0=="RECOVERING") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,0,255));
} else if(pv0=="OFF") {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(192,192,192));
} else {
  widget.setPropertyValue("foreground_color",ColorFontUtil.getColorFromRGB(0,0,0));
  widget.setPropertyValue("background_color",ColorFontUtil.getColorFromRGB(255,255,255));
}