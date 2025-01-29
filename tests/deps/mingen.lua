prj_lib = mg.project({
	name = "lib",
	type = mg.project_type.static_library,
	sources = {"../src/lib/**.cc"},
	compile_options = {"-g", "-Wall"},
	debug = {
		compile_options = {"-O0"}
	},
	release = {
		compile_options = {"-O2"}
	},
})


-- print("build_dir deps: " .. mg.get_build_dir())

-- glfw_dest_dir = mg.get_build_dir() .. "deps/glfw/"
-- if net.download("https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip", glfw_dest_dir) then
-- 	print('download done, building')
-- else
-- 	print('nothing to od')
-- end

-- glfw = mg.project({
-- 	name = "glfw",
-- 	type = mg.project_type.prebuilt,
-- 	static_libraries = {'glfw3.lib'},
-- 	static_library_directories = { glfw_dest_dir .. 'build/Release'}
-- })

-- imgui_dest_dir = mg.get_build_dir() .. "deps/imgui/"
-- if net.download("https://github.com/ocornut/imgui/archive/refs/tags/v1.91.7.zip", imgui_dest_dir) then
-- 	os.copy_file('imconfig.h', imgui_dest_dir .. 'imconfig.h')
-- end

-- imgui = mg.project({
-- 	name = "imgui",
-- 	type = mg.project_type.sources,
-- 	sources = {
-- 		imgui_dest_dir .. '*.cpp',
-- 		imgui_dest_dir .. 'backends/imgui_impl_vulkan.cpp',
-- 		imgui_dest_dir .. 'backends/imgui_impl_win32.cpp'},
-- 	compile_options = {'-g'},
-- 	debug = {
-- 		compile_options = {'-O0'}
-- 	},
-- 	release = {
-- 		compile_options = {'-O2'}
-- 	}
-- })

if mg.need_generate() then
	mg.generate({prj_lib})
end