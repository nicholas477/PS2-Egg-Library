#pragma once

#include "assert.hpp"
#include <cstddef>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <array>

#include "egg/string.hpp"


namespace Filesystem
{
struct Path;
enum class Type {
	uninitialized = 0, // default value, if any filesystem functions are run
	                   // before initialization then it will crash
	cdrom,
	host,
	rom
};

Type get_filesystem_type();
void set_filesystem_type(Type new_type);

void run_tests();

static constexpr const char* get_filesystem_prefix(Type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case Type::cdrom:
			return "cdrom0:\\";
		case Type::host:
			return "host0:";
		case Type::rom:
			return "rom0:";

		case Type::uninitialized:
			check(false);
			break;
	}

	return 0;
}

static constexpr char get_filesystem_separator(Type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case Type::cdrom:
			return '\\';
		case Type::host:
		case Type::rom:
			return '/';

		case Type::uninitialized:
			check(false);
			break;
	}

	return '\0';
}

std::string convert_filepath_to_systempath(std::string_view path, Type in_filesystem_type = get_filesystem_type());

template <Type type>
static constexpr const char* constexpr_convert_filepath_to_systempath(std::string_view path)
{
	size_t host_string_length = std::char_traits<char>::length(get_filesystem_prefix(type)) + 1; // + 1 for the filesystem separator
	host_string_length += path.length() + 1;
	char* out_path_chars = new char[host_string_length];

	snprintf(out_path_chars, host_string_length, "%s%c%.*s",
	         get_filesystem_prefix(type), get_filesystem_separator(type), path.length(), path.data());

	for (size_t i = 0; i < host_string_length; ++i)
	{
		if (out_path_chars[i] == '\\' || out_path_chars[i] == '/')
		{
			out_path_chars[i] = get_filesystem_separator(type);
		}
		else if (i > std::char_traits<char>::length(get_filesystem_prefix(type)))
		{
			if (out_path_chars[i] == '-')
			{
				out_path_chars[i] = '_';
			}
			out_path_chars[i] = toupper(out_path_chars[i]);
		}
	}

	return out_path_chars;
}

// Returns true if the file was loaded succesfully
bool load_file(const Path& path, std::vector<std::byte>& out_bytes);
bool load_file(const Path& path, std::unique_ptr<std::byte[]>& out_bytes, size_t& size, size_t alignment = 1);
bool file_exists(const Path& path);
void iterate_dir(const Path& dir, std::function<void(const Path&)> itr_func, bool recursive = false);

struct Path
{
	Path() = default;

	Path(const std::string& in_path, bool convert_path = true)
	{
		if (convert_path)
		{
			_path_str = convert_filepath_to_systempath(in_path);
		}
		else
		{
			_path_str = in_path;
		}
	}

#ifdef _MSC_VER
	Path(const wchar_t* in_path, bool convert_path = true)
	{
		static char buffer[256];
		size_t ret = wcstombs(buffer, in_path, sizeof(buffer));
		if (ret == sizeof(buffer))
		{
			buffer[sizeof(buffer) - 1] = '\0';
		}

		if (convert_path)
		{
			_path_str = convert_filepath_to_systempath(buffer);
		}
		else
		{
			_path_str = buffer;
		}
	}
#endif

	Path(const char* in_path, bool convert_path = true)
	{
		if (convert_path)
		{
			_path_str = convert_filepath_to_systempath(in_path);
		}
		else
		{
			_path_str = in_path;
		}
	}

	inline const char* c_str() const
	{
		return _path_str.c_str();
	}

	operator const std::string &() const
	{
		return _path_str;
	}

	operator std::string_view() const
	{
		return _path_str;
	}

	std::string _path_str;

	bool operator==(const Path& other) const
	{
		return _path_str == other._path_str;
	}
};

static void convert_to_83_path(std::string_view path, char* buffer, size_t len)
{
	// Iterate backwards and look for the beginning of the filename
	size_t filename_begin  = 0;
	size_t extension_begin = path.size();
	for (size_t i = path.size();; --i)
	{
		if (path[i] == '.')
		{
			extension_begin = i;
		}
		else if (path[i] == '/' || path[i] == '\\')
		{
			filename_begin = i + 1;
			break;
		}

		if (i == 0)
		{
			break;
		}
	}

	// Copy up to the first 8 filename characters over
	size_t itr_end = std::min(std::min(filename_begin + 8, extension_begin), path.size());
	for (size_t i = 0; i < itr_end; ++i)
	{
		buffer[i] = path[i];
	}

	// Copy the extension over
	size_t itr_extension_end = std::min((size_t)4U, path.size() - extension_begin);
	for (size_t i = 0; i < itr_extension_end; ++i)
	{
		buffer[itr_end + i] = path[extension_begin + i];
	}
	buffer[itr_end + itr_extension_end] = '\0';
}

class ConstexprPath
{
public:
	constexpr ConstexprPath() noexcept
	{
	}

	constexpr char& operator[](size_t id) noexcept { return mem[id]; }
	constexpr const char& operator[](size_t id) const noexcept { return mem[id]; }

	constexpr const char* data() const noexcept { return mem; }
	constexpr size_t max_size() const noexcept { return 256; }
	constexpr size_t length() const { return constexpr_strlen(mem); }

	constexpr bool operator==(const char* rhs)
	{
		return std::string_view(mem) == std::string_view(rhs);
	}

	char mem[256];
};

constexpr ConstexprPath constexpr_convert_to_83_path(const char* path)
{
	ConstexprPath buffer;

	// Iterate backwards and look for the beginning of the filename
	size_t filename_begin  = 0;
	size_t extension_begin = constexpr_strlen(path);
	for (size_t i = constexpr_strlen(path);; --i)
	{
		if (path[i] == '.')
		{
			extension_begin = i;
		}
		else if (path[i] == '/' || path[i] == '\\')
		{
			filename_begin = i + 1;
			break;
		}

		if (i == 0)
		{
			break;
		}
	}

	// Copy up to the first 8 filename characters over
	size_t itr_end = std::min(std::min(filename_begin + 8, extension_begin), constexpr_strlen(path));
	for (size_t i = 0; i < itr_end; ++i)
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			buffer[i] = get_filesystem_separator(Type::cdrom);
		}
		else if (path[i] == '-')
		{
			buffer[i] = '_';
		}
		else
		{
			buffer[i] = charToUpper(path[i]);
		}
	}

	// Copy the extension over
	size_t itr_extension_end = std::min((size_t)4U, constexpr_strlen(path) - extension_begin);
	for (size_t i = 0; i < itr_extension_end; ++i)
	{
		if (path[extension_begin + i] == '-')
		{
			buffer[itr_end + i] = '_';
		}
		else
		{
			buffer[itr_end + i] = charToUpper(path[extension_begin + i]);
		}
	}
	buffer[itr_end + itr_extension_end] = '\0';

	return buffer;
}

static_assert(constexpr_convert_to_83_path("/asdf-ggggg/sdkfjs.egg2") == "\\ASDF_GGGGG\\SDKFJS.EGG");

} // namespace Filesystem

// Path literal constructor
//
// Converts the path to a system path. I.e: assets/sounds/vine_boom.adpcm -> cdrom0:/ASSETS/SOUNDS/VINE_BOOM.ADPCM
static Filesystem::Path operator""_p(const char* p, std::size_t)
{
	return Filesystem::Path(p);
}

template <>
struct std::hash<Filesystem::Path>
{
	std::size_t operator()(const Filesystem::Path& k) const
	{
		return std::hash<std::string>()(k._path_str);
	}
};