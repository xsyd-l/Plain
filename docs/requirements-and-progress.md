# 需求规格与完成情况

本文档记录 Plain 项目的核心需求定义、验收标准与当前实现进度。

- 项目：Plain（C++17 + SFML 3.0.2，2D 顶视角游戏原型/引擎示例）
- 文档最后更新：2026-09-20
- 代码基线：`src/`、`include/` 当前工作区版本

---

## 1. 需求总览

| 编号 | 需求 | 状态 | 说明 |
| --- | --- | --- | --- |
| R1 | 支持两种碰撞箱：圆形和矩形 | ✅ 已完成 | 已落地并参与实际碰撞解算 |
| R2 | 支持基于加速度的实体移动 | ✅ 已完成 | 固定时间步内的速度积分与阻尼 |
| R3 | 支持插值渲染 | ✅ 已完成 | 渲染位置与固定步物理位置解耦 |
| R4 | 支持 8 个方向的角色移动动画 | 🚧 进行中 | 基础设施已具备，方向数据与渲染匹配尚未打通 |
| R5 | 支持四叉树结构的碰撞检测性能优化 | 🚧 进行中 | 需求已确认，四叉树模块尚未开始编码 |

状态图例：✅ 已完成（可验收）｜🚧 进行中（已启动）｜⬜ 未开始

---

## 2. R1：两种碰撞箱（圆形 / 矩形）

**状态：✅ 已完成**

### 2.1 需求描述

实体（Entity）可选用矩形或圆形碰撞箱，两种形状之间以及与自身之间都要能进行：

1. 点包含检测（用于放置合法性、鼠标拾取等）；
2. 相交检测（用于碰撞判定）；
3. 分离向量计算（用于碰撞后的位置修正）。

### 2.2 实现位置

| 内容 | 位置 |
| --- | --- |
| 碰撞箱定义与接口 | `include/core/collision.hpp` |
| 碰撞算法实现 | `src/core/collision.cpp` |
| 实体与碰撞箱绑定 | `include/core/Entity.hpp`、`src/core/Entity.cpp` |
| 碰撞解算调用 | `Game::collision()`（`src/Game.cpp`） |
| 碰撞箱可视化 | `Game::drawColliders()`（`src/Game.cpp`，F3 开关） |

### 2.3 关键设计

- 抽象基类 `Collider` 定义 `containDot(sf::Vector2f)` 纯虚接口。
- 两个具体实现：
  - `Collider_Rect`：持有 `center_` 与 `halfsize_`，提供 `left()/right()/top()/bottom()` 便捷边界；
  - `Collider_Circle`：持有 `center_` 与 `radius_`。
- 使用 `using collider = std::variant<Collider_Rect, Collider_Circle>;` 做类型安全的联合体，配合 `std::visit` 双重分派，避免虚函数与 `dynamic_cast` 的开销与不安全。
- 对外两个自由函数：
  - `bool itrsctng(const collider& A, const collider& B)`：相交判定；
  - `sf::Vector2f sprtAFrmB(const collider& A, const collider& B)`：把 A 从 B 中推离的最小分离向量（MTV）。
- 四种组合均已实现：

| A \ B | Rect | Circle |
| --- | --- | --- |
| Rect | 轴对齐重叠区间比较 | 圆心在矩形内分支 + 最近点 clamp 分支 |
| Circle | 复用 Rect-Rect-Circle 逻辑并取反 | 圆心距与半径和比较 |

- `Entity` 提供两个构造函数分别对应矩形与圆形，并通过 `getHalfSize()` / `getRadius()` 返回 `std::optional`，表现“当前形状是否有该属性”。

### 2.4 验收标准

- 同一场景中矩形与圆形实体可共存，两两之间的相交判定结果正确（含对角贴近、圆内切矩形等边界情况）。
- 相交后分离向量方向正确、无抖动、无残留重叠（`Game::collision()` 中 Actor-Actor 各推一半，Actor-Building 全推 Actor）。
- 开启 F3 后可在屏幕上观察到两种碰撞箱轮廓与实际碰撞行为一致。

