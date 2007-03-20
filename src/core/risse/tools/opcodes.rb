#!/usr/bin/env ruby


# risseOpCodes.txt から定義を生成する

# ハッシュの計算

class String
	# このハッシュ表は Risse のハッシュ表で使われているのと同じ方式である。
	# 詳細は risseHashTable.h を参照のこと。
	def make_risse_hash
		hash = 0
		self.each_byte do |byte|
			hash += byte
			hash &= 0xffffffff
			hash += (hash << 10)
			hash &= 0xffffffff
			hash ^= (hash >> 6)
			hash &= 0xffffffff
		end
		hash += (hash << 3)
		hash &= 0xffffffff
		hash ^= (hash >> 11)
		hash &= 0xffffffff
		hash += (hash << 15)
		hash &= 0xffffffff
		hash = 0xffffffff if hash == 0
		hash
	end
end

# 入力ファイルを開いて一行ずつ処理をする
errorcount = 0
linecount = 0
On_defs = {'-' => '(tRisseVMInsnInfo::tInsnFlag)(0)'}
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
	elsif line =~ /(\w+)\s+(\w+)\s+([^\s]+)\s+([^\s]+)$/
		# defs
		defs << { :long_id => $1, :mnemonic => $2,
				:On => $3, :member_name => $4, :def_comment => defcomment }
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
	file.puts "enum tRisseOpCode {"
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
			file.puts("static const tRisseString & mn#{item[:long_id]} = "+
				"*reinterpret_cast<const tRisseString *>(&RisseVMInsnInfo[oc#{item[:long_id]}].RawMemberName);"+
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

	# RisseVMInsnInfo の tRisseString ストレージを書き出す
	enumcount = 0
	offset = 0
	file.puts "// RisseVMInsnInfoのMemberNameが参照する文字列領域"
	file.puts "// この領域は tRisseString の文字列ポインタが指す先と"
	file.puts "//       同じレイアウトになっている"
	defs.each do |item|
		name = item[:member_name]
		if name != '----'
			file.print "static risse_char v_#{item[:long_id]}[] = { "
			file.print "tRisseStringData::MightBeShared,"
			name.each_byte do |byte|
				file.print "#{byte.chr.dump.gsub(/^"/,"'").gsub(/"$/,"'")},"
			end
			file.print "0,"  # null terminator
			file.printf "0x%08x", name.make_risse_hash  # hash
			file.print " };"
			file.print " /*!< string of #{item[:def_comment]} */"
			file.print "\n"
		end
	end

	file.print "\n\n"

	# tRisseVMInsnInfo の構造体を書き出す
	enumcount = 0
	file.puts "const tRisseVMInsnInfo RisseVMInsnInfo[] = {"
	defs.each do |item|
#		file.printf "/* %3d */", enumcount
		file.printf "{"
		file.print "#{item[:long_id].dump}, #{item[:mnemonic].dump}, "
		file.print "{#{item[:On].split(/,/).map{ |i| On_defs[i] }.join(',')}}, "
		if item[:member_name] == '----'
			file.print "{RISSE_STRING_EMPTY_BUFFER,0}"
		else
			file.print "{v_#{item[:long_id]}+1,#{item[:member_name].length}}"
		end
		file.puts " /* #{item[:def_comment]} */},"
		enumcount += 1
	end

	file.puts "};"
end

#-----------------------------------------------------------------------


# 終了する
exit 0
