package b2daq.dqm.core;

import b2daq.dqm.graphics.GHisto;
import b2daq.dqm.graphics.HyperRef;
import b2daq.graphics.GraphicsDrawer;
import java.util.ArrayList;

public abstract class AbstractHisto extends MonObject {

    protected String title;
    protected Axis axisX;
    protected Axis axisY;
    protected Axis axisZ;
    protected NumberArray data;
    protected long time;
    protected ArrayList<HyperRef> channelRefs = new ArrayList<>();
    protected HyperRef selectedChannelRef = null;
    private boolean ajustableAxis = false;
    protected GHisto drawer;

    public AbstractHisto() {
        this("", "");
    }

    public AbstractHisto(String name, String title) {
        _position_id = 0;
        _tab_id = 0;
        setName(name);
        String[] title_v = title.split(";");
        this.title = new String("");
        axisX = new Axis(1, 0, 1, "");
        axisY = new Axis(1, 0, 1, "");
        axisZ = new Axis(1, 0, 1, "");
        if (title_v.length > 0) {
            this.title = title_v[0];
        }
        if (title_v.length > 1) {
            axisX.setTitle(title_v[1]);
        }
        if (title_v.length > 2) {
            axisY.setTitle(title_v[2]);
        }
        if (title_v.length > 3) {
            axisZ.setTitle(title_v[3]);
        }
    }

    public GHisto getDrawer() {
        return drawer;
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        drawer.draw(canvas);
    }

    public void setTime(long time) {
        this.time = time;
    }

    public long getTime() {
        return time;
    }

    public String getTitle() {
        return title;
    }

    public Axis getAxisX() {
        return axisX;
    }

    public Axis getAxisY() {
        return axisY;
    }

    public Axis getAxisZ() {
        return axisZ;
    }

    public void setTitle(String title) {
        String[] title_v = title.split(";");
        if (title_v.length > 0) {
            this.title = title_v[0];
        } else {
            this.title = title;
        }
        if (title_v.length > 1) {
            axisX.setTitle(title_v[1]);
        }
        if (title_v.length > 2) {
            axisY.setTitle(title_v[2]);
        }
        if (title_v.length > 3) {
            axisZ.setTitle(title_v[3]);
        }
    }

    public void setNbinsx(String nbinsx) {
        try {
            setNbinsx(Integer.parseInt(nbinsx));
        } catch (Exception e) {
        }
    }

    public void setXmin(String xmin) {
        try {
            setXmin(Double.parseDouble(xmin));
        } catch (Exception e) {
        }
    }

    public void setXmax(String xmax) {
        try {
            setXmax(Double.parseDouble(xmax));
        } catch (Exception e) {
        }
    }

    public void setNbinsy(String nbinsy) {
        try {
            setNbinsy(Integer.parseInt(nbinsy));
        } catch (Exception e) {
        }
    }

    public void setYmin(String ymin) {
        try {
            setYmin(Double.parseDouble(ymin));
        } catch (Exception e) {
        }
    }

    public void setYmax(String ymax) {
        try {
            setYmax(Double.parseDouble(ymax));
        } catch (Exception e) {
        }
    }

    public void setNbinsx(int nbinsx) {
    
    }

    public void setXmax(double xmax) {
        axisX.setMax(xmax);
    }
    
    public void setXmin(double xmin) {
        axisX.setMin(xmin);
    }

    public void setFixXmax(boolean fixed) {
        axisX.setFixedMax(fixed);
    }
    
    public void setFixXmin(boolean fixed) {
        axisX.setFixedMin(fixed);
    }

    public boolean getFixXmax() {
        return axisX.isFixedMax();
    }
    
    public boolean getFixXmin() {
        return axisX.isFixedMin();
    }

    public void setFixYmax(boolean fixed) {
        axisY.setFixedMax(fixed);
    }
    
    public void setFixYmin(boolean fixed) {
        axisY.setFixedMin(fixed);
    }

    public boolean getFixYmax() {
        return axisY.isFixedMax();
    }
    
    public boolean getFixYmin() {
        return axisY.isFixedMin();
    }

    public void setFixZmax(boolean fixed) {
        axisZ.setFixedMax(fixed);
    }
    
    public void setFixZmin(boolean fixed) {
        axisZ.setFixedMin(fixed);
    }

    public boolean getFixZmax() {
        return axisZ.isFixedMax();
    }
    
    public boolean getFixZmin() {
        return axisZ.isFixedMin();
    }

    public void setNbinsy(int nbinsx) {
    
    }

    public void setYmax(double ymax) {
        axisY.setMax(ymax);
    }
    
    public void setYmin(double ymin) {
        axisY.setMin(ymin);
    }

    public int getNbinsx() {
        System.out.println("getNbinsx ");
        return axisX.getNbins();
    }

    public double getXmin() {
        System.out.println("getXmin ");
        return axisX.getMin();
    }

    public double getXmax() {
        System.out.println("getXmax ");
        return axisX.getMax();
    }

    public int getNbinsy() {
        System.out.println("getNbinsy ");
        return axisY.getNbins();
    }

    public double getYmin() {
        System.out.println("getYmin ");
        return axisY.getMin();
    }

    public double getYmax() {
        System.out.println("getYmax ");
        return axisY.getMax();
    }

    public void setAxisX(Axis axis) {
        axisX.copy(axis);
    }

    public void setAxisY(Axis axis) {
        axisY.copy(axis);
    }

    public void setAxisZ(Axis axis) {
        axisZ.copy(axis);
    }

    public double getLatestPoint() {
        return 0;
    }

    public double getPoint(int nx) {
        return 0;
    }

    public double getPointX(int nx) {
        return 0;
    }

    public double getPointY(int nx) {
        return 0;
    }

    public double getBinContent(int nx) {
        return 0;
    }

    public double getBinContent(int nx, int ny) {
        return 0;
    }

    public double getOverFlow() {
        return 0;
    }

    public double getOverFlowX(int ny) {
        return 0;
    }

    public double getOverFlowY(int nx) {
        return 0;
    }

    public double getUnderFlow() {
        return 0;
    }

    public double getUnderFlowX(int ny) {
        return 0;
    }

    public double getUnderFlowY(int nx) {
        return 0;
    }

    public void addPoint(double data) {
    }

    public void setPoint(int nx, double data) {
    }

    public void setPointX(int nx, double data) {
    }

    public void setPointY(int nx, double data) {
    }

    public void setBinContent(int nx, double data) {
    }

    public void setBinContent(int nx, int ny, double data) {
    }

    public void setOverFlow(double data) {
    }

    public void setOverFlowX(int ny, double data) {
    }

    public void setOverFlowY(int nx, double data) {
    }

    public void setUnderFlow(double data) {
    }

    public void setUnderFlowX(int ny, double data) {
    }

    public void setUnderFlowY(int nx, double data) {
    }

    public ArrayList<HyperRef> getChannelRefs() {
        return channelRefs;
    }

    public void addChannelRef(HyperRef ref) {
        channelRefs.add(ref);
    }

    public HyperRef getSelectedChannelRef() {
        return selectedChannelRef;
    }

    public abstract double getMaximum();

    public abstract double getMinimum();

    public abstract void setMaximum(double data);

    public abstract void setMinimum(double data);

    public abstract void fixMaximum(double data, boolean fix);

    public abstract void fixMinimum(double data, boolean fix);

    public abstract int getDim();

    public abstract void setMaxAndMin();

    public abstract void fill(double vx);

    public abstract void fill(double vx, double vy);

    public abstract double getMean();

    public abstract double getRMS();

}
