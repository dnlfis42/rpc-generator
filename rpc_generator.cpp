#include "rpc_generator.h"

#include "ltf_parser.h"

#include <cstdio>

#include <stdexcept>

namespace rpc
{
	Creation creation;
	Namespace ns;
	TryCatch try_catch;
	Logger logger;
	Profiler profiler;
	Dispatch dispatch;
	SerialBuffer serial_buffer;
	Session session;
	Protocol protocol;

	FileInfo ssh; // server_stub_header
	FileInfo ssc; // server_stub_cpp
	FileInfo sph; // server_proxy_header
	FileInfo spc; // server_proxy_cpp
	FileInfo csh; // client_stub_header
	FileInfo csc; // client_stub_cpp
	FileInfo cph; // client_proxy_header
	FileInfo cpc; // client_proxy_cpp

	bool set_config();
	bool create_file();

	void parse();

	bool parse_line(const wchar_t* line);
	bool parse_protocol_type(const wchar_t* line);
	bool parse_protocol(const wchar_t* line);

	int until(const wchar_t* line, const wchar_t* token);
	int skip(const wchar_t* line, const wchar_t* token);

	void print_header(FileInfo& header, FileInfo& cpp, int s_c, int s_p);
	void print_namespace(FileInfo& header, FileInfo& cpp, int s_c);
	void print_class(FileInfo& header, int s_p);
	void print_dispatch_stub(FileInfo& header, FileInfo& cpp);
	void print_protocol();
	void print_protocol_stub(FileInfo& header, FileInfo& cpp);
	void print_protocol_proxy(FileInfo& header, FileInfo& cpp);
	void print_close();
	void print_close_stub(FileInfo& header, FileInfo& cpp);
	void print_close_proxy(FileInfo& header, FileInfo& cpp);
	void print_tab(FileInfo& info);

	void generate()
	{
		if (set_config() && create_file())
		{
			parse();
		}
	}

