#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace ltf
{
	class Document
	{
	private:
		static constexpr size_t WRONG_POS = -1;

	public:
		Document(wchar_t* src, size_t len) : buff_(src), size_(len)
		{
		}

		~Document()
		{
			delete buff_;
		}

	public:
		bool fill(const wchar_t* key, bool* value)
		{
			size_t value_pos = find(key);

			return check(value, value_pos);
		}

		bool fill(const wchar_t* key, int* value)
		{
			size_t value_pos = find(key);

			return check(value, value_pos);
		}

		bool fill(const wchar_t* key, double* value)
		{
			size_t value_pos = find(key);

			return check(value, value_pos);
		}

		bool fill(const wchar_t* key, wchar_t* value, size_t len)
		{
			size_t value_pos = find(key);

			return check(value, len, value_pos);
		}

	private:
		size_t find(const wchar_t* key)
		{
			wchar_t* section = nullptr;
			const wchar_t* real_key = nullptr;

			size_t key_len = wcslen(key);

			while (--key_len)
			{
				if (key[key_len] == L'.')
				{
					break;
				}
			}

			if (key_len > 0)
			{
				section = new wchar_t[key_len + 1];
				wcsncpy_s(section, key_len + 1, key, key_len);

				real_key = key + key_len + 1;
			}
			else
			{
				real_key = key;
			}

			size_t value_pos = find(section, real_key);

			if (section)
			{
				delete[] section;
			}

			return value_pos;
		}

		size_t find(const wchar_t* section, const wchar_t* key)
		{
			size_t pos = skip_ignorable(buff_);
			size_t cnt;

			if (pos == WRONG_POS)
			{
				return WRONG_POS;
			}

			if (section)
			{
				size_t section_len = wcslen(section);

				while (pos < size_)
				{
					cnt = skip(buff_ + pos, L"[");

					if (cnt == WRONG_POS)
					{
						return WRONG_POS;
					}

					pos += cnt;

					cnt = until(buff_ + pos, L"]");

					if (cnt == WRONG_POS)
					{
						return WRONG_POS;
					}

					if (section_len == cnt && wcsncmp(section, buff_ + pos, section_len) == 0)
					{
						pos = pos + cnt + 1;

						break;
					}
				}

				if (pos >= size_)
				{
					return WRONG_POS;
				}

				// 현재 pos는 유효공간 내부 (앞에 white space 붙어있을수도 없을수도 있음) -> section 파트 처리 때문에
				cnt = skip_ignorable(buff_ + pos);

				if (cnt == WRONG_POS)
				{
					return WRONG_POS;
				}

				pos += cnt;
			}

			size_t key_len = wcslen(key);

			// 현재 pos 는 유효한 키의 시작을 가리킴

			size_t limit = until(buff_ + pos, L"["); // 값이 나오면 유효 공간, WRONG_POS면 끝까지 봐야함
			size_t true_limit;

			if (limit == WRONG_POS)
			{
				true_limit = size_;
			}
			else
			{
				true_limit = pos + limit;
			}

			while (pos < true_limit)
			{
				// 유효 키 위치 도달

				cnt = until(buff_ + pos, L" \t");

				if (cnt == WRONG_POS)
				{
					break;
				}

				if (key_len == cnt && wcsncmp(key, buff_ + pos, cnt) == 0) // 찾음
				{
					pos += cnt; // 현재 key 다음 빈 공간

					cnt = skip(buff_ + pos, L"=");

					if (cnt == WRONG_POS)
					{
						break;
					}

					pos += cnt; // = 다음 공간

					cnt = skip_ignorable(buff_ + pos);

					if (cnt == WRONG_POS)
					{
						break;
					}

					pos += cnt; // value의 시작 인덱스

					return pos;
				}

				// 해당 키 라인이 아닌 경우

				pos += cnt; // 현재 key 다음 빈 공간

				cnt = skip(buff_ + pos, L"=");

				if (cnt == WRONG_POS)
				{
					break;
				}

				pos += cnt;

				cnt = skip_ignorable(buff_ + pos);

				if (cnt == WRONG_POS)
				{
					break;
				}

				pos += cnt; // value의 시작 인덱스

				cnt = skip_value(buff_ + pos);

				if (cnt == WRONG_POS)
				{
					break;
				}

				pos += cnt;

				cnt = skip_ignorable(buff_ + pos);

				if (cnt == WRONG_POS)
				{
					break;
				}

				pos += cnt;
			}

			return WRONG_POS;
		}

	private:
		size_t until(const wchar_t* str, const wchar_t* token)
		{
			auto token_cnt = wcslen(token);

			size_t pos = 0;

			while (str[pos] != L'\0')
			{
				for (size_t i = 0; i < token_cnt; ++i)
				{
					if (str[pos] == token[i])
					{
						return pos;
					}
				}

				++pos;
			}

			return WRONG_POS;
		}

		size_t skip(const wchar_t* str, const wchar_t* token)
		{
			size_t pos = until(str, token);

			if (pos == WRONG_POS)
			{
				return WRONG_POS;
			}
			else
			{
				return pos + 1;
			}
		}

		/**
		* @brief 돌려준 pos 위치 \0인지 확인 안함
		*/
		size_t skip_single_comment(const wchar_t* str)
		{
			size_t pos = until(str, L"\n");

			if (pos == WRONG_POS)
			{
				return WRONG_POS;
			}
			else
			{
				return pos + 1;
			}
		}

		/**
		* @brief 돌려준 pos 위치 \0인지 확인 안함
		*/
		size_t skip_multiple_comment(const wchar_t* str)
		{
			size_t pos = 0;

			while (str[pos] != L'\0')
			{
				size_t cnt = until(str + pos, L"*");

				if (cnt == WRONG_POS)
				{
					break;
				}

				pos += cnt; // 현재 *를 가리킴

				if (str[pos + 1] == L'/') // */에서 /를 찾았다면 / 다음을 돌려줌
				{
					pos += 2;

					return pos;
				}

				++pos;
			}

			return WRONG_POS;
		}

		size_t skip_ignorable(const wchar_t* str)
		{
			size_t pos = 0;

			size_t cnt;

			while (str[pos] != L'\0')
			{
				if (str[pos] == L' ' || str[pos] == L'\t' || str[pos] == L'\r' || str[pos] == L'\n')
				{
					++pos;
				}
				else if (str[pos] == L'/')
				{
					if (str[pos + 1] == L'/')
					{
						pos += 2;

						cnt = skip_single_comment(str + pos);

						if (cnt == WRONG_POS)
						{
							break;
						}
						else
						{
							pos += cnt;
						}
					}
					else if (str[pos + 1] == L'*')
					{
						pos += 2;

						cnt = skip_multiple_comment(str + pos);

						if (cnt == WRONG_POS)
						{
							break;
						}
						else
						{
							pos += cnt;
						}
					}
					else // 문제 상황
					{
						break;
					}
				}
				else
				{
					return pos;
				}
			}

			return WRONG_POS;
		}

		size_t skip_value(const wchar_t* str)
		{
			size_t pos = 0;

			size_t cnt;

			if (str[pos] == L'`')
			{
				pos++;

				cnt = until(str + pos, L"`");

				if (cnt == WRONG_POS)
				{
					return WRONG_POS;
				}

				return pos + cnt + 1;
			}
			else if (str[pos] == L'"')
			{
				pos++;

				cnt = until(str + pos, L"\"");

				if (cnt == WRONG_POS)
				{
					return WRONG_POS;
				}

				return pos + cnt + 1;
			}
			else
			{
				cnt = until(str + pos, L" \t\r\n/");

				if (cnt == WRONG_POS)
				{
					return WRONG_POS;
				}

				return pos + cnt;
			}
		}

	private:
		bool check(bool* value, size_t pos)
		{
			if (pos == WRONG_POS)
			{
				return false;
			}

			size_t value_len = until(buff_ + pos, L" \t\r\n/");

			if (value_len == WRONG_POS)
			{
				return false;
			}

			if (value_len == 4)
			{
				if (_wcsnicmp(L"true", buff_ + pos, 4) == 0)
				{
					*value = true;

					return true;
				}
			}
			else if (value_len == 5)
			{
				if (_wcsnicmp(L"false", buff_ + pos, 5) == 0)
				{
					*value = false;

					return true;
				}
			}

			return false;
		}

		bool check(int* value, size_t pos)
		{
			if (pos == WRONG_POS)
			{
				return false;
			}

			size_t value_len = until(buff_ + pos, L" \t\r\n/");

			if (value_len == 0)
			{
				return false;
			}
			else if (value_len == WRONG_POS)
			{
				value_len = size_ - pos;
			}

			bool is_minus = false;
			int base = 10;

			if (buff_[pos] == L'+')
			{
				pos++;
				value_len--;
			}
			else if (buff_[pos] == L'-')
			{
				is_minus = true;
				pos++;
				value_len--;
			}

			if (value_len == 0)
			{
				return false;
			}

			if (buff_[pos] == L'0')
			{
				if (value_len == 1)
				{
					*value = 0;

					return true;
				}

				pos++;
				value_len--;

				if (buff_[pos] >= L'0' && buff_[pos] < L'8')
				{
					base = 8;
				}
				else if (buff_[pos] == L'B' || buff_[pos] == L'b')
				{
					base = 2;
					pos++;
					value_len--;
				}
				else if (buff_[pos] == L'X' || buff_[pos] == L'x')
				{
					base = 16;
					pos++;
					value_len--;
				}
				else
				{
					return false;
				}
			}

			if (value_len == 0)
			{
				return false;
			}

			int result = wcsntoi(base, buff_ + pos, value_len);

			if (result == -1)
			{
				return false;
			}

			if (is_minus)
			{
				result *= -1;
			}

			*value = result;

			return true;
		}

		bool check(double* value, size_t pos)
		{
			if (pos == WRONG_POS)
			{
				return false;
			}

			size_t value_len = until(buff_ + pos, L" \t\r\n/");

			if (value_len == WRONG_POS || value_len == 0)
			{
				return false;
			}

			wchar_t* end;

			*value = wcstod(buff_ + pos, &end);

			return true;
		}

		bool check(wchar_t* value, size_t len, size_t pos)
		{
			if (pos == WRONG_POS)
			{
				return false;
			}

			size_t value_len = 0;
			size_t cnt;
			size_t copy_size;

			if (buff_[pos] == L'`')
			{
				pos++;

				value_len = until(buff_ + pos, L"`");
				if (value_len == WRONG_POS)
				{
					return false;
				}

				copy_size = min(len - 1, value_len);

				wcsncpy_s(value, len, buff_ + pos, copy_size);
				value[copy_size] = L'\0';

				return true;
			}
			else if (buff_[pos] == L'"')
			{
				pos++;

				while (pos < size_)
				{
					cnt = until(buff_ + pos, L"\"");

					value_len += cnt;

					if (buff_[pos - 1] != L'\\') // 도착
					{
						size_t copy_size = min(len - 1, value_len);

						wcsncpy_s(value, len, buff_ + pos, copy_size);
						value[copy_size] = L'\0';

						return true;
					}

					pos = pos + cnt + 1;
					value_len++;
				}
			}

			return false;
		}

	private:
		int wcsntoi(int base, const wchar_t* number, size_t len)
		{
			if (base < 2 || base > 36) // 지원 안 하는 진법은 무시
			{
				return -1;
			}

			wchar_t digits[] = L"0123456789abcdefghijklmnopqrstuvwxyz";
			int ret = 0;

			for (size_t i = 0; i < len; ++i)
			{
				wchar_t ch = towlower(number[i]);  // 대소문자 무시

				int value = -1;
				for (int j = 0; j < base; ++j)
				{
					if (ch == digits[j])
					{
						value = j;
						break;
					}
				}

				if (value == -1)
				{
					return -1;
				}

				ret = ret * base + value;
			}

			return ret;
		}

	private:
		wchar_t* buff_;
		size_t size_;
	};

	inline Document parse(const wchar_t* filename)
	{
		HANDLE file = CreateFileW(
			filename,				// 파일 또는 디바이스의 경로
			GENERIC_READ,			// 접근 권한
			FILE_SHARE_READ,		// 다른 프로세스의 접근 권한
			NULL,					// 보안 속성 및 핸들 상속 여부
			OPEN_EXISTING,			// 파일이 존재할 때와 존재하지 않을 때의 동작
			FILE_ATTRIBUTE_NORMAL,	// 파일 또는 디바이스의 특성과 플래그 지정
			NULL);					// 기존 파일에서 속성을 복사하고 싶을 때

		if (file == INVALID_HANDLE_VALUE)
		{
			// 로그 GetLastError();

			throw std::runtime_error("failed to open file");
		}

		DWORD file_size = GetFileSize(file, nullptr);
		if (file_size == INVALID_FILE_SIZE)
		{
			// 로그 GetLastError();

			CloseHandle(file);

			throw std::runtime_error("file size is not correct");
		}

		if (file_size < 2 || file_size % 2)
		{
			// 로그

			CloseHandle(file);

			throw std::runtime_error("file size is not correct");
		}

		BYTE* raw_buffer = new BYTE[file_size];
		DWORD bytes_read = 0;

		if (!ReadFile(file, raw_buffer, file_size, &bytes_read, nullptr))
		{
			// 로그 GetLastError();

			delete[] raw_buffer;

			CloseHandle(file);

			throw std::runtime_error("failed to read file");
		}

		if (bytes_read != file_size)
		{
			// 로그

			delete[] raw_buffer;

			CloseHandle(file);

			throw std::runtime_error("failed to read file");
		}

		CloseHandle(file);

		size_t offset = 0;
		if (raw_buffer[0] != 0xFF || raw_buffer[1] != 0xFE)
		{
			// 로그

			delete[] raw_buffer;

			throw std::runtime_error("no UTF-16 LE BOM detected");
		}

		offset = 2;

		size_t wchar_cnt = (file_size - offset) / sizeof(wchar_t);

		wchar_t* buff = new wchar_t[wchar_cnt + 1];
		wmemcpy(buff, reinterpret_cast<wchar_t*>(raw_buffer + offset), wchar_cnt);

		buff[wchar_cnt] = L'\0';

		delete[] raw_buffer;

		return Document(buff, wchar_cnt);
	}
}