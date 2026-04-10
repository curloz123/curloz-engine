-- Project Configuration

set_project("curloz-engine")
set_version("0.1.0")
set_languages("c++17")

add_rules("mode.debug", "mode.release")
set_defaultmode("debug")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })


-- Dependencies




target("curloz engine")
    set_kind("binary")
    set_rundir("$(projectdir)")
    set_toolchains("gcc", "clang")

    -- source files
    add_files("src/**/*.cpp")
    add_files("src/*.cpp")
    add_files("external/**/*.cpp", "external/**/*.c")

    -- include dirs
    add_includedirs("include", { public = true })
    add_includedirs("external", { public = true })

    -- Project Definitions
    add_defines("GLFW_INCLUDE_NONE")

    -- external libraries folder
    local lib_folder = ""
    if is_plat("linux") then
        add_linkdirs("lib/linux")
    elseif is_plat("windows") then
        add_linkdirs("lib/windows")
    end

    -- Manual libraries
    add_links("glfw3")

    -- system libraries
    add_links("vulkan", "fmt")

    if is_plat("linux") then
        add_syslinks("pthread", "dl")
    end

    -- Compile all shaders in assets/shaders/ to SPIR-V before every run
    before_run(function (target)
        local shader_dir = path.join(os.projectdir(), "assets", "shaders")
        local glslc = find_tool("glslc") or find_tool("glslangValidator")
        if not glslc then
            raise("Shader compiler not found. Please install glslc (Vulkan SDK) or glslangValidator.")
        end
        local exts = { ".vert", ".frag", ".comp", ".geom", ".tesc", ".tese" }
        for _, ext in ipairs(exts) do
            for _, src in ipairs(os.files(path.join(shader_dir, "*" .. ext))) do
                -- e.g. assets/shaders/triangle_vert.vert -> assets/shaders/triangle_vert.spirv
                local base = path.basename(src)
                local spv  = path.join(shader_dir, base .. ".spirv")
                print("Compiling shader: " .. path.filename(src) .. " -> " .. path.filename(spv))
                os.vrunv(glslc.program, { src, "-o", spv })
            end
        end
    end)
