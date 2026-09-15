#pragma once
#include "entities/Actor.hpp"
#include "entities/Building.hpp"
#include "entities/Character.hpp"
#include "controllers/PlayerController.hpp"
#include "WorldContext.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

class Game : public WorldContext {
private:
    sf::Font font_;
    sf::RenderWindow& window_;
    sf::View worldView_;  // 游戏世界视图
    static constexpr float WORLD_WIDTH = 800.f;   // 游戏世界宽度
    static constexpr float WORLD_HEIGHT = 600.f;  // 游戏世界高度
    
    std::vector<std::unique_ptr<Actor>> actors_;    // 所有 Actor
    std::vector<std::unique_ptr<Building>> buildings_;  // 所有 Building
    std::vector<Renderable*> renderables_;
    std::vector<std::unique_ptr<Controller>> controllers_;
    sf::Clock clock_;
    float accumulator_ = 0.f;
    float fixed_dt = 0.1f / 120.f;
    
    // 帧率统计相关
    sf::Clock fpsClock_;
    int frameCount_ = 0;
    int current_fps_ = 0;
    // 显示模式相关
    bool show_details_ = false;
    bool isFullscreen_ = false;

    void controllersUpdate();
    void controllersWork();
    void fixedUpDate(float fixed_dt);
    void collision();
    void toScreen(float alpha, float dt);
    void draw();
    void updateFPS();
    void showFPS();
    void showCounts();
    void drawColliders();
    void onWindowResized();
public:
    Game(sf::RenderWindow& window);
    void run();
    
    // WorldContext接口实现
    sf::Vector2f getPlayerPosition() const override;
    Character* getPlayer() const override;
    std::vector<sf::Vector2f> getObstaclePositions() const override;
    bool isPositionOccupied(sf::Vector2f position) const override;
    sf::Vector2f getWorldBounds() const override;
};
