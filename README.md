# 多策略路径规划可视化系统

面向对象程序设计课程设计 — **C++17 后端** + **Bootstrap Web 图形界面**。  
在二维网格地图上编辑障碍与起终点，运行多种寻路算法，以动画展示搜索过程，并对比性能指标。

---

## 功能概览

| 模块 | 说明 |
|------|------|
| **10 种寻路算法** | BFS、DFS、Dijkstra、A*、Greedy Best-First、Weighted A*、双向 BFS、双向 A*、IDA*、IDDFS |
| **Web 图形界面** | 地图编辑、逐步搜索动画、多算法性能对比柱状图、中英文切换 |
| **地图工具** | 障碍/填充/起终点编辑、迷宫生成、随机散点障碍、自定义尺寸、保存/加载 |
| **撤销/重做** | 命令模式记录编辑历史，支持 Ctrl+Z / Ctrl+Y |
| **本地 HTTP 服务** | C++ 内置 REST API（cpp-httplib），浏览器访问，无需安装 Qt 等 GUI 库 |
| **控制台模式** | 支持命令行交互、三算法演示、HTML 导出 |

---

## 环境要求

- Windows 10/11
- C++17 编译器（MinGW 或 Visual Studio）
- CMake 3.16+（可选；未安装时自动使用 MinGW 直接编译）
- 网络（仅首次运行 `scripts\setup_frontend.bat` 下载前端依赖）

---

## 快速开始

在资源管理器中 **双击**：

```
一键启动.bat
```

或双击 `一键启动.vbs`（效果相同）。

脚本流程：检查前端资源 → 若源码有更新则自动编译 → 启动 HTTP 服务 → 自动打开浏览器。  
**无需打开 IDE。** 关闭黑色命令行窗口即停止服务。

- 默认地址：**http://127.0.0.1:8765**
- 端口占用时可运行：`一键启动.bat 9000`
- 可选：双击 `创建桌面快捷方式.vbs` 在桌面生成快捷图标

### 手动步骤

```bat
scripts\setup_frontend.bat   :: 首次：下载 Bootstrap / Chart.js / Icons
build.bat                    :: 编译
run.bat                      :: 启动 Web 界面
```

### 其他运行方式

```bat
run.bat web                  :: 显式启动 Web 界面
run.bat web 9000             :: 指定端口
run.bat console              :: 控制台交互模式
run.bat demo                 :: 命令行多算法对比
```

---

## Web 界面操作

| 操作 | 说明 |
|------|------|
| 编辑工具 | 障碍 / 填充 / 起点 / 终点 |
| 左键 | 应用当前工具；拖拽可连续绘制 |
| 右键 | 快速填充空地 |
| 运行寻路 | 主按钮或 `Enter` |
| 算法对比 | 勾选多种算法后对比，点击结果可立即播放动画 |
| 搜索动画 | 播放 / 暂停 / 重置 / 拖动进度条 / 调节速度 |
| 地图生成 | 「生成迷宫」重置地图；「随机散点障碍」在当前地图上追加障碍 |
| 撤销重做 | 按钮或 `Ctrl+Z` / `Ctrl+Y` |
| 语言切换 | 顶部导航栏选择 **简体中文** / **English** |
| 快捷键 | `1`障碍 `2`填充 `3`起点 `4`终点 · `Enter`运行 · `Esc`停止动画 |

---

## 系统架构

系统采用 **前后端分离 + 分层架构**：浏览器负责交互与可视化，C++ 进程负责寻路计算、会话状态与 HTTP 服务。

```
┌─────────────────────────────────────────────────────────────┐
│                     浏览器 (web/)                            │
│  index.html + app.css + app.js + i18n.js + Chart.js         │
│  · 网格渲染与编辑  · 动画播放  · 对比图表  · 多语言 UI        │
└──────────────────────────┬──────────────────────────────────┘
                           │ HTTP / JSON (127.0.0.1:8765)
┌──────────────────────────▼──────────────────────────────────┐
│                   API 层 (src/api/)                          │
│  ApiServer        — 路由、静态资源、请求分发                  │
│  GridSerializer   — Grid ↔ JSON 序列化、动画采样              │
│  PathfindingService — 调用策略上下文执行寻路/对比             │
│  MapSession       — 单例会话：当前地图 + 命令历史              │
└──────────────────────────┬──────────────────────────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        ▼                  ▼                  ▼
┌───────────────┐  ┌───────────────┐  ┌───────────────┐
│  patterns/    │  │  algorithms/  │  │  core/        │
│  设计模式层    │  │  10 种算法     │  │  领域模型      │
│  策略/命令/    │  │  继承          │  │  Grid         │
│  观察者/工厂   │  │  GraphPathFinder│ │  PathResult   │
└───────────────┘  └───────────────┘  └───────────────┘
```

