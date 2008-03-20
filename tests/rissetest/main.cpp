// Risse テスト用プログラム

#include "prec.h"
#include "risseString.h"
#include "risseVariant.h"
#include "gc_cpp.h"
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/dir.h>



#include "risseScriptEngine.h"
#include "risseClass.h"
#include "risseStaticStrings.h"
#include "risseNativeBinder.h"
#include "risseStringTemplate.h"
#include "risseObjectClass.h"
#include "risseExceptionClass.h"

RISSE_DEFINE_SOURCE_ID(1760,7877,28237,16679,32159,45258,11038,1907);


using namespace Risse;

//---------------------------------------------------------------------------
//! @brief		パッケージ検索のためのインターフェース
//---------------------------------------------------------------------------
class tPackageFileSystemInterfaceImpl : public tPackageFileSystemInterface
{
public:
	virtual void List(const tString & dir, gc_vector<tString> & files)
	{
		wxString native_name(ConvertToNativePathDelimiter(dir.AsWxString()));
		wxDir dir_object;
		if(!dir_object.Open(native_name)) return; // ディレクトリを開けなかった
		wxString filename;
		bool cont;

		// ファイルを列挙
		cont = dir_object.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
		while(cont)
		{
			if(!filename.StartsWith(wxT(".")))
			{
				files.push_back(tString(filename));
			}
			cont = dir_object.GetNext(&filename);
		}
		// ディレクトリを列挙
		cont = dir_object.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
		while(cont)
		{
			if(!filename.StartsWith(wxT(".")))
			{
				files.push_back(tString(filename + wxT("/")));
			}
			cont = dir_object.GetNext(&filename);
		}
	}

	virtual int GetType(const tString & file)
	{
		wxString native_name(ConvertToNativePathDelimiter(file.AsWxString()));
		if(wxDirExists(native_name)) return 2;
		if(wxFileExists(native_name)) return 1;
		return 0;
	}

	virtual tString ReadFile(const tString & file)
	{
		// name を取ってきて eval する
		wxFile file_object;
		wxString native_name(ConvertToNativePathDelimiter(file.AsWxString()));
		if(file_object.Open(native_name))
		{
			// 内容を読み込む
			size_t length = file_object.Length();
			char *buf = new (PointerFreeGC) char [length + 1];
			file_object.Read(buf, length);
			buf[length] = 0;

			return tString(buf);
		}
		else
		{
			// TODO: IOException
			return tString();
		}
	}

	static wxString ConvertToNativePathDelimiter(const wxString & path)
	{
		wxString ret(path);
		if(ret.Length() > 0)
		{
			wxChar pathsep = static_cast<wxChar>(wxFileName::GetPathSeparator());
			for(size_t n = 0; ret.GetChar(n); n++)
			{
				if(ret.GetChar(n) == wxT('/'))
					ret.GetWritableChar(n) = pathsep;
			}
		}
		return ret;
	}
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		テスト用ネイティブ実装インスタンス
//---------------------------------------------------------------------------
class tTestInstance : public tObjectBase
{
	tString str;

	virtual ~tTestInstance() {;} //!< おそらく呼ばれないデストラクタ

public:
	void construct() {;}
	void initialize(const tNativeCallInfo & info)
	{
		// 親クラスの同名メソッドを呼び出す
		info.InitializeSuperClass();
	}

	const tString & getString() { return str; }
	const tString & get_propString() const { return str; }
	void set_propString(const tString & v) { str = v; }
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		"Test" クラス
//---------------------------------------------------------------------------
class tTestClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	//! @brief		コンストラクタ
	//! @param		engine		スクリプトエンジンインスタンス
	tTestClass(tScriptEngine * engine) :
		tClassBase(tSS<'T','e','s','t'>(), engine->ObjectClass)
	{
		RegisterMembers();
	}

	//! @brief		各メンバをインスタンスに追加する
	void RegisterMembers()
	{
		// 親クラスの RegisterMembers を呼ぶ
		inherited::RegisterMembers();

		// クラスに必要なメソッドを登録する
		// 基本的に ss_construct と ss_initialize は各クラスごとに
		// 記述すること。たとえ construct の中身が空、あるいは initialize の
		// 中身が親クラスを呼び出すだけだとしても、記述すること。

		BindFunction(this, ss_ovulate, &tTestClass::ovulate);
		BindFunction(this, ss_construct, &tTestInstance::construct);
		BindFunction(this, ss_initialize, &tTestInstance::initialize);
		BindFunction(this, tSS<'g','e','t','S','t','r','i','n','g'>(), &tTestInstance::getString);
		BindProperty(this, tSS<'p','r','o','p','S','t','r','i','n','g'>(),
			&tTestInstance::get_propString, &tTestInstance::set_propString);
	}

