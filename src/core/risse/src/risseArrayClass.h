//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risse用 "Array" クラスの実装
//---------------------------------------------------------------------------

#ifndef risseArrayClassH
#define risseArrayClassH

#include "risseObject.h"
#include "risseClass.h"
#include "risseGC.h"
#include "risseNativeBinder.h"

namespace Risse
{
//---------------------------------------------------------------------------
//! @brief		"Array" クラスのインスタンス用 C++クラス
//---------------------------------------------------------------------------
class tArrayInstance : public tObjectBase
{
public:
	typedef gc_deque<tVariant> tArray; //!< 配列の中身のtypedef

private:
	tArray Array; //!< 配列の中身

public:
	//! @brief		Arrayへの参照を得る
	//! @return		Arrayへの参照
	tArray & GetArray() { return Array; }

	//! @brief		ダミーのデストラクタ(おそらく呼ばれない)
	virtual ~tArrayInstance() {;}

public: // Risse用メソッドなど

	void construct();
	void initialize(const tNativeCallInfo &info);
	tVariant iget(risse_offset ofs_index);
	void iset(const tVariant & value, risse_offset ofs_index);
	void push(const tMethodArgument & args);
	tVariant pop();
	void unshift(const tMethodArgument & args);
	tVariant shift();
	size_t get_length() const;
	void set_length(size_t new_size);
	tString join(const tMethodArgument & args);
	tVariant remove(const tVariant & value, const tMethodArgument & args);
	tVariant erase(risse_offset ofs_index);
	bool has(const tVariant & value);
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		"Array" クラス
//---------------------------------------------------------------------------
class tArrayClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	//! @brief		コンストラクタ
	//! @param		engine		スクリプトエンジンインスタンス
	tArrayClass(tScriptEngine * engine);

	//! @brief		各メンバをインスタンスに追加する
	void RegisterMembers();

	//! @brief		newの際の新しいオブジェクトを作成して返す
	static tVariant ovulate();

public:
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		"Enumerable" クラス状の物を簡単にイテレートするためのクラス
// TODO: Array ではなくて enumerable への対応？
//---------------------------------------------------------------------------
class tEnumerableIterator : public tCollectee
{
	const tVariant & Array; //!< 配列
	risse_size Count; //!< 配列の値
	tVariant Value; //!< 現在の値
	risse_size Index; //!< 現在のイテレーションのインデックス

public:
	//! @brief		コンストラクタ
	//! @param		array		Enumerable なオブジェクト(vtObjectあるいはvtDataであること)
	tEnumerableIterator(const tVariant & array);

	//! @brief		配列から次の要素を取り出す
	//! @return		配列にすでに要素がない場合は false
	//! @note		Index と Value にそれぞれインデックスとそれに対応した値が格納されるので
	//!				GetIndex() や GetValue() で取得すること
	bool Next();

	//! @brief		配列の要素数を取り出す
	//! @return		配列の要素数
	risse_size GetCount() const { return Count; }

	//! @brief		現在のインデックスを取得する
	//! @return		現在のインデックス
	risse_size GetIndex() const { return Index; }

	//! @brief		現在の値を取得する
	//! @return		現在の値
	const tVariant & GetValue() const { return Value; }


};
//---------------------------------------------------------------------------

} // namespace Risse


#endif
