# Plain

Plain 是一个基于 C++17 和 SFML 3 的 2D 顶视角游戏原型/引擎示例。项目当前重点在于：

- 一个基于固定时间步的游戏循环
- 实体与建筑的碰撞处理
- 角色动画与渲染
- 玩家控制器和 AI 控制器
- 轻量级的世界上下文接口，用于 AI 获取玩家和障碍信息

这个项目更像是“游戏引擎框架 + 小型玩法原型”，而不是完整的工业级引擎；它的结构比较清晰，适合进一步扩展成更复杂的 RTS / ARPG / 生存类游戏。

---

## 1. 项目目标

当前代码实现了以下核心能力：

- 2D 世界场景渲染
- 玩家移动、加速和受力运动
- 角色攻击与伤害判定
- AI 追逐与攻击逻辑
- 建筑物/障碍物放置
- 视口跟随和世界坐标转换
- 简单的 FPS 和数值调试显示

从架构上看，它遵循了“组件化/分层”的思路：

- 物理/碰撞层：Entity、Collider
- 渲染层：Renderable、Actor、Building
- 控制层：Controller、PlayerController、AIController
- 游戏主循环层：Game
- 世界协议层：WorldContext

---

## 2. 目录结构

```text
plain/
├── include/
│   ├── Game.hpp
│   ├── WorldContext.hpp
│   ├── animation/
│   │   └── Animation.hpp
│   ├── controllers/
│   │   ├── Controller.hpp
│   │   ├── PlayerController.hpp
│   │   └── AIController.hpp
│   ├── core/
│   │   ├── collision.hpp
│   │   ├── Entity.hpp
│   │   ├── GameObject.hpp
│   │   ├── Renderable.hpp
│   │   └── VectorMath.hpp
│   ├── entities/
│   │   ├── Actor.hpp
│   │   ├── Building.hpp
│   │   ├── Character.hpp
│   │   └── ...
│   └── menu/
│       └── menu.hpp
├── src/
│   ├── Game.cpp
│   ├── Plain.cpp
│   ├── animation/
│   │   └── Animation.cpp
│   ├── controllers/
│   │   ├── Controller.cpp
│   │   ├── PlayerController.cpp
│   │   └── AIController.cpp
│   ├── core/
│   │   ├── collision.cpp
│   │   ├── Entity.cpp
│   │   ├── VectorMath.cpp
│   │   └── ...
│   ├── entities/
│   │   ├── Actor.cpp
│   │   ├── Building.cpp
│   │   ├── Character.cpp
│   │   └── ...
│   └── menu/
│       └── menu.cpp
├── texture/
├── fonts/
├── makefile
├── README.md
└── obj/
```

从结构上看：

- include/：对外声明和接口定义
- src/：具体实现
- texture/：资源文件
- fonts/：字体资源
- obj/：编译生成的中间文件
- makefile：构建脚本

---

## 3. 架构总览

从代码运行顺序看，整个项目的主线是：

1. `main()` 创建 `sf::RenderWindow`
2. `Game game(window);`
3. `game.run();`
4. `Game::run()` 进入主循环
5. 处理事件、输入、控制器更新、物理更新、渲染

主入口在 [src/Plain.cpp](src/Plain.cpp)。

核心主循环由 [src/Game.cpp](src/Game.cpp) 中的 `Game::run()` 提供。

---

## 4. 核心模块说明

### 4.1 Game：整个游戏的总控中心

文件：

- [include/Game.hpp](include/Game.hpp)
- [src/Game.cpp](src/Game.cpp)

`Game` 是游戏最关键的管理类，它负责：

- 持有所有 actor
- 持有所有 building
- 持有所有 controller
- 管理渲染列表
- 处理世界视图和相机
- 执行固定时间步更新
- 处理碰撞
- 渲染 UI 和调试信息

它继承自 `WorldContext`，也就是说它不仅是“游戏对象容器”，也是 AI 查询世界状态的接口实现者。

#### 关键职责

- `run()`：主事件循环
- `controllersUpdate()`：更新所有控制器输入意图
- `controllersWork()`：让控制器作用于角色
- `fixedUpDate(float fixed_dt)`：推进实体固定步物理更新
- `collision()`：检测 Actor/Actor 与 Actor/Building 碰撞
- `draw()`：绘制世界和 UI
- `onWindowResized()`：处理视口适配

其中固定时间步的设计很典型：

