(() => {
  "use strict";

  const STORAGE_KEY = "pathplanner_lang";

  const messages = {
    zh: {
      docTitle: "多策略路径规划可视化系统",
      brandTitle: "多策略路径规划可视化",
      presetMapTitle: "加载预设地图",
      presetMapPlaceholder: "选择预设地图...",
      help: "帮助",
      sectionEditTools: "编辑工具",
      toolWall: "障碍",
      toolErase: "填充",
      toolStart: "起点",
      toolGoal: "终点",
      toolWallTitle: "左键绘制障碍",
      toolEraseTitle: "左键填充空地（清除障碍）",
      toolStartTitle: "左键放置起点",
      toolGoalTitle: "左键放置终点",
      editHint: "左键应用工具 · 右键快速填充 · 拖拽连续编辑 · Ctrl+Z/Y 撤销",
      undo: "撤销",
      redo: "重做",
      undoTitle: "撤销 (Ctrl+Z)",
      redoTitle: "重做 (Ctrl+Y)",
      undoPrefix: "撤销：{desc} (Ctrl+Z)",
      redoPrefix: "重做：{desc} (Ctrl+Y)",
      sectionMapSize: "地图尺寸",
      mapSizeHint: "自定义行/列（5~80），大地图自动缩小格子",
      labelRows: "行",
      labelCols: "列",
      btnApply: "应用",
      btnApplyTitle: "应用自定义尺寸",
      sectionMapGen: "地图生成",
      btnGenMaze: "生成迷宫",
      btnGenScatter: "随机散点障碍",
      btnClear: "清空地图",
      sectionSaveMap: "保存地图",
      btnSave: "保存",
      legendStart: "起点",
      legendGoal: "终点",
      legendWall: "障碍",
      legendVisited: "已访问",
      legendPath: "路径",
      gridAriaLabel: "路径规划地图",
      sectionAlgorithm: "寻路算法",
      btnRun: "运行寻路",
      btnCompare: "算法对比 ▾",
      selectAll: "全选",
      btnCompareRun: "对比选中算法",
      sectionAnimation: "搜索动画",
      speed: "速度",
      speedFast: "快",
      speedMedium: "中",
      speedSlow: "慢",
      sectionResults: "运行结果",
      statsEmpty: "点击「运行寻路」或「算法对比」查看结果",
      sectionCompare: "性能对比",
      helpTitle: "操作说明",
      help1: "<strong>编辑</strong>：先选工具，再左键编辑；右键任意位置可快速擦除。",
      help2: "<strong>运行寻路</strong>：选算法后点主按钮，或按 <kbd>Enter</kbd>。",
      help3: "<strong>随机散点障碍</strong>：在当前地图上随机添加障碍，不重置地图。",
      help4: "<strong>算法对比</strong>：勾选算法对比后，点击下方各算法即可播放寻路动画。",
      help5: "<strong>填充</strong>：左键或右键将格子变为空地，可快速修正误画的障碍。",
      help6: "<strong>快捷键</strong>：1障碍 2填充 3起点 4终点 · Enter 运行 · Esc 停动画",
      langSelectTitle: "界面语言",
      currentTool: "当前工具：{tool}",
      coord: "坐标：({row}, {col})",
      coordDash: "坐标：-",
      mapSize: "地图：{rows} × {cols}",
      currentNode: "当前节点：({row}, {col})",
      currentNodeDash: "当前节点：-",
      stepLabel: "步骤 {step} / {max}",
      stepSampled: "步骤 {step} / {max}（采样自 {total} 步）",
      stepZero: "步骤 0 / 0",
      resultFound: "找到路径",
      resultNotFound: "未找到路径",
      resultLabel: "结果：",
      pathLength: "路径长度：",
      expandedNodes: "扩展节点：",
      visitedNodes: "访问节点：",
      elapsed: "耗时：",
      elapsedMs: "{ms} ms",
      compareSummary: "已对比 <strong>{count}</strong> 种算法，点击下方算法或图表即可播放寻路动画",
      compareDone: "已对比 {count} 种算法",
      pathfindDone: "寻路完成",
      selectOneAlgo: "请至少选择一种算法",
      scatterDone: "已在当前地图随机添加障碍",
      scatterFail: "未能添加障碍，请降低地图密度或重试",
      mazeDone: "迷宫已生成",
      mapResized: "地图已设为 {rows}×{cols}",
      clearConfirm: "确定清空所有障碍并重置起终点？",
      mapCleared: "地图已清空",
      resizeConfirm: "将地图设为 {rows}×{cols}，当前内容会被重置，是否继续？",
      saveExtFail: "文件名请以 .txt 结尾",
      mapSaved: "地图已保存到 maps/{filename}",
      mapLoaded: "已加载地图：{name}",
      initFail: "初始化失败：{msg}",
      chartExpanded: "扩展节点",
      chartElapsed: "耗时(ms)",
      chartTooltipElapsed: "耗时: {ms} ms",
      apiFileProtocol: "请双击「一键启动.bat」启动，不要直接打开 HTML 文件",
      apiRequestFailed: "请求失败 ({status})",
      apiCancelled: "请求已取消或超时，请重试",
      apiNoBackend: "无法连接后端：请确认「一键启动.bat」窗口未关闭，然后刷新页面",
      cancelledKeyword: "已取消",
    },
    en: {
      docTitle: "Multi-Strategy Path Planning Visualizer",
      brandTitle: "Path Planning Visualizer",
      presetMapTitle: "Load preset map",
      presetMapPlaceholder: "Select preset map...",
      help: "Help",
      sectionEditTools: "Edit Tools",
      toolWall: "Wall",
      toolErase: "Fill",
      toolStart: "Start",
      toolGoal: "Goal",
      toolWallTitle: "Left-click to draw walls",
      toolEraseTitle: "Left-click to clear cell (remove wall)",
      toolStartTitle: "Left-click to place start",
      toolGoalTitle: "Left-click to place goal",
      editHint: "LMB apply tool · RMB quick fill · drag to paint · Ctrl+Z/Y undo",
      undo: "Undo",
      redo: "Redo",
      undoTitle: "Undo (Ctrl+Z)",
      redoTitle: "Redo (Ctrl+Y)",
      undoPrefix: "Undo: {desc} (Ctrl+Z)",
      redoPrefix: "Redo: {desc} (Ctrl+Y)",
      sectionMapSize: "Map Size",
      mapSizeHint: "Custom rows/cols (5–80); large maps shrink cells automatically",
      labelRows: "Rows",
      labelCols: "Cols",
      btnApply: "Apply",
      btnApplyTitle: "Apply custom size",
      sectionMapGen: "Map Generator",
      btnGenMaze: "Generate Maze",
      btnGenScatter: "Random Obstacles",
      btnClear: "Clear Map",
      sectionSaveMap: "Save Map",
      btnSave: "Save",
      legendStart: "Start",
      legendGoal: "Goal",
      legendWall: "Wall",
      legendVisited: "Visited",
      legendPath: "Path",
      gridAriaLabel: "Path planning grid",
      sectionAlgorithm: "Pathfinding",
      btnRun: "Run",
      btnCompare: "Compare ▾",
      selectAll: "Select all",
      btnCompareRun: "Compare Selected",
      sectionAnimation: "Search Animation",
      speed: "Speed",
      speedFast: "Fast",
      speedMedium: "Medium",
      speedSlow: "Slow",
      sectionResults: "Results",
      statsEmpty: "Click Run or Compare to see results",
      sectionCompare: "Performance Compare",
      helpTitle: "How to Use",
      help1: "<strong>Edit</strong>: Pick a tool, then left-click; right-click anywhere to erase quickly.",
      help2: "<strong>Run</strong>: Choose an algorithm and click Run, or press <kbd>Enter</kbd>.",
      help3: "<strong>Random obstacles</strong>: Scatter walls on the current map without resetting it.",
      help4: "<strong>Compare</strong>: Select algorithms, then click an entry below to play its animation.",
      help5: "<strong>Fill</strong>: Turn a cell into open space to fix misplaced walls.",
      help6: "<strong>Shortcuts</strong>: 1 wall 2 fill 3 start 4 goal · Enter run · Esc stop animation",
      langSelectTitle: "Language",
      currentTool: "Tool: {tool}",
      coord: "Coord: ({row}, {col})",
      coordDash: "Coord: -",
      mapSize: "Map: {rows} × {cols}",
      currentNode: "Node: ({row}, {col})",
      currentNodeDash: "Node: -",
      stepLabel: "Step {step} / {max}",
      stepSampled: "Step {step} / {max} (sampled from {total})",
      stepZero: "Step 0 / 0",
      resultFound: "Path found",
      resultNotFound: "No path",
      resultLabel: "Result: ",
      pathLength: "Path length: ",
      expandedNodes: "Expanded: ",
      visitedNodes: "Visited: ",
      elapsed: "Time: ",
      elapsedMs: "{ms} ms",
      compareSummary: "Compared <strong>{count}</strong> algorithms — click below or on the chart to play",
      compareDone: "Compared {count} algorithms",
      pathfindDone: "Pathfinding complete",
      selectOneAlgo: "Select at least one algorithm",
      scatterDone: "Random obstacles added to current map",
      scatterFail: "Could not add obstacles — reduce density or retry",
      mazeDone: "Maze generated",
      mapResized: "Map set to {rows}×{cols}",
      clearConfirm: "Clear all walls and reset start/goal?",
      mapCleared: "Map cleared",
      resizeConfirm: "Set map to {rows}×{cols}? Current content will be reset.",
      saveExtFail: "Filename must end with .txt",
      mapSaved: "Map saved to maps/{filename}",
      mapLoaded: "Loaded map: {name}",
      initFail: "Init failed: {msg}",
      chartExpanded: "Expanded nodes",
      chartElapsed: "Time (ms)",
      chartTooltipElapsed: "Time: {ms} ms",
      apiFileProtocol: "Run 一键启动.bat — do not open the HTML file directly",
      apiRequestFailed: "Request failed ({status})",
      apiCancelled: "Request cancelled or timed out — retry",
      apiNoBackend: "Cannot reach backend — keep 一键启动.bat running and refresh",
      cancelledKeyword: "cancelled",
    },
  };

  const algoDescriptions = {
    zh: {
      1: "按层扩展，保证无权图最短步数",
      2: "深度优先，不保证最优，用于对比",
      3: "经典单源最短路径，等权网格稳定",
      4: "启发式 f=g+h，通常扩展节点最少",
      5: "仅按 h 值扩展，速度快但不保证最优",
      6: "加权 f=g+w·h，平衡速度与最优性",
      7: "起点终点同时 BFS，相遇即停",
      8: "双向 A* 搜索，大图效率更高",
      9: "迭代加深 + 启发式，省内存",
      10: "迭代加深 DFS，逐步增加深度限制",
    },
    en: {
      1: "Layer-by-layer; guarantees shortest steps on unweighted grids",
      2: "Depth-first; not optimal — useful for comparison",
      3: "Classic single-source shortest path on equal-cost grids",
      4: "Heuristic f=g+h; usually fewest expanded nodes",
      5: "Expands by h only; fast but not guaranteed optimal",
      6: "Weighted f=g+w·h; balances speed and optimality",
      7: "Bidirectional BFS from start and goal",
      8: "Bidirectional A*; efficient on large maps",
      9: "Iterative deepening + heuristic; memory-friendly",
      10: "Iterative deepening DFS with increasing depth limit",
    },
  };

  const apiErrorMap = {
    zh: {},
    en: {
      "地图不存在": "Map not found",
      "起点或终点不可达，请检查位置": "Start or goal unreachable — check positions",
      "起点或终点不可达": "Start or goal unreachable",
      "无法添加障碍：会破坏起点到终点的通路": "Cannot add walls — would block start-to-goal path",
      "未能添加障碍，请降低地图障碍密度或重试": "Could not add obstacles — lower density or retry",
    },
  };

  let lang = "en";

  function format(template, params = {}) {
    return template.replace(/\{(\w+)\}/g, (_, key) => (params[key] != null ? params[key] : `{${key}}`));
  }

  function t(key, params) {
    const table = messages[lang] || messages.zh;
    const text = table[key] ?? messages.zh[key] ?? key;
    return params ? format(text, params) : text;
  }

  function getLang() {
    return lang;
  }

  function algoDesc(id) {
    const table = algoDescriptions[lang] || algoDescriptions.zh;
    return table[id] || "";
  }

  function translateCommandDesc(desc) {
    if (!desc || lang === "zh") return desc;
    if (desc === "移动起点") return "Move start";
    if (desc === "移动终点") return "Move goal";
    const m = desc.match(/^编辑单元 \((\d+),(\d+)\)$/);
    if (m) return `Edit cell (${m[1]},${m[2]})`;
    return desc;
  }

  function translateError(msg) {
    if (!msg || lang === "zh") return msg;
    const map = apiErrorMap.en;
    return map[msg] || msg;
  }

  function applyStatic() {
    document.documentElement.lang = lang === "zh" ? "zh-CN" : "en";
    document.title = t("docTitle");

    document.querySelectorAll("[data-i18n]").forEach((el) => {
      el.textContent = t(el.dataset.i18n);
    });
    document.querySelectorAll("[data-i18n-title]").forEach((el) => {
      el.title = t(el.dataset.i18nTitle);
    });
    document.querySelectorAll("[data-i18n-placeholder]").forEach((el) => {
      el.placeholder = t(el.dataset.i18nPlaceholder);
    });
    document.querySelectorAll("[data-i18n-html]").forEach((el) => {
      el.innerHTML = t(el.dataset.i18nHtml);
    });
    document.querySelectorAll("[data-i18n-aria]").forEach((el) => {
      el.setAttribute("aria-label", t(el.dataset.i18nAria));
    });

    const langSelect = document.getElementById("langSelect");
    if (langSelect) langSelect.value = lang;
  }

  function setLang(nextLang) {
    if (nextLang !== "zh" && nextLang !== "en") return;
    lang = nextLang;
    try {
      localStorage.setItem(STORAGE_KEY, lang);
    } catch {
      /* ignore */
    }
    applyStatic();
  }

  function init() {
    try {
      const saved = localStorage.getItem(STORAGE_KEY);
      if (saved === "zh" || saved === "en") lang = saved;
    } catch {
      /* ignore */
    }
    applyStatic();
    const statsPanel = document.getElementById("statsPanel");
    if (statsPanel && statsPanel.classList.contains("empty-state")) {
      statsPanel.textContent = t("statsEmpty");
    }
  }

  window.I18n = {
    t,
    getLang,
    setLang,
    init,
    applyStatic,
    algoDesc,
    translateCommandDesc,
    translateError,
    format,
  };
})();
