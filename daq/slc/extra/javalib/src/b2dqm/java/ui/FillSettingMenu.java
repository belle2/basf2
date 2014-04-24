package b2dqm.java.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import b2daq.graphics.FillProperty;
import b2daq.graphics.HtmlColor;


public class FillSettingMenu extends JMenu {

	private static final long serialVersionUID = 1L;
	private FillProperty _fill;
	private JPanel _panel;

	public FillSettingMenu(String title, FillProperty fill, JPanel panel){
		super(title);
		_fill = fill;
		_panel = panel;
		JMenuItem item = new JMenuItem("Set Color");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			
			String str = JOptionPane.showInputDialog( arg0.getSource() , _fill.getColor().toString());
			if ( str != null ) {
				_fill.setColor(new HtmlColor(str));
				_panel.repaint();
			}
		}
		});
		add(item);
		item = new JMenuItem("Set Transparency");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _fill.getTransparency());
				if ( str != null ) {
					_fill.setTransparency(Double.parseDouble(str));
					_panel.repaint();
				}
			}
		});
		add(item);
		item = new JMenuItem("Set Style");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _fill.getStyle());
				if ( str != null ) {
					_fill.setStyle(str);
					_panel.repaint();
				}
			}
		});
		add(item);
	}
	
}