```cpp
float frametime = clock_.restart().asSeconds();
accumulator_ += frametime;
while (accumulator_ >= fixed_dt) {
    controllersWork();
    fixedUpDate(fixed_dt);
    collision();
    accumulator_ -= fixed_dt;
}
```

这是一种经典的固定步更新模式，可以让移动和碰撞稳定，不受帧率影响。

---

### 4.2 WorldContext：AI 世界查询接口

文件：

- [include/WorldContext.hpp](include/WorldContext.hpp)

`WorldContext` 定义了 AI 需要的世界信息接口，类似“游戏世界协议”。

它的职责：

- 给 AI 提供玩家位置
- 提供障碍物位置
- 检查某位置是否被占用
- 提供世界边界

这是 AIController 能够做寻路和追击的基础。

---

### 4.3 Entity：基础物理实体

文件：

- [include/core/Entity.hpp](include/core/Entity.hpp)
- [src/core/Entity.cpp](src/core/Entity.cpp)

`Entity` 是整个物理世界的基础类。

它封装了：

- `collider_`：碰撞盒
- `MoveTag`：是否可移动
- 中心点 / 半尺寸 / 半径等统一接口

它通过 `std::variant` 管理两种碰撞形状：

- `Collider_Rect`
- `Collider_Circle`

这使得实体能在同一抽象层使用矩形或圆形碰撞体，而不需要为不同形状分开写大量重复逻辑。

#### 设计要点

- `getCenter()`：统一获取中心点
- `setCenter()`：移动实体中心
- `containDot()`：测试某点是否落在碰撞体内
- `moveCollider()`：移动碰撞体

这层很好地解耦了“实体位置”与“碰撞表示”，让其他模块不必关心具体碰撞体形状。

---

### 4.4 Collider：碰撞系统

文件：

- [include/core/collision.hpp](include/core/collision.hpp)

这个文件定义了碰撞体结构和通用检测函数：

- `Collider_Rect`
- `Collider_Circle`
- `itrsctng(...)`：检测两个碰撞体是否相交
- `sprtAFrmB(...)`：计算分离向量，解决重叠

从命名上看，碰撞系统非常直接，属于经典的 AABB / Circle 的轻量算法实现。

#### 它的定位

- 不负责整个游戏世界管理
- 只负责“两个碰撞盒是否重叠，以及如何分离”
- 由 `Game::collision()` 调用，统一解决物理响应

---

### 4.5 Renderable：渲染接口

文件：

- [include/core/Renderable.hpp](include/core/Renderable.hpp)

这是一个非常薄的抽象基类：

```cpp
class Renderable {
public:
    virtual void toScreen(float alpha = 0.f, float dt = 0.f) = 0;
    virtual const sf::Drawable &getDrawable() const = 0;
    virtual float getRenderOrder() const = 0;
};
```

它统一了“能被渲染对象”的接口，允许 `Game::draw()` 只依赖统一方法绘制不同类型对象：

- Actor
- Building
- 后续可能扩展的效果对象、粒子、UI 图层等

在设计上，这个层次很好为未来添加更多实体提供扩展点。

---

### 4.6 Actor：可移动的游戏对象

文件：

- [include/entities/Actor.hpp](include/entities/Actor.hpp)
- [src/entities/Actor.cpp](src/entities/Actor.cpp)

`Actor` 是可移动单位的基类。它继承自：

- `Entity`：基础碰撞与位置
- `Renderable`：渲染接口

#### 它管理的核心数据

- `velocity_`：当前速度
- `acceleration_`：当前加速度
- `decefactor_`：阻尼/摩擦系数
- `foothold_`：脚底落点，用于贴地/贴图对齐
- `animations_`：动画列表
- `current_animation_`：当前播放动画

#### 关键方法

- `fixedUpDate(float fixed_dt)`：固定步更新位移和速度
- `toScreen(float alpha, float dt)`：插值布局并更新动画显示
- `move(sf::Vector2f velocity)`：直接移动
- `accelerate(sf::Vector2f acceleration)`：施加加速度
- `addAnimation(...)`：添加动画状态

这层是整个游戏对象更新核心，它把“位置更新”和“动画显示”的职责分开了：

- 物理固定步更新：`fixedUpDate`
- 画面显示：`toScreen`

这是很典型的游戏循环组织方式。

---

### 4.7 Character：角色对象

文件：

- [include/entities/Character.hpp](include/entities/Character.hpp)
- [src/entities/Character.cpp](src/entities/Character.cpp)

