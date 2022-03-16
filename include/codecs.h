
int audiocodec_get_num_codecs();
lqt_codec_info_static_t * audiocodec_get_codec_info(int index);
lqt_init_codec_func_t audiocodec_get_codec(int index);

int videocodec_get_num_codecs();
lqt_codec_info_static_t * videocodec_get_codec_info(int index);
lqt_init_codec_func_t videocodec_get_codec(int index);

int ffmpegcodec_get_num_codecs();
lqt_codec_info_static_t * ffmpegcodec_get_codec_info(int index);
lqt_init_codec_func_t ffmpegcodec_get_codec(int index);