	bool set_config()
	{
		try
		{
			auto config = ltf::parse(L"rpc/config.txt");

			do
			{
				if (!config.fill(L"file.creation.server_side", &creation.server_side))
				{
					wprintf(L"failed fill file.creation.server_side\n");
					break;
				}

				if (!config.fill(L"file.creation.client_side", &creation.client_side))
				{
					wprintf(L"failed fill file.creation.client_side\n");
					break;
				}

				if (!config.fill(L"file.name.prefix.enable_global", &creation.global_prefix))
				{
					wprintf(L"failed fill file.name.prefix.enable_global\n");
					break;
				}

				if (!config.fill(L"file.name.prefix.enable_side", &creation.side_prefix))
				{
					wprintf(L"failed fill file.name.prefix.enable_side\n");
					break;
				}

				if (!config.fill(L"namespace.enable", &ns.enable))
				{
					wprintf(L"failed fill namespace.enable\n");
					break;
				}

				if (ns.enable)
				{
					if (!config.fill(L"namespace.global", ns.global, BUFFER_SIZE))
					{
						wprintf(L"failed fill namespace.global\n");
						break;
					}
				}

				if (!config.fill(L"namespace.side.enable", &ns.enable_side))
				{
					wprintf(L"failed fill namespace.side.enable\n");
					break;
				}

				if (!config.fill(L"dispatch.profiler.enable", &profiler.enable))
				{
					wprintf(L"failed fill dispatch.profiler.enable\n");
					break;
				}

				if (profiler.enable)
				{
					if (!config.fill(L"dispatch.profiler.header", profiler.header, BUFFER_SIZE))
					{
						wprintf(L"failed fill dispatch.profiler.header\n");
						break;
					}

					if (!config.fill(L"dispatch.profiler.expression", profiler.expression, BUFFER_SIZE))
					{
						wprintf(L"failed fill dispatch.profiler.expression\n");
						break;
					}
				}

				if (!config.fill(L"dispatch.try-catch.enable", &try_catch.enable))
				{
					wprintf(L"failed fill dispatch.try-catch.enable\n");
					break;
				}

				if (try_catch.enable)
				{
					if (!config.fill(L"dispatch.try-catch.type", try_catch.type, BUFFER_SIZE))
					{
						wprintf(L"failed fill dispatch.try-catch.type\n");
						break;
					}
				}

				if (!config.fill(L"dispatch.log.enable", &logger.enable))
				{
					wprintf(L"failed fill dispatch.log.enable\n");
					break;
				}

				if (logger.enable)
				{
					if (!config.fill(L"dispatch.log.header", logger.header, BUFFER_SIZE))
					{
						wprintf(L"failed fill dispatch.log.header\n");
						break;
					}
				}

				if (!config.fill(L"dispatch.log.weird type.enable", &dispatch.enable_weird_type))
				{
					wprintf(L"failed fill dispatch.log.weird type.enable\n");
					break;
				}

				if (dispatch.enable_weird_type)
				{
					if (!config.fill(L"dispatch.log.weird type.expression", dispatch.weird_type, BUFFER_SIZE))
					{
						wprintf(L"failed fill dispatch.log.weird type.expression\n");
						break;
					}
				}

				if (!config.fill(L"dispatch.log.exception.enable", &dispatch.enable_exception))
				{
					wprintf(L"failed fill dispatch.log.exception.enable\n");
					break;
				}

				if (dispatch.enable_exception)
				{
					if (!config.fill(L"dispatch.log.exception.expression", dispatch.exception, BUFFER_SIZE))
					{
						wprintf(L"failed fill dispatch.log.exception.expression\n");
						break;
					}
				}

				if (!config.fill(L"serial buffer.header", serial_buffer.header, BUFFER_SIZE))
				{
					wprintf(L"failed fill serial buffer.header\n");
					break;
				}

				if (!config.fill(L"serial buffer.type", serial_buffer.type, BUFFER_SIZE))
				{
					wprintf(L"failed fill serial buffer.type\n");
					break;
				}

				if (!config.fill(L"serial buffer.parameter", serial_buffer.param, BUFFER_SIZE))
				{
					wprintf(L"failed fill serial buffer.parameter\n");
					break;
				}

				if (!config.fill(L"session.header", session.header, BUFFER_SIZE))
				{
					wprintf(L"failed fill session.header\n");
					break;
				}

				if (!config.fill(L"session.type", session.type, BUFFER_SIZE))
				{
					wprintf(L"failed fill session.type\n");
					break;
				}

				if (!config.fill(L"session.parameter", session.param, BUFFER_SIZE))
				{
					wprintf(L"failed fill session.parameter\n");
					break;
				}

				if (!config.fill(L"session.header.prefix.enable_global", &session.global_prefix))
				{
					wprintf(L"failed fill session.header.prefix.enable_global\n");
					break;
				}

				if (!config.fill(L"session.header.prefix.enable_side", &session.side_prefix))
				{
					wprintf(L"failed fill session.header.prefix.enable_side\n");
					break;
				}

				return true;
			} while (false);

			return false;
		}
		catch (std::runtime_error& e)
		{
			wprintf(L"error caught: %hs\n", e.what());
			return false;
		}
	}

	bool create_file()
	{
		FileInfo* file_info[2][2][2] =
		{
			{{&ssh,&ssc},
			{&sph,&spc}},
			{{&csh,&csc},
			{&cph,&cpc}}
		};

		for (int s_c = 0; s_c < 2; ++s_c)
		{
			if (s_c == SERVER && !creation.server_side)
			{
				continue;
			}

			if (s_c == CLIENT && !creation.client_side)
			{
				continue;
			}

			for (int s_p = 0; s_p < 2; ++s_p)
			{
				FileInfo* head = file_info[s_c][s_p][HEADER];
				FileInfo* cpp = file_info[s_c][s_p][CPP];

				head->s_c = s_c;
				cpp->s_c = s_c;

				head->s_p = s_p;
				cpp->s_p = s_p;

				head->h_c = HEADER;
				cpp->h_c = CPP;

				head->filename[0] = L'\0';
				cpp->filename[0] = L'\0';

				if (creation.global_prefix)
				{
					wcscat_s(head->filename, FILENAME_MAX, ns.global);
					wcscat_s(head->filename, FILENAME_MAX, L"_");
					wcscat_s(cpp->filename, FILENAME_MAX, ns.global);
					wcscat_s(cpp->filename, FILENAME_MAX, L"_");
				}

				if (creation.side_prefix)
				{
					wcscat_s(head->filename, FILENAME_MAX, RPC_SIDE[s_c]);
					wcscat_s(head->filename, FILENAME_MAX, L"_");
					wcscat_s(cpp->filename, FILENAME_MAX, RPC_SIDE[s_c]);
					wcscat_s(cpp->filename, FILENAME_MAX, L"_");
				}

				wcscat_s(head->filename, FILENAME_MAX, RPC_PART[s_p]);
				wcscat_s(cpp->filename, FILENAME_MAX, RPC_PART[s_p]);

				wcscat_s(head->filename, FILENAME_MAX, EXTENSION[HEADER]);
				wcscat_s(cpp->filename, FILENAME_MAX, EXTENSION[CPP]);

				if (_wfopen_s(&head->file, head->filename, L"wt, ccs=UNICODE") != 0 ||
					head->file == nullptr)
				{
					return false;
				}

				if (_wfopen_s(&cpp->file, cpp->filename, L"wt, ccs=UNICODE") != 0 ||
					cpp->file == nullptr)
				{
					return false;
				}

				print_header(*head, *cpp, s_c, s_p);

				if (ns.enable || ns.enable_side)
				{
					print_namespace(*head, *cpp, s_c);
				}

				print_class(*head, s_p);

				if (s_p == STUB)
				{
					print_dispatch_stub(*head, *cpp);
				}
			}
		}

		return true;
	}

