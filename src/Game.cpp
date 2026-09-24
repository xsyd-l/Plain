#include "../include/Game.hpp"
#include "../include/controllers/AIController.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../include/core/collision.hpp"
#include "../include/spark/Spark.hpp"
#include <functional>

Game::Game(sf::RenderWindow &window):window_(window) {
    worldView_.setSize(sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT));
    worldView_.setCenter(sf::Vector2f(WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f));
    onWindowResized();  // 初始化视口，确保初始窗口比例正确
    if (!font_.openFromFile("./fonts/AliPuHui.ttf")) {
        std::cerr << "Failed to load font file." << std::endl;
    }
}

void Game::run() {
    // 创建玩家角色,添加到相关管理
    auto character = std::make_unique<Character>(sf::Vector2f{400.f, 300.f}, 8.f, "./texture/player.png");
    character->setFootHold(sf::Vector2f(16.f, 32.f));
    Character* character_ptr = character.get();
    character_ptr->addAnimation(AnimationState::IDLE, {0, 0}, 5, 0, 0, 0.2f, {1.f, 1.f}, {16.f, 32.f});
    character_ptr->addAnimation(AnimationState::MOVING,{1, 0}, 5, 0, 4, 0.2f, {1.f, 1.f}, {16.f, 32.f});//向右移动动画
    character_ptr->addAnimation(AnimationState::MOVING,{-1, 0}, 5, 0, 4, 0.2f, {-1.f, 1.f}, {16.f, 32.f});//向下移动动画
    actors_.push_back(std::move(character));
    renderables_.push_back(character_ptr); 
    controllers_.push_back(std::make_unique<PlayerController>(character_ptr, window_));
    
    // 创建文字提示SoA
    SparkSoA sparkSoA("./fonts/AliPuHui.ttf");
    // 暴露弹幕添加指针（使用 lambda 以确保类型匹配）
    spawnSpark = [&sparkSoA](SparkType type, std::string content) {
        sparkSoA.spawnSpark(type, content);
    };

    while (window_.isOpen()) {
        float frametime = clock_.restart().asSeconds();
        accumulator_ += frametime;
        // frametime起作用的地点：
        // 1、物理的帧位置插值
        // 2、动画的帧选择
        // 3、数字弹幕的运动状态更新
        
        //事件处理
        while (auto event = window_.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window_.close();
            }
            // 处理窗口大小改变
            if (event->getIf<sf::Event::Resized>()) {
                onWindowResized();
            }
            if (auto keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                // F11 切换全屏
                if (keyPressed->code == sf::Keyboard::Key::F11) {
                    if (window_.hasFocus()) {
                        if (isFullscreen_) {
                            // 切换回窗口模式
                            window_.create(sf::VideoMode({800, 600}), "Plain");
                            isFullscreen_ = false;
                        } else {
                            // 切换到全屏模式
                            window_.create(sf::VideoMode::getDesktopMode(), "Plain", sf::State::Fullscreen);
                            isFullscreen_ = true;
                        }
                        // 重新初始化视图
                        worldView_.setSize(sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT));
                        worldView_.setCenter(sf::Vector2f(WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f));
                        // 重新计算视口
                        onWindowResized();
                    }
                }
                // F3键切换参数显示
                if (keyPressed->code == sf::Keyboard::Key::F3) {
                    show_details_ = !show_details_;
                }
                
                sf::Vector2f mousePos = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
                
                // 先对齐到网格（与 createBuilding 中的处理一致）
                mousePos.x = (std::floor(mousePos.x / 16.0f) * 16.0f) + 8.0f;
                mousePos.y = (std::floor(mousePos.y / 16.0f) * 16.0f) + 8.0f;
                
                // 检测对齐后的位置是否被占用
                bool canPlace = true;
                for (const auto& actor : actors_) {
                    if (actor->containDot(mousePos)) {
                        canPlace = false;
                        break;
                    }
                }
                if (canPlace) {
                    for (const auto& building : buildings_) {
                        if (building->containDot(mousePos)) {
                            canPlace = false;
                            break;
                        }
                    }
                }
                
                if (canPlace) {
                    if (keyPressed->code == sf::Keyboard::Key::Num1) {
                        auto building = createBuilding(BuildingID::tree, mousePos);
                        Building* building_ptr = building.get();
                        buildings_.push_back(std::move(building));
                        renderables_.push_back(building_ptr);  // 添加到 renderables_
                    }
                    if (keyPressed->code == sf::Keyboard::Key::Num2) {
                        auto building = createBuilding(BuildingID::stone, mousePos);
                        Building* building_ptr = building.get();
                        buildings_.push_back(std::move(building));
                        renderables_.push_back(building_ptr);
                    }
                    if (keyPressed->code == sf::Keyboard::Key::Num3) {
                        // 创建AI角色
                        auto aiCharacter = std::make_unique<Character>(mousePos, 8.f, "./texture/ai.png");
                        Character* aiCharacter_ptr = aiCharacter.get();
                        actors_.push_back(std::move(aiCharacter));
                        renderables_.push_back(aiCharacter_ptr);
                        // 添加AI控制器
                        controllers_.push_back(std::make_unique<AIController>(aiCharacter_ptr, this));
                    }
                }
            }
        }

        //控制器生成意图
        controllersUpdate();

        while (accumulator_ >= fixed_dt) {
            controllersWork();
            fixedUpDate(fixed_dt);
            collision();
            accumulator_ -= fixed_dt;
        }
        float alpha = accumulator_ / fixed_dt;
        toScreen(alpha, frametime);
        draw();

        // 每SPARK_UPDATE_DT的时间更新一次弹幕的运动状态，采用插值的方式
        sparkSoA.accumulator_ += frametime;
        while (sparkSoA.accumulator_ >= SPARK_UPDATE_DT) {
            sparkSoA.accumulator_ -= SPARK_UPDATE_DT;
            
        }

        // 处理信息显示
        // 帧率和所有游戏参数相关的ui要在最顶层打印
        updateFPS();
        if (show_details_) {
            showFPS();
            showCounts();
        }
        window_.display();
    }
}

