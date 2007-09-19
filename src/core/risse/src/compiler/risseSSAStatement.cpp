//---------------------------------------------------------------------------
/*
	Risse [りせ]
	 stands for "Risse Is a Sweet Script Engine"
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief SSA形式における「文」
//---------------------------------------------------------------------------
#include "../prec.h"

#include "risseSSAStatement.h"
#include "risseSSAVariable.h"
#include "risseSSABlock.h"
#include "risseCodeGen.h"
#include "risseSSAForm.h"
#include "risseCompiler.h"

namespace Risse
{
RISSE_DEFINE_SOURCE_ID(33139,58829,49251,19299,61572,36837,14859,14043);



//---------------------------------------------------------------------------
tSSAStatement::tSSAStatement(tSSAForm * form,
	risse_size position, tOpCode code)
{
	// フィールドの初期化
	Form = form;
	Position = position;
	Id = Form->GetFunction()->GetFunctionGroup()->GetCompiler()->GetUniqueNumber();
		// Id は文と文を識別できる程度にユニークであればよい
	Code = code;
	Block = NULL;
	Pred = NULL;
	Succ = NULL;
	Declared = NULL;
	Order = risse_size_max;
	FuncExpandFlags = 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::AddUsed(tSSAVariable * var)
{
	var->AddUsed(this);
	Used.push_back(var);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::DeleteUsed(risse_size index)
{
	RISSE_ASSERT(Used.size() > index);
	tSSAVariable * var = Used[index];
	Used.erase(Used.begin() + index);
	var->DeleteUsed(this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::OverwriteUsed(tSSAVariable * old_var, tSSAVariable * new_var)
{
	gc_vector<tSSAVariable*>::iterator i = std::find(Used.begin(), Used.end(), old_var);

	// すでに old_var が new_var に書き換わっている可能性もあるので
	// 一応 ASSERT はするが、old_var が無い場合は何もしない
	RISSE_ASSERT(i != Used.end() ||
		std::find(Used.begin(), Used.end(), new_var) != Used.end());

	if(i != Used.end())
		*i = new_var;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::DeleteUsed()
{
	risse_size i = Used.size();
	while(i > 0)
	{
		i--;

		DeleteUsed(i);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::TraceCoalescable()
{
	switch(Code)
	{
	case ocPhi: // phi関数
		RISSE_ASSERT(Declared != NULL);

		// 2 引数以上のphi関数
		if(Used.size() > 1)
		{
			// Used と Declared 間の干渉を探る
			// Used を一つずつみていき、Declared と Used が干渉している場合は
			// 干渉を除去する。 Sreedhar らによる方法。
			bool interference_found = false;
			for(gc_vector<tSSAVariable*>::iterator i = Used.begin();
				i != Used.end(); i++)
			{
				if((*i)->CheckInterferenceWith(Declared))
				{
					interference_found = true;
					break;
				}
			}

			if(interference_found)
			{
				tSSAVariable * orig_decl_var = Declared;
				tSSAVariable * tmp_var = new tSSAVariable(Form, NULL, orig_decl_var->GetName());
wxFprintf(stderr, wxT("variable interference found at phi statement, inserting %s at %s\n"),
	tmp_var->GetQualifiedName().AsWxString().c_str(),
	Block->GetName().AsWxString().c_str());
				tSSAStatement * new_stmt =
					new tSSAStatement(Form, Position, ocAssign);
				new_stmt->AddUsed(const_cast<tSSAVariable*>(tmp_var));
				orig_decl_var->SetDeclared(new_stmt);
				new_stmt->SetDeclared(orig_decl_var);
				tmp_var->SetDeclared(this);
				this->SetDeclared(tmp_var);
				Block->InsertStatement(new_stmt, tSSABlock::sipAfterPhi);

				// 干渉グラフを更新する。
				// tmp_var は ここ一連のすべてのphi関数の宣言、およびその使用変数すべてと
				// 干渉すると見なす。
				// ブロック単位の livein, liveout には影響しない。
				tSSAStatement * cur = this;
				tSSAStatement * first = cur;
				while(cur) { first = cur; cur = cur->Pred; }
				for(cur = first; cur && cur->Code == ocPhi; cur = cur->Succ)
				{
					RISSE_ASSERT(cur->Declared);
					if(tmp_var != cur->Declared) tmp_var->SetInterferenceWith(cur->Declared);
					for(gc_vector<tSSAVariable*>::iterator i =cur-> Used.begin();
						i != cur->Used.end(); i++)
					{
						if(tmp_var != *i)
							tmp_var->SetInterferenceWith(*i);
					}
				}

				// この時点で Declared は、新しく定義されたテンポラリ変数を指しているはず
				RISSE_ASSERT(Declared == tmp_var);
			}

			// 関連する変数の合併を行う
			for(gc_vector<tSSAVariable*>::iterator i = Used.begin();
				i != Used.end(); i++)
					Declared->CoalesceCoalescableList(*i);

			break;
		}

		// 一応 ここに break は *おかない*
		// 1引数のphi関数は単純な代入として扱うため

	case ocAssign: // 単純代入

		// 昔はここでコピー伝播を行おうとしていたのだけれども
		// コピー伝播は他の所でやってます

	default: ;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::Coalesce()
{
	// Declared に対して合併を実行する
	// これにより SSA 性は破壊される
	if(Declared) Declared->Coalesce();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::SetTrueBranch(tSSABlock * block)
{
	RISSE_ASSERT(Code == ocBranch);
	if(Targets.size() != 2) Targets.resize(2, NULL);
	Targets[0] = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tSSABlock * tSSAStatement::GetTrueBranch() const
{
	RISSE_ASSERT(Code == ocBranch);
	RISSE_ASSERT(Targets.size() == 2);
	return Targets[0];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::SetFalseBranch(tSSABlock * block)
{
	RISSE_ASSERT(Code == ocBranch);
	if(Targets.size() != 2) Targets.resize(2, NULL);
	Targets[1] = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tSSABlock * tSSAStatement::GetFalseBranch() const
{
	RISSE_ASSERT(Code == ocBranch);
	RISSE_ASSERT(Targets.size() == 2);
	return Targets[1];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::SetJumpTarget(tSSABlock * block)
{
	RISSE_ASSERT(Code == ocJump);
	if(Targets.size() != 1) Targets.resize(1, NULL);
	Targets[0] = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tSSABlock * tSSAStatement::GetJumpTarget() const
{
	RISSE_ASSERT(Code == ocJump);
	RISSE_ASSERT(Targets.size() == 1);
	return Targets[0];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::SetTryExitTarget(tSSABlock * block)
{
	RISSE_ASSERT(Code == ocCatchBranch);
	if(Targets.size() < 2) Targets.resize(2, NULL);
	Targets[0] = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tSSABlock * tSSAStatement::GetTryExitTarget() const
{
	RISSE_ASSERT(Code == ocCatchBranch);
	RISSE_ASSERT(Targets.size() >= 2);
	return Targets[0];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::SetTryCatchTarget(tSSABlock * block)
{
	RISSE_ASSERT(Code == ocCatchBranch);
	if(Targets.size() < 2) Targets.resize(2, NULL);
	Targets[1] = block;
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tSSABlock * tSSAStatement::GetTryCatchTarget() const
{
	RISSE_ASSERT(Code == ocCatchBranch);
	RISSE_ASSERT(Targets.size() >= 2);
	return Targets[1];
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::AddTarget(tSSABlock * block)
{
	RISSE_ASSERT(Code == ocCatchBranch);
	RISSE_ASSERT(Targets.size() >= 2);
	Targets.push_back(block);
	block->AddPred(Block);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::SetName(const tString & name)
{
	RISSE_ASSERT(
			Code == ocParentRead || Code == ocParentWrite ||
			Code == ocChildRead || Code == ocChildWrite ||
			Code == ocReadVar || Code == ocWriteVar ||
			Code == ocRead || Code == ocWrite ||
			Code == ocDefineLazyBlock || Code == ocDefineClass || Code == ocAddBindingMap);
	Name = new tString(name);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
const tString & tSSAStatement::GetName() const
{
	RISSE_ASSERT(
			Code == ocParentRead || Code == ocParentWrite ||
			Code == ocChildRead || Code == ocChildWrite ||
			Code == ocReadVar || Code == ocWriteVar ||
			Code == ocRead || Code == ocWrite ||
			Code == ocDefineLazyBlock || Code == ocDefineClass || Code == ocAddBindingMap);
	RISSE_ASSERT(Name != NULL);
	return *Name;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::CreateVariableInterferenceGraph(gc_map<const tSSAVariable *, risse_size> &livemap)
{
	// 一応このメソッドが実行時には SSA性が保持されていると見なす
	RISSE_ASSERT(Block->GetForm()->GetState() == tSSAForm::ssSSA);
	RISSE_ASSERT(Order != risse_size_max); // Order が設定されていること

wxFprintf(stderr, wxT("at [%d]:"), (int)Order);

	// この文で定義された変数があるならば livemap にその変数を追加する
	bool has_new_declared = false;
	if(Declared)
	{
		// ただし、dead store な場合は追加しない
		if(Declared->GetUsed().size() != 0)
		{
			// 既にlivemap にあるわけがない(LiveInにそれがある場合は除く)
			RISSE_ASSERT(Block->GetLiveness(Declared, false) ||
				livemap.find(Declared) == livemap.end());
			// livemap に追加
			livemap.insert(gc_map<const tSSAVariable *, risse_size>::value_type(Declared, risse_size_max));
			has_new_declared = true;
wxFprintf(stderr, wxT("adding %s  "), Declared->GetQualifiedName().AsWxString().c_str());
		}
	}

	bool interf_added = false;
	if(!(Code == ocAssign || Code == ocPhi))
	{
		// 一時的処置
		// TODO: これあとで削除
		// 現時点では各命令は3アドレス方式をとっており、また
		// レジスタの read と write が同じレジスタにたいしておこると
		// おかしくなる場合があるため、全面的に2アドレス方式をとるまでは
		// 一時的に read と write が干渉している (=別のレジスタに割り当たる)
		// ようにする。ただし ocAssign / ocPhi は別。
		if(Declared)
		{
			for(gc_map<const tSSAVariable *, risse_size>::iterator li = livemap.begin();
				li != livemap.end(); li++)
			{
				if(li->first == Declared) continue;
				Declared->SetInterferenceWith(const_cast<tSSAVariable *>(li->first));
wxFprintf(stderr, wxT("interf %s - %s  "), Declared->GetQualifiedName().AsWxString().c_str(), li->first->GetQualifiedName().AsWxString().c_str());
			}
		}
		interf_added = true;
	}

	// この文で使用された変数があり、それがこの文で使用が終了していればlivemapから削除する
	// この文で使用が終了しているかどうかの判定は、
	// ・ブロックのLiveOut にその変数がない
	// かつ
	// ・このブロックにこれ以降この変数を使用している箇所がない
	for(gc_vector<tSSAVariable*>::const_iterator i = Used.begin();
		i != Used.end(); i++)
		(*i)->SetMark(NULL); // マークをいったんクリア

	for(gc_vector<tSSAVariable*>::const_iterator i = Used.begin();
		i != Used.end(); i++)
	{
		// この変数は既に処理をしたか
		if((*i)->GetMark() != NULL) continue; // 既に処理されている
		(*i)->SetMark(this);

		// Block の LiveOut にその変数があるか
		if(Block->GetLiveness(*i, true)) continue; // まだ生きている
		// このブロックにこれ以降この変数を使用している箇所がないか
		bool is_last = true;
		risse_size current_order = Order;
		const gc_vector<tSSAStatement *> & used_list = (*i)->GetUsed();
		for(gc_vector<tSSAStatement *>::const_iterator si = used_list.begin();
			si != used_list.end(); si++)
		{
			if((*si)->Block == Block)
			{
				risse_size order = (*si)->GetOrder();
				if(order > current_order)
				{
					is_last = false;
					break;
				}
			}
		}

		if(is_last)
		{
			// livemapからこれを削除する
			// φ関数などで前の関数と同じ順位の場合は
			// 前の関数ですでに変数が削除されている可能性がある
wxFprintf(stderr, wxT("deleting %s:"), (*i)->GetQualifiedName().AsWxString().c_str());
			gc_map<const tSSAVariable *, risse_size>::iterator fi =
				livemap.find((*i));
			RISSE_ASSERT(Code == ocPhi || fi != livemap.end());
			if(fi != livemap.end())
			{
wxFprintf(stderr, wxT("deleted  "), (*i)->GetQualifiedName().AsWxString().c_str());
				livemap.erase(fi);
			}
			else
			{
wxFprintf(stderr, wxT("not found  "), (*i)->GetQualifiedName().AsWxString().c_str());
			}
		}
	}

	// 宣言された変数がある場合は、変数の干渉を追加する
	if(!interf_added)
	{
		if(Declared)
		{
			for(gc_map<const tSSAVariable *, risse_size>::iterator li = livemap.begin();
				li != livemap.end(); li++)
			{
				if(li->first == Declared) continue;
				Declared->SetInterferenceWith(const_cast<tSSAVariable *>(li->first));
wxFprintf(stderr, wxT("interf %s - %s  "), Declared->GetQualifiedName().AsWxString().c_str(), li->first->GetQualifiedName().AsWxString().c_str());
			}
		}
	}

wxFprintf(stderr, wxT("\n"));

}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::OptimizeAtStatementLevel(gc_map<risse_size, tSSAStatement *> &statements)
{
	// コピー伝播
	if(Code == ocPhi && Used.size() == 1 ||
		Code == ocAssign)
	{
		// 引数が一個の phi あるいは単純コピーの場合
		// コピー伝播を行う
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1);
		// コピー先である Declared で使用している文に対し、
		// Declared を使用している所をすべて Used[0] に置き換える。
		const gc_vector<tSSAStatement *> & used_list = Declared->GetUsed();
		for(gc_vector<tSSAStatement *>::const_iterator si = used_list.begin();
			si != used_list.end(); si++)
		{
			if(*si == this) continue; // 自分自身は除外
			(*si)->OverwriteUsed(Declared, Used[0]); // Declared を Used[0] に置き換え
			// Used[0] の Used に (*si) を追加
			Used[0]->AddUsed(*si);
		}
		// Used[0] の Used から this を削除
		Used[0]->DeleteUsed(this);
		// this を block から削除
		Block->DeleteStatement(this);
		// statements から this を削除
		gc_map<risse_size, tSSAStatement *>::iterator sti = statements.find(this->GetId());
		if(sti != statements.end()) statements.erase(sti);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::AnalyzeConstantPropagation(
		gc_vector<tSSAVariable *> &variables,
		gc_vector<tSSABlock *> &blocks)
{
	// Code ごとに処理を行う

	if(!Declared) return; // Declared が無い文は相手にしない

	// Declared の古い状態をとっておく
	tSSAVariable::tValueState old_value_state     = Declared->GetValueState();
	tSSAVariable::tValueState old_valuetype_state = Declared->GetValueTypeState();

	// phi 関数以外は、基本的に使用されいてる値によって定義されている
	// 変数の型や値がどうなるかを考える
	// たぶんここら辺の話は Modern Compiler Implementation in * に
	// 書いてあると思う

	switch(Code)
	{
	//--------------- φ関数
	case ocPhi:
	{
		const gc_vector<tSSABlock *> & pred_blocks = Block->GetPred();
		RISSE_ASSERT(Used.size() == pred_blocks.size());
		bool done;

		// なかなかコンパクトな実装が見つからないわけだが、ここは
		// 値に対してと型に対してまったく同じアルゴリズムを実装する
		// ために、似たようなコードを２回繰り返して書いてあるので注意すること。

		//----- 値に対して
		// 実行可能なpredブロックから来た変数にvsVaryingな物が一つでもあれば
		// このφ関数で定義された変数もvsVaryingになる
		done = false;
		for(risse_size i = 0; i < pred_blocks.size(); i++)
		{
			if(pred_blocks[i]->GetAlive() && Used[i]->GetValueState() == tSSAVariable::vsVarying)
			{
				Declared->RaiseValueState(tSSAVariable::vsVarying);
				done = true;
				break;
			}
		}
		// すでに定数であるとマークされている変数の方向の実行可能なpredの定数の
		// 値が異なる場合
		// このφ関数で定義された変数もvsVaryingになる
		if(!done)
		{
			tVariant C;
			bool const_found = false;
			for(risse_size i = 0; i < pred_blocks.size(); i++)
			{
				if(pred_blocks[i]->GetAlive() && Used[i]->GetValueState() == tSSAVariable::vsSet)
				{
					if(!const_found)
					{
						const_found = true;
						C = Used[i]->GetValue();
					}
					else
					{
						if(!Used[i]->GetValue().DiscEqual(C))
						{
							// 値が違う
							Declared->RaiseValueState(tSSAVariable::vsVarying);
							done = true;
							break;
						}
					}
				}
			}
		}
		// どれか一つ、実行可能なpredから来た定数をとるUsedがあったとしてその値をCとする
		// 他のすべてのUsedが下のいずれかの場合
		// ・vsNotSet
		// ・値がC
		// ・その方向のPredが実行不可能
		// このφ関数で定義された変数は C になる
		if(!done)
		{
			// C を探す
			risse_size C_idx = risse_size_max;
			tVariant C;
			for(risse_size i = 0; i < pred_blocks.size(); i++)
			{
				if(pred_blocks[i]->GetAlive() && Used[i]->GetValueState() == tSSAVariable::vsSet)
				{
					C = Used[i]->GetValue();
					C_idx = i;
					break;
				}
			}
			if(C_idx != risse_size_max)
			{
				bool fail = false;
				for(risse_size i = 0; i < pred_blocks.size(); i++)
				{
					if(i == C_idx) continue;
					if(!(
						Used[i]->GetValueState() == tSSAVariable::vsNotSet ||
						Used[i]->GetValueState() == tSSAVariable::vsSet &&
							Used[i]->GetValue().DiscEqual(C) ||
						!pred_blocks[i]->GetAlive()
						))
					{
						fail = true;
						break;
					}
				}
				if(!fail)
				{
					Declared->RaiseValueState(tSSAVariable::vsSet);
					Declared->SetValue(C);
				}
			}
		}

		//----- 型に対して
		// 実行可能なpredブロックから来た変数の型にvsVaryingな物が一つでもあれば
		// このφ関数で定義された変数の型もvsVaryingになる
		done = false;
		for(risse_size i = 0; i < pred_blocks.size(); i++)
		{
			if(pred_blocks[i]->GetAlive() && Used[i]->GetValueTypeState() == tSSAVariable::vsVarying)
			{
				Declared->RaiseValueTypeState(tSSAVariable::vsVarying);
				done = true;
				break;
			}
		}
		// すでに型が分かっている変数の方向の実行可能なpredの定数の
		// 値が異なる場合
		// このφ関数で定義された変数もvsVaryingになる
		if(!done)
		{
			tVariant::tType C;
			bool const_found = false;
			for(risse_size i = 0; i < pred_blocks.size(); i++)
			{
				if(pred_blocks[i]->GetAlive() && Used[i]->GetValueTypeState() == tSSAVariable::vsSet)
				{
					if(!const_found)
					{
						const_found = true;
						C = Used[i]->GetValueType();
					}
					else
					{
						if(!Used[i]->GetValueType() == C)
						{
							// 値が違う
							Declared->RaiseValueTypeState(tSSAVariable::vsVarying);
							done = true;
							break;
						}
					}
				}
			}
		}
		// どれか一つ、実行可能なpredから来た定数をとるUsedがあったとしてその値をCとする
		// 他のすべてのUsedが下のいずれかの場合
		// ・vsNotSet
		// ・値がC
		// ・その方向のPredが実行不可能
		// このφ関数で定義された変数は C になる
		if(!done)
		{
			// C を探す
			risse_size C_idx = risse_size_max;
			tVariant::tType C;
			for(risse_size i = 0; i < pred_blocks.size(); i++)
			{
				if(pred_blocks[i]->GetAlive() && Used[i]->GetValueTypeState() == tSSAVariable::vsSet)
				{
					C = Used[i]->GetValueType();
					C_idx = i;
					break;
				}
			}
			if(C_idx != risse_size_max)
			{
				bool fail = false;
				for(risse_size i = 0; i < pred_blocks.size(); i++)
				{
					if(i == C_idx) continue;
					if(!(
						Used[i]->GetValueTypeState() == tSSAVariable::vsNotSet ||
						Used[i]->GetValueTypeState() == tSSAVariable::vsSet &&
							Used[i]->GetValueType() == C ||
						!pred_blocks[i]->GetAlive()
						))
					{
						fail = true;
						break;
					}
				}
				if(!fail)
				{
					Declared->RaiseValueTypeState(tSSAVariable::vsSet);
					Declared->SetValueType(C);
				}
			}
		}

		break;
	}

	//--------------- 単純代入
	case ocAssign:
		RISSE_ASSERT(Used.size() == 1);
		RISSE_ASSERT(Declared != NULL);
		Declared->RaiseValueState(Used[0]->GetValueState());
		Declared->RaiseValueTypeState(Used[0]->GetValueTypeState());
		if(Declared->GetValueState() == tSSAVariable::vsSet)
			Declared->SetValue(Used[0]->GetValue());
		if(Declared->GetValueTypeState() == tSSAVariable::vsSet)
			Declared->SetValueType(Used[0]->GetValueType());
		break;

	//--------------- 定数代入
	case ocAssignConstant:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Value != NULL);
		Declared->SuggestValue(*Value);
		Declared->SuggestValueType(Value->GetType());
		break;

	//--------------- 値はどうなるかわからないが、型は vtObject を代入する物
	case ocAssignNewBinding:
	case ocAssignThisProxy:
	case ocAssignGlobal:
	case ocAssignNewArray:
	case ocAssignNewDict:
	case ocAssignNewRegExp:
	case ocAssignNewFunction:
	case ocAssignNewProperty:
	case ocAssignNewClass:
	case ocAssignNewModule:
	case ocTryFuncCall:
	case ocSync:
	case ocSetFrame:
	case ocSetShare:
		RISSE_ASSERT(Declared != NULL);
		Declared->RaiseValueState(tSSAVariable::vsVarying); // どんな値になるかはわからない
		Declared->SuggestValueType(tVariant::vtObject);
		break;

	//--------------- 値も型もどうなるかわからないもの
	case ocGetExitTryValue:
	case ocAssignThis:
	case ocAssignParam:
	case ocAssignBlockParam:
	case ocRead:
	case ocNew:
	case ocFuncCall:
	case ocFuncCallBlock:
		RISSE_ASSERT(Declared != NULL);
		Declared->RaiseValueState(tSSAVariable::vsVarying); // どんな値になるかはわからない
		Declared->RaiseValueTypeState(tSSAVariable::vsVarying); // どんな型になるかはわからない
		break;

	//--------------- 宣言する変数がない物
	case ocAddBindingMap:
	case ocWrite:
	case ocReturn:
	case ocDebugger:
	case ocThrow:
	case ocExitTryException:
		// declared の無いタイプ
		RISSE_ASSERT(Declared == NULL);
		break;

	//--------------- 分岐関連
	case ocJump:
	case ocBranch:
	case ocCatchBranch:
		break;

	//--------------- 結果はUsedに依存し、boolean を帰す物
	//-- 単項
	case ocLogNot:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1);
		Declared->SuggestValueType(tVariant::vtBoolean); // 常に boolean
		if(Used[0]->GetValueState() == tSSAVariable::vsSet)
			Declared->SuggestValue(Used[0]->GetValue().LogNot());
		break;

	case ocLogOr:
	case ocLogAnd:
	case ocNotEqual:
	case ocEqual:
	case ocDiscNotEqual:
	case ocDiscEqual:
	case ocLesser:
	case ocGreater:
	case ocLesserOrEqual:
	case ocGreaterOrEqual:


	case ocBitNot:
	case ocDecAssign:
	case ocIncAssign:
	case ocPlus:
	case ocMinus:
	case ocString:
	case ocBoolean:
	case ocReal:
	case ocInteger:
	case ocOctet:
	case ocBitOr:
	case ocBitXor:
	case ocBitAnd:
	case ocRBitShift:
	case ocLShift:
	case ocRShift:
	case ocMod:
	case ocDiv:
	case ocIdiv:
	case ocMul:
	case ocAdd:
	case ocSub:
	case ocInContextOf:
	case ocInContextOfDyn:
	case ocInstanceOf:
	case ocDGet:
	case ocDGetF:
	case ocIGet:
	case ocDDelete:
	case ocIDelete:
	case ocDSetAttrib:
	case ocDSet:
	case ocDSetF:
	case ocISet:
	case ocAssert:
	case ocBitAndAssign:
	case ocBitOrAssign:
	case ocBitXorAssign:
	case ocSubAssign:
	case ocAddAssign:
	case ocModAssign:
	case ocDivAssign:
	case ocIdivAssign:
	case ocMulAssign:
	case ocLogOrAssign:
	case ocLogAndAssign:
	case ocRBitShiftAssign:
	case ocLShiftAssign:
	case ocRShiftAssign:
	case ocSetDefaultContext:
	case ocGetDefaultContext:
	case ocDefineAccessMap:
	case ocDefineLazyBlock:
	case ocDefineClass:
	case ocEndAccessMap:
	case ocParentWrite:
	case ocParentRead:
	case ocChildWrite:
	case ocChildRead:
	case ocWriteVar:
	case ocReadVar:
	case ocOpCodeLast:


	//--------------- ???? な物
	case ocAssignSuper:
	case ocNoOperation:
	case ocVMCodeLast:
		// とりあえず Declared を varying に設定してしまおう
		Declared->RaiseValueState(tSSAVariable::vsVarying); // どんな値になるかはわからない
		Declared->RaiseValueTypeState(tSSAVariable::vsVarying); // どんな型になるかはわからない
		break;

	}

	// Declared の ValueState や ValueTypeState がランクアップしているようだったら
	// variables に Declared を push する
	if(old_value_state < Declared->GetValueState() ||
		old_valuetype_state < Declared->GetValueTypeState())
		variables.push_back(Declared);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::Check3AddrAssignee()
{
	// VM命令の中にはdestinationとその他の引数が同じだった場合に異常な動作をする
	// 命令があるため、ここで暫定的に対処。本来はコードジェネレータがやるべき
	// 仕事かもしれないね。
	if(!Declared) return;
	if(Code == ocAssign || Code == ocPhi) return; // 単純コピーやφ関数は無視

	bool do_save = false;
	for(gc_vector<tSSAVariable*>::const_iterator i = Used.begin();
		i != Used.end(); i++)
	{
		if((*i) == Declared)
		{
			do_save = true;
			break;
		}
	}

	if(do_save)
	{
		tSSAVariable * orig_decl_var = Declared;
		tSSAVariable * tmp_var = new tSSAVariable(Form, NULL, orig_decl_var->GetName());
wxFprintf(stderr, wxT("assignee is the same with the argument, inserting %s after [%d]\n"),
tmp_var->GetQualifiedName().AsWxString().c_str(),
(int)Order);
		tSSAStatement * new_stmt =
			new tSSAStatement(Form, Position, ocAssign);
		new_stmt->AddUsed(const_cast<tSSAVariable*>(tmp_var));
		orig_decl_var->SetDeclared(new_stmt);
		new_stmt->SetDeclared(orig_decl_var);
		tmp_var->SetDeclared(this);
		this->SetDeclared(tmp_var);
		Block->InsertStatement(new_stmt, this);

		// 干渉グラフを更新する。
		// tmp_var はここで生きているすべての文と干渉すると見なす。
		// ここで生きているすべての文というのは、この文の使用と宣言
		// そのものと、それが干渉している物すべて。
		// ブロック単位の livein, liveout には影響しない。
		for(gc_vector<tSSAVariable*>::iterator i =Used.begin();
			i != Used.end(); i++)
			tmp_var->SetInterferenceWithAll(*i);
		tmp_var->SetInterferenceWithAll(orig_decl_var);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::AssignRegisters(gc_vector<void*> & assign_work)
{
	// Declared および Used をみて、それぞれの変数にレジスタが割り当たってないようならば
	// レジスタを割り当てる
	if(Declared) Declared->AssignRegister(assign_work);
	for(gc_vector<tSSAVariable*>::const_iterator i = Used.begin();
		i != Used.end(); i++)
		(*i)->AssignRegister(assign_work);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tSSAStatement::GenerateCode(tCodeGenerator * gen) const
{
	// gen にソースコード上の位置を伝える
	gen->SetSourceCodePosition(Position);

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
		RISSE_ASSERT(Value != NULL);
		gen->PutAssign(Declared, *Value);
		break;

	case ocAssignNewBinding:
	case ocAssignThis:
	case ocAssignThisProxy:
	case ocAssignSuper:
	case ocAssignGlobal:
	case ocAssignNewArray:
	case ocAssignNewDict:
		RISSE_ASSERT(Declared != NULL);
		gen->PutAssign(Declared, Code);
		break;

	case ocAssignNewRegExp:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutAssignNewRegExp(Declared, Used[0], Used[1]);
		break;

	case ocAssignNewFunction:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1);
		gen->PutAssignNewFunction(Declared, Used[0]);
		break;

	case ocAssignNewProperty:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutAssignNewProperty(Declared, Used[0], Used[1]);
		break;

	case ocAssignNewClass:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutAssignNewClass(Declared, Used[0], Used[1]);
		break;

	case ocAssignNewModule:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1);
		gen->PutAssignNewModule(Declared, Used[0]);
		break;

	case ocAssignParam:
		RISSE_ASSERT(Declared != NULL);
		gen->PutAssignParam(Declared, Index);
		break;

	case ocAssignBlockParam:
		RISSE_ASSERT(Declared != NULL);
		gen->PutAssignBlockParam(Declared, Index);
		break;

	case ocAddBindingMap:
		RISSE_ASSERT(Used.size() == 2);
		gen->PutAddBindingMap(Used[0], Used[1], *Name);
		break;

	case ocWrite: // 共有変数領域への書き込み
		RISSE_ASSERT(Name != NULL);
		RISSE_ASSERT(Used.size() == 1);
		gen->PutWrite(*Name, Used[0]);
		break;

	case ocRead: // 共有変数領域からの読み込み
		RISSE_ASSERT(Name != NULL);
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 0);
		gen->PutRead(Declared, *Name);
		break;

	case ocNew:
	case ocFuncCall:
	case ocTryFuncCall:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() >= 1);

		{
			gc_vector<const tSSAVariable *> args, blocks;

			risse_size arg_count = Used.size() - 1 - BlockCount;
			risse_size block_count = BlockCount;

			for(risse_size i = 0; i < arg_count; i++)
				args.push_back(Used[i + 1]);
			for(risse_size i = 0; i < block_count; i++)
				blocks.push_back(Used[i + arg_count + 1]);

			gen->PutFunctionCall(Declared, Used[0], Code, 
								FuncExpandFlags, args, blocks);
		}
		break;

	case ocSync:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutSync(Declared, Used[0], Used[1]);
		break;

	case ocJump:
		gen->PutJump(GetJumpTarget());
		break;

	case ocBranch:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutBranch(Used[0], GetTrueBranch(), GetFalseBranch());
		break;

	case ocCatchBranch:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutCatchBranch(Used[0], TryIdentifierIndex, Targets);
		break;

	case ocReturn:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutReturn(Used[0]);
		break;

	case ocDebugger:
		gen->PutDebugger();
		break;

	case ocThrow:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutThrow(Used[0]);
		break;

	case ocExitTryException:
		RISSE_ASSERT(Used.size() <= 1);
		gen->PutExitTryException(Used.size() >= 1 ? Used[0]: NULL, TryIdentifierIndex, Index);
		break;

	case ocGetExitTryValue:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1);
		gen->PutGetExitTryValue(Declared, Used[0]);
		break;

	case ocLogNot:
	case ocBitNot:
	case ocPlus:
	case ocMinus:
	case ocString:
	case ocBoolean:
	case ocReal:
	case ocInteger:
	case ocOctet:
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
	case ocInstanceOf:
	case ocRBitShift:
	case ocLShift:
	case ocRShift:
	case ocMod:
	case ocDiv:
	case ocIdiv:
	case ocMul:
	case ocAdd:
	case ocSub:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutOperator(Code, Declared, Used[0], Used[1]);
		break;

	case ocInContextOf:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 1 || Used.size() == 2);
		if(Used.size() == 2)
			gen->PutInContextOf(Declared, Used[0], Used[1]); // 普通の incontextof
		else
			gen->PutInContextOf(Declared, Used[0], NULL); // incontextof dynamic
		break;

	case ocDGet:
	case ocIGet:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutGet(Code, Declared, Used[0], Used[1], OperateFlagsValue);
		break;

	case ocDDelete:
	case ocIDelete:
		RISSE_ASSERT(Declared != NULL);
		RISSE_ASSERT(Used.size() == 2);
		gen->PutOperator(Code, Declared, Used[0], Used[1]);
		break;

	case ocDSet:
	case ocISet:
		RISSE_ASSERT(Used.size() == 3);
		gen->PutSet(Code, Used[0], Used[1], Used[2], OperateFlagsValue);
		break;

	case ocDSetAttrib:
		RISSE_ASSERT(Used.size() == 2);
		gen->PutSetAttribute(Used[0], Used[1], OperateFlagsValue);
		break;

	case ocAssert:
		RISSE_ASSERT(Used.size() == 1);
		gen->PutAssert(Used[0], GetMessage());
		break;

	case ocDefineAccessMap:
		{
			RISSE_ASSERT(Declared != NULL);
		}
		break;

	case ocDefineLazyBlock:
		{
			tSSAForm * child_form = DefinedForm;
			RISSE_ASSERT(child_form != NULL);
			RISSE_ASSERT(Declared != NULL);
			// この文のDeclaredは、子SSA形式を作成して返すようになっているが、
			// コードブロックの参照の問題があるのでいったんリロケーション用の機構を通す
			gen->PutCodeBlockRelocatee(Declared, child_form->GetCodeBlockIndex());
			if(child_form->GetUseParentFrame())
				gen->PutSetFrame(Declared);
			else
				gen->PutSetShare(Declared);
		}
		break;

	case ocDefineClass:
		{
			tSSAForm * class_form = DefinedForm;
			RISSE_ASSERT(class_form != NULL);
			RISSE_ASSERT(Declared != NULL);
			// この文のDeclaredは、子SSA形式を作成して返すようになっているが、
			// コードブロックの参照の問題があるのでいったんリロケーション用の機構を通す
			gen->PutCodeBlockRelocatee(Declared, class_form->GetCodeBlockIndex());
		}
		break;

	case ocChildWrite:
		{
			RISSE_ASSERT(Used.size() == 2);
			RISSE_ASSERT(Declared != NULL);
			RISSE_ASSERT(Name != NULL);
			// Used[0] が define された文は ocDefineAccessMap であるはずである
			// TODO: この部分は変数の併合を実装するに当たり書き換わる可能性が高い。
			//       現状の実装は暫定的なもの。
			tSSAStatement * lazy_stmt = Used[0]->GetDeclared();
			RISSE_ASSERT(lazy_stmt->GetCode() == ocDefineAccessMap);
wxFprintf(stderr, wxT("registering %s\n"), Name->AsWxString().c_str());
			gen->RegisterVariableMapForChildren(Declared, *Name);
			gen->PutAssign(gen->FindVariableMapForChildren(*Name), Used[1]);
		}
		break;

	case ocChildRead:
		{
			RISSE_ASSERT(Used.size() == 1);
			RISSE_ASSERT(Name != NULL);
			// Used[0] が define された文は ocDefineAccessMap であるはずである
			// TODO: この部分は変数の併合を実装するに当たり書き換わる可能性が高い。
			//       現状の実装は暫定的なもの。
			tSSAStatement * lazy_stmt = Used[0]->GetDeclared();
			RISSE_ASSERT(lazy_stmt->GetCode() == ocDefineAccessMap);
			gen->PutAssign(Declared, gen->FindVariableMapForChildren(*Name));
		}
		break;

	case ocEndAccessMap:
		// アクセスマップの使用終了
		// 暫定実装
		break;


	case ocParentWrite:
		// 暫定実装
		RISSE_ASSERT(Used.size() == 1);
		RISSE_ASSERT(Name != NULL);
		gen->PutAssign(gen->GetParent()->FindVariableMapForChildren(*Name), Used[0]);
		break;

	case ocParentRead:
		// 暫定実装
		RISSE_ASSERT(Used.size() == 0);
		RISSE_ASSERT(Name != NULL);
		RISSE_ASSERT(Declared != NULL);
		gen->PutAssign(Declared, gen->GetParent()->FindVariableMapForChildren(*Name));
		break;

	default:
		RISSE_ASSERT(!"not acceptable SSA operation code");
		break;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tString tSSAStatement::Dump() const
{
	switch(Code)
	{
	case ocNoOperation:			// なにもしない
		return RISSE_WS("nop");

	case ocPhi:		// φ関数
		{
			tString ret;

			RISSE_ASSERT(Declared != NULL);
			ret += Declared->Dump() + RISSE_WS(" = PHI(");

			// φ関数の引数を追加
			tString used;
			for(gc_vector<tSSAVariable*>::const_iterator i = Used.begin();
					i != Used.end(); i++)
			{
				if(!used.IsEmpty()) used += RISSE_WS(", ");
				used += (*i)->Dump();
			}

			ret += used + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocAssignNewRegExp: // 新しい正規表現オブジェクト
		{
			tString ret;
			RISSE_ASSERT(Used.size() == 2);
			ret += Declared->Dump() + RISSE_WS(" = AssignNewRegExp(");

			ret +=	Used[0]->Dump() + RISSE_WS(", ") +
					Used[1]->Dump() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocAssignNewFunction: // 新しい関数インスタンス
		{
			tString ret;
			RISSE_ASSERT(Used.size() == 1);
			ret += Declared->Dump() + RISSE_WS(" = AssignNewFunction(");

			ret +=	Used[0]->Dump() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocAssignNewProperty: // 新しいプロパティインスタンス
		{
			tString ret;
			RISSE_ASSERT(Used.size() == 2);
			ret += Declared->Dump() + RISSE_WS(" = AssignNewProperty(");

			ret +=	Used[0]->Dump() + RISSE_WS(", ") +
					Used[1]->Dump() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocAssignNewClass: // 新しいクラスインスタンス
		{
			tString ret;
			RISSE_ASSERT(Used.size() == 2);
			ret += Declared->Dump() + RISSE_WS(" = AssignNewClass(");

			ret +=	Used[0]->Dump() + RISSE_WS(", ") +
					Used[1]->Dump() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocAssignNewModule: // 新しいモジュールインスタンス
		{
			tString ret;
			RISSE_ASSERT(Used.size() == 1);
			ret += Declared->Dump() + RISSE_WS(" = AssignNewModule(");

			ret +=	Used[0]->Dump() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocAssignParam:
		{
			tString ret;
			RISSE_ASSERT(Used.size() == 0);
			ret += Declared->Dump() + RISSE_WS(" = AssignParam(") +
				tString::AsString((risse_int)Index) + RISSE_WS(")");
			return ret;
		}

	case ocJump:
		{
			RISSE_ASSERT(GetJumpTarget() != NULL);
			return RISSE_WS("goto *") + GetJumpTarget()->GetName();
		}

	case ocBranch:
		{
			RISSE_ASSERT(GetTrueBranch() != NULL);
			RISSE_ASSERT(GetFalseBranch() != NULL);
			RISSE_ASSERT(Used.size() == 1);
			return
					RISSE_WS("if ") + (*Used.begin())->Dump() + 
					RISSE_WS(" then *") + GetTrueBranch()->GetName() +
					RISSE_WS(" else *") + GetFalseBranch()->GetName();
		}

	case ocCatchBranch:
		{
			RISSE_ASSERT(GetTryExitTarget() != NULL);
			RISSE_ASSERT(GetTryCatchTarget() != NULL);
			RISSE_ASSERT(Used.size() == 1);
			tString ret =
					RISSE_WS("catch branch (try id=") +
					tString::AsString((risse_int64)(TryIdentifierIndex)) +
					RISSE_WS(") ") +
					(*Used.begin())->Dump() + 
					RISSE_WS(" exit:*") + GetTryExitTarget()->GetName() +
					RISSE_WS(" catch:*") + GetTryCatchTarget()->GetName();
			for(risse_size n = 2; n < Targets.size(); n++)
			{
				ret += RISSE_WS(" ") + tString::AsString((risse_int64)(n)) +
					RISSE_WS(": *") + Targets[n]->GetName();
			}
			return ret;
		}

	case ocExitTryException:
		{
			RISSE_ASSERT(Used.size() <= 1);
			tString ret;
			ret = RISSE_WS("ExitTryException(") +
				((Used.size() >= 1)?(Used[0]->Dump()):tString(RISSE_WS("<none>"))) +
				RISSE_WS(", try_id=")+
				tString::AsString((risse_int64)(TryIdentifierIndex)) +
				RISSE_WS(", index=")+
				tString::AsString((risse_int64)(Index)) +
				RISSE_WS(")");
			return ret;
		}

	case ocDefineLazyBlock: // 遅延評価ブロックの定義
	case ocDefineClass: // クラスの定義
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Declared != NULL);

			tString ret;
			ret += Declared->Dump() +
				(Code == ocDefineLazyBlock ?
					RISSE_WS(" = DefineLazyBlock(") :
					RISSE_WS(" = DefineClass(") );

			ret +=	Name->AsHumanReadable() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
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

			tString ret;
			ret += Declared->Dump() + RISSE_WS(" = ParentRead(");

			ret +=	Name->AsHumanReadable() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocChildWrite: // 子名前空間への書き込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Used.size() == 2);
			RISSE_ASSERT(Declared != NULL);
			tString ret;
			if(Declared)
				ret += Declared->Dump() + RISSE_WS(" = "); // この文で宣言された変数がある
			ret += Used[0]->Dump()  + RISSE_WS(".ChildWrite(") +
					Name->AsHumanReadable() + RISSE_WS(", ") + Used[1]->Dump() +
					RISSE_WS(")");
			return ret + Declared->GetComment();
		}

	case ocChildRead: // 子名前空間からの読み込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Declared != NULL);
			RISSE_ASSERT(Used.size() == 1);

			tString ret;
			ret += Declared->Dump() + RISSE_WS(" = ") + Used[0]->Dump();
			ret += RISSE_WS(".ChildRead(");
			ret +=	Name->AsHumanReadable() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocWrite: // 共有変数領域への書き込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Used.size() == 1);
			return Used[0]->Dump() + RISSE_WS(".Write(") +
					Name->AsHumanReadable() +
					RISSE_WS(")");
		}

	case ocRead: // 共有変数領域からの読み込み
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Declared != NULL);
			RISSE_ASSERT(Used.size() == 0);

			tString ret;
			ret += Declared->Dump() + RISSE_WS(" = ");
			ret += RISSE_WS("Read(");
			ret +=	Name->AsHumanReadable() + RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocSync: // synchronized
		{
			RISSE_ASSERT(Used.size() == 2);
			RISSE_ASSERT(Declared != NULL);

			tString ret;
			ret += Declared->Dump() + RISSE_WS(" = ");
			ret += Used[0]->Dump() + RISSE_WS("()");
			ret += RISSE_WS(" Synchronized(") + Used[1]->Dump() + 
				RISSE_WS(")");

			// 定義された変数のコメントを追加して返す
			return ret + Declared->GetComment();
		}

	case ocAddBindingMap: // ローカル変数のバインディング情報を追加
		{
			RISSE_ASSERT(Name != NULL);
			RISSE_ASSERT(Used.size() == 2);

			return Used[0]->Dump() + RISSE_WS(".AddBindingMap(") +
					Used[1]->Dump() + RISSE_WS(", ") +
					Name->AsHumanReadable() +
					RISSE_WS(")");
		}

	default:
		{
			tString ret;

			// 変数の宣言
			if(Declared)
				ret += Declared->Dump() + RISSE_WS(" = "); // この文で宣言された変数がある

			if(Code == ocAssign || Code == ocReadVar || Code == ocWriteVar)
			{
				// 単純代入/共有変数の読み書き
				RISSE_ASSERT(Used.size() == 1);
				ret += (*Used.begin())->Dump();

				if(Code == ocReadVar)
					ret += RISSE_WS(" (read from ") + Name->AsHumanReadable()
						+ RISSE_WS(")");
				else if(Code == ocWriteVar)
					ret += RISSE_WS(" (write to ") + Name->AsHumanReadable()
						+ RISSE_WS(")");
			}
			else
			{
				// 関数呼び出しの類？
				bool is_funccall = (Code == ocFuncCall || Code == ocNew || Code == ocTryFuncCall);

				// 使用している引数の最初の引数はメッセージの送り先なので
				// オペレーションコードよりも前に置く
				if(Used.size() != 0)
					ret += (*Used.begin())->Dump() + RISSE_WC('.');

				// オペレーションコード
				ret += tString(VMInsnInfo[Code].Name);

				// 使用している引数
				if(is_funccall && (FuncExpandFlags & FuncCallFlag_Omitted))
				{
					// 関数呼び出しかnew
					ret += RISSE_WS("(...)");
				}
				else if(Used.size() >= 2)
				{
					// 引数がある
					tString used;
					risse_int arg_index = 0;
					for(gc_vector<tSSAVariable*>::const_iterator i = Used.begin() + 1;
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

			// Value があればそれを追加
			if(Value && Code == ocAssignConstant)
			{
				ret += RISSE_WS(" Value=");
				ret += Value->AsHumanReadable();
			}

			// 変数の宣言に関してコメントがあればそれを追加
			if(Declared)
				ret += Declared->GetComment();

			// DSet あるいは DGet についてフラグがあればそれを追加
			if(Code == ocDGet || Code == ocDSet)
			{
				ret +=
					RISSE_WS(" Flags=(") +
					tOperateFlags(OperateFlagsValue).AsString() +
					RISSE_WS(")");
			}

			// DSetAttrib についてフラグがあればそれを追加
			if(Code == ocDSetAttrib)
			{
				ret +=
					RISSE_WS(" Flags=(") +
					tOperateFlags(OperateFlagsValue).AsString() +
					RISSE_WS(")");
			}

			return ret;
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
} // namespace Risse