	void parse()
	{
		FILE* file = nullptr;
		if (_wfopen_s(&file, L"rpc/protocol.txt", L"rt, ccs=UNICODE") != 0 || file == nullptr)
		{
			wprintf(L"failed to open ./rpc/protocol.txt\n");
			return;
		}

		wchar_t line[FILENAME_MAX];
		while (fgetws(line, FILENAME_MAX, file))
		{
			if (!parse_line(line))
			{
				break;
			}
		}

		print_close();

		fclose(file);
	}

	bool parse_line(const wchar_t* line)
	{
		int pos = skip(line, L" \t\r\n");
		if (pos == -1)
		{
			return true;
		}

		if (wcsncmp(line + pos, L"TYPE", 4) == 0)
		{
			return parse_protocol_type(line + pos + 4);
		}
		else
		{
			return parse_protocol(line + pos);
		}

		return false;
	}

	bool parse_protocol_type(const wchar_t* line)
	{
		int pos = skip(line, L" \t=");
		if (pos == -1)
		{
			return false;
		}

		int len = until(line + pos, L" \t\r\n");
		if (len == 0)
		{
			return false;
		}

		protocol.protocol_type = _wtoi(line + pos);

		return true;
	}

	bool parse_protocol(const wchar_t* line)
	{
		int pos = skip(line, L" \t");
		if (pos == -1)
		{
			return false;
		}

		int len = until(line + pos, L"(");
		if (len == 0)
		{
			return false;
		}

		int copy_len = min(len, BUFFER_SIZE - 1);
		wcsncpy_s(protocol.name, BUFFER_SIZE, line + pos, copy_len);
		protocol.name[copy_len] = L'\0';

		for (int i = 0; i <= copy_len; ++i)
		{
			protocol.lower_name[i] = towlower(protocol.name[i]);
		}

		pos = pos + len + 1; // ( 다음을 가리킴

		int end_pos = pos + until(line + pos, L")");

		while (pos < end_pos && protocol.param_cnt < MAX_PARAMETER_CNT)
		{
			while (line[pos] == L' ' || line[pos] == L'\t')
			{
				++pos;
			}

			len = until(line + pos, L",)");

			while (line[pos + len - 1] == L' ' || line[pos + len - 1] == L'\t')
			{
				--len;
			}

			// pos = 정확히 타입 시작 인덱스, len = 타입 + 이름의 길이

			int name_len = 1;
			while (line[pos + len - name_len] != L' ')
			{
				++name_len;
			}

			// name_len 은 스페이스바를 포함한 길이

			int type_len = len - name_len;

			wcsncpy_s(protocol.param_type[protocol.param_cnt], BUFFER_SIZE, line + pos, type_len);

			pos = pos + type_len + 1;

			wcsncpy_s(protocol.param_name[protocol.param_cnt], BUFFER_SIZE, line + pos, name_len - 1);

			len = until(line + pos, L",)");

			pos = pos + len + 1;

			++protocol.param_cnt;
		}

		print_protocol();

		++protocol.protocol_type;

		protocol.param_cnt = 0;

		return true;
	}

	int until(const wchar_t* line, const wchar_t* token)
	{
		int token_cnt = static_cast<int>(wcslen(token));
		int len = 0;

		while (line[len])
		{
			bool matched = false;

			for (int i = 0; i < token_cnt; ++i)
			{
				if (line[len] == token[i])
				{
					matched = true;
					break;
				}
			}

			if (matched)
			{
				break;
			}

			++len;
		}

		return len;
	}

