
local stru_AVRational = hffi.struct{
	int, "num"; --///< Numerator
    int, "den"; --///< Denominator
}
-- uint64_t error[AV_NUM_DATA_POINTERS];
local AV_NUM_DATA_POINTERS = 8;
local arr_error = hffi.array(uint64_t, AV_NUM_DATA_POINTERS);
-- struct AVCodecContext
local stru_context = hffi.struct({
	no_data = true;
	free_data = false;
	pointer, "av_class"; -- AVClass*	
	int, "log_level_offset";
	int, "codec_type";	 -- enum AVMediaType
	pointer, "codec";
	int, "codec_id";	 -- enum AVCodecID
	uint, "codec_tag";
	pointer, "priv_data";
	pointer, "internal";
	pointer, "opaque";
	sint64, "bit_rate";
	int, "bit_rate_tolerance";
	int, "global_quality";
	int, "compression_level";
	int, "flags";
	int, "flags2";
	pointer, "extradata";
	int, "extradata_size";
	stru_AVRational.copy(), "time_base";
	int, "ticks_per_frame";
	int, "delay";
	int, "width";
	int, "height";
	int, "coded_width";
	int, "coded_height";
	int, "gop_size";
	int, "pix_fmt"; -- AVPixelFormat
	pointer, "draw_horiz_band";
	pointer, "get_format";
	int, "max_b_frames";
	float, "b_quant_factor";
	int, "b_frame_strategy";
	float, "b_quant_offset";
	int, "has_b_frames";
	int, "mpeg_quant";
	float, "i_quant_factor";
	float, "i_quant_offset";
	float, "lumi_masking";
	float, "temporal_cplx_masking";
	float, "spatial_cplx_masking";
	float, "p_masking";
	float, "dark_masking";
	int, "slice_count";
	int, "prediction_method";
	pointer, "slice_offset";
	stru_AVRational.copy(), "sample_aspect_ratio";
	int, "me_cmp";
	int, "me_sub_cmp";
	int, "mb_cmp";
	int, "ildct_cmp";
	int, "dia_size";
	int, "last_predictor_count";
	int, "pre_me";
	int, "me_pre_cmp";
	int, "pre_dia_size";
	int, "me_subpel_quality";
	int, "me_range";
	int, "slice_flags";
	int, "mb_decision";
	pointer, "intra_matrix";
	pointer, "inter_matrix";
	int, "scenechange_threshold";
	int, "noise_reduction";
	int, "intra_dc_precision";
	int, "skip_top";
	int, "skip_bottom";
	int, "mb_lmin";
	int, "mb_lmax";
	int, "me_penalty_compensation";
	int, "bidir_refine";
	int, "brd_scale";
	int, "keyint_min";
	int, "refs";
	int, "mv0_threshold";
	int, "b_sensitivity";
	int, "color_primaries"; -- enum
	int, "color_trc";
	int, "colorspace";
	int, "color_range";
	int, "chroma_sample_location";
	int, "slices";
	int, "field_order";
	int, "sample_rate";
	int, "channels";
	int, "sample_fmt";
	int, "frame_size";
	int, "frame_number";
	int, "block_align";
	
	int, "cutoff";
	uint64, "channel_layout";
	uint64, "request_channel_layout";
	int, "audio_service_type";
	int, "request_sample_fmt";
	pointer, "get_buffer2";
	int, "refcounted_frames";
	float, "qcompress";
	float, "qblur";
	int, "qmin";
	int, "qmax";
	
	int, "max_qdiff";
	int, "rc_buffer_size";
	int, "rc_override_count";
	pointer, "rc_override";
	
	sint64, "rc_max_rate";
	sint64, "rc_min_rate";
	float, "rc_max_available_vbv_use";
	float, "rc_min_vbv_overflow_use";
	
	int, "rc_initial_buffer_occupancy";
	int, "coder_type";
	int, "context_model";
	int, "frame_skip_threshold";
	int, "frame_skip_factor";
	int, "frame_skip_exp";
	int, "frame_skip_cmp";
	int, "trellis";
	int, "min_prediction_order";
	int, "max_prediction_order";
	
	sint64, "timecode_frame_start";
	pointer, "rtp_callback";
	int, "rtp_payload_size";
	int, "mv_bits";
	int, "header_bits";
	int, "i_tex_bits";
	int, "p_tex_bits";
	int, "i_count";
	int, "p_count";
	int, "skip_count";
	int, "misc_bits";
	int, "frame_bits";
	pointer, "stats_out";
	pointer, "stats_in";
	int, "workaround_bugs";
	int, "strict_std_compliance";
	int, "error_concealment";
	int, "debug";
	int, "err_recognition"; --TODO continue.
	
	int64_t,"reordered_opaque";
	pointer, "hwaccel";
	pointer, "hwaccel_context";
	arr_error, "error";
	
	int, "dct_algo";
	int, "idct_algo";
	int, "bits_per_coded_sample";
	int, "bits_per_raw_sample";
	int, "lowres";
	pointer, "coded_frame";
	int, "thread_count";
	int, "thread_type";
	int, "active_thread_type";
	int, "thread_safe_callbacks";
	pointer, "execute";
	pointer, "execute2";
	
	int, "nsse_weight";
	int, "profile";
	int, "level";
	int, "skip_loop_filter"; -- enum
	int, "skip_idct";
	int, "skip_frame";
	pointer, "subtitle_header";
	int, "subtitle_header_size";
	uint64_t, "vbv_delay";
	int, "side_data_only_packets";
	int, "initial_padding";
	stru_AVRational.copy(), "framerate";
	
	int, "sw_pix_fmt";
	stru_AVRational.copy(), "pkt_timebase";
	pointer, "codec_descriptor";
	int64_t, "pts_correction_num_faulty_pts";
	int64_t, "pts_correction_num_faulty_dts";
	int64_t, "pts_correction_last_pts";
	int64_t, "pts_correction_last_dts";
	pointer, "sub_charenc";
	int, "sub_charenc_mode";
	int, "skip_alpha";
	int, "seek_preroll";
	int, "debug_mv";
	pointer, "chroma_intra_matrix";
	
	pointer, "dump_separator";
	pointer, "codec_whitelist";
	int, "properties"; --unsigned
	pointer, "coded_side_data";
	int, "nb_coded_side_data";
	pointer, "hw_frames_ctx";
	int, "sub_text_format";
	int, "trailing_padding";
	int64_t, "max_pixels";
	pointer, "hw_device_ctx";
	int, "hwaccel_flags";
	int, "apply_cropping";
	int, "extra_hw_frames";
	
	int, "discard_damaged_percentage";
	int64_t, "max_samples";
	int, "export_side_data";
	pointer, "get_encode_buffer";
})

