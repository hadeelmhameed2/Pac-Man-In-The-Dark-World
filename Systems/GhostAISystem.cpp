#include "GhostAISystem.h"
#include "Constants.h"
#include "GridMovement.h"
#include <cmath>
#include <limits>

namespace {
    struct DirChoice {
        float vx;
        float vy;
    };

    float entityCenterX(float x) {
        return x + TILE_SIZE * 0.5f;
    }

    float entityCenterY(float y) {
        return y + TILE_SIZE * 0.5f;
    }

    int tileCol(float x) {
        return static_cast<int>(std::floor(entityCenterX(x) / TILE_SIZE));
    }

    int tileRow(float y) {
        return static_cast<int>(std::floor(entityCenterY(y) / TILE_SIZE));
    }

    bool isAtTileCenter(float x, float y) {
        const float cx = entityCenterX(x);
        const float cy = entityCenterY(y);
        const float tx = std::fmod(std::fmod(cx, TILE_SIZE) + TILE_SIZE, TILE_SIZE);
        const float ty = std::fmod(std::fmod(cy, TILE_SIZE) + TILE_SIZE, TILE_SIZE);
        constexpr float eps = 4.0f;
        return std::abs(tx - TILE_SIZE * 0.5f) <= eps && std::abs(ty - TILE_SIZE * 0.5f) <= eps;
    }

    float distanceSq(float ax, float ay, float bx, float by) {
        const float dx = ax - bx;
        const float dy = ay - by;
        return dx * dx + dy * dy;
    }

    SDL_FPoint scatterTarget(int ghostType) {
        switch (ghostType) {
            case 0: return { WINDOW_WIDTH - TILE_SIZE, TILE_SIZE };
            case 1: return { TILE_SIZE, TILE_SIZE };
            case 2: return { WINDOW_WIDTH - TILE_SIZE, WINDOW_HEIGHT - TILE_SIZE };
            default: return { TILE_SIZE, WINDOW_HEIGHT - TILE_SIZE };
        }
    }

    SDL_FPoint pacmanLeadTarget(const PositionComponent& pacPos, const MovementComponent& pacMove, int tilesAhead) {
        const float pacCenterX = entityCenterX(pacPos.x);
        const float pacCenterY = entityCenterY(pacPos.y);
        const float lead = static_cast<float>(tilesAhead) * TILE_SIZE;

        if (pacMove.vx > 0.0f) return { pacCenterX + lead, pacCenterY };
        if (pacMove.vx < 0.0f) return { pacCenterX - lead, pacCenterY };
        if (pacMove.vy > 0.0f) return { pacCenterX, pacCenterY + lead };
        if (pacMove.vy < 0.0f) return { pacCenterX, pacCenterY - lead };
        return { pacCenterX, pacCenterY };
    }

    SDL_FPoint chaseTarget(
        int ghostType,
        const PositionComponent& pacPos,
        const MovementComponent& pacMove,
        const SDL_FPoint& blinkyPos,
        float distToPacmanSq
    ) {
        const float pacCenterX = entityCenterX(pacPos.x);
        const float pacCenterY = entityCenterY(pacPos.y);

        switch (ghostType) {
            case 0:
                return { pacCenterX, pacCenterY };
            case 1:
                return pacmanLeadTarget(pacPos, pacMove, 4);
            case 2: {
                const float leadX = pacmanLeadTarget(pacPos, pacMove, 2).x;
                const float leadY = pacmanLeadTarget(pacPos, pacMove, 2).y;
                return { leadX + (leadX - blinkyPos.x), leadY + (leadY - blinkyPos.y) };
            }
            case 3:
                if (distToPacmanSq > CLYDE_SHY_DISTANCE * CLYDE_SHY_DISTANCE) {
                    return { pacCenterX, pacCenterY };
                }
                return scatterTarget(3);
            default:
                return { pacCenterX, pacCenterY };
        }
    }

    bool isSameDirection(const DirChoice& dir, const MovementComponent& move) {
        return dir.vx == move.vx && dir.vy == move.vy;
    }

    void chooseDirection(MovementComponent& move, const SDL_FPoint& target, float posX, float posY, float speed) {
        const float cx = entityCenterX(posX);
        const float cy = entityCenterY(posY);

        const DirChoice options[] = {
            { 0.0f, -speed },
            { 0.0f, speed },
            { -speed, 0.0f },
            { speed, 0.0f }
        };

        auto tryPick = [&](bool allowReverse) {
            float bestDist = std::numeric_limits<float>::max();
            DirChoice best = options[0];

            for (const auto& dir : options) {
                if (!allowReverse) {
                    if (dir.vx == -move.vx && move.vx != 0.0f) continue;
                    if (dir.vy == -move.vy && move.vy != 0.0f) continue;
                }

                const float nextDist = distanceSq(
                    target.x, target.y,
                    cx + dir.vx * 0.05f, cy + dir.vy * 0.05f
                );

                if (nextDist < bestDist - 1.0f || (std::abs(nextDist - bestDist) <= 1.0f && isSameDirection(dir, move))) {
                    bestDist = nextDist;
                    best = dir;
                }
            }

            return best;
        };

        DirChoice best = tryPick(false);
        if (best.vx == 0.0f && best.vy == 0.0f) {
            best = tryPick(true);
        }

        move.vx = best.vx;
        move.vy = best.vy;
        enforceCardinalMovement(move);
    }