`Character` 是 `Actor` 的子类，扩展了“战斗”能力。

它增加了：

- `health_`：生命值
- `damage_`：攻击力
- `damagerange_`：攻击范围/触发角度
- `attackCoolDown_`：攻击冷却
- `isAttacking_`：是否正在攻击

#### 关键行为

- `attack(Character &victim, sf::Vector2f direction)`：对敌人造成伤害
- `fixedUpDate(float fixed_dt)`：在 Actor 更新基础上处理攻击冷却
- `getAnimationState()`：根据状态返回 Idle / Move / Attack 等动画状态

角色层把战斗和状态切换耦合在了通用 Actor 上，结构很清晰，但也说明这套架构比较偏“轻量游戏角色模板”。

---

### 4.8 Building：建筑与障碍物

文件：

- [include/entities/Building.hpp](include/entities/Building.hpp)
- [src/entities/Building.cpp](src/entities/Building.cpp)

`Building` 也是实体，但通常是静态/半静态对象：

- 树
- 石头
- 地图障碍

它继承 `Entity` 和 `Renderable`，并可以被放置到游戏世界中。它的完整构建逻辑由 `createBuilding(...)` 提供，使用 `BuildingID` 索引注册表生成不同种类的建筑。

#### 特点

- 与 `Actor` 不同，它通常不参与速度和移动
- 但它同样参与渲染和碰撞
- 通过 `alignToMap()` 对齐网格，保证放置时贴合地图格子

---

### 4.9 Controller：输入与 AI 控制抽象层

文件：

- [include/controllers/Controller.hpp](include/controllers/Controller.hpp)
- [src/controllers/Controller.cpp](src/controllers/Controller.cpp)

这是游戏控制逻辑的统一抽象。

它的核心成员：

- `Character* character_`
- `ControllerType type_`
- `Intention intention_`

`Intention` 表示一帧中的控制意图：

- `victimType`
- `attack`
- `accele`
- `aimDir`

这是一种非常适合做游戏 AI / 玩家输入分层的设计：

- 控制器决定“想做什么”
- 物理/角色更新根据意图去执行

#### 设计价值

- 玩家控制器只负责读取输入，并将意图写入 `Intention`
- AI 控制器负责计算追逐、攻击、路径重规划
- `Game::controllersWork()` 再统一执行这些意图

这使得输入和 AI 的统一逻辑更清晰。

---

### 4.10 PlayerController：玩家输入

文件：

- [include/controllers/PlayerController.hpp](include/controllers/PlayerController.hpp)
- [src/controllers/PlayerController.cpp](src/controllers/PlayerController.cpp)

玩家控制器负责：

- 读取 W/A/S/D 作为加速度方向
- 读取鼠标位置作为瞄准方向
- 读取左键判断是否攻击

它将这些状态转成 `Intention`，交给 `Game` 的统一处理逻辑。

特点：

- 不直接修改角色位置
- 只生成“动作意图”
- 遵循“输入层 → 意图层 → 角色逻辑层”的分层设计

---

### 4.11 AIController：AI 逻辑

文件：

- [include/controllers/AIController.hpp](include/controllers/AIController.hpp)
- [src/controllers/AIController.cpp](src/controllers/AIController.cpp)

AI 控制器是这个项目里最复杂的模块之一，负责：

- 跟踪玩家位置
- 判断检测范围与攻击范围
- 更新 AI 状态：IDLE / CHASING / ATTACKING
- 通过 A* 寻路计算路径
- 结合 `WorldContext` 访问障碍信息

#### 关键部分

- `worldToGrid()` / `gridToWorld()`：世界坐标与网格坐标转换
- `findPath()`：A* 路径搜索
- `updatePath()`：定期更新路径
- `getNextDirection()`：沿路径前进
- `updateAIState()`：更新 AI 状态

这个模块说明项目尝试把“AI 与世界信息”拆成独立系统，而不仅仅是 hard-code 的指令式逻辑。

---

### 4.12 Animation：动画系统

文件：

- [include/animation/Animation.hpp](include/animation/Animation.hpp)
- [src/animation/Animation.cpp](src/animation/Animation.cpp)

动画系统分为两个层面：

- `Animation`：单个状态下的帧管理
- `Actor`：持有多个 `Animation` 实例并负责状态切换

`AnimationState` 定义了常见动作状态：