---

## 3. R2：基于加速度的实体移动

**状态：✅ 已完成**

### 3.1 需求描述

实体的移动由加速度驱动并随时间积分，而不是直接设置位置；停止输入后应平滑减速并最终静止。

### 3.2 实现位置

| 内容 | 位置 |
| --- | --- |
| 移动状态与积分 | `Actor::fixedUpDate(float fixed_dt)`（`src/entities/Actor.cpp`） |
| 加速接口 | `Actor::accelerate(sf::Vector2f)`、`Actor::move(sf::Vector2f)` |
| 意图到加速度的转换 | `Game::controllersWork()`（`src/Game.cpp`，`intention.accele * 4000.f`） |
| 固定时间步驱动 | `Game::run()` 中的 `accumulator_` while 循环 |

### 3.3 关键设计

`Actor` 持有 `velocity_`、`acceleration_`、`acceleration_limit_`、`decefactor_`，每个固定步执行：

1. 记录 `previous_position`（供插值渲染使用）；
2. 速度积分：`velocity_ += fixed_dt * acceleration_`；
3. 阻尼衰减：`velocity_ *= decefactor_`（默认 `0.95f`）；
4. 低速归零：无加速度且 `velocity_ · velocity_ <= 900.f` 时直接置零，避免浮点残余速度导致的“永远漂移”；
5. 位移：`moveCollider(velocity_ * fixed_dt)`；
6. 清空本步加速度：`acceleration_ = {0.f, 0.f}`（加速度是“每步意图”，需要每步重新施加）。

所有移动都与时间相关（时间依赖），因此必须由固定时间步调用，保证不同帧率下运动学行为一致。

### 3.4 验收标准

- 在不同机器帧率下，相同输入的角色位移与加速过程一致。
- 松开按键后角色平滑减速并完全静止（无亚像素漂移）。
- 碰撞将实体推出障碍后，速度与位置状态保持自洽。

---

## 4. R3：插值渲染

**状态：✅ 已完成**

### 4.1 需求描述

渲染帧率与物理固定步长解耦：物理以固定步推进，渲染时在“上一物理位置”和“当前物理位置”之间插值，以消除画面抖动（stutter）。

### 4.2 实现位置

| 内容 | 位置 |
| --- | --- |
| 插值系数计算 | `Game::run()`：`float alpha = accumulator_ / fixed_dt;` |
| 插值落地 | `Actor::toScreen(float alpha, float dt)`（`src/entities/Actor.cpp`） |
| 统一驱动 | `Game::toScreen(alpha, dt)` 遍历 actors/buildings |
| 视觉同步 | `Renderable` 接口（`include/core/Renderable.hpp`） |

### 4.3 关键设计

- `Actor::fixedUpDate()` 在推进物理前保存 `previous_position`；
- 渲染阶段执行：`interPolatePos = previous_position + alpha * (getCenter() - previous_position)`，其中 `alpha ∈ [0, 1)` 表示当前渲染时刻落在两个物理步之间的比例；
- 插值只作用于渲染用 `sprite_` 的位置，物理碰撞箱仍位于离散物理位置，逻辑层保持确定性；
- 渲染顺序使用 `getRenderOrder()`（取中心 Y）做 `std::stable_sort`，保证同深度对象次序稳定；
- 动画推进复用渲染帧的 `dt`（真实帧间隔），与物理步长独立。

### 4.4 验收标准

- 在高刷新率显示器上匀速移动时画面平滑，无周期性抖动。
- `alpha` 恒在 `[0, 1)` 区间内，插值不会造成“超前/回退”的视觉跳动。

---

## 5. R4：8 方向角色移动动画

**状态：🚧 进行中（基础设施已具备，尚未打通方向匹配与资源侧）**

### 5.1 需求描述

角色的移动动画需区分 8 个方向：上、下、左、右、左上、右上、左下、右下；方向切换时动画应立即切换且不出现跳帧或闪烁。

### 5.2 现状（已具备的部分）

