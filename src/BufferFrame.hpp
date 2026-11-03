#pragma once

#include <Geode/Geode.hpp>
#include <vector>

// Estado y utilidades del corrector de clicks.
//
// Idea general:
//   1. Cuando das un click (salto) se guarda una foto del juego (checkpoint interno).
//   2. Si mueres dentro de la ventana de revisión, se cancela la muerte, se vuelve a
//      la foto y se repite todo con el click retrasado 1, 2, 3... ticks.
//   3. Se queda con el primer retraso con el que sobrevives. Si ninguno funciona
//      (o llegas al retraso máximo), mueres normal.
namespace bf {
    using namespace geode::prelude;

    struct Input {
        unsigned int offset; // ticks desde la foto
        bool down;
        int button;
        bool player1;
    };

    enum class Phase {
        Idle,      // sin nada que vigilar
        Watching,  // click real dado, vigilando si mueres
        Replaying  // repitiendo los inputs con retraso
    };

    struct State {
        Phase phase = Phase::Idle;
        CheckpointObject* snapshot = nullptr;
        unsigned int baseTick = 0;
        unsigned int delay = 0;
        std::vector<Input> inputs;
        std::size_t nextInput = 0;
        bool rewindPending = false;
        bool injecting = false;
    };

    inline State g;

    inline bool enabled() {
        return Mod::get()->getSettingValue<bool>("enabled");
    }

    inline unsigned int maxDelay() {
        return static_cast<unsigned int>(Mod::get()->getSettingValue<int64_t>("max-delay"));
    }

    inline unsigned int lookahead() {
        return static_cast<unsigned int>(Mod::get()->getSettingValue<int64_t>("lookahead"));
    }

    inline unsigned int tick(GJBaseGameLayer* layer) {
        return layer->m_gameState.m_currentProgress;
    }

    inline void reset() {
        if (g.snapshot) {
            g.snapshot->release();
            g.snapshot = nullptr;
        }
        g.phase = Phase::Idle;
        g.baseTick = 0;
        g.delay = 0;
        g.inputs.clear();
        g.nextInput = 0;
        g.rewindPending = false;
        g.injecting = false;
    }

    // Toma la foto en el momento del click y empieza a vigilar.
    inline void beginWatch(PlayLayer* pl, int button, bool player1) {
        reset();
        auto cp = pl->createCheckpoint();
        if (!cp) return;
        cp->retain();
        g.snapshot = cp;
        g.phase = Phase::Watching;
        g.baseTick = tick(pl);
        g.inputs.push_back({0, true, button, player1});
    }

    // Vuelve a la foto y prepara la repetición con un tick más de retraso.
    inline void rewind(PlayLayer* pl) {
        g.rewindPending = false;
        if (!g.snapshot) {
            reset();
            return;
        }
        ++g.delay;
        pl->loadFromCheckpoint(g.snapshot);
        g.baseTick = tick(pl);
        g.nextInput = 0;
        g.phase = Phase::Replaying;
    }

    // Aviso que se muestra una sola vez por sesión.
    inline void warnOnce() {
        static bool shown = false;
        if (shown) return;
        shown = true;
        Loader::get()->queueInMainThread([] {
            FLAlertLayer::create(
                "Buffer Frame",
                "Este mod es un <cr>cheat</c> de prueba.\n"
                "<cy>No lo uses para rankeds, listas ni verificaciones.</c>\n"
                "Mientras esté activo, los niveles se juegan en modo prueba y no se guarda progreso.",
                "OK"
            )->show();
        });
    }
}
