#include <Geode/Geode.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

using namespace geode::prelude;

namespace {
    // Walks the whole node tree and makes every text box's placeholder white.
    void whitenPlaceholders(CCNode* node) {
        if (!node) return;

        if (auto input = typeinfo_cast<CCTextInputNode*>(node)) {
            input->setLabelPlaceholderColor({ 255, 255, 255 });
        }

        auto children = node->getChildren();
        if (!children) return;

        for (auto* child : CCArrayExt<CCNode*>(children)) {
            whitenPlaceholders(child);
        }
    }
}

// The level edit screen (level name + description boxes)
class $modify(WhitePlaceholderEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

        whitenPlaceholders(this);

        return true;
    }
};
