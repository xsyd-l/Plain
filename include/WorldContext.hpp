#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

class Character;

// 世界上下文接口，用于AI获取世界信息
class WorldContext {
public:
    virtual ~WorldContext() = default;
    
    // 获取玩家位置
    virtual sf::Vector2f getPlayerPosition() const = 0;
    
    // 获取玩家指针
    virtual Character* getPlayer() const = 0;
    
    // 获取所有障碍物的位置（用于寻路）
    virtual std::vector<sf::Vector2f> getObstaclePositions() const = 0;
    
    // 检查某个位置是否被占用
    virtual bool isPositionOccupied(sf::Vector2f position) const = 0;
    
    // 获取世界边界
    virtual sf::Vector2f getWorldBounds() const = 0;
};