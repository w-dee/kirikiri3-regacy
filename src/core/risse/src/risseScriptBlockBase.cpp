//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief スクリプトブロック管理
//---------------------------------------------------------------------------

#include "prec.h"
#include "risseScriptBlockBase.h"
#include "risseException.h"
#include "compiler/risseCompiler.h"
#include "risseCodeBlock.h"


namespace Risse
{
RISSE_DEFINE_SOURCE_ID(10462,6972,23868,17748,24487,5141,43296,28534);


//---------------------------------------------------------------------------
tRisseScriptBlockBase::tRisseScriptBlockBase(const tRisseString & script,
						const tRisseString & name, risse_size lineofs)
{
	// フィールドの初期化
	LinesToPosition = NULL;
	Script = script;
	Name = name;
	LineOffset = lineofs;
	RootCodeBlock = NULL;
	CodeBlocks = new gc_vector<tRisseCodeBlock *>();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseScriptBlockBase::CreateLinesToPositionArary() const
{
	// まず、全体の行数を数える
	LineCount = 0;

	const risse_char *script = Script.c_str();
	const risse_char *ls = script;
	const risse_char *p = ls;
	while(*p)
	{
		if(*p == RISSE_WC('\r') || *p == RISSE_WC('\n'))
		{
			LineCount ++;
			if(*p == RISSE_WC('\r') && p[1] == RISSE_WC('\n')) p++;
			p++;
			ls = p;
		}
		else
		{
			p++;
		}
	}

	if(p != ls)
	{
		LineCount ++;
	}

	// メモリを確保
	LinesToPosition = new (PointerFreeGC) risse_size[LineCount];

	// 各行の先頭のコードポイント位置を配列に入れていく
	ls = script;
	p = ls;
	risse_size i = 0;
	while(*p)
	{
		if(*p == RISSE_WC('\r') || *p == RISSE_WC('\n'))
		{
			LinesToPosition[i++] = p - script;
			if(*p == RISSE_WC('\r') && p[1] == RISSE_WC('\n')) p++;
			p++;
			ls = p;
		}
		else
		{
			p++;
		}
	}

	if(p != ls)
	{
		LinesToPosition[i++] = p - script;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseScriptBlockBase::PositionToLineAndColumn(risse_size pos,
						risse_size *line, risse_size *col) const
{
	// LinesToPosition 配列が作られていなければ作る
	EnsureLinesToPositionArary();

	// 目的の位置を binary search で探す
	risse_size s = 0;
	risse_size e = LineCount;
	while(true)
	{
		if(e-s <= 1)
		{
			if(line) *line = s + LineOffset; // LineOffsetが加算される
			if(col) *col = pos - LinesToPosition[s];
			return;
		}
		risse_uint m = s + (e-s)/2;
		if(LinesToPosition[m] > pos)
			e = m;
		else
			s = m;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseScriptBlockBase::Compile(tRisseASTNode * root, bool need_result, bool is_expression)
{
	// コンパイラオブジェクトを作成してコンパイルを行う
	tRisseCompiler * compiler = new tRisseCompiler(this);
	compiler->Compile(root, need_result, is_expression);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_size tRisseScriptBlockBase::AddCodeBlock(tRisseCodeBlock * codeblock)
{
	RISSE_ASSERT(CodeBlocks != NULL); // このメソッドが呼ばれるのは Fixup 以前でなければならない
	CodeBlocks->push_back(codeblock);
	return CodeBlocks->size() - 1;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseCodeBlock * tRisseScriptBlockBase::GetCodeBlockAt(risse_size index) const
{
	RISSE_ASSERT(CodeBlocks != NULL);
	RISSE_ASSERT(index < CodeBlocks->size());
	return (*CodeBlocks)[index];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseScriptBlockBase::Fixup()
{
	// すべてのコードブロックの Fixup() を呼ぶ
	for(gc_vector<tRisseCodeBlock *>::iterator i = CodeBlocks->begin();
		i != CodeBlocks->end(); i++)
		(*i)->Fixup(this);

	// CodeBlocks をクリアする
	CodeBlocks = NULL;
}
//---------------------------------------------------------------------------

}
#include "risseCodeExecutor.h" // for test ###################################
namespace Risse {
//---------------------------------------------------------------------------
void tRisseScriptBlockBase::Evaluate(tRisseVariant * result, bool is_expression)
{
	// まず、コンパイルを行う
	// (TODO: スクリプトブロックのキャッシュ対策)

	// AST ノードを用意する
	tRisseASTNode * root_node = GetASTRootNode(result != NULL, is_expression);

	// コンパイルする
	Compile(root_node, result != NULL, is_expression);

	// Fixup する
	Fixup();

	// テスト実行
	RisseFPrint(stdout,(RISSE_WS("---------- Result ----------\n")));
	tRisseVariant ret;
	RISSE_ASSERT(RootCodeBlock != NULL);
	RootCodeBlock->GetExecutor()->Execute(tRisseVariant(), &ret);
	RisseFPrint(stdout, ret.AsHumanReadable().c_str());
}
//---------------------------------------------------------------------------


} // namespace Risse