void Game::controllersUpdate() {
    for (const auto &ptr : controllers_) {
        ptr->update();
    }
}

void Game::controllersWork() {
    for (const auto &ptr : controllers_) {
        Character *character = ptr->getCharacter();
        Intention &intention = ptr->getIntention();
        
        // 使用intention.aimDir作为攻击方向
        sf::Vector2f direction = intention.aimDir;
        
        character->accelerate(intention.accele * 4000.f);
        if (intention.attack) {
            for (auto &ptr_victim : controllers_) {
                if (&ptr == &ptr_victim) {
                    continue;
                }
                // 根据victimType过滤攻击目标
                if (intention.victimType != ptr_victim->getType()) {
                    continue;
                }
                character->attack(*(ptr_victim->getCharacter()), direction);
            }
        }
    }
}

void Game::fixedUpDate(float fixed_dt) {
    for (auto &ptr : actors_) {
        ptr->fixedUpDate(fixed_dt);
    }
}

void Game::collision() {
    // Actor vs Actor
    for (std::size_t i = 0; i < actors_.size(); ++i) {
        for (std::size_t j = i + 1; j < actors_.size(); ++j) {
            const collider& colA = actors_[i]->getCollider();
            const collider& colB = actors_[j]->getCollider();
            
            if (itrsctng(colA, colB)) {
                sf::Vector2f sepVec = sprtAFrmB(colA, colB);
                actors_[i]->move(sepVec * 0.5f);
                actors_[j]->move(-sepVec * 0.5f);
            }
        }
    }
    
    // Actor vs Building
    for (auto& actor : actors_) {
        for (const auto& building : buildings_) {
            const collider& colA = actor->getCollider();
            const collider& colB = building->getCollider();
            
            if (itrsctng(colA, colB)) {
                sf::Vector2f sepVec = sprtAFrmB(colA, colB);
                actor->move(sepVec);
            }
        }
    }
}