	int skip(const wchar_t* line, const wchar_t* token)
	{
		int token_cnt = static_cast<int>(wcslen(token));
		int len = 0;

		while (line[len])
		{
			bool matched = false;

			for (int i = 0; i < token_cnt; ++i)
			{
				if (line[len] == token[i])
				{
					matched = true;
					++len;
					break;
				}
			}

			if (!matched)
			{
				break;
			}
		}

		return line[len] == L'\0' ? -1 : len;
	}

	void print_header(FileInfo& header, FileInfo& cpp, int s_c, int s_p)
	{
		fwprintf(header.file, L"#pragma once\n\n");
		fwprintf(cpp.file, L"#include \"%s\"\n\n", header.filename);

		if (s_p == STUB)
		{
			fwprintf(header.file, L"#include \"");
			fwprintf(cpp.file, L"#include \"");

			if (ns.enable || session.global_prefix)
			{
				fwprintf(header.file, L"%s_", ns.global);
				fwprintf(cpp.file, L"%s_", ns.global);
			}

			if (session.side_prefix)
			{
				fwprintf(header.file, L"%s_", RPC_SIDE[s_c]);
				fwprintf(cpp.file, L"%s_", RPC_SIDE[s_c]);
			}

			fwprintf(header.file, L"%s\"\n\n", session.header);
			fwprintf(cpp.file, L"%s\"\n\n", session.header);

			if (logger.enable)
			{
				fwprintf(cpp.file, L"#include \"%s\"\n", logger.header);
			}

			if (profiler.enable)
			{
				fwprintf(cpp.file, L"#include \"%s\"\n", profiler.header);
			}
		}

		fwprintf(header.file, L"#include \"%s\"\n", serial_buffer.header);
		fwprintf(cpp.file, L"#include \"%s\"\n", serial_buffer.header);

		if (s_p == PROXY)
		{
			fwprintf(header.file, L"\n#define WIN32_LEAN_AND_MEAN\n#include <Windows.h>\n");
			fwprintf(cpp.file, L"\n#define WIN32_LEAN_AND_MEAN\n#include <Windows.h>\n");
		}

		if (try_catch.enable)
		{
			fwprintf(cpp.file, L"\n#include <stdexcept>\n");
		}

		fwprintf(header.file, L"\n");
		fwprintf(cpp.file, L"\n");
	}

	void print_namespace(FileInfo& header, FileInfo& cpp, int s_c)
	{
		if (ns.enable)
		{
			fwprintf(header.file, L"namespace %s\n", ns.global);
			fwprintf(cpp.file, L"namespace %s\n", ns.global);

			fwprintf(header.file, L"{\n");
			fwprintf(cpp.file, L"{\n");

			++header.tab_cnt;
			++cpp.tab_cnt;

			if (ns.enable_side)
			{
				print_tab(header);
				fwprintf(header.file, L"namespace %s\n", RPC_SIDE[s_c]);

				print_tab(cpp);
				fwprintf(cpp.file, L"namespace %s\n", RPC_SIDE[s_c]);

				print_tab(header);
				fwprintf(header.file, L"{\n");

				print_tab(cpp);
				fwprintf(cpp.file, L"{\n");

				++header.tab_cnt;
				++cpp.tab_cnt;
			}
		}
		else if (ns.enable_side)
		{
			fwprintf(header.file, L"namespace %s\n", RPC_SIDE[s_c]);
			fwprintf(cpp.file, L"namespace %s\n", RPC_SIDE[s_c]);

			fwprintf(header.file, L"{\n");
			fwprintf(cpp.file, L"{\n");

			++header.tab_cnt;
			++cpp.tab_cnt;
		}
	}

	void print_class(FileInfo& header, int s_p)
	{
		print_tab(header);
		fwprintf(header.file, L"class %s\n", RPC_CLASS[s_p]);

		print_tab(header);
		fwprintf(header.file, L"{\n");

		print_tab(header);
		fwprintf(header.file, L"public:\n");

		++header.tab_cnt;

		if (s_p == STUB)
		{
			print_tab(header);
			fwprintf(header.file, L"virtual ~%s() {}\n\n", RPC_CLASS[s_p]);
		}
	}

