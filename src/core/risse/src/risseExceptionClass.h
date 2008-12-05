//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risse 例外クラス
//---------------------------------------------------------------------------


#ifndef risseExceptionClassH
#define risseExceptionClassH

#include "risseCharUtils.h"
#include "risseTypes.h"
#include "risseAssert.h"
#include "risseString.h"
#include "risseGC.h"
#include "risseObject.h"
#include "risseClass.h"

namespace Risse
{
//---------------------------------------------------------------------------
/**
 * 仮例外用クラス
 * @note	例外クラスはスクリプトエンジンインスタンスに依存しているため、
 *			スクリプトエンジンインスタンスへのポインタが利用できない場合は
 *			この例外を仮になげておく。スクリプトエンジンインスタンスが利用
 *			可能な場所でこの例外を受け取った場合は、
 *			tTemporaryException::ThrowConverted() メソッドで
 *			本来の例外に変換し、再び投げることができる。
 */
class tTemporaryException : public tCollectee
{
	tString ExceptionClassName; //!< 例外クラス名
	tString PackageName; //!< パッケージ名
	gc_vector<tVariant> Arguments; //!< 例外クラスのコンストラクタに渡すための引数
public:
	/**
	 * コンストラクタ
	 * @param package	パッケージ名
	 * @param classname	クラス名
	 */
	tTemporaryException(const tString & package, const tString & classname);

	/**
	 * コンストラクタ
	 * @param package	パッケージ名
	 * @param classname	クラス名
	 * @param arg1		引数1
	 */
	tTemporaryException(const tString & package, const tString & classname,
		const tVariant & arg1);

	/**
	 * コンストラクタ
	 * @param package	パッケージ名
	 * @param classname	クラス名
	 * @param arg1		引数1
	 * @param arg2		引数2
	 */
	tTemporaryException(const tString & package, const tString & classname,
		const tVariant & arg1, const tVariant & arg2);

	/**
	 * コンストラクタ
	 * @param package	パッケージ名
	 * @param classname	クラス名
	 * @param arg1		引数1
	 * @param arg2		引数2
	 * @param arg3		引数3
	 */
	tTemporaryException(const tString & package, const tString & classname,
		const tVariant & arg1, const tVariant & arg2, const tVariant & arg3);


	/**
	 * 例外を本来の例外に変換する
	 * @param engine	スクリプトエンジン
	 * @return	変換された例外オブジェクト
	 */
	tVariant * Convert(tScriptEngine * engine) const;

	/**
	 * 例外を本来の例外に変換し、再び投げる
	 * @param engine	スクリプトエンジン
	 */
	void ThrowConverted(tScriptEngine * engine) const RISSE_NORETURN
	{ throw Convert(engine); }

	/**
	 * 例外クラス名を得る
	 */
	const tString & GetExceptionClassName() const { return ExceptionClassName; }

	/**
	 * 内容を標準エラー出力にダンプする
	 */
	void Dump() const;
};
//---------------------------------------------------------------------------








class tScriptBlockInstance;
//---------------------------------------------------------------------------
/**
 * RisseのReturnExceptionやBreakExceptionなどのインスタンス(暫定実装)
 */
class tExitTryExceptionClass : public tObjectInterface
{
	const void * Identifier; //!< tryブロックを識別するための情報
	risse_uint32 BranchTargetIndex; //!< 分岐ターゲットのインデックス
	const tVariant * Value; //!< 例外とともに投げられる情報 (もしあれば)

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 * @param id		tryブロックを識別するための情報
	 * @param targ_idx	分岐ターゲットのインデックス
	 * @param value		例外とともに投げられる情報 (無ければ NULL)
	 */
	tExitTryExceptionClass(tScriptEngine * engine,
					const void * id, risse_uint32 targ_idx,
						const tVariant * value = NULL);

	// 暫定実装 TODO:ちゃんと実装
	const void * GetIdentifier() const { return Identifier; }
	risse_uint32 GetBranchTargetIndex() const { return BranchTargetIndex; }
	const tVariant * GetValue() const { return Value; }


public: // tObjectInterface メンバ

