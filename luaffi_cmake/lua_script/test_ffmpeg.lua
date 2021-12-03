-- for gcc/vc c-enum is 4 size.
-- : https://github.com/FFmpeg/FFmpeg/blob/release%2F4.4/doc/examples/decode_video.c
print("-------- start test_ffmpeg---------");
hffi.defines();

local lib_Dir ="D:/study/tools/ffmpeg/gcc-win64-lgpl-shared-4.4/bin/"
local avcodec_path = "avcodec-58";

-- pkt = av_packet_alloc();
-- need AVPacket, AVBufferRef, AVBuffer. AVPacketSideData
local stru_pkt = hffi.struct{
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

local avcodec = hffi.loadLib(lib_Dir, avcodec_path)
local ret_alloc_pkt = hffi.value(pointer, void, true) -- no data
print("ret_alloc_pkt.addr():", ret_alloc_pkt.addr())
local pkt = avcodec.av_packet_alloc {ret = ret_alloc_pkt}
print("pkt.addr():", pkt.addr())

hffi.undefines();
print("-------- end test_ffmpeg ---------");