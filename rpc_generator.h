#pragma once

#include <cstdio>

namespace rpc
{
	constexpr int BUFFER_SIZE = 64;

	constexpr int MAX_PARAMETER_CNT = 16;

	constexpr int SERVER = 0;
	constexpr int CLIENT = 1;
	constexpr wchar_t RPC_SIDE[2][8] = { L"server",L"client" };

	constexpr int STUB = 0;
	constexpr int PROXY = 1;
	constexpr wchar_t RPC_PART[2][8] = { L"stub",L"proxy" };
	constexpr wchar_t RPC_CLASS[2][8] = { L"Stub",L"Proxy" };

	constexpr int HEADER = 0;
	constexpr int CPP = 1;
	constexpr wchar_t EXTENSION[2][8] = { L".h",L".cpp" };

	constexpr wchar_t CODE_VALUE[] = L"0x89";

	struct Creation
	{
		bool server_side;
		bool client_side;
		bool global_prefix;
		bool side_prefix;
	};

	struct Namespace
	{
		wchar_t global[BUFFER_SIZE];
		bool enable;
		bool enable_side;
	};

	struct TryCatch
	{
		wchar_t type[BUFFER_SIZE];
		bool enable;
	};

	struct Logger
	{
		wchar_t header[BUFFER_SIZE];
		bool enable;
	};

	struct Profiler
	{
		wchar_t header[BUFFER_SIZE];
		wchar_t expression[BUFFER_SIZE];
		bool enable;
	};

	struct Dispatch
	{
		wchar_t weird_type[BUFFER_SIZE];
		wchar_t exception[BUFFER_SIZE];
		bool enable_weird_type;
		bool enable_exception;
	};

	struct SerialBuffer
	{
		wchar_t header[BUFFER_SIZE];
		wchar_t type[BUFFER_SIZE];
		wchar_t param[BUFFER_SIZE];
	};

	struct Session
	{
		wchar_t header[BUFFER_SIZE];
		wchar_t type[BUFFER_SIZE];
		wchar_t param[BUFFER_SIZE];
		bool global_prefix;
		bool side_prefix;
	};

	struct Protocol
	{
		wchar_t name[BUFFER_SIZE];
		wchar_t lower_name[BUFFER_SIZE];
		wchar_t param_type[MAX_PARAMETER_CNT][BUFFER_SIZE];
		wchar_t param_name[MAX_PARAMETER_CNT][BUFFER_SIZE];
		int protocol_type;
		int param_cnt;
	};

	struct FileInfo
	{
		wchar_t filename[FILENAME_MAX]{};
		FILE* file = nullptr;
		int tab_cnt = 0;
		int s_c = -1;
		int s_p = -1;
		int h_c = -1;
	};

	void generate();
}