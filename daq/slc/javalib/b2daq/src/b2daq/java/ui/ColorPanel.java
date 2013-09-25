package b2daq.java.ui;

import java.awt.Color;

import javax.swing.JPanel;

import b2daq.graphics.HtmlColor;


public class ColorPanel extends JPanel {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	private HtmlColor _color;
	
	public ColorPanel(HtmlColor color) {
		setSize(20, 20);
		_color = color;
		if ( !_color.isNull() ) {
			setBackground(new Color(_color.getRed(),_color.getGreen(),_color.getBlue()));
		}
	}
	
	public HtmlColor getColor() {
		return _color;
	}

	public void setColor(HtmlColor color) {
		_color = color;
	}
}
