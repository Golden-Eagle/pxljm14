# add resources as targets so they show up in IDEs

SET(SHADERS
	"shader/fullscreen.glsl"
	"shader/scene_default.glsl"
)

add_custom_target(
	res
	SOURCES
	${SHADERS}
)

source_group("shader" FILES ${SHADERS})
