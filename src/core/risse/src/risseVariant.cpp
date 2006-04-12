//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief バリアント型の実装
//---------------------------------------------------------------------------
#include "prec.h"

#include "risseVariant.h"


namespace Risse
{
RISSE_DEFINE_SOURCE_ID(8265,43737,22162,17503,41631,46790,57901,27164);

//---------------------------------------------------------------------------
tRisseString tRisseVariantBlock::AsHumanReadable_Void     (risse_size maxlen) const
{
	return RISSE_WS("void");
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseVariantBlock::AsHumanReadable_Integer  (risse_size maxlen) const
{
	risse_char buf[40];
	Risse_int64_to_str(AsInteger(), buf);
	return buf;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseVariantBlock::AsHumanReadable_Real     (risse_size maxlen) const
{
	risse_char buf[25];
	Risse_real_to_str(AsReal(), buf);
	return buf;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseVariantBlock::AsHumanReadable_Bool     (risse_size maxlen) const
{
	return AsBool()?
		RISSE_WS("true"):
		RISSE_WS("false");
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
} // namespace Risse