local arr_frame_data = hffi.array(pointer, AV_NUM_DATA_POINTERS)

local arr_linesize = hffi.array(int, AV_NUM_DATA_POINTERS)
local arr_error = hffi.array(int, AV_NUM_DATA_POINTERS)

local arr_buf = hffi.array(pointer, AV_NUM_DATA_POINTERS)

local stru_avframe = hffi.struct{
	no_data = true;
	free_data = false;
	arr_frame_data, "data";
	arr_linesize, "linesize";
	pointer, "extended_data";
	int, "width";
	int, "height";
	int, "nb_samples";
	int, "format";
	int, "key_frame";
	int, "pict_type"; -- enum
	stru_AVRational.copy(), "sample_aspect_ratio";
	int64_t, "pts";
	int64_t, "pkt_pts";
	int64_t, "pkt_dts";
	int, "coded_picture_number";
	int, "display_picture_number";
	int, "quality";
	pointer, "opaque";
	arr_error, "error";
	int, "repeat_pict";
	int, "interlaced_frame";
	int, "top_field_first";
	int, "palette_has_changed";
	int64_t, "reordered_opaque";
	
	int, "sample_rate";
	uint64_t, "channel_layout";
	arr_buf.copy(), "buf";
	pointer, "extended_buf";
	int, "nb_extended_buf";
	
	pointer, "side_data";
	int, "nb_side_data";
	int, "flags";
	int, "color_range";
	int, "color_primaries";
	int, "color_trc";
	int, "colorspace";
	int, "chroma_location";
	int64_t, "best_effort_timestamp";
	int64_t, "pkt_pos";
	int64_t, "pkt_duration";
	pointer, "metadata";
	
	int, "decode_error_flags";
	int, "channels";
	int, "pkt_size";
	pointer, "qscale_table";
	int, "qstride";
	int, "qscale_type";
	pointer, "qp_table_buf";
	pointer, "hw_frames_ctx";
	pointer, "opaque_ref";
	
	size_t, "crop_top";
	size_t, "crop_bottom";
	size_t, "crop_left";
	size_t, "crop_right";
	pointer, "private_ref";
}
-- avcodec 
local stru_codec = hffi.struct{
	no_data = true;
	free_data = false;
	pointer, "name";  -- const char*
	pointer, "long_name";
	int, "type"; --  enum AVMediaType
	int, "id"; --    enum AVCodecID
	int, "capabilities";
	pointer, "supported_framerates";  -- AVRational *
	pointer, "pix_fmts";  			  -- AVPixelFormat *
	pointer, "supported_samplerates"; -- int*
	pointer, "sample_fmts";			  -- AVSampleFormat *	
	pointer, "channel_layouts";		  -- uint64_t*
	uint8, "max_lowres";			  
	pointer, "priv_class";		 -- AVClass *
	pointer, "profiles";	     -- AVProfile*
	pointer, "wrapper_name";     -- const char*	
	pointer, "priv_data_size";	 -- int
	pointer, "next";			 -- AVCodec* 
	pointer, "update_thread_context";	-- closure
	pointer, "defaults";	 -- AVCodecDefault*
	
	pointer, "init_static_data";  -- closure ...	
	pointer, "init";	
	pointer, "encode_sub";	
	pointer, "encode2";	
	pointer, "decode";	
	pointer, "close";	
	pointer, "receive_packet";	
	pointer, "receive_frame";	
	pointer, "flush";	
	
	int, "caps_internal";	
	pointer, "bsfs";	-- const char* 
	pointer, "hw_configs";	-- AVCodecHWConfigInternal**
	pointer, "codec_tags";	-- uint32_t*	
}
-- AVPacket
local stru_pkt = hffi.struct{
	no_data = true;
	free_data = false;
	pointer, "buf"; -- AVBufferRef*
	sint64, "pts";
	sint64, "dts";
	pointer, "data"; -- uint8_t*
	int, "size";
	int, "stream_index";
	int, "flags";
	pointer, "side_data"; -- AVPacketSideData*
	int, "side_data_elems";
	
	sint64, "duration";
	sint64, "pos";
	sint64, "convergence_duration";
}

return {
avcontext = stru_context,
avframe = stru_avframe,
avcodec = stru_codec,
avpacket = stru_pkt,
}