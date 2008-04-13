//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief フレームクラス
//---------------------------------------------------------------------------
#include "risa/prec.h"
#include "risa/packages/risa/widget/Frame.h"


namespace Risa {
RISSE_DEFINE_SOURCE_ID(4811,32114,33460,19785,53925,19531,59339,65072);
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
//! @brief		Risaのフレームを表す wxFrame 派生クラス用のイベントテーブル
//---------------------------------------------------------------------------
BEGIN_EVENT_TABLE(tFrame, wxFrame)
	EVT_CLOSE(						tFrame::OnClose)
END_EVENT_TABLE()
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
tFrame::tFrame(tFrameInternal * internal) : inherited(NULL, -1, wxT(""))
{
	Internal = internal;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tFrame::~tFrame()
{
	fprintf(stderr, "tFrame::~tFrame()\n");
	fflush(stderr);

}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrame::OnClose(wxCloseEvent & event)
{
	// onClose を呼び出す
	Internal->GetInstance()->Operate(ocFuncCall, NULL, tSS<'o','n','C','l','o','s','e'>(),
			0, tMethodArgument::New(!event.CanVeto()));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tFrame::Destroy()
{
	// Internal にウィンドウが破棄されたことを通知する
	Internal->NotifyDestroy();

	// Internal を一応切り離す
	Internal = NULL;

	// 親クラスのメソッドを呼び出す
	return inherited::Destroy();
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
tWindowList::tWindowList()
{
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tWindowList::~tWindowList()
{
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWindowList::Add(void * instance)
{
	volatile tCriticalSection::tLocker cs_holder(CS);
	RISSE_ASSERT(std::find(List.begin(), List.end(), instance) == List.end());
	List.push_back(instance);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWindowList::Remove(void * instance)
{
	volatile tCriticalSection::tLocker cs_holder(CS);
	gc_vector<void *>::iterator i;
	i = std::find(List.begin(), List.end(), instance);
	RISSE_ASSERT(i != List.end());
	List.erase(i);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
tFrameInternal::tFrameInternal(tFrameInstance * instance)
{
	Instance = instance;
	Frame = new tFrame(this);
	tWindowList::instance()->Add(this);

	Frame->Show();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tFrameInternal::~tFrameInternal()
{
	if(Instance)
		tWindowList::instance()->Remove(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrameInternal::NotifyDestroy()
{
	// tFrame からフレームの破棄が伝えられるとき。
	if(Instance)
	{
		tWindowList::instance()->Remove(this);
		Instance->NotifyDestroy(); // インスタンスにも通知する
		Instance = NULL; // インスタンスへの参照を断ち切る
		Frame.set(NULL); // フレームもすでに削除されることになっているので削除キューに登録する必要はない
	}
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
tFrameInstance::tFrameInstance()
{
	Internal = NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrameInstance::NotifyDestroy()
{
	fprintf(stderr, "tFrameInstance::NotifyDestroy()\n");
	fflush(stderr);

	Internal = NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tMainThreadAutoPtr<tFrame> & tFrameInstance::GetWxWindow() const
{
	volatile tSynchronizer sync(this); // sync
	if(!Internal) tInaccessibleResourceExceptionClass::Throw();

	return Internal->GetFrame();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrameInstance::construct()
{
	// 特にやること無し
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrameInstance::initialize(const tNativeCallInfo &info)
{
	volatile tSynchronizer sync(this); // sync

	// 親クラスの同名メソッドを呼び出す
	info.InitializeSuperClass();

	// フレームを作成
	if(Internal) Internal->GetFrame()->Destroy();
	Internal = new tFrameInternal(this);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void tFrameInstance::dispose()
{
	// TODO: 呼び出すスレッドのチェックまたはロック

	if(!Internal) tInaccessibleResourceExceptionClass::Throw();

	Internal->GetFrame()->Destroy();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrameInstance::close(const tMethodArgument &args)
{
	// TODO: 呼び出すスレッドのチェックまたはロック

	if(!Internal) tInaccessibleResourceExceptionClass::Throw();

	bool force = args.HasArgument(0) ? args[0].operator bool() : false;

	Internal->GetFrame()->Close(force);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrameInstance::onClose(bool force)
{
	// TODO: 呼び出すスレッドのチェックまたはロック

	// デフォルトの動作は dispose メソッドを呼び出すこと
	Operate(ocFuncCall, NULL, tSS<'d','i','s','p','o','s','e'>(),
			0, tMethodArgument::Empty());
}
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
tFrameClass::tFrameClass(tScriptEngine * engine) :
	tClassBase(tSS<'F','r','a','m','e'>(),
		tClassHolder<tEventSourceClass>::instance()->GetClass())
{
	RegisterMembers();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tFrameClass::RegisterMembers()
{
	// 親クラスの RegisterMembers を呼ぶ
	inherited::RegisterMembers();

	// クラスに必要なメソッドを登録する
	// 基本的に ss_construct と ss_initialize は各クラスごとに
	// 記述すること。たとえ construct の中身が空、あるいは initialize の
	// 中身が親クラスを呼び出すだけだとしても、記述すること。

	BindFunction(this, ss_ovulate, &tFrameClass::ovulate);
	BindFunction(this, ss_construct, &tFrameInstance::construct);
	BindFunction(this, ss_initialize, &tFrameInstance::initialize);
	BindFunction(this, tSS<'d','i','s','p','o','s','e'>(), &tFrameInstance::dispose);
	BindFunction(this, tSS<'c','l','o','s','e'>(), &tFrameInstance::close);
	BindFunction(this, tSS<'o','n','C','l','o','s','e'>(), &tFrameInstance::onClose);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tVariant tFrameClass::ovulate()
{
	return tVariant(new tFrameInstance());
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
//! @brief		Frame クラスレジストラ
//---------------------------------------------------------------------------
template class tClassRegisterer<
	tSS<'r','i','s','a','.','w','i','d','g','e','t'>,
	tFrameClass>;
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
} // namespace Risa


