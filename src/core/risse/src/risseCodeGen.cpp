//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2006 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief バイトコードジェネレータ
//---------------------------------------------------------------------------
#include "prec.h"

#include "risseCodeGen.h"
#include "risseException.h"
#include "risseScriptBlockBase.h"


namespace Risse
{
RISSE_DEFINE_SOURCE_ID(52364,51758,14226,19534,54934,29340,32493,12680);




//---------------------------------------------------------------------------
bool tRisseSSAFlatNamespace::Find(const tRisseString & name) const
{
	return Map.find(name) != Map.end();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisseSSAFlatNamespace::FindAndSetUsed(const tRisseString & name, bool write)
{
	tMap::iterator i = Map.find(name);
	if(i != Map.end())
	{
		if(write)
			i->second.Write = true;
		else
			i->second.Read = true;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAFlatNamespace::Add(const tRisseString & name)
{
	Map.insert(tMap::value_type(name, tInfo())); // 新規に挿入
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAFlatNamespace::GenerateChildRead(tRisseSSAForm * form, risse_size pos,
		tRisseSSAVariable* block_var)
{
	for(tMap::iterator i = Map.begin(); i != Map.end(); i++)
	{
		if(i->second.Read)
		{
			// 読み込みが発生している
			tRisseSSAVariable * var =
				form->GetLocalNamespace()->Read(form, pos, i->first);
			form->AddStatement(pos, ocChildWrite, NULL, block_var, var)->SetName(i->first);
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAFlatNamespace::GenerateChildWrite(tRisseSSAForm * form, risse_size pos,
		tRisseSSAVariable* block_var)
{
	for(tMap::iterator i = Map.begin(); i != Map.end(); i++)
	{
		if(i->second.Write)
		{
			// 書き込みが発生している
			tRisseSSAVariable * var = NULL;
			form->AddStatement(pos, ocChildRead, &var, block_var)->SetName(i->first);
			form->GetLocalNamespace()->Write(form, pos, i->first, var);
		}
	}
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------
tRisseSSALocalNamespace::tRisseSSALocalNamespace()
{
	Block = NULL;
	Chained = NULL;
	Push(); // 最初の名前空間を push しておく
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSALocalNamespace::tRisseSSALocalNamespace(const tRisseSSALocalNamespace &ref)
{
	Block = ref.Block;
	Chained = ref.Chained;
	Scopes.reserve(ref.Scopes.size());
	for(tScopes::const_iterator i = ref.Scopes.begin();
		i != ref.Scopes.end(); i++)
	{
		Scopes.push_back(new tScope(**i));
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSALocalNamespace::GetNumberedName(
				const tRisseString & name, risse_int num)
{
	// num を文字列化
	return name + RISSE_WC('#') + tRisseString::AsString(num);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSALocalNamespace::Push()
{
	Scopes.push_back(new tScope());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSALocalNamespace::Pop()
{
	Scopes.pop_back();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSALocalNamespace::Add(const tRisseString & name, tRisseSSAVariable * where)
{
	RISSE_ASSERT(where != NULL);

	// 番号を決める
	risse_int num = Block->GetForm()->GetUniqueNumber();
	tRisseString n_name = GetNumberedName(name, num);

	// 一番深いレベルのスコープにエイリアスを追加/上書きする
	tAliasMap::iterator i = Scopes.back()->AliasMap.find(name);
	if(i != Scopes.back()->AliasMap.end())
		i->second = n_name; // 上書き
	else
		Scopes.back()->AliasMap.insert(
			tAliasMap::value_type(name, n_name)); // 新規に挿入

	// 番号付きの名前を登録する
	Scopes.back()->VariableMap.insert(tVariableMap::value_type(n_name, where));

	// 名前と番号付きの名前を where に設定する
	where->SetName(name);
	where->SetNumberedName(n_name);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisseSSALocalNamespace::Find(const tRisseString & name,
	tRisseSSAVariable ** var) const
{
	// Scopes を頭から見ていき、最初に見つかった変数を返す
	// (スコープによる変数の可視・不可視のルールに従う)
	for(tScopes::const_reverse_iterator ri = Scopes.rbegin(); ri != Scopes.rend(); ri++)
	{
		tAliasMap::iterator i = (*ri)->AliasMap.find(name);
		if(i != (*ri)->AliasMap.end())
		{
			// 見つかった
			tRisseString n_name = i->second; // 番号付き変数名

			// 同じスコープ内に番号付きの名前があるはず
			tVariableMap::iterator vi = (*ri)->VariableMap.find(n_name);
			RISSE_ASSERT(vi != (*ri)->VariableMap.end()); // 同じスコープ内にあるよね

			if(var) *var = vi->second;

			return true;
		}
	}
	// 見つからなかった
	return false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisseSSALocalNamespace::IsAvailable(const tRisseString & name) const
{
	if(Find(name)) return true;

	//「チェーンされた」名前空間も探す
	if(Chained) return Chained->Find(name);

	// 見つからなかった
	return false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisseSSALocalNamespace::Delete(const tRisseString & name)
{
	for(tScopes::reverse_iterator ri = Scopes.rbegin(); ri != Scopes.rend(); ri++)
	{
		tAliasMap::iterator i = (*ri)->AliasMap.find(name);
		if(i != (*ri)->AliasMap.end())
		{
			// 見つかった
			(*ri)->AliasMap.erase(i); // 削除

			// ここでは AliasMap から削除を行うのみとなる

			return true;
		}
	}

	// 見つからなかった
	return false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseSSALocalNamespace::MakePhiFunction(
					risse_size pos, const tRisseString & name, const tRisseString & n_name)
{
	// ここで渡される name は 
	for(tScopes::reverse_iterator ri = Scopes.rbegin(); ri != Scopes.rend(); ri++)
	{
		tRisseString name_to_find;
		if(n_name.IsEmpty())
		{
			// 番号付きの名前が分からない場合はエイリアスを検索する
			tAliasMap::iterator i = (*ri)->AliasMap.find(name);
			if(i == (*ri)->AliasMap.end()) continue; // 見つからなかった

			name_to_find = i->second;
		}
		else
		{
			name_to_find = n_name;
		}

		tVariableMap::iterator vi = (*ri)->VariableMap.find(name_to_find);
		if(vi != (*ri)->VariableMap.end())
		{
			// 見つかった
			if(vi->second != NULL) return vi->second; // 見つかったがφ関数を作成する必要はない
			// 見つかったがφ関数を作成する必要がある
			Block->AddPhiFunction(pos, name, name_to_find, vi->second);
			return vi->second;
		}
	}

	// 見つからなかった
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSALocalNamespace::MarkToCreatePhi()
{
	for(tScopes::reverse_iterator ri = Scopes.rbegin(); ri != Scopes.rend(); ri++)
	{
		for(tVariableMap::iterator i = (*ri)->VariableMap.begin();
					i != (*ri)->VariableMap.end(); i++)
			i->second = NULL;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseSSALocalNamespace::Read(tRisseSSAForm * form,
							risse_size pos, const tRisseString & name)
{
	tRisseSSAVariable * ret = MakePhiFunction(pos, name);
	if(!ret && Chained)
	{
		// チェーンされた名前空間を探す
		bool found = Chained->FindAndSetUsed(name, false);
		if(found)
		{
			// 見つかった
			// チェーンされた名前空間から読み込むための文を生成
			tRisseSSAStatement *stmt = form->AddStatement(pos, ocParentRead, &ret);
			stmt->SetName(name);
		}
	}
	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisseSSALocalNamespace::Write(tRisseSSAForm * form, risse_size pos,
					const tRisseString & name, tRisseSSAVariable * value)
{
	// ローカル変数に上書きする
	RISSE_ASSERT(value != NULL);

	// 名前空間を探す
	for(tScopes::reverse_iterator ri = Scopes.rbegin(); ri != Scopes.rend(); ri++)
	{
		tAliasMap::iterator i = (*ri)->AliasMap.find(name);
		if(i != (*ri)->AliasMap.end())
		{
			// 見つかった
			tRisseString n_name = i->second; // 番号付き変数名

			// 同じスコープ内に番号付きの名前があるはず
			tVariableMap::iterator vi = (*ri)->VariableMap.find(n_name);
			RISSE_ASSERT(vi != (*ri)->VariableMap.end()); // 同じスコープ内にあるよね

			// 文の作成
			tRisseSSAVariable * ret_var = NULL;
			form->AddStatement(pos, ocAssign, &ret_var, value);

			// 変数の上書き
			vi->second = ret_var; // 上書き

			// 番号付きの名前を ret_var に設定する
			ret_var->SetName(name);
			ret_var->SetNumberedName(n_name);

			return true; // 戻る
		}
	}

	// 見つからない
	// チェーンされた名前空間に書き込む
	if(Chained)
	{
		bool found = Chained->FindAndSetUsed(name, true);
		if(found)
		{
			// 見つかった
			// チェーンされた名前空間に書き込むための文を生成
			tRisseSSAStatement *stmt = form->AddStatement(pos, ocParentWrite, NULL, value);
			stmt->SetName(name);
			return true; // 戻る
		}
	}

	return false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAFlatNamespace * tRisseSSALocalNamespace::CreateFlatNamespace() const
{
	tRisseSSAFlatNamespace * ns;

	if(Chained)
	{
		// もしチェーンされた名前空間があればそれをコピーする
		ns = new tRisseSSAFlatNamespace(*Chained);
	}
	else
	{
		// チェーンされた名前空間がないので新規に作成
		ns = new tRisseSSAFlatNamespace();
	}

	// 名前空間を順にイテレーションして ns に追加
	for(tScopes::const_iterator si = Scopes.begin(); si != Scopes.end(); si++)
	{
		for(tAliasMap::iterator i = (*si)->AliasMap.begin();
				i != (*si)->AliasMap.end(); i++)
			ns->Add(i->first);
	}

	return ns;
}
//---------------------------------------------------------------------------




















//---------------------------------------------------------------------------
tRisseSSAVariable::tRisseSSAVariable(tRisseSSAForm * form, 
	tRisseSSAStatement *stmt, const tRisseString & name)
{
	// フィールドの初期化
	Form = form;
	Declared = stmt;
	Value = NULL;
	ValueType = tRisseVariant::vtVoid;
	Mark = NULL;

	// この変数が定義された文の登録
	if(Declared) Declared->SetDeclared(this);

	// 名前と番号を設定する
	SetName(name);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAVariable::SetName(const tRisseString & name)
{
	// 名前を設定する
	Name = name;

	// 通し番号の準備
	Version = Form->GetUniqueNumber();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSAVariable::GetQualifiedName() const
{
	if(Name.IsEmpty())
	{
		// 一時変数の場合は _tmp@ の次にバージョン文字列
		return tRisseString(RISSE_WS("_tmp@")) + tRisseString::AsString(Version);
	}
	else
	{
		// NumberedName がある場合はそれを、Nameを使う
		tRisseString n;
		if(!NumberedName.IsEmpty()) n = NumberedName; else n = Name;
		// 普通の変数の場合は 変数名@バージョン文字列
		return n + RISSE_WC('@') + tRisseString::AsString(Version);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAVariable::DeleteUsed(tRisseSSAStatement * stmt)
{
	gc_vector<tRisseSSAStatement*>::iterator it =
		std::find(Used.begin(), Used.end(), stmt);
	if(it != Used.end()) Used.erase(it);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseSSAVariable::GenerateFuncCall(risse_size pos, const tRisseString & name,
			tRisseSSAVariable * param1,
			tRisseSSAVariable * param2,
			tRisseSSAVariable * param3)
{
	tRisseSSAVariable * func_var = NULL;

	if(!name.IsEmpty())
	{
		// 関数名が指定されている場合

		// メソッド名を置く
		tRisseSSAVariable * method_name_var =
			Form->AddConstantValueStatement(pos, name);

		// メソッドオブジェクトを得る
		Form->AddStatement(pos, ocDGet, &func_var,
								this, method_name_var);
	}
	else
	{
		func_var = this;
	}

	// 関数呼び出し文を生成する
	tRisseSSAVariable * ret_var = NULL;
	tRisseSSAStatement * call_stmt =
		Form->AddStatement(pos, ocFuncCall, &ret_var, func_var, param1, param2, param3);
	call_stmt->SetFuncArgOmitted(false);
	call_stmt->SetFuncExpandFlags(0);

	// 戻る
	return ret_var;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSAVariable::Dump() const
{
	return GetQualifiedName();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSAVariable::GetTypeComment() const
{
	if(Value)
	{
		// 定数である
		return tRisseString(RISSE_WS("constant ")) +
			Value->AsHumanReadable();
	}
	else if(ValueType != tRisseVariant::vtVoid)
	{
		// 型が決まっている
		return tRisseString(RISSE_WS("always type ")) +
			tRisseVariant::GetTypeString(ValueType);
	}
	else return tRisseString();
}
//---------------------------------------------------------------------------



















//---------------------------------------------------------------------------
tRisseSSAStatement::tRisseSSAStatement(tRisseSSAForm * form,
	risse_size position, tRisseOpCode code)
{
	// フィールドの初期化
	Form = form;
	Position = position;
	Code = code;
	Block = NULL;
	Pred = NULL;
	Succ = NULL;
	Declared = NULL;
	TrueBranch = FalseBranch = NULL;
	JumpTarget = NULL;
	FuncExpandFlags = 0;
	FuncArgOmitted = false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAStatement::AddUsed(tRisseSSAVariable * var)
{
	var->AddUsed(this);
	Used.push_back(var);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAStatement::DeleteUsed(risse_size index)
{
	RISSE_ASSERT(Used.size() > 0); // いまのところは。
	tRisseSSAVariable * var = Used[index];
	Used.erase(Used.begin() + index);
	var->DeleteUsed(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAStatement::SetTrueBranch(tRisseSSABlock * block)
{
	TrueBranch = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAStatement::SetJumpTarget(tRisseSSABlock * block)
{
	JumpTarget = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAStatement::SetFalseBranch(tRisseSSABlock * block)
{
	FalseBranch = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAStatement::SetName(const tRisseString & name)
{
	RISSE_ASSERT(
			Code == ocParentRead || Code == ocParentWrite ||
			Code == ocChildRead || Code == ocChildWrite || Code == ocDefineLazyBlock);
	Name = new tRisseString(name);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
const tRisseString & tRisseSSAStatement::GetName() const
{
	RISSE_ASSERT(
			Code == ocParentRead || Code == ocParentWrite ||
			Code == ocChildRead || Code == ocChildWrite || Code == ocDefineLazyBlock);
	RISSE_ASSERT(Name != NULL);
	return *Name;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAStatement::GenerateCode(tRisseCodeGenerator * gen) const
{
	// gen に一つ文を追加する
	switch(Code)
	{
	case ocNoOperation:
		gen->PutNoOperation();
		break;

	case ocAssign:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1);
		gen->PutAssign(Declared, Used[0]);
		break;

	case ocAssignConstant:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Declared->GetValue() != NULL);
		gen->PutAssign(Declared, *Declared->GetValue());
		break;

	case ocAssignThis:
	case ocAssignSuper:
	case ocAssignGlobal:
	case ocAssignNewArray:
	case ocAssignNewDict:
	case ocAssignNewRegExp:
		RISSE_ASSERT(Declared != NULL);
		gen->PutAssign(Declared, Code);
		break;

	case ocFuncCall:
	case ocNew:
	case ocFuncCallBlock:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() >= 1);

		{
			gc_vector<const tRisseSSAVariable *> args, blocks;
			// TODO: ブロックの引き渡し

			for(risse_size i = /*注意*/1; i < Used.size(); i++)
				args.push_back(Used[i]);
			gen->PutFunctionCall(Declared, Used[0], Code == ocNew, FuncArgOmitted,
						FuncExpandFlags, args, blocks);
		}
		break;

	case ocJump:
		gen->PutJump(JumpTarget);
		break;

	case ocBranch:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutBranch(Used[0], TrueBranch, FalseBranch);
		break;

	case ocDebugger:
		gen->PutDebugger();
		break;

	case ocThrow:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutThrow(Used[0]);
		break;

	case ocReturn:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutReturn(Used[0]);

	case ocLogNot:
	case ocBitNot:
	case ocPlus:
	case ocMinus:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1);
		gen->PutOperator(Code, Declared, Used[0]);
		break;

	case ocLogOr:
	case ocLogAnd:
	case ocBitOr:
	case ocBitXor:
	case ocBitAnd:
	case ocNotEqual:
	case ocEqual:
	case ocDiscNotEqual:
	case ocDiscEqual:
	case ocLesser:
	case ocGreater:
	case ocLesserOrEqual:
	case ocGreaterOrEqual:
	case ocRBitShift:
	case ocLShift:
	case ocRShift:
	case ocMod:
	case ocDiv:
	case ocIdiv:
	case ocMul:
	case ocAdd:
	case ocSub:
	case ocIncontextOf:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutOperator(Code, Declared, Used[0], Used[1]);
		break;

	case ocDGet:
	case ocIGet:
	case ocDDelete:
	case ocIDelete:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutOperator(Code, Declared, Used[0], Used[1]);
		break;

	case ocDSet:
	case ocISet:
		RISSE_ASSERT(Used.size() == 3);
		gen->PutSet(Code, Used[0], Used[1], Used[2]);
		break;

	default:
		RISSE_ASSERT(!"not acceptable SSA operation code");
		break;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSAStatement::Dump() const
{
	switch(Code)
	{
	case ocNoOperation:			// なにもしない
		return RISSE_WS("nop");

	case ocPhi:		// φ関数
		{
			tRisseString ret;

			RISSE_ASSERT(Declared != NULL);
			ret += Declared->Dump() + RISSE_WS(" = PHI(");

			// φ関数の引数を追加
			tRisseString used;
			for(gc_vector<tRisseSSAVariable*>::const_iterator i = Used.begin();
					i != Used.end(); i++)
			{
				if(!used.IsEmpty()) used += RISSE_WS(", ");
				used += (*i)->Dump();
			}

			ret += used + RISSE_WS(")");

			// 変数のコメントを追加
			tRisseString comment = Declared->GetTypeComment();
			if(!comment.IsEmpty())
				ret += RISSE_WS(" // ") + Declared->Dump() + RISSE_WS(" = ") + comment;

			return ret;
		}

	case ocAssignNewRegExp: // 新しい正規表現オブジェクト
		{
			tRisseString ret;
			RISSE_ASSERT(Used.size() == 2);
			ret += Declared->Dump() + RISSE_WS(" = AssignNewRegExp(");

			ret +=	Used[0]->Dump() + RISSE_WS(", ") +
					Used[1]->Dump() + RISSE_WS(")");

			// 変数のコメントを追加
			tRisseString comment = Declared->GetTypeComment();
			if(!comment.IsEmpty())
				ret += RISSE_WS(" // ") + Declared->Dump() + RISSE_WS(" = ") + comment;

			return ret;
		}

	case ocJump:
		{
			RISSE_ASSERT(JumpTarget != NULL);
			return RISSE_WS("goto *") + JumpTarget->GetName();
		}

	case ocBranch:
		{
			RISSE_ASSERT(TrueBranch != NULL);
			RISSE_ASSERT(FalseBranch != NULL);
			RISSE_ASSERT(Used.size() == 1);
			return
					RISSE_WS("if ") + (*Used.begin())->Dump() + 
					RISSE_WS(" then *") + TrueBranch->GetName() +
					RISSE_WS(" else *") + FalseBranch->GetName();
		}

	case ocDefineLazyBlock: // 遅延評価ブロックの定義
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Declared != NULL);

			tRisseString ret;
			ret += Declared->Dump() + RISSE_WS(" = DefineLazyBlock(");

			ret +=	Name->AsHumanReadable() + RISSE_WS(")");

			// 変数のコメントを追加
			tRisseString comment = Declared->GetTypeComment();
			if(!comment.IsEmpty())
				ret += RISSE_WS(" // ") + Declared->Dump() + RISSE_WS(" = ") + comment;

			return ret;
		}

	case ocParentWrite: // 親名前空間への書き込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Used.size() == 1);
			return (*Used.begin())->Dump()  + RISSE_WS(".ParentWrite(") + Name->AsHumanReadable() +
					RISSE_WS(")");
		}

	case ocParentRead: // 親名前空間からの読み込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Declared != NULL);

			tRisseString ret;
			ret += Declared->Dump() + RISSE_WS(" = ParentRead(");

			ret +=	Name->AsHumanReadable() + RISSE_WS(")");

			// 変数のコメントを追加
			tRisseString comment = Declared->GetTypeComment();
			if(!comment.IsEmpty())
				ret += RISSE_WS(" // ") + Declared->Dump() + RISSE_WS(" = ") + comment;

			return ret;
		}

	case ocChildWrite: // 子名前空間への書き込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Used.size() == 2);
			return Used[0]->Dump()  + RISSE_WS(".ChildWrite(") +
					Name->AsHumanReadable() + RISSE_WS(", ") + Used[1]->Dump() +
					RISSE_WS(")");
		}

	case ocChildRead: // 子名前空間からの読み込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Declared != NULL);
			RISSE_ASSERT(Used.size() == 1);

			tRisseString ret;
			ret += Declared->Dump() + RISSE_WS(" = ") + Used[0]->Dump();
			ret += RISSE_WS(".ChildRead(");
			ret +=	Name->AsHumanReadable() + RISSE_WS(")");

			// 変数のコメントを追加
			tRisseString comment = Declared->GetTypeComment();
			if(!comment.IsEmpty())
				ret += RISSE_WS(" // ") + Declared->Dump() + RISSE_WS(" = ") + comment;

			return ret;
		}

	default:
		{
			tRisseString ret;

			// 変数の宣言
			if(Declared)
				ret += Declared->Dump() + RISSE_WS(" = "); // この文で宣言された変数がある

			if(Code == ocAssign)
			{
				// 単純代入
				RISSE_ASSERT(Used.size() == 1);
				ret += (*Used.begin())->Dump();
			}
			else
			{
				// 関数呼び出しの類？
				bool is_funccall = (Code == ocFuncCall || Code == ocNew);

				// 使用している引数の最初の引数はメッセージの送り先なので
				// オペレーションコードよりも前に置く
				if(Used.size() != 0)
					ret += (*Used.begin())->Dump() + RISSE_WC('.');

				// オペレーションコード
				ret += tRisseString(RisseVMInsnInfo[Code].Name);

				// 使用している引数
				if(is_funccall && FuncArgOmitted)
				{
					// 関数呼び出しかnew
					ret += RISSE_WS("(...)");
				}
				else if(Used.size() >= 2)
				{
					// 引数がある
					tRisseString used;
					risse_int arg_index = 0;
					for(gc_vector<tRisseSSAVariable*>::const_iterator i = Used.begin() + 1;
							i != Used.end(); i++, arg_index++)
					{
						if(!used.IsEmpty()) used += RISSE_WS(", ");
						used += (*i)->Dump();
						if(is_funccall && (FuncExpandFlags & (1<<arg_index)))
							used += RISSE_WC('*'); // 展開フラグ
					}
					ret += RISSE_WS("(") + used +
									RISSE_WS(")");
				}
				else
				{
					// 引数が無い
					ret += RISSE_WS("()");
				}
			}

			// 変数の宣言に関してコメントがあればそれを追加
			if(Declared)
			{
				tRisseString comment = Declared->GetTypeComment();
				if(!comment.IsEmpty())
					ret += RISSE_WS(" // ") + Declared->Dump() + RISSE_WS(" = ") + comment;
			}
			return ret;
		}
	}
}
//---------------------------------------------------------------------------






















//---------------------------------------------------------------------------
tRisseSSABlock::tRisseSSABlock(tRisseSSAForm * form, const tRisseString & name)
{
	Form = form;
	FirstStatement = LastStatement = NULL;
	LocalNamespace = NULL;
	Mark = NULL;
	Traversing = false;
	LiveIn = LiveOut = NULL;

	// 通し番号の準備
	Name = name + RISSE_WC('_') + tRisseString::AsString(form->GetUniqueNumber());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::InsertStatement(tRisseSSAStatement * stmt, tStatementInsertPoint point)
{
	tRisseSSAStatement * cs = NULL;
	switch(point)
	{
	case sipHead:
		cs = FirstStatement;
		break;

	case sipAfterPhi:
		for(cs = FirstStatement;
			cs != NULL && cs->GetCode() == ocPhi;
			cs = cs->GetSucc()) ; /* ←空文 */
		break;

	case sipBeforeBranch:
		{
			tRisseSSAStatement * i;
			cs = NULL;
			for(i = LastStatement; i != NULL; i = i->GetPred())
			{
				if(i->IsBranchStatement())
					cs = i;
				else
					break;
			}
			// 分岐文が見つからなかった場合は cs = NULL (つまり最後に追加)
		}
		break;

	case sipTail:
		cs = NULL;
		break;
	}

	// この時点で cs は挿入したい文の直後の文
	// cs==NULLの場合は最後に追加
	tRisseSSAStatement * cs_pred = cs ? cs->GetPred() : LastStatement;
	if(cs_pred)	cs_pred->SetSucc(stmt);
	if(cs)		cs->SetPred(stmt);
	stmt->SetPred(cs_pred);
	if(cs_pred == NULL) FirstStatement = stmt;
	stmt->SetSucc(cs);
	if(cs == NULL) LastStatement = stmt;

	stmt->SetBlock(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::DeleteStatement(tRisseSSAStatement * stmt)
{
	tRisseSSAStatement * stmt_pred = stmt->GetPred();
	tRisseSSAStatement * stmt_succ = stmt->GetSucc();
	if(stmt_pred) stmt_pred->SetSucc(stmt_succ);
	if(stmt_pred == NULL) FirstStatement = stmt_succ;
	if(stmt_succ) stmt_succ->SetPred(stmt_pred);
	if(stmt_succ == NULL) LastStatement = stmt_pred;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::AddPhiFunction(risse_size pos,
	const tRisseString & name, const tRisseString & n_name, tRisseSSAVariable *& decl_var)
{
	// φ関数を追加する
	tRisseSSAStatement * stmt = new tRisseSSAStatement(Form, pos, ocPhi);

	// 戻りの変数を作成する
	// この AddPhiFunction メソッドを呼び出す tRisseSSALocalNamespace::MakePhiFunction()
	// は、見つかった変数のmap のsecondをvar引数に渡してくる。このため、
	// decl_var に代入した時点でこの変数の新しいSSA変数が可視になる。
	decl_var = new tRisseSSAVariable(Form, stmt, name);
	decl_var->SetNumberedName(n_name);

	// φ関数は必ずブロックの先頭に追加される
	InsertStatement(stmt, sipHead);

	// 関数の引数を調べる
	// 関数の引数は、直前のブロックのローカル名前空間のスナップショットから
	// 変数名を探すことで得る
	for(gc_vector<tRisseSSABlock *>::iterator i = Pred.begin(); i != Pred.end(); i ++)
	{
		RISSE_ASSERT((*i)->LocalNamespace != NULL);

		tRisseSSAVariable * found_var =
			(*i)->LocalNamespace->MakePhiFunction(pos,
					decl_var->GetName(), decl_var->GetNumberedName());
		if(!found_var)
		{
			// 変数が見つからない
			// エラーにする
			// TODO: もっと親切なエラーメッセージ
			eRisseCompileError::Throw(
				tRisseString(
					RISSE_WS_TR("local variable '%1' is from out of scope"),
					decl_var->GetName()),
					Form->GetScriptBlock(), stmt->GetPosition());
		}
		stmt->AddUsed(found_var);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::AddPred(tRisseSSABlock * block)
{
	// 直前の基本ブロックを追加する
	Pred.push_back(block);

	// block の直後基本ブロックとして this を追加する
	block->AddSucc(this);

	// 既存のφ関数は、すべて再調整しなければならない
	tRisseSSAStatement *stmt = FirstStatement;
	while(stmt)
	{
		if(stmt->GetCode() != ocPhi) break;
		RISSE_ASSERT(!stmt->GetDeclared()->GetName().IsEmpty());
		RISSE_ASSERT(block->LocalNamespace != NULL);

		tRisseSSAVariable * decl_var = stmt->GetDeclared();

		tRisseSSAVariable * found_var =
			block->LocalNamespace->MakePhiFunction(stmt->GetPosition(),
				decl_var->GetName(), decl_var->GetNumberedName());
		if(!found_var)
		{
			// 変数が見つからない
			// エラーにする
			// TODO: もっと親切なエラーメッセージ
			eRisseCompileError::Throw(
				tRisseString(
					RISSE_WS_TR("local variable '%1' is from out of scope"),
					decl_var->GetName()),
					Form->GetScriptBlock(), stmt->GetPosition());
		}
		stmt->AddUsed(found_var);

		stmt = stmt->GetSucc();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::DeletePred(risse_size index)
{
	// 直前の基本ブロックを削除する
	RISSE_ASSERT(Pred.size() > 0);
	Pred.erase(Pred.begin() + index);

	// 既存のφ関数からも削除する
	tRisseSSAStatement *stmt = FirstStatement;
	while(stmt)
	{
		if(stmt->GetCode() != ocPhi) break;

		stmt->DeleteUsed(index);
			// φ関数の Used の順番は Pred の順番と同じなので index を
			// そのまま DeleteUsed() に渡す

		stmt = stmt->GetSucc();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::AddSucc(tRisseSSABlock * block)
{
	Succ.push_back(block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::DeleteUnmarkedPred()
{
	// Pred は削除の効率を考え、逆順に見ていく
	if(Pred.size() > 0)
	{
		risse_size i = Pred.size() - 1;
		while(true)
		{
			if(!Pred[i]->Mark) DeletePred(i);

			if(i == 0) break;
			i--;
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::TakeLocalNamespaceSnapshot(tRisseSSALocalNamespace * ref)
{
	LocalNamespace = new tRisseSSALocalNamespace(*ref);
	LocalNamespace->SetBlock(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::AddLiveness(const tRisseSSAVariable * var, bool out)
{
	tLiveVariableMap * map = out ? LiveOut : LiveIn;
	RISSE_ASSERT(map != NULL);
	map->insert(tLiveVariableMap::value_type(var, NULL));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisseSSABlock::GetLiveness(const tRisseSSAVariable * var, bool out) const
{
	tLiveVariableMap * map = out ? LiveOut : LiveIn;
	RISSE_ASSERT(map != NULL);
	return map->find(var) != map->end();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::ClearMark() const
{
	gc_vector<tRisseSSABlock *> blocks;
	Traverse(blocks);
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->SetMark(NULL);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::Traverse(gc_vector<tRisseSSABlock *> & blocks) const
{
	risse_size new_start = 0;
	risse_size new_count = 1;
	// 自分自身を blocks に追加
	blocks.push_back(const_cast<tRisseSSABlock*>(this));
	Traversing = true;

	// new_count が 0 で無い限り繰り返す(幅優先探索)
	while(new_count != 0)
	{
		risse_size start = blocks.size();
		// [new_start, new_start + new_count) の直後基本ブロックを追加する
		for(risse_size n = new_start; n < new_start + new_count; n++)
		{
			blocks.reserve(blocks.size() + blocks[n]->Succ.size());
			for(gc_vector<tRisseSSABlock *>::iterator
				i = blocks[n]->Succ.begin(); i != blocks[n]->Succ.end(); i++)
			{
				if(!(*i)->Traversing)
				{
					(*i)->Traversing = true; // 二度と追加しないように
					blocks.push_back(*i);
				}
			}
		}
		new_start = start;
		new_count = blocks.size() - new_start;
	}

	// Traversing フラグを倒す
	for(gc_vector<tRisseSSABlock *>::iterator
		i = blocks.begin(); i != blocks.end(); i++) (*i)->Traversing = false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::CreateLiveInAndLiveOut()
{
	// Pred と Succ の間に LiveIn と LiveOut を作成する
	// 接続されている二つのブロックの接続点の LiveOut と LiveIn は
	// 同じオブジェクトになる

	// すべての Pred の LiveOut に設定を行う
	if(!LiveIn) LiveIn = new tLiveVariableMap();
	for(gc_vector<tRisseSSABlock *>::iterator i = Pred.begin();
			i != Pred.end(); i++)
		(*i)->LiveOut = LiveIn;

	// すべての Succ の LiveIn に設定を行う
	if(!LiveOut) LiveOut = new tLiveVariableMap();
	for(gc_vector<tRisseSSABlock *>::iterator i = Succ.begin();
			i != Succ.end(); i++)
		(*i)->LiveIn = LiveOut;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::AnalyzeVariableLiveness()
{
	// すべてのマークされていない変数に対して生存区間解析を行う

	// すべての文を検査し、それぞれの文で使用されている「変数」について解析を行う
	// 「文」につく「マーク」は 文のポインタそのもの
	tRisseSSAStatement *stmt;
	for(stmt = FirstStatement;
		stmt;
		stmt = stmt->GetSucc())
	{
		const gc_vector<tRisseSSAVariable *> & used = stmt->GetUsed();
		for(risse_size s = 0; s < used.size(); s++)
		{
			if(used[s]->GetMark() != stmt)
			{
				Form->AnalyzeVariableLiveness(used[s]);
				used[s]->SetMark(stmt);
			}
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::RemovePhiStatements()
{
	// すべてのφ関数について処理
	tRisseSSAStatement *stmt;
	for(stmt = FirstStatement;
		stmt && stmt->GetCode() == ocPhi;
		stmt = FirstStatement)
	{
		// φ関数を削除する処理は簡単にはそれぞれの pred ブロックの分岐の
		// 最後に代入文を生成し、φ関数を削除する。
		// ただし、この代入文の挿入によって変数が上書きされてしまう場合が
		// ある(lost copy problem)ため、変数の有効範囲の解析を行う

		// TODO: 変数の併合(coalescing)

		// 代入文を生成するに先立ち、代入文を生成することにより変数が上書き
		// されないことを確認する。具体的には、代入文を生成する場所で
		// 代入先変数が生存していれば、そこに代入文を生成してしまうと
		// 変になる。この場合は phi 変数の戻り値をいったん一時変数にとるような
		// 文を挿入することで生存範囲の干渉を避ける。

		// stmt の used の配列
		const gc_vector<tRisseSSAVariable *> phi_used = stmt->GetUsed();

		// stmt で宣言された変数
		tRisseSSAVariable * stmt_decld = stmt->GetDeclared();


		// pred をたどる
		bool var_used = false;
		for(gc_vector<tRisseSSABlock *>::iterator i = Pred.begin();
			i != Pred.end(); i++)
		{
			// pred の最後で stmt_decld が存在しているかどうかを調べる
			if((*i)->GetLiveness(stmt_decld))
				{ var_used = true; break; }
		}

		if(var_used)
		{
			// TODO: 変数の干渉が見つかった場合の処理
			RisseFPrint(stderr, tRisseString(RISSE_WS("variable interference found at block %1\n"), Dump()).c_str());
			RISSE_ASSERT("variable interference found");
		}

		// 各 pred の分岐文の直前に 代入文を生成する
		for(risse_size index = 0; index < Pred.size(); index ++)
		{
			tRisseSSAStatement * new_stmt =
				new tRisseSSAStatement(Form, stmt->GetPosition(), ocAssign);
			new_stmt->AddUsed(const_cast<tRisseSSAVariable*>(phi_used[index]));
			new_stmt->SetDeclared(stmt_decld);

			Pred[index]->InsertStatement(new_stmt, sipBeforeBranch);

			// LiveOut にも stmt_decld を追加する
			Pred[index]->AddLiveness(stmt_decld);
		}

		// φ関数を除去
		DeleteStatement(stmt);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSABlock::GenerateCode(tRisseCodeGenerator * gen) const
{
	// この基本ブロックを gen に登録する
	gen->AddBlockMap(this);

	// それぞれの文についてコードを生成させる
	tRisseSSAStatement *stmt;
	for(stmt = FirstStatement;
		stmt;
		stmt = stmt->GetSucc())
	{
		stmt->GenerateCode(gen);
	}

	// 不要な変数を解放する
	gen->FreeUnusedRegisters(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSABlock::Dump() const
{
	tRisseString ret;

	// ラベル名と直前のブロックを列挙
	ret +=  + RISSE_WS("*") + Name;
	if(Pred.size() != 0)
	{
		ret += RISSE_WS(" // pred: ");
		for(gc_vector<tRisseSSABlock *>::const_iterator i = Pred.begin();
										i != Pred.end(); i ++)
		{
			if(i != Pred.begin()) ret += RISSE_WS(", ");
			ret += RISSE_WS("*") + (*i)->GetName();
		}
	}
	ret += RISSE_WS("\n");

	// LiveIn を列挙
	if(LiveIn && LiveIn->size() > 0)
	{
		ret += RISSE_WS("// LiveIn: ");
		for(tLiveVariableMap::iterator i = LiveIn->begin(); i != LiveIn->end(); i++)
		{
			if(i != LiveIn->begin()) ret += RISSE_WS(", ");
			ret += i->first->GetQualifiedName();
		}
		ret += RISSE_WS("\n");
	}

	// 文を列挙
	if(!FirstStatement)
	{
		// 一つも文を含んでいない？？
		ret += RISSE_WS("This SSA basic block does not contain any statements\n");
	}
	else
	{
		// すべての文をダンプ
		tRisseSSAStatement * stmt = FirstStatement;
		do 
		{
			ret += stmt->Dump() + RISSE_WS("\n");
			stmt = stmt->GetSucc();
		} while(stmt != NULL);
	}

	// LiveOut を列挙
	if(LiveOut && LiveOut->size() > 0)
	{
		ret += RISSE_WS("// LiveOut: ");
		for(tLiveVariableMap::iterator i = LiveOut->begin(); i != LiveOut->end(); i++)
		{
			if(i != LiveOut->begin()) ret += RISSE_WS(", ");
			ret += i->first->GetQualifiedName();
		}
		ret += RISSE_WS("\n");
	}

	return ret + RISSE_WS("\n");
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------
void tRisseSSALabelMap::AddMap(const tRisseString &labelname, tRisseSSABlock * block, risse_size pos)
{
	tLabelMap::iterator i = LabelMap.find(labelname);
	if(i != LabelMap.end())
	{
		// すでにラベルがある
		eRisseCompileError::Throw(
			tRisseString(RISSE_WS_TR("label '%1' is already defined"), labelname),
				Form->GetScriptBlock(), pos);
	}

	LabelMap.insert(tLabelMap::value_type(labelname, block));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSALabelMap::AddPendingLabelJump(tRisseSSABlock * block, risse_size pos,
		const tRisseString & labelname)
{
	PendingLabelJumps.push_back(tPendingLabelJump(block, pos, labelname));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSALabelMap::BindAll()
{
	for(tPendingLabelJumps::iterator i = PendingLabelJumps.begin();
		i != PendingLabelJumps.end(); i++)
	{
		// それぞれの i について、その基本ブロックの最後にジャンプ文を生成する

		// ジャンプ先を検索
		tLabelMap::iterator label_pair = LabelMap.find(i->LabelName);
		if(label_pair == LabelMap.end())
			eRisseCompileError::Throw(
				tRisseString(RISSE_WS_TR("label '%1' is not defined"), i->LabelName),
					Form->GetScriptBlock(), i->Position);

		// ジャンプ文を生成
		tRisseSSAStatement * stmt = new tRisseSSAStatement(Form, i->Position, ocJump);
		i->Block->AddStatement(stmt);
		stmt->SetJumpTarget(label_pair->second);
	}
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
void tRisseBreakInfo::AddJump(tRisseSSAStatement * jump_stmt)
{
	PendingJumps.push_back(jump_stmt);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseBreakInfo::BindAll(tRisseSSABlock * target)
{
	for(tPendingJumps::iterator i = PendingJumps.begin(); i != PendingJumps.end(); i++)
	{
		// それぞれの i について、ジャンプ文のジャンプ先を設定する
		(*i)->SetJumpTarget(target);
	}
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
tRisseSSAForm::tRisseSSAForm(tRisseCompiler * compiler,
	tRisseASTNode * root, const tRisseString & name)
{
	Compiler = compiler;
	Parent = NULL;
	Root = root;
	Name = name;
	UniqueNumber = 0;
	LocalNamespace = new tRisseSSALocalNamespace();
	LabelMap = new tRisseSSALabelMap(this);
	EntryBlock = NULL;
	CurrentBlock = NULL;
	CurrentSwitchInfo = NULL;
	CurrentBreakInfo = NULL;
	CurrentContinueInfo = NULL;
	FunctionCollapseArgumentVariable = NULL;

	// compiler に自身を登録する
	compiler->AddSSAForm(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::SetParent(tRisseSSAForm * form)
{
	Parent = form;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::Generate()
{
	// AST をたどり、それに対応する SSA 形式を作成する

	// エントリー位置の基本ブロックを生成する
	EntryBlock = new tRisseSSABlock(this, RISSE_WS("entry"));
	LocalNamespace->SetBlock(EntryBlock);
	CurrentBlock = EntryBlock;

	// ルートノードを処理する
	Root->GenerateReadSSA(this);

	// 未バインドのラベルジャンプをすべて解決
	LabelMap->BindAll();

	// 到達しない基本ブロックからのパスを削除
	LeapDeadBlocks();

	// 変数の有効範囲を解析
	AnalyzeVariableLiveness();

	// φ関数を除去
	RemovePhiStatements();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseScriptBlockBase * tRisseSSAForm::GetScriptBlock() const
{
	return Compiler->GetScriptBlock();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSABlock * tRisseSSAForm::CreateNewBlock(const tRisseString & name, tRisseSSABlock * pred)
{
	// 今までの (Current) の基本ブロックに名前空間のスナップショットを作る
	CurrentBlock->TakeLocalNamespaceSnapshot(LocalNamespace);

	// ローカル変数名前空間をいったんすべてφ関数を見るようにマークする
	LocalNamespace->MarkToCreatePhi();

	// 新しい基本ブロックを作成する
	tRisseSSABlock * new_block = new tRisseSSABlock(this, name);

	if(pred) new_block->AddPred(pred);

	LocalNamespace->SetBlock(new_block);

	// 新しい「現在の」基本ブロックを設定し、それを返す
	CurrentBlock = new_block;
	return CurrentBlock;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAVariable * tRisseSSAForm::AddConstantValueStatement(
										risse_size pos,
										const tRisseVariant & val)
{
	// 文の作成
	tRisseSSAStatement * stmt =
		new tRisseSSAStatement(this, pos, ocAssignConstant);
	// 変数の作成
	tRisseSSAVariable * var = new tRisseSSAVariable(this, stmt);
	var->SetValue(new tRisseVariant(val));
	// 文の追加
	CurrentBlock->AddStatement(stmt);
	// 戻る
	return var;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSAStatement * tRisseSSAForm::AddStatement(risse_size pos, tRisseOpCode code,
		tRisseSSAVariable ** ret_var,
			tRisseSSAVariable *using1,
			tRisseSSAVariable *using2,
			tRisseSSAVariable *using3,
			tRisseSSAVariable *using4
			)
{
	// 文の作成
	tRisseSSAStatement * stmt =
		new tRisseSSAStatement(this, pos, code);

	if(ret_var)
	{
		// 戻りの変数の作成
		*ret_var = new tRisseSSAVariable(this, stmt);
	}

	// 文のSSAブロックへの追加
	GetCurrentBlock()->AddStatement(stmt);

	// 文に変数の使用を追加
	if(using1) stmt->AddUsed(using1);
	if(using2) stmt->AddUsed(using2);
	if(using3) stmt->AddUsed(using3);
	if(using4) stmt->AddUsed(using4);

	// 文を返す
	return stmt;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void * tRisseSSAForm::CreateLazyBlock(tRisseASTNode * node, tRisseSSAVariable *& block_var)
{
	// 遅延評価ブロックの名称を決める
	tRisseString block_name = RISSE_WS("lazy block ") + tRisseString::AsString(GetUniqueNumber());

	// 遅延評価ブロックを生成して内容をコンパイルする
	tRisseSSAForm *newform =
		new tRisseSSAForm(Compiler, node, block_name);
	newform->SetParent(this);

	// ローカル名前空間の「チェーン」につなぐ
	tRisseSSAFlatNamespace * chain = LocalNamespace->CreateFlatNamespace();
	newform->LocalNamespace->SetChained(chain);

	// 内容を生成
	newform->Generate();


	// 遅延評価ブロックを生成する文を追加する
	tRisseSSAStatement * lazy_stmt =
		AddStatement(node->GetPosition(), ocDefineLazyBlock, &block_var);
	lazy_stmt->SetName(block_name);

	// 遅延評価ブロックで読み込みが起こった変数を処理する
	chain->GenerateChildRead(this, node->GetPosition(), block_var);

	// 情報を返す
	tLazyBlockParam * param = new tLazyBlockParam();
	param->Chain = chain;
	param->Position = node->GetPosition();
	param->BlockVariable = block_var;
	return reinterpret_cast<void *>(param);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::CleanupLazyBlock(void * param)
{
	// param はtLazyBlockParam を表している
	tLazyBlockParam * info_param = reinterpret_cast<tLazyBlockParam *>(param);

	// 遅延評価ブロックで書き込みが起こった変数を処理する
	info_param->Chain->GenerateChildWrite(this, info_param->Position, info_param->BlockVariable);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSAForm::Dump() const
{
	// この form の EntryBlock から到達可能な基本ブロックをすべてダンプする
	gc_vector<tRisseSSABlock *> blocks;
	EntryBlock->Traverse(blocks);
	tRisseString ret;
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		ret += (*i)->Dump();

	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::LeapDeadBlocks()
{
	// EntryBlock から到達可能なすべての基本ブロックのマークを解除する
	// TODO: 正確な動作は「(到達可能である・なしに関わらず)すべての基本ブロックのマークを解除する」
	gc_vector<tRisseSSABlock *> blocks;
	EntryBlock->Traverse(blocks);
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->SetMark(NULL);

	// EntryBlock から到達可能なすべての基本ブロックをマークする
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->SetMark();

	// EntryBlock から到達可能なすべての基本ブロックのうち、
	// 到達できないブロックが Pred にあれば削除する
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->DeleteUnmarkedPred();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::AnalyzeVariableLiveness()
{
	// EntryBlock から到達可能なすべての基本ブロックを得る
	gc_vector<tRisseSSABlock *> blocks;
	EntryBlock->Traverse(blocks);
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->SetMark(NULL);

	// LiveIn と LiveOut を作成する
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->CreateLiveInAndLiveOut();

	// それぞれのブロック内にある変数に対して生存区間解析を行う
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->AnalyzeVariableLiveness();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::AnalyzeVariableLiveness(tRisseSSAVariable * var)
{
	// それぞれの 変数の使用位置について、変数の宣言か、この変数が使用されていると
	// マークされている箇所にまで逆順にブロックをたどる
	// (すでにたどったブロックはたどらない)

	// この変数が宣言されている文
	tRisseSSAStatement * decl_stmt = var->GetDeclared();

	// この変数が宣言されているブロック
	tRisseSSABlock * decl_block = decl_stmt->GetBlock();

	// この変数が使用されているそれぞれの文について
	const gc_vector<tRisseSSAStatement *> & used = var->GetUsed();
	for(gc_vector<tRisseSSAStatement *>::const_iterator si = used.begin();
		si != used.end(); si++)
	{
		// この文のブロックは
		tRisseSSABlock * used_block = (*si)->GetBlock();
		RISSE_ASSERT(used_block != NULL);

		// ブロックを逆順にたどる
		// 終了条件は
		// ・たどるべきブロックがなくなった場合 -> あり得ない
		// ・変数の宣言されたブロックにたどり着いた場合
		// ・すでにこの変数が使用されているとマークされているブロックにたどり着いた場合

		gc_vector<tRisseSSABlock *> Stack;
		Stack.push_back(used_block); // 初期ノードを入れる
		do
		{
			bool stop = false;

			// スタックから値を取り出す
			tRisseSSABlock * quest_block = Stack.back();
			Stack.pop_back();

			// 変数が宣言されているブロックにたどり着いた場合は、そこでこのノード
			// の先をたどるのは辞める
			if(quest_block == decl_block)
				stop = true;

			// quest_block の LiveOut または LiveIn にこの変数が追加されているか
			// 追加されているならば そこでこのノード
			// の先をたどるのは辞める
			if(!stop &&(
					quest_block->GetLiveness(var, true) ||
					quest_block->GetLiveness(var, true)))
				stop = true;

			if(!stop)
			{
				// この時点で quest_block では
				// 変数が宣言されていない→これよりも前のブロックで変数が宣言されている
				// →このブロックとpredの間では変数は生存している
				// つまり、LiveIn に変数を追加する
				quest_block->AddLiveness(var, false);

				// スタックに quest_block のpred を追加する
				for(gc_vector<tRisseSSABlock *>::const_iterator i = quest_block->GetPred().begin();
					i != quest_block->GetPred().end(); i++)
					Stack.push_back(*i);
			}
		} while(Stack.size() > 0);

	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::RemovePhiStatements()
{
	// 基本ブロックのリストを取得
	gc_vector<tRisseSSABlock *> blocks;
	EntryBlock->Traverse(blocks);

	// それぞれのブロックにつき処理
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->RemovePhiStatements();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSAForm::GenerateCode(tRisseCodeGenerator * gen) const
{
	// 子SSA形式インスタンスを含め、バイトコードを生成する

	// EntryBlock から到達可能なすべての基本ブロックを得る
	gc_vector<tRisseSSABlock *> blocks;
	EntryBlock->Traverse(blocks);

	// すべての基本ブロックに対してコード生成を行わせる
	for(gc_vector<tRisseSSABlock *>::iterator i = blocks.begin(); i != blocks.end(); i++)
		(*i)->GenerateCode(gen);

	// コードを確定する
	gen->FixCode();
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
tRisseCodeGenerator::tRisseCodeGenerator()
{
	NumUsedRegs = 0;
	MaxNumUsedRegs = 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::AddBlockMap(const tRisseSSABlock * block)
{
	BlockMap.insert(tBlockMap::value_type(block, Code.size()));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::AddPendingBlockJump(const tRisseSSABlock * block)
{
	PendingBlockJumps.push_back(std::pair<const tRisseSSABlock *, risse_size>(block, Code.size()));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_size tRisseCodeGenerator::FindConst(const tRisseVariant & value)
{
	// 直近のMaxConstSearch個をConstsから探し、一致すればそのインデックスを返す
	risse_size const_size = Consts.size();
	risse_size search_limit = const_size > MaxConstSearch ? MaxConstSearch : const_size;
	for(risse_size n = 0; n < search_limit; n++)
	{
		if(Consts[const_size - n - 1].discequal(value))
			return const_size - n - 1; // 見つかった
	}

	// 見つからなかったので Consts に追加
	Consts.push_back(value);
	return const_size; // これは Consts に push_back された値のインデックスを表しているはず
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutNoOperation()
{
	PutWord(ocNoOperation);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutAssign(const tRisseSSAVariable * dest, const tRisseSSAVariable * src)
{
	PutWord(ocAssign);
	PutWord(FindRegMap(dest));
	PutWord(FindRegMap(src));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutAssign(const tRisseSSAVariable * dest, const tRisseVariant & value)
{
	PutWord(ocAssignConstant);
	PutWord(FindRegMap(dest));
	PutWord(FindConst(value));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutAssign(const tRisseSSAVariable * dest, tRisseOpCode code)
{
	// 一応 code を assert (完全ではない)
	RISSE_ASSERT(RisseVMInsnInfo[code].Flags[0] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[code].Flags[1] == tRisseVMInsnInfo::vifVoid);

	PutWord(static_cast<risse_uint32>(code));
	PutWord(FindRegMap(dest));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutFunctionCall(const tRisseSSAVariable * dest,
		const tRisseSSAVariable * func,
		bool is_new, bool omit, risse_uint32 expbit,
		const gc_vector<const tRisseSSAVariable *> & args,
		const gc_vector<const tRisseSSAVariable *> & blocks)
{
	RISSE_ASSERT(!(is_new && blocks.size() != 0)); // ブロック付き new はない
	RISSE_ASSERT(!(omit && args.size() != 0)); // omit なのに引数があるということはない

	tRisseOpCode code;
	if(is_new)
		code = ocNew;
	else
		code = blocks.size() != 0 ? ocFuncCallBlock : ocFuncCall;

	PutWord(static_cast<risse_uint32>(code));
	PutWord(FindRegMap(dest));
	PutWord(FindRegMap(func));

	if(omit) expbit |= RisseFuncCallFlag_Omitted;
	PutWord(expbit); // フラグ

	PutWord(static_cast<risse_uint32>(args.size()));
	if(blocks.size() != 0) PutWord(static_cast<risse_uint32>(blocks.size()));

	// 引数をput
	for(gc_vector<const tRisseSSAVariable *>::const_iterator i = args.begin();
		i != args.end(); i++)
		PutWord(FindRegMap(*i));
	// ブロックをput
	for(gc_vector<const tRisseSSAVariable *>::const_iterator i = blocks.begin();
		i != blocks.end(); i++)
		PutWord(FindRegMap(*i));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutJump(const tRisseSSABlock * target)
{
	PutWord(ocJump);
	AddPendingBlockJump(target);
	PutWord(0); // 仮
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutBranch(const tRisseSSAVariable * ref,
		const tRisseSSABlock * truetarget, const tRisseSSABlock * falsetarget)
{
	PutWord(ocBranch);
	PutWord(FindRegMap(ref));
	AddPendingBlockJump(truetarget);
	PutWord(0); // 仮
	AddPendingBlockJump(falsetarget);
	PutWord(0); // 仮
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutDebugger()
{
	PutWord(ocDebugger);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutThrow(const tRisseSSAVariable * throwee)
{
	PutWord(ocThrow);
	PutWord(FindRegMap(throwee));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutReturn(const tRisseSSAVariable * value)
{
	PutWord(ocReturn);
	PutWord(FindRegMap(value));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutOperator(tRisseOpCode op, const tRisseSSAVariable * dest,
		const tRisseSSAVariable * arg1)
{
	// 一応 op を assert (完全ではない)
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[0] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[1] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[2] == tRisseVMInsnInfo::vifVoid);

	PutWord(static_cast<risse_uint32>(op));
	PutWord(FindRegMap(dest));
	PutWord(FindRegMap(arg1));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutOperator(tRisseOpCode op, const tRisseSSAVariable * dest,
		const tRisseSSAVariable * arg1, const tRisseSSAVariable * arg2)
{
	// 一応 code を assert (完全ではない)
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[0] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[1] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[2] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[3] == tRisseVMInsnInfo::vifVoid);

	PutWord(static_cast<risse_uint32>(op));
	PutWord(FindRegMap(dest));
	PutWord(FindRegMap(arg1));
	PutWord(FindRegMap(arg2));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutOperator(tRisseOpCode op, const tRisseSSAVariable * dest,
		const tRisseSSAVariable * arg1, const tRisseSSAVariable * arg2, const tRisseSSAVariable * arg3)
{
	// 一応 code を assert (完全ではない)
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[0] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[1] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[2] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[3] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[3] == tRisseVMInsnInfo::vifVoid);

	PutWord(static_cast<risse_uint32>(op));
	PutWord(FindRegMap(dest));
	PutWord(FindRegMap(arg1));
	PutWord(FindRegMap(arg2));
	PutWord(FindRegMap(arg3));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::PutSet(tRisseOpCode op, const tRisseSSAVariable * obj,
		const tRisseSSAVariable * name, const tRisseSSAVariable * value)
{
	// 一応 code を assert (完全ではない)
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[0] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[1] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[2] == tRisseVMInsnInfo::vifRegister);
	RISSE_ASSERT(RisseVMInsnInfo[op].Flags[3] == tRisseVMInsnInfo::vifVoid);

	PutWord(static_cast<risse_uint32>(op));
	PutWord(FindRegMap(obj));
	PutWord(FindRegMap(name));
	PutWord(FindRegMap(value));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_size tRisseCodeGenerator::FindRegMap(const tRisseSSAVariable * var)
{
	// RegMap から指定された変数を探す
	// 指定された変数が無ければ変数の空きマップからさがし、変数を割り当てる
	tRegMap::iterator f = RegMap.find(var);
	if(f != RegMap.end())
	{
		 // 変数が見つかった
		 return f->second;
	}

	// 変数がないので空きレジスタを探す
	risse_size assigned_reg;
	if(RegFreeMap.size() == 0)
	{
		// 空きレジスタがない
		assigned_reg = NumUsedRegs; // 新しいレジスタを割り当てる
	}
	else
	{
		// 空きレジスタがある
		assigned_reg = RegFreeMap.back();
		RegFreeMap.pop_back();
	}

	NumUsedRegs ++;
	if(MaxNumUsedRegs < NumUsedRegs) MaxNumUsedRegs = NumUsedRegs;

	RegMap.insert(tRegMap::value_type(var, assigned_reg)); // RegMapに登録

	return assigned_reg;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::FreeUnusedRegisters(const tRisseSSABlock *block)
{
	gc_vector<const tRisseSSAVariable *> free; // 開放すべき変数

	// RegMap にある変数をすべて見る
	for(tRegMap::iterator i = RegMap.begin(); i != RegMap.end(); i++)
	{
		if(!block->GetLiveness(i->first, true))
		{
			// RegMap にあって block の LiveOut にない
			free.push_back(i->first);
		}
	}

	// free にある変数をすべてRegMapから削除する
	for(gc_vector<const tRisseSSAVariable *>::iterator i = free.begin(); i != free.end(); i++)
	{
		tRegMap::iterator f = RegMap.find(*i);
		RISSE_ASSERT(f != RegMap.end());
		RegFreeMap.push_back(f->second); // 変数を空き配列に追加
		NumUsedRegs --;
		RegMap.erase(f); // 削除
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCodeGenerator::FixCode()
{
	// ジャンプアドレスのfixup
	for(gc_vector<std::pair<const tRisseSSABlock *, risse_size> >::iterator i =
		PendingBlockJumps.begin(); i != PendingBlockJumps.end(); i++)
	{
		tBlockMap::iterator b = BlockMap.find(i->first);
		RISSE_ASSERT(b != BlockMap.end());
		Code[i->second] = b->second;
	}
}
//---------------------------------------------------------------------------








}
# include "risseCodeBlock.h"
namespace Risse{
//---------------------------------------------------------------------------
void tRisseCompiler::Compile(tRisseASTNode * root, bool need_result, bool is_expression)
{
	// (テスト) ASTのダンプを行う
	RisseFPrint(stdout, RISSE_WS("---------- AST ----------\n"));
	tRisseString str;
	root->Dump(str);
	RisseFPrint(stdout, str.c_str());

	// (テスト)
	tRisseSSAForm * form = new tRisseSSAForm(this, root, RISSE_WS("root"));
	form->Generate();

	// SSA 形式のダンプ
	for(gc_vector<tRisseSSAForm *>::iterator i = SSAForms.begin();
		i != SSAForms.end(); i++)
	{
		RisseFPrint(stdout,(	RISSE_WS("---------- SSA (") + (*i)->GetName() +
								RISSE_WS(") ----------\n")).c_str());
		str = (*i)->Dump();
		RisseFPrint(stdout, str.c_str());
	}

	// SSA 形式のダンプ
	for(gc_vector<tRisseSSAForm *>::iterator i = SSAForms.begin();
		i != SSAForms.end(); i++)
	{
		RisseFPrint(stdout,(	RISSE_WS("---------- VM (") + (*i)->GetName() +
								RISSE_WS(") ----------\n")).c_str());

		tRisseCodeGenerator *gen = new tRisseCodeGenerator();
		(*i)->GenerateCode(gen);
		tRisseCodeBlock * cb = new tRisseCodeBlock(gen);
		str = cb->Dump();
		RisseFPrint(stdout, str.c_str());
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseCompiler::AddSSAForm(tRisseSSAForm * ssaform)
{
	SSAForms.push_back(ssaform);
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
} // namespace Risse