### 请求处理流程（以「运行寻路」为例）

1. 前端 `POST /api/pathfind`，body 含 `algorithm` id 与当前 `grid` JSON。
2. `ApiServer` 解析请求，经 `GridSerializer::gridFromJson` 还原 `Grid`。
3. `PathfindingService::run` 通过 `AlgorithmFactory::create` 实例化算法，注入 `PathfindingContext`。
4. 具体算法 `findPath()` 在搜索过程中通过 `SearchSubject::notify` 发出事件（访问/扩展节点等）。
5. 算法返回 `PathResult`（路径、访问序、统计信息），`GridSerializer` 采样 `visitedOrder` 后序列化为 JSON。
6. 前端接收结果，更新统计面板并驱动 Canvas/DOM 网格动画。

### 地图编辑与撤销流程

1. 用户拖拽编辑时，前端批量收集 `setCell` / `moveStart` / `moveGoal` 命令。
2. `POST /api/session/macro` 将命令数组提交给 `MapSession`。
3. `CommandManager` 执行命令并压入 undo 栈；redo 栈清空。
4. `POST /api/session/undo` / `redo` 触发命令的 `undo()` / `execute()`，并返回最新 grid 状态。

### 核心类职责

| 类 | 职责 |
|----|------|
| `Grid` | 二维网格数据、起终点、邻接查询、文件读写 |
| `PathFinder` | 寻路策略抽象接口 |
| `GraphPathFinder` | 图搜索算法公共基类：路径重建、观察者桥接 |
| `PathfindingContext` | 策略模式上下文，持有并调用当前 `PathFinder` |
| `AlgorithmFactory` | 根据 `AlgorithmType` 枚举创建具体算法实例 |
| `SearchSubject` / `SearchObserver` | 观察者模式：搜索事件发布/订阅 |
| `MapCommand` 及子类 | 命令模式：可撤销的地图编辑操作 |
| `CommandManager` | 维护 undo/redo 双栈（最多 120 步） |
| `MapSession` | 服务端单例，统一管理 grid 与命令历史 |
| `MazeGenerator` | 递归回溯迷宫、随机散点障碍、连通性检测 |
| `PathfindingService` | 门面式服务：单算法运行与多算法对比 |

---

## 面向对象设计（OOP）

### 四大特性在本项目中的体现

#### 1. 封装（Encapsulation）

- **`Grid`** 将 `cells_`、`start_`、`goal_` 设为私有，对外只暴露 `at` / `set`、`isWalkable`、`neighbors4` 等语义化接口，调用方无需关心一维数组下标编码。
- **`CommandManager`** 封装 undo/redo 双栈及 `kMaxHistory` 上限，外部仅调用 `execute` / `undo` / `redo`。
- **`MapSession`** 单例封装服务端会话状态，API 层不直接操作命令栈。

#### 2. 继承（Inheritance）

```
PathFinder（抽象策略）
    └── GraphPathFinder（图搜索基类，混入 SearchSubject）
            ├── BFSPathFinder
            ├── DFSPathFinder
            ├── DijkstraPathFinder
            ├── AStarPathFinder
            ├── GreedyBestFirstPathFinder
            ├── WeightedAStarPathFinder
            ├── BidirectionalBFSPathFinder
            ├── BidirectionalAStarPathFinder
            ├── IDAStarPathFinder
            └── IDDFSPathFinder

MapCommand（抽象命令）
    ├── SetCellCommand      — 修改单格类型
    ├── MoveMarkerCommand   — 移动起点/终点
    └── MacroCommand        — 组合多条命令为宏命令

SearchObserver（抽象观察者）
    └── ConsoleRenderer     — 控制台输出搜索事件
```

- **`GraphPathFinder`** 提取公共能力：坐标编解码、`reconstructPath`、耗时统计，避免 10 个算法重复实现。
- 启发式函数集中在 **`Heuristic`**，供 A* 族算法复用。

#### 3. 多态（Polymorphism）

- **编译期多态**：`Grid` 的接口设计、模板方法 `elapsedMs`。
- **运行期多态**（核心）：
  ```cpp
  PathfindingContext context;
  context.setStrategy(AlgorithmFactory::create(type));  // 返回 unique_ptr<PathFinder>
  PathResult result = context.run(grid);                // 虚函数 findPath() 动态绑定
  ```
- **命令多态**：`CommandManager` 以 `unique_ptr<MapCommand>` 存储命令，统一调用 `execute` / `undo`，无需区分具体命令类型。
- **观察者多态**：`SearchObserver::onSearchEvent` 虚函数，`ConsoleRenderer` 在控制台模式订阅事件；Web 模式则将 `visitedOrder` 写入 `PathResult` 供前端回放。

