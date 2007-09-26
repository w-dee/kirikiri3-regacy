#!/usr/bin/env ruby


# risseOpCodes.txt から定義を生成する


# 入力ファイルを開いて一行ずつ処理をする
errorcount = 0
linecount = 0
On_defs = {'-' => '(tVMInsnInfo::tInsnFlag)(0)'}
Eff_defs = {}
defs = []
File.open(ARGV[0]).readlines.each do |line|
	linecount += 1
	# コメントの除去
	line.gsub!(/\/\/.*/, '')
	# 空白のみになったらその行は処理しない
	next if line =~ /^\s*$/
	# 定義コメントの取り出し
	defcomment = ''
	line.gsub!(/\#!<(.*)/) { defcomment=$1; '' }
	# 行先頭と最後の空白の除去
	line.gsub!(/^\s+/, '')
	line.gsub!(/\s+$/, '')
	# パターンに従って分岐
	if    line =~ /^-On\s+([^\s])\s+([^\s]+)$/
		# On_defs
		On_defs[$1] = $2
	elsif line =~ /^-Eff\s+([^\s])\s+([^\s]+)$/
		# Eff_defs
		Eff_defs[$1] = $2
	elsif line =~ /(\w+)\s+(\w+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)$/
		# defs
		defs << { :long_id => $1, :mnemonic => $2,
				:On => $3, :member_name => $4, :Eff => $5, :def_comment => defcomment }
	else
		STDERR.puts "Unrecognized definition at line #{linecount}\n"
		errorcount += 1
	end

end

exit if errorcount != 0

#-----------------------------------------------------------------------

File.open(ARGV[1], "w") do |file|
	# ヘッダを書き出す
file.puts <<EOS
// generated by tools/opecodes.rb
// do not edit this file by hand

EOS

	# 列挙型を書き出す
	enumcount = 0
	file.puts "//! @brief オペレーションコードの列挙型"
	file.puts "enum tOpCode {"
	defs.each do |item|
#		file.printf "/* %3d */ ", enumcount
		file.puts "oc#{item[:long_id]} /*!<#{item[:def_comment]} */,"
		enumcount += 1
	end

	file.puts "};"

	# 演算子メンバ名へのエイリアスを書き出す
	defs.each do |item|
		name = item[:member_name]
		if name != '----'
			file.puts("static const tString & mn#{item[:long_id]} = "+
				"*reinterpret_cast<const tString *>(&VMInsnInfo[oc#{item[:long_id]}].RawMemberName);"+
				" //!< (演算子メンバ名) #{item[:def_comment]}")
		end
	end
end

#-----------------------------------------------------------------------


File.open(ARGV[2], "w") do |file|
	# ヘッダを書き出す
file.puts <<EOS
// generated by tools/opecodes.rb
// do not edit this file by hand

EOS

	# tVMInsnInfo の tString ストレージを書き出す
	enumcount = 0
	offset = 0
	file.puts "// RawMemberNameの領域は tString の文字列ポインタが指す先と"
	file.puts "//       同じレイアウトになっている"
	file.puts "// risseStaticStrings も似た実装になっているので参照のこと"

	# tVMInsnInfo の構造体を書き出す
	enumcount = 0
	file.puts "const tVMInsnInfo VMInsnInfo[] = {"
	defs.each do |item|
#		file.printf "/* %3d */", enumcount
		file.printf "{"
		file.print "#{item[:long_id].dump}, #{item[:mnemonic].dump}, "
		file.print "{#{item[:On].split(/,/).map{ |i| On_defs[i] }.join(',')}}, "
		file.print "#{Eff_defs[item[:Eff]]}, "
		if item[:member_name] == '----'
			file.print "{RISSE_STRING_EMPTY_BUFFER,0}"
		else
			file.print "{"
			file.print "tSS<"
			chars = ''
			item[:member_name].each_byte do |byte|
				chars << ',' if chars != ''
				chars << "#{byte.chr.dump.gsub(/^"/,"'").gsub(/"$/,"'")}"
			end
			file.print chars
			file.print ">::data.Buffer"
			file.print ",#{item[:member_name].length}}"
		end
		file.puts " /* #{item[:def_comment]} */},"
		enumcount += 1
	end

	file.puts "};"
end

#-----------------------------------------------------------------------


# 終了する
exit 0
