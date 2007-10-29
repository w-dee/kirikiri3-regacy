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
//! @brief RINA GDS (Generational Data Structure)
//---------------------------------------------------------------------------
#include "prec.h"
#include "visual/rina/rinaGDS.h"


namespace Rina {
RISSE_DEFINE_SOURCE_ID(36775,10222,29493,19855,18072,28928,53028,33122);
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
tGDSGraph::tGDSGraph()
{
	CS = new tCriticalSection();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tGDSNodeBase * tGDSGraph::GetRoot()
{
	volatile tCriticalSection::tLocker lock(*CS);
	return RootNode;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tGDSNodeData * tGDSGraph::Freeze()
{
	volatile tCriticalSection::tLocker lock(*CS);

	++LastFreezedGeneration;

	return RootNode->GetCurrent();
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
tGDSNodeData::tUpdateLock::tUpdateLock(tGDSNodeData * nodedata) :
	NodeData(nodedata),
	NewNodeData(NULL),
	Lock(nodedata->Node->GetGraph()->GetCS())
{
	NewNodeData = nodedata->BeginIndepend(NULL, NULL);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tGDSNodeData::tUpdateLock::~tUpdateLock()
{
	NodeData->EndIndepend(NewNodeData);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
tGDSNodeData::tGDSNodeData(tGDSNodeBase * node)
{
	Node = node;
	RefCount = 0;
	LastGeneration = Node->GetGraph()->GetLastFreezedGeneration();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tGDSNodeData::Release()
{
	if(--RefCount == 0)
	{
		// 参照カウンタが 0 になった
		Node->GetPool()->Deallocate(this);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tGDSNodeData::SetParentAt(risse_size pn, tGDSNodeData * nodedata, risse_size cn)
{
	// ノードデータの親を設定する

	// 親子の領域は空いてる？
	if(Parents.size() <= pn)
		Parents.resize(pn + 1, NULL);
	if(nodedata->Children.size() <= cn)
		nodedata->Children.resize(cn + 1, NULL);

	// 置き換える場所に既に子が設定されている場合は その子を Release する
	if(nodedata->Children[cn] != NULL) nodedata->Children[cn]->Release();

	// 子を書き込み、子 (this) を AddRef する
	nodedata->Children[cn] = this;
	AddRef();

	// 親を書き込む
	Parents[pn] = nodedata;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tGDSNodeData * tGDSNodeData::BeginIndepend(tGDSNodeData * oldchild, tGDSNodeData * newchild)
{
	// このノードが記録している最終フリーズ世代とグラフ全体の最終フリーズ世代を比べる。
	// もしそれらが同じならばコピーは行わなくてよいが、異なっていればコピーを行う
	bool need_clone = LastGeneration != Node->GetGraph()->GetLastFreezedGeneration();

	// 必要ならば最新状態のクローンを作成
	tGDSNodeData * newnodedata;
	if(need_clone)
	{
		newnodedata = Node->GetPool()->Allocate();
		newnodedata->Copy(this);
	}
	else
	{
		newnodedata = this;
	}

	// クローン/あるいはthisの子ノードのうち、oldchild に向いている
	// ポインタを newchild に置き換える
	if(oldchild) newnodedata->ReconnectChild(oldchild, newchild);

	return newnodedata;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tGDSNodeData::EndIndepend(tGDSNodeData * newnodedata)
{
	// クローンを行わなかった場合は親ノードに再帰する必要はないので、ここで戻る
	if(newnodedata == this) return;

	// ルートノードに向かってノードのクローンを作成していく
	if(Parents.size() == 0)
	{
		// 自分がルート
	}
	else
	{
		// 親に向かって再帰
		for(tNodeVector::iterator i = newnodedata->Parents.begin(); i != newnodedata->Parents.end(); i++)
		{
			if(*i)
			{
				tGDSNodeData * parent_node_data = (*i)->BeginIndepend(this, newnodedata);
				(*i)->EndIndepend(parent_node_data);
			}
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tGDSNodeData::ReconnectChild(tGDSNodeData * oldnodedata, tGDSNodeData * newnodedata)
{
	// TODO: これ線形検索でいいんかいな
	tNodeVector::iterator i = std::find(Children.begin(), Children.end(), oldnodedata);
	RISSE_ASSERT(i != Children.end());
	oldnodedata->Release();
	newnodedata->AddRef();
	(*i) = newnodedata;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tGDSNodeData::DumpGraphviz() const
{
	// ヘッダを出力
	wxPrintf(wxT("digraph GDS {\n"));

	gc_vector<const tGDSNodeData *> nodedatas;

	// phase 1: すべてのノードを書き出す
	nodedatas.push_back(this);
	while(nodedatas.size() > 0)
	{
		const tGDSNodeData * nodedata = nodedatas.back();
		nodedatas.pop_back();
		wxPrintf(wxT("\t0x%p [label=\"%s\", shape=box];\n"),
			nodedata, nodedata->GetName().AsWxString().c_str());

		for(tNodeVector::const_iterator i = nodedata->Children.begin();
			i != nodedata->Children.end(); i++)
			nodedatas.push_back(*i);
	}

	// phase 2: すべてのエッジを書き出す
	nodedatas.push_back(this);
	while(nodedatas.size() > 0)
	{
		const tGDSNodeData * nodedata = nodedatas.back();
		nodedatas.pop_back();
		for(tNodeVector::const_iterator i = nodedata->Children.begin();
			i != nodedata->Children.end(); i++)
		{
			wxPrintf(wxT("\t0x%p -> 0x%p;\n"),
				nodedata, (*i));
			nodedatas.push_back(*i);
		}
	}

	// フッタを出力
	wxPrintf(wxT("}\n"));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tGDSNodeData::Copy(const tGDSNodeData * rhs)
{
	Node = rhs->Node;
	Parents = rhs->Parents;
	Children = rhs->Children;
	LastGeneration = Node->GetGraph()->GetLastFreezedGeneration();
	// 参照カウンタはコピーしない

	// コピーした時点で子の参照カウンタがそれぞれ増える
	for(tNodeVector::iterator i = Children.begin(); i != Children.end(); i++)
	{
		(*i)->AddRef();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tGDSNodeData::Free()
{
	RefCount = 0; // 参照カウンタを 0 にリセットしておく
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tString tGDSNodeData::GetName() const
{
	return tString(RISSE_WS("Generation ")) +
		tString::AsString((int)LastGeneration.operator risse_uint32());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tString tGDSNodeData::DumpText() const
{
	return tString();
}
//---------------------------------------------------------------------------












//---------------------------------------------------------------------------
tGDSNodeBase::tGDSNodeBase(tGDSGraph * graph, tGDSPoolBase * pool)
{
	Graph = graph;
	Pool = pool;
	Current = Pool->Allocate();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tString tGDSNodeBase::GetName() const
{
	risse_char tmp[25];
	pointer_to_str(this, tmp);

	return tString(RISSE_WS("Node@0x")) + tmp;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tString tGDSNodeBase::DumpText() const
{
	return tString();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
} // namespace Rina





#define RINA_GDS_TRIVIAL_TEST
#ifdef RINA_GDS_TRIVIAL_TEST
//---------------------------------------------------------------------------
namespace Risa {

using namespace Rina;
//---------------------------------------------------------------------------
class tTestNodeData : public tGDSNodeData
{
	tString Text; //!< データとして持つテキスト
public:
	tTestNodeData(tGDSNodeBase * node) : tGDSNodeData(node) {;}
	const tString & GetText() const { return Text; }
	void SetText(const tString & text) { Text = text; }
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class tTestNode : public tGDSNode<tTestNodeData>
{
	typedef tGDSNode<tTestNodeData> inherited;
public:
	tTestNode(tGDSGraph * graph) : inherited(graph) {;}
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class tGDSTester : public singleton_base<tGDSTester>
{
public:
	tGDSTester();
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tGDSTester::tGDSTester()
{
	tGDSGraph * graph = new tGDSGraph();

	tTestNode * node1 = new tTestNode(graph); // will be root
	tTestNode * node2 = new tTestNode(graph);
	tTestNode * node3 = new tTestNode(graph);

	node2->GetCurrent()->SetParentAt(0, node1->GetCurrent(), 0);
	node3->GetCurrent()->SetParentAt(0, node1->GetCurrent(), 1);

	node1->GetCurrent()->DumpGraphviz();
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif



