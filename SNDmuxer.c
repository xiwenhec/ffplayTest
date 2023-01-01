
#include "libavcodec/avcodec.h"
#include "libavcodec/codec.h"
#include "libavcodec/packet.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include <stdio.h>
#include <unistd.h>

AVFrame *frame;
static int frame_count = 0;
static AVStream *videoStream;

int open_codec_context(AVFormatContext *fmt_ctx, AVCodecContext **dec_ctx,
                       int *stream_idx, enum AVMediaType type) {

  int ret = av_find_best_stream(fmt_ctx, type, -1, 1, NULL, 0);

  if (ret < 0) {
    return ret;
  }
  int streamIndex = ret;
  AVStream *st = fmt_ctx->streams[streamIndex];
  const AVCodec *dec = avcodec_find_decoder(st->codecpar->codec_id);
  if (!dec) {
    return -1;
  }

  *dec_ctx = avcodec_alloc_context3(dec);
  if (!*dec_ctx) {
    return -1;
  }
  ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar);
  if (ret < 0) {
    return ret;
  }

  ret = avcodec_open2(*dec_ctx, dec, NULL);
  if (ret < 0) {
    return ret;
  }
  *stream_idx = streamIndex;
  return 0;
}

int output_video_frame(AVFrame *frame) {
  frame_count++;
  double pts = av_rescale_q(frame->pts , videoStream->time_base, av_get_time_base_q());
  //
  printf("pkt pts=%lf,time_base = %d/%d\n", pts,frame->time_base.num, frame->time_base.den);
  // printf("frame_count = %d\n", frame_count);
  return 0;
}

int decode_packet(AVCodecContext *dec, const AVPacket *packet) {
  
  sleep(1);
  int64_t duration = av_rescale_q(packet->duration , videoStream->time_base, av_get_time_base_q());

  printf("packet duration = %ld\n", duration);
 

  int ret = avcodec_send_packet(dec, packet);
  if (ret < 0) {
    return ret;
  }

  while (ret >= 0) {
    ret = avcodec_receive_frame(dec, frame);
    if (ret < 0) {
      // those two return values are special and mean there is no output
      // frame available, but there were no errors during decoding
      if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
        return 0;
      }
      fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
      return ret;
    }
    // write the frame data to output file
    if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
      ret = output_video_frame(frame);
    else {
      // ret = output_audio_frame(frame);
    }
    av_frame_unref(frame);
  }
  return 0;
}

int main() {

  char sourcePath[] = "./res/ceshi.mp4";
  AVFormatContext *mCtx = NULL;

  int ret = avformat_open_input(&mCtx, sourcePath, NULL, NULL);
  ret = avformat_find_stream_info(mCtx, NULL);

  videoStream = mCtx->streams[0];
  AVStream *stream1 = mCtx->streams[1];

  printf("duration = %ld\n", mCtx->duration);
// av_get_time_base_q()
  // stream
  printf("stream: duration = %ld, time_base = %d/%d\n", videoStream->duration,
         videoStream->time_base.num, videoStream->time_base.den);
  printf("stream: duration = %ld, time_base = %d/%d\n", stream1->duration,
         stream1->time_base.num, stream1->time_base.den);
  
  printf("start time = %ld\n",mCtx->start_time);

  AVCodecContext *videoCodecCtx = NULL;
  int videoStreamIndex = -1;
  ret = open_codec_context(mCtx, &videoCodecCtx, &videoStreamIndex,
                           AVMEDIA_TYPE_VIDEO);
  if (ret < 0) {
    printf("open_code_context failed.\n");
  }

  int width = videoCodecCtx->width;
  int height = videoCodecCtx->height;
  enum AVPixelFormat pixFmt = videoCodecCtx->pix_fmt;

  printf("video width = %d, height = %d pixFormat = %d\n", width, height,
         pixFmt);

  static uint8_t *videoDstData[4] = {NULL};
  static int video_dst_linesize[4];
  ret = av_image_alloc(videoDstData, video_dst_linesize, width, height, pixFmt,
                       1);


  frame = av_frame_alloc();

  AVPacket *pkt = av_packet_alloc();
  while (av_read_frame(mCtx, pkt) >= 0) {
    if (pkt->stream_index == videoStreamIndex) {
      ret = decode_packet(videoCodecCtx, pkt);
    }
    av_packet_unref(pkt);
    if (ret < 0) {
      break;
    }
  }

  return 0;
}