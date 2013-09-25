package b2daq.java.graphics;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;

import javax.swing.JFrame;
import javax.swing.JPanel;

import b2daq.graphics.GCircle;
import b2daq.graphics.GLine;
import b2daq.graphics.GPath;
import b2daq.graphics.GPolygon;
import b2daq.graphics.GRect;
import b2daq.graphics.GShape;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;


public class JavaGCanvasPanel extends JPanel {

	private static final long serialVersionUID = 1L;

	private JavaGCanvas _canvas = new JavaGCanvas();
	
	public JavaGCanvasPanel() {}
	public JavaGCanvasPanel(int width, int height) {
		setPreferredSize(new Dimension(width, height));
	}
	
	public void setPreferredSize(Dimension dim) {
		super.setPreferredSize(dim);
		_canvas.setWidth(dim.width);
		_canvas.setHeight(dim.height);
		repaint();
	}
	
	public void add(GShape shape) { 
		_canvas.add(shape);
	}
	
	public void remove(GShape shape) { 
		_canvas.remove(shape);
	}
	
	public void paintComponent(Graphics g){
		((Graphics2D)g).setRenderingHint(RenderingHints.KEY_ANTIALIASING, 
				RenderingHints.VALUE_ANTIALIAS_ON);
		_canvas.setGraphics(g);
		_canvas.store();
		_canvas.setWidth(getWidth());
		_canvas.setHeight(getHeight());
		_canvas.update();
	}
	
	static public void main(String[] argv) {
		JavaGCanvasPanel c = new JavaGCanvasPanel();
		c.setSize(500, 500);
		c.add(new GLine(0, 0, 1, 1, HtmlColor.GREEN));
		c.add(new GRect(0.2, 0.6, 0.2, 0.2, HtmlColor.PINK, HtmlColor.RED));
		double[] x = {0, 0.2, 0.7}; 
		double[] y = {1, 0.6, 1}; 
		c.add(new GPolygon(x, y, HtmlColor.CYAN, HtmlColor.BLUE));
		c.add(new GCircle(0.5, 0.5 ,0.3, 0.3, HtmlColor.YELLOW, HtmlColor.ORANGE));
		c.add(new GText("KONNO", 0.5, 0.5, "center", 0., HtmlColor.RED));
		GPath path = new GPath();
		path.moveTo(0, 0.5);
		path.drawTo(0.2, 0.5);
		path.moveTo(0.4, 0.5);
		path.drawTo(0.6, 0.5);
		path.moveTo(0.8, 0.5);
		path.drawTo(0.9, 0.5);
		path.moveTo(0.9, 0.2);
		path.drawTo(0.8, 0.2);
		c.add(path);
		JFrame frame = new JFrame();
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setSize(500,500);
		frame.add(c);
		c.repaint();
		frame.setVisible(true);
	}
	
}
