mg.configurations({"debug", "release"})

require("deps/mingen")

-- mg.dir("build")


local prj_exe = mg.project({
	name = "exe",
	type = mg.project_type.executable,
	sources = {"src/exe/**.cc"},
	includes = {"src/lib"},
	compile_options = {"-g", "-Wall"},
	dependencies = {prj_lib}
})

if mg.need_generate() then
	mg.generate({prj_exe})
end