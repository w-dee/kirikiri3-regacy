//---------------------------------------------------------------------------
/*
	TVP3 ( T Visual Presenter 3 )  A script authoring tool
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief XP4 メタデータの読み込み
//---------------------------------------------------------------------------
#ifndef _READXP4META_H_
#define _READXP4META_H_

#include "ProgressCallback.h"
#include "XP4Hash.h"
/*
	ここでは、XP4 アーカイブファイルのうち、アーカイブ内ファイルのファイル名と
	タイムスタンプ、フラグを得るだけを目的とする。
	実際の吉里吉里で使用されるルーチン群はパフォーマンスの最適化などで
	かなり複雑になるため、まったく別の実装とする。
*/


//---------------------------------------------------------------------------
//! @brief		アーカイブ内のストレージアイテムを現すクラス
//---------------------------------------------------------------------------
class tTVPXP4MetadataReaderStorageItem
{
protected:
	wxString InArchiveName; //!< ストレージ名
	wxUint16 Flags; //!< フラグ
	wxFileOffset Size; //!< (無圧縮時の)ファイルサイズ
	wxDateTime Time; //!< タイムスタンプ
	tTVPXP4Hash Hash; //!< ハッシュ

public:
	tTVPXP4MetadataReaderStorageItem(
		const wxString & inarchivename,
		wxUint16 flags = 0,
		wxFileOffset size = -1,
		const wxDateTime & time = wxDateTime()) :
		InArchiveName(inarchivename),
		Flags(flags),
		Size(size),
		Time(time)
		{;} //!< コンストラクタ

	tTVPXP4MetadataReaderStorageItem(const unsigned char * meta, size_t metasize);

	const wxString & GetInArchiveName() const { return InArchiveName; } //!< ストレージ名を得る
	wxFileOffset GetSize() const { return Size; } //!< サイズを得る
	wxUint16 GetFlags() const { return Flags; } //!< フラグを得る
	const wxDateTime & GetTime() const { return Time; } //!< タイムスタンプを得る
	void SetFlags(wxUint16 flags) { Flags = flags; } 
	const tTVPXP4Hash & GetHash() const { return Hash; } //!< ハッシュを得る
	tTVPXP4Hash & GetHash() { return Hash; } //!< ハッシュを得る
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		一つのアーカイブを現すクラス
//---------------------------------------------------------------------------
class tTVPXP4MetadataReaderArchive
{
	wxString TargetDir;
	std::vector<tTVPXP4MetadataReaderStorageItem> ItemVector; //!< ストレージの配列

public:
	tTVPXP4MetadataReaderArchive(const wxString & filename);

	const std::vector<tTVPXP4MetadataReaderStorageItem> &
		GetItemVector() const { return ItemVector; } //!< ストレージの配列を得る

	const wxString & GetTargetDir() const
		{ return TargetDir; } //!< このアーカイブの元となった対象ディレクトリを得る
};
//---------------------------------------------------------------------------
#endif
