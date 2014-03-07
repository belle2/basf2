package b2dqm.io;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

import b2daq.core.NSMData;
import b2daq.graphics.GShape;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2dqm.core.Histo1;
import b2dqm.core.TimedGraph1;
import b2dqm.graphics.GEnumLabel;
import b2dqm.graphics.GMonObject;
import b2dqm.graphics.GRCStateLabel;

public class NSMDataManager {
	
	private ArrayList<DataHisto> _histo_v = new ArrayList<DataHisto>();
	private HashMap<String, NSMData> _data_m = new HashMap<String, NSMData>();
	
	public NSMDataManager() {
	}
	
	public boolean hasData(String name) {
		return _data_m.containsKey(name);
	}
	
	public NSMData getData(String name) {
		return _data_m.get(name);
	}
	
	public void add(NSMData data) {
		_data_m.put(data.getName(), data);
	}
	
	public void addHisto(GMonObject histo, String dataname) {
		DataHisto h = new DataHisto();
		h._histo = histo;
		parse(h, dataname);
		_histo_v.add(h);
	}
	
	public void addShape(GShape shape, String dataname, String format) {
		DataHisto h = new DataHisto();
		h._shape = shape;
		parse(h, dataname);
		if (h._color_v != null) {
			if (h._format.length() > 0) {
				format = h._format;
			}
			String [] s_enum_v = format.split(",");
			for (String s_enum: s_enum_v) {
				String [] s_v = s_enum.split("\\:");
				if (s_v.length > 4) {
					int i = Integer.parseInt(s_v[0]);
					ColorInfo color = new ColorInfo();
					color.text = s_v[1].replace("\"", "");
					color.linecolor = new HtmlColor(s_v[2]);
					color.fillcolor = new HtmlColor(s_v[3]);
					color.fontcolor = new HtmlColor(s_v[4]);
					h._color_v.put(i, color);
				}
			}
		} else {
			h._format = format;
		}
		_histo_v.add(h);
	}
	
	public void addMessage(UpdatableMessage message, String dataname, String format) {
		DataHisto h = new DataHisto();
		h._message = message;
		h._format = format;
		parse(h, dataname);
		_histo_v.add(h);
	}
	
	public void update() {
		for (DataHisto h : _histo_v) {
			h.update(_data_m);
		}
	}

	private void parse(DataHisto h, String dataname) {
		String [] str_v = dataname.split(",");
		for (String str : str_v) {
			String [] name_v = str.split("\\.");
			UpdateInfo info = new UpdateInfo();
			boolean use_timestamp = name_v[0].startsWith("Timestamp");
			if (use_timestamp) {
				name_v[0] = name_v[0].replace("Timestamp(", "").replace(")", "");
				info._timeformat = new SimpleDateFormat("YYYY/MM/dd hh:mm:ss");
			} else if (name_v[0].startsWith("Enum")) {
				name_v[0] = name_v[0].replace("Enum(", "").replace(")", "");
				h._color_v = new HashMap<Integer,ColorInfo>();
			} else if (name_v[0].startsWith("RunState")) {
				name_v[0] = name_v[0].replace("RunState(", "").replace(")", "");
			} else if (name_v[0].startsWith("ColorCode")) {
				info._color = new HtmlColor();
				name_v[0] = name_v[0].replace("ColorCode(", "").replace(")", "");
			}
			info._dataname = name_v[0];
			int index = 0;
			if (name_v.length > 1) {
				if (name_v[1].contains("[")) {
					String [] s_v = name_v[1].split("\\[");
					name_v[1] = s_v[0];
					index = Integer.parseInt(s_v[1].split("\\]")[0]);
				}
				info._paraname = name_v[1].replace(")", "");
			}
			info.index = index;
			h._info_v.add(info);
		}
	}
	
	private class UpdateInfo {
		public String _dataname;
		public String _paraname;
		public int index = 0;
		public SimpleDateFormat _timeformat = null;
		public HtmlColor _color = null;
	}
	
	private class ColorInfo {
		public String text = "";
		public HtmlColor linecolor;
		public HtmlColor fillcolor;
		public HtmlColor fontcolor;
	}
	
	private class DataHisto {
		public GMonObject _histo = null;
		public GShape _shape = null;
		public UpdatableMessage _message = null;
		public String _format = "";
		public ArrayList<UpdateInfo> _info_v = new ArrayList<UpdateInfo>();
		public HashMap<Integer,ColorInfo> _color_v = null;
		private long _time = 0;
		
