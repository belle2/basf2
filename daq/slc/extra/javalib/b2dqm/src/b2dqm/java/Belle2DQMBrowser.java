package b2dqm.java;

import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;

import b2daq.java.io.SocketDataReader;
import b2daq.java.io.SocketDataWriter;
import b2daq.java.io.ZlibInflater;
import b2daq.java.ui.JavaEntoryPoint;
import b2daq.java.ui.LoginPanel;
import b2dqm.core.Histo;
import b2dqm.core.Histo1F;
import b2dqm.core.Histo2F;
import b2dqm.core.HistoPackage;
import b2dqm.core.PackageInfo;
import b2dqm.java.ui.DQMMainFrame;
import b2dqm.java.ui.PackageSelectPanel;

public class Belle2DQMBrowser extends JavaEntoryPoint {

	public static final String VERSION = "0.0.2";
	static private String __host = "loclhost";
	static private int __port = 50100;

	public static final int FLAG_LIST = 1;
	public static final int FLAG_CONFIG = 2;
	public static final int FLAG_UPDATE = 3;
	
	public static final int YES = 1;
	public static final int NO = -1;
	
	private Socket _socket = null;
	private DQMMainFrame _frame = null;
	private ArrayList<HistoPackage> _pack_v = null;
	private ArrayList<PackageInfo> _info_v = null;
	private int _expno = 0;
	private int _runno = 0;
	
	public Belle2DQMBrowser() {
	}

	public void init(String host, int port, String client) {
		__host = host;
		__port = port;
	}

