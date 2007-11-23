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
//! @brief RINA ピン管理
//---------------------------------------------------------------------------
#ifndef RINAPIN_H
#define RINAPIN_H

#include "visual/rina/rinaNode.h"

namespace Rina {
//---------------------------------------------------------------------------


class tProcessNode;
//---------------------------------------------------------------------------
//! @brief		ピン
//---------------------------------------------------------------------------
class tPin : public tCollectee
{
	typedef tCollectee inherited;

	tProcessNode * Node; //!< このピンを保有しているノード

public:
	//! @brief		コンストラクタ
	tPin();

	//! @brief		指定された形式が接続可能かどうかを判断する
	//! @param		type		接続形式
	//! @return		接続可能かどうか
	virtual bool CanConnect(risse_uint32 type) = 0;

	//! @brief		プロセスノードにこのピンをアタッチする
	//! @param		node		プロセスノード (NULL=デタッチ)
	void Attach(tProcessNode * node) { Node = node; }
};
//---------------------------------------------------------------------------



class tOutputPin;
//---------------------------------------------------------------------------
//! @brief		入力ピン
//---------------------------------------------------------------------------
class tInputPin : public tPin
{
	typedef tPin inherited;

	tOutputPin * OutputPin; //!< この入力ピンにつながっている出力ピン

public:
	//! @brief		コンストラクタ
	tInputPin();

	//! @brief		出力ピンを接続する
	//! @param		output_pin		出力ピン(NULL=接続解除)
	//! @note		サブクラスでオーバーライドしたときは最後に親クラスのこれを呼ぶこと。
	//! @note		実際に形式が合うかどうかのチェックはここでは行わない。強制的につないでしまうので注意。
	//! @note		ピンは入力ピンがかならず何かの出力ピンを接続するという方式なので
	//!				出力ピン側のこのメソッドはprotectedになっていて外部かｒアクセスできない。
	virtual void Connect(tOutputPin * output_pin);
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		出力ピン
//---------------------------------------------------------------------------
class tOutputPin : public tPin
{
	friend class tInputPin;
	typedef tPin inherited;

	tInputPin * InputPin; //!< この出力ピンにつながっている入力ピン

public:
	//! @brief		コンストラクタ
	tOutputPin();

protected:
	//! @brief		入力ピンを接続する(tInputPin::Connectから呼ばれる)
	//! @param		input_pin	入力ピン
	//! @note		サブクラスでオーバーライドしたときは最後に親クラスのこれを呼ぶこと。
	virtual void Connect(tInputPin * input_pin);
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
}

#endif
