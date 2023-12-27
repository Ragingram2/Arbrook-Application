--ProjectDir = os.getcwd().."/../"
projectDir = getProjectDir()
print(projectDir)
local project =
{
    name = "sandbox",
    group = "applications",
    type = "ConsoleApp",
    location = projectDir.."src/sandbox",
    toolset = "clang",
    lang = "C++",
    dialect = "C++20",
    buildOptions = {DefaultBuildOptions, "TRACY_IMPORTS"},
    targetDir = SolutionDir.."bin\\build\\",
    defines = DefaultDefines
}

includedirs {
    basicIncludes,
    "include/bx/include/compat/msvc/"
}   

files { 
    SolutionDir.."arbrook/Arbrook.cpp",
    project.location.."/**.h",
    project.location.."/**.hpp",
    project.location.."/**.inl",
    project.location.."/**.c",
    project.location.."/**.cpp" 
}


dofile (SolutionDir.."arbrook/core/include-core.lua")
dofile (SolutionDir.."arbrook/graphics/include-graphics.lua")
dofile (SolutionDir.."arbrook/input/include-input.lua")

print(project.name .. " Project")

return project