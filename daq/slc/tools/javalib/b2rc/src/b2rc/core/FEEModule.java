package b2rc.core;

import java.util.ArrayList;

public class FEEModule extends Module {

	private String _type;
	private ArrayList<Register> _reg_v = new ArrayList<Register>();

	public FEEModule() {
	}

	public FEEModule(String type) {
		_type = type;
	}

	public String getType() {
		return _type;
	}

	public ArrayList<Register> getRegisters() {
		return _reg_v;
	}

	public Register getRegister(int index) {
		return _reg_v.get(index);
	}

	public Register getRegister(String name) {
		for (Register reg : _reg_v) {
			if (reg.getName() == name)
				return reg;
		}
		return null;
	}

	public void setType(String type) {
		_type = type;
	}

	public void addRegister(Register reg) {
		for ( int i = 0; i < _reg_v.size(); i++ ) {
			if ( reg.getName().matches(_reg_v.get(i).getName()) ) {
				return;
			}
		}
		_reg_v.add(reg);
	}

	public void setRegister(int index, Register reg) {
		_reg_v.set(index, reg);
	}

	public void setRegisters(ArrayList<Register> reg_v) {
		_reg_v = new ArrayList<Register>();
		for ( Register reg : reg_v ) {
			Register reg_new = new Register();
			reg_new._address = reg._address;
			reg_new._name = reg._name;
			reg_new._size = reg._size;
			reg_new._value_v = new ArrayList<Integer>();
			for (Integer value : reg._value_v ) {
				reg_new._value_v.add(value);
			}
			_reg_v.add(reg_new);
		}
	}

	public Register createRegister() {
		return new Register();
	};

	public int count() {
		int c = 0;
		for (Register reg : _reg_v) {
			c += reg.count();
		}
		return c;
	}

	@Override
	public String getSQLFields() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLFields());
		for ( int i = 0; i < _reg_v.size(); i++ ) {
			for ( int ch = 0; ch < _reg_v.get(i).length(); ch++ ) {
				buf.append(", " + _reg_v.get(i).getName()+"_"+ch + " int");
			}
		}
		return buf.toString();
	}

	@Override
	public String getSQLLabels() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLLabels());
		for ( int i = 0; i < _reg_v.size(); i++ ) {
			for ( int ch = 0; ch < _reg_v.get(i).length(); ch++ ) {
				buf.append(", " + _reg_v.get(i).getName()+"_"+ch);
			}
		}
		return buf.toString();
	}

	@Override
	public String getSQLValues() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLValues());
		for ( int i = 0; i < _reg_v.size(); i++ ) {
			for ( int ch = 0; ch < _reg_v.get(i).length(); ch++ ) {
				buf.append(", " + _reg_v.get(i).getValue(ch));
			}
		}
		return buf.toString();
	}

	@Override
	public String getTag() {
		return _type;
	}

	public class Register {

		private String _name;
		private int _address;
		private ArrayList<Integer> _value_v = new ArrayList<Integer>();
		private int _size;

		public Register() {
			_size = 1;
		}

		public String getName() {
			return _name;
		}

		public int getAddress() {
			return _address;
		}

		public int getValue(int ch) {
			return _value_v.get(ch);
		}

		public ArrayList<Integer> getValues() {
			return _value_v;
		}

		public int length() {
			return _value_v.size();
		}

		public int getSize() {
			return _size;
		}

		public void setName(String name) {
			_name = name;
		}

		public void setAddress(int address) {
			_address = address;
		}

		public void setValues(ArrayList<Integer> value_v) {
			_value_v = value_v;
		}

		public void setValues(int value) {
			for (int ch = 0; ch < _value_v.size(); ch++)
				_value_v.set(ch, value);
		}

		public void setValue(int ch, int value) {
			_value_v.set(ch, value);
		}

		public void setLength(int length) {
			if (length > 0) {
				_value_v = new ArrayList<Integer>();
				for (int i = 0; i < length; i++) {
					_value_v.add(0);
				}
			}
		}

		public void setSize(int size) {
			if (size > 0)
				_size = size;
		}

		public void set(String name, int address, int length, int size) {
			_name = name;
			_address = address;
			setLength(length);
			setSize(size);
		}

		public int count() {
			return 4 * length();
		}

	}

}
