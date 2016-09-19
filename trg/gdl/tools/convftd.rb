#!/usr/bin/env ruby

def usage
  print <<EOF
  
  #{File.basename($0)} [-vhfp] ftd_v.vv[...].vhd

  generate ftd_v.vv.inp, ftd_v.vv.oup, ftd_v.vv.alg, ftd_v.vv.cc files.

      -h: this help
      -v: verbose
      -f: force overwrite
      -p: print. not generate files.

  Supported:
      > upto 3
      = upto 3
      < not supported

  Required notation in ftd.vhd:

    -- Inputs    <= Start of inp bits
      signal inp_name std_logic[_vector(X downto Y)];
      signal ...
    -- Outputs   <= Start of oup bits
      signal oup_name std_logic;
      signal ...
    begin        <= End of oup bits
      ...

  bits are defined in this order from bit 0.
    
EOF
  exit
end

$verbose = false
$vhdl = nil
$force = nil
$print_flg = nil
until ARGV.empty?
  arg = ARGV.shift
  case arg
  when /^-h$/
    usage
  when /^\d+$/
    $this_exp = arg.to_i
  when /^-p$/
    $print_flg = true
  when /^-f$/
    $force = true
  when /^-v$/
    $verbose = true
  else
    if /.vhd$/ =~ arg and File.stat(arg).file?
      $vhdl = arg
    end
  end
end

usage unless /ftd_([0-9\.]{4}).*\.vhd/ =~ $vhdl
$ftd_version = $1.sub(/\./, "_")

$io = nil
$inp_ary = []
$alg_hash = Hash.new #
$oup_hash = Hash.new # k=oupname, v=obit.to_i
$inp_hash = Hash.new # {[["n_t3_full"], [0, 1, 2]], [[n_t3_short], .]}

$oup_fname = $vhdl.sub(/\.vhd$/, ".oup")
$inp_fname = $vhdl.sub(/\.vhd$/, ".inp")
$alg_fname = $vhdl.sub(/\.vhd$/, ".alg")
$ccc_fname = $vhdl.sub(/\.vhd$/, ".cc")

def gt(minbit, nbit, np)
  # n_klm > 0 -> gt(54, 3, 0)
  # returns [54, 55, 56]
  result = []
  if np == 0 # np>0
    for i in 0...nbit
      result.push (minbit+i).to_s
    end
  elsif np == 1 # np>1
    for i in 1...nbit
      result.push (minbit+i).to_s
    end
  elsif np == 2 # np>2
    result.push "#{minbit} * #{minbit+1}"
    for i in 2...nbit
      result.push (minbit+i).to_s
    end
  elsif np == 3 # np>3
    for i in 2...nbit
      result.push (minbit+i).to_s
    end
  end
  if result.empty?
    puts "error: gt(#{minbit}), nbit(#{nbit}), np(#{np})"
    exit
  end
  result
end

def eql(minbit, nbit, np)
  result = ""
  if np == 0
    result += "!#{minbit}"
    for i in 1...nbit
      result += " * !#{minbit+i}"
    end
  elsif np == 1
    result += "#{minbit}"
    for i in 1...nbit
      result += " * !#{minbit+i}"
    end
  elsif np == 2
    result += "!#{minbit} * #{minbit}"
    for i in 1...nbit
      result += " * !#{minbit+i}"
    end
  end
  if result.empty?
    puts "error: eql(#{minbit}), nbit(#{nbit}), np(#{np})"
    exit
  end
  [result]
end

