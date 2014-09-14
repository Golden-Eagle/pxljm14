# add resources as targets so they show up in IDEs

SET(SHADERS
	"shader/fullscreen.glsl"
	"shader/scene.glsl"
	"shader/scene_default.glsl"
	"shader/chunk_standard.glsl"
	"shader/background.glsl"
	"shader/spine.glsl"
)

add_custom_target(
	res
	SOURCES
	${SHADERS}
)

source_group("shader" FILES ${SHADERS})