	tRetValue Operate(RISSE_OBJECTINTERFACE_OPERATE_DECL_ARG);


};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
/**
 * "SourcePoint" クラスのインスタンス用 C++クラス
 */
class tSourcePointInstance : public tObjectBase
{
public: // Risse用メソッドなど
	void construct();
	void initialize(const tNativeCallInfo & info);
	tString toString();
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/**
 * "SourcePoint" クラス
 * @note	ソースコード上の位置を表すクラス@n
 *			注意: このクラスの実装は将来的に変更される可能性がある。
 *			このクラスのインスタンスを自前で作成したり、インスタンスのメンバに
 *			書き込んだりしないこと。
 */
class tSourcePointClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tSourcePointClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

	/**
	 * newの際の新しいオブジェクトを作成して返す
	 */
	static tVariant ovulate();
};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
/**
 * "Throwable" クラス
 * @note	このクラスでは専用のインスタンス用 C++クラスを用いない
 *			(tObjectBase を直接使う)。そのため BindFunction に渡す
 *			C++メソッドはすべてstatic宣言になり、あまりNativeBinderの恩恵が
 *			無いが、しかたない。
 */
class tThrowableClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tThrowableClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct(const tNativeCallInfo & info);
	static void initialize(const tNativeCallInfo & info);
	static tString toString(const tNativeCallInfo & info);
	static void addTrace(const tVariant & point,
		const tNativeCallInfo & info);
	static tVariant toException(const tNativeCallInfo & info);
};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
/**
 * "Error" クラス
 */
class tErrorClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tErrorClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);
};
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
/**
 * "AssertionError" クラス
 */
class tAssertionErrorClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tAssertionErrorClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct(const tNativeCallInfo & info);
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 例外を投げる
	 * @param engine		スクリプトエンジンインスタンス
	 * @param expression	ASSERTに失敗した式
	 */
	static void Throw(tScriptEngine * engine, const tString & expression) RISSE_NORETURN;

	/**
	 * 例外を投げる
	 * @param expression	ASSERTに失敗した式
	 */
	static void Throw(const tString & expression) RISSE_NORETURN { Throw(NULL, expression); }
};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
/**
 * "BlockExitException" クラス
 */
class tBlockExitExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tBlockExitExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct(const tNativeCallInfo & info);
	static void initialize(const tNativeCallInfo & info);
};
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
/**
 * "Exception" クラス
 */
class tExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);
};
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
/**
 * "InsufficientResourceException" クラス
 */
class tInsufficientResourceExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tInsufficientResourceExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「コルーチンを作成できない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowCouldNotCreateCoroutine(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「コルーチンを作成できない」例外を発生
	 */
	static void ThrowCouldNotCreateCoroutine() RISSE_NORETURN { ThrowCouldNotCreateCoroutine(NULL); }
};
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
/**
 * "IOException" クラス
 */
class tIOExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tIOExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param msg		例外メッセージ
	 */
	static void Throw(tScriptEngine * engine, const tString & message) RISSE_NORETURN;
	/**
	 * 「読み込みエラーが発生した」例外を発生
	 */
	static void Throw(const tString & message) RISSE_NORETURN
		{ Throw(NULL, message); }

	/**
	 * 「読み込みエラーが発生した」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		ストリームなどの名前
	 */
	static void ThrowReadError(tScriptEngine * engine,
		const tString & name = tString::GetEmptyString()) RISSE_NORETURN;
	/**
	 * 「読み込みエラーが発生した」例外を発生
	 */
	static void ThrowReadError(const tString & name =
		tString::GetEmptyString()) RISSE_NORETURN
		{ ThrowReadError(NULL, name); }

	/**
	 * 「書き込みエラーが発生した」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		ストリームなどの名前
	 */
	static void ThrowWriteError(tScriptEngine * engine,
		const tString & name = tString::GetEmptyString()) RISSE_NORETURN;
	/**
	 * 「書き込みエラーが発生した」例外を発生
	 */
	static void ThrowWriteError(const tString & name =
		tString::GetEmptyString()) RISSE_NORETURN { ThrowWriteError(NULL, name); }

	/**
	 * 「シークエラーが発生した」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		ストリームなどの名前
	 * @param pos		シークしようとした位置
	 */
	static void ThrowSeekError(tScriptEngine * engine,
		const tString & name = tString::GetEmptyString(),
		risse_size pos = risse_size_max) RISSE_NORETURN;
	/**
	 * 「シークエラーが発生した」例外を発生
	 */
	static void ThrowSeekError(const tString & name =
		tString::GetEmptyString(), risse_size pos = risse_size_max) RISSE_NORETURN
		{ ThrowSeekError(NULL, name, pos); }

	/**
	 * 「ファイルの切りつめに失敗した」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		ストリームなどの名前
	 * @param pos		切りつめようとした位置
	 */
	static void ThrowTruncateError(tScriptEngine * engine,
		const tString & name = tString::GetEmptyString(),
		risse_size pos = risse_size_max) RISSE_NORETURN;
	/**
	 * 「ファイルの切りつめに失敗した」例外を発生
	 */
	static void ThrowTruncateError(const tString & name =
		tString::GetEmptyString(), risse_size pos = risse_size_max) RISSE_NORETURN
		{ ThrowTruncateError(NULL, name, pos); }
