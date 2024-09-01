local prj_HelloWorld = project("HelloWorld", {
	["sources"] = {"**.c", "**.cpp"},
	["compile_options"] = {"-g", "-Wall"},
	["link_options"] = {"--shared"}
})

for i=1, #prj_HelloWorld do
	print(prj_HelloWorld[i])
end


-- local prj_app = project("HelloApp", {
-- 	["sources"] = "src/bin/**",
-- 	["compile_options"] = {"-g", "-Wall"},
-- 	["deps"] = {prj_HelloWorld}
-- })

-- generate(prj_app)