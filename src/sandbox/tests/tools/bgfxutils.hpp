#pragma once
#include <chrono>
#include <ctime>
#include <string>
#include <iostream>
#include <fstream>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bimg/bimg.h>
#include <bx/math.h>
#include <bx/allocator.h>
#include <bx/file.h>
#include <bx/bounds.h>
#include <bx/pixelformat.h>
#include <bx/string.h>

#include <rsl/logging>

namespace entry
{
	static uint32_t s_debug = BGFX_DEBUG_NONE;
	static uint32_t s_reset = BGFX_RESET_NONE;
	static uint32_t s_width = 1280;
	static uint32_t s_height = 720;
	static bool s_exit = false;

	static bx::FileReaderI* s_fileReader = NULL;
	static bx::FileWriterI* s_fileWriter = NULL;

	extern bx::AllocatorI* getDefaultAllocator();
	inline bx::AllocatorI* g_allocator = getDefaultAllocator();

	typedef bx::StringT<&g_allocator> String;

	static String s_currentDir;

	class FileReader : public bx::FileReader
	{
		typedef bx::FileReader super;

	public:
		virtual bool open(const bx::FilePath& _filePath, bx::Error* _err) override
		{
			String filePath(s_currentDir);
			filePath.append(_filePath);
			return super::open(filePath.getPtr(), _err);
		}
	};

	class FileWriter : public bx::FileWriter
	{
		typedef bx::FileWriter super;

	public:
		virtual bool open(const bx::FilePath& _filePath, bool _append, bx::Error* _err) override
		{
			String filePath(s_currentDir);
			filePath.append(_filePath);
			return super::open(filePath.getPtr(), _append, _err);
		}
	};

	inline void setCurrentDir(const char* _dir)
	{
		s_currentDir.set(_dir);
	}

	inline bx::FileReaderI* getFileReader()
	{
		if (!s_fileReader)
		{
			s_fileReader = BX_NEW(g_allocator, FileReader);
		}
		return s_fileReader;
	}

	inline bx::FileWriterI* getFileWriter()
	{
		if (!s_fileWriter)
		{
			s_fileWriter = BX_NEW(g_allocator, FileWriter);
		}
		return s_fileWriter;
	}

	inline bx::AllocatorI* getDefaultAllocator()
	{
		BX_PRAGMA_DIAGNOSTIC_PUSH();
		BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4459); // warning C4459: declaration of 's_allocator' hides global declaration
		BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wshadow");
		static bx::DefaultAllocator s_allocator;
		return &s_allocator;
		BX_PRAGMA_DIAGNOSTIC_POP();
	}
}

namespace rythe::testing
{
	namespace log = rsl::log;
	inline const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
	{
		if (bx::open(_reader, _filePath))
		{
			uint32_t size = (uint32_t)bx::getSize(_reader);
			const bgfx::Memory* mem = bgfx::alloc(size + 1);
			bx::read(_reader, mem->data, size, bx::ErrorAssert{});
			bx::close(_reader);
			mem->data[mem->size - 1] = '\0';
			return mem;
		}

		log::debug("Failed to load {}.", _filePath);
		return NULL;
	}

	inline void* loadMem(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
	{
		if (bx::open(_reader, _filePath))
		{
			uint32_t size = (uint32_t)bx::getSize(_reader);
			void* data = bx::alloc(_allocator, size);
			bx::read(_reader, data, size, bx::ErrorAssert{});
			bx::close(_reader);

			if (NULL != _size)
			{
				*_size = size;
			}
			return data;
		}

		log::debug("Failed to load {}.", _filePath);
		return NULL;
	}

	inline bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
	{
		char filePath[512];

		const char* shaderPath = "???";

		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12: shaderPath = "resources/shaders/bgfx/dx11/";  break;
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:        shaderPath = "resources/shaders/pssl/";  break;
		case bgfx::RendererType::Metal:      shaderPath = "resources/shaders/metal/"; break;
		case bgfx::RendererType::Nvn:        shaderPath = "resources/shaders/nvn/";   break;
		case bgfx::RendererType::OpenGL:     shaderPath = "resources/shaders/bgfx/ogl/";  break;
		case bgfx::RendererType::OpenGLES:   shaderPath = "resources/shaders/essl/";  break;
		case bgfx::RendererType::Vulkan:     shaderPath = "resources/shaders/spirv/"; break;

		case bgfx::RendererType::Count:
			BX_ASSERT(false, "You should not be here!");
			break;
		}

		bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
		bx::strCat(filePath, BX_COUNTOF(filePath), _name);
		bx::strCat(filePath, BX_COUNTOF(filePath), ".shader");

		bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath));
		bgfx::setName(handle, _name);

		return handle;
	}

	inline bgfx::ShaderHandle loadShader(const char* _name)
	{
		return loadShader(entry::getFileReader(), _name);
	}


	inline bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
	{
		bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (NULL != _fsName)
		{
			fsh = loadShader(_reader, _fsName);
		}

		return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
	}

	inline bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName)
	{
		return loadProgram(entry::getFileReader(), _vsName, _fsName);
	}


	struct BgfxCallback : public bgfx::CallbackI
	{
		virtual ~BgfxCallback()
		{
		}

		virtual void fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str) override
		{
			BX_UNUSED(_filePath, _line);

			// Something unexpected happened, inform user and bail out.
			log::error("Fatal error {}:{} [{}]: {}", _filePath, _line, _code, _str);

			//abort();
		}

		virtual void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override
		{
			//log::info("{} ({}): ", _filePath, _line);
			bx::debugPrintfVargs(_format, _argList);
			//std::printf(_format,_argList);
			//log::info(_format, _argList);
		}

		virtual void profilerBegin(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override
		{
		}

		virtual void profilerBeginLiteral(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override
		{
		}

		virtual void profilerEnd() override
		{
		}

		virtual uint32_t cacheReadSize(uint64_t _id) override
		{
			return 0;
		}

		virtual bool cacheRead(uint64_t _id, void* _data, uint32_t _size) override
		{
			return false;
		}

		virtual void cacheWrite(uint64_t _id, const void* _data, uint32_t _size) override
		{

		}

		virtual void screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _data, uint32_t /*_size*/, bool _yflip) override
		{

		}

		virtual void captureBegin(uint32_t _width, uint32_t _height, uint32_t /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool _yflip) override
		{

		}

		virtual void captureEnd() override
		{

		}

		virtual void captureFrame(const void* _data, uint32_t /*_size*/) override
		{
		}
	};
}