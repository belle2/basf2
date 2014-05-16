package b2daq.dqm.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import b2daq.graphics.HtmlColor;
import b2daq.graphics.LineProperty;


public class LineSettingMenu extends JMenu {

	private static final long serialVersionUID = 1L;
	private LineProperty _line;
	private JPanel _panel;

	public LineSettingMenu(String title, LineProperty line, JPanel panel){
		super(title);
		_line = line;
		_panel = panel;
		JMenuItem item = new JMenuItem("Set Line Color");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _line.getColor().toString());
			if ( str != null ) {
				_line.setColor(new HtmlColor(str));
				_panel.repaint();
			}
		}
		});
		add(item);
		item = new JMenuItem("Set Line Width");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _line.getWidth());
				if ( str != null ) {
					_line.setWidth((int)Double.parseDouble(str));
					_panel.repaint();
				}
			}
		});
		add(item);
		item = new JMenuItem("Set Line Style");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _line.getStyle());
				if ( str != null ) {
					_line.setStyle(str);
					_panel.repaint();
				}
			}
		});
		add(item);
	}
	
}
