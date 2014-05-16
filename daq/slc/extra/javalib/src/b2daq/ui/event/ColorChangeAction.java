package b2daq.ui.event;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JFrame;

import b2daq.graphics.HtmlColor;
import b2daq.ui.ColorPaletWindow;
import b2daq.ui.Updatable;


public class ColorChangeAction implements ActionListener {

	private HtmlColor _color;
	private Updatable _target;
	private int _x;
	private int _y;
	
	public ColorChangeAction(Updatable target, HtmlColor color, int x, int y) {
		_target = target;
		_color = color;
		_x = x;
		_y = y;
	}

	public void actionPerformed(ActionEvent arg0) {
		JFrame frame = new ColorPaletWindow(_target, _color);
		frame.setBounds(_x, _y, 100, 80);
		frame.setVisible(true);
	}

}
