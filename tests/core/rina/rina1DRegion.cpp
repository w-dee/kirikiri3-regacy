//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"

	Rina stands for "Rina is an Imaging Network Assembler"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief 1次元のリージョン管理
//---------------------------------------------------------------------------
#include "prec.h"
#include "rina1DRegion.h"


namespace Rina {
RISSE_DEFINE_SOURCE_ID(8780,6469,60190,18639,56962,35297,60958,36445);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
t1DRegion::t1DRegion()
{
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void t1DRegion::Add(t1DArea area)
{
	// うーんと
	// 真面目にやるとこれ結構厄介なんですが
	// ここはテスト目的なので適当に………

	// 重なってるエリアを検索 -> 重なっているエリアはすべて削除
	for(tAreas::iterator i = Areas.begin(); i != Areas.end(); /**/ )
	{
		if(area.Extend(*i))
			i = Areas.erase(i);
		else
			i++;
	}

	// area を追加
	Areas.push_back(area);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
}
