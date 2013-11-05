package b2daq.java.ui;

import java.awt.GridLayout;
import java.awt.Rectangle;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.BevelBorder;

import b2daq.graphics.HtmlColor;
import b2daq.ui.Updatable;


public class ColorPaletWindow extends JFrame {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private ColorPanel _selected_panel;
	private HtmlColor _color;
	private Updatable _target;
	
	public HtmlColor getSelectedColor() {
		return _selected_panel.getColor();
	}
	
	public void setSelectedColor(ColorPanel select) {
		_selected_panel = select;
	}
	
	public ColorPaletWindow(Updatable target, HtmlColor color){
		super();
		_color = color;
		_target = target;
		ArrayList<HtmlColor> color_v = new ArrayList<HtmlColor>();
		color_v.add(new HtmlColor(HtmlColor.NULL));
		color_v.add(new HtmlColor(HtmlColor.WHITE));
		color_v.add(new HtmlColor(HtmlColor.BLACK));
		color_v.add(new HtmlColor(HtmlColor.GRAY));
		color_v.add(new HtmlColor(HtmlColor.BLUE));
		color_v.add(new HtmlColor(HtmlColor.GREEN));
		color_v.add(new HtmlColor(HtmlColor.ORANGE));
		color_v.add(new HtmlColor(HtmlColor.MAGENTA));
		color_v.add(new HtmlColor(HtmlColor.RED));
		color_v.add(new HtmlColor(HtmlColor.PINK));
		color_v.add(new HtmlColor(HtmlColor.YELLOW));
		color_v.add(new HtmlColor(HtmlColor.CYAN));
		
		setLayout(new GridLayout(2, 6));
		setBounds(new Rectangle(450, 250, 100, 80));
		ColorSelectEvent event = new ColorSelectEvent(this);
		for (int n = 0; n < color_v.size(); n++ ) {
			ColorPanel panel = new ColorPanel(color_v.get(n));
			add(panel);
			panel.addMouseListener(event);
		}
	}

	public HtmlColor getColorTarget() {
		return _color;
	}
	
	public class ColorSelectEvent implements MouseListener {
		
		private JFrame fFrame;
		
		public ColorSelectEvent(JFrame frame) {
			fFrame = frame;
		}
		
		public void mouseClicked(MouseEvent e) {
			setSelectedColor((ColorPanel)e.getSource());
			JPanel panel = (JPanel) ((ColorPanel)e.getSource()).getParent();
			for ( int n = 0; n < panel.getComponentCount(); n++ ) {
				((ColorPanel)panel.getComponent(n)).setBorder(null);
			}
			((ColorPanel)e.getSource()).setBorder(new BevelBorder(BevelBorder.LOWERED));
			if ( _color == null ) _color = new HtmlColor();
			_color.copy(((ColorPanel)e.getSource()).getColor());
			_target.update();
			fFrame.dispose();
		}
		
		public void mouseEntered(MouseEvent e) {}
		public void mouseExited(MouseEvent e) {}
		public void mousePressed(MouseEvent e) {}
		public void mouseReleased(MouseEvent e) {}
	}
}
