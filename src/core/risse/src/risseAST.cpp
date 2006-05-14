//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief AST(抽象構文木) の操作
//---------------------------------------------------------------------------
#include "prec.h"
#include "risseAST.h"
#include "risseCodeGen.h"

// 名前表の読み込み
#undef risseASTH
#define RISSE_AST_DEFINE_NAMES
#include "risseAST.h"

namespace Risse
{
RISSE_DEFINE_SOURCE_ID(29091,1243,20617,17999,61570,21800,19479,2186);


/*
	ここから AST のダンプに関わる部分
	(後ろのほうに SSA 形式の生成に関わる部分がある)
*/

//---------------------------------------------------------------------------
void tRisseASTNode::Dump(tRisseString & result, risse_int level)
{
	result += RisseASTNodeTypeNames[Type];
	tRisseString comment = GetDumpComment();
	if(!comment.IsEmpty())
		result += tRisseString(RISSE_WS(" (%1)"), comment);
#ifdef RISSE_TEXT_OUT_CRLF
	result += RISSE_WS("\r\n");
#else
	result += RISSE_WS("\n");
#endif

	level ++;

	risse_size child_count = GetChildCount();
	for(risse_size i = 0; i < child_count; i++)
	{
		result += tRisseString(RISSE_WS(" ")).Times(level) + GetChildNameAt(i);
		result += RISSE_WC(':');
		tRisseASTNode * child = GetChildAt(i);
		if(child)
			child->Dump(result, level);
		else
		#ifdef RISSE_TEXT_OUT_CRLF
			result += RISSE_WS("(null)\r\n");
		#else
			result += RISSE_WS("(null)\n");
		#endif
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_OneExpression::GetChildNameAt(risse_size index) const
{
	if(index == 0) return RISSE_WS("expression"); else return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_List::GetChildNameAt(risse_size index) const
{
	if(index < Array.size())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("node")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncCall::GetChildNameAt(risse_size index) const
{
	if(index == 0) return RISSE_WS("function");
	index --;
	if(index < inherited::GetChildCount())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("argument")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncCall::GetDumpComment() const
{
	tRisseString ret;
	if(CreateNew) ret += RISSE_WS("create_new");
	if(Omit) { if(!ret.IsEmpty()) ret += RISSE_WC(' '); ret += RISSE_WS("omit_arg"); }
	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncCallArg::GetChildNameAt(risse_size index) const
{
	if(index == 0)
		return RISSE_WS("expression");
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncCallArg::GetDumpComment() const
{
	if(Expand)
		return RISSE_WS("expand");
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_RegExp::GetDumpComment() const
{
	return tRisseString(RISSE_WS("pattern=")) + Pattern.AsHumanReadable() +
		RISSE_WS(", flags=") + Flags.AsHumanReadable();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Context::GetDumpComment() const
{
	return RisseASTContextTypeNames[ContextType];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Factor::GetDumpComment() const
{
	tRisseString ret = RisseASTFactorTypeNames[FactorType];
	if(FactorType == aftConstant)
	{
		ret += RISSE_WS(" ");
		ret += Value.AsHumanReadable();
	}
	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Id::GetDumpComment() const
{
	return Name.AsHumanReadable();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Unary::GetChildNameAt(risse_size index) const
{
	if(index == 0)
		return RISSE_WS("child");
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Unary::GetDumpComment() const
{
	return RisseASTUnaryTypeNames[UnaryType];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Binary::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("child0");
	case 1: return RISSE_WS("child1");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Binary::GetDumpComment() const
{
	return RisseASTBinaryTypeNames[BinaryType];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Trinary::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("child0");
	case 1: return RISSE_WS("child1");
	case 2: return RISSE_WS("child2");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Trinary::GetDumpComment() const
{
	return RisseASTTrinaryTypeNames[TrinaryType];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Array::GetChildNameAt(risse_size index) const
{
	if(index < inherited::GetChildCount())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("item")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Dict::GetChildNameAt(risse_size index) const
{
	if(index < inherited::GetChildCount())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("item")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_DictPair::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("name");
	case 1: return RISSE_WS("value");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_If::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("condition");
	case 1: return RISSE_WS("true");
	case 2: return RISSE_WS("false");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_While::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("condition");
	case 1: return RISSE_WS("body");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_While::GetDumpComment() const
{
	if(SkipFirstCheck)
		return RISSE_WS("SkipFirstCheck");
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_For::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("initializer");
	case 1: return RISSE_WS("condition");
	case 2: return RISSE_WS("iterator");
	case 3: return RISSE_WS("body");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_VarDecl::GetChildNameAt(risse_size index) const
{
	if(index < inherited::GetChildCount())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("item")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_VarDeclPair::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("initializer");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_With_Switch::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("object");
	case 1: return RISSE_WS("body");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Case::GetDumpComment() const
{
	if(GetExpression() == NULL) return RISSE_WS("default");
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Try::GetChildNameAt(risse_size index) const
{
	if(index == 0) return RISSE_WS("body");
	if(index == inherited::GetChildCount() + 1) return RISSE_WS("finally");
	index --;
	if(index < inherited::GetChildCount())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("catch")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Catch::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return RISSE_WS("condition");
	case 1: return RISSE_WS("body");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_Catch::GetDumpComment() const
{
	return tRisseString(RISSE_WS("variable=")) + Name.AsHumanReadable();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncDecl::GetChildNameAt(risse_size index) const
{
	if(index == inherited::GetChildCount()) return RISSE_WS("body");
	if(index < inherited::GetChildCount())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("argument")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncDecl::GetDumpComment() const
{
	tRisseString attrib = Attribute.AsString();
	if(!attrib.IsEmpty()) attrib += RISSE_WC(' ');
	if(Name.IsEmpty()) return attrib + RISSE_WS("anonymous");
	return attrib + Name.AsHumanReadable();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncDeclArg::GetChildNameAt(risse_size index) const
{
	if(index == 0)
		return RISSE_WS("initializer");
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_FuncDeclArg::GetDumpComment() const
{
	tRisseString ret = Name.AsHumanReadable();
	if(Collapse)
		ret += RISSE_WS(", collapse");
	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_PropDecl::GetChildNameAt(risse_size index) const
{
	switch(index)
	{
	case 0: return tRisseString(RISSE_WS("setter(argument=")) +
			SetterArgumentName.AsHumanReadable() + RISSE_WS(")");
	case 1: return RISSE_WS("getter");
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_PropDecl::GetDumpComment() const
{
	tRisseString attrib = Attribute.AsString();
	if(!attrib.IsEmpty()) attrib += RISSE_WC(' ');
	if(Name.IsEmpty()) return attrib + RISSE_WS("anonymous");
	return attrib + Name.AsHumanReadable();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_ClassDecl::GetChildNameAt(risse_size index) const
{
	if(index == inherited::GetChildCount()) return RISSE_WS("body");
	if(index < inherited::GetChildCount())
	{
		risse_char buf[40];
		return tRisseString(RISSE_WS("super")) + Risse_int64_to_str(index, buf);
	}
	return tRisseString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseASTNode_ClassDecl::GetDumpComment() const
{
	tRisseString attrib = Attribute.AsString();
	if(!attrib.IsEmpty()) attrib += RISSE_WC(' ');
	if(Name.IsEmpty()) return attrib + RISSE_WS("anonymous");
	return attrib + Name.AsHumanReadable();
}
//---------------------------------------------------------------------------














/*
	ここから SSA 形式の生成に関わる部分
*/










//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_Context::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	// ContextType がトップレベルの場合は、SSA 生成においては、おおかたこのノードが
	// 一番最初に呼ばれることになる。

	// ローカル変数スコープの生成  - コンテキストの種類に従って分岐
	switch(ContextType)
	{
	case actTopLevel: // トップレベル
		break; // 何もしない
	case actBlock: // ブロック
		form->GetLocalNamespace()->Push(); // スコープを push
		break;
	}

	// すべての子ノードに再帰する
	for(risse_size i = 0; i < GetChildCount(); i++)
		GetChildAt(i)->GenerateSSA(sb, form);

	// ローカル変数スコープの消滅  - コンテキストの種類に従って分岐
	switch(ContextType)
	{
	case actTopLevel: // トップレベル
		break; // 何もしない
	case actBlock: // ブロック
		form->GetLocalNamespace()->Pop(); // スコープを pop
		break;
	}

	// このノードは答えを返さない
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_ExprStmt::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	// このノードは式を保持しているだけなので子ノードに処理をさせるだけ
	GetChildAt(0)->GenerateSSA(sb, form);

	// このノードは答えを返さない
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_Factor::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	// "項"
	switch(FactorType)
	{
	case aftConstant:	// 定数
			return form->GetCurrentBlock()->AddConstantValueStatement(GetPosition(), Value);

	case aftThis:		// "this"
		{
			// 文の作成
			tRisseSSAStatement * stmt =
				new tRisseSSAStatement(form, GetPosition(), ocAssignThis);
			// 変数の作成
			tRisseSSAVariable * var = new tRisseSSAVariable(form, stmt);
			// 文の追加
			form->GetCurrentBlock()->AddStatement(stmt);
			// 戻る
			return var;
		}
	case aftSuper:		// "super"
		{
			// 文の作成
			tRisseSSAStatement * stmt =
				new tRisseSSAStatement(form, GetPosition(), ocAssignSuper);
			// 変数の作成
			tRisseSSAVariable * var = new tRisseSSAVariable(form, stmt);
			// 文の追加
			form->GetCurrentBlock()->AddStatement(stmt);
			// 戻る
			return var;
		}
	case aftGlobal:		// "global"
		{
			// 文の作成
			tRisseSSAStatement * stmt =
				new tRisseSSAStatement(form, GetPosition(), ocAssignGlobal);
			// 変数の作成
			tRisseSSAVariable * var = new tRisseSSAVariable(form, stmt);
			var->SetValueType(tRisseVariant::vtObject); // global オブジェクトは常に vtObject
			// 文の追加
			form->GetCurrentBlock()->AddStatement(stmt);
			// 戻る
			return var;
		}
	}
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_Id::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	// 識別子
	tRisseSSAVariable * var = form->GetLocalNamespace()->Find(Name);
	if(!var)
	{
		// ローカル変数に見つからない /// XXXXXX
		return NULL;
	}
	else
	{
		// ローカル変数に見つかった
		return var;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_Unary::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	// 子の計算結果を得る
	tRisseSSAVariable * child_var = Child->GenerateSSA(sb, form);
	if(!child_var)
	{
		// エラー
	}

	// 単項演算子
	switch(UnaryType)
	{
	case autLogNot:	// "!" logical not
	case autBitNot:	// "~" bit not
	case autPlus:	// "+"
	case autMinus:	// "-"
		{
			// オペコードを決定
			tRisseOpCode oc;
			switch(UnaryType)
			{
			case autLogNot:	oc = ocLogNot;	break;
			case autBitNot:	oc = ocBitNot;	break;
			case autPlus:	oc = ocPlus;	break;
			case autMinus:	oc = ocMinus;	break;
			default: oc = ocNoOperation;
			}
			// 文の作成
			tRisseSSAStatement * stmt =
				new tRisseSSAStatement(form, GetPosition(), oc);
			stmt->AddUsed(child_var); // この文の使用リストに変数を加える
			// 変数の作成
			tRisseSSAVariable * var = new tRisseSSAVariable(form, stmt);
		//	var->SetValueType(tRisseVariant::vtBoolean); // 結果は常に vtBoolean

			//////////////////////////////////////////////////////////////////////////////////////////
			// 演算子のオーバーロードによっては ! 演算子は boolean を返さない可能性がある
			// この仕様は後に変更の可能性アリ (! 演算子をオーバーロードできないようにする可能性がある)
			// 他の ~ や + などの演算子についてもそうなる可能性がある
			//////////////////////////////////////////////////////////////////////////////////////////

			// 文の追加
			form->GetCurrentBlock()->AddStatement(stmt);
			// 戻る
			return var;
		}

	case autPreDec:		// "--" pre-positioned decrement
	case autPreInc:		// "++" pre-positioned increment
		{
			// インクリメント、デクリメント演算子は、整数値 1 を加算あるいは
			// 減算するものとして扱われる
			// (実際にそれを inc や dec の VM 命令にするのは CG や optimizer の
			//  仕事)
			tRisseSSAVariable * one_var =
				form->GetCurrentBlock()->AddConstantValueStatement(
						GetPosition(), tRisseVariant((risse_int64)1));
			tRisseSSAStatement * stmt =
				new tRisseSSAStatement(form, GetPosition(),
					UnaryType == autPreDec ? ocSub:ocAdd);
			stmt->AddUsed(child_var); // この文の使用リストに変数を加える
			stmt->AddUsed(one_var); // この文の使用リストに変数を加える
			// 変数の作成
			tRisseSSAVariable * var = new tRisseSSAVariable(form, stmt);
			// 文の追加
			form->GetCurrentBlock()->AddStatement(stmt);
			// 戻る
			return var;
		}

	case autPostDec:	// "--" post-positioned decrement
	case autPostInc:	// "++" post-positioned increment
	case autDelete:		// "delete"
		;
	}
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_Binary::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	// 演算子のタイプに従って分岐
	switch(BinaryType)
	{
	case abtAssign:				// =
		{
			// 単純代入
			// 右辺の値を得る
			tRisseSSAVariable * rhs_var = Child2->GenerateSSA(sb, form);
			// 左辺のタイプをチェック
			if(Child1->GetType() == antId)
			{
				// 識別子
				tRisseASTNode_Id * lhs =
					reinterpret_cast<tRisseASTNode_Id *>(Child1);
				tRisseString var_name = lhs->GetName(); // 変数名
				tRisseSSAVariable * dest_var = form->GetLocalNamespace()->Find(var_name);
				if(!dest_var)
				{
					// ローカル変数に見つからない /// XXXXXX
					return NULL;
				}
				else
				{
					// ローカル変数に見つかった;ローカル変数に上書きする
					// 文の作成
					tRisseSSAStatement * stmt =
						new tRisseSSAStatement(form, GetPosition(), ocAssign);
					stmt->AddUsed(rhs_var); // この文の使用リストに変数を加える
					// 変数の作成
					tRisseSSAVariable * var = new tRisseSSAVariable(form, stmt, var_name);
					// 文の追加
					form->GetCurrentBlock()->AddStatement(stmt);
					// 変数の登録
					form->GetLocalNamespace()->Add(var_name, var);
					// var を返す
					return var;
				}
			}
			return NULL;
		}

	case abtIf:					// if
	case abtComma:				// 
	case abtBitAndAssign:		// &=
	case abtBitOrAssign:		// |=
	case abtBitXorAssign:		// ^=
	case abtSubAssign:			// -=
	case abtAddAssign:			// +=
	case abtModAssign:			// %=
	case abtDivAssign:			// /=
	case abtIdivAssign:			// \=
	case abtMulAssign:			// *=
	case abtLogOrAssign:		// ||=
	case abtLogAndAssign:		// &&=
	case abtRBitShiftAssign:	// >>>=
	case abtLShiftAssign:		// <<=
	case abtRShiftAssign:		// >>=
	case abtLogOr:				// ||
	case abtLogAnd:				// &&
	case abtBitOr:				// |
	case abtBitXor:				// ^
	case abtBitAnd:				// &
	case abtNotEqual:			// !=
	case abtEqual:				// ==
	case abtDiscNotEqual:		// !==
	case abtDiscEqual:			// ===
	case abtSwap:				// <->
	case abtLesser:				// <
	case abtGreater:			// >
	case abtLesserOrEqual:		// <=
	case abtGreaterOrEqual:		// >=
	case abtRBitShift:			// >>>
	case abtLShift:				// <<
	case abtRShift:				// >>
	case abtMod:				// %
	case abtDiv:				// /
	case abtIdiv:				// \ (integer div)
	case abtMul:				// *
	case abtAdd:				// +
	case abtSub:				// -
	case abtDirectSel:			// .
	case abtIndirectSel:		// [ ]
	case abtIncontextOf:		// incontextof

	default:
		return NULL;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_VarDecl::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	// 変数宣言
	// 子に再帰する
	for(risse_size i = 0; i < GetChildCount(); i++)
		GetChildAt(i)->GenerateSSA(sb, form);
	// このノードは答えを返さない
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseASTNode_VarDeclPair::GenerateSSA(
						tRisseScriptBlockBase * sb, tRisseSSAForm *form) const
{
	tRisseSSAVariable * init_var;
	if(Initializer)
	{
		// 初期化値がある
		init_var = Initializer->GenerateSSA(sb, form);
	}
	else
	{
		// 定数値の作成
		init_var = form->GetCurrentBlock()->AddConstantValueStatement(
					GetPosition(), tRisseVariant());
	}
	// 文の作成
	tRisseSSAStatement * stmt =
		new tRisseSSAStatement(form, GetPosition(), ocAssign);
	stmt->AddUsed(init_var); // この文の使用リストに変数を加える
	// 変数の作成
	tRisseSSAVariable * var = new tRisseSSAVariable(form, stmt, Name);
	// 文の追加
	form->GetCurrentBlock()->AddStatement(stmt);
	// 変数の登録
	form->GetLocalNamespace()->Add(Name, var);
	// このノードは答えを返さない
	return NULL;
}
//---------------------------------------------------------------------------

} // namespace Risse
