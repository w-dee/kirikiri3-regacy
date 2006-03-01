//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief ログ管理
//---------------------------------------------------------------------------
#ifndef _LOG_H
#define _LOG_H

#include "risse/include/risse.h"
#include "base/utils/Singleton.h"
#include "base/utils/RingBuffer.h"
#include "base/utils/RisaThread.h"
#include <deque>
#include <wx/datetime.h>


class tRisaLogReceiver;
//---------------------------------------------------------------------------
//! @brief		ロガークラス(シングルトン)
//---------------------------------------------------------------------------
class tRisaLogger : public singleton_base<tRisaLogger>
{
public:
	//! @brief ログアイテムのレベル(ログレベル)
	enum tLevel
	{
		llDebug,	//!< デバッグに関する物
		llInfo,		//!< 情報通知
		llNotice,	//!< 通常状態だが大事な情報通知
		llWarning,	//!< 警告
		llError,	//!< 通常のエラー
		llRecord,	//!< 記録すべき重要な情報
		llCritical	//!< 致命的なエラー
	};

	//! @brief ログの１アイテム(１行) を表す構造体
	struct tItem
	{
		wxDateTime Timestamp; //!< ログが行われた日付時刻
		ttstr Content; //!< ログの内容
		ttstr Link; //!< リンク情報
		tLevel Level; //!< ログレベル
	};

private:
	tRisaCriticalSection CS; //!< このオブジェクトを保護するクリティカルセクション
	const static size_t MaxLogItems = 2048; //!< 最大のログ行数
	const static tLevel LogPreserveMinLevel = llError; //!< PreserveBuffer に入れる最小のログレベル


	tRisaRingBuffer<tItem> Buffer; //!< ログを格納するためのリングバッファ
	std::vector<tItem> PreserveBuffer; //!< ログからあふれたアイテムを保持し続けるバッファ
	std::vector<tRisaLogReceiver*> Receivers; //!< ログを受信するオブジェクト(レシーバ)の配列
	bool LogSending; //!< レシーバにログを送っている最中に真

public:
	tRisaLogger();
	~tRisaLogger();

public:
	const tRisaRingBuffer<tItem> & GetBuffer() const 
		{ return Buffer; } //!< Buffer への参照を得る

	void SendPreservedLogs(tRisaLogReceiver *target);
	void SendLogs(tRisaLogReceiver *target, size_t maxitems = static_cast<size_t>(-1L));

	void RegisterReceiver(tRisaLogReceiver * receiver);
	void UnregisterReceiver(tRisaLogReceiver * receiver);

private:
	void InternalLog(const ttstr & content, tLevel level = llInfo,
		const ttstr & linkinfo = RisseEmptyString);

public:
	//! @brief ログを行う
	//! @note 通常のログ記録にはこちらを使うこと
	static void Log(const ttstr & content, tLevel level = llInfo,
		const ttstr & linkinfo = RisseEmptyString)
	{
		if(tRisaLogger::alive())
			tRisaLogger::instance()->InternalLog(content, level, linkinfo);
	}
};
//---------------------------------------------------------------------------


class tRisaLogger;
//---------------------------------------------------------------------------
//! @brief		ログを受け取るためのインターフェース
//---------------------------------------------------------------------------
class tRisaLogReceiver
{
public:
	//! @brief ログが追加されるとき
	//! @param	item  ログアイテム
	//! @note  このメソッドは複数のスレッドから呼ばれることがある。
	//!			ただし、このメソッドが複数のスレッドから
	//!			「同時には呼ばれない」ことは tRisaLogger が保証する。
	//!			(もっとも他のメソッドは複数のスレッドから同時に呼ばれる
	//!			可能性はあるので、どっちにしろ厳重なスレッド保護を行った
	//!			ほうがよい)
	virtual void OnLog(const tRisaLogger::tItem & item) = 0;
};
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
//! @brief		wxWidgets のログを Risa のログ機構に流し込むためのクラス
//---------------------------------------------------------------------------
class tRisaWxLogProxy :
	public wxLog,
	public singleton_base<tRisaWxLogProxy>,
	depends_on<tRisaLogger>
{
	wxLog * OldLog; //!< このオブジェクトが作成される前に存在していたActiveなログ

public:
	tRisaWxLogProxy();
	~tRisaWxLogProxy();

protected:
	// implement sink function
	virtual void DoLog(wxLogLevel level, const wxChar *szString, time_t t);

	DECLARE_NO_COPY_CLASS(tRisaWxLogProxy)
};
//---------------------------------------------------------------------------


#endif
