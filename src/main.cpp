#include "BufferFrame.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(BFGameLayer, GJBaseGameLayer) {
    // Registra tus clicks y toma la foto del juego al primer salto.
    void handleButton(bool down, int button, bool isPlayer1) {
        auto pl = PlayLayer::get();
        bool isPlayLayer = pl && static_cast<GJBaseGameLayer*>(pl) == static_cast<GJBaseGameLayer*>(this);

        if (!isPlayLayer || !bf::enabled() || bf::g.injecting) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto& g = bf::g;
        if (g.phase == bf::Phase::Idle) {
            if (down && button == 1 && pl->m_player1 && !pl->m_player1->m_isDead) {
                bf::beginWatch(pl, button, isPlayer1);
            }
        }
        else if (g.phase == bf::Phase::Watching) {
            g.inputs.push_back({bf::tick(pl) - g.baseTick, down, button, isPlayer1});
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    // Durante la repetición, mete los inputs guardados en su tick (con el retraso).
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto pl = PlayLayer::get();
        auto& g = bf::g;

        if (pl && bf::enabled() && g.phase == bf::Phase::Replaying && !g.rewindPending
            && static_cast<GJBaseGameLayer*>(pl) == static_cast<GJBaseGameLayer*>(this)) {
            unsigned int rel = bf::tick(pl) - g.baseTick;
            while (g.nextInput < g.inputs.size() && g.inputs[g.nextInput].offset + g.delay <= rel) {
                auto input = g.inputs[g.nextInput++];
                g.injecting = true;
                GJBaseGameLayer::handleButton(input.down, input.button, input.player1);
                g.injecting = false;
            }
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(BFPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        bf::reset();

        if (bf::enabled()) {
            // Cheat de prueba: el intento no guarda progreso ni se puede enviar.
            m_isTestMode = true;

            if (Mod::get()->getSettingValue<bool>("show-label")) {
                auto winSize = CCDirector::get()->getWinSize();
                auto label = CCLabelBMFont::create("Buffer Frame - modo prueba", "bigFont.fnt");
                label->setScale(0.3f);
                label->setOpacity(110);
                label->setAnchorPoint({0.f, 1.f});
                label->setPosition({6.f, winSize.height - 6.f});
                this->addChild(label, 1000);
            }

            bf::warnOnce();
        }
        return true;
    }

    // Aquí se hace el "viaje atrás" fuera del medio de un paso de física.
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        auto& g = bf::g;
        if (!bf::enabled()) {
            if (g.phase != bf::Phase::Idle) bf::reset();
            return;
        }

        if (g.rewindPending) {
            bf::rewind(this);
            return;
        }

        if (g.phase != bf::Phase::Idle) {
            unsigned int rel = bf::tick(this) - g.baseTick;
            if (rel > bf::lookahead() + g.delay) {
                // Sobreviviste toda la ventana: el click (con su retraso) fue bueno.
                bf::reset();
            }
        }
    }

    // Si mueres cerca de un click, se cancela la muerte y se reintenta con más retraso.
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        auto& g = bf::g;

        if (bf::enabled() && g.phase != bf::Phase::Idle && g.snapshot
            && object != m_anticheatSpike) {
            if (g.rewindPending) return;
            if (g.delay < bf::maxDelay()) {
                g.rewindPending = true;
                return;
            }
        }

        PlayLayer::destroyPlayer(player, object);
        bf::reset();
    }

    void resetLevel() {
        bf::reset();
        PlayLayer::resetLevel();
        if (bf::enabled()) m_isTestMode = true;
    }

    void onQuit() {
        bf::reset();
        PlayLayer::onQuit();
    }

    void levelComplete() {
        bf::reset();
        PlayLayer::levelComplete();
    }
};
