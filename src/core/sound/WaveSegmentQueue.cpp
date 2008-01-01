//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000-2008 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Waveセグメント/イベントキュー管理
//---------------------------------------------------------------------------
#include "prec.h"
#include "sound/WaveSegmentQueue.h"



namespace Risa {
RISSE_DEFINE_SOURCE_ID(41279,59678,37773,16849,57737,10358,8969,57779);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void tWaveSegmentQueue::Clear()
{
	Events.clear();
	Segments.clear();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Enqueue(const tWaveSegmentQueue & queue)
{
	Enqueue(queue.Events); // events をエンキュー(こっちを先にしないとだめ)
	Enqueue(queue.Segments); // segments をキュー(こっちは後)
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Enqueue(const tWaveSegment & segment)
{
	if(Segments.size() > 0)
	{
		// 既存のセグメントが 1 個以上ある
		tWaveSegment & last = Segments.back();
		// 最後のセグメントとこれから追加しようとするセグメントが連続してるか？
		if(last.Start + last.Length == segment.Start &&
			(double)last.FilteredLength / last.Length ==
			(double)segment.FilteredLength / segment.Length)
		{
			// 連続していて、かつ、比率も完全に同じなので
			// 既存の最後のセグメントを延長する
			// (ちなみにここで比率の比較の際に誤差が生じたとしても
			//  大きな問題とはならない)
			last.FilteredLength += segment.FilteredLength;
			last.Length += segment.Length;
			return ; // おわり
		}
	}

	// 単純に最後に要素を追加
	Segments.push_back(segment);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Enqueue(const tWaveEvent & event)
{
	Events.push_back(event);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Enqueue(const gc_deque<tWaveSegment> & segments)
{
	// segment の追加
	for(gc_deque<tWaveSegment>::const_iterator i = segments.begin();
		i != segments.end(); i++)
		Enqueue(*i);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Enqueue(const gc_deque<tWaveEvent> & events)
{
	// オフセットに加算する値を得る
	risse_int64 event_offset = GetFilteredLength();

	// event の追加
	for(gc_deque<tWaveEvent>::const_iterator i = events.begin();
		i != events.end(); i++)
	{
		tWaveEvent one_event(*i);
		one_event.Offset += event_offset; // offset の修正
		Enqueue(one_event);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Dequeue(tWaveSegmentQueue & dest, risse_int64 length)
{
	risse_int64 remain;
	// dest をクリア
	dest.Clear();

	// Segments を切り出す
	remain = length;
	while(Segments.size() > 0 && remain > 0)
	{
		if(Segments.front().FilteredLength <= remain)
		{
			// Segments.front().FilteredLength が remain 以下
			// → この要素を dest にエンキューして this から削除
			remain -= Segments.front().FilteredLength;
			dest.Enqueue(Segments.front());
			Segments.pop_front();
		}
		else
		{
			// Segments.front().FilteredLength が remain よりも大きい
			// → 要素を途中でぶったぎって dest にエンキュー
			// FilteredLength を元に切り出しを行ってるので
			// Length は 線形補間を行う
			risse_int64 newlength =
				static_cast<risse_int64>(
					(double)Segments.front().Length / (double)Segments.front().FilteredLength * remain);
			if(newlength > 0)
				dest.Enqueue(tWaveSegment(Segments.front().Start, newlength, remain));

			// Segments.front() の Start, Length と FilteredLength を修正
			Segments.front().Start += newlength;
			Segments.front().Length -= newlength;
			Segments.front().FilteredLength -= remain;

			if(Segments.front().Length == 0 || Segments.front().FilteredLength == 0)
			{
				// ぶった切った結果 (線形補完した結果の誤差で)
				// 長さが0になってしまった
				Segments.pop_front(); // セグメントを捨てる
			}
			remain = 0; // ループを抜ける
		}
	}

	// Events を切り出す
	size_t events_to_dequeue = 0;
	for(gc_deque<tWaveEvent>::iterator i = Events.begin();
		i != Events.end(); i++)
	{
		risse_int64 newoffset = i->Offset - length;
		if(newoffset < 0)
		{
			// newoffset が負 なので dest に入れる
			dest.Enqueue(*i);
			events_to_dequeue ++; // あとで dequeue
		}
		else
		{
			// *i のオフセットを修正
			i->Offset = newoffset;
		}
	}

	while(events_to_dequeue--) Events.pop_front(); // コピーしたEvents を削除
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_int64 tWaveSegmentQueue::GetFilteredLength() const
{
	// キューの長さは すべての Segments のFilteredLengthの合計
	risse_int64 length = 0;
	for(gc_deque<tWaveSegment>::const_iterator i = Segments.begin();
		i != Segments.end(); i++)
		length += i->FilteredLength;

	return length;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Scale(risse_int64 new_total_filtered_length)
{
	// キューの FilteredLength を変化させる
	risse_int64 total_length_was = GetFilteredLength(); // 変化前の長さ

	if(total_length_was == 0) return; // 元の長さがないのでスケール出来ない

	// Segments の修正
	risse_int64 offset_was = 0; // 変化前のオフセット
	risse_int64 offset_is = 0; // 変化後のオフセット

	for(gc_deque<tWaveSegment>::iterator i = Segments.begin();
		i != Segments.end(); i++)
	{
		risse_int64 old_end = offset_was + i->FilteredLength;
		offset_was += i->FilteredLength;

		// old_end は全体から見てどの位置にある？
		double ratio = static_cast<double>(old_end) /
						static_cast<double>(total_length_was);

		// 新しい old_end はどの位置にあるべき？
		risse_int64 new_end = static_cast<risse_int64>(ratio * new_total_filtered_length);

		// FilteredLength の修正
		i->FilteredLength = new_end - offset_is;

		offset_is += i->FilteredLength;
	}

	// からっぽのSegments の除去
	for(gc_deque<tWaveSegment>::iterator i = Segments.begin();
		i != Segments.end() ; )
	{
		if(i->FilteredLength == 0 || i->Length == 0)
			i = Segments.erase(i);
		else
			i++;
	}

	// Events の修正
	double ratio = (double)new_total_filtered_length / (double)total_length_was;
	for(gc_deque<tWaveEvent>::iterator i = Events.begin();
		i != Events.end(); i++)
	{
		i->Offset = static_cast<risse_int64>(i->Offset * ratio);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
risse_int64 tWaveSegmentQueue::FilteredPositionToDecodePosition(risse_int64 pos) const
{
	// Segments の修正
	risse_int64 offset_filtered = 0;

	for(gc_deque<tWaveSegment>::const_iterator i = Segments.begin();
		i != Segments.end(); i++)
	{
		if(offset_filtered <= pos && pos < offset_filtered + i->FilteredLength)
		{
			// 対応する区間が見つかったので線形で補完して返す
			return (risse_int64)(i->Start + (pos - offset_filtered) *
				(double)i->Length / (double)i->FilteredLength );
		}

		offset_filtered += i->FilteredLength;
	}

	// 対応する区間が見つからないので、明らかに負であれば 0 を、
	// そうでなければ最後の位置を返す
	fprintf(stderr, "position not found\n");
	if(pos<0) return 0;
	if(Segments.size() == 0) return 0;
	return Segments[Segments.size()-1].Start + Segments[Segments.size()-1].Length;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tWaveSegmentQueue::Dump() const
{
	bool first;

	fprintf(stderr, "Segments [");
	first = true;
	for(gc_deque<tWaveSegment>::const_iterator i = Segments.begin();
		i != Segments.end(); i++)
	{
		if(!first)
			fprintf(stderr, ", ");
		first = false;
		fprintf(stderr, "St:%ld Len:%ld FLen:%ld", (long)i->Start, (long)i->Length, (long)i->FilteredLength);
	}
	fprintf(stderr, "]");

	fprintf(stderr, "  Events [");
	first = true;
	for(gc_deque<tWaveEvent>::const_iterator i = Events.begin();
		i != Events.end(); i++)
	{
		if(!first)
			fprintf(stderr, ", ");
		first = false;
		wxFprintf(stderr, wxT("Name:%s Pos:%ld Ofs:%d"), i->Name.AsWxString().c_str(),
			(long)i->Position, i->Offset);
	}
	fprintf(stderr, "]");
	fprintf(stderr, "\n");
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
} // namespace Risa


