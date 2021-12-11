-- for gcc/vc c-enum is 4 size.
-- : https://github.com/FFmpeg/FFmpeg/blob/release%2F4.4/doc/examples/decode_video.c
-- ffmpeg -i foreman.mp4 -c:v mpeg2video -q:v 5 -c:a mp2 -f vob video.mpg
local ints = require("ints")
local AVERROR_EOF = -ints.shift_bor2("EOF ")
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
-- ./decode_video ~/Videos/MyVideo.mpeg ~/Videos/frame
local filename = "foreman.mpeg"; -- TODO '.mpeg'
local outfilename = "foreman";

hffi.defines();
local ffmpeg_structs = require("ffmpeg_structs")

local lib_Dir ="D:/study/tools/ffmpeg/gcc-win64-lgpl-shared-4.4/bin/"
local avcodec_path = "avcodec-58";
local avutil_path = "avutil-56";

-- pkt = av_packet_alloc();
-- need AVPacket, AVBufferRef, AVBuffer. AVPacketSideData
local stru_pkt = ffmpeg_structs.avpacket
-- 1， uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
--     memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE); hffi default all is zero in array.
local inbuf = hffi.array(uint8, INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE) -- default zero


local avcodec = hffi.loadLib(lib_Dir, avcodec_path)
local avutil = hffi.loadLib(lib_Dir, avutil_path)

avcodec.avcodec_register_all{}

-- 2， pkt = av_packet_alloc();
print("--- 2, start call 'avcodec.av_packet_alloc(...)' ---")
local tmp_val = hffi.valuePtr(void);
assert(tmp_val.hasData() == false)
local pkt = avcodec.av_packet_alloc {ret = tmp_val}
check_ptr(pkt, "pkt allocate failed\n")
print("pkt.addr():", pkt.addr())
pkt.as(stru_pkt)

-- /* find the MPEG-1 video decoder */
-- 3, codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
local stru_codec = ffmpeg_structs.avcodec;

print("--- 3, start call 'avcodec.avcodec_find_decoder(...)' ---")
local codec = avcodec.avcodec_find_decoder{ret = hffi.valuePtr(void), hffi.value(int, AV_CODEC_ID_MPEG1VIDEO)}
check_ptr(codec, "Codec not found\n")

codec.as(stru_codec)

-- 4, parser = av_parser_init(codec->id);
print("--- 4, start call 'avcodec.av_parser_init(...)' ---")
local parser = avcodec.av_parser_init{ret = hffi.valuePtr(void), hffi.value(int, stru_codec.id)}
check_ptr(parser, "Parser not found\n")

-- 5, c = avcodec_alloc_context3(codec);
print("--- 5, start call 'avcodec.avcodec_alloc_context3(...)' ---")
local c = avcodec.avcodec_alloc_context3 {ret = hffi.valuePtr(void), hffi.valuePtr(stru_codec)}
check_ptr(parser, "Could not allocate video codec context\n")

-- struct AVCodecContext
local stru_context = ffmpeg_structs.avcontext
c.as(stru_context)

-- 6, avcodec_open2(...)
-- if (avcodec_open2(c, codec, NULL) < 0) ...
print("--- 6, start call 'avcodec.avcodec_open2(...)' ---")
if(avcodec.avcodec_open2{ret = hffi.value(int), c, codec}.get() < 0) then
	print("Could not open codec\n")	
	os.exit(1)	
end

-- 7, f = fopen(filename, "rb");
print("--- 7, start call 'c_runtime.fopen(...)' ---")
local f = c_runtime.fopen {ret = hffi.valuePtr(void), filename, "rb"} 
check_ptr(f, "Could not open "..filename)

-- 8, frame = av_frame_alloc();
print("--- 8, start call 'avutil.av_frame_alloc(...)' ---")
local frame = avutil.av_frame_alloc({ret = hffi.valuePtr(void)})
check_ptr(frame, "Could not allocate video frame\n")
local stru_frame = ffmpeg_structs.avframe;
frame.as(stru_frame)


