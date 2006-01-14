//---------------------------------------------------------------------------
/*
	Risa [è³]      alias g¢g¢3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risseã¹ã¯ãªããã¨ã³ã¸ã³ã®éå§ã»çµäºã»ã¹ã¯ãªããå®è¡ãªã©ã®ã¤ã³ã¿ã¼ãã§ã¼ã¹
//---------------------------------------------------------------------------
#ifndef RisseENGINEH
#define RisseENGINEH

#include "risse.h"
#include "Singleton.h"


//---------------------------------------------------------------------------
//! @brief		Risseã¹ã¯ãªããã¨ã³ã¸ã³ã¸ã®ã¤ã³ã¿ã¼ãã§ã¼ã¹
//---------------------------------------------------------------------------
class tRisaRisseScriptEngine
{
	tRisse *Engine;

public:
	tRisaRisseScriptEngine();
	~tRisaRisseScriptEngine();

private:
	tRisaSingletonObjectLifeTracer<tRisaRisseScriptEngine> singleton_object_life_tracer;
public:
	static boost::shared_ptr<tRisaRisseScriptEngine> & instance() { return
		tRisaSingleton<tRisaRisseScriptEngine>::instance();
			} //!< ãã®ã·ã³ã°ã«ãã³ã®ã¤ã³ã¹ã¿ã³ã¹ãè¿ã

	void Shutdown();

	tRisse * GetEngineNoAddRef() { return Engine; } //!< ã¹ã¯ãªããã¨ã³ã¸ã³ãè¿ã
	iRisseDispatch2 * GetGlobalNoAddRef()
		{ if(!Engine) return NULL; return Engine->GetGlobalNoAddRef(); } //!< ã¹ã¯ãªããã¨ã³ã¸ã³ãè¿ã
	void RegisterGlobalObject(const risse_char *name, iRisseDispatch2 * object);

};
//---------------------------------------------------------------------------




#endif
//---------------------------------------------------------------------------
