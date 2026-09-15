#include "../include/controllers/AIController.hpp"
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <algorithm>
#include <limits>

AIController::AIController(Character *character, WorldContext *worldContext)
    : Controller(ControllerType::ai, character), worldContext_(worldContext), state_(AIState::IDLE),
      detectionRange_(1000.f), attackRange_(50.f), currentPathIndex_(0),
      pathUpdateTimer_(0.f), pathUpdateInterval_(0.5f) {
    getIntention().setZero();
}

// A*寻路：计算两个节点间的距离
int AIController::getDistance(PathNode* a, PathNode* b) {
    int dstX = std::abs(a->x - b->x);
    int dstY = std::abs(a->y - b->y);
    if (dstX > dstY) {
        return 14 * dstY + 10 * (dstX - dstY);
    }
    return 14 * dstX + 10 * (dstY - dstX);
}

// A*寻路：网格坐标转到向量坐标
sf::Vector2f AIController::gridToWorld(int x, int y) const {
    return sf::Vector2f(x * GRID_CELL_SIZE + GRID_CELL_SIZE / 2.f,
                         y * GRID_CELL_SIZE + GRID_CELL_SIZE / 2.f);
}

// A*寻路：世界坐标转到网格坐标
sf::Vector2i AIController::worldToGrid(sf::Vector2f pos) const {
    return sf::Vector2i(static_cast<int>(pos.x / GRID_CELL_SIZE),
                        static_cast<int>(pos.y / GRID_CELL_SIZE));
}

// 获取对应的node，不存在则返回nullptr
AIController::PathNode* AIController::getNode(int x, int y, std::vector<std::vector<PathNode>>& grid) {
    if (x < 0 || y < 0 || x >= static_cast<int>(grid.size()) || y >= static_cast<int>(grid[0].size())) {
        return nullptr;
    }
    return &grid[x][y];
}

// 检查该位置是否可通行
bool AIController::isWalkable(int x, int y, sf::Vector2f end) {
    if (!worldContext_) return true;
    
    sf::Vector2f worldPos = gridToWorld(x, y);
    
    // 终点位置总是可通行的（AI要到达玩家位置）
    sf::Vector2i endGrid = worldToGrid(end);
    if (x == endGrid.x && y == endGrid.y) {
        return true;
    }
    
    return !worldContext_->isPositionOccupied(worldPos);
}

