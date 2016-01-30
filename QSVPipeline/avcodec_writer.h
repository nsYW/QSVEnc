﻿//  -----------------------------------------------------------------------------------------
//    QSVEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  ---------------------------------------------------------------------------------------
#ifndef _AVCODEC_WRITER_H_
#define _AVCODEC_WRITER_H_

#include "qsv_queue.h"

#if ENABLE_AVCODEC_QSV_READER
#include <thread>
#include <atomic>
#include "qsv_version.h"
#include "avcodec_qsv.h"
#include "avcodec_reader.h"
#include "qsv_output.h"

using std::vector;

#define USE_CUSTOM_IO 1

static const int SUB_ENC_BUF_MAX_SIZE = 1024 * 1024;

typedef struct AVMuxFormat {
    AVFormatContext      *pFormatCtx;           //出力ファイルのformatContext
    char                  metadataStr[256];     //出力ファイルのエンコーダ名
    AVOutputFormat       *pOutputFmt;           //出力ファイルのoutputFormat

#if USE_CUSTOM_IO
    mfxU8                *pAVOutBuffer;         //avio_alloc_context用のバッファ
    uint32_t              nAVOutBufferSize;     //avio_alloc_context用のバッファサイズ
    FILE                 *fpOutput;             //出力ファイルポインタ
    char                 *pOutputBuffer;        //出力ファイルポインタ用のバッファ
    uint32_t              nOutputBufferSize;    //出力ファイルポインタ用のバッファサイズ
#endif //USE_CUSTOM_IO
    bool                  bStreamError;         //エラーが発生
    bool                  bIsMatroska;          //mkvかどうか
    bool                  bIsPipe;              //パイプ出力かどうか
    bool                  bFileHeaderWritten;   //ファイルヘッダを出力したかどうか
} AVMuxFormat;

typedef struct AVMuxVideo {
    AVCodec              *pCodec;               //出力映像のCodec
    AVCodecContext       *pCodecCtx;            //出力映像のCodecContext
    AVRational            nFPS;                 //出力映像のフレームレート
    AVStream             *pStream;              //出力ファイルの映像ストリーム
    bool                  bDtsUnavailable;      //出力映像のdtsが無効 (API v1.6以下)
    const AVCodecContext *pInputCodecCtx;       //入力映像のコーデック情報
    int64_t               nInputFirstPts;       //入力映像の最初のpts
    int                   nFpsBaseNextDts;      //出力映像のfpsベースでのdts (API v1.6以下でdtsが計算されない場合に使用する)
    bool                  bIsPAFF;              //出力映像がPAFFである
    mfxVideoParam         mfxParam;             //動画パラメータのコピー
    mfxExtCodingOption2   mfxCop2;              //動画パラメータのコピー
} AVMuxVideo;

