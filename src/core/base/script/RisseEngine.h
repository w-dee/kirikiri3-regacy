//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risseスクリプトエンジンの開始・終了・スクリプト実行などのインターフェース
//---------------------------------------------------------------------------
#ifndef RisseENGINEH
#define RisseENGINEH

#include "risse/include/risse.h"
#include "base/utils/Singleton.h"


//---------------------------------------------------------------------------
//! @brief		Risseスクリプトエンジンへのインターフェース
//---------------------------------------------------------------------------
class tRisaRisseScriptEngine : public singleton_base<tRisaRisseScriptEngine>
{
	tRisse *Engine;

public:
	tRisaRisseScriptEngine();
	~tRisaRisseScriptEngine();

	void Shutdown();

	tRisse * GetEngineNoAddRef() { return Engine; } //!< スクリプトエンジンを返す
	iRisseDispatch2 * GetGlobalNoAddRef()
		{ if(!Engine) return NULL; return Engine->GetGlobalNoAddRef(); } //!< スクリプトエンジンを返す
	void RegisterGlobalObject(const risse_char *name, iRisseDispatch2 * object);
	void EvalExpresisonAndPrintResultToConsole(const ttstr & expression);
	void ExecuteScript(const ttstr &script, tRisseVariant *result = NULL,
		iRisseDispatch2 *context = NULL,
		const ttstr *name = NULL, risse_int lineofs = 0);
};
//---------------------------------------------------------------------------




#endif
//---------------------------------------------------------------------------
