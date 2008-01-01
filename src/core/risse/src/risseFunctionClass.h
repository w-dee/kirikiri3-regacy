//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risse用 "Function" クラスの実装
//---------------------------------------------------------------------------

#ifndef risseFunctionClassH
#define risseFunctionClassH

#include "risseObject.h"
#include "risseClass.h"
#include "risseGC.h"
#include "risseNativeBinder.h"

namespace Risse
{
//---------------------------------------------------------------------------
//! @brief		"Function" クラスのインスタンス用 C++クラス
//! @note		ネイティブ関数もスクリプトで生成した関数もいったんこのクラスを
//!				ゲートウェイとして使う。Risseで"Function"クラスのインスタンスである
//!				と見せかけるための仕掛け。
//---------------------------------------------------------------------------
class tFunctionInstance : public tObjectBase
{
	typedef tObjectBase inherited; //!< 親クラスの typedef

private:
	tVariant Body; //!< 関数のBodyを表すオブジェクト
	bool Synchronized; //!< synchronized メソッドかどうか

public:
	//! @brief		コンストラクタ
	tFunctionInstance();

	//! @brief		関数のBodyを表すオブジェクトを取得する
	//! @return		関数のBodyを表すオブジェクト
	tVariant & GetBody() { return Body; }

	//! @brief		関数のBodyを表すオブジェクトを設定する
	//! @param		m		関数のBodyを表すオブジェクト
	void SetBody(const tVariant & m) { Body = m; }


	//! @brief		ダミーのデストラクタ(おそらく呼ばれない)
	virtual ~tFunctionInstance() {;}

	//! @brief		オブジェクトに対して操作を行う
	virtual tRetValue Operate(RISSE_OBJECTINTERFACE_OPERATE_DECL_ARG);

	//! @brief		synchronized メソッドかどうかを取得する
	//! @return		synchronized メソッドかどうか
	bool GetSynchronized() const { return Synchronized; }

	//! @brief		synchronized メソッドかどうかを設定する
	//! @param		b	synchronized メソッドかどうか
	void SetSynchronized(bool b) { Synchronized = b; }


public: // Risse用メソッドとか
	void construct();
	void initialize(const tNativeCallInfo & info);
	bool get_synchronized() const { return GetSynchronized(); }
	void set_synchronized(bool b) { SetSynchronized(b); }
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		"Function" クラス
//---------------------------------------------------------------------------
class tFunctionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	//! @brief		コンストラクタ
	//! @param		engine		スクリプトエンジンインスタンス
	tFunctionClass(tScriptEngine * engine);

	//! @brief		各メンバをインスタンスに追加する
	void RegisterMembers();

	//! @brief		newの際の新しいオブジェクトを作成して返す
	static tVariant ovulate();
};
//---------------------------------------------------------------------------
} // namespace Risse


#endif
