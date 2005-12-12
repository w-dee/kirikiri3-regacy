open FH, "CP932.TXT" or die;

%cmap = ();

# ファイルを全部読み込んで UNICODE->SJIS のマッピングを作成

while($line = <FH>)
{
	chomp $line;

	if($line =~ /^0x([A-Za-z0-9]+)\t0x([A-Za-z0-9]+)/)
	{
		$to   = sprintf("%04X", hex($1));
		$from = sprintf("%04X", hex($2));
		$cmap{$from} = $to;
	}
}

open FH, ">../uni_cp932.cpp" or die;
open HH, ">../uni_cp932.h" or die;


$head =  <<EOF;
//---------------------------------------------------------------------------
/*
	TVP3 ( T Visual Presenter 3 )  A script authoring tool
	Copyright (C) 2000-2005  W.Dee <dee\@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! \@file
//! \@brief UNICODE->CP932(≒SJIS) 変換
//---------------------------------------------------------------------------
// UNICODEとSJIS(cp932)の変換マップについては unicode.org を参照のこと

EOF

print FH $head;
print HH $head;

print HH <<EOF;

#ifndef _UNI_CP932_H_
#define _UNI_CP932_H_

tjs_uint TVPUnicodeToSJIS(tjs_char in);

#endif
EOF

print FH <<EOF;
#include "prec.h"

//---------------------------------------------------------------------------
//! \@brief		UNICODEとShiftJISコードの組を表す型
//---------------------------------------------------------------------------
struct tTVPUnicodeAndSJISPair
{
	tjs_uint16 Unicode;
	tjs_uint16 SJIS;
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//! \@brief		UNICODEとShiftJISコードの組(UNICODEでソート済み)
//---------------------------------------------------------------------------
static const tTVPUnicodeAndSJISPair TVPUnicodeAndSJISPair[] = {
EOF

$n = 0;
foreach $each (sort keys %cmap)
{
	printf FH "{ 0x%sU, 0x%sU },", $each, $cmap{$each};
	$n ++;
	if($n == 3)
	{
		printf FH "\n";
		$n = 0;
	}
}

print FH <<EOF;
};

//---------------------------------------------------------------------------
//! \@brief		UNICODEとShiftJISコードの組の数
//---------------------------------------------------------------------------
#define TVPNumUnicodeAndSJISPair \\
	(sizeof(TVPUnicodeAndSJISPair) / sizeof(TVPUnicodeAndSJISPair[0]))
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//! \@brief		UNICODEをShiftJIS(CP932)に変換する
//! \@param		in 変換したい文字
//! \@return	変換されたShiftJIS文字(0=変換失敗)
//! \@note		ShiftJIS 文字は 16bit 数値で返される(実際の戻り値の型はtjs_uint)
//! \@note		が、いわゆる半角文字の場合は上位8ビットは0、いわゆる全角文字は
//! \@note		ShiftJISコードがそのまま入る。
//---------------------------------------------------------------------------
tjs_uint TVPUnicodeToSJIS(tjs_char in)
{
	// TVPUnicodeAndSJISPair に対して二分検索を行う
	if(in >= 0x10000) return 0;
		// 0x10000 以上のコードポイントに対してはSJIS文字変換は定義されていない

	tjs_uint s = 0, e = TVPNumUnicodeAndSJISPair;
	while(e - s > 1)
	{
		tjs_int m = (s + e) / 2;
		if(TVPUnicodeAndSJISPair[m].Unicode <= static_cast<tjs_uint16>(in))
			s = m;
		else
			e = m;
	}
	if(TVPUnicodeAndSJISPair[s].Unicode == static_cast<tjs_uint16>(in))
		return TVPUnicodeAndSJISPair[s].SJIS; // 見つかった
	return 0;
}
//---------------------------------------------------------------------------


#ifdef SELFTEST

#include <stdio.h>
#include "cp932_uni.h"

int main(void)
{
	for(tjs_uint i = 1; i < 65536; i++)
	{
		tjs_uint sjis = TVPUnicodeToSJIS(i);
		if(sjis == 0) continue;
		tjs_uint uni_out = TVPSJISToUnicode(sjis);
		if(uni_out != i)
		{
			printf("fail: uni input:%x, sjis:%x, uni output:%x\\n",
				i, sjis, uni_out);
		}
	}
}

#endif

EOF

