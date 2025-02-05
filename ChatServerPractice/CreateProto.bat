pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./ChatMessage.proto
protoc.exe -I=./ --cpp_out=./ ./ServerResponse.proto