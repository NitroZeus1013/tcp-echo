workspace "TCP"
    configurations {"Debug", "Release"}
    location "build"

project "server"
    kind "ConsoleApp"
    targetdir "bin/%{cfg.buildcfg}"
    language "C++"
    cppdialect "C++17"
    includedirs {"include"}
    files {"src/server/server.cpp","src/utils/**.cpp","include/**/*.hpp"}

    filter "configurations:Debug"
        defines {"Debug"}
        symbols "On"
    
    filter "configurations:Release"
        defines {"NDEBUG"}
        optimize "On"

project "client"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    includedirs {"include"}
    targetdir "bin/%{cfg.buildcfg}"
    files {"src/client/client.cpp","src/utils/**.cpp","include/**/*.hpp"}

    filter "configurations:Debug"
        defines {"Debug"}
        symbols "On"

    filter "configurations:Release"
        defines {"NDEBUG"}
        optimize "On"
