#include "GhostAISystem.h"
#include "Constants.h"
#include "GridMovement.h"
#include "Maze.h"
#include <cmath>
#include <limits>
#include <vector>

namespace {
    struct DirChoice {
        float vx;
        float vy;
        int nextCol;
        int nextRow;
    };

    float entityCenterX(float x) {
        return x + 16.0f;
    }

    float entityCenterY(float y) {
        return y + 16.0f;
    }

    int tileCol(float x) {
        return static_cast<int>(std::floor((entityCenterX(x) - MAZE_START_X) / MAZE_TILE_SIZE));
    }

    int tileRow(float y) {
        return static_cast<int>(std::floor((entityCenterY(y) - MAZE_START_Y) / MAZE_TILE_SIZE));
    }

    bool isAtTileCenter(float x, float y) {
        const float cx = entityCenterX(x);
        const float cy = entityCenterY(y);
        const float rx = cx - MAZE_START_X;
        const float ry = cy - MAZE_START_Y;
        const float tx = std::fmod(std::fmod(rx, MAZE_TILE_SIZE) + MAZE_TILE_SIZE, MAZE_TILE_SIZE);
        const float ty = std::fmod(std::fmod(ry, MAZE_TILE_SIZE) + MAZE_TILE_SIZE, MAZE_TILE_SIZE);
        constexpr float eps = 4.0f;
        return std::abs(tx - MAZE_TILE_SIZE * 0.5f) <= eps && std::abs(ty - MAZE_TILE_SIZE * 0.5f) <= eps;
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

    void chooseDirection(MovementComponent& move, const SDL_FPoint& target, float posX, float posY, float speed) {
        const float cx = entityCenterX(posX);
        const float cy = entityCenterY(posY);
        const int col = static_cast<int>(std::floor((cx - MAZE_START_X) / MAZE_TILE_SIZE));
        const int row = static_cast<int>(std::floor((cy - MAZE_START_Y) / MAZE_TILE_SIZE));

        const DirChoice options[] = {
            { 0.0f, -speed, col, row - 1 },
            { 0.0f, speed, col, row + 1 },
            { -speed, 0.0f, col - 1, row },
            { speed, 0.0f, col + 1, row }
        };

        std::vector<DirChoice> validChoices;
        std::vector<DirChoice> reverseChoices;

        for (const auto& opt : options) {
            if (isWall(opt.nextCol, opt.nextRow)) {
                continue;
            }
            const bool isReverse = (opt.vx == -move.vx && move.vx != 0.0f) || (opt.vy == -move.vy && move.vy != 0.0f);
            if (isReverse) {
                reverseChoices.push_back(opt);
            } else {
                validChoices.push_back(opt);
            }
        }

        const auto& choices = validChoices.empty() ? reverseChoices : validChoices;

        if (choices.empty()) {
            move.vx = 0.0f;
            move.vy = 0.0f;
            return;
        }

        float bestDist = std::numeric_limits<float>::max();
        DirChoice best = choices[0];

        for (const auto& opt : choices) {
            const float nextCenterX = MAZE_START_X + opt.nextCol * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
            const float nextCenterY = MAZE_START_Y + opt.nextRow * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
            const float nextDist = distanceSq(target.x, target.y, nextCenterX, nextCenterY);

            const bool isSameDir = (opt.vx == move.vx && opt.vy == move.vy);
            if (nextDist < bestDist - 1.0f || (std::abs(nextDist - bestDist) <= 1.0f && isSameDir)) {
                bestDist = nextDist;
                best = opt;
            }
        }

        move.vx = best.vx;
        move.vy = best.vy;
        enforceCardinalMovement(move);
    }

    void chooseFrightenedDirection(MovementComponent& move, int ghostId, int col, int row, float speed) {
        const DirChoice options[] = {
            { 0.0f, -speed, col, row - 1 },
            { 0.0f, speed, col, row + 1 },
            { -speed, 0.0f, col - 1, row },
            { speed, 0.0f, col + 1, row }
        };

        std::vector<DirChoice> validChoices;
        std::vector<DirChoice> reverseChoices;

        for (const auto& opt : options) {
            if (isWall(opt.nextCol, opt.nextRow)) {
                continue;
            }
            const bool isReverse = (opt.vx == -move.vx && move.vx != 0.0f) || (opt.vy == -move.vy && move.vy != 0.0f);
            if (isReverse) {
                reverseChoices.push_back(opt);
            } else {
                validChoices.push_back(opt);
            }
        }

        const auto& choices = validChoices.empty() ? reverseChoices : validChoices;

        if (choices.empty()) {
            move.vx = 0.0f;
            move.vy = 0.0f;
            return;
        }

        const int seed = col * 31 + row * 17 + ghostId * 13;
        int pick = std::abs(seed) % choices.size();

        move.vx = choices[pick].vx;
        move.vy = choices[pick].vy;
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