typedef struct AVMuxAudio {
    int                   nInTrackId;           //ソースファイルの入力トラック番号
    int                   nInSubStream;         //ソースファイルの入力サブストリーム番号
    AVCodecContext       *pCodecCtxIn;          //入力音声のCodecContextのコピー
    int                   nStreamIndexIn;       //入力音声のStreamのindex
    int                   nDelaySamplesOfAudio; //入力音声の遅延 (pkt_timebase基準)
    AVStream             *pStream;              //出力ファイルの音声ストリーム
    int                   nPacketWritten;       //出力したパケットの数

    //変換用
    AVCodec              *pOutCodecDecode;      //変換する元のコーデック
    AVCodecContext       *pOutCodecDecodeCtx;   //変換する元のCodecContext
    AVCodec              *pOutCodecEncode;      //変換先の音声のコーデック
    AVCodecContext       *pOutCodecEncodeCtx;   //変換先の音声のCodecContext
    bool                  bDecodeError;         //デコード処理中にエラーが発生
    bool                  bEncodeError;         //エンコード処理中にエラーが発生
    AVPacket              OutPacket;            //変換用の音声バッファ

    //現在の音声のフォーマット
    int                   nChannels;            //現在のchannel数 (pSwrContext == nullptrなら、encoderの入力、そうでないならresamplerの入力)
    uint64_t              nChannelLayout;       //現在のchannel_layout (pSwrContext == nullptrなら、encoderの入力、そうでないならresamplerの入力)
    int                   nSampleRate;          //現在のsampling rate (pSwrContext == nullptrなら、encoderの入力、そうでないならresamplerの入力)
    AVSampleFormat        sampleFmt;            //現在のSampleformat (pSwrContext == nullptrなら、encoderの入力、そうでないならresamplerの入力)
    
    //resampler
    SwrContext           *pSwrContext;          //Sampleformatの変換用
    uint8_t             **pSwrBuffer;           //Sampleformatの変換用のバッファ
    uint32_t              nSwrBufferSize;       //Sampleformatの変換用のバッファのサイズ
    int                   nSwrBufferLinesize;   //Sampleformatの変換用
    AVFrame              *pDecodedFrameCache;   //デコードされたデータのキャッシュされたもの
    uint64_t              pnStreamChannels[MAX_SPLIT_CHANNELS]; //音声のチャンネル分離
    //AACの変換用
    AVBitStreamFilterContext *pAACBsfc;         //必要なら使用するbitstreamfilter

    int                   nOutputSamples;       //出力音声の出力済みsample数
    mfxI64                nLastPtsIn;           //入力音声の前パケットのpts
    mfxI64                nLastPtsOut;          //出力音声の前パケットのpts
} AVMuxAudio;

typedef struct AVMuxSub {
    int                   nInTrackId;           //ソースファイルの入力トラック番号
    AVCodecContext       *pCodecCtxIn;          //入力字幕のCodecContextのコピー
    int                   nStreamIndexIn;       //入力字幕のStreamのindex
    AVStream             *pStream;              //出力ファイルの字幕ストリーム

    //変換用
    AVCodec              *pOutCodecDecode;      //変換する元のコーデック
    AVCodecContext       *pOutCodecDecodeCtx;   //変換する元のCodecContext
    AVCodec              *pOutCodecEncode;      //変換先の音声のコーデック
    AVCodecContext       *pOutCodecEncodeCtx;   //変換先の音声のCodecContext

    uint8_t              *pBuf;                 //変換用のバッファ
} AVMuxSub;

enum {
    MUX_DATA_TYPE_NONE   = 0,
    MUX_DATA_TYPE_PACKET = 1, //AVPktMuxDataに入っているデータがAVPacket
    MUX_DATA_TYPE_FRAME  = 2, //AVPktMuxDataに入っているデータがAVFrame
};

typedef struct AVPktMuxData {
    int         type;        //MUX_DATA_TYPE_xxx
    AVPacket    pkt;         //type == MUX_DATA_TYPE_PACKET 時有効
    AVMuxAudio *pMuxAudio;   //type == MUX_DATA_TYPE_PACKET 時有効
    int64_t     dts;         //type == MUX_DATA_TYPE_PACKET 時有効
    int         samples;     //type == MUX_DATA_TYPE_PACKET 時有効
    AVFrame    *pFrame;      //type == MUX_DATA_TYPE_FRAME 時有効
    int         got_result;  //type == MUX_DATA_TYPE_FRAME 時有効
};

enum {
    AUD_QUEUE_PROCESS = 0,
    AUD_QUEUE_ENCODE  = 1,
    AUD_QUEUE_OUT     = 2,
};