    void chooseFrightenedDirection(MovementComponent& move, int ghostId, int col, int row, float speed) {
        const DirChoice options[] = {
            { 0.0f, -speed },
            { 0.0f, speed },
            { -speed, 0.0f },
            { speed, 0.0f }
        };

        const float prevVx = move.vx;
        const float prevVy = move.vy;
        const int seed = col * 31 + row * 17 + ghostId * 13;
        int pick = std::abs(seed) % 4;

        if ((prevVx != 0.0f && options[pick].vx == -prevVx) || (prevVy != 0.0f && options[pick].vy == -prevVy)) {
            pick = (pick + 1) % 4;
        }

        move.vx = options[pick].vx;
        move.vy = options[pick].vy;
        enforceCardinalMovement(move);
    }

    void ensureGhostKeepsMoving(MovementComponent& move, const SDL_FPoint& target, float posX, float posY) {
        if (std::abs(move.vx) > 0.1f || std::abs(move.vy) > 0.1f) {
            return;
        }
        chooseDirection(move, target, posX, posY, move.speed);
    }
}

void GhostAISystem::update(bagel::ent_type pacmanId, float deltaTime) {
    static float modeTimer = 0.0f;
    modeTimer += deltaTime;

    const float cycle = GHOST_SCATTER_TIME + GHOST_CHASE_TIME;
    const float phase = std::fmod(modeTimer, cycle);
    const bool scatterPhase = phase < GHOST_SCATTER_TIME;

    static const bagel::Mask ghostMask = bagel::MaskBuilder()
        .set<GhostAI>()
        .set<PositionComponent>()
        .set<MovementComponent>()
        .build();

    static int q = bagel::World::createQuery(ghostMask);
    const auto& pacmanPos = bagel::World::getComponent<PositionComponent>(pacmanId);
    const auto& pacmanMove = bagel::World::getComponent<MovementComponent>(pacmanId);
    const float pacCenterX = entityCenterX(pacmanPos.x);
    const float pacCenterY = entityCenterY(pacmanPos.y);

    SDL_FPoint blinkyPos{ pacCenterX, pacCenterY };
    for (bagel::Entity other = bagel::World::first(q); !bagel::World::eof(q); other = bagel::World::next(q)) {
        if (other.get<GhostAI>().ghostType == 0) {
            const auto& blinky = other.get<PositionComponent>();
            blinkyPos = { entityCenterX(blinky.x), entityCenterY(blinky.y) };
            break;
        }
    }

    for (bagel::Entity e = bagel::World::first(q); !bagel::World::eof(q); e = bagel::World::next(q)) {
        auto& ai = e.get<GhostAI>();
        auto& pos = e.get<PositionComponent>();
        auto& move = e.get<MovementComponent>();

        move.speed = GHOST_SPEED;

        if (ai.state != GhostState::FRIGHTENED && ai.state != GhostState::EATEN) {
            ai.state = scatterPhase ? GhostState::SCATTER : GhostState::CHASE;
        }

        const float ghostCenterX = entityCenterX(pos.x);
        const float ghostCenterY = entityCenterY(pos.y);
        const float distToPacmanSq = distanceSq(ghostCenterX, ghostCenterY, pacCenterX, pacCenterY);

        switch (ai.state) {
            case GhostState::CHASE:
                ai.target = chaseTarget(ai.ghostType, pacmanPos, pacmanMove, blinkyPos, distToPacmanSq);
                break;
            case GhostState::SCATTER:
                ai.target = scatterTarget(ai.ghostType);
                break;
            case GhostState::EATEN:
                ai.target = { GHOST_HOUSE_X, GHOST_HOUSE_Y };
                break;
            case GhostState::FRIGHTENED:
                break;
        }

        const int col = tileCol(pos.x);
        const int row = tileRow(pos.y);
        const bool atIntersection = isAtTileCenter(pos.x, pos.y);
        const bool enteredNewTile = col != ai.lastTurnTileCol || row != ai.lastTurnTileRow;
        const bool mustTurn = std::abs(move.vx) < 0.1f && std::abs(move.vy) < 0.1f;

        if (mustTurn || (atIntersection && enteredNewTile)) {
            if (ai.state == GhostState::FRIGHTENED) {
                chooseFrightenedDirection(move, e.entity().id, col, row, move.speed);
            } else {
                chooseDirection(move, ai.target, pos.x, pos.y, move.speed);
            }

            ai.lastTurnTileCol = col;
            ai.lastTurnTileRow = row;
        }

        ensureGhostKeepsMoving(move, ai.target, pos.x, pos.y);
        enforceCardinalMovement(move);
        snapEntityToGridLane(pos, move);
    }
}
