package b2dqm.java.ui;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsEnvironment;
import java.awt.RenderingHints;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.image.BufferedImage;

import javax.swing.JPanel;
import javax.swing.border.BevelBorder;

import b2daq.graphics.GCanvas;
import b2daq.java.graphics.JavaGCanvas;
import b2daq.ui.Updatable;
import b2dqm.graphics.Canvas;
import b2dqm.graphics.GHisto;
import b2dqm.graphics.GMonObject;
import b2dqm.java.ui.event.CanvasMouseListener;


public class CanvasPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;

	private static final int WIDTH_MIN = (int)(GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().width / 3 * 0.8);
	private static final int HEIGHT_MIN = (int)(GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().height / 2 * 0.8);
	
	private Canvas _canvas;
	private JavaGCanvas _graphics = null;
	private BufferedImage _image_buffer = null;
	
	public CanvasPanel(Canvas canvas, int width, int height){
		if (width < WIDTH_MIN) width = WIDTH_MIN;
		if (height < HEIGHT_MIN) height = HEIGHT_MIN;
		_graphics = new JavaGCanvas();		
		_canvas = canvas;
		setName(canvas.getName());
		setBorder(new BevelBorder(BevelBorder.RAISED));
		setPreferredSize(new Dimension(width, height));
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				setPreferredSize( new Dimension(getWidth(), getHeight()) );
				repaint();
			}
    	});
		CanvasMouseListener listener = new CanvasMouseListener(_canvas);
		addMouseListener(listener);
		addMouseMotionListener(listener);
		setDoubleBuffered(true);
	}
	
	public CanvasPanel(String name, String title, int width, int height){
		if (width < WIDTH_MIN) width = WIDTH_MIN;
		if (height < HEIGHT_MIN) height = HEIGHT_MIN;
		_graphics = new JavaGCanvas();		
		_canvas = new Canvas(name,title, width, height);
		setName(name);
		setBorder(new BevelBorder(BevelBorder.RAISED));
		setPreferredSize(new Dimension(width, height));
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				setPreferredSize( new Dimension(getWidth(), getHeight()) );
				repaint();
			}
    	});
		CanvasMouseListener listener = new CanvasMouseListener(_canvas);
		addMouseListener(listener);
		addMouseMotionListener(listener);
		setDoubleBuffered(true);
	}

	public CanvasPanel() { this("", "", 100, 100); }
	public CanvasPanel(String name, String title) { this(name, title, 100, 100); }

	public Canvas getCanvas(){ return _canvas; }

	public GMonObject getMonObject(int index) {
		return _canvas.getMonObject(index);
	}
	
	public GHisto getHisto(int index) {
		return _canvas.getHisto(index);
	}

	public int getNHistos() {
		return _canvas.getNHistos();
	}

	public void setLegend(String list) {
	}
	
	public void setPreferredSize(Dimension dim){
		int width = dim.width;
		int height = dim.height;
		int size = (int)((double)width/27);
		_graphics.setFontRealSize(size);
		if (width < WIDTH_MIN) width = WIDTH_MIN;
		if (height < HEIGHT_MIN) height = HEIGHT_MIN;
		super.setPreferredSize(new Dimension(width, height));
		repaint();
	}
	
	public synchronized void paintComponent(Graphics g){
		Graphics2D g2 = ((Graphics2D)g); 
		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		_graphics.setGraphics(g2);
		_graphics.store();
		_graphics.setWidth(getWidth());
		_graphics.setHeight(getHeight());
		_canvas.draw(_graphics);
		_graphics.restore();
	}
	
	public synchronized void update() {
		repaint();
	}
	
	public GCanvas getGCanvas() {
		return _graphics;
	}
	
	public BufferedImage getImage() {
		if ( _image_buffer == null ) {
			_image_buffer = (BufferedImage) createImage(getWidth(), getHeight());
		}
		return _image_buffer;
	}

}