	void print_dispatch_stub(FileInfo& header, FileInfo& cpp)
	{
		print_tab(header);
		fwprintf(header.file, L"bool dispatch(%s* %s, unsigned char type, %s* %s);\n\n",
			session.type, session.param, serial_buffer.type, serial_buffer.param);

		print_tab(cpp);
		fwprintf(cpp.file, L"bool %s::dispatch(%s* %s, unsigned char type, %s* %s)\n",
			RPC_CLASS[STUB], session.type, session.param, serial_buffer.type, serial_buffer.param);

		print_tab(cpp);
		fwprintf(cpp.file, L"{\n");

		++cpp.tab_cnt;

		if (profiler.enable)
		{
			print_tab(cpp);
			fwprintf(cpp.file, L"%s\n\n", profiler.expression);
		}

		if (try_catch.enable)
		{
			print_tab(cpp);
			fwprintf(cpp.file, L"try\n");

			print_tab(cpp);
			fwprintf(cpp.file, L"{\n");

			++cpp.tab_cnt;
		}

		print_tab(cpp);
		fwprintf(cpp.file, L"switch (type)\n");

		print_tab(cpp);
		fwprintf(cpp.file, L"{\n");
	}

	void print_protocol()
	{
		if (protocol.name[0] == L'S') // server proxy, client stub에서 사용
		{
			if (creation.server_side)
			{
				print_protocol_proxy(sph, spc);
			}

			if (creation.client_side)
			{
				print_protocol_stub(csh, csc);
			}
		}
		else // L'C' -> server stub, client proxy에서 사용
		{
			if (creation.server_side)
			{
				print_protocol_stub(ssh, ssc);
			}

			if (creation.client_side)
			{
				print_protocol_proxy(cph, cpc);
			}
		}
	}

	void print_protocol_stub(FileInfo& header, FileInfo& cpp)
	{
		print_tab(header);
		fwprintf(header.file, L"virtual bool %s(%s* %s, %s* %s) = 0;\n",
			protocol.lower_name, session.type, session.param, serial_buffer.type, serial_buffer.param);

		print_tab(cpp);
		fwprintf(cpp.file, L"case %d:\n", protocol.protocol_type);

		print_tab(cpp);
		fwprintf(cpp.file, L"\treturn %s(%s, %s);\n",
			protocol.lower_name, session.param, serial_buffer.param);
	}

	void print_protocol_proxy(FileInfo& header, FileInfo& cpp)
	{
		print_tab(header);
		fwprintf(header.file, L"void mp_%s(%s* %s, ",
			protocol.lower_name, serial_buffer.type, serial_buffer.param);

		print_tab(cpp);
		fwprintf(cpp.file, L"void %s::mp_%s(%s* %s, ",
			RPC_CLASS[PROXY], protocol.lower_name, serial_buffer.type, serial_buffer.param);

		for (int i = 0; i < protocol.param_cnt; ++i)
		{
			fwprintf(header.file, L"%s %s", protocol.param_type[i], protocol.param_name[i]);
			fwprintf(cpp.file, L"%s %s", protocol.param_type[i], protocol.param_name[i]);

			if (i != protocol.param_cnt - 1)
			{
				fwprintf(header.file, L", ");
				fwprintf(cpp.file, L", ");
			}
		}
		fwprintf(header.file, L");\n");
		fwprintf(cpp.file, L")\n");

		print_tab(cpp);
		fwprintf(cpp.file, L"{\n");

		++cpp.tab_cnt;

		print_tab(cpp);
		fwprintf(cpp.file, L"unsigned char code = %s;\n", CODE_VALUE);

		print_tab(cpp);
		fwprintf(cpp.file, L"unsigned char size = 0;\n");

		print_tab(cpp);
		fwprintf(cpp.file, L"unsigned char type = %d;\n", protocol.protocol_type);

		print_tab(cpp);
		fwprintf(cpp.file, L"%s->clear();\n", serial_buffer.param);

		print_tab(cpp);
		fwprintf(cpp.file, L"*%s << code << size << type << ", serial_buffer.param);

		for (int i = 0; i < protocol.param_cnt; ++i)
		{
			fwprintf(cpp.file, L"%s", protocol.param_name[i]);

			if (i != protocol.param_cnt - 1)
			{
				fwprintf(cpp.file, L" << ");
			}
		}

		fwprintf(cpp.file, L";\n");

		print_tab(cpp);
		fwprintf(cpp.file, L"reinterpret_cast<unsigned char*>(%s->read_pos())[1] = static_cast<unsigned char>(%s->size() - 3);\n",
			serial_buffer.param, serial_buffer.param);

		--cpp.tab_cnt;

		print_tab(cpp);
		fwprintf(cpp.file, L"}\n\n");
	}