local function pgm_save(buf, linesize, xsize, ysize, filename)
	local f = c_runtime.fopen{ret = hffi.valuePtr(void); filename, "wb"}	
	c_runtime.fprintf {ret = int; var_count = 3; f, "P5\n%d %d\n%d\n", xsize, ysize, hffi.value(int, 255)}
	local val_c = hffi.value(size_t, 0);
	local val_1 = hffi.value(size_t, 1);
	local val_xsize = hffi.value(size_t, xsize.get());
	local data = hffi.valuePtr(void)
	local arr_buf = buf.get();
	for i = 1, ysize.get(), 1 do
		data.setPtr(arr_buf, (i - 1) * linesize.get())
		-- size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
		c_runtime.fwrite{ret = val_c; data, val_1, val_xsize, f}
	end
	c_runtime.fclose{f}
end

-- static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, const char *filename)
local function decode(c, stru_context, frame, stru_frame, stru_pkt, filename)
	print("--- decode ---")
	--TODO 
	local val_ret = avcodec.avcodec_send_packet({ret = int; c, hffi.valuePtr(stru_pkt)})
	print("avcodec.avcodec_send_packet: result = ", val_ret)
	check_state(val_ret.get() < 0, "Error sending a packet for decoding\n");
	
	local arr_buf = hffi.array(sint8, 1024)
	local buf = hffi.valuePtr(arr_buf)
	
	local val_width = hffi.value(int, 0)
	local val_height = hffi.value(int, 0)
	local val_frame_num = hffi.value(int, 0)
	local val_linesize0 = hffi.value(int, 0)

	while(val_ret.get() >= 0) do
		avcodec.avcodec_receive_frame {ret = val_ret; c, frame}
		if(val_ret.get() == EAGAIN) then
			return
		end
		-- TODO latter handle EOF. (  FFERRTAG( 'E','O','F',' ') )
		if(val_ret.get() == AVERROR_EOF) then 
			return
		end
		check_state(val_ret.get() < 0, "Error during decoding\n");
		frame.as(stru_frame)
		c.as(stru_context)
		-- sync width and height
		val_width.set(stru_frame.width)
		val_height.set(stru_frame.height)
		val_frame_num.set(stru_context.frame_number)
		val_linesize0.set(stru_frame.linesize[0])
		
		c_runtime.printf{var_count = 1; "saving frame %3d\n", val_frame_num}
		c_runtime.fflush{stdout}
		c_runtime.snprintf{var_count = 2; buf, 1024, "%s-%d", filename, val_frame_num}
		--print("saving frame: ", stru_context.frame_number)
		--print(filename.."-"..stru_context.frame_number)
		pgm_save(stru_frame.data[0], val_linesize0, 
			val_width, val_height, buf)
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
	if(_ret ~= 0) then
		print("c_runtime.feof:  end.")
		break;
	end
	print("c_runtime.feof: "..tostring(_ret))
	
	--data_size = fread(inbuf, 1, INBUF_SIZE, f);
	c_runtime.fread{ret = data_size ; hffi.valuePtr(inbuf), hffi.value(uint, 1), hffi.value(uint, INBUF_SIZE), f}
	print("c_runtime.fread: ", data_size)
	if data_size.get() == 0 then
		break;
	end
	-- data = inbuf
	offset = 0;
	data.setPtr(inbuf, offset)
	while(data_size.get() > 0) do
		--  ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
        --                           data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0); 
		print("--- avcodec.av_parser_parse2 ---")
		_ret = avcodec.av_parser_parse2({ret = val_int; parser, c, pkt_data_ptr, pkt_size_ptr, 
				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, ZERO}).get()
		if(_ret < 0) then 
			print("Error while parsing\n")
			os.exit(1)
		end	
		-- sync ctx
		c.as(stru_context)
		-- reset pkt size.
		stru_pkt.size = pkt_size_ptr.get()	
		--  data      += ret;	data_size -= ret; 	
		offset = offset + _ret;
		data_size.add(-_ret)
		data.setPtr(inbuf, offset)
		
		-- if (pkt->size) decode(c, frame, pkt, outfilename);
		if(pkt_size_ptr.get() ~= 0) then
			decode(c, stru_context, frame, stru_frame, stru_pkt, outfilename);
		end
	end
end
 -- /* flush the decoder */
decode(c, stru_context, frame, stru_frame, nil, outfilename);
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

avutil.av_frame_free({frame.ptrValue()})
frame.ptrToNull()

avcodec.av_packet_free({pkt.ptrValue()})
pkt.ptrToNull()
stru_pkt.ptrToNull()

hffi.undefines();
print("-------- end test_ffmpeg ---------");