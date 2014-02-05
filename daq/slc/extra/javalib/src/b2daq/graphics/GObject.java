package b2daq.graphics;

public interface GObject {
	public void draw(GCanvas c);
	public boolean hit(double x, double y);

}
