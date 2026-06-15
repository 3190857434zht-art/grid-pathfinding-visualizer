(() => {
  "use strict";

  const CellType = { Empty: 0, Wall: 1, Start: 2, Goal: 3 };
  const MIN_SIZE = 5;
  const MAX_SIZE = 80;

  const state = {
    rows: 15,
    cols: 21,
    cells: [],
    start: { row: 1, col: 1 },
    goal: { row: 13, col: 19 },
    tool: "wall",
    isDrawing: false,
    pendingBatch: [],
    visitedOrder: [],
    path: [],
    animStep: 0,
    animTimer: null,
    compareChart: null,
    lastResult: null,
    compareResults: null,
    algorithms: [],
    statsVisitedTotal: 0,
    canUndo: false,
    canRedo: false,
  };

  const $ = (id) => document.getElementById(id);
  const t = (key, params) => I18n.t(key, params);

  const els = {
    gridBoard: $("gridBoard"),
    gridWrapper: $("gridWrapper"),
    presetMapSelect: $("presetMapSelect"),
    algorithmSelect: $("algorithmSelect"),
    algorithmDesc: $("algorithmDesc"),
    statsPanel: $("statsPanel"),
    compareSection: $("compareSection"),
    compareChart: $("compareChart"),
    animSlider: $("animSlider"),
    animStepLabel: $("animStepLabel"),
    animCurrentLabel: $("animCurrentLabel"),
    statusTool: $("statusTool"),
    statusCoord: $("statusCoord"),
    statusMap: $("statusMap"),
    btnUndo: $("btnUndo"),
    btnRedo: $("btnRedo"),
    comparePanel: $("comparePanel"),
    compareAlgoList: $("compareAlgoList"),
    comparePlaybackList: $("comparePlaybackList"),
  };

  const getToolName = (tool) => {
    const map = { wall: t("toolWall"), erase: t("toolErase"), start: t("toolStart"), goal: t("toolGoal") };
    return map[tool] || tool;
  };

  let busyCount = 0;
  let apiAbort = null;
  let renderScheduled = false;

  function setBusy(busy) {
    busyCount += busy ? 1 : -1;
    if (busyCount < 0) busyCount = 0;
    const disabled = busyCount > 0;
    ["btnRun", "btnCompareRun", "btnCompareToggle", "btnGenMaze", "btnGenScatter", "btnClear", "btnResize", "btnSaveMap"].forEach((id) => {
      const el = $(id);
      if (el) el.disabled = disabled;
    });
    document.body.classList.toggle("is-busy", disabled);
  }

  async function api(path, options = {}, timeoutMs = 60000) {
    if (location.protocol === "file:") {
      throw new Error(t("apiFileProtocol"));
    }

    if (options.cancelPrevious && apiAbort) {
      apiAbort.abort();
    }

    const controller = new AbortController();
    if (options.cancelPrevious) {
      apiAbort = controller;
    }

    const timer = setTimeout(() => controller.abort(), timeoutMs);
    const { cancelPrevious, ...fetchOptions } = options;

    try {
      const res = await fetch(path, {
        headers: { "Content-Type": "application/json" },
        signal: controller.signal,
        ...fetchOptions,
      });
      const data = await res.json().catch(() => ({}));
      if (!res.ok) {
        throw new Error(I18n.translateError(data.error || t("apiRequestFailed", { status: res.status })));
      }
      return data;
    } catch (err) {
      if (err.name === "AbortError") {
        throw new Error(t("apiCancelled"));
      }
      if (err.message === "Failed to fetch") {
        throw new Error(t("apiNoBackend"));
      }
      throw err;
    } finally {
      clearTimeout(timer);
    }
  }

  function isCancelledError(message) {
    return message.includes(t("cancelledKeyword")) || message.includes("已取消");
  }

  function toast(message, type = "info") {
    const node = document.createElement("div");
    node.className = `app-toast ${type}`;
    node.textContent = message;
    document.body.appendChild(node);
    setTimeout(() => node.remove(), 3200);
  }

  function createEmptyCells(rows, cols) {
    return Array.from({ length: rows }, () => Array(cols).fill(CellType.Empty));
  }

  function cloneGridData() {
    return {
      rows: state.rows,
      cols: state.cols,
      cells: state.cells.map((row) => [...row]),
      start: { ...state.start },
      goal: { ...state.goal },
    };
  }

  function computeCellSize() {
    const maxW = Math.max(200, els.gridWrapper.clientWidth - 40);
    const maxH = Math.max(200, els.gridWrapper.clientHeight - 40);
    return Math.max(8, Math.min(32, Math.floor(Math.min(maxW / state.cols, maxH / state.rows))));
  }

  function applyGridData(data, keepAnimation = false) {
    state.rows = data.rows;
    state.cols = data.cols;
    state.cells = data.cells.map((row) => row.map(Number));
    state.start = { ...data.start };
    state.goal = { ...data.goal };
    $("mapRows").value = state.rows;
    $("mapCols").value = state.cols;
    if (!keepAnimation) resetAnimation();
    renderGrid();
    updateStatusBar();
  }

  function applySessionResponse(response, keepAnimation = false) {
    applyGridData(response.grid, keepAnimation);
    state.canUndo = !!response.canUndo;
    state.canRedo = !!response.canRedo;
    els.btnUndo.disabled = !state.canUndo;
    els.btnRedo.disabled = !state.canRedo;
    els.btnUndo.title = response.undoDescription
      ? t("undoPrefix", { desc: I18n.translateCommandDesc(response.undoDescription) })
      : t("undoTitle");
    els.btnRedo.title = response.redoDescription
      ? t("redoPrefix", { desc: I18n.translateCommandDesc(response.redoDescription) })
      : t("redoTitle");
  }

  async function syncSessionLoad(gridData) {
    const response = await api("/api/session/load", {
      method: "POST",
      body: JSON.stringify({ grid: gridData }),
    });
    applySessionResponse(response);
    return response;
  }

  function cellKey(row, col) {
    return `${row},${col}`;
  }

  function isWalkableCell(cell) {
    return cell !== CellType.Wall;
  }

  function isGridConnected(cells, start, goal, rows, cols) {
    if (!isWalkableCell(cells[start.row][start.col]) || !isWalkableCell(cells[goal.row][goal.col])) {
      return false;
    }
    const queue = [[start.row, start.col]];
    const seen = new Set([cellKey(start.row, start.col)]);
    while (queue.length > 0) {
      const [r, c] = queue.shift();
      if (r === goal.row && c === goal.col) return true;
      for (const [dr, dc] of [
        [-1, 0],
        [1, 0],
        [0, -1],
        [0, 1],
      ]) {
        const nr = r + dr;
        const nc = c + dc;
        if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
        const key = cellKey(nr, nc);
        if (seen.has(key) || !isWalkableCell(cells[nr][nc])) continue;
        seen.add(key);
        queue.push([nr, nc]);
      }
    }
    return false;
  }

  function buildScatteredGrid(density = 0.12, maxAttempts = 8) {
    const { rows, cols, start, goal } = state;
    for (let attempt = 0; attempt < maxAttempts; attempt++) {
      const cells = state.cells.map((row) => [...row]);
      let changed = false;
      for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
          if ((r === start.row && c === start.col) || (r === goal.row && c === goal.col)) continue;
          if (cells[r][c] !== CellType.Wall && Math.random() < density) {
            cells[r][c] = CellType.Wall;
            changed = true;
          }
        }
      }
      if (changed && isGridConnected(cells, start, goal, rows, cols)) {
        return { rows, cols, cells, start: { ...start }, goal: { ...goal } };
      }
    }
    return null;
  }

  function scheduleRenderGrid() {
    if (renderScheduled) return;
    renderScheduled = true;
    requestAnimationFrame(() => {
      renderScheduled = false;
      renderGrid();
    });
  }

  function renderGrid() {
    const cellSize = computeCellSize();
    els.gridBoard.style.gridTemplateColumns = `repeat(${state.cols}, ${cellSize}px)`;

    const totalSteps = state.visitedOrder.length;
    const showPath = state.animStep >= totalSteps && totalSteps > 0;
    const visitedSlice = state.visitedOrder.slice(0, state.animStep);
    const visitedSet = new Set(visitedSlice.map((p) => cellKey(p.row, p.col)));
    const pathSet = showPath ? new Set(state.path.map((p) => cellKey(p.row, p.col))) : new Set();
    const current = state.animStep > 0 ? state.visitedOrder[state.animStep - 1] : null;
    const previous = state.animStep > 1 ? state.visitedOrder[state.animStep - 2] : null;

    if (current) {
      els.animCurrentLabel.textContent = t("currentNode", { row: current.row, col: current.col });
    } else if (totalSteps === 0) {
      els.animCurrentLabel.textContent = t("currentNodeDash");
    }

    els.gridBoard.innerHTML = "";
    const fragment = document.createDocumentFragment();
    for (let r = 0; r < state.rows; r++) {
      for (let c = 0; c < state.cols; c++) {
        const cell = document.createElement("div");
        cell.className = "grid-cell";
        cell.style.width = `${cellSize}px`;
        cell.style.height = `${cellSize}px`;
        cell.dataset.row = r;
        cell.dataset.col = c;

        const value = state.cells[r][c];
        if (value === CellType.Wall) cell.classList.add("wall");
        if (value === CellType.Start) cell.classList.add("start");
        if (value === CellType.Goal) cell.classList.add("goal");

        const key = cellKey(r, c);
        if (visitedSet.has(key)) cell.classList.add("visited");
        if (pathSet.has(key)) cell.classList.add("on-path");
        if (current && current.row === r && current.col === c) cell.classList.add("current");
        if (previous && previous.row === r && previous.col === c && !(current && current.row === r && current.col === c)) {
          cell.classList.add("just-visited");
        }

        cell.addEventListener("mousedown", onCellMouseDown);
        cell.addEventListener("mouseenter", onCellMouseEnter);
        cell.addEventListener("contextmenu", (e) => e.preventDefault());
        fragment.appendChild(cell);
      }
    }
    els.gridBoard.appendChild(fragment);

    if (current) {
      const selector = `.grid-cell[data-row="${current.row}"][data-col="${current.col}"]`;
      const node = els.gridBoard.querySelector(selector);
      if (node) node.scrollIntoView({ block: "nearest", inline: "nearest", behavior: "smooth" });
    }
  }

  function updateStatusBar() {
    els.statusTool.textContent = t("currentTool", { tool: getToolName(state.tool) });
    els.statusMap.textContent = t("mapSize", { rows: state.rows, cols: state.cols });
  }

  function setTool(tool) {
    state.tool = tool;
    document.querySelectorAll(".tool-btn").forEach((btn) => {
      btn.classList.toggle("active", btn.dataset.tool === tool);
    });
    updateStatusBar();
  }

  function queueSetCell(row, col, oldType, newType) {
    if (oldType === newType) return;
    const existing = state.pendingBatch.find((c) => c.type === "setCell" && c.row === row && c.col === col);
    if (existing) {
      existing.newType = newType;
      if (existing.oldType === newType) {
        state.pendingBatch = state.pendingBatch.filter((c) => c !== existing);
      }
    } else {
      state.pendingBatch.push({ type: "setCell", row, col, oldType, newType });
    }
  }

  function queueMoveStart(oldPos, newPos, oldAtNew) {
    state.pendingBatch.push({
      type: "moveStart",
      oldRow: oldPos.row,
      oldCol: oldPos.col,
      newRow: newPos.row,
      newCol: newPos.col,
      oldAtNew,
    });
  }

  function queueMoveGoal(oldPos, newPos, oldAtNew) {
    state.pendingBatch.push({
      type: "moveGoal",
      oldRow: oldPos.row,
      oldCol: oldPos.col,
      newRow: newPos.row,
      newCol: newPos.col,
      oldAtNew,
    });
  }

  function applyToolLocal(row, col, tool = state.tool) {
    if (row < 0 || col < 0 || row >= state.rows || col >= state.cols) return false;

    if (tool === "start") {
      if (state.cells[row][col] === CellType.Wall) return false;
      if (state.start.row === row && state.start.col === col) return false;
      const oldAtNew = state.cells[row][col];
      const oldStart = { ...state.start };
      if (state.start.row !== state.goal.row || state.start.col !== state.goal.col) {
        state.cells[state.start.row][state.start.col] = CellType.Empty;
      }
      state.start = { row, col };
      state.cells[row][col] = CellType.Start;
      queueMoveStart(oldStart, { row, col }, oldAtNew);
      return true;
    }

    if (tool === "goal") {
      if (state.cells[row][col] === CellType.Wall) return false;
      if (state.goal.row === row && state.goal.col === col) return false;
      const oldAtNew = state.cells[row][col];
      const oldGoal = { ...state.goal };
      if (state.goal.row !== state.start.row || state.goal.col !== state.start.col) {
        state.cells[state.goal.row][state.goal.col] = CellType.Empty;
      }
      state.goal = { row, col };
      state.cells[row][col] = CellType.Goal;
      queueMoveGoal(oldGoal, { row, col }, oldAtNew);
      return true;
    }

    const oldType = state.cells[row][col];
    let newType = oldType;

    if (tool === "erase") {
      if (oldType === CellType.Start || oldType === CellType.Goal) return false;
      newType = CellType.Empty;
    } else if (tool === "wall") {
      if (oldType === CellType.Start || oldType === CellType.Goal) return false;
      newType = CellType.Wall;
    }

    if (oldType === newType) return false;
    state.cells[row][col] = newType;
    queueSetCell(row, col, oldType, newType);
    return true;
  }

  async function flushPendingBatch() {
    if (state.pendingBatch.length === 0) return;
    const batch = state.pendingBatch;
    state.pendingBatch = [];
    try {
      const response = await api("/api/session/macro", {
        method: "POST",
        body: JSON.stringify({ commands: batch }),
      });
      applySessionResponse(response, false);
    } catch (err) {
      toast(err.message, "error");
      await syncSessionLoad(cloneGridData());
    }
  }

  function onCellMouseDown(e) {
    const row = Number(e.currentTarget.dataset.row);
    const col = Number(e.currentTarget.dataset.col);

    state.isDrawing = true;
    state.pendingBatch = [];
    const tool = e.button === 2 ? "erase" : state.tool;
    if (applyToolLocal(row, col, tool)) {
      resetAnimation();
      renderGrid();
    }
  }

  function onCellMouseEnter(e) {
    const row = Number(e.currentTarget.dataset.row);
    const col = Number(e.currentTarget.dataset.col);
    els.statusCoord.textContent = t("coord", { row, col });
    if (!state.isDrawing) return;
    const tool = e.buttons === 2 ? "erase" : e.buttons === 1 ? state.tool : null;
    if (!tool) return;
    if (applyToolLocal(row, col, tool)) {
      scheduleRenderGrid();
    }
  }

  document.addEventListener("mouseup", () => {
    if (state.isDrawing) {
      state.isDrawing = false;
      flushPendingBatch();
    }
  });

  function resetAnimation() {
    stopAnimation();
    state.visitedOrder = [];
    state.path = [];
    state.animStep = 0;
    state.lastResult = null;
    state.compareResults = null;
    els.animSlider.value = 0;
    els.animSlider.max = 0;
    els.animSlider.disabled = true;
    $("btnPlay").disabled = true;
    $("btnPause").disabled = true;
    $("btnResetAnim").disabled = true;
    els.animStepLabel.textContent = t("stepZero");
    els.animCurrentLabel.textContent = t("currentNodeDash");
    els.compareSection.classList.add("d-none");
    if (els.comparePlaybackList) els.comparePlaybackList.innerHTML = "";
  }

  function stopAnimation() {
    if (state.animTimer) {
      clearInterval(state.animTimer);
      state.animTimer = null;
    }
  }

  function setupAnimation(result) {
    state.visitedOrder = result.visitedOrder || [];
    state.path = result.path || [];
    state.statsVisitedTotal = result.stats?.visitedTotal || state.visitedOrder.length;
    state.animStep = 0;
    const max = state.visitedOrder.length;
    els.animSlider.max = String(max);
    els.animSlider.value = "0";
    els.animSlider.disabled = max === 0;
    $("btnPlay").disabled = max === 0;
    $("btnPause").disabled = true;
    $("btnResetAnim").disabled = max === 0;
    els.animStepLabel.textContent = t("stepLabel", { step: 0, max });
    renderGrid();
  }

  function setAnimStep(step) {
    state.animStep = Math.max(0, Math.min(step, Number(els.animSlider.max)));
    els.animSlider.value = String(state.animStep);
    const total = state.visitedOrder.length;
    const label = total > 0 && state.statsVisitedTotal && state.statsVisitedTotal > total
      ? t("stepSampled", { step: state.animStep, max: els.animSlider.max, total: state.statsVisitedTotal })
      : t("stepLabel", { step: state.animStep, max: els.animSlider.max });
    els.animStepLabel.textContent = label;
    scheduleRenderGrid();
  }

  function playAnimation() {
    stopAnimation();
    if (state.visitedOrder.length === 0) return;
    const speed = Number($("animSpeed").value);
    $("btnPlay").disabled = true;
    $("btnPause").disabled = false;
    state.animTimer = setInterval(() => {
      if (state.animStep >= state.visitedOrder.length) {
        stopAnimation();
        $("btnPlay").disabled = false;
        $("btnPause").disabled = true;
        renderGrid();
        return;
      }
      setAnimStep(state.animStep + 1);
    }, speed);
  }

  function renderStats(result) {
    els.statsPanel.classList.remove("empty-state");
    els.statsPanel.innerHTML = `
      <div><strong>${result.algorithmName}</strong></div>
      <div>${t("resultLabel")}<span class="badge ${result.found ? "badge-found" : "badge-miss"}">${result.found ? t("resultFound") : t("resultNotFound")}</span></div>
      <div>${t("pathLength")}${result.stats.pathLength}</div>
      <div>${t("expandedNodes")}${result.stats.expandedNodes}</div>
      <div>${t("visitedNodes")}${result.stats.visitedNodes}</div>
      <div>${t("elapsed")}${t("elapsedMs", { ms: result.stats.elapsedMs.toFixed(3) })}</div>
    `;
  }

  function setStatsEmpty() {
    els.statsPanel.classList.add("empty-state");
    els.statsPanel.textContent = t("statsEmpty");
  }

  function playCompareResult(result, index) {
    stopAnimation();
    state.lastResult = result;
    renderStats(result);
    setupAnimation(result);
    els.comparePlaybackList.querySelectorAll(".compare-play-btn").forEach((btn, i) => {
      btn.classList.toggle("active", i === index);
    });
    playAnimation();
  }

  function renderComparePlayback(results) {
    els.comparePlaybackList.innerHTML = results
      .map(
        (r, i) => `
      <button type="button" class="btn btn-outline-secondary btn-sm compare-play-btn w-100" data-index="${i}">
        <span>${r.algorithmName}</span>
        <span class="play-icon"><i class="bi bi-play-fill"></i></span>
      </button>`,
      )
      .join("");
    els.comparePlaybackList.querySelectorAll(".compare-play-btn").forEach((btn) => {
      btn.addEventListener("click", () => {
        const index = Number(btn.dataset.index);
        playCompareResult(results[index], index);
      });
    });
  }

  function renderCompareChart(results) {
    els.compareSection.classList.remove("d-none");
    const labels = results.map((r) => {
      const name = r.algorithmName;
      return name.length > 14 ? name.slice(0, 12) + "…" : name;
    });
    const expandedData = results.map((r) => r.stats.expandedNodes);
    const timeData = results.map((r) => r.stats.elapsedMs);
    const maxTime = Math.max(...timeData, 0.001);

    if (state.compareChart) state.compareChart.destroy();
    state.compareChart = new Chart(els.compareChart, {
      type: "bar",
      data: {
        labels,
        datasets: [
          {
            label: t("chartExpanded"),
            data: expandedData,
            yAxisID: "yNodes",
            backgroundColor: "rgba(56,189,248,0.7)",
            borderRadius: 4,
          },
          {
            label: t("chartElapsed"),
            data: timeData,
            yAxisID: "yTime",
            backgroundColor: "rgba(34,197,94,0.65)",
            borderRadius: 4,
          },
        ],
      },
      options: {
        responsive: true,
        interaction: { mode: "index", intersect: false },
        onClick: (_evt, elements) => {
          if (elements.length > 0) {
            const index = elements[0].index;
            playCompareResult(results[index], index);
          }
        },
        plugins: {
          legend: { labels: { color: "#cbd5e1", boxWidth: 12, font: { size: 11 } } },
          tooltip: {
            callbacks: {
              label(ctx) {
                const value = ctx.parsed.y;
                if (ctx.dataset.yAxisID === "yTime") {
                  return t("chartTooltipElapsed", { ms: Number(value).toFixed(3) });
                }
                return `${ctx.dataset.label}: ${value}`;
              },
            },
          },
        },
        scales: {
          x: { ticks: { color: "#94a3b8", maxRotation: 45, minRotation: 45, font: { size: 10 } }, grid: { color: "rgba(148,163,184,0.08)" } },
          yNodes: {
            type: "linear",
            position: "left",
            beginAtZero: true,
            title: { display: true, text: t("chartExpanded"), color: "#38bdf8", font: { size: 11 } },
            ticks: { color: "#94a3b8" },
            grid: { color: "rgba(148,163,184,0.08)" },
          },
          yTime: {
            type: "linear",
            position: "right",
            beginAtZero: true,
            suggestedMax: maxTime * 1.2,
            title: { display: true, text: t("chartElapsed"), color: "#22c55e", font: { size: 11 } },
            ticks: {
              color: "#94a3b8",
              maxTicksLimit: 6,
              callback: (value) => (value < 1 ? Number(value).toFixed(2) : Number(value).toFixed(1)),
            },
            grid: { drawOnChartArea: false },
          },
        },
      },
    });
  }

  async function loadAlgorithms() {
    state.algorithms = await api("/api/algorithms");
    els.algorithmSelect.innerHTML = state.algorithms
      .map((a) => `<option value="${a.id}">${a.name}</option>`)
      .join("");
    els.algorithmSelect.value = "4";
    updateAlgorithmDesc();
    buildCompareList();
  }

  function buildCompareList() {
    const defaultChecked = new Set([1, 3, 4]);
    els.compareAlgoList.innerHTML = state.algorithms
      .map((a) => {
        const checked = defaultChecked.has(a.id) ? "checked" : "";
        const shortName = a.name.length > 18 ? a.name.slice(0, 16) + "…" : a.name;
        return `<label class="compare-algo-item"><input type="checkbox" value="${a.id}" ${checked}><span>${shortName}</span></label>`;
      })
      .join("");
  }

  function getSelectedCompareIds() {
    return [...els.compareAlgoList.querySelectorAll("input:checked")].map((el) => Number(el.value));
  }

  function updateAlgorithmDesc() {
    const id = Number(els.algorithmSelect.value);
    els.algorithmDesc.textContent = I18n.algoDesc(id);
  }

  async function loadPresetMaps() {
    const maps = await api("/api/maps");
    maps.forEach((m) => {
      const opt = document.createElement("option");
      opt.value = m.id;
      opt.textContent = m.name;
      els.presetMapSelect.appendChild(opt);
    });
  }

  async function loadMapByName(name) {
    const response = await api(`/api/maps/${encodeURIComponent(name)}`);
    applySessionResponse(response);
    toast(t("mapLoaded", { name }), "success");
  }

  async function runPathfind() {
    setBusy(true);
    $("btnRun").disabled = true;
    try {
      const result = await api("/api/pathfind", {
        method: "POST",
        cancelPrevious: true,
        body: JSON.stringify({ algorithm: Number(els.algorithmSelect.value), grid: cloneGridData() }),
      }, 30000);
      state.lastResult = result;
      renderStats(result);
      setupAnimation(result);
      toast(t("pathfindDone"), "success");
    } catch (err) {
      if (!isCancelledError(err.message)) toast(err.message, "error");
    } finally {
      setBusy(false);
      $("btnRun").disabled = false;
    }
  }

  async function runCompareSelected() {
    const ids = getSelectedCompareIds();
    if (ids.length === 0) {
      toast(t("selectOneAlgo"), "error");
      return;
    }
    els.comparePanel.classList.add("d-none");
    setBusy(true);
    $("btnCompareRun").disabled = true;
    try {
      const data = await api("/api/compare", {
        method: "POST",
        cancelPrevious: true,
        body: JSON.stringify({ grid: cloneGridData(), algorithms: ids }),
      }, 60000);
      state.compareResults = data.results;
      els.statsPanel.classList.remove("empty-state");
      els.statsPanel.innerHTML = t("compareSummary", { count: data.results.length });
      renderCompareChart(data.results);
      renderComparePlayback(data.results);
      toast(t("compareDone", { count: data.results.length }), "success");
    } catch (err) {
      if (!isCancelledError(err.message)) toast(err.message, "error");
    } finally {
      setBusy(false);
      $("btnCompareRun").disabled = false;
    }
  }

  async function generateMap(type) {
    setBusy(true);
    try {
      if (type === "scatter") {
        const grid = buildScatteredGrid(0.12);
        if (!grid) throw new Error(t("scatterFail"));
        await syncSessionLoad(grid);
        toast(t("scatterDone"), "success");
        return;
      }

      const rows = clampSize(Number($("mapRows").value));
      const cols = clampSize(Number($("mapCols").value));
      $("mapRows").value = rows;
      $("mapCols").value = cols;
      const response = await api("/api/maze/random", {
        method: "POST",
        body: JSON.stringify({ rows, cols }),
      });
      applySessionResponse(response);
      toast(t("mazeDone"), "success");
    } catch (err) {
      toast(err.message, "error");
    } finally {
      setBusy(false);
    }
  }

  function clampSize(n) {
    return Math.max(MIN_SIZE, Math.min(MAX_SIZE, n));
  }

  async function applyNewSize(rows, cols, confirmMsg) {
    rows = clampSize(rows);
    cols = clampSize(cols);
    if (confirmMsg && !confirm(confirmMsg)) return;
    const cells = createEmptyCells(rows, cols);
    const start = { row: 1, col: 1 };
    const goal = { row: rows - 2, col: cols - 2 };
    cells[start.row][start.col] = CellType.Start;
    cells[goal.row][goal.col] = CellType.Goal;
    await syncSessionLoad({ rows, cols, cells, start, goal });
    toast(t("mapResized", { rows, cols }), "success");
  }

  async function clearMap() {
    if (!confirm(t("clearConfirm"))) return;
    const cells = createEmptyCells(state.rows, state.cols);
    const start = { row: 1, col: 1 };
    const goal = { row: state.rows - 2, col: state.cols - 2 };
    cells[start.row][start.col] = CellType.Start;
    cells[goal.row][goal.col] = CellType.Goal;
    await syncSessionLoad({ rows: state.rows, cols: state.cols, cells, start, goal });
    setStatsEmpty();
    toast(t("mapCleared"), "success");
  }

  async function undo() {
    if (!state.canUndo) return;
    try {
      applySessionResponse(await api("/api/session/undo", { method: "POST", body: "{}" }));
      resetAnimation();
    } catch (err) {
      toast(err.message, "error");
    }
  }

  async function redo() {
    if (!state.canRedo) return;
    try {
      applySessionResponse(await api("/api/session/redo", { method: "POST", body: "{}" }));
      resetAnimation();
    } catch (err) {
      toast(err.message, "error");
    }
  }

  async function saveMap() {
    const filename = $("saveFilename").value.trim();
    if (!filename.endsWith(".txt")) {
      toast(t("saveExtFail"), "error");
      return;
    }
    await api("/api/maps/save", { method: "POST", body: JSON.stringify({ filename, grid: cloneGridData() }) });
    toast(t("mapSaved", { filename }), "success");
  }

  function refreshUiLanguage() {
    I18n.applyStatic();
    updateStatusBar();
    updateAlgorithmDesc();
    if (state.algorithms.length) buildCompareList();
    if (els.statsPanel.classList.contains("empty-state")) setStatsEmpty();
    else if (state.compareResults && !state.lastResult) {
      els.statsPanel.innerHTML = t("compareSummary", { count: state.compareResults.length });
    } else if (state.lastResult) renderStats(state.lastResult);
    if (state.compareResults) {
      renderCompareChart(state.compareResults);
      renderComparePlayback(state.compareResults);
    }
    if (state.visitedOrder.length > 0) {
      setAnimStep(state.animStep);
    } else {
      els.animStepLabel.textContent = t("stepZero");
      els.animCurrentLabel.textContent = t("currentNodeDash");
    }
  }

  function bindEvents() {
    $("langSelect").addEventListener("change", (e) => {
      I18n.setLang(e.target.value);
      refreshUiLanguage();
    });

    document.querySelectorAll(".tool-btn").forEach((btn) => {
      btn.addEventListener("click", () => setTool(btn.dataset.tool));
    });
    document.querySelectorAll(".preset-size").forEach((btn) => {
      btn.addEventListener("click", () => {
        applyNewSize(Number(btn.dataset.rows), Number(btn.dataset.cols),
          t("resizeConfirm", { rows: btn.dataset.rows, cols: btn.dataset.cols }));
      });
    });

    $("btnRun").addEventListener("click", runPathfind);
    $("btnCompareToggle").addEventListener("click", () => {
      els.comparePanel.classList.toggle("d-none");
    });
    $("btnCompareRun").addEventListener("click", runCompareSelected);
    $("compareSelectAll").addEventListener("change", (e) => {
      els.compareAlgoList.querySelectorAll("input").forEach((cb) => {
        cb.checked = e.target.checked;
      });
    });
    document.addEventListener("click", (e) => {
      if (!e.target.closest(".compare-dropdown")) {
        els.comparePanel.classList.add("d-none");
      }
    });
    $("btnGenMaze").addEventListener("click", () => generateMap("maze").catch((e) => toast(e.message, "error")));
    $("btnGenScatter").addEventListener("click", () => generateMap("scatter").catch((e) => toast(e.message, "error")));
    $("btnClear").addEventListener("click", () => clearMap().catch((e) => toast(e.message, "error")));
    $("btnResize").addEventListener("click", () => {
      applyNewSize(Number($("mapRows").value), Number($("mapCols").value),
        t("resizeConfirm", { rows: $("mapRows").value, cols: $("mapCols").value }));
    });
    $("btnSaveMap").addEventListener("click", () => saveMap().catch((e) => toast(e.message, "error")));
    els.btnUndo.addEventListener("click", undo);
    els.btnRedo.addEventListener("click", redo);
    els.algorithmSelect.addEventListener("change", updateAlgorithmDesc);

    els.presetMapSelect.addEventListener("change", (e) => {
      if (!e.target.value) return;
      loadMapByName(e.target.value).catch((err) => toast(err.message, "error"));
    });

    $("btnPlay").addEventListener("click", playAnimation);
    $("btnPause").addEventListener("click", () => { stopAnimation(); $("btnPlay").disabled = false; $("btnPause").disabled = true; });
    $("btnResetAnim").addEventListener("click", () => setAnimStep(0));
    els.animSlider.addEventListener("input", (e) => { stopAnimation(); setAnimStep(Number(e.target.value)); $("btnPlay").disabled = false; $("btnPause").disabled = true; });

    window.addEventListener("resize", () => scheduleRenderGrid());

    document.addEventListener("keydown", (e) => {
      if (e.target.tagName === "INPUT" || e.target.tagName === "TEXTAREA") return;
      if (e.ctrlKey && e.key === "z") { e.preventDefault(); undo(); return; }
      if (e.ctrlKey && (e.key === "y" || (e.shiftKey && e.key === "Z"))) { e.preventDefault(); redo(); return; }
      if (e.key === "1") setTool("wall");
      if (e.key === "2") setTool("erase");
      if (e.key === "3") setTool("start");
      if (e.key === "4") setTool("goal");
      if (e.key === "Enter") runPathfind();
      if (e.key === "Escape") {
        stopAnimation();
        els.comparePanel.classList.add("d-none");
        $("btnPlay").disabled = false;
        $("btnPause").disabled = true;
      }
    });
  }

  async function bootstrap() {
    I18n.init();
    bindEvents();
    try {
      await loadAlgorithms();
      await loadPresetMaps();
    } catch (err) {
      toast(t("initFail", { msg: err.message }), "error");
    }
    try {
      await loadMapByName("sample_map.txt");
    } catch {
      const cells = createEmptyCells(15, 21);
      cells[1][1] = CellType.Start;
      cells[13][19] = CellType.Goal;
      await syncSessionLoad({ rows: 15, cols: 21, cells, start: { row: 1, col: 1 }, goal: { row: 13, col: 19 } });
    }
    updateStatusBar();
  }

  bootstrap();
})();
