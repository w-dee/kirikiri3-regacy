//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risaで内部的に用いている例外クラスの管理など
//---------------------------------------------------------------------------
#ifndef _ExceptionH_
#define _ExceptionH_

#include "risse/include/risseExceptionClass.h"
#include "base/exception/UnhandledException.h"

namespace Risa {
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//! @brief		Risa用の汎用例外クラス
//---------------------------------------------------------------------------
class eRisaException
{
	//! このクラスはインスタンスを生成できない
	eRisaException();

public:
	//! @brief		eRisaException型の例外を投げる
	static void Throw(const tString &msg);

	//! @brief		eRisaException型の例外を投げる
	//! @param		str  文字列 (中に %1 などの指令を埋め込む)
	//! @param		s1   文字列中の %1 と置き換えたい文字列
	static void Throw(const tString &msg, const tString & s1);

	//! @brief		eRisaException型の例外を投げる
	//! @param		str  文字列 (中に %1 などの指令を埋め込む)
	//! @param		s1   文字列中の %1 と置き換えたい文字列
	//! @param		s2   文字列中の %2 と置き換えたい文字列
	static void Throw(const tString &msg, const tString & s1, const tString & s2);

	//! @brief		eRisaException型の例外を投げる
	//! @param		str  文字列 (中に %1 などの指令を埋め込む)
	//! @param		s1   文字列中の %1 と置き換えたい文字列
	//! @param		s2   文字列中の %2 と置き換えたい文字列
	//! @param		s3   文字列中の %3 と置き換えたい文字列
	static void Throw(const tString &msg, const tString & s1, const tString & s2, const tString & s3);

	//! @brief		eRisaException型の例外を投げる
	//! @param		str  文字列 (中に %1 などの指令を埋め込む)
	//! @param		s1   文字列中の %1 と置き換えたい文字列
	//! @param		s2   文字列中の %2 と置き換えたい文字列
	//! @param		s3   文字列中の %3 と置き換えたい文字列
	//! @param		s4   文字列中の %4 と置き換えたい文字列
	static void Throw(const tString &msg, const tString & s1, const tString & s2, const tString & s3, const tString & s4);

	//! @brief		内部エラー例外を発生させる
	//! @param		line     エラーの起こった行
	//! @param		filename エラーの起こったファイル名
	static void ThrowInternalErrorException(int line, const char * filename);
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
//! @brief		内部エラーをthrowするマクロ
//---------------------------------------------------------------------------
#define ThrowInternalError eRisaException::ThrowInternalErrorException(__LINE__, __FILE__)
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
//! @brief		例外を捕捉し、必要ならばエラー表示を行うマクロ
//---------------------------------------------------------------------------
#define RISA_CATCH_AND_SHOW_SCRIPT_EXCEPTION(origin) \
	catch(const tVariant * e) \
	{ \
		tUnhandledExceptionHandler::ShowScriptException(e); \
	} \


/*\
	catch(eRisseScriptException &e) \
	{ \
		e.AddTrace(tString(origin)); \
		tUnhandledExceptionHandler::Process(e); \
	} \
	catch(eRisseScriptError &e) \
	{ \
		e.AddTrace(tString(origin)); \
		tUnhandledExceptionHandler::Process(e); \
	} \
	catch(eRisse &e) \
	{ \
		tUnhandledExceptionHandler::Process(e); \
	} \
	catch(...) \
	{ \
		throw; \
	}*/
// TODO:implement this
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		例外を捕捉し、強制的にエラー表示を行うマクロ
//---------------------------------------------------------------------------
#define RISA_CATCH_AND_SHOW_SCRIPT_EXCEPTION_FORCE_SHOW_EXCEPTION(origin) \
	catch(...) { throw; }

/*\
	catch(eRisseScriptError &e) \
	{ \
		e.AddTrace(tString(origin)); \
		tUnhandledExceptionHandler::ShowScriptException(e); \
	} \
	catch(eRisse &e) \
	{ \
		tUnhandledExceptionHandler::ShowScriptException(e); \
	} \
	catch(...) \
	{ \
		throw; \
	}*/
// TODO: implement this
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
} // namespace Risa



#endif
