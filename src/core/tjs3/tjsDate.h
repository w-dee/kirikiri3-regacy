//---------------------------------------------------------------------------
/*
	TJS3 Script Engine
	Copyright (C) 2000-2005  W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @brief Date クラス実装
//---------------------------------------------------------------------------

#ifndef tjsDateH
#define tjsDateH

#include <time.h>
#include "tjsNative.h"

namespace TJS
{
//---------------------------------------------------------------------------
class tTJSNI_Date : public tTJSNativeInstance
{
public:
	tTJSNI_Date();
	time_t DateTime;
private:
};

//---------------------------------------------------------------------------
class tTJSNC_Date : public tTJSNativeClass
{
public:
	tTJSNC_Date();

	static tjs_uint32 ClassID;

private:
	tTJSNativeInstance *CreateNativeInstance();
};
//---------------------------------------------------------------------------
} // namespace TJS

#endif
