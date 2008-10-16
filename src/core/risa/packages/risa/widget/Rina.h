//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief フレームクラス
//---------------------------------------------------------------------------
#ifndef WIDGETRINAH
#define WIDGETRINAH

#include "risa/common/RisaException.h"
#include "risa/packages/risa/event/Event.h"
#include "risa/common/RisaGC.h"
#include "risa/packages/risa/widget/Frame.h"
#include "risa/packages/risa/graphic/rina/rinaNode.h"
#include "risa/packages/risa/graphic/rina/rinaImageEdge.h"

namespace Risa {
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		Rina widget 用のノード
//---------------------------------------------------------------------------
class tRinaWidgetNodeInstance : public tNodeInstance
{
	typedef tNodeInstance inherited; //!< 親クラス

	tInputPinArrayInstance * InputPinArrayInstance; //!< 入力ピン配列インスタンス
	tOutputPinArrayInstance * OutputPinArrayInstance; //!< 出力ピン配列インスタンス
	tImageInputPinInstance * InputPinInstance; //!< 入力ピンインスタンス

public:
	//! @brief		コンストラクタ
	tRinaWidgetNodeInstance();

	//! @brief		デストラクタ(おそらく呼ばれない)
	virtual ~tRinaWidgetNodeInstance() {}

public: // サブクラスで実装すべき物
	//! @brief		入力ピンの配列を得る
	//! @return		入力ピンの配列
	virtual tInputPinArrayInstance & GetInputPinArrayInstance();

	//! @brief		出力ピンの配列を得る
	//! @return		出力ピンの配列
	virtual tOutputPinArrayInstance & GetOutputPinArrayInstance();

public: // Risse用メソッドなど
	void construct();
	void initialize(const tNativeCallInfo &info);
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		"RinaWidgetNode" クラス
//---------------------------------------------------------------------------
class tRinaWidgetNodeClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	//! @brief		コンストラクタ
	//! @param		engine		スクリプトエンジンインスタンス
	tRinaWidgetNodeClass(tScriptEngine * engine);

	//! @brief		各メンバをインスタンスに追加する
	void RegisterMembers();

	//! @brief		newの際の新しいオブジェクトを作成して返す
	static tVariant ovulate();

public: // Risse 用メソッドなど
};
//---------------------------------------------------------------------------










class tRinaInstance;
class tImageInstance;
//---------------------------------------------------------------------------
//! @brief		Rinaコントロールを表す wcControl 派生クラス
//---------------------------------------------------------------------------
class tRina : public wxControl, public tRisaWindowBahavior<tRina, tRinaInstance>
{
	typedef wxControl inherited;

	tImageInstance * TestImage; //!< テスト用イメージ

public:
	//! @brief		コンストラクタ
	//! @param		instance	Rinaクラスのインスタンスへのポインタ
	//! @param		parent		親コントロール
	tRina(tRinaInstance * internal, wxWindow * parent);

	//! @brief		デストラクタ
	~tRina();

public:
private:
	//! @brief		イベントテーブルの定義
	DECLARE_EVENT_TABLE()

protected: // イベント
	//! @brief		内容をペイントするとき
	void OnPaint(wxPaintEvent& event);
};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
//! @brief		Rinaコントロールクラスのインスタンス
//---------------------------------------------------------------------------
class tRinaInstance : public tWindowInstance
{
private:
	tRina::tInternal * Internal; //!< 内部実装クラスへのポインタ
	tRinaWidgetNodeInstance * RinaWidgetNode; //!< RinaWidgetNode インスタンス

public:
	//! @brief		コンストラクタ
	tRinaInstance();

	//! @brief		デストラクタ(おそらく呼ばれない)
	virtual ~tRinaInstance() {;}

public: // Risse用メソッドなど
	void construct();
	void initialize(const tVariant & parent, const tVariant & graph, const tNativeCallInfo &info);
};
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
//! @brief		"Rina" クラス
//---------------------------------------------------------------------------
class tRinaClass : public tClassBase
{
	typedef tClassBase inherited; //!< 親クラスの typedef

public:
	//! @brief		コンストラクタ
	//! @param		engine		スクリプトエンジンインスタンス
	tRinaClass(tScriptEngine * engine);

	//! @brief		各メンバをインスタンスに追加する
	void RegisterMembers();

	//! @brief		newの際の新しいオブジェクトを作成して返す
	static tVariant ovulate();
};
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
} // namespace Risa


#endif