# write .alg, fill alg_hash
def alg(oupname, lg)

  # example
  # zzx <= '1' when (n_t3_full>1) and
  #                 (n_t3_short>0) and
  #                 cdc_open45='1' and
  #                 (veto='0')     else '0';  --  0

  lg = lg.sub(/.*when\s*/, "").sub(/\s*else.*/, "").sub(/;.*/, "")
  # (n_t3_full>1) and (n_t3_short>0) and cdc_open45='1' and (veto='0')

  lgg = lg.gsub(/[\(\)]/, " ").gsub(/"/, "").split(/\s+and\s+/)
  # ["n_t3_full>1", "n_t3_short>0", "cdc_open45='1'",  "veto='0'"]

  # inpname='1' -> inpname
  # inpname='0' -> !inpname
  lgg.collect!{|xx|
    if /\s*='1'/ =~ xx
      $`
    elsif /\s*='0'/ =~ xx
      "!" + $`
    else
      xx
    end}
  # ["n_t3_full>1", "n_t3_short>0", "cdc_open45",  "!veto"]

  lggg = []
  lgg.each{|eqn|
    if /^\s*(.*)\s*>\s*(\d+)\s*$/ =~ eqn # greater than
      # for n_t3_full>1
      inpname = $1 # n_t3_full
      np = $2.to_i # 1
      lggg.push gt($inp_hash[inpname][0],   # 1st bit of inpname
                   $inp_hash[inpname].size, # bit width of inpname
                   np)                      # greater than np
    elsif /^\s*(.*)\s*=\s*(\d+)\s*$/ =~ eqn # equal
      inpname = $1
      np = $2.to_i
      lggg.push eql($inp_hash[inpname][0],  # 1st bit of inpname
                    $inp_hash[inpname].size,# bit width of inpname
                    np)                     # equal to np
    elsif /</ =~ eqn # less than
      puts "lt not supported: #{eqn}"
      exit
    else # no <=>. just bitname or !bitname.
      inpname = eqn.sub(/!/,"")
      if /!/ =~ eqn # ='0'
        lggg.push ["!#{$inp_hash[inpname][0]}"]
      else          # ='1'
        lggg.push ["#{$inp_hash[inpname][0]}"]
      end
    end
  }
  obit = $oup_hash[oupname]
  if $verbose and !$print_flg
    printf("%11s %2d : ( %s )\n", oupname, obit, lggg)
  end

  
  nterms = 1
  ans = []
  lggg.each{|aaa| nterms = nterms * aaa.length}
  nterms.times{ans.push []}

  # nterms = 6 for [[a, b, c], [d, e], [f]]
  # lggg.length = 3
  # lggg[0].length = 3
  # lggg[1].length = 2
  # lggg[2].length = 1
  # i=0 [a    ], [b    ], [c    ], [a    ], [b    ], [c    ]
  # i=1 [a*d  ], [b*e  ], [c*d  ], [a*e  ], [b*d  ], [c*e  ]
  # i=2 [a*d*f], [b*e*f], [c*d*f], [a*e*f], [b*d*f], [c*e*f]
  for i in 0...lggg.length
    for j in 0...lggg[i].length
      k = j
      while k < nterms 
        ans[k].push lggg[i][j]
        k += lggg[i].length
      end
    end
  end
  ans_ans = []
  ans.each{|bbb| ans_ans.push bbb.join(" * ")}

  # write .alg
  $io.puts "#{obit} : ( #{ans_ans.join(" + ")} )"

  $alg_hash[obit] = ans_ans

end
      
def fill_signals

  inp_region = false
  oup_region = false
  obit = 0
  open($vhdl).read.split(/\n/).each{|l|
    if /--\s*Inputs/ =~ l
      inp_region = true
    elsif /--\s*Outputs/ =~ l
      inp_region = false
      oup_region = true
    elsif oup_region and /^\s+begin/ =~ l
      oup_region = false
    end
    
    # oup bits are always std_logic
    if oup_region and /^\s*signal\s+([\w]+)\s*:\s*std_logic/i =~ l
      $oup_hash[$1] = obit
      obit += 1
    end
    
    # inp bits can be std_logic_vector
    if inp_region and /^\s*signal\s+([\w]+)\s*:\s*std_logic/i =~ l
      bit_name = $1
      logic = $' # logic || logic_vector(n downto m)
      if /vector\(\s*(\d+)\s+downto\s+(\d+)\s*\)/ =~ logic
        bit_length = $1.to_i + 1
      else
        bit_length = 1
      end
      $inp_ary.push [bit_name, bit_length]
    end

  }
end

def get_io(fname)
  if $print_flg 
    $io = $stdout
  elsif File.exist?(fname) and !$force
    print "File #{fname} exists. Overwrite? (Y/n): "
    yn = $stdin.gets
    if /^Y*$/ =~ yn
      $io = open(fname, "w")
    else
      exit
    end
  else
    $io = open(fname, "w")
  end
  $io
end

def write_inp # write .inp
  $io = get_io($inp_fname)
  bit = 0
  $inp_ary.each{|a|
    1.step(a[1], 1){|s|
    if a[1]>1
      $io.puts "#{bit}   #{a[0]}#{s-1}"
    else
      $io.puts "#{bit}   #{a[0]}"
    end
    bit += 1
    }
  }
  $io.close unless $print_flg
end
      
def write_oup # write .oup
  $io = get_io($oup_fname)
  $oup_hash.each{|k,v| $io.puts "#{v}   #{k}"}
  $io.close unless $print_flg
end

def fill_inp_hash
  # from $inp_ary=[["n_t3_full", 3], ["n_t3_short", 3], ["n_t2_full", 3]...]
  # to $inp_hash=[["n_t3_full", [0,1,2]], ["n_t3_short", [3,4,5]], ...]
  bit = 0
  $inp_ary.each{|a|
    1.step(a[1], 1){|s|
      if $inp_hash.has_key?(a[0])
        $inp_hash[a[0]].push bit
      else
        $inp_hash[a[0]] = [bit]
      end
      bit += 1
    }
  }
end

def write_alg # write alg and fill alg_hash calling alg()
  $io = get_io($alg_fname)
  logic_region = false
  one_line = true
  logic = ""
  bit_name = ""
  open($vhdl).read.split(/\n/).each{|l|
    if one_line
      if /^\s*([\w]+)\s*<=\s*/ =~ l
        bit_name = $1
        logic = $'.chomp
        if $oup_hash.has_key?(bit_name)
          if /;/ =~ logic
            alg(bit_name, logic)
          else
            one_line = false
            next
          end
        end
      end
    else
      logic += l.chomp
      if /;/ =~ logic
        one_line = true
        alg(bit_name, logic)
      end
    end
  }
  $io.close unless $print_flg
end

def write_cc # write .cc using alg_hash

  $io = get_io($ccc_fname)
  $io.print <<EOF
namespace Belle2 {
  void #{$ftd_version}(bool * b, bool * i) {
EOF
  $alg_hash.keys.sort.each{|obit|

    $io.printf("    b[%d] = ", obit)

    terms = $alg_hash[obit]
    for i in 0...terms.size
      
      term = terms[i].gsub(/\*/, "&&").scan(/\S+/).map{|word|
        if /(\d+)/ =~ word
          b = $1.to_i
          word.sub!(/#{b}/, "i[#{sprintf("%2d",b)}]")
        end
        word
      }.join(" ")
      
      $io.printf("%s(%s)%s\n",
                 (i == 0 ? "":(obit<10 ? " "*11:" "*12)),
                 term,
                 (i == terms.size-1 ? ";":" ||"))
      
    end # for i in 0...terms.size
  }
  $io.print <<EOF
  }
}
EOF
  $io.close unless $print_flg

end
  
fill_signals # fill inp_ary, oup_hash
write_inp
write_oup
fill_inp_hash
write_alg  # write .alg and fill alg_hash
p $alg_hash  if $verbose and !$print_flg
write_cc  # write .cc using alg_hash
      

