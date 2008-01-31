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
//! @brief テスト用の複数形式をサポートするテキストプロバイダノード管理
//---------------------------------------------------------------------------
#include "prec.h"
#include "rinaMultiTextProviderNode.h"
#include "rinaWideTextEdge.h"
#include "rinaWideTextProviderNode.h"
#include "rinaNarrowTextEdge.h"
#include "rinaNarrowTextProviderNode.h"

namespace Rina {
RISSE_DEFINE_SOURCE_ID(47508,49325,57519,19588,18101,2946,50970,8610);
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
tMultiTextOutputPin::tMultiTextOutputPin()
{
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
const gc_vector<risse_uint32> & tMultiTextOutputPin::GetSupportedTypes()
{
	// 暫定実装、できれば static かシングルトン上の配列への参照を返した方がよい
	gc_vector<risse_uint32> * arr = new gc_vector<risse_uint32>();
	arr->push_back(WideTextEdgeType);
	arr->push_back(NarrowTextEdgeType);
	return *arr;
}
//---------------------------------------------------------------------------

















//---------------------------------------------------------------------------
tMultiTextProviderNode::tMultiTextProviderNode(tGraph * graph) : inherited(graph)
{
	// 出力ピンを作成
	OutputPin = new tMultiTextOutputPin();
	OutputPin->Attach(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_size tMultiTextProviderNode::GetOutputPinCount()
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	return 1; // 出力ピンは１個
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tOutputPin * tMultiTextProviderNode::GetOutputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// TODO: 例外
	if(n == 0) return OutputPin;
	return NULL; // 出力ピンはない
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tMultiTextProviderNode::InsertOutputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// 出力ピンを追加することはできない
	// TODO: 例外
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tMultiTextProviderNode::DeleteOutputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// 出力ピンを削除することはできない
	// TODO: 例外
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_size tMultiTextProviderNode::GetInputPinCount()
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	return 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tInputPin * tMultiTextProviderNode::GetInputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// XXX: 範囲外例外
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tMultiTextProviderNode::InsertInputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// XXX: 入力ピンを追加することはできない
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tMultiTextProviderNode::DeleteInputPinAt(risse_size n)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());
	// XXX: 入力ピンを削除することはできない
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tMultiTextProviderNode::BuildQueue(tQueueBuilder & builder)
{
	RISSE_ASSERT_CS_LOCKED(GetGraph()->GetCS());

	// 出力ピンの先に繋がってる入力ピンそれぞれについて
	for(tOutputPin::tInputPins::const_iterator i = OutputPin->GetInputPins().begin();
		i != OutputPin->GetInputPins().end(); i++)
	{
		// レンダリング世代が最新の物かどうかをチェック
		if((*i)->GetRenderGeneration() != builder.GetRenderGeneration()) continue;

		// 入力ピンのタイプをチェックし、適切なキューノードを作成
		risse_uint32 target_type = (*i)->GetAgreedType();

		RISSE_ASSERT(target_type == WideTextEdgeType || target_type == NarrowTextEdgeType); // 暫定
		if(target_type == WideTextEdgeType)
		{
			// 入力ピンのすべてのリクエストに答えるためのキューノードを作成する
			const tInputPin::tRenderRequests & requests = (*i)->GetRenderRequests();
			for(tInputPin::tRenderRequests::const_iterator i =
				requests.begin(); i != requests.end(); i ++)
			{
				const tWideTextRenderRequest * req = Risa::DownCast<const tWideTextRenderRequest*>(*i);
				new tWideTextProviderQueueNode(
					req,
					Caption,
					t1DArea(req->GetArea(), t1DArea(0, Caption.GetLength())),
					t1DArea(req->GetArea(), t1DArea(0, Caption.GetLength())).GetStart());
			}
		}
		else if(target_type == NarrowTextEdgeType)
		{
			// 入力ピンのすべてのリクエストに答えるためのキューノードを作成する
			const tInputPin::tRenderRequests & requests = (*i)->GetRenderRequests();
			for(tInputPin::tRenderRequests::const_iterator i =
				requests.begin(); i != requests.end(); i ++)
			{
				const char * caption = Caption.AsNarrowString();
				const tNarrowTextRenderRequest * req = Risa::DownCast<const tNarrowTextRenderRequest*>(*i);
				new tNarrowTextProviderQueueNode(
					req,
					caption,
					t1DArea(req->GetArea(), t1DArea(0, strlen(caption))),
					t1DArea(req->GetArea(), t1DArea(0, strlen(caption))).GetStart());
			}
		}
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
}