	//! @brief		newの際の新しいオブジェクトを作成して返す
	static tVariant ovulate()
	{
		return tVariant(new tTestInstance());
	}
public:
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
//! @brief		暫定 Script クラス
//---------------------------------------------------------------------------
class tScriptClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	//! @brief		コンストラクタ
	//! @param		engine		スクリプトエンジンインスタンス
	tScriptClass(tScriptEngine * engine) :
		tClassBase(tSS<'S','c','r','i','p','t'>(), engine->ObjectClass)
	{
		RegisterMembers();
	}

	//! @brief		各メンバをインスタンスに追加する
	void RegisterMembers()
	{
		BindFunction(this, ss_construct, &tScriptClass::construct);
		BindFunction(this, ss_initialize, &tScriptClass::initialize);
		BindFunction(this, tSS<'r','e','q','u','i','r','e'>(), &tScriptClass::require);
		BindFunction(this, tSS<'p','r','i','n','t'>(), &tScriptClass::print);
	}

	static void construct()
	{
	}

	static void initialize()
	{
	}

	static void require(const tString & name, tScriptEngine * engine)
	{
		// name を取ってきて eval する
		wxFile file;
		if(file.Open(name.AsWxString()))
		{
			// 内容を読み込む
			size_t length = file.Length();
			char *buf = new (PointerFreeGC) char [length + 1];
			file.Read(buf, length);
			buf[length] = 0;

			// 内容を評価する
			fflush(stderr);
			fflush(stdout);
			engine->Evaluate((tString)(buf), name, 0, NULL);
			fflush(stderr);
			fflush(stdout);
		}
		else
		{
			// TODO: IOException
		}
	}

	static void print(const tMethodArgument & args)
	{
		fflush(stderr);
		fflush(stdout);
		for(risse_size i = 0; i < args.GetArgumentCount(); i++)
			FPrint(stdout, args[i].operator tString().c_str());
		fflush(stderr);
		fflush(stdout);
	}

public:
};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
//! @brief		警告情報の出力先インターフェース
//---------------------------------------------------------------------------
class tWarningOutput : public tLineOutputInterface
{
	void Output(const tString & info)
	{
		fflush(stderr);
		fflush(stdout);
		FPrint(stderr, RISSE_WS("warning: "));
		FPrint(stderr, info.c_str());
		FPrint(stderr, RISSE_WS("\n"));
		fflush(stderr);
	}
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		アプリケーションクラス
//---------------------------------------------------------------------------
class Application : public wxAppConsole
{
public:
	virtual bool OnInit();
	virtual int OnRun();

private:
	wxLocale locale;
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// アプリケーションメインルーチン定義
//---------------------------------------------------------------------------
IMPLEMENT_APP_CONSOLE(Application)
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		アプリケーションが開始するとき
//! @return		成功すれば真
//---------------------------------------------------------------------------
bool Application::OnInit()
{
	return true;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
//! @brief		アプリケーションメインルーチン
//! @return		終了コード
//---------------------------------------------------------------------------
int Application::OnRun()
{
	if(argc < 2)
	{
		fprintf(stderr, "Specify a file name to read.\n");
		return 0;
	}

	// Risse スクリプトエンジンを作成する
	try
	{
		tScriptEngine engine;
		engine.SetAssertionEnabled(true);
		engine.SetWarningOutput(new tWarningOutput());
		engine.SetPackageFileSystem(new tPackageFileSystemInterfaceImpl());

		// Script クラスを追加する
		(new tScriptClass(&engine))->
				RegisterInstance(engine.GetRissePackageGlobal());

		// Test クラスを追加する
		(new tTestClass(&engine))->
				RegisterInstance(engine.GetRissePackageGlobal());

		// 入力ファイルを開く
		wxFile file;
		if(file.Open(argv[1]))
		{
			// スクリプトのある場所をカレントディレクトリに指定する
			wxFileName filename(argv[1]);
			wxFileName::SetCwd(filename.GetPath());

			// 内容を読み込む
			size_t length = file.Length();
			char *buf = new (PointerFreeGC) char [length + 1];
			file.Read(buf, length);
			buf[length] = 0;

			// 内容を評価する
			tVariant result;
			engine.Evaluate((tString)(buf), argv[1], 0, &result);
			FPrint(stderr,(RISSE_WS("========== Result ==========\n")));
			fflush(stderr);
			fflush(stdout);
			FPrint(stdout, result.AsHumanReadable().c_str());
		}

	}
	catch(const tTemporaryException * te)
	{
		te->Dump();
	}
	catch(const tVariant * e)
	{
		fflush(stderr);
		fflush(stdout);
		wxFprintf(stdout, wxT("exception: %s\n"), e->operator tString().AsWxString().c_str());
	}
	return 0;
}
//---------------------------------------------------------------------------

