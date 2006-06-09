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


namespace Risse
{
RISSE_DEFINE_SOURCE_ID(52364,51758,14226,19534,54934,29340,32493,12680);

//---------------------------------------------------------------------------
tRisseSSALocalNamespace::tRisseSSALocalNamespace()
{
	Block = NULL;
	Push(); // 最初の名前空間を push しておく
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseSSALocalNamespace::tRisseSSALocalNamespace(const tRisseSSALocalNamespace &ref)
{
	Block = ref.Block;
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
	risse_char num_str[40];
	Risse_int_to_str(num, num_str);

	return name + RISSE_WC('#') + num_str;
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
			std::pair<tRisseString, tRisseString>(name, n_name)); // 新規に挿入

	// 番号付きの名前を登録する
	Scopes.back()->VariableMap.insert(std::pair<tRisseString, tRisseSSAVariable *>(n_name, where));

	// 名前と番号付きの名前を where に設定する
	where->SetName(name);
	where->SetNumberedName(n_name);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisseSSALocalNamespace::Update(const tRisseString & name, tRisseSSAVariable * where)
{
	RISSE_ASSERT(where != NULL);

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

			vi->second = where; // 上書き

			// 番号付きの名前を where に設定する
			where->SetName(name);
			where->SetNumberedName(n_name);

			return; // 戻る
		}
	}

	// 見つからない
	RISSE_ASSERT(!"variable not found in tRisseSSALocalNamespace::Update");
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
tRisseSSAVariable::tRisseSSAVariable(tRisseSSAForm * form, 
	tRisseSSAStatement *stmt, const tRisseString & name)
{
	// フィールドの初期化
	Form = form;
	Declared = stmt;
	Value = NULL;
	ValueType = tRisseVariant::vtVoid;

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
	// Version を文字列化
	risse_char uniq[40];
	Risse_int_to_str(Version, uniq);

	if(Name.IsEmpty())
	{
		// 一時変数の場合は _tmp@ の次にバージョン文字列
		return tRisseString(RISSE_WS("_tmp@")) + uniq;
	}
	else
	{
		// NumberedName がある場合はそれを、Nameを使う
		tRisseString n;
		if(!NumberedName.IsEmpty()) n = NumberedName; else n = Name;
		// 普通の変数の場合は 変数名@バージョン文字列
		return n + RISSE_WC('@') + uniq;
	}
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
				ret += tRisseString(RisseOpCodeNames[Code]);

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
	InDump = Dumped = false;

	// 通し番号の準備
	risse_char uniq[40];
	Risse_int_to_str(form->GetUniqueNumber(), uniq);

	Name = name + RISSE_WC('_') + uniq;
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
	if(!FirstStatement)
	{
		// 最初の文
		FirstStatement = LastStatement = stmt;
	}
	else
	{
		// ２つ目以降の文
		FirstStatement->SetPred(stmt);
		stmt->SetSucc(FirstStatement);
		FirstStatement = stmt;
	}

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
void tRisseSSABlock::AddSucc(tRisseSSABlock * block)
{
	Succ.push_back(block);
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
void tRisseSSABlock::ClearDumpFlags() const
{
	InDump = true;
	Dumped = false;
	for(gc_vector<tRisseSSABlock *>::const_iterator i = Succ.begin();
									i != Succ.end(); i ++)
	{
		if(!(*i)->InDump)
			(*i)->ClearDumpFlags();
	}
	InDump = false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSABlock::Dump() const
{
	tRisseString ret;

	Dumped = true; // ２回以上ダンプしないように

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

	if(!FirstStatement)
	{
		// 一つも文を含んでいない？？
		ret += RISSE_WS("This SSA basic block does not contain any statements\n\n");
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

		ret += RISSE_WS("\n");
	}

	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSABlock::DumpChildren() const
{
	tRisseString ret;

	InDump = true; // 再入しないように
	try
	{
		gc_vector<tRisseSSABlock *> vec;

		// 直後のブロックをダンプ
		for(gc_vector<tRisseSSABlock *>::const_iterator i = Succ.begin();
										i != Succ.end(); i ++)
		{
			if(!(*i)->InDump && !(*i)->Dumped)
			{
				vec.push_back(*i); // いったん vec に入れる
				ret += (*i)->Dump();
			}
		}

		// 直後のブロックを再帰
		for(gc_vector<tRisseSSABlock *>::const_iterator i = vec.begin();
										i != vec.end(); i ++)
		{
			ret += (*i)->DumpChildren();
		}

	}
	catch(...)
	{
		InDump = false;
		throw;
	}
	InDump = false;

	return ret;
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

	LabelMap.insert(std::pair<tRisseString, tRisseSSABlock *>(labelname, block));
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
tRisseSSAForm::tRisseSSAForm(tRisseScriptBlockBase * scriptblock, tRisseASTNode * root)
{
	ScriptBlock = scriptblock;
	Root = root;
	UniqueNumber = 0;
	LocalNamespace = new tRisseSSALocalNamespace();
	LabelMap = new tRisseSSALabelMap(this);
	EntryBlock = NULL;
	CurrentBlock = NULL;
	CurrentSwitchInfo = NULL;
	CurrentBreakInfo = NULL;
	CurrentContinueInfo = NULL;
	FunctionCollapseArgumentVariable = NULL;
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
			tRisseSSAVariable *using3)
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

	// 文を返す
	return stmt;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisseString tRisseSSAForm::Dump() const
{
	// この form から到達可能な基本ブロックをすべてダンプする
	return EntryBlock->Dump() + EntryBlock->DumpChildren();
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
} // namespace Risse
