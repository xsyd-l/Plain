#pragma once
#include "Controller.hpp"
#include "../entities/Character.hpp"
#include "../WorldContext.hpp"
#include <vector>
#include <list>

class AIController : public Controller {
private:
    WorldContext *worldContext_;
    
    // AI状态
    enum class AIState {
        IDLE,
        CHASING,
        ATTACKING
    };
    AIState state_;
    
    // 寻路相关
    float detectionRange_;
    float attackRange_;
    
    // A*寻路节点
    struct PathNode {
        int x, y;
        int gCost; // 从起点到当前节点的代价
        int hCost; // 从当前节点到终点的预估代价
        int fCost() const { return gCost + hCost; }
        PathNode* parent;
        bool isBlocked;
        
        PathNode(int x_, int y_) : x(x_), y(y_), gCost(0), hCost(0), parent(nullptr), isBlocked(false) {}
    };
    
    static constexpr int GRID_CELL_SIZE = 32;  // 网格单元格大小
    std::vector<PathNode> currentPath_;  // 当前寻路路径
    int currentPathIndex_;  // 当前路径索引
    float pathUpdateTimer_;  // 路径更新计时器
    float pathUpdateInterval_;  // 路径更新间隔
    
    // A*寻路
    sf::Vector2f gridToWorld(int x, int y) const;
    sf::Vector2i worldToGrid(sf::Vector2f pos) const;
    std::vector<PathNode> findPath(sf::Vector2f start, sf::Vector2f end);
    int getDistance(PathNode* a, PathNode* b);
    PathNode* getNode(int x, int y, std::vector<std::vector<PathNode>>& grid);
    bool isWalkable(int x, int y, sf::Vector2f end);
    
    // 辅助函数
    sf::Vector2f getNextDirection();
    sf::Vector2f calculateDirectionToPlayer();
    bool isPlayerInRange();
    void updateAIState();
    void updatePath();
    
public:
    AIController(Character *character, WorldContext *worldContext);

    void update();
};