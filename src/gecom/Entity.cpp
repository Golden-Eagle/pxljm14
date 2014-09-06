#include "Entity.hpp"

#include <atomic>

std::atomic<gecom::entity_id_t> gecom::Entity::sm_ID(1);