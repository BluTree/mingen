function project(name, prj)
	print(name)
	print("\tprj[\"sources\"] = " .. prj["sources"])
	print("\tprj[\"compile_options\"] = {")
	for i=1, #prj["compile_options"] do
		print("\t\t" .. prj["compile_options"][i])
	end
	print("\t}")
	print("\tprj[\"link_options\"] = {")
	for i=1, #prj["link_options"] do
		print("\t\t" .. prj["link_options"][i])
	end
	print("\t}")
end

function generate(prj)
	print(prj)
end

local prj_HelloWorld = project("HelloWorld", {
	["sources"] = "src/**",
	["compile_options"] = {"-g", "-Wall"},
	["link_options"] = {"--shared"}
})

local prj_app = project("HelloApp", {
	["sources"] = "src/bin/**",
	["compile_options"] = {"-g", "-Wall"},
	["deps"] = {prj_HelloWorld}
})

generate(prj_app)