//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risa (吉里吉里3) メインルーチン
//---------------------------------------------------------------------------
#include "prec.h"
#include <wx/filename.h>
#include <wx/app.h>
#include "base/event/Event.h"
#include "base/event/IdleEvent.h"
#include "base/event/TickCount.h"
#include "base/script/RisseEngine.h"
#include "base/ui/console/Console.h"
#include "base/ui/editor/ScriptEditor.h"
#include "base/log/Log.h"

RISSE_DEFINE_SOURCE_ID(17420,39507,42749,18842,4255,44341,64162,32476);


//---------------------------------------------------------------------------
//! @brief アプリケーションクラス
//---------------------------------------------------------------------------
class tRisaApplication : public wxApp
{
public:
	bool OnInit();
	int OnExit();
	bool ProcessIdle();

private:
	void OnActivate(wxActivateEvent & event);
	void OnActivateApp(wxActivateEvent & event);

	DECLARE_EVENT_TABLE()
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// アプリケーションメインルーチン定義
//---------------------------------------------------------------------------
IMPLEMENT_APP(tRisaApplication)
wxLocale locale;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// アプリケーションイベントテーブル
//---------------------------------------------------------------------------
BEGIN_EVENT_TABLE(tRisaApplication, wxApp)
	EVT_ACTIVATE(			tRisaApplication::OnActivate)
	EVT_ACTIVATE_APP(		tRisaApplication::OnActivateApp)
END_EVENT_TABLE()
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		アプリケーションが開始するとき
//---------------------------------------------------------------------------
bool tRisaApplication::OnInit()
{
	// アプリケーションのディレクトリを得て、その下にある locale か、
	// <アプリケーションのあるディレクトリ>../share/locale をメッセージカタログ
	// の検索パスに指定する
	// wxApp::argv[0] がアプリケーションのファイル名を表していると仮定する
	wxFileName appfilename(argv[0]);
	wxString appdir = appfilename.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);

	locale.Init(wxLANGUAGE_DEFAULT);
	locale.AddCatalogLookupPathPrefix(appdir + wxT("locale")); 
	locale.AddCatalogLookupPathPrefix(appdir + wxT("../share/locale")); 

	// メッセージカタログを追加する
	locale.AddCatalog(wxT("k3"));
	locale.AddCatalog(wxT("wxstd"));

	// UpdateUIEvent と IdleEvent を指定されたウィンドウだけに送るように
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);
	wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);

	// すべてのシングルトンインスタンスを初期化する
	try
	{
		// 先だって初期化しておきたい物
		tRisaWxLogProxy::ensure();

		// 残り全てのシングルトンインスタンスを初期化
		singleton_manager::init_all();
	}
	catch(...)
	{
		// something ??
		fprintf(stderr, "something caught\n");
		return false;
	}

	//---- ↓↓テストコード↓↓ ----
	// ファイルシステムのルートにカレントディレクトリをマウント
	tRisaRisseScriptEngine::instance()->GetEngineNoAddRef()->EvalExpression(
		RISSE_WS("FileSystem.mount('/', new FileSystem.OSFS('.'))"),
		NULL, NULL, NULL);

	// コンソールをメインウィンドウとして表示
	tRisaConsoleFrame *console = new tRisaConsoleFrame();
//	wxFrame *frame = new wxFrame(NULL, wxID_ANY, wxT("hoge"));
	console->Show(true);

	tRisaScriptEditorFrame *editor = new tRisaScriptEditorFrame();
	editor->Show(true);

	//---- ↑↑テストコード↑↑ ----

	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		アプリケーションが終了するとき
//---------------------------------------------------------------------------
int tRisaApplication::OnExit()
{
	printf("tRisaApplication::OnExit entered\n");

	// スクリプトエンジンをシャットダウンする
	tRisaRisseScriptEngine::instance()->Shutdown();

	// すべてのシングルトンインスタンスへの参照を切る
	singleton_manager::disconnect_all();

	printf("all singletons should be destroyed within this time ...\n");

	// まだシステムに残っているシングルトンインスタンスを表示する
	singleton_manager::report_alive_objects();

	printf("tRisaApplication::OnExit ended\n");

	return 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		Idleイベントを処理するとき
//---------------------------------------------------------------------------
bool tRisaApplication::ProcessIdle()
{
	bool cont = false;
	if(tRisaTickCount::alive())
	{
		// この回で呼び出すハンドラに渡すtickを得る
		risse_uint64 tick = tRisaTickCount::instance()->Get();

		// 各サブシステムを呼び出す

		// イベントの配信
		if(tRisaEventSystem::alive())
			cont = tRisaEventSystem::instance()->ProcessEvents(tick) || cont;

		// アイドルイベントの配信
		if(tRisaIdleEventManager::alive())
			cont = tRisaIdleEventManager::instance()->Deliver(tick) || cont;
	}
	cont = wxApp::ProcessIdle() || cont;
	return cont;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		ウィンドウなどがアクティブ化/非アクティブ化したとき
//---------------------------------------------------------------------------
void tRisaApplication::OnActivate(wxActivateEvent & event)
{
	if(!event.GetActive() && tRisaCompactEventManager::alive())
	{
		tRisaCompactEventManager::instance()->OnDeactivate();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		アプリケーションがアクティブ化/非アクティブ化したとき
//---------------------------------------------------------------------------
void tRisaApplication::OnActivateApp(wxActivateEvent & event)
{
	if(!event.GetActive() && tRisaCompactEventManager::alive())
	{
		tRisaCompactEventManager::instance()->OnDeactivateApp();
	}
}
//---------------------------------------------------------------------------











namespace boost
{

//---------------------------------------------------------------------------
//! @brief		boost の assertion failure を捕捉する
//---------------------------------------------------------------------------
void assertion_failed(char const * expr, char const * function, char const * file, long line)
{
	// user defined
	fprintf(stderr, "boost assertion failure on expression '%s' at function '%s' file %s line %ld\n",
		expr, function, file, line);
	abort();
}
//---------------------------------------------------------------------------


} // namespace boost