// A*寻路主算法（改进版：初始化代价，并限制搜索半径）
std::vector<AIController::PathNode> AIController::findPath(sf::Vector2f start, sf::Vector2f end) {
    std::vector<PathNode> emptyPath;

    sf::Vector2i startGrid = worldToGrid(start);
    sf::Vector2i endGrid = worldToGrid(end);

    if (startGrid.x == endGrid.x && startGrid.y == endGrid.y) {
        return emptyPath;
    }

    // 获取世界边界
    sf::Vector2f worldBounds(800.f, 600.f);
    if (worldContext_) {
        worldBounds = worldContext_->getWorldBounds();
    }

    int gridCols = static_cast<int>(worldBounds.x) / GRID_CELL_SIZE + 1;
    int gridRows = static_cast<int>(worldBounds.y) / GRID_CELL_SIZE + 1;

    // 最大搜索半径（以格子计）: 将 detectionRange_ 限制为搜索半径
    int maxSearchCells = static_cast<int>(std::ceil(detectionRange_ / static_cast<float>(GRID_CELL_SIZE)));

    // 创建网格并初始化
    std::vector<std::vector<PathNode>> grid(gridCols, std::vector<PathNode>(gridRows, PathNode(0, 0)));
    for (int ix = 0; ix < gridCols; ++ix) {
        for (int iy = 0; iy < gridRows; ++iy) {
            grid[ix][iy] = PathNode(ix, iy);
            grid[ix][iy].isBlocked = !isWalkable(ix, iy, end);
            grid[ix][iy].gCost = std::numeric_limits<int>::max();
            grid[ix][iy].hCost = 0;
            grid[ix][iy].parent = nullptr;
        }
    }

    PathNode* startNode = getNode(startGrid.x, startGrid.y, grid);
    PathNode* endNode = getNode(endGrid.x, endGrid.y, grid);

    if (!startNode || !endNode || startNode->isBlocked) {
        return emptyPath;
    }

    // 启动A*
    std::vector<PathNode*> openSet;
    std::vector<PathNode*> closedSet;

    startNode->gCost = 0;
    startNode->hCost = getDistance(startNode, endNode);
    openSet.push_back(startNode);

    while (!openSet.empty()) {
        // 找到fCost最小的节点
        PathNode* currentNode = openSet[0];
        int currentIndex = 0;
        for (int i = 1; i < static_cast<int>(openSet.size()); ++i) {
            if (openSet[i]->fCost() < currentNode->fCost() ||
                (openSet[i]->fCost() == currentNode->fCost() && openSet[i]->hCost < currentNode->hCost)) {
                currentNode = openSet[i];
                currentIndex = i;
            }
        }

        // 移除当前节点并加入 closedSet
        openSet.erase(openSet.begin() + currentIndex);
        closedSet.push_back(currentNode);

        // 到达终点
        if (currentNode == endNode) {
            std::vector<PathNode> path;
            PathNode* node = endNode;
            while (node != nullptr && node != startNode) {
                path.push_back(*node);
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        // 遍历邻居（8向）
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;

                int nx = currentNode->x + dx;
                int ny = currentNode->y + dy;

                // 越界或不可通行
                PathNode* neighbor = getNode(nx, ny, grid);
                if (!neighbor || neighbor->isBlocked) continue;

                // 限制搜索半径：如果 neighbor 超出从 start 的 maxSearchCells，则跳过
                int relX = std::abs(neighbor->x - startGrid.x);
                int relY = std::abs(neighbor->y - startGrid.y);
                if (relX > maxSearchCells || relY > maxSearchCells) continue;

                // 如果已经在 closedSet 中，跳过
                bool inClosed = false;
                for (const auto& n : closedSet) { if (n == neighbor) { inClosed = true; break; } }
                if (inClosed) continue;

                // 对角穿越检查
                if (dx != 0 && dy != 0) {
                    PathNode* n1 = getNode(currentNode->x + dx, currentNode->y, grid);
                    PathNode* n2 = getNode(currentNode->x, currentNode->y + dy, grid);
                    if ((n1 && n1->isBlocked) || (n2 && n2->isBlocked)) continue;
                }

                int moveCost = (dx != 0 && dy != 0) ? 14 : 10;
                int tentativeG = currentNode->gCost + moveCost;

                // 如果发现更优路径，更新 neighbor
                if (tentativeG < neighbor->gCost) {
                    neighbor->gCost = tentativeG;
                    neighbor->hCost = getDistance(neighbor, endNode);
                    neighbor->parent = currentNode;

                    // 如果不在 openSet 中则加入
                    bool inOpen = false;
                    for (auto* p : openSet) { if (p == neighbor) { inOpen = true; break; } }
                    if (!inOpen) openSet.push_back(neighbor);
                }
            }
        }
    }

    // 未找到路径
    return emptyPath;
}

void AIController::updatePath() {
    if (!worldContext_) return;
    
    pathUpdateTimer_ += 0.016f; // 假设 ~60fps
    
    if (pathUpdateTimer_ >= pathUpdateInterval_) {
        pathUpdateTimer_ = 0.f;
        
        sf::Vector2f start = getCharacter()->getCenter();
        // 如果没有玩家或玩家超出检测范围，则不进行全场寻路
        Character* player = worldContext_->getPlayer();
        if (!player) {
            currentPath_.clear();
            currentPathIndex_ = 0;
            return;
        }

        sf::Vector2f end = worldContext_->getPlayerPosition();
        // 计算与玩家距离
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float distSqr = dx * dx + dy * dy;
        if (distSqr > detectionRange_ * detectionRange_) {
            // 玩家在检测范围外，清空路径并返回
            currentPath_.clear();
            currentPathIndex_ = 0;
            return;
        }

        currentPath_ = findPath(start, end);
        currentPathIndex_ = 0;
    }
}

