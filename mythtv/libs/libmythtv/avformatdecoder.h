#ifndef AVFORMATDECODER_H_
#define AVFORMATDECODER_H_

#include <qstring.h>
#include <qmap.h>

#include "programinfo.h"
#include "format.h"
#include "decoderbase.h"

extern "C" {
#include "frame.h"
#include "../libavcodec/avcodec.h"
#include "../libavformat/avformat.h"
}

class ProgramInfo;
class MythSqlDatabase;

/// A decoder for video files.

/// The AvFormatDecoder is used to decode non-NuppleVideo files.
/// It's used a a decoder of last resort after trying the NuppelDecoder
/// and IvtvDecoder (if "USING_IVTV" is defined).
class AvFormatDecoder : public DecoderBase
{
  public:
    AvFormatDecoder(NuppelVideoPlayer *parent, MythSqlDatabase *db,
                    ProgramInfo *pginfo);
   ~AvFormatDecoder();

    void Reset(void);

    /// Perform an av_probe_input_format on the passed data to see if we can decode it with this class.
    static bool CanHandle(char testbuf[2048], const QString &filename);

    /// Open our file and set up or audio and video parameters.
    int OpenFile(RingBuffer *rbuffer, bool novideo, char testbuf[2048]);

    /// Decode a frame of video/audio.  If onlyvideo is set, just decode the video portion.
    void GetFrame(int onlyvideo);

    bool isLastFrameKey(void) { return false; }

    /// This is a No-op for this class.
    void WriteStoredData(RingBuffer *rb, bool storevid)
                           { (void)rb; (void)storevid; }

    /// This is a No-op for this class.
    void SetRawAudioState(bool state) { (void)state; }

    /// This is a No-op for this class.
    bool GetRawAudioState(void) { return false; }

    /// This is a No-op for this class.
    void SetRawVideoState(bool state) { (void)state; }
    /// This is a No-op for this class.

    bool GetRawVideoState(void) { return false; }

    /// This is a No-op for this class.
    long UpdateStoredFrameNum(long frame) { (void)frame; return 0;}

    QString GetEncodingType(void) { return QString("MPEG-2"); }

    void SetPixelFormat(const int);

    virtual void incCurrentAudioTrack();
    virtual void decCurrentAudioTrack();
    virtual bool setCurrentAudioTrack(int trackNo);

  protected:
    /// Loop through the streams in the file to identify audio streams.
    bool scanAudioTracks();

    /// Attempt to find the optimal audio stream to use based on the number of channels,
    /// and if we're doing AC3 passthrough.  This will select the highest stream number
    /// that matches our criteria.
    bool autoSelectAudioTrack();

    RingBuffer *getRingBuf(void) { return ringBuffer; }

  private:
    friend int get_avf_buffer(struct AVCodecContext *c, AVFrame *pic);
    friend void release_avf_buffer(struct AVCodecContext *c, AVFrame *pic);

    friend int get_avf_buffer_xvmc(struct AVCodecContext *c, AVFrame *pic);
    friend void release_avf_buffer_xvmc(struct AVCodecContext *c, AVFrame *pic);
    friend void render_slice_xvmc(struct AVCodecContext *c, const AVFrame *src,
                                  int offset[4], int y, int type, int height);

    friend int open_avf(URLContext *h, const char *filename, int flags);
    friend int read_avf(URLContext *h, uint8_t *buf, int buf_size);
    friend int write_avf(URLContext *h, uint8_t *buf, int buf_size);
    friend offset_t seek_avf(URLContext *h, offset_t offset, int whence);
    friend int close_avf(URLContext *h);

    void InitByteContext(void);

    /// Preprocess a packet, setting the video parms if nessesary. Also feeds HandleGopStart for MPEG2 files.
    void MpegPreProcessPkt(AVStream *stream, AVPacket *pkt);

    float GetMpegAspect(AVCodecContext *context, int aspect_ratio_info,
                        int width, int height);

    void SeekReset(long long newKey = 0, int skipFrames = 0,
                   bool needFlush = false);

    /// See if the video parameters have changed, return true if so.
    bool CheckVideoParams(int width, int height);

    /// See if the audio parameters have changed, return true if so.
    bool CheckAudioParams(int freq, int channels);

    int EncodeAC3Frame(unsigned char* data, int len, short *samples,
                       int &samples_size);

    // Update our position map, keyframe distance, and the like.  Called for key frame packets.
    void HandleGopStart(AVPacket *pkt);

    AVFormatContext *ic;
    AVFormatParameters params;

    URLContext readcontext;

    int frame_decoded;

    bool directrendering;
    bool drawband;

    int audio_sample_size;
    int audio_sampling_rate;
    int audio_channels;

    int audio_check_1st;         ///< Used by CheckAudioParams
    int audio_sampling_rate_2nd; ///< Used by CheckAudioParams
    int audio_channels_2nd;      ///< Used by CheckAudioParams

    bool hasbframes;             ///< Set in open but not used.

    int bitrate;

    bool gopset;
    /// A flag to indicate that we've seen a GOP frame.  Used in junction with seq_count.
    bool seen_gop;
    int seq_count; ///< A counter used to determine if we need to force a call to HandleGopStart

    QPtrList<VideoFrame> inUseBuffers;

    QPtrList<AVPacket> storedPackets;

    int firstgoppos;
    int prevgoppos;

    bool gotvideo;

    unsigned char prvpkt[3];

    long long video_last_P_pts;
    long long lastvpts;
    long long lastapts;

    bool do_ac3_passthru;

    short int audioSamples[AVCODEC_MAX_AUDIO_FRAME_SIZE];

    QValueVector<int> audioStreams;
    int wantedAudioStream;
};

#endif
