hffi.defines();
local ffmpeg_structs = require("ffmpeg_structs")

local lib_Dir ="D:/study/tools/ffmpeg/gcc-win64-lgpl-shared-4.4/bin/"
local avcodec_path = "avcodec-58";
local avutil_path = "avutil-56";
local avformat_path = "avformat-58";

local avcodec = hffi.loadLib(lib_Dir, avcodec_path)
local avutil = hffi.loadLib(lib_Dir, avutil_path)
local avformat = hffi.loadLib(lib_Dir, avformat_path)
-- params
local input_file = ""; --todo
-------------------

avcodec.av_register_all{}
avformat.avformat_network_init {}
local pFormatCtx = avformat.avformat_alloc_context{}
avformat.avformat_open_input{}
--[[
if(avformat_open_input(&pFormatCtx,input_str,NULL,NULL)!=0){
LOGE("Couldn't open input stream.\n");
return -1;
}]]
local result = avformat.avformat_open_input {ret = int; pFormatCtx.ptrValue(), input_file, nil, nil}
if(result.get() ~= 0)then
    print("open file failed. ", input_file)
    os.exit(1);
end

--[[
if(avformat_find_stream_info(pFormatCtx,NULL)<0){
    LOGE("Couldn't find stream information.\n");
return -1;
}]]

avformat.avformat_find_stream_info{ret = result, pFormatCtx.ptrValue(), nil}
if(result.get() < 0) then
    print("avformat_find_stream_info failed. file is ", input_file)
    os.exit(1);
end
local videoindex = -1;
pFormatCtx.as(ffmpeg_structs.avframeCtx);
--[[
for(i=0; i<pFormatCtx->nb_streams; i++)
if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
videoindex=i;
break;
}]]
local nb_streams = ffmpeg_structs.avframeCtx.nb_streams;
--     struct AVStream** streams;
for i = 1, nb_streams do

end
