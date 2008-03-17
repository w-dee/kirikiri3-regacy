//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief tVariantやtObjectInterfaceのOperateメソッドのflags引数の処理
//---------------------------------------------------------------------------
#ifndef risseOperateFlagsH
#define risseOperateFlagsH

#include "risseGC.h"
#include "risseMemberAttribute.h"

//---------------------------------------------------------------------------
namespace Risse
{
//---------------------------------------------------------------------------
class tOperateFlags : public tCollectee
{
	risse_uint32 Flags; //!< フラグの値
public:
	static const risse_uint32 ofMemberEnsure = 0x1000;
		//!< メンバが無かった場合に作成を行う(ocDSetのみ)
	static const risse_uint32 ofInstanceMemberOnly = 0x2000;
		//!< インスタンスメンバのみ参照(クラスのメンバを見に行かない)
		//!< (ocDSet/ocDGetやメンバを参照するもの全般)
	static const risse_uint32 ofFinalOnly = 0x4000;
		//!< finalメンバのみを探す。見つかった場合はrvMemberIsFinalが帰る。
		//!< 見つからなかった場合や、見つかってもfinalメンバではなかった場合は
		//!< rvMemberNotFound が帰る。
	static const risse_uint32 ofUseClassMembersRule = 0x8000;
		//!< 常にThisパラメータで渡されたインスタンスをコンテキストとして使う。
		//!< また、クラスやモジュールにおいては、
		//!< members の中身を見に行く (クラスそのものを見ない)

public:
	//! @brief		デフォルトコンストラクタ
	tOperateFlags() { Flags = 0; }

	//! @brief		コンストラクタ (tMemberAttribute から)
	//! @param		attrib		メンバ属性
	tOperateFlags(tMemberAttribute attrib)
		{ Flags = (risse_uint32)attrib; }

	//! @brief		コンストラクタ (フラグ/risse_uint32から)
	//! @param		flags		フラグ
	tOperateFlags(risse_uint32 flags)
		{ Flags = flags; }

	//! @brief		| 演算子
	//! @param		rhs		右辺
	tOperateFlags operator | (risse_uint32 rhs) const { return tOperateFlags(Flags | rhs); }

	//! @brief		& 演算子
	//! @param		rhs		右辺
	tOperateFlags operator & (risse_uint32 rhs) const { return tOperateFlags(Flags & rhs); }

	//! @brief		bool へのキャスト
	operator bool () const { return Flags != 0; }

	//! @brief		tMemberAttributeへのキャスト
	operator tMemberAttribute () const { return tMemberAttribute(Flags); }

	//! @brief		risse_uint32 へのキャスト
	operator risse_uint32() const { return Flags; }

	//! @brief		属性を持っているかどうかを調べる
	//! @param		v	変更性
	bool Has(tMemberAttribute::tMutabilityControl v) const { return tMemberAttribute(Flags).Has(v); }

	//! @brief		属性を持っているかどうかを調べる
	//! @param		v	オーバーライド性
	bool Has(tMemberAttribute::tOverrideControl v) const { return tMemberAttribute(Flags).Has(v); }

	//! @brief		属性を持っているかどうかを調べる
	//! @param		v	プロパティアクセス方法
	bool Has(tMemberAttribute::tPropertyControl v) const { return tMemberAttribute(Flags).Has(v); }

	//! @brief		フラグを持っているかどうかを調べる
	//! @param		v	フラグ
	bool Has(risse_uint32 v) const { return Flags & v; }

	//! @brief		フラグを文字列化する
	//! @return		文字列化されたフラグ
	tString AsString() const;


};
//---------------------------------------------------------------------------
}
#endif

