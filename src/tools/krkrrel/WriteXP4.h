//---------------------------------------------------------------------------
/*
	TVP3 ( T Visual Presenter 3 )  A script authoring tool
	Copyright (C) 2000-2005  W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief XP4 アーカイブファイルの書き込み
//---------------------------------------------------------------------------

#ifndef _WRITEXP4_H_
#define _WRITEXP4_H_

#include "ProgressCallback.h"
#include "wxFileWrapper.h"
#include "ReadXP4Meta.h"

//---------------------------------------------------------------------------
// 使用する hash アルゴリズムに関する定義
//---------------------------------------------------------------------------
#define TVP_XP4_HASH_INIT						sha1_init
#define TVP_XP4_HASH_DO_PROCESS					sha1_process
#define TVP_XP4_HASH_DONE						sha1_done
#define TVP_XP4_HASH_DESC						sha1_desc
#define TVP_XP4_HASH_METHOD_STRING				"SHA1"
#define TVP_XP4_HASH_METHOD_INTERNAL_STRING		"sha1"
#define TVP_XP4_HASH_SIZE						20
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// ハッシュを表すクラス
//---------------------------------------------------------------------------
class tTVPXP4WriterHash
{
	unsigned char Hash[TVP_XP4_HASH_SIZE];
public:
	operator unsigned char *() { return Hash; }
	bool operator < (const tTVPXP4WriterHash & rhs) const
	{
		return memcmp(Hash, rhs.Hash, sizeof(Hash));
	}
	static int GetSize() { return TVP_XP4_HASH_SIZE; }
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		ストレージ内の各セグメントを表すクラス
//---------------------------------------------------------------------------
class tTVPXP4WriterStorage;
class tTVPXP4WriterSegment
{
	wxFileOffset Offset; //!< (非圧縮時の)ストレージ先頭からのオフセット
	wxFileOffset Size; //!< (非圧縮時の)サイズ
	bool IsCompressed; //!< ZLIB 圧縮の際に真

	wxFileOffset StoreOffset; //!< (実際に格納されている)オフセット
	wxFileOffset StoreSize; //!< (実際に格納されている)サイズ  無圧縮の場合は Size と同じ

public:
	tTVPXP4WriterSegment(
		wxFileOffset offset,
		wxFileOffset size,
		bool iscompressed);

	~tTVPXP4WriterSegment();

	wxFileOffset GetStoreOffset() const //!< (実際に格納された)オフセットを得る
		{ return StoreOffset; }
	wxFileOffset GetStoreSize() const //!< (実際に格納された)サイズを得る
		{ return StoreSize; }

	void WriteBody(iTVPProgressCallback * callback,
		wxFileEx & input, wxFileEx & file);

	void WriteMetaData(wxMemoryBuffer & buf);
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		XP4アーカイブへ格納するファイルのアイテム
//---------------------------------------------------------------------------
class tTVPXP4WriterInputFile : public tTVPXP4MetadataReaderStorageItem
{
protected:
	wxString InputName; //!< 入力ファイル名(ベースディレクトリ名部分をのぞく)
	wxString BaseDirName; //!< 入力ファイルのベースディレクトリ名

public:
	tTVPXP4WriterInputFile(
		const wxString & inarchivename,
		wxUint16 flags = 0,
		const wxDateTime & time = wxDateTime(),
		const wxString & inputname = wxEmptyString,
		const wxString & basedirname = wxEmptyString) :
				tTVPXP4MetadataReaderStorageItem(inarchivename, flags, time),
				InputName(inputname),
				BaseDirName(basedirname)
				 {;}  //!< コンストラクタ
	tTVPXP4WriterInputFile(const tTVPXP4MetadataReaderStorageItem & ref) :
		tTVPXP4MetadataReaderStorageItem(ref)
				 {;} //!< コンストラクタ

	bool operator < (const tTVPXP4WriterInputFile & rhs) const
	{
		// 比較用演算子
		return InputName < rhs.InputName;
	}

	const wxString & GetInputName() const { return InputName; } //!< 入力ファイル名を得る
	const wxString & GetBaseDirName() const { return BaseDirName; } //!< 入力ファイルのベースディレクトリ名を得る
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		アーカイブ内のストレージアイテムを表すクラス
//---------------------------------------------------------------------------
class tTVPXP4WriterStorage : public tTVPXP4WriterInputFile
{
	wxFileOffset Size; //!< (無圧縮時の)ファイルサイズ
	std::vector<tTVPXP4WriterSegment> SegmentVector; //!< セグメントの配列
	tTVPXP4WriterHash Hash; //!< ハッシュ
	bool IsReference; //!< 他のストレージアイテムを参照している場合は 真

public:
	tTVPXP4WriterStorage(
		const tTVPXP4WriterInputFile & inputfile);

	tTVPXP4WriterStorage(
		const tTVPXP4WriterInputFile & inputfile,
		const tTVPXP4WriterStorage & ref);

	wxFileOffset GetSize() const { return Size; } //!< サイズを得る
	const tTVPXP4WriterHash & GetHash() const { return Hash; } //!< ハッシュを得る

	void MakeHash(iTVPProgressCallback * callback);

	void WriteBody(iTVPProgressCallback * callback, wxFileEx & archive);

	void WriteMetaData(wxMemoryBuffer & buf);

	//! @brief ソート用比較関数
	bool operator < (const tTVPXP4WriterStorage &rhs) const
	{
		// サイズにおいて降順でソートするための関数
		// サイズは 2MB 単位で比較する
		wxFileOffset thissize =     Size / (2*1024*1024);
		wxFileOffset rhssize  = rhs.Size / (2*1024*1024);
		return thissize > rhssize;
	}
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		アーカイブを表すクラス
//---------------------------------------------------------------------------
class tTVPXP4WriterArchive
{
	std::vector<tTVPXP4WriterStorage> StorageVector; //!< このアーカイブ内に含まれるtTVPXP4WriterStorage の配列
	wxFileOffset Size; //!< アーカイブのサイズ
	wxString FileName; //!< アーカイブファイル名

public:
	tTVPXP4WriterArchive(const wxString & filename);
	wxFileOffset GetSize() const { return Size; } //!< アーカイブのファイルサイズを得る
	const tTVPXP4WriterStorage & GetStorageItem(size_t idx) { return StorageVector[idx]; } //!< 指定されたインデックスにあるストレージオブジェクトを得る
	size_t AddAndWriteBody(iTVPProgressCallback * callback,
		const tTVPXP4WriterStorage & storage);
	void WriteMetaData(iTVPProgressCallback * callback, bool compress);
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		XP4アーカイブへの書き込みを管理するクラス
//---------------------------------------------------------------------------
class tTVPXP4Writer
{
	iTVPProgressCallback * ProgressCallback; //!< コールバック用オブジェクト
	wxString BaseFileName;	//!< ベースファイル名(パス付きだが拡張子をのぞく)
	wxFileOffset  SplitLimit;	//!< 分割する際のファイルサイズの上限(バイト単位) 0=分割なし
	std::vector<tTVPXP4WriterInputFile> List; //!< 入力ファイルのリスト
	std::vector<tTVPXP4WriterArchive> ArchiveVector; //!< 各ボリュームファイルを表す配列


	unsigned int NewArchive();

public:
	tTVPXP4Writer(
		iTVPProgressCallback * callback,
		const wxString & basefilename,
		wxFileOffset splitlimit,
		const std::vector<tTVPXP4WriterInputFile> & list);
	~tTVPXP4Writer();

	void MakeArchive();
};
//---------------------------------------------------------------------------

#endif

