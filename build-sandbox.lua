createProject("applications","sandbox","ConsoleApp")
targetdir "$(SolutionDir)bin\\build\\"
defines{
    "TRACY_IMPORTS",
    "BX_CONFIG_DEBUG",
    "BGFX_CONFIG_RENDERER_OPENGL=44"
}
includedirs {
    basicIncludes,
    "include/bx/include/compat/msvc/"
}   

files { 
    "../../arbrook/Arbrook.cpp",
    "src/sandbox/**.h",
    "src/sandbox/**.hpp",
    "src/sandbox/**.inl",
    "src/sandbox/**.c",
    "src/sandbox/**.cpp" 
}


dofile "arbrook/core/include-core.lua"
dofile "arbrook/graphics/include-graphics.lua"
dofile "arbrook/input/include-input.lua"