| 已有能力 | 位置 |
| --- | --- |
| 动画状态枚举（IDLE / MOVING / IDLE_ATTACK / MOVING_ATTACK / DEAD） | `include/animation/Animation.hpp` |
| 动画以 `(state, direction)` 唯一标识 | `Animation::getState()` / `getDirection()` |
| 按方向注册动画 | `Actor::addAnimation(...)`（含 `direction`、`scale_factor`、`foothold` 参数） |
| 方向向量产出 | `Actor::getDirection()`（由速度各轴正负组合，已可得到 8 个方向） |
| 左右镜像 | 通过 `scale_factor = {-1.f, 1.f}` 实现 |
| 状态机回调 | `Actor::getAnimationState()`（虚函数，可由 `Character` 重写） |

当前 `Game::run()` 中仅为玩家注册了 3 个动画：`IDLE{0,0}`、`MOVING{1,0}`（向右）、`MOVING{-1,0}`（向左，镜像）。

### 5.3 差距与待办

1. **方向匹配被压缩为水平方向**
   `Actor::toScreen()` 中 `matchDir` 会把任意方向的 `x` 归约为 `±1 / 0`，只区分左、右，导致上/下/斜向移动时仍播放水平动画。
   → 需改为使用完整 8 方向匹配，并定义**回退优先级**（例如：斜向缺帧 → 退回水平 → 退回垂直 → 退回 `{0,0}`）。

2. **纹理布局只支持单行**
   `Animation` 构造函数中 `frameW = texture.getSize().x / frame_total_`、`frameH = texture.getSize().y`，隐含“一行帧序列 + 单一方向”的假设。
   → 需扩展为支持多行（每行一个方向/状态），或按方向拆分为多张纹理。

3. **动画资源不足**
   现有的 `texture/player.png`、`texture/ai.png` 只包含左右方向的帧序列。
   → 需补齐 8 方向的美术资源，或在缺少资源时明确使用镜像/回退策略。

4. **方向注册与状态联动**
   → 为 8 方向 × 各状态（IDLE / MOVING / ATTACK / DEAD）补全 `addAnimation` 注册；
   → 明确斜向移动时“朝向”的判定规则（例如按速度分量绝对值大小决定主朝向）。

5. **可观测性**
   → 建议在 F3 调试信息中增加“当前动画状态 + 当前方向”显示，便于验收 8 方向切换是否正确。

### 5.4 验收标准

- 8 个方向的持续移动分别播放对应方向的动画，肉眼可辨；
- 方向切换（含跨斜向）无闪烁、无错帧、无“卡在上一方向”的现象；
- 缺少某方向资源时有确定且可预期的回退行为；
- 左右镜像方向的落脚点 `foothold` 与碰撞箱中心对齐，无视觉偏移。

---

## 6. R5：四叉树碰撞检测性能优化

**状态：🚧 进行中（需求已确认，尚未开始编码）**

### 6.1 需求描述

用四叉树对碰撞检测做宽阶段（broad phase）加速，降低实体数量增长时的检测复杂度，同时保持与暴力检测一致的碰撞结果。

### 6.2 现状（性能瓶颈点）

| 现有实现 | 复杂度 | 位置 |
| --- | --- | --- |
| Actor vs Actor 暴力两两检测 | $O(n^2)$ | `Game::collision()` |
| Actor vs Building 暴力检测 | $O(n \times m)$ | `Game::collision()` |
| 放置合法性检测（遍历全部实体） | $O(n + m)$ | `Game::run()` 按键处理段 |
| 单点占用查询 | $O(n + m)$ | `Game::isPositionOccupied()` |

当前实体数量较少时暴力检测完全够用，但随着 Actor/Building 数量增长，每固定步都要执行全量两两检测（固定步长仅为 `0.1f / 120.f` 秒，即每个渲染帧可能执行多次），会成为主循环的主要热点。

### 6.3 计划方案

1. **新增模块**
   - `include/core/Quadtree.hpp` + `src/core/Quadtree.cpp`（或模板实现全部放在头文件）。
