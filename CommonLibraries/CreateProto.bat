pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./ClientRequest.proto