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

print("build_dir deps: " .. mg.get_build_dir())

if net.download("https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip", mg.get_build_dir() .. "deps/glfw/") then
	print("download done")
end

glfw = mg.project({
	name = "glfw",
	type = mg.project_type.prebuilt,
	static_libraries = {'glfw3.lib'},
	static_library_directories = { mg.get_build_dir() .. 'deps/glfw/lib-vc2022'}
})

if mg.need_generate() then
	mg.generate({prj_lib})
end