	void print_close()
	{
		if (creation.server_side)
		{
			print_close_stub(ssh, ssc);
			print_close_proxy(sph, spc);
		}

		if (creation.client_side)
		{
			print_close_stub(csh, csc);
			print_close_proxy(cph, cpc);
		}
	}

	void print_close_stub(FileInfo& header, FileInfo& cpp)
	{
		--header.tab_cnt;
		print_tab(header);
		fwprintf(header.file, L"};\n"); // class

		if (ns.enable_side)
		{
			--header.tab_cnt;
			print_tab(header);
			fwprintf(header.file, L"}");

			if (ns.enable)
			{
				fwprintf(header.file, L"\n");

				--header.tab_cnt;
				print_tab(header);
				fwprintf(header.file, L"}");
			}
		}
		else if (ns.enable)
		{
			--header.tab_cnt;
			print_tab(header);
			fwprintf(header.file, L"}");
		}

		// cpp

		print_tab(cpp);
		fwprintf(cpp.file, L"default:\n");

		++cpp.tab_cnt;
		if (logger.enable && dispatch.enable_weird_type)
		{
			print_tab(cpp);
			fwprintf(cpp.file, L"%s\n", dispatch.weird_type);
		}

		print_tab(cpp);
		fwprintf(cpp.file, L"return false;\n");

		--cpp.tab_cnt;

		print_tab(cpp);
		fwprintf(cpp.file, L"}\n"); // switch

		if (try_catch.enable)
		{
			--cpp.tab_cnt;

			print_tab(cpp);
			fwprintf(cpp.file, L"}\n"); // try

			print_tab(cpp);
			fwprintf(cpp.file, L"catch (const %s& e)\n", try_catch.type);

			print_tab(cpp);
			fwprintf(cpp.file, L"{\n");

			++cpp.tab_cnt;

			if (logger.enable && dispatch.enable_exception)
			{
				print_tab(cpp);
				fwprintf(cpp.file, L"%s\n", dispatch.exception);
			}

			print_tab(cpp);
			fwprintf(cpp.file, L"return false;\n");

			--cpp.tab_cnt;
			print_tab(cpp);
			fwprintf(cpp.file, L"}\n"); // catch
		}

		--cpp.tab_cnt;
		print_tab(cpp);
		fwprintf(cpp.file, L"}\n"); // dispatch

		if (ns.enable_side)
		{
			--cpp.tab_cnt;
			print_tab(cpp);
			fwprintf(cpp.file, L"}");

			if (ns.enable)
			{
				fwprintf(cpp.file, L"\n");

				--cpp.tab_cnt;
				print_tab(cpp);
				fwprintf(cpp.file, L"}");
			}
		}
		else if (ns.enable)
		{
			--cpp.tab_cnt;
			print_tab(cpp);
			fwprintf(cpp.file, L"}");
		}
	}

	void print_close_proxy(FileInfo& header, FileInfo& cpp)
	{
		--header.tab_cnt;
		print_tab(header);
		fwprintf(header.file, L"};\n"); // class

		if (ns.enable_side)
		{
			--header.tab_cnt;
			print_tab(header);
			fwprintf(header.file, L"}");

			--cpp.tab_cnt;
			print_tab(cpp);
			fwprintf(cpp.file, L"}");

			if (ns.enable)
			{
				fwprintf(header.file, L"\n");

				--header.tab_cnt;
				print_tab(header);
				fwprintf(header.file, L"}");

				fwprintf(cpp.file, L"\n");

				--cpp.tab_cnt;
				print_tab(cpp);
				fwprintf(cpp.file, L"}");
			}
		}
		else if (ns.enable)
		{
			--header.tab_cnt;
			print_tab(header);
			fwprintf(header.file, L"}");

			--cpp.tab_cnt;
			print_tab(cpp);
			fwprintf(cpp.file, L"}");
		}
	}

	void print_tab(FileInfo& info)
	{
		for (int i = 0; i < info.tab_cnt; ++i)
		{
			fwprintf(info.file, L"\t");
		}
	}
}

int main()
{
	rpc::generate();

	return 0;
}