#if ENABLE_AVCODEC_OUT_THREAD
typedef struct AVMuxThread {
    bool                         bEnableOutputThread;       //出力スレッドを使用する
    bool                         bEnableAudProcessThread;   //音声処理スレッドを使用する
    bool                         bEnableAudEncodeThread;    //音声エンコードスレッドを使用する
    std::atomic<bool>            bAbortOutput;              //出力スレッドに停止を通知する
    std::thread                  thOutput;                  //出力スレッド(mux部分を担当)
    std::atomic<bool>            bThAudProcessAbort;        //音声処理スレッドに停止を通知する
    std::thread                  thAudProcess;              //音声処理スレッド(デコード/thAudEncodeがなければエンコードも担当)
    std::atomic<bool>            bThAudEncodeAbort;         //音声エンコードスレッドに停止を通知する
    std::thread                  thAudEncode;               //音声エンコードスレッド(エンコードを担当)
    HANDLE                       heEventPktAddedOutput;     //キューのいずれかにデータが追加されたことを通知する
    HANDLE                       heEventClosingOutput;      //出力スレッドが停止処理を開始したことを通知する
    HANDLE                       heEventPktAddedAudProcess; //キューのいずれかにデータが追加されたことを通知する
    HANDLE                       heEventClosingAudProcess;  //音声処理スレッドが停止処理を開始したことを通知する
    HANDLE                       heEventPktAddedAudEncode;  //キューのいずれかにデータが追加されたことを通知する
    HANDLE                       heEventClosingAudEncode;   //音声処理スレッドが停止処理を開始したことを通知する
    CQueueSPSP<mfxBitstream, 64> qVideobitstreamFreeI;      //映像 Iフレーム用に空いているデータ領域を格納する
    CQueueSPSP<mfxBitstream, 64> qVideobitstreamFreePB;     //映像 P/Bフレーム用に空いているデータ領域を格納する
    CQueueSPSP<mfxBitstream, 64> qVideobitstream;           //映像パケットを出力スレッドに渡すためのキュー
    CQueueSPSP<AVPktMuxData, 64> qAudioPacketProcess;       //処理前音声パケットをデコード/エンコードスレッドに渡すためのキュー
    CQueueSPSP<AVPktMuxData, 64> qAudioFrameEncode;         //デコード済み音声フレームをエンコードスレッドに渡すためのキュー
    CQueueSPSP<AVPktMuxData, 64> qAudioPacketOut;           //音声パケットを出力スレッドに渡すためのキュー
} AVMuxThread;
#endif

typedef struct AVMux {
    AVMuxFormat         format;
    AVMuxVideo          video;
    vector<AVMuxAudio>  audio;
    vector<AVMuxSub>    sub;
    vector<sTrim>       trim;
#if ENABLE_AVCODEC_OUT_THREAD
    AVMuxThread         thread;
#endif
} AVMux;

typedef struct AVOutputStreamPrm {
    AVDemuxStream src;          //入力音声・字幕の情報
    const TCHAR  *pEncodeCodec; //音声をエンコードするコーデック
    int           nBitrate;     //ビットレートの指定
} AVOutputStreamPrm;

typedef struct AvcodecWriterPrm {
    const AVDictionary          *pInputFormatMetadata;    //入力ファイルのグローバルメタデータ
    const TCHAR                 *pOutputFormat;           //出力のフォーマット
    const mfxInfoMFX            *pVideoInfo;              //出力映像の情報
    bool                         bVideoDtsUnavailable;    //出力映像のdtsが無効 (API v1.6以下)
    const AVCodecContext        *pVideoInputCodecCtx;     //入力映像のコーデック情報
    int64_t                      nVideoInputFirstPts;     //入力映像の最初のpts
    vector<sTrim>                trimList;                //Trimする動画フレームの領域のリスト
    const mfxExtVideoSignalInfo *pVideoSignalInfo;        //出力映像の情報
    vector<AVOutputStreamPrm>    inputStreamList;         //入力ファイルの音声・字幕の情報
    vector<const AVChapter *>    chapterList;             //チャプターリスト
    int                          nBufSizeMB;              //出力バッファサイズ
    int                          nOutputThread;           //出力スレッド数
    int                          nAudioThread;            //音声処理スレッド数
} AvcodecWriterPrm;

class CAvcodecWriter : public CQSVOut
{
public:
    CAvcodecWriter();
    virtual ~CAvcodecWriter();

    virtual mfxStatus Init(const TCHAR *strFileName, const void *option, shared_ptr<CEncodeStatusInfo> pEncSatusInfo) override;

