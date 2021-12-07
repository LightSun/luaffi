-- for gcc/vc c-enum is 4 size.
-- : https://github.com/FFmpeg/FFmpeg/blob/release%2F4.4/doc/examples/decode_video.c
print("-------- start test_ffmpeg---------");
AV_INPUT_BUFFER_PADDING_SIZE = 64
-- enum 
AV_CODEC_ID_MPEG1VIDEO = 1

local function check_ptr(val, msg)
	if not val.hasData() then
		print(msg)	
		os.exit(1)	
	end
end

local function check_state(v, msg)
	if not v then
		print(msg)	
		os.exit(1)	
	end
end

local INBUF_SIZE = 4096
local filename = ""; -- TODO
local outfilename = "";

hffi.defines();

local lib_Dir ="D:/study/tools/ffmpeg/gcc-win64-lgpl-shared-4.4/bin/"
local avcodec_path = "avcodec-58";

-- pkt = av_packet_alloc();
-- need AVPacket, AVBufferRef, AVBuffer. AVPacketSideData
local stru_pkt = hffi.struct{
	"no_data" = true;
	"free_data" = false;
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
-- 1， uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
local inbuf = hffi.array(uint8, INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE) -- default zero


local avcodec = hffi.loadLib(lib_Dir, avcodec_path)

-- 2， pkt = av_packet_alloc();
local pkt = avcodec.av_packet_alloc {ret = hffi.valuePtr(void)}
print("pkt.addr():", pkt.addr())
pkt.as(stru_pkt)

-- /* find the MPEG-1 video decoder */
-- codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
local stru_codec = hffi.struct{
	"no_data" = true;
	"free_data" = false;
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

local codec = avcodec.avcodec_find_decoder{ret = hffi.valuePtr(void), hffi.value(int, AV_CODEC_ID_MPEG1VIDEO)}
check_ptr(codec, "Codec not found\n")

codec.as(stru_codec)

-- parser = av_parser_init(codec->id);
local parser = avcodec.av_parser_init{ret = hffi.valuePtr(void), hffi.value(int, stru_codec.id)}
check_ptr(parser, "Parser not found\n")

-- c = avcodec_alloc_context3(codec);
local c = avcodec.avcodec_alloc_context3 {ret = hffi.valuePtr(void), hffi.valuePtr(stru_codec)}
check_ptr(parser, "Could not allocate video codec context\n")

local stru_AVRational = hffi.struct{
	int "num"; --///< Numerator
    int "den"; --///< Denominator
}
-- struct AVCodecContext
local stru_context = hffi.struct{
	"no_data" = true;
	"free_data" = false;
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
	stru_AVRational, "time_base";
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
	
	
}

-- if (avcodec_open2(c, codec, NULL) < 0) ...
if(avcodec.avcodec_open2{ret = hffi.value(int), c, codec}.get() < 0) then
	print("Could not open codec\n")	
	os.exit(1)	
end

-- f = fopen(filename, "rb");
local f = c_runtime.fopen {ret = hffi.valuePtr(void), hffi.valuePtr(filename), hffi.valuePtr("rb") } 
check_ptr(f, "Could not open "..filename)

-- frame = av_frame_alloc();
local frame = avcodec.av_frame_alloc({ret = hffi.valuePtr(void)})
check_ptr(frame, "Could not allocate video frame\n")


-- static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, const char *filename)
local function decode(c, frame, stru_pkt, outfilename)
	--TODO 
	local val_ret = avcodec.avcodec_send_packet({ret=int;c, stru_pkt})
	check_state(val_ret.get() < 0, "Error sending a packet for decoding\n");
	while(val_ret.get() >= 0) do
		avcodec.avcodec_receive_frame {ret = val_ret; c, frame}
		if(val_ret.get() == EAGAIN) then
			return
		end
		if(val_ret.get() == AVERROR_EOF) then
			return
		end
		check_state(val_ret.get() < 0, "Error during decoding\n");
		print("saving frame ", )
	end
end

local val_int = hffi.value(int, 0)
local data_size = hffi.value(uint, 0)
local _ret;
-- &pkt->data
local pkt_data_ptr = stru_pkt.data.ptrValue(); 
local pkt_size_ptr = hffi.value(pointer, int, stru_pkt.size)
-- data = inbuf
local data = hffi.valuePtr(void)
local AV_NOPTS_VALUE = hffi.value(sint64, 0x8000000000000000)
print("AV_NOPTS_VALUE:", AV_NOPTS_VALUE.get())
local ZERO = hffi.value(int, 0)
local offset = 0;
while(true) do
	_ret = c_runtime.feof({ret = val_int, f}).get()
	print("c_runtime.feof: result: "..tostring(_ret))
	if(_ret == 0) then
		print("c_runtime.feof:  end.")
		break;
	end
	print("c_runtime.feof: "..tostring(_ret))
	
	--data_size = fread(inbuf, 1, INBUF_SIZE, f);
	c_runtime.fread{ret = data_size ; hffi.valuePtr(inbuf), hffi.value(uint, 1), hffi.value(uint, INBUF_SIZE), f}
	if data_size.get() == 0 then
		break;
	end
	-- data = inbuf
	offset = 0;
	data.setPtr(inbuf, offset)
	while(data_size.get() > 0) do
		--  ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
        --                           data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0); 
		_ret = avcodec.av_parser_parse2({ret = val_int; parser, c, pkt_data_ptr, pkt_size_ptr, 
				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, ZERO}).get()
		if(_ret < 0) then 
			print("Error while parsing\n")
			os.exit(1)
		end	
		-- reset pkt size.
		stru_pkt.size = pkt_size_ptr.get()	
		--  data      += ret;	data_size -= ret; 	
		offset += _ret;
		data_size.add(-_ret)
		data.setPtr(inbuf, offset)
		
		-- if (pkt->size) decode(c, frame, pkt, outfilename);
		if(pkt_size_ptr.get() != 0) then
			decode(c, frame, stru_pkt, outfilename);
		end
	end
end
 -- /* flush the decoder */
decode(c, frame, nil, outfilename);
-- fclose(f);
c_runtime.fclose{f}
f.ptrToNull()
--[[ 
 av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
]]-- 
avcodec.av_parser_close({parser})
parser.ptrToNull()

avcodec.avcodec_free_context({c.ptrValue()})
c.ptrToNull()

avcodec.av_frame_free({frame.ptrValue()})
frame.ptrToNull()

avcodec.av_packet_free({pkt.ptrValue()})
pkt.ptrToNull()
stru_pkt.ptrToNull()

hffi.undefines();
print("-------- end test_ffmpeg ---------");