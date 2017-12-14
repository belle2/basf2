package org.belle2.daq.dqm.graphics;

import java.util.ArrayList;

import org.belle2.daq.dqm.Axis;
import org.belle2.daq.graphics.GraphicsDrawer;
import org.belle2.daq.graphics.HtmlColor;

public class GColorAxis extends GAxisY {

	public final static HtmlColor[] COLOR_LIST = { new HtmlColor(153, 0, 255), new HtmlColor(102, 0, 255),
			new HtmlColor(51, 0, 255), new HtmlColor(0, 0, 255), new HtmlColor(0, 51, 255), new HtmlColor(0, 102, 255),
			new HtmlColor(0, 153, 255), new HtmlColor(0, 204, 255), new HtmlColor(0, 255, 255), new HtmlColor(0, 255, 204),
			new HtmlColor(0, 255, 153), new HtmlColor(0, 255, 102), new HtmlColor(0, 255, 51), new HtmlColor(0, 255, 0),
			new HtmlColor(51, 255, 0), new HtmlColor(102, 255, 0), new HtmlColor(153, 255, 00), new HtmlColor(204, 255, 0),
			new HtmlColor(255, 255, 0), new HtmlColor(255, 204, 0), new HtmlColor(255, 153, 00), new HtmlColor(255, 102, 0),
			new HtmlColor(255, 51, 0), new HtmlColor(255, 0, 0), new HtmlColor(255, 0, 51) };

	private ArrayList<HtmlColor> colorPattern = new ArrayList<>();

	public GColorAxis() {
		super();
		direction = "right";
	}

	public GColorAxis(Axis axis) {
		super(axis);
		direction = "right";
	}

	public GColorAxis(int nbin, double min, double max, String label, boolean fixMin, boolean fixMax, boolean logon) {
		super(nbin, min, max, label, fixMin, fixMax, logon);
		direction = "right";
		setHand(HAND_L);
		getLabelsFont().setSize(0.85);
	}

	@Override
	public void draw(GraphicsDrawer canvas) {
		if (!isVisible()) {
			return;
		}
		canvas.setFont(getLabelsFont());
		canvas.setLine(null);
		if (colorPattern.size() == 0) {
			for (HtmlColor color : COLOR_LIST) {
				colorPattern.add(color);
			}
		}
		double height = length / colorPattern.size();
		double X = x0 - padding_x;
		double Y = y0;
		double px_height = height;
		if (height < 0.005) {
			px_height = 0.005;
		}
		for (HtmlColor color : colorPattern) {
			Y -= height;
			canvas.getFill().setColor(color);
			canvas.drawRect(X, Y, padding_x * 1.05, px_height * 1.05, 1);
		}
		super.draw(canvas);
	}

	public boolean hit(double x, double y) {
		if (!visible) {
			return false;
		}
		return x >= x0 - padding_x && x <= x0 + padding_x && y <= y0 && y >= y0 - length;
	}

	public ArrayList<HtmlColor> getColorPattern() {
		return colorPattern;
	}

	public void setColorPattern(ArrayList<HtmlColor> pattern) {
		colorPattern.clear();
		colorPattern.addAll(pattern);
	}

	public void setColorPattern(HtmlColor[] pattern) {
		colorPattern.clear();
		for (HtmlColor color : pattern)
			colorPattern.add(color);
	}

	public void setColorPattern(String pattern) {
		String[] str = pattern.split(",");
		ArrayList<HtmlColor> paint = new ArrayList<>();
		for (int i = 0; i < str.length; i++) {
			paint.add(new HtmlColor(str[i]));
		}
		colorPattern.clear();
		colorPattern.addAll(paint);
	}

}