		public void update(HashMap<String, NSMData> data_m) {
			for (UpdateInfo info : _info_v) {
				if (!data_m.containsKey(info._dataname)) return;
				NSMData data = data_m.get(info._dataname);
				if (!data.hasValue(info._paraname)) return;
			}
			try {
				if (_histo != null) {
					long time = new Date().getTime();
					for (UpdateInfo info : _info_v) {
						NSMData data = data_m.get(info._dataname);
						if (_histo.get().getDataType().contains("TG")) {
							TimedGraph1 gr = (TimedGraph1)(_histo.get());
							if (_time > 0)
								gr.addPoint(_time, data.getValue(info._paraname, info.index));
							gr.addPoint(time, data.getValue(info._paraname, info.index));
							gr.setUpdateTime(time);
						} else if (_histo.get().getDataType().contains("H1")) {
							Histo1 h = (Histo1)(_histo.get());
							h.fill(data.getValue(info._paraname, info.index));
						}
					}
					_time = time;
				} else if (_shape != null) {
					if (_shape instanceof GText) {
						GText label = (GText)_shape;
						label.setText(toString(data_m));
					} else if (_shape instanceof GRCStateLabel) {
						if (_info_v.size() > 0) {
							UpdateInfo info = _info_v.get(0);
							GRCStateLabel label = (GRCStateLabel)_shape;
							NSMData data = data_m.get(info._dataname);
							label.setState(data.getInt(info._paraname, info.index));
						}
						return;
					} else if (_shape instanceof GEnumLabel) {
						if (_info_v.size() > 0) {
							UpdateInfo info = _info_v.get(0);
							NSMData data = data_m.get(info._dataname);
							GEnumLabel label = (GEnumLabel)_shape;
							int i = data.getInt(info._paraname, info.index);
							ColorInfo color = _color_v.get(i);
							if (color != null) {
								label.getText().setText(color.text);
								label.getRect().setLineColor(color.linecolor);
								label.getRect().setFillColor(color.fillcolor);
								label.setFontColor(color.fontcolor);
							}
						}
					}
				} else if (_message != null) {
					_message.setText(toString(data_m));
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		private String toString(HashMap<String, NSMData> data_m) {
			try {
				Object [] obj_v = new Object[_info_v.size()];
				String format = _format;
				for (int i = 0; i < _info_v.size(); i++) {
					NSMData data = data_m.get(_info_v.get(i)._dataname);
					String name = _info_v.get(i)._paraname;
					NSMData.ParamInfo info = data.getParam(name);
					int type = info.type;
					int index = _info_v.get(i).index;
					if (type == NSMData.CHAR || type == NSMData.BYTE8) {
						obj_v[i] = data.getChar(name, index);
					} else if (type == NSMData.INT16 || type == NSMData.UINT16) {
						obj_v[i] = data.getShort(name, index);
					} else if (type == NSMData.INT32 || type == NSMData.UINT32) {
						if (_info_v.get(i)._timeformat != null) {
							obj_v[i] = _info_v.get(i)._timeformat.format(new Date(data.getInt(name, index)*1000));
						} else if (_info_v.get(i)._color != null){
							int value = data.getInt(name, index);
							int red = value >> 16 & 0xFF;
							int green = value >> 8 & 0xFF;
							int blue = value & 0xFF;
							_info_v.get(i)._color.setRGB(red, green, blue);
							obj_v[i] = _info_v.get(i)._color.toString();
						} else {
							obj_v[i] = data.getInt(name, index);
						}
					} else if (type == NSMData.INT64 || type == NSMData.UINT64) {
						if (_info_v.get(i)._timeformat != null) {
							obj_v[i] = _info_v.get(i)._timeformat.format(new Date(data.getLong(name, index)*1000));
						} else {
							obj_v[i] = data.getLong(name, index);
						}
					} else if (type == NSMData.FLOAT) {
						obj_v[i] = data.getFloat(name, index);
					} else if (type == NSMData.DOUBLE) {
						obj_v[i] = data.getDouble(name, index);
					} 
				}
				if (format.length() > 0) {
					return String.format(format, obj_v);
				} else {
					StringBuffer ss = new StringBuffer();
					for (Object obj : obj_v) {
						ss.append(obj);
					}
					return ss.toString();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
			return "Internal error";
		}
	}
	
}
