-- mg.dir("build")

local prj_lib = mg.project({
	name = "lib",
	type = mg.project_type.static_library,
	sources = {"src/lib/**.cc"},
	compile_options = {"-g", "-Wall"}
})

local prj_exe = mg.project({
	name = "exe",
	type = mg.project_type.executable,
	sources = {"src/exe/**.cc"},
	compile_options = {"-g", "-Wall"},
	dependencies = {prj_lib}
})

mg.generate({prj_exe})