    virtual mfxStatus SetVideoParam(const mfxVideoParam *pMfxVideoPrm, const mfxExtCodingOption2 *cop2) override;

    virtual mfxStatus WriteNextFrame(mfxBitstream *pMfxBitstream) override;

    virtual mfxStatus WriteNextFrame(mfxFrameSurface1 *pSurface) override;

    virtual mfxStatus WriteNextPacket(AVPacket *pkt);

    virtual vector<int> GetStreamTrackIdList();

    virtual void Close();

#if USE_CUSTOM_IO
    int readPacket(uint8_t *buf, int buf_size);
    int writePacket(uint8_t *buf, int buf_size);
    int64_t seek(int64_t offset, int whence);
#endif //USE_CUSTOM_IO
    //出力スレッドのハンドルを取得する
    HANDLE getThreadHandleOutput();
    HANDLE getThreadHandleAudProcess();
    HANDLE getThreadHandleAudEncode();
private:
    //別のスレッドで実行する場合のスレッド関数 (出力)
    mfxStatus WriteThreadFunc();

    //別のスレッドで実行する場合のスレッド関数 (音声処理)
    mfxStatus ThreadFuncAudThread();

    //別のスレッドで実行する場合のスレッド関数 (音声エンコード処理)
    mfxStatus ThreadFuncAudEncodeThread();

    //音声出力キューに追加 (音声処理スレッドが有効な場合のみ有効)
    mfxStatus AddAudQueue(AVPktMuxData *pktData, int type);

    //AVPktMuxDataを初期化する
    AVPktMuxData pktMuxData(const AVPacket *pkt);

    //AVPktMuxDataを初期化する
    AVPktMuxData pktMuxData(AVFrame *pFrame);

    //WriteNextFrameの本体
    mfxStatus WriteNextFrameInternal(mfxBitstream *pMfxBitstream, int64_t *pWrittenDts);

    //WriteNextPacketの本体
    mfxStatus WriteNextPacketInternal(AVPktMuxData *pktData);

    //WriteNextPacketの音声処理部分(デコード/thAudEncodeがなければエンコードも担当)
    mfxStatus WriteNextPacketAudio(AVPktMuxData *pktData);

    //WriteNextPacketの音声処理部分(エンコード)
    mfxStatus WriteNextPacketAudioFrame(AVPktMuxData *pktData);

    //音声フレームをエンコード
    mfxStatus WriteNextAudioFrame(AVPktMuxData *pktData);

    //CodecIDがPCM系かどうか判定
    bool codecIDIsPCM(AVCodecID targetCodec);

    //PCMのコーデックがwav出力時に変換を必要とするかを判定する
    AVCodecID PCMRequiresConversion(const AVCodecContext *audioCtx);

    //QSVのコーデックFourccからAVCodecのCodecIDを返す
    AVCodecID getAVCodecId(mfxU32 QSVFourcc);

    //"<mes> for codec"型のエラーメッセージを作成する  
    tstring errorMesForCodec(const TCHAR *mes, AVCodecID targetCodec);

    //AAC音声にBitstreamフィルターを適用する
    void applyBitstreamFilterAAC(AVPacket *pkt, AVMuxAudio *pMuxAudio);

    //H.264ストリームからPAFFのフィールドの長さを返す
    mfxU32 getH264PAFFFieldLength(mfxU8 *ptr, mfxU32 size);

    //extradataをコピーする
    void SetExtraData(AVCodecContext *codecCtx, const mfxU8 *data, mfxU32 size);
    
    //映像の初期化
    mfxStatus InitVideo(const AvcodecWriterPrm *prm);

    //音声の初期化
    mfxStatus InitAudioResampler(AVMuxAudio *pMuxAudio, int channels, uint64_t channel_layout, int sample_rate, AVSampleFormat sample_fmt);

    //音声の初期化
    mfxStatus InitAudio(AVMuxAudio *pMuxAudio, AVOutputStreamPrm *pInputAudio);

