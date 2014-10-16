rm ./enc

gcc example_pcm2amr.c ../codec/audio_format_convert.c ../codec/amrnb.c ../codec/bs.h -o enc -I'../opencore-amr-0.1.3/x86/include' -I'../codec' -L'../opencore-amr-0.1.3/x86/lib' -lopencore-amrnb

cp '../opencore-amr-0.1.3/x86/lib/libopencore-amrnb.so.0.0.3' './libopencore-amrnb.so.0'