#### 4. 抽象（Abstraction）

- **`PathFinder`** 定义「给定 Grid，返回 PathResult」的统一契约，上层 `PathfindingService`、前端对比功能均只依赖该抽象。
- **`MapCommand`** 抽象「可执行且可撤销」的编辑操作，使撤销系统与具体编辑行为解耦。

---

## 设计模式详解

### 策略模式（Strategy）

| 角色 | 实现 |
|------|------|
| Strategy | `PathFinder` 及 10 个子类 |
| Context | `PathfindingContext` |
| Client | `PathfindingService`、`Application` |

**意图**：将寻路算法族封装为可互换策略，运行时切换而无需修改客户端代码。

**协作过程**：
1. `AlgorithmFactory::create(AlgorithmType)` 生产具体策略对象。
2. `PathfindingContext::setStrategy` 注入策略。
3. `context.run(grid)` 委托给 `strategy_->findPath(grid)`。

**优势**：新增算法只需：实现 `PathFinder` → 在 `AlgorithmFactory` 注册 → 前端算法列表自动可用。

### 观察者模式（Observer）

| 角色 | 实现 |
|------|------|
| Subject | `SearchSubject`（由 `GraphPathFinder` 继承使用） |
| Observer | `SearchObserver` → `ConsoleRenderer` |
| Event | `SearchEvent`（NodeVisited / NodeExpanded / PathFound / SearchFinished） |

**意图**：将搜索进度通知与核心寻路逻辑分离。

**协作过程**：算法在扩展节点时调用 `notify(event)`，`ConsoleRenderer` 在 verbose 模式下打印进度。Web 模式下算法同时将访问序列写入 `PathResult::visitedOrder`，由前端动画系统消费（观察者思想的延伸应用）。

### 命令模式（Command）

| 角色 | 实现 |
|------|------|
| Command | `MapCommand` 接口 |
| ConcreteCommand | `SetCellCommand`、`MoveMarkerCommand`、`MacroCommand` |
| Invoker | `CommandManager` |
| Receiver | `Grid` |

**意图**：将编辑操作对象化，支持撤销、重做与批量宏命令。

**协作过程**：
1. 前端一次拖拽产生多条 `setCell` 命令。
2. `MapSession::applyMacro` 合并为 `MacroCommand` 或单条命令执行。
3. `CommandManager::execute` 修改 `Grid` 并记录到 undo 栈。
4. `undo()` 调用最近命令的 `undo(grid)`，命令对象保存了 `oldType` / `newType` 等逆向信息。

**宏命令**：多条原子命令包装为一个 `MacroCommand`，撤销时整体回滚，保证 UX 连贯。

### 工厂方法（Factory Method）

| 角色 | 实现 |
|------|------|
| Factory | `AlgorithmFactory::create` |
| Product | `PathFinder` 各子类 |

**意图**：集中管理算法实例化，客户端通过 `AlgorithmType` 枚举或 id 获取算法，避免 `switch` 散落各处。

### 单例模式（Singleton）

- **`MapSession::instance()`**：保证 Web 服务进程内只有一份地图会话与命令历史，所有 `/api/session/*` 请求共享状态。

---

## 寻路算法一览

| ID | 算法 | 特点 |
|----|------|------|
| 1 | BFS | 无权图最短步数，按层扩展 |
| 2 | DFS | 深度优先，不保证最优 |
| 3 | Dijkstra | 经典单源最短路，等权网格 |
| 4 | A* | f = g + h，通常扩展节点较少 |
| 5 | Greedy Best-First | 仅按 h 扩展，快但不保证最优 |
| 6 | Weighted A* | f = g + w·h，平衡速度与最优性 |
| 7 | Bidirectional BFS | 起点终点同时 BFS |
| 8 | Bidirectional A* | 双向 A*，大图更高效 |
| 9 | IDA* | 迭代加深 + 启发式，省内存 |
| 10 | IDDFS | 迭代加深 DFS |

---

## 项目结构

