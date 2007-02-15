//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief "Class" (クラス) の実装
//---------------------------------------------------------------------------
#include "prec.h"

#include "risseClass.h"
#include "risseClassClass.h"
#include "risseModuleClass.h"
#include "risseObjectClass.h"
#include "risseNativeFunction.h"
#include "risseNativeProperty.h"
#include "risseOpCodes.h"
#include "risseStaticStrings.h"

namespace Risse
{
RISSE_DEFINE_SOURCE_ID(28480,29035,20490,18954,3474,2858,57740,45280);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		NativeFunction: Class.construct
//---------------------------------------------------------------------------
static void Class_construct(RISSE_NATIVEFUNCTION_CALLEE_ARGS)
{
	// デフォルトでは何もしない
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		NativeFunction: Class.initialize
//---------------------------------------------------------------------------
static void Class_initialize(RISSE_NATIVEFUNCTION_CALLEE_ARGS)
{
	// 親クラスの同名メソッドを呼び出す
	// 引数は  { 親クラス, 名前 }
	if(args.HasArgument(1))
	{
		// 名前を渡す
		tRisseClassClass::GetPointer()->CallSuperClassMethod(NULL, ss_initialize, 0, tRisseMethodArgument::New(args[1]), This);
	}
	else
	{
		// 名前がないので引数無し
		tRisseClassClass::GetPointer()->CallSuperClassMethod(NULL, ss_initialize, 0, tRisseMethodArgument::Empty(), This);
	}

	if(args.HasArgument(0) && !args[0].IsNull())
	{
		// スーパークラスが指定されている
		// super を登録
		tRisseOperateFlags access_flags =
			tRisseOperateFlags::ofMemberEnsure|tRisseOperateFlags::ofInstanceMemberOnly;
		This.SetPropertyDirect(ss_super,
			tRisseOperateFlags(tRisseMemberAttribute(tRisseMemberAttribute::pcVar))|access_flags,
			args[0], This);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		NativeFunction: Class.include
//---------------------------------------------------------------------------
static void Class_include(RISSE_NATIVEFUNCTION_CALLEE_ARGS)
{
	// クラスの modules 配列にモジュールを追加する

	// modules を取り出す
	tRisseVariant modules = This.GetPropertyDirect(ss_modules, tRisseOperateFlags::ofInstanceMemberOnly);

	// Array.unshift を行う
	modules.Do(ocFuncCall, NULL, ss_unshift, 0, args);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseClassClass::tRisseClassClass() : tRisseClassBase(tRisseModuleClass::GetPointer())
{
	RegisterMembers();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseClassClass::RegisterMembers()
{
	// 親クラスの RegisterMembers を呼ぶ
	inherited::RegisterMembers();

	// クラスに必要なメソッドを登録する

	// construct, initialize などは新しいオブジェクトのコンテキスト上で実行されるので
	// コンテキストとしては null を指定する

	// construct
	RegisterNormalMember(ss_construct, tRisseVariant(tRisseNativeFunction::New(Class_construct)));
	// initialize
	RegisterNormalMember(ss_initialize, tRisseVariant(tRisseNativeFunction::New(Class_initialize)));

	// include
	RegisterNormalMember(ss_include, tRisseVariant(tRisseNativeFunction::New(Class_include)));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseClassClass::tRetValue tRisseClassClass::Operate(RISSE_OBJECTINTERFACE_OPERATE_IMPL_ARG)
{
	// ocCreateNewObjectBase の処理をオーバーライドする
	if(code == ocCreateNewObjectBase && name.IsEmpty())
	{
		// 親クラスの機能を呼ぶ
		tRetValue rv = inherited::Operate(RISSE_OBJECTINTERFACE_PASS_ARG);
		if(rv != rvNoError) return rv;

		// デフォルトのコンテキストを null に設定する。
		// 親クラスの ocCreateNewObjectBase ではデフォルトのコンテキストがそのクラス自身に
		// 設定されたはずだが(普通のインスタンスならばこれでよい)、
		// クラスインスタンスが返すデフォルトのコンテキストは NULL でなくてはならない。
		RISSE_ASSERT(result != NULL);
		result->Do(ocSetDefaultContext, NULL, tRisseString::GetEmptyString(), 0,
					tRisseMethodArgument::New(tRisseVariant::GetNullObject()));

		return rvNoError;
	}
	return inherited::Operate(RISSE_OBJECTINTERFACE_PASS_ARG);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseVariant tRisseClassClass::CreateNewObjectBase()
{
	return tRisseVariant(new tRisseClassInstance());
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
tRisseClassInstance::tRetValue tRisseClassInstance::Operate(RISSE_OBJECTINTERFACE_OPERATE_IMPL_ARG)
{
	// ocCreateNewObjectBase の処理をオーバーライドする
	if(code == ocCreateNewObjectBase && name.IsEmpty())
	{
		// 親クラス(tRisseClassClass)ではなく、tRisseClassBaseの機能を呼ぶ

		// 親クラスの tRisseClassClass はデフォルトのコンテキストを NULL にしてしまうが
		// それはこのクラスにおいては困るので tRisseClassBase の機能を呼ぶ
		return tRisseClassBase::Operate(RISSE_OBJECTINTERFACE_PASS_ARG);
	}
	return inherited::Operate(RISSE_OBJECTINTERFACE_PASS_ARG);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseVariant tRisseClassInstance::CreateNewObjectBase()
{
	return tRisseVariant(new tRisseObjectBase());
}
//---------------------------------------------------------------------------

} // namespace Risse
