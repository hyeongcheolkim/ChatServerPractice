pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./Chat.proto