```
C+ program/
├── 一键启动.bat / 一键启动.vbs / build.bat / build_mingw.bat / run.bat
├── scripts/
│   ├── setup_frontend.bat      # 下载 Bootstrap、Chart.js 等到 web/vendor
│   └── check_rebuild.ps1       # 检测源码是否比 exe 新，供一键启动自动编译
├── web/                        # 前端（静态资源 + JS）
│   ├── index.html
│   ├── css/app.css
│   ├── js/app.js               # 主逻辑：编辑、动画、对比、API 调用
│   ├── js/i18n.js              # 中英文文案
│   └── vendor/                 # Bootstrap / Chart.js / Icons
├── third_party/
│   ├── httplib/                # cpp-httplib HTTP 服务器
│   └── json/                   # nlohmann/json
├── maps/                       # 地图文本文件
│   ├── sample_map.txt
│   └── open_map.txt
└── src/
    ├── main.cpp
    ├── app/
    │   ├── Application.cpp       # 入口：web / console / demo 模式
    │   └── Application.h
    ├── api/
    │   ├── ApiServer.cpp         # HTTP 路由与静态文件服务
    │   ├── GridSerializer.cpp    # JSON 序列化、visitedOrder 采样
    │   ├── MapSession.cpp        # 会话单例 + 命令执行
    │   └── PathfindingService.cpp
    ├── core/
    │   ├── Grid.cpp / Grid.h     # 网格领域模型
    │   ├── MazeGenerator.cpp     # 迷宫与随机障碍
    │   ├── PathFinder.h          # 策略接口
    │   └── PathResult.h          # 寻路结果 DTO
    ├── algorithms/               # 10 种具体算法 + Heuristic
    │   ├── GraphPathFinder.cpp   # 公共基类
    │   ├── BFSPathFinder.cpp
    │   ├── AStarPathFinder.cpp
    │   └── ...
    ├── patterns/
    │   ├── AlgorithmFactory.cpp  # 工厂
    │   ├── PathfindingContext.cpp# 策略上下文
    │   ├── Observer.cpp          # 观察者
    │   ├── CommandManager.cpp    # 命令调用者
    │   └── commands/             # SetCell / MoveMarker / Macro
    └── ui/
        ├── ConsoleRenderer.cpp   # 控制台观察者
        └── HtmlExporter.cpp      # HTML 报告导出
```

---

## REST API

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/` | Web 主页面 |
| GET | `/api/algorithms` | 算法列表（id、name、description） |
| GET | `/api/maps` | 预设地图文件列表 |
| GET | `/api/maps/:name` | 加载指定地图并初始化会话 |
| POST | `/api/pathfind` | 运行单个算法，body: `{ algorithm, grid }` |
| POST | `/api/compare` | 多算法对比，body: `{ grid, algorithms?: [1,3,4] }` |
| POST | `/api/maze/random` | 生成迷宫，body: `{ rows, cols, seed? }` |
| POST | `/api/map/scatter` | 在当前 grid 上散点障碍，body: `{ grid, density?, seed? }` |
| GET | `/api/session/state` | 当前会话 grid 与 undo/redo 状态 |
| POST | `/api/session/load` | 加载 grid 并清空命令历史 |
| POST | `/api/session/macro` | 批量执行编辑命令 |
| POST | `/api/session/undo` | 撤销 |
| POST | `/api/session/redo` | 重做 |
| POST | `/api/maps/save` | 保存地图到 `maps/` |

---

## 地图文件格式

文本文件，第一行为行数与列数，之后每行对应网格一行：

```
15 21
0 0 0 0 ...
1 1 0 0 ...
```

| 数值 | 含义 |
|------|------|
| 0 | 空地 |
| 1 | 障碍 |
| 2 | 起点 |
| 3 | 终点 |

---

## 技术栈

| 层次 | 技术 |
|------|------|
| 语言 | C++17 |
| HTTP | [cpp-httplib](https://github.com/yhirose/cpp-httplib) |
| JSON | [nlohmann/json](https://github.com/nlohmann/json) |
| 前端 | Bootstrap 5、Bootstrap Icons、Chart.js |
| 构建 | CMake 或 MinGW 直接编译（`build_mingw.bat`） |

---

## 常见问题

| 问题 | 处理 |
|------|------|
| 浏览器显示 Failed to fetch | 确认 `一键启动.bat` 窗口未关闭，刷新页面 |
| 端口 8765 被占用 | 运行 `一键启动.bat 9000` |
| 修改 C++ 后功能未更新 | 关闭服务窗口后重新运行 `一键启动.bat`（会自动检测源码并编译） |
| 不要直接双击 HTML | 必须通过 HTTP 服务访问，否则 API 不可用 |

---

## 课程设计要点总结

本项目将 **OOP 四大特性** 与 **策略、观察者、命令、工厂、单例** 等设计模式落地到可运行的可视化系统中：

- **策略 + 工厂**：算法可扩展、可对比，符合开闭原则。
- **观察者**：搜索过程与输出/动画解耦。
- **命令**：编辑操作可撤销，支持宏命令批量处理。
- **分层架构**：`core` 领域模型 → `algorithms` / `patterns` → `api` 服务 → `web` 展示，职责清晰，便于答辩讲解与后续维护。
