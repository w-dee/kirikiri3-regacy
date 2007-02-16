/*---------------------------------------------------------------------------*/
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Risse用 "String" クラスの実装
//---------------------------------------------------------------------------

#include "risseTypes.h"
#include "risseStringClass.h"
#include "risseNativeFunction.h"
#include "risseNativeProperty.h"
#include "risseStaticStrings.h"
#include "risseObjectClass.h"

/*
	Risseスクリプトから見える"String" クラスの実装
*/

namespace Risse
{
RISSE_DEFINE_SOURCE_ID(44706,36741,55501,19515,15528,60571,63357,21717);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
tRisseStringClass::tRisseStringClass() : tRissePrimitiveClassBase(tRissePrimitiveClass::GetPointer())
{
	RegisterMembers();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseStringClass::RegisterMembers()
{
	// 親クラスの RegisterMembers を呼ぶ
	inherited::RegisterMembers();

	// クラスに必要なメソッドを登録する

	// construct は tRissePrimitiveClass 内ですでに登録されている

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	RISSE_BEGIN_NATIVE_METHOD(ss_initialize)
	{
		// 親クラスの同名メソッドは「呼び出されない」

		// 引数をすべて連結した物を初期値に使う
		// 注意: いったん CreateNewObjectBase で作成されたオブジェクトの中身
		//       を変更するため、const_cast を用いる
		for(risse_size i = 0; i < args.GetArgumentCount(); i++)
			*const_cast<tRisseVariant*>(&This) += args[i];
	}
	RISSE_END_NATIVE_METHOD

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	RISSE_BEGIN_NATIVE_METHOD(ss_charAt)
	{
		if(args.GetArgumentCount() < 1) RisseThrowBadArgumentCount(args.GetArgumentCount(), 1);

		if(result)
		{
			const tRisseString & str = This.operator tRisseString();
			risse_offset index = (risse_int64)args[0];
			if(index < 0) index += str.GetLength();
			if(index < 0 || static_cast<risse_size>(index) >= str.GetLength())
				result->Clear(); // 値が範囲外なので void を返す
			else
				*result = tRisseString(str, static_cast<risse_size>(index), 1);
		}
	}
	RISSE_END_NATIVE_METHOD

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	RISSE_BEGIN_NATIVE_PROPERTY(ss_length)
	{
		RISSE_BEGINE_NATIVE_PROPERTY_GETTER
		{
			if(result) *result = (risse_int64)This.operator tRisseString().GetLength();
		}
		RISSE_END_NATIVE_PROPERTY_GETTER
	}
	RISSE_END_NATIVE_PROPERTY

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseVariant tRisseStringClass::CreateNewObjectBase()
{
	return tRisseVariant(tRisseString());
}
//---------------------------------------------------------------------------

} /* namespace Risse */

