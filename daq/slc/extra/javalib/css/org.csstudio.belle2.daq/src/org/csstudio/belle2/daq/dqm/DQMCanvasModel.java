package org.csstudio.belle2.daq.dqm;

import org.csstudio.opibuilder.model.AbstractContainerModel;
import org.csstudio.opibuilder.properties.BooleanProperty;
import org.csstudio.opibuilder.properties.ColorProperty;
import org.csstudio.opibuilder.properties.DoubleProperty;
import org.csstudio.opibuilder.properties.IntegerProperty;
import org.csstudio.opibuilder.properties.StringProperty;
import org.csstudio.opibuilder.properties.WidgetPropertyCategory;
import org.eclipse.swt.graphics.RGB;
import org.csstudio.opibuilder.util.OPIColor;

public class DQMCanvasModel extends AbstractContainerModel {

	// public static final String ID = "org.csstudio.belle2.daq.dqm.DQMCanvas";
	public static final String ID = "DQMCanvas";

	public static final int maxhists = 10;
	public static final String useLegend = "useLegend";
	public static final String nhists = "nhists";
	public static final String canvasTitle = "canvasTitle";
	public static final String canvasXTitle = "canvasXTitle";
	public static final String canvasYTitle = "canvasYTitle";
	public static final String canvasFillColor = "canvasFillColor";
	public static final String canvasLineColor = "canvasLineColor";
	public static final String legendFillColor = "legendFillColor";
	public static final String legendLineColor = "legendLineColor";
	public static final String padFillColor = "padFillColor";
	public static final String padLineColor = "padLineColor";
	public static final String padLogX = "padLogX";
	public static final String padLogY = "padLogY";
	public static final String padAvarageFactor = "padAvarageFactor";
	public static final String padMinY = "padMinY";
	public static final String padMaxY = "padMaxY";
	public static final String padMinZ = "padMinZ";
	public static final String padMaxZ = "padMaxZ";
	public static final String histUseFillColor = "histUseFillColor";
	public static final String histFillColor = "histFillColor";
	public static final String histUseLineColor = "histUseLineColor";
	public static final String histLineColor = "histLineColor";
	public static final String histLineWidth = "histLineWidth";
	public static final String histName = "histName";
	public static final String histTitle = "histTitle";
	public static final String histNdim = "histNdim";
	public static final String histNbinsX = "histNbinsX";
	public static final String histXmin = "histXmin";
	public static final String histXmax = "histXmax";
	public static final String histNbinsY = "histNbinsY";
	public static final String histYmin = "histYmin";
	public static final String histYmax = "histYmax";
	public static final String histNbinsZ = "histNbinsZ";
	public static final String histZmin = "histZmin";
	public static final String histZmax = "histZmax";

	public final static WidgetPropertyCategory Canvas = new WidgetPropertyCategory() {
		public String toString() {
			return "Canvas";
		}
	};

	public final static WidgetPropertyCategory Pad = new WidgetPropertyCategory() {
		public String toString() {
			return "Pad";
		}
	};

	public final static WidgetPropertyCategory Legend = new WidgetPropertyCategory() {
		public String toString() {
			return "Legend";
		}
	};

	public static WidgetPropertyCategory createHistoPropertyCategory(final int id) {
		WidgetPropertyCategory category = new WidgetPropertyCategory() {
			public String toString() {
				return "Histogram_" + id;
			}
		};
		return category;
	}

