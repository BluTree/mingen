mg.configurations({"debug", "release"})

require("deps/mingen")

-- mg.dir("build")

print("build_dir main: " .. mg.get_build_dir())

local prj_exe = mg.project({
	name = "exe",
	type = mg.project_type.executable,
	sources = {"src/exe/**.cc"},
	includes = {"src/lib"},
	compile_options = {"-g", "-Wall"},
	dependencies = {prj_lib, glfw}
})

mg.add_pre_build_cmd(prj_exe, {
	input = '',
	output = 'hello_out',
	cmd = 'echo hello'
})

mg.add_pre_build_cmd(prj_exe, {
	output = 'hello2_out',
	cmd = 'echo hello2'
})

mg.add_pre_build_copy(prj_exe, {
	input = 'mingen.lua',
	output = 'build/mingen.lua.out'
})

-- print(#prj_exe.pre_build_cmds)
-- for i=1,#prj_exe.pre_build_cmds do
-- 	print(prj_exe.pre_build_cmds[i].cmd)
-- end

if mg.need_generate() then
	mg.generate({prj_exe})
end