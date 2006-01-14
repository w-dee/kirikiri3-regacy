//---------------------------------------------------------------------------
/*
	TVP3 ( T Visual Presenter 3 )  A script authoring tool
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief アーカイブファイルのストリームのキャッシュ
//---------------------------------------------------------------------------

#ifndef XP4STREAMCACHEH
#define XP4STREAMCACHEH

#include "Singleton.h"

//---------------------------------------------------------------------------
//!@brief		ストリームキャッシュクラス
//---------------------------------------------------------------------------
class tTVPXP4StreamCache
{
	static const int MAX_ITEM = 8; //!< キャッシュするハンドル数

	tRisseCriticalSection CS; //!< このオブジェクトを保護するクリティカルセクション
	risse_uint Age; //!< キャッシュ世代

	//!@brief キャッシュアイテムの構造体
	struct tItem
	{
		void * Pointer; //!< アーカイブインスタンスへのポインタ
		tRisseBinaryStream * Stream; //!< 入力ストリームオブジェクト
		risse_uint Age; //!< キャッシュ世代
	} Pool[MAX_ITEM];

public:
	tTVPXP4StreamCache();
	~tTVPXP4StreamCache();

private:
	tTVPSingletonObjectLifeTracer<tTVPXP4StreamCache> singleton_object_life_tracer;
public:
	static boost::shared_ptr<tTVPXP4StreamCache> & instance() { return
		tTVPSingleton<tTVPXP4StreamCache>::instance();
			} //!< このシングルトンのインスタンスを返す

public:
	tRisseBinaryStream * GetStream(void * pointer, const ttstr & name);
	void ReleaseStream(void * pointer, tRisseBinaryStream * stream);
	void ReleaseStreamByPointer(void * pointer);
	void ReleaseAll();
	void Clear() { ReleaseAll(); }
};
//---------------------------------------------------------------------------



#endif
