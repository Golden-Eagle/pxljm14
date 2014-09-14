#include "ProtagonistDrawable.hpp"

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path){
	gecom::image* texture = new gecom::image(gecom::image::type_png(), path, false);

	//if (!texture->loadFromFile(path)) return;
	//texture->setSmooth(true);
	self->rendererObject = texture;
	self->width = texture->width();
	self->height = texture->height();
}

void _spAtlasPage_disposeTexture(spAtlasPage* self){
	delete (gecom::image*)self->rendererObject;
}

char* _spUtil_readFile(const char* path, int* length){
	return _readFile(path, length);
}