- `IDLE`
- `MOVING`
- `IDLE_ATTACK`
- `MOVING_ATTACK`
- `DEAD`

#### 工作方式

- 每个状态对应一组帧
- `Actor::toScreen()` 根据当前 `velocity_` 和方向决定动画状态
- `Animation::update(dt)` 基于时间推进帧

这样可以让角色在移动或攻击时切换不同纹理序列。

---

## 5. 游戏主循环设计

整个循环非常典型，核心在 [src/Game.cpp](src/Game.cpp) 的 `Game::run()`：

1. 处理窗口事件
2. 读取输入
3. 更新控制器
4. 进入固定时间步物理循环
5. 进行碰撞检测
6. 计算插值位置
7. 渲染世界
8. 渲染 UI 与调试信息
9. `window_.display()`

### 固定步的意思

这里使用了固定更新步长：

```cpp
float fixed_dt = 0.1f / 120.f;
```

也就是每 1/1200 秒左右更新一次逻辑，这使得：

- 物理更稳定
- 冲突更容易控制
- 不容易受不同机器帧率影响

但值得注意的是：

- 渲染仍然用 `alpha = accumulator_ / fixed_dt` 做插值
- 这也是一个很常见的“固定步 + 插值渲染”设计

---

## 6. 模块关系图

```text
main
 └── Game::run()
      ├── 处理 SFML 事件
      ├── PlayerController::update()
      ├── AIController::update()
      ├── Game::controllersWork()
      │    ├── Actor::accelerate()
      │    └── Character::attack()
      ├── Game::fixedUpDate()
      │    └── Actor::fixedUpDate()
      ├── Game::collision()
      │    ├── Actor vs Actor
      │    └── Actor vs Building
      ├── Game::toScreen()
      │    └── Actor::toScreen()
      └── Game::draw()
           └── Renderable::getDrawable()
```

这说明项目的逻辑大体上可以理解为：

- 输入/AI -> 意图 -> 物理 -> 渲染

是非常标准的游戏逻辑流。

---

## 7. 设计优点

这一版架构的优点主要体现在：

- 分层明确：控制层、实体层、渲染层、世界层职责分离
- 可扩展：新增角色、建筑、AI 都能继续沿用当前结构
- 固定步更新：数学上更稳定
- 碰撞抽象：统一了矩形和圆形处理
- 可视化调试：F3 可显示 FPS 和对象数量

---

## 8. 当前项目的局限

项目已经具备雏形，但在工程层面还存在一些典型的简化设计：

- 所有权管理还比较简单，控制器和对象可能存在重复持有风险
- 世界查询接口依赖“第一控制器就是玩家”的假设
- AI 路径更新使用固定帧率近似值，而不是真实 dt
- 资源加载错误没有做严格检查
- 代码中有一些命名风格和 API 约定不完全统一

这些问题并不妨碍当前项目做原型，但如果要扩展成正式游戏，建议优先解决这些基础工程问题。

---

## 9. 编译与运行

工程使用 makefile 构建，默认示例为：

```bash
make
```

在 Windows + MinGW 环境下，可以直接运行生成的 `Plain.exe`。

如果要重新编译：

```bash
make clean
make
```

也可以使用 release 版本：

```bash
make release
```

---

## 10. 适合的后续扩展方向

如果准备继续大规模开发，这个项目目前最适合往以下方向扩展：

1. 真实资源管理器
   - 统一管理纹理、字体、音频

2. ECS/数据组件体系
   - 把 `Actor` / `Character` / `Building` 改成更灵活的数据驱动结构

3. 更稳定的物理系统
   - 统一使用 `dt` 和组件更新系统

4. 更多地图与 AI 逻辑
   - 地图网格、寻路缓存、状态机

5. 事件系统与脚本系统
   - 让游戏逻辑更适合大项目协作

---

## 11. 结论

Plain 是一个“非常适合学习游戏开发和 C++ 引擎结构”的项目。它没有追求过度抽象，而是用清晰的类层次和典型游戏循环展示了：

- 角色控制
- 碰撞
- 动画
- AI
- 渲染
- 固定时间步逻辑

如果你想继续开发这个项目，最值得优先做的事情不是新增更多功能，而是把“对象所有权、时间步、世界查询接口”这些底层架构问题修好。这样后续扩展才会稳定。

---

如果你愿意，我还可以继续为这个项目补一份更偏“源码导读版”的 README，或者直接生成一份带流程图和类关系图的技术文档版本。