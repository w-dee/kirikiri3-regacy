//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief OpenAL ソース管理
//---------------------------------------------------------------------------
#include "prec.h"
#include "base/utils/Singleton.h"
#include "sound/ALSource.h"
#include "base/utils/Singleton.h"
#include "base/utils/RisaThread.h"
#include "base/event/TickCount.h"
#include "base/event/IdleEvent.h"
#include "base/utils/PointerList.h"
#include "sound/Sound.h"
#include "base/log/Log.h"

namespace Risa {
RISSE_DEFINE_SOURCE_ID(51552,26074,48813,19041,30653,39645,11297,33602);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		デコード用スレッド
//! @note		サウンドソースが再生中の間は、tWaveDecodeThreadPool -> tWaveDecodeThread -> Source
//!				の参照があるため、Source が GC により回収されることはない。
//---------------------------------------------------------------------------
class tWaveDecodeThread : public tThread, protected depends_on<tTickCount>
{
	tALSource * Source; //!< このスレッドに関連づけられているソース
	tThreadEvent Event; //!< スレッドをたたき起こすため/スレッドを眠らせるためのイベント
	tCriticalSection CS; //!< このオブジェクトを保護するクリティカルセクション

public:
	tWaveDecodeThread();
	~tWaveDecodeThread();