void Game::toScreen(float alpha, float dt) {
    for (auto &ptr : actors_) {
        ptr->toScreen(alpha, dt);
    }
    for (auto &ptr : buildings_) {
        ptr->toScreen(alpha, dt);
    }
}

void Game::draw() {
    window_.clear(sf::Color::Black);

    // 更新视图中心为角色位置（第一帧获取角色指针）
    static Character* player = nullptr;
    if (!player && !actors_.empty()) {
        player = dynamic_cast<Character*>(actors_.front().get());
    }
    
    // 如果有玩家，更新视图中心跟随玩家（不做边界限制）
    if (player) {
        sf::Vector2f playerPos = player->getCenter();
        worldView_.setCenter(playerPos);
    }
    
    // 应用游戏世界视图（必须在绘制任何世界物体之前设置）
    window_.setView(worldView_);

    // 绘制世界背景（在 worldView_ 坐标系下）
    sf::Vector2f viewCenter = worldView_.getCenter();
    sf::Vector2f viewSize   = worldView_.getSize();
    sf::Vector2f viewTopLeft = viewCenter - viewSize / 2.0f;
    
    sf::RectangleShape worldBg(viewSize);
    worldBg.setFillColor(sf::Color::White);
    worldBg.setPosition(viewTopLeft);
    window_.draw(worldBg);
    
    // 使用稳定排序，保持相同 renderOrder 的对象相对顺序
    std::stable_sort(renderables_.begin(), renderables_.end(), [](Renderable* r1, Renderable* r2) {
        return r1->getRenderOrder() < r2->getRenderOrder();
    });
    
    // 渲染
    for (const auto& renderable : renderables_) {
        window_.draw(renderable->getDrawable());
    }
    
    // 绘制碰撞箱
    if (show_details_) {
        drawColliders();
    }
}

void Game::updateFPS() {
    frameCount_++;
    float elapsed = fpsClock_.getElapsedTime().asSeconds();
    if (elapsed >= 1.0f) {
        current_fps_ = frameCount_ / elapsed;
        frameCount_ = 0;
        fpsClock_.restart();
    }
}

void Game::showFPS() {
    std::string msg = "fps:" + std::to_string(current_fps_);
    sf::Text fps(font_, msg, 16u);
    fps.setFillColor(sf::Color::White);
    fps.setOutlineColor(sf::Color::Black);
    fps.setOutlineThickness(2.f);

    // 切换到默认视图，在屏幕空间绘制 UI（避免随世界视图移动）
    sf::View prevView = window_.getView();
    window_.setView(window_.getDefaultView());

    fps.setPosition({10.f, 10.f});
    window_.draw(fps);

    // 恢复之前的视图
    window_.setView(prevView);
}

void Game::showCounts() {
    std::string msg = "actors:" + std::to_string(actors_.size())
                    + " buildings:" + std::to_string(buildings_.size());
    sf::Text info(font_, msg, 16u);
    info.setFillColor(sf::Color::White);
    info.setOutlineColor(sf::Color::Black);
    info.setOutlineThickness(2.f);

    sf::View prevView = window_.getView();
    window_.setView(window_.getDefaultView());

    info.setPosition({10.f, 30.f});
    window_.draw(info);

    window_.setView(prevView);
}

