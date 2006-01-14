//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief 圧縮ブロックを展開された状態でメモリ上に保持するクラス
//---------------------------------------------------------------------------


#ifndef DECOMPRESSEDHOLDERH
#define DECOMPRESSEDHOLDERH




//---------------------------------------------------------------------------
//!@brief		圧縮ストリームを展開された状態で保持するオブジェクト
//---------------------------------------------------------------------------
class tRisaDecompressedHolder
{
private:
	risse_size Size; //!< このデータが保持する(無圧縮状態の)サイズ
	risse_uint8 * Data; //!< このデータが保持する(無圧縮状態の)データブロック

	tRisaDecompressedHolder::tRisaDecompressedHolder(const tRisaDecompressedHolder & ref) {;} // コピー禁止

public:
	enum tMethod 
	{
		dhmZLib // zlib 圧縮
	};

	tRisaDecompressedHolder(tMethod method, const risse_uint8 * indata, risse_size insize, 
		risse_size uncomp_size);
	tRisaDecompressedHolder(tMethod method, tRisseBinaryStream * instream, risse_size insize, 
		risse_size uncomp_size);
	~tRisaDecompressedHolder();

	risse_size GetSize() const { return Size; } //!< サイズを返す
	risse_uint8 * GetData() const { return Data; } //!< データブロックを返す

private:
	void Decompress(tMethod method, const risse_uint8 * indata, risse_size insize, 
		risse_size uncomp_size);
};
//---------------------------------------------------------------------------



#endif
