package b2daq.dqm.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;

import b2daq.dqm.graphics.GPad;


public class PadBoundsSettingMenu extends JMenu {

	private static final long serialVersionUID = 1L;
	private GPad _pad;
	private CanvasPanel _panel;

	public PadBoundsSettingMenu(String title, GPad pad, CanvasPanel panel) {
		super(title);
		_pad = pad;
		_panel = panel;
		JMenuItem item = new JMenuItem("Set X");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _pad.getX());
			if ( str != null ) {
				_pad.setX(Double.parseDouble(str));
				_panel.getCanvas().resetPadding();
				_panel.repaint();
			}
		}
		});
		add(item);
		item = new JMenuItem("Set Y");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _pad.getY());
			if ( str != null ) {
				_pad.setY(Double.parseDouble(str));
				_panel.getCanvas().resetPadding();
				_panel.repaint();
			}
		}
		});
		add(item);
		item = new JMenuItem("Set Width");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _pad.getWidth());
			if ( str != null ) {
				_pad.setWidth(Double.parseDouble(str));
				_panel.getCanvas().resetPadding();
				_panel.repaint();
			}
		}
		});
		add(item);
		item = new JMenuItem("Set Height");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _pad.getHeight());
			if ( str != null ) {
				_pad.setHeight(Double.parseDouble(str));
				_panel.getCanvas().resetPadding();
				_panel.repaint();
			}
		}
		});
		add(item);
	}

}
