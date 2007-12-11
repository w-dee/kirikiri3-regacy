//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"

	Rina stands for "Rina is an Imaging Network Assembler"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief テスト用プログラム
//---------------------------------------------------------------------------
#include "prec.h"
#include "base/utils/Singleton.h"
#include "visual/rina/test/rinaTextProviderNode.h"
#include "visual/rina/test/rinaTextPin.h"
#include "visual/rina/test/rinaTextDrawDeviceNode.h"
#include "visual/rina/test/rinaTextMixerNode.h"

using namespace Risa;

namespace Rina {
RISSE_DEFINE_SOURCE_ID(32159,46024,56879,16658,9903,15595,38105,12016);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class tTester : public singleton_base<tTester>
{
public:
	tTester();
private:
	void Test();
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTester::tTester()
{
	Test();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTester::Test()
{
	tTextProviderNode * provider_node1 = new tTextProviderNode();
	provider_node1->SetPosition(1);
	provider_node1->SetCaption(RISSE_WS("aaaaaa"));

	tTextProviderNode * provider_node2 = new tTextProviderNode();
	provider_node2->SetPosition(10);
	provider_node2->SetCaption(RISSE_WS("bbbbbbbb"));

	tTextDrawDeviceNode * dd_node = new tTextDrawDeviceNode();
	dd_node->InsertInputPinAt(0);
	dd_node->InsertInputPinAt(1);

	dd_node->GetInputPinAt(0)->Connect(provider_node1->GetOutputPinAt(0));
	dd_node->GetInputPinAt(1)->Connect(provider_node2->GetOutputPinAt(0));

	wxPrintf(wxT("%08x\n"), dd_node->GetInputPinAt(0)->GetAgreedType());

	tTextMixerNode * mixer_node = new tTextMixerNode();
	mixer_node->SetPosition(30);

	tTextProviderNode * provider_node3 = new tTextProviderNode();
	provider_node3->SetPosition(3);
	provider_node3->SetCaption(RISSE_WS("ccc"));

	tTextProviderNode * provider_node4 = new tTextProviderNode();
	provider_node4->SetPosition(12);
	provider_node4->SetCaption(RISSE_WS("dddd"));

	mixer_node->InsertInputPinAt(0);
	mixer_node->InsertInputPinAt(1);

	mixer_node->GetInputPinAt(0)->Connect(provider_node3->GetOutputPinAt(0));
	mixer_node->GetInputPinAt(1)->Connect(provider_node4->GetOutputPinAt(0));

	dd_node->InsertInputPinAt(2);

	dd_node->GetInputPinAt(2)->Connect(mixer_node->GetOutputPinAt(0));


	// render
	tCommandQueue queue;
	queue.Process(dd_node);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
}