	public void run() {
		try {
			_socket = new Socket(__host, __port);
			SocketDataWriter socket_writer = new SocketDataWriter(_socket);
			SocketDataReader socket_reader = new SocketDataReader(_socket);
			ZlibInflater inflater = new ZlibInflater();
			while (true) {
				int flag = socket_reader.readInt();
				switch ( flag ) {
				case FLAG_LIST : {
					if ( _frame != null) _frame.dispose();
					_frame = new DQMMainFrame();		
					_pack_v = new ArrayList<HistoPackage>();
					int npacks = socket_reader.readInt();
					for ( int n = 0; n < npacks; n++ ) {
						String name = socket_reader.readString();
						HistoPackage pack = new HistoPackage(name);
						_pack_v.add(pack);
					}
					ArrayList<Boolean> monitored_v = PackageSelectPanel.showPane(_pack_v);
					for ( int n = 0; n < monitored_v.size(); n++ ) {
						socket_writer.writeByte(monitored_v.get(n)?(byte)1:0);
					}
					int magic = socket_reader.readInt();
					if (magic != 0x7FFF) {
						throw new Exception("Wrong magic:" + magic);
					}
				}
				break;
				case FLAG_CONFIG : {
					int npacks = socket_reader.readInt();
					System.out.println("config : # of packages ="+npacks);
					_pack_v = new ArrayList<HistoPackage>();
					_info_v = new ArrayList<PackageInfo>();
					for (int n = 0; n < npacks; n++) {
						String name = socket_reader.readString();
						HistoPackage pack = new HistoPackage(name);
						int nhists = socket_reader.readInt();
						System.out.println("config : # of histograms for " + name + " : " + nhists);
						for (int i = 0; i < nhists; i++) {
							String class_name = socket_reader.readString();
							name = socket_reader.readString();
							String title = socket_reader.readString();
							int nbinsx = socket_reader.readInt();
							double xmin = socket_reader.readDouble();
							double xmax = socket_reader.readDouble();
							System.out.println("config : histogram " + name);
							if (class_name.contains("TH1")) {
								pack.addHisto(new Histo1F(name, title, nbinsx, xmin, xmax));
							} else if (class_name.contains("TH2")) {
								int nbinsy = socket_reader.readInt();
								double ymin = socket_reader.readDouble();
								double ymax = socket_reader.readDouble();
								pack.addHisto(new Histo2F(name, title, nbinsx, xmin, xmax,
																nbinsy, ymin, ymax));
							}
							int magic = socket_reader.readInt();
							if (magic != 0x7FFF) {
								throw new Exception("Wrong magic:" + magic);
							}
						}
						int magic = socket_reader.readInt();
						if (magic != 0x7FFF) {
							throw new Exception("Wrong magic:" + magic);
						}
						_pack_v.add(pack);
						PackageInfo info = new PackageInfo();
						_info_v.add(info);
					}
					int magic = socket_reader.readInt();
					if (magic != 0x7FFF) {
						throw new Exception("Wrong magic:" + magic);
					}
					_frame.init(_pack_v, _info_v);
				}
				break;
				case FLAG_UPDATE : {
					int size = socket_reader.readInt();
					inflater.setBufferSize(size);
					inflater.readBuffer(socket_reader);
					int expno = inflater.readInt();
					int runno = inflater.readInt();
					int stateno = inflater.readInt();
					_frame.getStatusPnale().update(expno, runno, stateno);
					System.out.println("update : expno = " + expno + " runno = "+runno+" state = "+stateno);
					int npacks = inflater.readInt();
					if (_expno != expno || _runno != runno) {
						_expno = expno;
						_runno = runno;
						for (int n = 0; n < npacks; n++) {
							_pack_v.get(n).reset();
						}							
					}
					for (int n = 0; n < npacks; n++) {
						HistoPackage pack = _pack_v.get(n);
						String name = inflater.readString();
						if (!name.matches(pack.getName())) {
							throw new Exception("Wrong package name :" + name + " for " + pack.getName());
						}
						System.out.println("update : package = " + pack.getName());
						int nhists = inflater.readInt();
						for (int i = 0; i < nhists; i++) {
							name = inflater.readString();
							Histo h = (Histo)pack.getHisto(i);
							Histo h_diff = (Histo)pack.getHisto(h.getName()+"_diff");
							Histo h_tmp = (Histo)pack.getHisto(h.getName()+"_tmp");
							if (!name.matches(h.getName())) {
								throw new Exception("Wrong histo name :" + name + " for " + h.getName());
							}
							if (h.getDim() == 1) {
								for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
									h.setBinContent(nx, inflater.readFloat());
								}
							} else if (h.getDim() == 2) {
								for (int ny = 0; ny < h.getAxisY().getNbins(); ny++) {
									for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
										h.setBinContent(nx, ny, inflater.readFloat());
									}
								}
							} 
							double entries = h_tmp.getEntries();
							if (entries > 0 && h.getEntries() != entries) {
								h_diff.reset();
								h_diff.add(h_tmp, -1);
								h_diff.add(h);
								h_tmp.reset();
								h_tmp.add(h);
							} else {
								h_tmp.reset();
								h_tmp.add(h);
							}
							int magic = inflater.readInt();
							if (magic != 0x7FFF) {
								throw new Exception("Wrong magic:" + magic);
							}
						}
						int magic = inflater.readInt();
						if (magic != 0x7FFF) {
							throw new Exception("Wrong magic:" + magic);
						}
					}	
					_frame.update();
				}
				break;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			try {
				_socket.close();
			} catch (IOException e1) {}
			if (_frame != null)_frame.dispose();
			if ( !LoginPanel.showLoginDialog(_frame, __host, __port, 
					"Belle-II DQM browser Login", "System ERROR! Tring reconnect?",
					new Belle2DQMBrowser()) ) {
				System.exit(-1);
			}
		}
	}
	
	public static void main(String[] argv) {
		Belle2DQMBrowser gui = new Belle2DQMBrowser();
		String host = (argv.length > 0)? argv[0]:"localhost";//"130.87.227.252";//
		int port = (argv.length > 1)? Integer.parseInt(argv[1]):50100;
		gui.init(host, port, "");
		gui.run();
	}
}
