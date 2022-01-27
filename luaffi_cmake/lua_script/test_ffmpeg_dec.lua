hffi.defines();
local ffmpeg_structs = require("ffmpeg_structs")

local lib_Dir ="D:/study/tools/ffmpeg/gcc-win64-lgpl-shared-4.4/bin/"
local avcodec_path = "avcodec-58";
local avutil_path = "avutil-56";
local avformat_path = "avformat-58";

local avcodec = hffi.loadLib(lib_Dir, avcodec_path)
local avutil = hffi.loadLib(lib_Dir, avutil_path)
local avformat = hffi.loadLib(lib_Dir, avformat_path)

avcodec.av_register_all{}
avformat.avformat_network_init {}
local pFormatCtx = avformat.avformat_alloc_context{}
avformat.avformat_open_input{}