#if 0
	See tInaccessibleResourceExceptionClass::Throw
	/**
	 * 「ストリームは閉じられている」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		ストリームなどの名前
	 */
	static void ThrowStreamIsClosed(tScriptEngine * engine,
		const tString & name = tString::GetEmptyString()) RISSE_NORETURN;
	/**
	 * 「ストリームは閉じられている」例外を発生
	 */
	static void ThrowStreamIsClosed(const tString & name =
		tString::GetEmptyString()) RISSE_NORETURN
		{ ThrowStreamIsClosed(NULL, name); }
#endif
};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
/**
 * "CharConversionException" クラス
 */
class tCharConversionExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tCharConversionExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「無効なUTF-8文字列です」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowInvalidUTF8String(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「無効なUTF-8文字列です」例外を発生
	 */
	static void ThrowInvalidUTF8String() RISSE_NORETURN { ThrowInvalidUTF8String(NULL); }
};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
/**
 * "RuntimeException" クラス
 */
class tRuntimeExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tRuntimeExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);
};
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
/**
 * "CompileException" クラス
 */
class tCompileExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tCompileExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「コンパイルエラー」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param reason	例外の理由
	 * @param sb		スクリプトブロック
	 * @param pos		スクリプト上の位置
	 */
	static void Throw(tScriptEngine * engine, const tString & reason,
		const tScriptBlockInstance * sb = NULL, risse_size pos = risse_size_max) RISSE_NORETURN;
	/**
	 * 「コンパイルエラー」例外を発生
	 * @param reason	例外の理由
	 */
	static void Throw(const tString & reason) RISSE_NORETURN
		{ Throw(NULL, reason); }
};
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
/**
 * "ImportException" クラス
 */
class tImportExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tImportExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「プリミティブ型インスタンスのコンテキストにはインポートできない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowCannotImportIntoPrimitiveInstanceContext(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「プリミティブ型クラスのコンテキストにはインポートできない」例外を発生
	 */
	static void ThrowCannotImportIntoPrimitiveInstanceContext() RISSE_NORETURN
		{ ThrowCannotImportIntoPrimitiveInstanceContext(NULL); }

	/**
	 * 「パッケージが見つからない」例外を発生
	 * @param engine		スクリプトエンジンインスタンス
	 * @param package_name	パッケージ名
	 */
	static void ThrowPackageNotFound(tScriptEngine * engine, const tString & package_name) RISSE_NORETURN;
	/**
	 * 「パッケージが見つからない」例外を発生
	 * @param package_name	パッケージ名
	 */
	static void ThrowPackageNotFound(const tString & package_name) RISSE_NORETURN
		{ ThrowPackageNotFound(NULL, package_name); }

	/**
	 * 「パッケージは現在初期化中なのでインポートできない」例外を発生
	 * @param engine		スクリプトエンジンインスタンス
	 * @param package_name	パッケージ名
	 */
	static void ThrowPackageIsBeingInitialized(tScriptEngine * engine, const tString & package_name) RISSE_NORETURN;
	/**
	 * 「パッケージは現在初期化中なのでインポートできない」例外を発生
	 * @param package_name	パッケージ名
	 */
	static void ThrowPackageIsBeingInitialized(const tString & package_name) RISSE_NORETURN
		{ ThrowPackageIsBeingInitialized(NULL, package_name); }

	/**
	 * 「識別子をインポートできない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param ids		インポートできなかった識別子名
	 */
	static void ThrowCannotImportIds(tScriptEngine * engine,
			const gc_vector<tString> & ids) RISSE_NORETURN;
	/**
	 * 「識別子をインポートできない」例外を発生
	 * @param ids	インポートできなかった識別子名
	 */
	static void ThrowCannotImportIds(const gc_vector<tString> & ids) RISSE_NORETURN
		{ ThrowCannotImportIds(NULL, ids); }

	/**
	 * 「不正な相対パッケージ名指定」例外を発生
	 * @param engine		スクリプトエンジンインスタンス
	 * @param package_name	パッケージ名
	 * @param ref			基準となったパッケージ名
	 */
	static void ThrowInvalidRelativePckageName(tScriptEngine * engine,
			const tString & package_name, const tString & ref) RISSE_NORETURN;
	/**
	 * 「不正な相対パッケージ名指定」例外を発生
	 * @param ids			パッケージ名
	 * @param package_name	パッケージ名
	 * @param ref			基準となったパッケージ名
	 */
	static void ThrowInvalidRelativePckageName(
		const tString & package_name, const tString & ref) RISSE_NORETURN
		{ ThrowInvalidRelativePckageName(NULL, package_name, ref); }

};
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
/**
 * "ClassDefinitionException" クラス
 */
class tClassDefinitionExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tClassDefinitionExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「extensibleでないクラスのサブクラスを作成しようとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowCannotCreateSubClassOfNonExtensibleClass(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「extensibleでないクラスのサブクラスを作成しようとした」例外を発生
	 */
	static void ThrowCannotCreateSubClassOfNonExtensibleClass() RISSE_NORETURN
		{ ThrowCannotCreateSubClassOfNonExtensibleClass(NULL); }

	/**
	 * 「スーパークラスはクラスではない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowSuperClassIsNotAClass(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「スーパークラスはクラスではない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowSuperClassIsNotAClass() RISSE_NORETURN
		{ ThrowSuperClassIsNotAClass(NULL); }
};
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
/**
 * "InstantiationException" クラス
 */
class tInstantiationExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tInstantiationExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「クラスでない物からインスタンスを生成しようとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowCannotCreateInstanceFromNonClassObject(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「クラスでない物からインスタンスを生成しようとした」例外を発生
	 */
	static void ThrowCannotCreateInstanceFromNonClassObject() RISSE_NORETURN
		{ ThrowCannotCreateInstanceFromNonClassObject(NULL); }

	/**
	 * 「このクラスからインスタンスは作成できません」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowCannotCreateInstanceFromThisClass(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「このクラスからインスタンスは作成できません」例外を発生
	 */
	static void ThrowCannotCreateInstanceFromThisClass() RISSE_NORETURN
		{ ThrowCannotCreateInstanceFromThisClass(NULL); }
};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
/**
 * "BadContextException" クラス
 */
class tBadContextExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tBadContextExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「異なるクラスのコンテキストです」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void Throw(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「異なるクラスのコンテキストです」例外を発生
	 */
	static void Throw() RISSE_NORETURN { Throw(NULL); }
};
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
/**
 * "UnsupportedOperationException" クラス
 */
class tUnsupportedOperationExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tUnsupportedOperationExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「関数でない物を呼び出そうとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowCannotCallNonFunctionObjectException(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「関数でない物を呼び出そうとした」例外を発生
	 */
	static void ThrowCannotCallNonFunctionObjectException() RISSE_NORETURN
		{ ThrowCannotCallNonFunctionObjectException(NULL); }

	/**
	 * 「機能が実装されていません」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowOperationIsNotImplemented(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「機能が実装されていません」例外を発生
	 */
	static void ThrowOperationIsNotImplemented() RISSE_NORETURN
		{ ThrowOperationIsNotImplemented(NULL); }
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
/**
 * "ArithmeticException" クラス
 */
class tArithmeticExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tArithmeticExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「0で除算をしようとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowDivideByZeroException(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「0で除算をしようとした」例外を発生
	 */
	static void ThrowDivideByZeroException() RISSE_NORETURN
		{ ThrowDivideByZeroException(NULL); }
};
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
/**
 * "ArgumentException" クラス
 */
class tArgumentExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tArgumentExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);
};
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
/**
 * "NullObjectException" クラス
 */
class tNullObjectExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tNullObjectExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「nullオブジェクトにアクセスしようとしました」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void Throw(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「nullオブジェクトにアクセスしようとしました」例外を発生
	 */
	static void Throw() RISSE_NORETURN { Throw(NULL); }
};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
/**
 * "IllegalArgumentClassException" クラス
 */
class tIllegalArgumentClassExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tIllegalArgumentClassExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「method_nameにクラスclass_nameは受け入れられない」例外を発生
	 * @param engine		スクリプトエンジンインスタンス
	 * @param method_name	メソッド名
	 * @param class_name	クラス名
	 */
	static void ThrowNonAcceptableClass(tScriptEngine * engine,
		const tString & method_name, const tString & class_name) RISSE_NORETURN;
	/**
	 * 「method_nameにクラスclass_nameは受け入れられない」例外を発生
	 * @param method_name	メソッド名
	 * @param class_name	クラス名
	 */
	static void ThrowNonAcceptableClass(const tString & method_name, const tString & class_name) RISSE_NORETURN
		{ ThrowNonAcceptableClass(NULL, method_name, class_name); }

	/**
	 * 「このクラスではこのmethod_nameは実行できない」例外を発生
	 * @param engine		スクリプトエンジンインスタンス
	 * @param method_name	メソッド名
	 */
	static void ThrowIllegalOperationMethod(tScriptEngine * engine,
		const tString & method_name) RISSE_NORETURN;
	/**
	 * 「method_nameにクラスclass_nameは受け入れられない」例外を発生
	 * @param method_name	メソッド名
	 */
	static void ThrowIllegalOperationMethod(const tString & method_name) RISSE_NORETURN
		{ ThrowIllegalOperationMethod(NULL, method_name); }

	/**
	 * 「クラスclass_nameのインスタンスを指定すべし」例外を発生
	 * @param engine		スクリプトエンジンインスタンス
	 * @param class_name	クラス名
	 */
	static void ThrowSpecifyInstanceOfClass(tScriptEngine * engine,
		const tString & class_name) RISSE_NORETURN;
	/**
	 * 「クラスclass_nameのインスタンスを指定すべし」例外を発生
	 * @param method_name	メソッド名
	 * @param class_name	クラス名
	 */
	static void ThrowSpecifyInstanceOfClass(const tString & class_name) RISSE_NORETURN
		{ ThrowSpecifyInstanceOfClass(NULL, class_name); }


};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
/**
 * "IllegalArgumentException" クラス
 */
class tIllegalArgumentExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tIllegalArgumentExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 例外を投げる
	 * @param engine	スクリプトエンジンインスタンス
	 * @param message	メッセージ
	 */
	static void Throw(tScriptEngine * engine, const tString & message) RISSE_NORETURN;
	/**
	 * 例外を投げる
	 * @param message	メッセージ
	 */
	static void Throw(const tString & message) RISSE_NORETURN
		{ Throw(NULL, message); }

	/**
	 * 「無効な日付文字列」例外を投げる
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void ThrowInvalidDateString(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「無効な日付文字列」例外を投げる
	 */
	static void ThrowInvalidDateString() RISSE_NORETURN { ThrowInvalidDateString(NULL); }
};
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
/**
 * "BadArgumentCountException" クラス
 */
class tBadArgumentCountExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tBadArgumentCountExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 普通の引数の数が期待した数でなかった場合の例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param passed	渡された引数の数
	 * @param expected	期待した数
	 */
	static void ThrowNormal(tScriptEngine * engine, risse_size passed, risse_size expected) RISSE_NORETURN;
	/**
	 * 普通の引数の数が期待した数でなかった場合の例外を発生
	 * @param passed	渡された引数の数
	 * @param expected	期待した数
	 */
	static void ThrowNormal(risse_size passed, risse_size expected) RISSE_NORETURN
		{ ThrowNormal(NULL, passed, expected); }

	/**
	 * ブロック引数の数が期待した数でなかった場合の例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param passed	渡された引数の数
	 * @param expected	期待した数
	 */
	static void ThrowBlock(tScriptEngine * engine, risse_size passed, risse_size expected) RISSE_NORETURN;
	/**
	 * ブロック引数の数が期待した数でなかった場合の例外を発生
	 * @param passed	渡された引数の数
	 * @param expected	期待した数
	 */
	static void ThrowBlock(risse_size passed, risse_size expected) RISSE_NORETURN
		{ ThrowBlock(NULL, passed, expected); }
};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
/**
 * "MemberAccessException" クラス
 */
class tMemberAccessExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tMemberAccessExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct(const tNativeCallInfo & info);
	static void initialize(const tNativeCallInfo & info);
};
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
/**
 * "NoSuchMemberException" クラス
 */
class tNoSuchMemberExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tNoSuchMemberExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void Throw(tScriptEngine * engine, const tString & name) RISSE_NORETURN;
	/**
	 * 例外を発生
	 * @param name	メンバ名
	 */
	static void Throw(const tString & name) RISSE_NORETURN { Throw(NULL, name); }
};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
/**
 * "IllegalMemberAccessException" クラス
 */
class tIllegalMemberAccessExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tIllegalMemberAccessExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「読み出し専用メンバに上書きしようとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowMemberIsReadOnly(tScriptEngine * engine, const tString & name) RISSE_NORETURN;
	/**
	 * 「読み出し専用メンバに上書きしようとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowMemberIsReadOnly(const tString & name) RISSE_NORETURN
		{ ThrowMemberIsReadOnly(NULL, name); }

	/**
	 * 「finalメンバをオーバーライドしようとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowMemberIsFinal(tScriptEngine * engine, const tString & name) RISSE_NORETURN;
	/**
	 * 「finalメンバをオーバーライドしようとした」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowMemberIsFinal(const tString & name) RISSE_NORETURN
		{ ThrowMemberIsFinal(NULL, name); }

	/**
	 * 「このプロパティからは読み込むことができない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowPropertyCannotBeRead(tScriptEngine * engine, const tString & name) RISSE_NORETURN;
	/**
	 * 「このプロパティからは読み込むことができない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowPropertyCannotBeRead(const tString & name) RISSE_NORETURN
		{ ThrowPropertyCannotBeRead(NULL, name); }

	/**
	 * 「このプロパティには書き込むことができない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowPropertyCannotBeWritten(tScriptEngine * engine, const tString & name) RISSE_NORETURN;
	/**
	 * 「このプロパティには書き込むことができない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 * @param name		メンバ名
	 */
	static void ThrowPropertyCannotBeWritten(const tString & name) RISSE_NORETURN
		{ ThrowPropertyCannotBeWritten(NULL, name); }
};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
/**
 * "IllegalStateException" クラス
 */
class tIllegalStateExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tIllegalStateExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
/**
 * "InaccessibleResourceException" クラス
 */
class tInaccessibleResourceExceptionClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	/**
	 * コンストラクタ
	 * @param engine	スクリプトエンジンインスタンス
	 */
	tInaccessibleResourceExceptionClass(tScriptEngine * engine);

	/**
	 * 各メンバをインスタンスに追加する
	 */
	void RegisterMembers();

public: // Risse用メソッドなど
	static void construct();
	static void initialize(const tNativeCallInfo & info);

public:
	/**
	 * 「リソースにアクセスできない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void Throw(tScriptEngine * engine) RISSE_NORETURN;
	/**
	 * 「リソースにアクセスできない」例外を発生
	 * @param engine	スクリプトエンジンインスタンス
	 */
	static void Throw() RISSE_NORETURN
		{ Throw(NULL); }
};
//---------------------------------------------------------------------------






} // namespace Risse
//---------------------------------------------------------------------------


#endif // #ifndef risseExceptionH