	void SetSource(tALSource * source);
	tALSource * GetSource() { volatile tCriticalSection::tLocker cs_holder(CS);  return Source; }

protected:
	void Execute(void);
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		コンストラクタ
//---------------------------------------------------------------------------
tWaveDecodeThread::tWaveDecodeThread()
{
	Source = NULL;
	Run(); // スレッドの実行を開始
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		デストラクタ
//---------------------------------------------------------------------------
tWaveDecodeThread::~tWaveDecodeThread()
{
	Terminate(); // スレッドの終了を伝える
	Event.Signal(); // スレッドをたたき起こす
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		ソースを設定する
//! @param		source ソース
//---------------------------------------------------------------------------
void tWaveDecodeThread::SetSource(tALSource * source)
{
	volatile tCriticalSection::tLocker cs_holder(CS);
	Source = source;
	Event.Signal(); // スレッドをたたき起こす
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		スレッドのエントリーポイント
//---------------------------------------------------------------------------
void tWaveDecodeThread::Execute(void)
{
	while(!ShouldTerminate())
	{
		int sleep_ms  = 0;
		tALSource * source;

		{ // entering critical section
			volatile tCriticalSection::tLocker cs_holder(CS);
			source = Source;
		}

		{
			if(!source)
			{
				// まだソースが割り当てられていない
				sleep_ms = 60*1000; // 適当な時間待つ
			}
			else
			{
				// ソースが割り当てられている
				risse_uint64 start_tick = tTickCount::instance()->Get();

				source->FillBuffer();
					// デコードを行う(このメソッドは、本来呼び出されるべきで
					// ない場合に呼び出されても単に無視するはず)

				// 眠るべき時間を計算する
				// ここでは大まかに FillBuffer が tALBuffer::STREAMING_CHECK_SLEEP_MS
				// 周期で実行されるように調整する。
				// 楽観的なアルゴリズムなので Sleep 精度は問題にはならない。
				risse_uint64 end_tick = tTickCount::instance()->Get();
				sleep_ms = 
					tALBuffer::STREAMING_CHECK_SLEEP_MS -
						static_cast<int>(end_tick - start_tick);
				if(sleep_ms < 0) sleep_ms = 1; // 0 を指定すると無限に待ってしまうので
				if(static_cast<unsigned int>(sleep_ms ) > tALBuffer::STREAMING_CHECK_SLEEP_MS)
					sleep_ms = tALBuffer::STREAMING_CHECK_SLEEP_MS;
			}
		} // end of critical section

		Event.Wait(sleep_ms); // 眠る
	}
}
//---------------------------------------------------------------------------


























//---------------------------------------------------------------------------
//! @brief		デコード用スレッドのプール
//---------------------------------------------------------------------------
class tWaveDecodeThreadPool :
								public singleton_base<tWaveDecodeThreadPool>,
								manual_start<tWaveDecodeThreadPool>,
								protected tCompactEventDestination
{
	tCriticalSection CS;

	gc_vector<tWaveDecodeThread *> FreeThreads; //!< 使用していないスレッドのリスト
	pointer_list<tWaveDecodeThread> UsingThreads; //!< 貸し出し中のスレッドのリスト

public:
	tWaveDecodeThreadPool();
	~tWaveDecodeThreadPool();

protected:
	void OnCompact(tCompactLevel level);

public:
	tWaveDecodeThread * Acquire(tALSource * source);
	void Unacquire(tWaveDecodeThread * buffer);
	bool CallWatchCallbacks();
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		コンストラクタ
//---------------------------------------------------------------------------
tWaveDecodeThreadPool::tWaveDecodeThreadPool()
{
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		デストラクタ
//---------------------------------------------------------------------------
tWaveDecodeThreadPool::~tWaveDecodeThreadPool()
{
	volatile tCriticalSection::tLocker cs_holder(CS);

	// FreeThreads を解放する
	for(gc_vector<tWaveDecodeThread*>::iterator i = FreeThreads.begin();
		i != FreeThreads.end(); i++)
		delete (*i);

	// すべての UsingThreads の Source を stop する

	volatile pointer_list<tWaveDecodeThread>::scoped_lock lock(UsingThreads);
	size_t count = UsingThreads.get_locked_count();
	for(size_t i = 0; i < count; i++)
	{
		tWaveDecodeThread * th = UsingThreads.get_locked(i);
		if(th) th->GetSource()->Stop(false);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		コンパクトイベント
//---------------------------------------------------------------------------
void tWaveDecodeThreadPool::OnCompact(tCompactLevel level)
{
	volatile tCriticalSection::tLocker cs_holder(CS);

	if(level >= clSlowBeat)
	{
		// FreeThreads を解放する
		for(gc_vector<tWaveDecodeThread*>::iterator i = FreeThreads.begin();
			i != FreeThreads.end(); i++)
			delete (*i);
		FreeThreads.clear();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		スレッドを一つ得る
//! @return		スレッド
//---------------------------------------------------------------------------
tWaveDecodeThread * tWaveDecodeThreadPool::Acquire(tALSource * source)
{
	volatile tCriticalSection::tLocker cs_holder(CS);

	// Free が空か？
	tWaveDecodeThread * newthread;
	if(FreeThreads.size() == 0)
	{
		// 新しくスレッドを作成して返す
		newthread = new tWaveDecodeThread();
	}
	else
	{
		// Free から一つスレッドをとってきて返す
		tWaveDecodeThread * th = FreeThreads.back();
		FreeThreads.pop_back();
		newthread = th;
	}

	// UsingThreads に追加
	UsingThreads.add(newthread);

	// tWaveWatchThread をたたき起こす
	tWaveWatchThread::instance()->Wakeup();

	// newthread を帰す
	newthread->SetSource(source);
	return newthread;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		スレッドを一つ返す
//! @param		thread スレッド
//---------------------------------------------------------------------------
void tWaveDecodeThreadPool::Unacquire(tWaveDecodeThread * thread)
{
	volatile tCriticalSection::tLocker cs_holder(CS);

	thread->SetSource(NULL); // source を null に

	// UsingThreads からスレッドを削除
	UsingThreads.remove(thread);

	// FreeThreads にスレッドを入れる
	FreeThreads.push_back(thread);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief		使用中の各スレッドの WatchCallback を呼び出す
//! @param		一つでも呼び出す物があった場合に true
//---------------------------------------------------------------------------
bool tWaveDecodeThreadPool::CallWatchCallbacks()
{
	// ここで使用している pointer_list は自分でスレッド保護を行うので
	// 明示的なロックはここでは必要ない。

	volatile pointer_list<tWaveDecodeThread>::scoped_lock lock(UsingThreads);
	size_t count = UsingThreads.get_locked_count();
	for(size_t i = 0; i < count; i++)
	{
		tWaveDecodeThread * th = UsingThreads.get_locked(i);
		if(th)
		{
			tALSource * source = th->GetSource();
			if(source) source->WatchCallback();
			// この中で Unacquire が呼ばれる可能性があるので注意
			// (pointer_list はそういう状況でもうまく扱うことができるので問題ない)
		}
	}
	return count != 0;
}
//---------------------------------------------------------------------------











/*
	tWaveWatchThread の定義はこのヘッダファイルにある
*/


//---------------------------------------------------------------------------
tWaveWatchThread::tWaveWatchThread()
{
	tWaveDecodeThreadPool::ensure(); // tWaveDecodeThreadPool を作っておく
	Run(); // スレッドの実行を開始
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tWaveWatchThread::~tWaveWatchThread()
{
	Terminate(); // スレッドの終了を伝える
	Event.Signal(); // スレッドをたたき起こす
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveWatchThread::Wakeup()
{
	Event.Signal(); // スレッドをたたき起こす
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveWatchThread::Execute(void)
{
	while(!ShouldTerminate())
	{
		long sleep_time;

		if(tWaveDecodeThreadPool::instance()->CallWatchCallbacks())
			sleep_time = 50; // 50ms 固定
		else
			sleep_time = 60 * 1000; // ソースが登録されていないので長めに眠る

		Event.Wait(sleep_time); // 適当な時間待つ
	}
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------
tALSource::tInternalSource::tInternalSource()
{
	volatile tOpenAL::tCriticalSectionHolder cs_holder;

	// ソースの生成
	alGenSources(1, &Source);
	tOpenAL::instance()->ThrowIfError(RISSE_WS("alGenSources"));

	alSourcei(Source, AL_LOOPING, AL_FALSE);
	tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourcei(AL_LOOPING)"));
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tALSource::tInternalSource::~tInternalSource()
{
	volatile tOpenAL::tCriticalSectionHolder al_cs_holder;
	alDeleteSources(1, &Source);
	tOpenAL::instance()->ThrowIfError(RISSE_WS("alDeleteSources"));
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
tALSource::tALSource(tALBuffer * buffer,
	tWaveLoopManager * loopmanager) :
	CS(new tCriticalSection()), Buffer(buffer), LoopManager(loopmanager)
{
	Init(buffer);
	if(!Buffer->GetStreaming())
	{
		// バッファが非ストリーミングの場合は LoopManager は必要ないので解放する
		LoopManager = NULL; // TODO: real .Dispose();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tALSource::tALSource(const tALSource * ref) :
	CS(new tCriticalSection())
{
	// 他のソースと Buffer を共有したい場合に使う。
	// Buffer は 非Streaming バッファでなければならない。
	if(ref->Buffer->GetStreaming())
		tSoundExceptionClass::Throw(RISSE_WS_TR("Shared buffer must be a non-streaming buffer"));

	Init(ref->Buffer);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::Init(tALBuffer * buffer)
{
	// フィールドの初期化
	DecodeThread = NULL;
	Status = PrevStatus = ssStop;
	NeedRewind = false;
	Buffer = buffer; // 一応再設定

	// スレッドプールを作成
	tWaveDecodeThreadPool::ensure();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::EnsureSource()
{
	if(Source) return; // ソースがすでに存在する場合はなにもしない

	// ソースの生成
	int retry_count = 3;
	while(retry_count--)
	{
		try
		{
			Source = new tInternalSource();
		}
		catch(...)
		{
			if(retry_count == 0) throw; // リトライ失敗
			CollectGarbage(); // GC
		}
	}

	// ストリーミングを行わない場合は、バッファをソースにアタッチ
	if(!Buffer->GetStreaming())
	{
		volatile tOpenAL::tCriticalSectionHolder cs_holder;
		alSourcei(Source->Source, AL_BUFFER, Buffer->GetBuffer());
		tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourcei(AL_BUFFER)"));
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::DeleteSource()
{
	if(Source) delete Source, Source = NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::FillBuffer()
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RecheckStatus();

	if(!DecodeThread) return ; // デコードスレッドが使用不可なので呼び出さない
	if(!Source) return ; // デコードスレッドが使用不可なので呼び出さない

	if(Buffer->GetStreaming())
	{
		QueueBuffer();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::UnqueueAllBuffers()
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RISSE_ASSERT(Buffer->GetStreaming());

	// ソースの削除と再生成を行う
	DeleteSource();

/*
	下記のようにバッファの内容をいったんすべて alSourceUnqueueBuffers を使って
	アンキューする方法は、なぜか alGetSourcei(AL_BUFFERS_QUEUED) が 1 を返し、
	alSourceUnqueueBuffers でそのバッファをアンキューしようとしてもエラーに
	なる事がある。なので、上記のようにいったんソースを削除し、作成し直す
	事にする。
*/

/*
	// ソースにキューされているバッファの数を得る
	ALint queued;
	alGetSourcei(Source->Source, AL_BUFFERS_QUEUED, &queued);
	tOpenAL::instance()->ThrowIfError(
		RISSE_WS("alGetSourcei(AL_BUFFERS_QUEUED) at tALSource::UnqueueAllBuffers"));

	if(queued > 0)
	{
		ALuint dummy_buffers[STREAMING_NUM_BUFFERS];

		// アンキューする
		alSourceUnqueueBuffers(Source->Source, queued, dummy_buffers);
		tOpenAL::instance()->ThrowIfError(
			RISSE_WS("alSourceUnqueueBuffers at tALSource::UnqueueAllBuffers"));
	}
*/
	// バッファもすべてアンキュー
	Buffer->FreeAllBuffers();

	// セグメントキューもクリア
	SegmentQueues.clear();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::QueueBuffer()
{
	// アンキューできるバッファがあるかを調べる
	if(!Source) return; // ソースが無い場合はなにもできない

	{
		ALint processed;

		{
			volatile tOpenAL::tCriticalSectionHolder cs_holder;
			alGetSourcei(Source->Source, AL_BUFFERS_PROCESSED, &processed);
			tOpenAL::instance()->ThrowIfError(
				RISSE_WS("alGetSourcei(AL_BUFFERS_PROCESSED)"));
		}

		if(processed >= 1)
		{
			// アンキューできるバッファがある
			// 一つアンキューする
			ALuint  buffer = 0;

			{
				volatile tOpenAL::tCriticalSectionHolder cs_holder;
				alSourceUnqueueBuffers(Source->Source, 1, &buffer);
				tOpenAL::instance()->ThrowIfError(
					RISSE_WS("alSourceUnqueueBuffers at tALSource::QueueStream"));
			}

			Buffer->PushFreeBuffer(buffer); // アンキューしたバッファを返す

			{
				if(SegmentQueues.size() > 0)
					SegmentQueues.pop_front(); // セグメントキューからも削除
				else
					fprintf(stderr, "!!! none to popup from SegmentQueues\n"); fflush(stderr);
			}
		}
	}

	// バッファに空きバッファがあるか
	if(Buffer->HasFreeBuffer())
	{
		// データが流し込まれたバッファを得る
		// TODO: segments と events のハンドリング
		tWaveSegmentQueue segmentqueue;
		ALuint  buffer = 0;
		bool filled = Buffer->PopFilledBuffer(buffer, segmentqueue);

		// バッファにデータを割り当て、キューする
		if(filled)
		{
			volatile tOpenAL::tCriticalSectionHolder cs_holder;
			alSourceQueueBuffers(Source->Source, 1, &buffer);
			tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourceQueueBuffers"));
		}

		// セグメントキューに追加
		if(filled)
		{
			SegmentQueues.push_back(segmentqueue);
//			fprintf(stderr, "queue : ");
//			segmentqueue.Dump();
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::RecheckStatus()
{
	if(Source && Status == ssPlay)
	{
		ALint state;

		{
			volatile tOpenAL::tCriticalSectionHolder al_cs_holder;
			alGetSourcei(Source->Source, AL_SOURCE_STATE, &state );
			tOpenAL::instance()->ThrowIfError(RISSE_WS("alGetSourcei(AL_SOURCE_STATE)"));
		}

		if(state != AL_PLAYING)
		{
			// Playing が真を表しているにもかかわらず、OpenAL のソースは再生を
			// 停止している
			InternalStop(2); // 再生を停止, イベントは非同期イベントとして通知
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::WatchCallback()
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RecheckStatus();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::CallStatusChanged(bool async)
{
	if(PrevStatus != Status)
	{
		if(async)
			OnStatusChangedAsync(Status);
		else
			OnStatusChanged(Status);
		PrevStatus = Status;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::Play()
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RecheckStatus();

	if(Status == ssPlay) return; // すでに再生している場合は再生をしない

	EnsureSource();

	if(Buffer->GetStreaming())
	{
		// もし仮にデコードスレッドがあったとしても
		// ここで解放する
		if(DecodeThread)
			tWaveDecodeThreadPool::instance()->Unacquire(DecodeThread), DecodeThread = NULL;


		// 巻き戻しを行う
		if(NeedRewind)
		{
			// すべての状況で巻き戻しが必要なわけではないので
			// NeedRewind が真の時にしかここでは巻き戻しを行わない
			NeedRewind = false;
			LoopManager->SetPosition(0); // 再生位置を最初に
			Buffer->GetFilter()->Reset(); // フィルタをリセット
		}

		// 初期サンプルをいくつか queue する
		for(risse_uint n = 0; n < STREAMING_PREPARE_BUFFERS; n++)
			QueueBuffer();

		// デコードスレッドを作成する
		if(!DecodeThread) DecodeThread = tWaveDecodeThreadPool::instance()->Acquire(this);
		// デコードスレッドを作成するとその時点から
		// FillBuffer が呼ばれるようになる
	}

	// 再生を開始する
	{
		volatile tOpenAL::tCriticalSectionHolder al_cs_holder;

		alSourcePlay(Source->Source);
		tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourcePlay"));
	}

	// ステータスの変更を通知
	Status = ssPlay;
	CallStatusChanged(false);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::InternalStop(int notify)
{
	// デコードスレッドを削除する
	if(Buffer->GetStreaming())
	{
		if(DecodeThread)
			tWaveDecodeThreadPool::instance()->Unacquire(DecodeThread), DecodeThread = NULL;
		// この時点で
		// FillBuffer は呼ばれなくなる
	}

	// 再生を停止する
	if(Source)
	{
		volatile tOpenAL::tCriticalSectionHolder al_cs_holder;

		alSourceStop(Source->Source);
		tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourceStop"));
	}

	// ステータスの変更を通知
	Status = ssStop;
	if(notify != 0)
		CallStatusChanged(notify == 2);

	// 全てのバッファを unqueueする
	if(Source && Buffer->GetStreaming())
	{
		UnqueueAllBuffers();
	}

	// 次回再生開始前に巻き戻しが必要
	NeedRewind = true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::Stop(int notify)
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RecheckStatus();

	InternalStop(notify);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::Pause()
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RecheckStatus();

	EnsureSource();

	// 再生中の場合は
	if(Status == ssPlay)
	{
		// 再生を一時停止する
		{
			volatile tOpenAL::tCriticalSectionHolder al_cs_holder;

			alSourcePause(Source->Source);
			tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourcePause"));
		}

		// ステータスの変更を通知
		Status = ssPause;
		CallStatusChanged(false);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_uint64 tALSource::GetPosition()
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RecheckStatus();

	EnsureSource();

	// 再生中や一時停止中でない場合は 0 を返す
	if(Status != ssPlay && Status != ssPause) return 0;
	if(SegmentQueues.size() == 0) return 0;

	unsigned int unit = Buffer->GetOneBufferRenderUnit();

/*---- debug ----*/
{
	ALint queued;
	alGetSourcei(Source->Source, AL_BUFFERS_QUEUED, &queued);
	tOpenAL::instance()->ThrowIfError(
		RISSE_WS("alGetSourcei(AL_BUFFERS_QUEUED) at tALSource::UnqueueAllBuffers"));
	if(queued != (ALint) SegmentQueues.size())
		fprintf(stderr, "segment count does not match (al %d risa %d)\n", (int)queued, (int)SegmentQueues.size());
}
/*---- debug ----*/

	unsigned int queue_index;
	unsigned int queue_offset;

	// 再生位置を取得する
	ALint pos = 0;
	{
		volatile tOpenAL::tCriticalSectionHolder al_cs_holder;

		alGetSourcei(Source->Source, AL_SAMPLE_OFFSET, &pos);
		tOpenAL::instance()->ThrowIfError(RISSE_WS("alGetSourcei(AL_SAMPLE_OFFSET)"));

//		tLogger::Log(RISSE_WS("al pos : ") + tString::AsString(pos));
	}

	// ときどき、 alGetSourcei(AL_SAMPLE_OFFSET) はキューされているバッファを
	// 超えるような変なオフセットを返す。
	// どうも折り返させれば OK なようなのでそうする。正常なオフセットを返さないのは
	// OpenAL の実装のバグかとおもわれるが、それを回避する。
	ALint size_max = (unsigned int)SegmentQueues.size() * unit;
	while(pos >= size_max) pos -= size_max;

	// 返された値は キューの先頭からの再生オフセットなので、該当する
	// キューを探す
	queue_index  = pos / unit;
	queue_offset = pos % unit;

//	fprintf(stderr, "get position queue in offset %d at queue index %d : ", queue_offset, queue_index);
//	SegmentQueues[queue_index].Dump();

	// 得られたのはフィルタ後の位置なのでフィルタ前のデコード位置に変換してから返す
	return SegmentQueues[queue_index].FilteredPositionToDecodePosition(queue_offset);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tALSource::SetPosition(risse_uint64 pos)
{
	volatile tCriticalSection::tLocker cs_holder(*CS);

	RecheckStatus();

	EnsureSource();

	if(Buffer->GetStreaming())
	{
		if(Status == ssPlay || Status == ssPause)
		{
			// デコードスレッドをいったん削除する
			if(DecodeThread)
				tWaveDecodeThreadPool::instance()->Unacquire(DecodeThread), DecodeThread = NULL;
			// この時点で
			// FillBuffer は呼ばれなくなる

			// 再生を停止する
			{
				volatile tOpenAL::tCriticalSectionHolder al_cs_holder;

				alSourceStop(Source->Source);
				tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourceStop"));
			}
		}

		// 全てのバッファを unqueueする
		UnqueueAllBuffers();

		// 再生位置の変更を行う
		NeedRewind = false;
		LoopManager->SetPosition(pos); // 再生位置を最初に
		Buffer->GetFilter()->Reset(); // フィルタをリセット

		if(Status == ssPlay || Status == ssPause)
		{
			// 初期サンプルをいくつか queue する
			for(risse_uint n = 0; n < STREAMING_PREPARE_BUFFERS; n++)
				QueueBuffer();

			// デコードスレッドを作成する
			if(!DecodeThread) DecodeThread = tWaveDecodeThreadPool::instance()->Acquire(this);
			// デコードスレッドを作成するとその時点から
			// FillBuffer が呼ばれるようになる
		}

		if(Status == ssPlay)
		{
			// 再生を開始する
			{
				volatile tOpenAL::tCriticalSectionHolder al_cs_holder;

				alSourcePlay(Source->Source);
				tOpenAL::instance()->ThrowIfError(RISSE_WS("alSourcePlay"));
			}
		}
	}
	else
	{
		// TODO: 非ストリーミング時の動作
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
} // namespace Risa

