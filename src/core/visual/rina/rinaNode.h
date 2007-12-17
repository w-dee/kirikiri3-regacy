//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"

	Rina stands for "Rina is an Imaging Network Assembler"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief RINA ノード管理
//---------------------------------------------------------------------------
#ifndef RINANODE_H
#define RINANODE_H

#include "visual/rina/rinaIdRegistry.h"

namespace Rina {
//---------------------------------------------------------------------------


class tInputPin;
class tOutputPin;
class tQueueNode;
class tRenderState;
//---------------------------------------------------------------------------
//! @brief		プロセスノード
//---------------------------------------------------------------------------
class tProcessNode : public tCollectee
{
	typedef tCollectee inherited;

	risse_size	LongestDistance;
			//!< ルートノードからの最長距離(ステップ数)。
			//!< 依存関係のクイックなチェックに用いる

	risse_size	BuildQueueWaitingOutputPin; //!< BuildQueue にて待つべき出力/入力ピンの数

public:
	//! @brief		プロセスノードを LongestDistance で比較する関数
	struct tLongestDistanceComparator :
		public std::binary_function<bool, tProcessNode*, tProcessNode*>
	{
		bool operator ()(tProcessNode * a, tProcessNode * b)
		{
			if(a->GetLongestDistance() < b->GetLongestDistance()) return true;
			if(a < b) return true;
			return false;
		}
	};


public:
	//! @brief		コンストラクタ
	tProcessNode();

public:
	//! @brief		ルートノードからの最長距離を得る
	//! @return		ルートノードからの最長距離 (ルートノード = 0)
	//! @note		どの出力ピンも接続されていない状態では返される値の内容は不定
	risse_size GetLongestDistance() const { return LongestDistance; }

	//! @brief		ルートノードからの最長距離を設定する
	//! @note		このメソッドは入力ピン(子ノード)に再帰して、子すべての最長距離を更新する
	void CalcLongestDistance();

public: // サブクラスで実装すべき物
	//! @brief		出力ピンの個数を得る
	//! @return		出力ピンの個数
	virtual risse_size GetOutputPinCount() = 0;

	//! @brief		指定位置の出力ピンを得る
	//! @param		n		指定位置
	//! @return		指定位置の出力ピン
	virtual tOutputPin * GetOutputPinAt(risse_size n) = 0;

	//! @brief		指定位置に新規出力ピンを挿入する
	//! @param		n		指定位置
	virtual void InsertOutputPinAt(risse_size n) = 0;

	//! @brief		指定位置から出力ピンを削除する
	//! @param		n		指定位置
	virtual void DeleteOutputPinAt(risse_size n) = 0;


	//! @brief		入力ピンの個数を得る
	//! @return		入力ピンの個数
	virtual risse_size GetInputPinCount() = 0;

	//! @brief		指定位置の入力ピンを得る
	//! @param		n		指定位置
	//! @return		指定位置の入力ピン
	virtual tInputPin * GetInputPinAt(risse_size n) = 0;

	//! @brief		指定位置に入力ピンを挿入する
	//! @param		n		指定位置
	virtual void InsertInputPinAt(risse_size n) = 0;

	//! @brief		指定位置から入力ピンを削除する
	//! @param		n		指定位置
	virtual void DeleteInputPinAt(risse_size n) = 0;

	//! @brief		コマンドキューの組み立てを行う
	//! @param		state			レンダリングステート
	//! @param		input_pin		この情報を必要としているノードの入力ピン
	//! @param		parent			親となるであろうキューノード
	virtual void BuildQueue(tRenderState * state, tInputPin * input_pin, tQueueNode * parent) { return ; }


};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
}

#endif
