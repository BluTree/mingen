-- mg.dir("build")

mg.configurations({"debug", "release"})

local prj_lib = mg.project({
	name = "lib",
	type = mg.project_type.static_library,
	sources = {"src/lib/**.cc"},
	compile_options = {"-g", "-Wall"},
	debug = {
		compile_options = {"-O0"}
	},
	release = {
		compile_options = {"-O2"}
	},
})

local prj_exe = mg.project({
	name = "exe",
	type = mg.project_type.executable,
	sources = {"src/exe/**.cc"},
	includes = {"src/lib"},
	compile_options = {"-g", "-Wall"},
	dependencies = {prj_lib}
})

mg.generate({prj_exe})