sf::Vector2f AIController::getNextDirection() {
    if (currentPath_.empty() || currentPathIndex_ >= static_cast<int>(currentPath_.size())) {
        // 没有路径，直接走向玩家
        return calculateDirectionToPlayer();
    }
    
    // 获取下一个路径点
    sf::Vector2f nextWaypoint = gridToWorld(currentPath_[currentPathIndex_].x,
                                            currentPath_[currentPathIndex_].y);
    sf::Vector2f myPos = getCharacter()->getCenter();
    
    sf::Vector2f toWaypoint = nextWaypoint - myPos;
    float dist = std::sqrt(toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y);
    
    // 到达当前路径点，前进到下一个
    if (dist < GRID_CELL_SIZE * 0.5f) {
        ++currentPathIndex_;
        if (currentPathIndex_ >= static_cast<int>(currentPath_.size())) {
            return calculateDirectionToPlayer();
        }
        nextWaypoint = gridToWorld(currentPath_[currentPathIndex_].x,
                                   currentPath_[currentPathIndex_].y);
        toWaypoint = nextWaypoint - myPos;
        dist = std::sqrt(toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y);
    }
    
    if (dist > 0.f) {
        return toWaypoint / dist;
    }
    return sf::Vector2f(0.f, 0.f);
}

sf::Vector2f AIController::calculateDirectionToPlayer() {
    if (!worldContext_) {
        return sf::Vector2f(0.f, 0.f);
    }
    
    sf::Vector2f playerPos = worldContext_->getPlayerPosition();
    sf::Vector2f myPos = getCharacter()->getCenter();
    
    sf::Vector2f direction = playerPos - myPos;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length > 0.f) {
        direction.x /= length;
        direction.y /= length;
    }
    
    return direction;
}

bool AIController::isPlayerInRange() {
    if (!worldContext_) {
        return false;
    }
    
    sf::Vector2f playerPos = worldContext_->getPlayerPosition();
    sf::Vector2f myPos = getCharacter()->getCenter();
    
    float distance = std::sqrt(
        (playerPos.x - myPos.x) * (playerPos.x - myPos.x) +
        (playerPos.y - myPos.y) * (playerPos.y - myPos.y)
    );
    
    return distance <= attackRange_;
}

void AIController::updateAIState() {
    if (!worldContext_) {
        state_ = AIState::IDLE;
        return;
    }
    
    Character* player = worldContext_->getPlayer();
    if (!player) {
        state_ = AIState::IDLE;
        return;
    }
    
    sf::Vector2f playerPos = worldContext_->getPlayerPosition();
    sf::Vector2f myPos = getCharacter()->getCenter();
    
    float distance = std::sqrt(
        (playerPos.x - myPos.x) * (playerPos.x - myPos.x) +
        (playerPos.y - myPos.y) * (playerPos.y - myPos.y)
    );
    
    if (distance <= attackRange_) {
        state_ = AIState::ATTACKING;
    } else if (distance <= detectionRange_) {
        state_ = AIState::CHASING;
    } else {
        state_ = AIState::IDLE;
    }
}

void AIController::update() {
    // 重置意图
    getIntention().setZero();
    
    // 更新路径
    updatePath();
    
    // 更新AI状态
    updateAIState();
    
    // 计算瞄准方向（指向玩家）
    if (worldContext_) {
        Character* player = worldContext_->getPlayer();
        if (player) {
            getIntention().aimDir = player->getCenter() - getCharacter()->getCenter();
        } else {
            getIntention().aimDir = sf::Vector2f(0.f, 0.f);
        }
    }
    
    switch (state_) {
        case AIState::IDLE:
            // 闲置状态，不移动也不攻击
            break;
            
        case AIState::CHASING: {
            // 追逐玩家，跟随A*路径
            sf::Vector2f direction = getNextDirection();
            getIntention().accele = direction;
            break;
        }
            
        case AIState::ATTACKING: {
            // 攻击玩家
            getIntention().attack = true;
            getIntention().accele = sf::Vector2f(0.f, 0.f); // 攻击时停止移动
            break;
        }
    }
}