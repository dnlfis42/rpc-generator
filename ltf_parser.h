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

				// ���� pos�� ��ȿ���� ���� (�տ� white space �پ��������� �������� ����) -> section ��Ʈ ó�� ������
				cnt = skip_ignorable(buff_ + pos);

				if (cnt == WRONG_POS)
				{
					return WRONG_POS;
				}

				pos += cnt;
			}

			size_t key_len = wcslen(key);

			// ���� pos �� ��ȿ�� Ű�� ������ ����Ŵ

			size_t limit = until(buff_ + pos, L"["); // ���� ������ ��ȿ ����, WRONG_POS�� ������ ������
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
				// ��ȿ Ű ��ġ ����

				cnt = until(buff_ + pos, L" \t");

				if (cnt == WRONG_POS)
				{
					break;
				}

				if (key_len == cnt && wcsncmp(key, buff_ + pos, cnt) == 0) // ã��
				{
					pos += cnt; // ���� key ���� �� ����

					cnt = skip(buff_ + pos, L"=");

					if (cnt == WRONG_POS)
					{
						break;
					}

					pos += cnt; // = ���� ����

					cnt = skip_ignorable(buff_ + pos);

					if (cnt == WRONG_POS)
					{
						break;
					}

					pos += cnt; // value�� ���� �ε���

					return pos;
				}

				// �ش� Ű ������ �ƴ� ���

				pos += cnt; // ���� key ���� �� ����

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

				pos += cnt; // value�� ���� �ε���

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
		* @brief ������ pos ��ġ \0���� Ȯ�� ����
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
		* @brief ������ pos ��ġ \0���� Ȯ�� ����
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

				pos += cnt; // ���� *�� ����Ŵ

				if (str[pos + 1] == L'/') // */���� /�� ã�Ҵٸ� / ������ ������
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
					else // ���� ��Ȳ
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

					if (buff_[pos - 1] != L'\\') // ����
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
			if (base < 2 || base > 36) // ���� �� �ϴ� ������ ����
			{
				return -1;
			}

			wchar_t digits[] = L"0123456789abcdefghijklmnopqrstuvwxyz";
			int ret = 0;

			for (size_t i = 0; i < len; ++i)
			{
				wchar_t ch = towlower(number[i]);  // ��ҹ��� ����

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
			filename,				// ���� �Ǵ� ����̽��� ���
			GENERIC_READ,			// ���� ����
			FILE_SHARE_READ,		// �ٸ� ���μ����� ���� ����
			NULL,					// ���� �Ӽ� �� �ڵ� ��� ����
			OPEN_EXISTING,			// ������ ������ ���� �������� ���� ���� ����
			FILE_ATTRIBUTE_NORMAL,	// ���� �Ǵ� ����̽��� Ư���� �÷��� ����
			NULL);					// ���� ���Ͽ��� �Ӽ��� �����ϰ� ���� ��

		if (file == INVALID_HANDLE_VALUE)
		{
			// �α� GetLastError();

			throw std::runtime_error("failed to open file");
		}

		DWORD file_size = GetFileSize(file, nullptr);
		if (file_size == INVALID_FILE_SIZE)
		{
			// �α� GetLastError();

			CloseHandle(file);

			throw std::runtime_error("file size is not correct");
		}

		if (file_size < 2 || file_size % 2)
		{
			// �α�

			CloseHandle(file);

			throw std::runtime_error("file size is not correct");
		}

		BYTE* raw_buffer = new BYTE[file_size];
		DWORD bytes_read = 0;

		if (!ReadFile(file, raw_buffer, file_size, &bytes_read, nullptr))
		{
			// �α� GetLastError();

			delete[] raw_buffer;

			CloseHandle(file);

			throw std::runtime_error("failed to read file");
		}

		if (bytes_read != file_size)
		{
			// �α�

			delete[] raw_buffer;

			CloseHandle(file);

			throw std::runtime_error("failed to read file");
		}

		CloseHandle(file);

		size_t offset = 0;
		if (raw_buffer[0] != 0xFF || raw_buffer[1] != 0xFE)
		{
			// �α�

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