//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"

	Rina stands for "Rina is an Imaging Network Assembler"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief テスト用のナローテキストプロバイダノード管理
//---------------------------------------------------------------------------
#include "prec.h"
#include "rinaNarrowTextProviderNode.h"
#include "rinaNarrowTextEdge.h"

namespace Rina {
RISSE_DEFINE_SOURCE_ID(12623,61035,35256,19759,45714,27702,3711,32168);
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
tNarrowTextProviderNode::tNarrowTextProviderNode(tGraph * graph) : inherited(graph)
{
	Caption = "";
	// 出力ピンを作成
	OutputPin = new tNarrowTextOutputPin();
	OutputPin->Attach(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tNarrowTextProviderNode::SetCaption(const char * caption)
{
	volatile tGraphLocker lock(*this); 

	risse_size length_was = strlen(Caption);

	size_t len = strlen(caption);
	char * newbuf = static_cast<char *>(MallocAtomicCollectee(len + 1));
	strcpy(newbuf, caption);
	Caption = newbuf;

	// キャプションが変わると前の長さと新しい長さのどちらか長い方分までが更新される
	risse_size length_is  = len;
	OutputPin->NotifyUpdate(t1DArea(0, std::max(length_was, length_is)));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_size tNarrowTextProviderNode::GetOutputPinCount()
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	return 1; // 出力ピンは１個
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tOutputPin * tNarrowTextProviderNode::GetOutputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// TODO: 例外
	if(n == 0) return OutputPin;
	return NULL; // 出力ピンはない
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tNarrowTextProviderNode::InsertOutputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// 出力ピンを追加することはできない
	// TODO: 例外
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tNarrowTextProviderNode::DeleteOutputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// 出力ピンを削除することはできない
	// TODO: 例外
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_size tNarrowTextProviderNode::GetInputPinCount()
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	return 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tInputPin * tNarrowTextProviderNode::GetInputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// XXX: 範囲外例外
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tNarrowTextProviderNode::InsertInputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// XXX: 入力ピンを追加することはできない
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tNarrowTextProviderNode::DeleteInputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// XXX: 入力ピンを削除することはできない
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tNarrowTextProviderNode::BuildQueue(tQueueBuilder & builder)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());

	for(tOutputPin::tInputPins::const_iterator i = OutputPin->GetInputPins().begin();
		i != OutputPin->GetInputPins().end(); i++)
	{
		// レンダリング世代が最新の物かどうかをチェック
		if((*i)->GetRenderGeneration() != builder.GetRenderGeneration()) continue;

		// 入力ピンのタイプをチェック
		RISSE_ASSERT((*i)->GetAgreedType() == NarrowTextEdgeType);

		// 入力ピンのすべてのリクエストに答えるためのキューノードを作成する
		const tInputPin::tRenderRequests & requests = (*i)->GetRenderRequests();
		for(tInputPin::tRenderRequests::const_iterator i =
			requests.begin(); i != requests.end(); i ++)
		{
			const tNarrowTextRenderRequest * req = Risa::DownCast<const tNarrowTextRenderRequest*>(*i);
			new tNarrowTextProviderQueueNode(
				req,
				Caption,
				t1DArea(req->GetArea(), t1DArea(0, strlen(Caption))),
				t1DArea(req->GetArea(), t1DArea(0, strlen(Caption))).GetStart());
		}
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
}
