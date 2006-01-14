//---------------------------------------------------------------------------
/*
	TVP3 ( T Visual Presenter 3 )  A script authoring tool
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief アーカイブファイルのセグメントのキャッシュ
//---------------------------------------------------------------------------


#ifndef XP4SEGMENTCACHEH
#define XP4SEGMENTCACHEH


#include <boost/smart_ptr.hpp>
#include "DecompressedHolder.h"
#include "Singleton.h"



//---------------------------------------------------------------------------
//!@brief		セグメントキャッシュクラス
//---------------------------------------------------------------------------
class tTVPXP4SegmentCache
{
	static const risse_size ONE_LIMIT = 1024*1024; //!< これを超えるセグメントはキャッシュしない
	static const risse_size TOTAL_LIMIT = 1024*1024; //!< トータルでこれ以上はキャッシュしない

	tRisseCriticalSection CS; //!< このオブジェクトを保護するクリティカルセクション
	risse_size TotalBytes; //!< このクラスが保持しているトータルのバイト数

	//! @brief キャッシュアイテムのkeyとなる構造体
	struct tKey
	{
		void * Pointer; //!< アーカイブインスタンスへのポインタ
		risse_size StorageIndex; //!< storage index in archive
		risse_size SegmentIndex; //!< segment index in storage
		bool operator == (const tKey & rhs) const
			{ return Pointer == rhs.Pointer &&
				StorageIndex == rhs.StorageIndex &&
				SegmentIndex == rhs.SegmentIndex; }
	};

	//! @brief tKey のハッシュを作成するクラス
	class tKeyHasher
	{
	public:
		static risse_uint32 Make(const tKey &val)
		{
			risse_uint32 v;
			v  = reinterpret_cast<risse_uint32>(val.Pointer);
			v ^= v >> 4;
			v ^= static_cast<risse_uint32>(val.StorageIndex);
			v ^= static_cast<risse_uint32>(val.SegmentIndex<<2);
			return v;
		}
	};

public:
	typedef boost::shared_ptr<tTVPDecompressedHolder> tDataBlock; //!< キャッシュアイテムのvalueのtypedef

private:
	typedef tRisseHashTable<tKey, tDataBlock, tKeyHasher> tHashTable; //!< ハッシュテーブルのtypedef

	tHashTable HashTable; //!< ハッシュテーブル

public:
	tTVPXP4SegmentCache();
	~tTVPXP4SegmentCache();

private:
	tTVPSingletonObjectLifeTracer<tTVPXP4SegmentCache> singleton_object_life_tracer;
public:
	static boost::shared_ptr<tTVPXP4SegmentCache> & instance() { return
		tTVPSingleton<tTVPXP4SegmentCache>::instance();
			} //!< このシングルトンのインスタンスを返す

public:
	void CheckLimit();
	void Clear();
	tDataBlock
		Find(void * pointer, risse_size storage_index, risse_size segment_index,
			tRisseBinaryStream * instream, risse_uint64 dataofs, risse_size insize,
			risse_size uncomp_size);
};
//---------------------------------------------------------------------------



#endif
