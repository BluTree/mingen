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

if mg.need_generate() then
	mg.generate({prj_lib})
end