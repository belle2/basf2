package b2daq.dqm.io;

import b2daq.core.Reader;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.Histo1F;
import b2daq.dqm.core.Histo2F;
import b2daq.dqm.core.HistoPackage;
import b2daq.io.SocketDataReader;
import b2daq.io.SocketDataWriter;
import b2daq.io.ZlibInflater;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.zip.DataFormatException;
import javafx.application.Platform;

public class DQMListenerService extends Thread {

    public static final String VERSION = "0.0.2";
    private String host = "loclhost";
    private int port = 50100;

    public static final int FLAG_LIST = 1;
    public static final int FLAG_CONFIG = 2;
    public static final int FLAG_UPDATE = 3;

    public static final int YES = 1;
    public static final int NO = -1;

    private Socket socket = null;
    private DQMObserver observer = null;
    private ArrayList<HistoPackage> _pack_v = null;
    private ArrayList<PackageInfo> _info_v = null;
    private int _expno = 0;
    private int _runno = 0;

    public DQMListenerService() {
    }

    public void init(String host, int port, String client, DQMObserver obs) throws IOException {
        this.host = host;
        this.port = port;
        this.observer = obs;
    }

    @Override
    public void run() {
        try {
            socket = new Socket(host, port);
            SocketDataWriter socket_writer = new SocketDataWriter(socket);
            SocketDataReader socket_reader = new SocketDataReader(socket);
            ZlibInflater inflater = new ZlibInflater();
            boolean configured = false;
            while (true) {
                int flag = socket_reader.readInt();
                switch (flag) {
                    case FLAG_LIST: {
                        Platform.runLater(new Runnable() {
                            @Override
                            public void run() {
                                if (observer != null) {
                                    observer.reset();
                                }
                            }
                        });
                        _pack_v = new ArrayList<>();
                        int npacks = socket_reader.readInt();
                        System.out.println("npacks=" + npacks);
                        for (int n = 0; n < npacks; n++) {
                            String name = socket_reader.readString();
                            HistoPackage pack = new HistoPackage(name);
                            _pack_v.add(pack);
                        }
                        for (int n = 0; n < npacks; n++) {
                            socket_writer.writeByte((byte) 1);
                        }
                        int magic = socket_reader.readInt();
                        if (magic != 0x7FFF) {
                            throw new IOException("Wrong magic:" + magic);
                        }
                    }
                    break;
                    case FLAG_CONFIG: {
                        int npacks = socket_reader.readInt();
                        System.out.println("config : # of packages =" + npacks);
                        _pack_v = new ArrayList<>();
                        _info_v = new ArrayList<>();
                        for (int n = 0; n < npacks; n++) {
                            String name = socket_reader.readString();
                            HistoPackage pack = new HistoPackage(name);
                            int nhists = socket_reader.readInt();
                            System.out.println("config : # of histograms for " + name + " : " + nhists);
                            for (int i = 0; i < nhists; i++) {
                                String class_name = socket_reader.readString();
                                String dir = socket_reader.readString();
                                name = socket_reader.readString();
                                if (dir.isEmpty()) {
                                    dir = pack.getName() + ":default";
                                }
                                System.out.println("config : " + dir + " / " + name);
                                String title = socket_reader.readString();
                                int nlabelsx = socket_reader.readInt();
                                ArrayList<String> labelx = null;
                                if (nlabelsx > 0) {
                                    labelx = new ArrayList<>();
                                    for (int ix = 0; ix < nlabelsx; ix++) {
                                        labelx.add(socket_reader.readString());
                                    }
                                }
                                int nbinsx = socket_reader.readInt();
                                double xmin = socket_reader.readDouble();
                                double xmax = socket_reader.readDouble();
                                //System.out.println("config : histogram " + name);
                                if (class_name.contains("TH1")) {
                                    Histo1F h = new Histo1F(name, title, nbinsx, xmin, xmax);
                                    if (labelx != null) {
                                        h.getAxisX().setLabels(labelx);
                                    }
                                    pack.addHisto(dir, h);
                                } else if (class_name.contains("TH2")) {
                                    int nlabelsy = socket_reader.readInt();
                                    ArrayList<String> labely = null;
                                    if (nlabelsx > 0) {
                                        labely = new ArrayList<>();
                                        for (int iy = 0; iy < nlabelsy; iy++) {
                                            labely.add(socket_reader.readString());
                                        }
                                    }
                                    int nbinsy = socket_reader.readInt();
                                    double ymin = socket_reader.readDouble();
                                    double ymax = socket_reader.readDouble();
                                    Histo2F h = new Histo2F(name, title, nbinsx, xmin, xmax,
                                                            nbinsy, ymin, ymax);
                                    if (labelx != null) {
                                        h.getAxisX().setLabels(labelx);
                                    }
                                    if (labely != null) {
                                        h.getAxisY().setLabels(labely);
                                    }
                                    pack.addHisto(dir, h);
                                }
                                int magic = socket_reader.readInt();
                                if (magic != 0x7FFF) {
                                    throw new IOException("Wrong magic:" + magic);
                                }
                            }
                            int magic = socket_reader.readInt();
                            if (magic != 0x7FFF) {
                                throw new IOException("Wrong magic:" + magic);
                            }
                            _pack_v.add(pack);
                            PackageInfo info = new PackageInfo();
                            _info_v.add(info);
                        }
                        int magic = socket_reader.readInt();
                        if (magic != 0x7FFF) {
                            throw new IOException("Wrong magic:" + magic);
                        }
                        Platform.runLater(new Runnable() {
                            @Override
                            public void run() {
                                observer.init(_pack_v, _info_v);
                            }
                        });
                        configured = true;
                    }
                    break;
                    case FLAG_UPDATE: {
                        int expno = socket_reader.readInt();
                        int runno = socket_reader.readInt();
                        int stateno = socket_reader.readInt();
                        Platform.runLater(new Runnable() {
                            @Override
                            public void run() {
                                observer.update(expno, runno, stateno);
                            }
                        });
                        if (_expno != expno || _runno != runno) {
                            _expno = expno;
                            _runno = runno;
                            for (HistoPackage pack : _pack_v) {
                                pack.reset();
                            }
                        }
                        while (true) {
                            int n = socket_reader.readInt();
                            if (n < 0) {
                                break;
                            }
                            inflater.readBuffer(socket_reader);
                            HistoPackage pack = _pack_v.get(n);
                            readContents(inflater, pack);
                        }
                        Platform.runLater(new Runnable() {
                            @Override
                            public void run() {
                                observer.update(_expno, _runno, 0);
                            }
                        });
                    }
                    break;
                }
            }
        } catch (IOException | DataFormatException e) {
            e.printStackTrace();
            try {
                socket.close();
            } catch (IOException e1) {
            }
            Platform.runLater(new Runnable() {
                @Override
                public void run() {
                    if (observer != null) {
                        observer.reset();
                    }
                }
            });
        }
    }

