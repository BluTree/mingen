mg.configurations({"debug", "release"})

-- TODO custom require for relative path resolve + (optional) give config to run the file with
require("deps/mingen")

print("build_dir main: " .. mg.get_build_dir())

local prj_exe = mg.project({
	name = "exe",
	type = mg.project_type.executable,
	sources = {"src/exe/**.cc"},
	includes = {"src/lib"},
	compile_options = {"-g", "-Wall"},
	dependencies = {prj_lib, glfw}
})

mg.add_post_build_cmd(prj_exe, {
	input = {'hello', 'hello2'},
	output = {'hello_out', 'hello_out2'},
	cmd = 'echo ${in} - ${out}'
})

mg.add_pre_build_cmd(prj_exe, {
	input = 'hello2_in',
	output = 'hello2_out',
	cmd = 'echo hello2'
})

mg.add_pre_build_copy(prj_exe, {
	input = 'mingen.lua',
	output = 'build/mingen.lua.out'
})


if mg.need_generate() then
	mg.generate({prj_exe})
end