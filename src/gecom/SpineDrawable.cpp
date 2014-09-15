#include "SpineDrawable.hpp"

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

char* _spUtil_readFile(const char* path, int* length) {
    return _readFile(path, length);
}

void callback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) {
    spTrackEntry *entry = spAnimationState_getCurrent(state, trackIndex);
    const char *animationName = (entry && entry->animation) ? entry->animation->name : 0;
    switch (type) {
        case SP_ANIMATION_START:
            printf("%d start: %s\n", trackIndex, animationName);
            break;
        case SP_ANIMATION_END:
            printf("%d end: %s\n", trackIndex, animationName);
            break;
        case SP_ANIMATION_COMPLETE:
            printf("%d complete: %s, %d\n", trackIndex, animationName, loopCount);
            break;
        case SP_ANIMATION_EVENT:
            printf("%d event: %s, %s: %d, %f, %s\n", trackIndex, animationName, event->data->name, event->intValue, event->floatValue,
                    event->stringValue);
            break;
    }
    fflush(stdout);
}