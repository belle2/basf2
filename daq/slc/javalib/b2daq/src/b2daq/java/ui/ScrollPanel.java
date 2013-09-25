package b2daq.java.ui;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;

import javax.swing.JPanel;
import javax.swing.JScrollPane;

public class ScrollPanel extends JScrollPane {

	private static final long serialVersionUID = 1L;
	
	private Component _panel = new JPanel();
	
	public ScrollPanel() {
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				setSize(new Dimension(getWidth(), getHeight()));
				repaint();
			}
    	});
		setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	}
	
	public void setViewportView(Component panel) {
		super.setViewportView(panel);
		_panel = panel;
	}
	
	public void setSize(Dimension dim) {
		int width = dim.width;
		int height = dim.height;
		super.setSize(new Dimension(width, height));
		if ( _panel.getPreferredSize().height > height ) {
			_panel.setSize(new Dimension(width, height));
		} else {
			_panel.setSize(new Dimension(width - (int)(getVerticalScrollBar().getWidth()*1.2), _panel.getHeight()));
		}
	}
}
