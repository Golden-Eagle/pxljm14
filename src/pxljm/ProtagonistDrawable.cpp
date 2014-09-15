#include <gecom/SpineDrawable.hpp>

#include "ProtagonistDrawable.hpp"
#include "Player.hpp"

//void pxljm::ProtagonistDrawable::callback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) const {
//    auto parent_prot = std::static_pointer_cast<PlayerEntity>(getParent());
//    spTrackEntry *entry = spAnimationState_getCurrent(state, trackIndex);
//    const char *animationName = (entry && entry->animation) ? entry->animation->name : 0;
//
//    if(strcmp(animationName, "jump") == 0 && type == SP_ANIMATION_COMPLETE) {
//        parent_prot->finishJumping();
//    }
//}