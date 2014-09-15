#ifndef GECOM_PROT_HEADER
#define GECOM_PROT_HEADER

#include <memory>


#include <gecom/PNG.hpp>
#include <gecom/Entity.hpp>
#include <gecom/Scene.hpp>
#include <gecom/SpineDrawable.hpp>

namespace pxljm {
	class ProtagonistDrawable : public gecom::SpineDrawable {
	public:
		ProtagonistDrawable(const std::shared_ptr<gecom::Entity> parent) : gecom::SpineDrawable(std::string("protagonist"), parent, 0.015) { }
        //void callback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) const override;
	};

	class DroneDrawable : public gecom::SpineDrawable {
	public:
		DroneDrawable(const std::shared_ptr<gecom::Entity> parent) : SpineDrawable(std::string("drone"), parent, 0.005) { }
	};
}

#endif