    public void close() {
        try {
            socket.close();
        } catch (IOException ex) {
            Logger.getLogger(DQMListenerService.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    private void readContents(Reader reader, HistoPackage pack) throws IOException {
        String name = reader.readString();
        if (!name.matches(pack.getName())) {
            throw new IOException("Wrong package name :" + name + " for " + pack.getName());
        }
        int nhists = reader.readInt();
        for (int i = 0; i < nhists; i++) {
            name = reader.readString();
            Histo h = (Histo) pack.getHisto(i);
            Histo h_diff = (Histo) pack.getHisto(h.getName() + ":diff");
            Histo h_tmp = (Histo) pack.getHisto(h.getName() + ":tmp");
            if (h.getDim() == 1) {
                for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
                    double v = reader.readFloat();
                    h.setBinContent(nx, v);
                }
            } else if (h.getDim() == 2) {
                for (int ny = 0; ny < h.getAxisY().getNbins(); ny++) {
                    for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
                        double v = reader.readFloat();
                        h.setBinContent(nx, ny, v);
                    }
                }
            }
            if (h_tmp != null) {
                double entries = h_tmp.getEntries();
                if (h.getEntries() != entries) {
                    h_diff.reset();
                    h_diff.add(h_tmp, -1);
                    h_diff.add(h);
                    h_tmp.reset();
                    h_tmp.add(h);
                } else {
                    h_diff.reset();
                    h_tmp.reset();
                    h_tmp.add(h);
                }
            } else {
            }
            int magic = reader.readInt();
            if (magic != 0x7FFF) {
                throw new IOException(h.getName() + ": Wrong magic:" + magic);
            }
        }
        int magic = reader.readInt();
        if (magic != 0x7FFF) {
            throw new IOException("Wrong magic:" + magic);
        }
    }
}
