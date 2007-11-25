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



namespace Rina {
//---------------------------------------------------------------------------


class tInputPin;
class tOutputPin;
class tProperty;
//---------------------------------------------------------------------------
//! @brief		プロセスノード
//---------------------------------------------------------------------------
class tProcessNode : public tCollectee
{
	typedef tCollectee inherited;

	tProperty * Property; //!< このノードのプロパティ

public:
	//! @brief		コンストラクタ
	tProcessNode();

protected:
	//! @brief		プロパティインスタンスを設定する
	//! @param		prop		プロパティインスタンス
	void SetProperty(tProperty * prop) { Property = prop; }

public:
	//! @brief		プロパティインスタンスを得る
	//! @return		プロパティインスタンス
	tProperty * GetProperty() const { return Property; }

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

};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
}

#endif
