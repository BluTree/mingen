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

if mg.need_generate() then
	mg.generate({prj_exe})
end