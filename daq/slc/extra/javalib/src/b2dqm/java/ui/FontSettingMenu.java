package b2dqm.java.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import b2daq.graphics.FontProperty;
import b2daq.graphics.HtmlColor;


public class FontSettingMenu extends JMenu {

	private static final long serialVersionUID = 1L;
	private FontProperty _font;
	private JPanel _panel;

	public FontSettingMenu(String title, FontProperty font, JPanel panel){
		super(title);
		_font = font;
		_panel = panel;
		JMenuItem item = new JMenuItem("Set Color");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _font.getColor().toString());
			if ( str != null ) {
				_font.setColor(new HtmlColor(str));
				_panel.repaint();
			}
		}
		});
		add(item);
		item = new JMenuItem("Set Size");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _font.getSize());
				if ( str != null ) {
					_font.setSize(Double.parseDouble(str));
					_panel.repaint();
				}
			}
		});
		add(item);
		item = new JMenuItem("Set Family");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _font.getFamily());
				if ( str != null ) {
					_font.setFamily(str);
					_panel.repaint();
				}
			}
		});
		add(item);
	}
	
}
