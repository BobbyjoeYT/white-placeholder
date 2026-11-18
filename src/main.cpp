#include <Geode/Geode.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/FLAlertLayer.hpp>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
// Part 1: white placeholder text on the level edit screen
// ---------------------------------------------------------------------------
namespace {
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

class $modify(WhitePlaceholderEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;
        whitenPlaceholders(this);
        return true;
    }
};

// ---------------------------------------------------------------------------
// Part 2: recolor the brown song boxes in the Jukebox (NONG) popup
// ---------------------------------------------------------------------------
namespace {
    // The Jukebox mod paints each song box with this exact brown.
    bool isJukeboxBrown(ccColor3B const& c) {
        return c.r == 76 && c.g == 42 && c.b == 25;
    }

    // Walks through everything inside the popup. Every box called "background"
    // that is still the Jukebox brown (or that we already recolored before,
    // so the color setting can change live) gets the chosen color.
    void recolorJukeboxBoxes(CCNode* node, ccColor3B const& color) {
        if (!node) return;
        auto children = node->getChildren();
        if (!children) return;
        for (auto* child : CCArrayExt<CCNode*>(children)) {
            if (auto slice = typeinfo_cast<NineSlice*>(child)) {
                if (slice->getID() == "background") {
                    bool ours = slice->getUserObject("greybars-recolored") != nullptr;
                    if (ours || isJukeboxBrown(slice->getColor())) {
                        slice->setColor(color);
                        if (!ours) {
                            slice->setUserObject("greybars-recolored", CCString::create("yes"));
                        }
                    }
                }
                continue; // nothing else to find inside a box
            }
            recolorJukeboxBoxes(child, color);
        }
    }

    // A tiny invisible node that lives inside the Jukebox popup and re-checks
    // the boxes a few times per second, because the Jukebox rebuilds its list
    // whenever you pick a song, add one, or download one.
    class BoxRecolorer : public CCNode {
    public:
        static BoxRecolorer* create() {
            auto ret = new BoxRecolorer();
            if (ret->init()) {
                ret->autorelease();
                return ret;
            }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        bool init() override {
            if (!CCNode::init()) return false;
            this->setID("greybars-box-recolorer");
            this->schedule(schedule_selector(BoxRecolorer::tick), 0.1f);
            return true;
        }

        void tick(float) {
            this->recolorNow();
        }

        void recolorNow() {
            auto parent = this->getParent();
            if (!parent) return;
            auto color = Mod::get()->getSettingValue<ccColor3B>("jukebox-box-color");
            recolorJukeboxBoxes(parent, color);
        }
    };
}

class $modify(GreyBarsFLAlertLayer, FLAlertLayer) {
    void show() {
        FLAlertLayer::show();

        // Only act on the Jukebox popup, leave every other popup alone.
        auto name = geode::cocos::getObjectName(this);
        if (name.find("NongDropdownLayer") == std::string_view::npos) return;
        if (this->getChildByID("greybars-box-recolorer")) return;

        if (auto recolorer = BoxRecolorer::create()) {
            this->addChild(recolorer);
            recolorer->recolorNow();
        }
    }
};