void Game::drawColliders() {
    sf::Color colliderColor(255, 100, 100, 128);  // 透明浅红色
    
    // 绘制 Actor 的碰撞箱
    for (const auto& actor : actors_) {
        const collider& col = actor->getCollider();
        std::visit([this, &colliderColor](const auto& c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, Collider_Rect>) {
                sf::RectangleShape rect(sf::Vector2f(c.getHalfSize().x * 2, c.getHalfSize().y * 2));
                rect.setPosition(sf::Vector2f(c.getCenter().x - c.getHalfSize().x, c.getCenter().y - c.getHalfSize().y));
                rect.setFillColor(colliderColor);
                rect.setOutlineColor(sf::Color(255, 0, 0, 200));
                rect.setOutlineThickness(1.f);
                window_.draw(rect);
            } else if constexpr (std::is_same_v<T, Collider_Circle>) {
                sf::CircleShape circle(c.getRadius());
                circle.setPosition(sf::Vector2f(c.getCenter().x - c.getRadius(), c.getCenter().y - c.getRadius()));
                circle.setFillColor(colliderColor);
                circle.setOutlineColor(sf::Color(255, 0, 0, 200));
                circle.setOutlineThickness(1.f);
                window_.draw(circle);
            }
        }, col);
    }
    
    // 绘制 Building 的碰撞箱
    for (const auto& building : buildings_) {
        const collider& col = building->getCollider();
        std::visit([this, &colliderColor](const auto& c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, Collider_Rect>) {
                sf::RectangleShape rect(sf::Vector2f(c.getHalfSize().x * 2, c.getHalfSize().y * 2));
                rect.setPosition(sf::Vector2f(c.getCenter().x - c.getHalfSize().x, c.getCenter().y - c.getHalfSize().y));
                rect.setFillColor(colliderColor);
                rect.setOutlineColor(sf::Color(255, 0, 0, 200));
                rect.setOutlineThickness(1.f);
                window_.draw(rect);
            } else if constexpr (std::is_same_v<T, Collider_Circle>) {
                sf::CircleShape circle(c.getRadius());
                circle.setPosition(sf::Vector2f(c.getCenter().x - c.getRadius(), c.getCenter().y - c.getRadius()));
                circle.setFillColor(colliderColor);
                circle.setOutlineColor(sf::Color(255, 0, 0, 200));
                circle.setOutlineThickness(1.f);
                window_.draw(circle);
            }
        }, col);
    }
}

/**
 * 保持视口高度不变，视口宽度随窗口自适应，窗口多出来的部分使用黑边裁切
*/
void Game::onWindowResized() {
    float windowWidth = static_cast<float>(window_.getSize().x);
    float windowHeight = static_cast<float>(window_.getSize().y);

    float scale = windowHeight / WORLD_HEIGHT;
    float visibleWidth = WORLD_WIDTH * scale;
    float offsetX = (windowWidth - visibleWidth) / 2.f;

    sf::FloatRect viewport(
        sf::Vector2f(offsetX / windowWidth, 0.f), 
        sf::Vector2f(visibleWidth / windowWidth, 1.f)
    );
    worldView_.setViewport(viewport);
}

// WorldContext接口实现
sf::Vector2f Game::getPlayerPosition() const {
    if (controllers_.empty()) {
        return sf::Vector2f(0.f, 0.f);
    }
    // 假设第一个控制器是玩家控制器
    Character* player = controllers_[0]->getCharacter();
    if (player) {
        return player->getCenter();
    }
    return sf::Vector2f(0.f, 0.f);
}

Character* Game::getPlayer() const {
    if (controllers_.empty()) {
        return nullptr;
    }
    // 假设第一个控制器是玩家控制器
    return controllers_[0]->getCharacter();
}

std::vector<sf::Vector2f> Game::getObstaclePositions() const {
    std::vector<sf::Vector2f> obstacles;
    
    // 添加所有建筑物的位置
    for (const auto& building : buildings_) {
        obstacles.push_back(building->getCenter());
    }
    
    return obstacles;
}

bool Game::isPositionOccupied(sf::Vector2f position) const {
    // 检查是否有建筑物占用该位置
    for (const auto& building : buildings_) {
        if (building->containDot(position)) {
            return true;
        }
    }
    
    // 检查是否有其他角色占用该位置
    for (const auto& actor : actors_) {
        if (actor->containDot(position)) {
            return true;
        }
    }
    
    return false;
}

sf::Vector2f Game::getWorldBounds() const {
    return sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT);
}
