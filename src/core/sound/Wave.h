//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Wave(PCM) のプリミティブ型などの定義
//---------------------------------------------------------------------------

#include "risseTypes.h"

//---------------------------------------------------------------------------
// PCMサンプル型の定義
//---------------------------------------------------------------------------
//! @brief  PCMサンプル型
struct tRisaPCMTypes
{
	enum tType
	{
		tunknown = -1, //!< 無効
		ti8  = 0,		//!< 8bit integer linear PCM type
		ti16,			//!< 16bit integer linear PCM type
		ti24,			//!< 24bit integer linear PCM type
		ti32,			//!< 32bit integer linear PCM type
		tf32,			//!< 32bit float linear PCM type
		tnum			//!< 有効な型の数
	};

	//! @brief  8bit integer linear PCM type
	struct i8
	{
		static const int id = ti8;
		static const size_t size = 1;
		static const risse_int32 max_value= 127;
		static const risse_int32 min_value=-128;

		risse_uint8 value;

		i8(risse_int32 v) { value = (risse_uint8)(v + 0x80); }
		void operator = (risse_int32 v) { value = (risse_uint8)(v + 0x80); }
		operator risse_int32 () const { return (risse_int32)value - 0x80; }
		risse_int32 geti32() const { return (risse_int32)(value - 0x80) << 24; }
		void seti32(risse_int32 v) { value = (risse_uint8)((v >> 24) + 0x80); }
	};

	//! @brief  16bit integer linear PCM type
	struct i16
	{
		static const int id = ti16;
		static const size_t size = 2;
		static const risse_int32 max_value= 32767;
		static const risse_int32 min_value=-32768;

		risse_int16 value;

		i16(risse_int32 v) { value = (risse_int16)v; }
		void operator = (risse_int32 v) { value = (risse_int16)v ; }
		operator risse_int32 () const { return (risse_int32)value; }
		risse_int32 geti32() const { return (risse_int32)value << 16; }
		void seti32(risse_int32 v) { value = (risse_uint32)(value >> 16); }
	};

	//! @brief  24bit integer linear PCM type
	struct i24
	{
		static const int id = ti24;
		static const size_t size = 3;
		static const risse_int32 max_value= 8388607;
		static const risse_int32 min_value=-8388608;

		risse_uint8 value[3];
		i24(risse_int32 v)
		{
			operator = (v);
		}
		void operator =(risse_int32 v)
		{
	#if RISSE_HOST_IS_BIG_ENDIAN
			value[0] = (v & 0xff0000) >> 16;
			value[1] = (v & 0x00ff00) >> 8;
			value[2] = (v & 0x0000ff);
	#else
			value[0] = (v & 0x0000ff);
			value[1] = (v & 0x00ff00) >> 8;
			value[2] = (v & 0xff0000) >> 16;
	#endif
		}
		operator risse_int32 () const
		{
			risse_int t;
	#if RISSE_HOST_IS_BIG_ENDIAN
			t = ((risse_int32)value[0] << 16) + ((risse_int32)value[1] << 8) + ((risse_int32)value[2]);
	#else
			t = ((risse_int32)value[2] << 16) + ((risse_int32)value[1] << 8) + ((risse_int32)value[0]);
	#endif
			t |= -(t&0x800000); // extend sign
			return t;
		}
		risse_int32 geti32() const { return (risse_int32)*this << 8; }
		void seti32(risse_int32 v) { *this = (risse_uint32)(v >> 8); }
	};

	//! @brief  32bit integer linear PCM type
	struct i32
	{
		static const int id = ti32;
		static const size_t size = 4;
		static const risse_int32 max_value= 2147483647;
		static const risse_int32 min_value=-2147483647-1;

		risse_int32 value;

		i32(risse_int32 v) { value = v; }
		void operator = (risse_int32 v) { value = v ; }
		operator risse_int32 () const { return value; }
		risse_int32 geti32() const { return value; }
		void seti32(risse_int32 v) { value = v; }
	};

	//! @brief  32bit float linear PCM type
	struct f32
	{
		static const int id = tf32;
		static const size_t size = 4;
		static const risse_int32 max_value= 2147483647;
		static const risse_int32 min_value=-2147483647-1;

		float value;

		f32(risse_int32 v) { value = (float)v / (float)max_value; }
		void operator = (risse_int32 v) { value = (float)v * (1.0f/ (float)max_value); }
		operator risse_int32 () const
		{
			if(value > 1.0f) return max_value;
			if(value < 1.0f) return min_value;
			if       (value < 0)   return (risse_int32)(value * max_value - 0.5);
			else /*if(value > 0)*/ return (risse_int32)(value * max_value + 0.5);
		}
		risse_int32 geti32() const { return (risse_int32)*this; }
		void seti32(risse_int32 v) { *this = (risse_int32)v; }
	};

};
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
//! @brief		スピーカー定義
//---------------------------------------------------------------------------
#ifndef SPEAKER_FRONT_LEFT
// from Windows ksmedia.h
// speaker config

#define     SPEAKER_FRONT_LEFT              0x1
#define     SPEAKER_FRONT_RIGHT             0x2
#define     SPEAKER_FRONT_CENTER            0x4
#define     SPEAKER_LOW_FREQUENCY           0x8
#define     SPEAKER_BACK_LEFT               0x10
#define     SPEAKER_BACK_RIGHT              0x20
#define     SPEAKER_FRONT_LEFT_OF_CENTER    0x40
#define     SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
#define     SPEAKER_BACK_CENTER             0x100
#define     SPEAKER_SIDE_LEFT               0x200
#define     SPEAKER_SIDE_RIGHT              0x400
#define     SPEAKER_TOP_CENTER              0x800
#define     SPEAKER_TOP_FRONT_LEFT          0x1000
#define     SPEAKER_TOP_FRONT_CENTER        0x2000
#define     SPEAKER_TOP_FRONT_RIGHT         0x4000
#define     SPEAKER_TOP_BACK_LEFT           0x8000
#define     SPEAKER_TOP_BACK_CENTER         0x10000
#define     SPEAKER_TOP_BACK_RIGHT          0x20000

#endif
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
//! @brief		PCM データフォーマット (内部用)
//---------------------------------------------------------------------------
struct tRisaWaveFormat
{
	risse_uint Frequency;				//!< sample granule per sec
	risse_uint Channels;				//!< number of channels (1=Mono, 2=Stereo ... etc)
	risse_uint BitsPerSample;			//!< bits per one sample
	risse_uint BytesPerSample;			//!< bytes per one sample
	risse_uint64 TotalSampleGranules;	//!< total samples in sample granule; unknown for zero
	risse_uint64 TotalTime;				//!< in ms; unknown for zero
	risse_uint32 SpeakerConfig;			//!< bitwise OR of SPEAKER_* constants
	bool IsFloat;						//!< true if the data is IEEE floating point
	bool Seekable;						//!< true if able to seek, otherwise false

	tRisaPCMTypes::tType GetRisaPCMType() const;
};
//---------------------------------------------------------------------------