    //字幕の初期化
    mfxStatus InitSubtitle(AVMuxSub *pMuxSub, AVOutputStreamPrm *pInputSubtitle);

    //チャプターをコピー
    mfxStatus SetChapters(const vector<const AVChapter *>& chapterList);

    //メッセージを作成
    tstring GetWriterMes();

    //対象のパケットの必要な対象のストリーム情報へのポインタ
    AVMuxAudio *getAudioPacketStreamData(const AVPacket *pkt);

    //対象のパケットの必要な対象のストリーム情報へのポインタ
    AVMuxAudio *getAudioStreamData(int nTrackId, int nSubStreamId = 0);

    //対象のパケットの必要な対象のストリーム情報へのポインタ
    AVMuxSub *getSubPacketStreamData(const AVPacket *pkt);

    //音声のchannel_layoutを自動選択する
    uint64_t AutoSelectChannelLayout(const uint64_t *pChannelLayout, const AVCodecContext *pSrcAudioCtx);

    //音声のsample formatを自動選択する
    AVSampleFormat AutoSelectSampleFmt(const AVSampleFormat *pSamplefmtList, const AVCodecContext *pSrcAudioCtx);

    //音声のサンプリングレートを自動選択する
    int AutoSelectSamplingRate(const int *pSamplingRateList, int nSrcSamplingRate);

    //音声ストリームをすべて吐き出す
    void AudioFlushStream(AVMuxAudio *pMuxAudio, int64_t *pWrittenDts);

    //音声をデコード
    AVFrame *AudioDecodePacket(AVMuxAudio *pMuxAudio, const AVPacket *pkt, int *got_result);

    //音声をresample
    int AudioResampleFrame(AVMuxAudio *pMuxAudio, AVFrame **frame);

    //音声をエンコード
    int AudioEncodeFrame(AVMuxAudio *pMuxAudio, AVPacket *pEncPkt, const AVFrame *frame, int *got_result);

    //字幕パケットを書き出す
    mfxStatus SubtitleTranscode(const AVMuxSub *pMuxSub, AVPacket *pkt);

    //字幕パケットを書き出す
    mfxStatus SubtitleWritePacket(AVPacket *pkt);

    //パケットを実際に書き出す
    void WriteNextPacketProcessed(AVPktMuxData *pktData);

    //パケットを実際に書き出す
    void WriteNextPacketProcessed(AVMuxAudio *pMuxAudio, AVPacket *pkt, int samples, int64_t *pWrittenDts);

    //extradataに動画のヘッダーをセットする
    mfxStatus SetSPSPPSToExtraData(const mfxVideoParam *pMfxVideoPrm);

    //extradataにHEVCのヘッダーを追加する
    mfxStatus AddHEVCHeaderToExtraData(const mfxBitstream *pMfxBitstream);

    //ファイルヘッダーを書き出す
    mfxStatus WriteFileHeader(const mfxVideoParam *pMfxVideoPrm, const mfxExtCodingOption2 *cop2, const mfxBitstream *pMfxBitstream);

    //タイムスタンプをTrimなどを考慮しつつ計算しなおす
    //nTimeInがTrimで切り取られる領域の場合
    //lastValidFrame ... true 最後の有効なフレーム+1のtimestampを返す / false .. AV_NOPTS_VALUEを返す
    int64_t AdjustTimestampTrimmed(int64_t nTimeIn, AVRational timescaleIn, AVRational timescaleOut, bool lastValidFrame);

    void CloseSubtitle(AVMuxSub *pMuxSub);
    void CloseAudio(AVMuxAudio *pMuxAudio);
    void CloseVideo(AVMuxVideo *pMuxVideo);
    void CloseFormat(AVMuxFormat *pMuxFormat);
    void CloseThread();
    void CloseQueues();

    AVMux m_Mux;
    vector<AVPktMuxData> m_AudPktBufFileHead; //ファイルヘッダを書く前にやってきた音声パケットのバッファ
};

#endif //ENABLE_AVCODEC_QSV_READER

#endif //_AVCODEC_WRITER_H_