2. **数据结构设计**
   - 节点：世界边界 `sf::FloatRect`、节点容量 `capacity`、最大深度 `maxDepth`、实体索引列表、四个子节点（`std::unique_ptr`）。
   - `insert`：超容量且未达最大深度时分裂；实体跨节点时允许同时存在于多个子节点（或统一挂到父节点，二选一需在文档中固定）。
   - `retrieve(area)`：返回与查询区域相交的候选实体索引集合。
   - `clear()`：复用节点，避免每帧重新分配。
3. **接入策略**
   - 静态物体（Building）插入树中，仅在世界变化时更新；
   - 动态物体（Actor）在移动后更新所在叶节点（或每个固定步重建整棵树，视实现成本而定）；
   - 宽阶段用四叉树产出候选对，窄阶段仍调用 `itrsctng` / `sprtAFrmB`，**保证碰撞行为不变**。
4. **复用场景**
   - `Game::collision()` 的 Actor-Actor 与 Actor-Building 检测；
   - 建筑放置合法性检测与 `WorldContext::isPositionOccupied()` 的区域查询。
5. **度量方式**
   - 用现有 FPS 显示（F3）配合压测场景（批量生成 Actor）对比优化前后帧率；
   - 可选：增加“候选对数量 / 实际检测次数”调试计数，量化宽阶段的裁剪效果。

### 6.4 验收标准

- 相同场景下，开启四叉树前后的碰撞结果一致（同一随机种子下最终位置偏差在可接受阈值内）。
- Actor 数量达到数百级别时，单个固定步的碰撞检测耗时显著低于暴力实现。
- 四叉树不引入内存泄漏与明显的每帧分配抖动。

---

## 7. 里程碑与后续计划

| 阶段 | 内容 | 状态 |
| --- | --- | --- |
| M1 | 完成 R1 碰撞箱抽象与四种组合算法 | ✅ |
| M2 | 完成 R2 加速度移动与固定时间步积分 | ✅ |
| M3 | 完成 R3 插值渲染 | ✅ |
| M4 | 完成 R4 8 方向动画（方向匹配 + 纹理多行 + 资源） | 🚧 启动中 |
| M5 | 完成 R5 四叉树宽阶段并接入碰撞/查询 | 🚧 启动中 |
| M6 | 压测与性能对比数据归档到 `docs/` | ⬜ |

### 近期建议的工作顺序

1. 先做 R4 的方向匹配改造（`Actor::toScreen()` 中移除水平压缩逻辑，改为完整 8 方向匹配 + 回退优先级），用现有左右资源先验证方向切换的正确性；
2. 再扩展 `Animation` 的纹理行（direction row）支持，并补齐美术资源；
3. R5 先落地独立可测试的 `Quadtree` 模块（不接入主循环），验证查询正确性；
4. 最后接入 `Game::collision()`、放置检测与 `isPositionOccupied()`。

---

## 8. 风险与注意事项

- **R4 回退策略需先定义**：斜向动画资源缺失时的回退规则若不明确，容易出现方向抖动（相邻帧在两个动画间来回切换）。
- **R4 与 R2 的耦合**：`getDirection()` 直接依赖速度正负，速度接近 0 时方向会抖；插值 + 低速归零阈值（速度平方 900）可缓解，但仍建议为方向判定引入“最小速度阈值/滞回”。
- **R5 跨节点实体**：跨节点重复存储会使候选集合出现重复对，需在配对阶段去重（例如仅保留 `i < j` 或使用集合去重）。
- **R5 与确定性**：宽阶段只做裁剪，窄阶段算法与解算顺序必须保持不变，否则会改变碰撞解算结果。
- **性能度量基线**：在优化前先记录暴力实现的基线数据，否则无法证明 R5 的收益。

---

## 9. 修订记录

| 日期 | 变更 |
| --- | --- |
| 2026-09-20 | 初版：梳理 R1–R5 需求、验收标准与完成情况（R1–R3 已完成，R4–R5 进行中） |