	@Override
	protected void configureProperties() {
		addProperty(new BooleanProperty(useLegend, "UseLegend", Canvas, false));
		addProperty(new IntegerProperty(nhists, "NHists", Canvas, 0));
		addProperty(new StringProperty(canvasTitle, "Title", Canvas, ""));
		addProperty(new StringProperty(canvasXTitle, "XTitle", Canvas, ""));
		addProperty(new StringProperty(canvasYTitle, "YTitle", Canvas, ""));
		addProperty(new ColorProperty(canvasFillColor, "Fill Color", Canvas, new RGB(255, 255, 255)));
		addProperty(new ColorProperty(canvasLineColor, "Line Color", Canvas, new RGB(0, 0, 0)));
		addProperty(new ColorProperty(padFillColor, "Fill Color", Pad, new RGB(255, 255, 255)));
		addProperty(new ColorProperty(padLineColor, "Line Color", Pad, new RGB(0, 0, 0)));
		addProperty(new BooleanProperty(padLogX, "X Log scale", Pad, false));
		addProperty(new BooleanProperty(padLogY, "Y Log scale", Pad, false));
		addProperty(new DoubleProperty(padAvarageFactor, "Avarage factor", Pad, -1));
		addProperty(new DoubleProperty(padMinY, "Y Minimum", Pad, -1));
		addProperty(new DoubleProperty(padMaxY, "Y Maximum", Pad, -1));
		addProperty(new DoubleProperty(padMinZ, "Z Minimum", Pad, -1));
		addProperty(new DoubleProperty(padMaxZ, "Z Maximum", Pad, -1));
		for (int n = 0; n < maxhists; n++) {
			WidgetPropertyCategory category = createHistoPropertyCategory(n);
			addProperty(new BooleanProperty(histUseFillColor + n, "Use Fill Color", category, false));
			addProperty(new ColorProperty(histFillColor + n, "Fill Color", category, new RGB(255, 255, 255)));
			addProperty(new BooleanProperty(histUseLineColor + n, "Use Line Color", category, false));
			addProperty(new ColorProperty(histLineColor + n, "Line Color", category, new RGB(255, 0, 0)));
			addProperty(new DoubleProperty(histLineWidth + n, "Line Width", category, 1));
			addProperty(new StringProperty(histName + n, "Name", category, "histo_" + n));
			addProperty(new StringProperty(histTitle + n, "Tile", category, ""));
		}
		setHistoPropertyVisible(getNHists());
		addProperty(new ColorProperty(legendFillColor, "Fill Color", Legend, new RGB(255, 255, 255)));
		addProperty(new ColorProperty(legendLineColor, "Line Color", Legend, new RGB(0, 0, 0)));
		setLegendPropertyVisible(false);
	}

	public void setLegendPropertyVisible(boolean visible) {
		setPropertyVisible(legendFillColor, visible);
		setPropertyVisible(legendLineColor, visible);
	}

	@Override
	public String getTypeID() {
		return ID;
	}

	public boolean useLegend() {
		return getCastedPropertyValue(useLegend);
	}

	public int getNHists() {
		return getCastedPropertyValue(nhists);
	}

	public String getCanvasTitle() {
		return getCastedPropertyValue(canvasTitle);
	}

	public String getCanvasXTitle() {
		return getCastedPropertyValue(canvasXTitle);
	}

	public String getCanvasYTitle() {
		return getCastedPropertyValue(canvasYTitle);
	}

	public OPIColor getCanvasFillColor() {
		return getCastedPropertyValue(canvasFillColor);
	}

	public OPIColor getCanvasLineColor() {
		return getCastedPropertyValue(canvasLineColor);
	}

	public OPIColor getPadFillColor() {
		return getCastedPropertyValue(padFillColor);
	}

	public OPIColor getPadLineColor() {
		return getCastedPropertyValue(padLineColor);
	}

	public boolean getPadLogX() {
		return getCastedPropertyValue(padLogX);
	}

	public boolean getPadLogY() {
		return getCastedPropertyValue(padLogY);
	}

	public double getPadAvarageFactor() {
		return getCastedPropertyValue(padAvarageFactor);
	}

	public double getPadMinY() {
		return getCastedPropertyValue(padMinY);
	}

	public double getPadMaxY() {
		return getCastedPropertyValue(padMaxY);
	}

	public double getPadMinZ() {
		return getCastedPropertyValue(padMinZ);
	}

	public double getPadMaxZ() {
		return getCastedPropertyValue(padMaxZ);
	}

	public OPIColor getHistFillColor(int n) {
		return getCastedPropertyValue(histFillColor + n);
	}

	public OPIColor getHistLineColor(int n) {
		return getCastedPropertyValue(histLineColor + n);
	}

	public boolean getHistUseFillColor(int n) {
		return getCastedPropertyValue(histUseFillColor + n);
	}

	public boolean getHistUseLineColor(int n) {
		return getCastedPropertyValue(histUseLineColor + n);
	}

	public double getHistLineWidth(int n) {
		return getCastedPropertyValue(histLineWidth + n);
	}

	public String getHistName(int n) {
		return getCastedPropertyValue(histName + n);
	}

	public String getHistTitle(int n) {
		return getCastedPropertyValue(histTitle + n);
	}

	public OPIColor getLegendFillColor() {
		return getCastedPropertyValue(legendFillColor);
	}

	public OPIColor getLegendLineColor() {
		return getCastedPropertyValue(legendLineColor);
	}

	public void setHistoPropertyVisible(int nhists) {
		for (int n = 0; n < maxhists; n++) {
			boolean visible = n < nhists;
			setPropertyVisible(histUseFillColor + n, visible);
			setPropertyVisible(histFillColor + n, visible);
			setPropertyVisible(histUseLineColor + n, visible);
			setPropertyVisible(histLineColor + n, visible);
			setPropertyVisible(histLineWidth + n, visible);
			setPropertyVisible(histName + n, visible);
			setPropertyVisible(histTitle + n, visible);
		}
	}

}
