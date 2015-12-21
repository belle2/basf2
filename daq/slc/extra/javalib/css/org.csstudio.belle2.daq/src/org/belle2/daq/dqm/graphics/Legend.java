package org.belle2.daq.dqm.graphics;

import java.util.ArrayList;

import org.belle2.daq.dqm.Histo;
import org.belle2.daq.graphics.FillProperty;
import org.belle2.daq.graphics.FontProperty;
import org.belle2.daq.graphics.GraphicsDrawer;
import org.belle2.daq.graphics.GRect;
import org.belle2.daq.graphics.HtmlColor;
import org.belle2.daq.graphics.LineProperty;

public class Legend extends GRect {

	public static final int DIRECTION_Y = 0;
	public static final int DIRECTION_X = 1;

	private String title = "";
	private ArrayList<Histo> histograms = new ArrayList<>();
	private int direction;

	public Legend() {
		x = 0.68;
		y = 0.02;
		width = 0.28;
		height = 0.08;
		setFillColor(HtmlColor.WHITE);
		setLineColor(HtmlColor.BLACK);
		font.setSize(0.5);
	}

	public void setBounds(double x, double y, double width, double height) {
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
	}

	public void setDirection(int direction) {
		this.direction = direction;
	}

	public void setDirection(String direction) {
		if (direction.matches("x")) {
			this.direction = DIRECTION_X;
		} else {
			this.direction = DIRECTION_Y;
		}
	}

	public String getDirection() {
		if (this.direction == DIRECTION_X) {
			return "x";
		} else {
			return "y";
		}
	}

	public void add(Histo h) {
		histograms.add(h);
		height = histograms.size() * 0.05 + 0.02;
	}

	public void setTitle(String title) {
		this.title = title;
	}

	public String getTitle() {
		return title;
	}

	public ArrayList<Histo> getHistograms() {
		return histograms;
	}

	public void clear() {
		histograms.clear();
		histograms = new ArrayList<>();
	}
	
	@Override
	public void draw(GraphicsDrawer canvas) {
		FontProperty font = canvas.getFont();
		FillProperty fill = canvas.getFill();
		LineProperty line = canvas.getLine();
		canvas.setFont(getFont());
		canvas.setFill(getFill());
		canvas.setLine(getLine());
		canvas.drawRect(x, y, width, height);
		int n = 0;
		for (Histo h : histograms) {
			canvas.setFill(h.getFill());
			canvas.setLine(h.getLine());
			if (h.getDim() == 2) {
				canvas.drawString(h.getTitle(), x + 0.01, y + 0.01 * (3.5 + n * 5), "left middle");
			} else {
				if (h.getDraw().matches(".*L.*")) {
					if (h.getFill() != null && h.getFill().getColor() != null) {
						canvas.drawRect(x + 0.01, y + 0.01 * (2.5 + n * 5), 0.035, 0.02);
					} else {
						canvas.drawLine(x + 0.01, y + 0.01 * (3.5 + n * 5), x + 0.04, y + 0.01 * (3.5 + n * 5));
					}
				}
				if (h.getDraw().matches(".*P.*")) {
					canvas.drawMarker(x + 0.02, y + 0.01 * (4 + n * 5));
				}
				canvas.drawString(h.getTitle(), x + 0.06, y + 0.01 * (3.5 + n * 5), "left middle");
			}
			n++;
		}
		canvas.setFont(font);
		canvas.setFill(fill);
		canvas.setLine(line);
	}

}
