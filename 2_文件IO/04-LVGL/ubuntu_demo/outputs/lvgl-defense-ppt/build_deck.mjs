// Node-oriented editable pro deck builder.
// Run this after editing SLIDES, SOURCES, and layout functions.
// The init script installs a sibling node_modules/@oai/artifact-tool package link
// and package.json with type=module for shell-run eval builders. Run with the
// Node executable from Codex workspace dependencies or the platform-appropriate
// command emitted by the init script.
// Do not use pnpm exec from the repo root or any Node binary whose module
// lookup cannot resolve the builder's sibling node_modules/@oai/artifact-tool.

const fs = await import("node:fs/promises");
const path = await import("node:path");
const { Presentation, PresentationFile } = await import("@oai/artifact-tool");

const W = 1280;
const H = 720;

const DECK_ID = "lvgl-defense-ppt";
const OUT_DIR = "C:\\Users\\Zelda\\Desktop\\Rk3568_Code\\2_文件IO\\04-LVGL\\ubuntu_demo\\outputs\\lvgl-defense-ppt";
const REF_DIR = "C:\\Users\\Zelda\\Desktop\\Rk3568_Code\\2_文件IO\\04-LVGL\\ubuntu_demo\\outputs\\lvgl-defense-ppt\\references";
const SCRATCH_DIR = path.resolve(process.env.PPTX_SCRATCH_DIR || path.join("tmp", "slides", DECK_ID));
const PREVIEW_DIR = path.join(SCRATCH_DIR, "preview");
const VERIFICATION_DIR = path.join(SCRATCH_DIR, "verification");
const INSPECT_PATH = path.join(SCRATCH_DIR, "inspect.ndjson");
const MAX_RENDER_VERIFY_LOOPS = 3;

const INK = "#333333";
const GRAPHITE = "#666666";
const MUTED = "#8A8178";
const PAPER = "#F6F2EA";
const PAPER_96 = "#FFFCF7F2";
const WHITE = "#FFFFFF";
const ACCENT = "#AEEEEE";
const ACCENT_DARK = "#4A7C59";
const GOLD = "#E3D8BE";
const CORAL = "#F3D8D2";
const BORDER = "#E5DED4";
const ALERT = "#C0392B";
const TRANSPARENT = "#00000000";

const TITLE_FACE = "DengXian";
const BODY_FACE = "Microsoft YaHei";
const MONO_FACE = "Consolas";

const FALLBACK_PLATE_DATA_URL =
  "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/p9sAAAAASUVORK5CYII=";

const SOURCES = {
  appUi: "mycode/app_ui.c: login/menu creation, page jumps, rank screen refresh, lv_obj_delete_async cleanup.",
  userData: "mycode/user_data.c and mycode/user_data.h: registration/login, default admin, TXT persistence, best-score ranking.",
  medical: "code/medical.c and code/medical.h: queue data, add-patient form, IME, urgent insert, notice, idle timer, command parsing.",
  game2048: "mycode/2048.c: 4x4 matrix state, slide logic, score updates, best-score persistence.",
  album: "mycode/album.c: local BMP album, page wrap, auto-play timer, delete-time timer cleanup.",
  runtime: "main.c, lv_conf.h, CMakeLists.txt, mycode/ui_font.c: SDL runtime, fbdev hook, FreeType, IME, 2 MB LVGL memory pool, font loading.",
};

const TOTAL_SLIDES = 12;
const DECK_TITLE = "基于 LVGL 的多功能嵌入式交互系统设计与实现";
const DECK_SUBTITLE = "用户管理、医疗叫号、2048 与电子相册";
const OUTPUT_PPTX_PATH = path.join(OUT_DIR, "output.pptx");
const FINAL_PPTX_PATH = path.join(OUT_DIR, "LVGL_项目答辩汇报.pptx");
const FOOTER_NOTE = "截图、实物照片和代码截图由答辩前自行替换到占位框中";

const CODE_SNIPPETS = {
  urgent: [
    "int insert_idx = queue_count;",
    "bool is_urgent = lv_obj_has_state(add_form_urgent_sw, LV_STATE_CHECKED);",
    "",
    "if (is_urgent && queue_count > 0) {",
    "    insert_idx = 0;",
    "    for (int i = queue_count; i > 0; i--) {",
    "        wait_queue[i] = wait_queue[i - 1];",
    "    }",
    "}",
  ].join("\n"),
  game: [
    "rm_zero(temp_arr, &flag);",
    "hebing(temp_arr, &flag);",
    "rm_zero(temp_arr, &flag);",
    "",
    "if (flag)",
    "    rand_num();",
    "",
    "g_game.score += temp_arr[i];",
    "update_best_score();",
  ].join("\n"),
  cleanup: [
    "if (g_ui.medical_screen != NULL) {",
    "    lv_obj_delete_async(g_ui.medical_screen);",
    "    g_ui.medical_screen = NULL;",
    "}",
    "",
    "if (auto_play_timer != NULL) {",
    "    lv_timer_del(auto_play_timer);",
    "    auto_play_timer = NULL;",
    "}",
  ].join("\n"),
};

const inspectRecords = [];

async function pathExists(filePath) {
  try {
    await fs.access(filePath);
    return true;
  } catch {
    return false;
  }
}

async function readImageBlob(imagePath) {
  const bytes = await fs.readFile(imagePath);
  if (!bytes.byteLength) {
    throw new Error(`Image file is empty: ${imagePath}`);
  }
  return bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength);
}

async function normalizeImageConfig(config) {
  if (!config.path) {
    return config;
  }
  const { path: imagePath, ...rest } = config;
  return {
    ...rest,
    blob: await readImageBlob(imagePath),
  };
}

async function ensureDirs() {
  await fs.mkdir(OUT_DIR, { recursive: true });
  const obsoleteFinalArtifacts = [
    "preview",
    "verification",
    "inspect.ndjson",
    ["presentation", "proto.json"].join("_"),
    ["quality", "report.json"].join("_"),
  ];
  for (const obsolete of obsoleteFinalArtifacts) {
    await fs.rm(path.join(OUT_DIR, obsolete), { recursive: true, force: true });
  }
  await fs.mkdir(SCRATCH_DIR, { recursive: true });
  await fs.mkdir(PREVIEW_DIR, { recursive: true });
  await fs.mkdir(VERIFICATION_DIR, { recursive: true });
}

function lineConfig(fill = TRANSPARENT, width = 0) {
  return { style: "solid", fill, width };
}

function recordShape(slideNo, shape, role, shapeType, x, y, w, h) {
  if (!slideNo) return;
  inspectRecords.push({
    kind: "shape",
    slide: slideNo,
    id: shape?.id || `slide-${slideNo}-${role}-${inspectRecords.length + 1}`,
    role,
    shapeType,
    bbox: [x, y, w, h],
  });
}

function addShape(slide, geometry, x, y, w, h, fill = TRANSPARENT, line = TRANSPARENT, lineWidth = 0, meta = {}) {
  const shape = slide.shapes.add({
    geometry,
    position: { left: x, top: y, width: w, height: h },
    fill,
    line: lineConfig(line, lineWidth),
  });
  recordShape(meta.slideNo, shape, meta.role || geometry, geometry, x, y, w, h);
  return shape;
}

function normalizeText(text) {
  if (Array.isArray(text)) {
    return text.map((item) => String(item ?? "")).join("\n");
  }
  return String(text ?? "");
}

function textLineCount(text) {
  const value = normalizeText(text);
  if (!value.trim()) {
    return 0;
  }
  return Math.max(1, value.split(/\n/).length);
}

function requiredTextHeight(text, fontSize, lineHeight = 1.18, minHeight = 8) {
  const lines = textLineCount(text);
  if (lines === 0) {
    return minHeight;
  }
  return Math.max(minHeight, lines * fontSize * lineHeight);
}

function assertTextFits(text, boxHeight, fontSize, role = "text") {
  const required = requiredTextHeight(text, fontSize);
  const tolerance = Math.max(2, fontSize * 0.08);
  if (normalizeText(text).trim() && boxHeight + tolerance < required) {
    throw new Error(
      `${role} text box is too short: height=${boxHeight.toFixed(1)}, required>=${required.toFixed(1)}, ` +
        `lines=${textLineCount(text)}, fontSize=${fontSize}, text=${JSON.stringify(normalizeText(text).slice(0, 90))}`,
    );
  }
}

function wrapText(text, widthChars) {
  const words = normalizeText(text).split(/\s+/).filter(Boolean);
  const lines = [];
  let current = "";
  for (const word of words) {
    const next = current ? `${current} ${word}` : word;
    if (next.length > widthChars && current) {
      lines.push(current);
      current = word;
    } else {
      current = next;
    }
  }
  if (current) {
    lines.push(current);
  }
  return lines.join("\n");
}

function recordText(slideNo, shape, role, text, x, y, w, h) {
  const value = normalizeText(text);
  inspectRecords.push({
    kind: "textbox",
    slide: slideNo,
    id: shape?.id || `slide-${slideNo}-${role}-${inspectRecords.length + 1}`,
    role,
    text: value,
    textPreview: value.replace(/\n/g, " | ").slice(0, 180),
    textChars: value.length,
    textLines: textLineCount(value),
    bbox: [x, y, w, h],
  });
}

function recordImage(slideNo, image, role, imagePath, x, y, w, h) {
  inspectRecords.push({
    kind: "image",
    slide: slideNo,
    id: image?.id || `slide-${slideNo}-${role}-${inspectRecords.length + 1}`,
    role,
    path: imagePath,
    bbox: [x, y, w, h],
  });
}

function applyTextStyle(box, text, size, color, bold, face, align, valign, autoFit, listStyle) {
  box.text = text;
  box.text.fontSize = size;
  box.text.color = color;
  box.text.bold = Boolean(bold);
  box.text.alignment = align;
  box.text.verticalAlignment = valign;
  box.text.typeface = face;
  box.text.insets = { left: 0, right: 0, top: 0, bottom: 0 };
  if (autoFit) {
    box.text.autoFit = autoFit;
  }
  if (listStyle) {
    box.text.style = "list";
  }
}

function addText(
  slide,
  slideNo,
  text,
  x,
  y,
  w,
  h,
  {
    size = 22,
    color = INK,
    bold = false,
    face = BODY_FACE,
    align = "left",
    valign = "top",
    fill = TRANSPARENT,
    line = TRANSPARENT,
    lineWidth = 0,
    autoFit = null,
    listStyle = false,
    checkFit = true,
    role = "text",
  } = {},
) {
  if (!checkFit && textLineCount(text) > 1) {
    throw new Error("checkFit=false is only allowed for single-line headers, footers, and captions.");
  }
  if (checkFit) {
    assertTextFits(text, h, size, role);
  }
  const box = addShape(slide, "rect", x, y, w, h, fill, line, lineWidth);
  applyTextStyle(box, text, size, color, bold, face, align, valign, autoFit, listStyle);
  recordText(slideNo, box, role, text, x, y, w, h);
  return box;
}

async function addImage(slide, slideNo, config, position, role, sourcePath = null) {
  const image = slide.images.add(await normalizeImageConfig(config));
  image.position = position;
  recordImage(slideNo, image, role, sourcePath || config.path || config.uri || "inline-data-url", position.left, position.top, position.width, position.height);
  return image;
}

async function addPlate(slide, slideNo, opacityPanel = false) {
  slide.background.fill = PAPER;
  const platePath = path.join(REF_DIR, `slide-${String(slideNo).padStart(2, "0")}.png`);
  if (await pathExists(platePath)) {
    await addImage(
      slide,
      slideNo,
      { path: platePath, fit: "cover", alt: `Text-free art-direction plate for slide ${slideNo}` },
      { left: 0, top: 0, width: W, height: H },
      "art plate",
      platePath,
    );
  } else {
    await addImage(
      slide,
      slideNo,
      { dataUrl: FALLBACK_PLATE_DATA_URL, fit: "cover", alt: `Fallback blank art plate for slide ${slideNo}` },
      { left: 0, top: 0, width: W, height: H },
      "fallback art plate",
      "fallback-data-url",
    );
  }
  if (opacityPanel) {
    addShape(slide, "rect", 0, 0, W, H, "#FFFFFFB8", TRANSPARENT, 0, { slideNo, role: "plate readability overlay" });
  }
}

function addHeader(slide, slideNo, kicker, idx, total) {
  addText(slide, slideNo, String(kicker || "").toUpperCase(), 64, 34, 430, 24, {
    size: 13,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    checkFit: false,
    role: "header",
  });
  addText(slide, slideNo, `${String(idx).padStart(2, "0")} / ${String(total).padStart(2, "0")}`, 1114, 34, 104, 24, {
    size: 13,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    align: "right",
    checkFit: false,
    role: "header",
  });
  addShape(slide, "rect", 64, 64, 1152, 2, INK, TRANSPARENT, 0, { slideNo, role: "header rule" });
  addShape(slide, "ellipse", 57, 57, 16, 16, ACCENT, INK, 2, { slideNo, role: "header marker" });
}

function addTitleBlock(slide, slideNo, title, subtitle = null, x = 64, y = 86, w = 780, dark = false) {
  const titleColor = dark ? PAPER : INK;
  const bodyColor = dark ? PAPER : GRAPHITE;
  addText(slide, slideNo, title, x, y, w, 142, {
    size: 40,
    color: titleColor,
    bold: true,
    face: TITLE_FACE,
    role: "title",
  });
  if (subtitle) {
    addText(slide, slideNo, subtitle, x + 2, y + 148, Math.min(w, 720), 70, {
      size: 19,
      color: bodyColor,
      face: BODY_FACE,
      role: "subtitle",
    });
  }
}

function addIconBadge(slide, slideNo, x, y, accent = ACCENT, kind = "signal") {
  addShape(slide, "ellipse", x, y, 54, 54, PAPER_96, INK, 1.2, { slideNo, role: "icon badge" });
  if (kind === "flow") {
    addShape(slide, "ellipse", x + 13, y + 18, 10, 10, accent, INK, 1, { slideNo, role: "icon glyph" });
    addShape(slide, "ellipse", x + 31, y + 27, 10, 10, accent, INK, 1, { slideNo, role: "icon glyph" });
    addShape(slide, "rect", x + 22, y + 25, 19, 3, INK, TRANSPARENT, 0, { slideNo, role: "icon glyph" });
  } else if (kind === "layers") {
    addShape(slide, "roundRect", x + 13, y + 15, 26, 13, accent, INK, 1, { slideNo, role: "icon glyph" });
    addShape(slide, "roundRect", x + 18, y + 24, 26, 13, GOLD, INK, 1, { slideNo, role: "icon glyph" });
    addShape(slide, "roundRect", x + 23, y + 33, 20, 10, CORAL, INK, 1, { slideNo, role: "icon glyph" });
  } else {
    addShape(slide, "rect", x + 16, y + 29, 6, 12, accent, TRANSPARENT, 0, { slideNo, role: "icon glyph" });
    addShape(slide, "rect", x + 25, y + 21, 6, 20, accent, TRANSPARENT, 0, { slideNo, role: "icon glyph" });
    addShape(slide, "rect", x + 34, y + 14, 6, 27, accent, TRANSPARENT, 0, { slideNo, role: "icon glyph" });
  }
}

function addCard(slide, slideNo, x, y, w, h, label, body, { accent = ACCENT, fill = PAPER_96, line = INK, iconKind = "signal" } = {}) {
  if (h < 156) {
    throw new Error(`Card is too short for editable pro-deck copy: height=${h.toFixed(1)}, minimum=156.`);
  }
  addShape(slide, "roundRect", x, y, w, h, fill, line, 1.2, { slideNo, role: `card panel: ${label}` });
  addShape(slide, "rect", x, y, 8, h, accent, TRANSPARENT, 0, { slideNo, role: `card accent: ${label}` });
  addIconBadge(slide, slideNo, x + 22, y + 24, accent, iconKind);
  addText(slide, slideNo, label, x + 88, y + 22, w - 108, 28, {
    size: 15,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    role: "card label",
  });
  const wrapped = wrapText(body, Math.max(28, Math.floor(w / 13)));
  const bodyY = y + 86;
  const bodyH = h - (bodyY - y) - 22;
  if (bodyH < 54) {
    throw new Error(`Card body area is too short: height=${bodyH.toFixed(1)}, cardHeight=${h.toFixed(1)}, label=${JSON.stringify(label)}.`);
  }
  addText(slide, slideNo, wrapped, x + 24, bodyY, w - 48, bodyH, {
    size: 16,
    color: INK,
    face: BODY_FACE,
    role: `card body: ${label}`,
  });
}

function addMetricCard(slide, slideNo, x, y, w, h, metric, label, note = null, accent = ACCENT) {
  if (h < 132) {
    throw new Error(`Metric card is too short for editable pro-deck copy: height=${h.toFixed(1)}, minimum=132.`);
  }
  addShape(slide, "roundRect", x, y, w, h, PAPER_96, INK, 1.2, { slideNo, role: `metric panel: ${label}` });
  addShape(slide, "rect", x, y, w, 7, accent, TRANSPARENT, 0, { slideNo, role: `metric accent: ${label}` });
  addText(slide, slideNo, metric, x + 22, y + 24, w - 44, 54, {
    size: 34,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "metric value",
  });
  addText(slide, slideNo, label, x + 24, y + 82, w - 48, 38, {
    size: 16,
    color: GRAPHITE,
    face: BODY_FACE,
    role: "metric label",
  });
  if (note) {
    addText(slide, slideNo, note, x + 24, y + h - 42, w - 48, 22, {
      size: 10,
      color: MUTED,
      face: BODY_FACE,
      role: "metric note",
    });
  }
}

function addNotes(slide, body, sourceKeys) {
  const sourceLines = (sourceKeys || []).map((key) => `- ${SOURCES[key] || key}`).join("\n");
  slide.speakerNotes.setText(`${body || ""}\n\n[Sources]\n${sourceLines}`);
}

function addReferenceCaption(slide, slideNo) {
  addText(slide, slideNo, FOOTER_NOTE, 64, 676, 860, 18, {
    size: 10,
    color: MUTED,
    face: BODY_FACE,
    checkFit: false,
    role: "caption",
  });
}

function addPanel(slide, slideNo, x, y, w, h, role = "panel", fill = WHITE) {
  return addShape(slide, "roundRect", x, y, w, h, fill, BORDER, 1.2, { slideNo, role });
}

function addTag(slide, slideNo, text, x, y, w = 126, accent = ACCENT) {
  addShape(slide, "roundRect", x, y, w, 28, accent, TRANSPARENT, 0, { slideNo, role: `tag: ${text}` });
  addText(slide, slideNo, text, x + 10, y + 5, w - 20, 18, {
    size: 12,
    color: INK,
    bold: true,
    face: BODY_FACE,
    checkFit: false,
    role: "tag text",
  });
}

function addBulletLines(items) {
  return items.map((item) => `• ${item}`).join("\n");
}

function addBulletPanel(slide, slideNo, x, y, w, h, title, items, { accent = ACCENT, fontSize = 18 } = {}) {
  addPanel(slide, slideNo, x, y, w, h, `bullet panel: ${title}`);
  addShape(slide, "rect", x, y, w, 8, accent, TRANSPARENT, 0, { slideNo, role: `panel accent: ${title}` });
  addText(slide, slideNo, title, x + 22, y + 20, w - 44, 30, {
    size: 22,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "panel title",
  });
  addText(slide, slideNo, addBulletLines(items), x + 22, y + 66, w - 44, h - 88, {
    size: fontSize,
    color: INK,
    face: BODY_FACE,
    role: `panel body: ${title}`,
  });
}

function addKeyValueCard(slide, slideNo, x, y, w, h, key, value, accent = ACCENT) {
  addShape(slide, "roundRect", x, y, w, h, PAPER_96, BORDER, 1, { slideNo, role: `key card: ${key}` });
  addShape(slide, "rect", x, y, 8, h, accent, TRANSPARENT, 0, { slideNo, role: `key accent: ${key}` });
  addText(slide, slideNo, key, x + 22, y + 18, w - 40, 22, {
    size: 13,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    role: "key card label",
  });
  addText(slide, slideNo, value, x + 22, y + 48, w - 40, h - 64, {
    size: 17,
    color: INK,
    face: BODY_FACE,
    role: "key card body",
  });
}

function addPlaceholder(slide, slideNo, x, y, w, h, title, tip) {
  addShape(slide, "roundRect", x, y, w, h, "#FCFAF6", ACCENT_DARK, 1.5, { slideNo, role: `placeholder: ${title}` });
  addShape(slide, "rect", x + 20, y + 20, w - 40, 2, ACCENT_DARK, TRANSPARENT, 0, { slideNo, role: "placeholder rule" });
  addShape(slide, "rect", x + 20, y + h - 22, w - 40, 2, ACCENT_DARK, TRANSPARENT, 0, { slideNo, role: "placeholder rule" });
  addShape(slide, "roundRect", x + 24, y + 24, 112, 28, ACCENT, TRANSPARENT, 0, { slideNo, role: "placeholder badge" });
  addText(slide, slideNo, "图片占位", x + 36, y + 30, 86, 18, {
    size: 12,
    color: INK,
    bold: true,
    face: BODY_FACE,
    checkFit: false,
    role: "placeholder label",
  });
  addText(slide, slideNo, title, x + 30, y + Math.max(86, h / 2 - 26), w - 60, 36, {
    size: 24,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    align: "center",
    role: "placeholder title",
  });
  addText(slide, slideNo, tip, x + 36, y + Math.max(130, h / 2 + 12), w - 72, 52, {
    size: 15,
    color: GRAPHITE,
    face: BODY_FACE,
    align: "center",
    role: "placeholder tip",
  });
}

function addCodeBlock(slide, slideNo, x, y, w, h, title, code, note = null) {
  addShape(slide, "roundRect", x, y, w, h, "#F7F4ED", BORDER, 1.1, { slideNo, role: `code block: ${title}` });
  addShape(slide, "rect", x, y, w, 34, "#ECE6DC", TRANSPARENT, 0, { slideNo, role: "code header" });
  addText(slide, slideNo, title, x + 16, y + 8, w - 32, 18, {
    size: 13,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    checkFit: false,
    role: "code title",
  });
  addText(slide, slideNo, code, x + 18, y + 48, w - 36, h - (note ? 88 : 64), {
    size: 14,
    color: INK,
    face: MONO_FACE,
    role: "code text",
  });
  if (note) {
    addText(slide, slideNo, note, x + 18, y + h - 28, w - 36, 18, {
      size: 10,
      color: MUTED,
      face: BODY_FACE,
      checkFit: false,
      role: "code note",
    });
  }
}

function addStepCard(slide, slideNo, x, y, w, h, index, title, body, accent = ACCENT) {
  addShape(slide, "roundRect", x, y, w, h, WHITE, BORDER, 1.1, { slideNo, role: `step card: ${title}` });
  addShape(slide, "ellipse", x + 16, y + 14, 30, 30, accent, TRANSPARENT, 0, { slideNo, role: "step badge" });
  addText(slide, slideNo, String(index), x + 26, y + 21, 12, 16, {
    size: 13,
    color: INK,
    bold: true,
    face: MONO_FACE,
    checkFit: false,
    role: "step number",
  });
  addText(slide, slideNo, title, x + 56, y + 16, w - 72, 22, {
    size: 18,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "step title",
  });
  addText(slide, slideNo, body, x + 18, y + 47, w - 36, h - 59, {
    size: 14,
    color: GRAPHITE,
    face: BODY_FACE,
    role: "step body",
  });
}

function addFlowBar(slide, slideNo, x, y, w, h = 8) {
  addShape(slide, "roundRect", x, y, w, h, ACCENT, TRANSPARENT, 0, { slideNo, role: "flow bar" });
}

async function slide01Cover(presentation) {
  const slideNo = 1;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo);
  addShape(slide, "rect", 0, 0, W, H, "#FFFFFFD6", TRANSPARENT, 0, { slideNo, role: "cover overlay" });
  addPanel(slide, slideNo, 58, 54, 1164, 612, "cover frame", "#FFFEFC");
  addShape(slide, "rect", 58, 54, 18, 612, ACCENT, TRANSPARENT, 0, { slideNo, role: "cover accent stripe" });

  addText(slide, slideNo, "嵌入式图形界面课程答辩", 112, 96, 320, 24, {
    size: 15,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    checkFit: false,
    role: "cover kicker",
  });
  addText(slide, slideNo, DECK_TITLE, 112, 140, 640, 116, {
    size: 42,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "cover title",
  });
  addText(slide, slideNo, DECK_SUBTITLE, 114, 274, 520, 34, {
    size: 22,
    color: GRAPHITE,
    face: BODY_FACE,
    role: "cover subtitle",
  });
  addText(slide, slideNo, "围绕 ubuntu_demo 工程，按真实代码现状梳理项目目标、模块实现、核心算法、工程配置与后续扩展。", 114, 332, 620, 62, {
    size: 18,
    color: INK,
    face: BODY_FACE,
    role: "cover intro",
  });

  addTag(slide, slideNo, "用户系统", 114, 418, 110);
  addTag(slide, slideNo, "医疗叫号", 236, 418, 110);
  addTag(slide, slideNo, "2048", 358, 418, 94);
  addTag(slide, slideNo, "电子相册", 464, 418, 118);
  addTag(slide, slideNo, "工程适配", 594, 418, 110);

  addPanel(slide, slideNo, 832, 112, 316, 274, "cover info");
  addText(slide, slideNo, "答辩信息", 858, 136, 120, 24, {
    size: 21,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "cover info title",
  });
  addText(slide, slideNo, "汇报人：姓名占位\n学号：学号占位\n课程：课程名称占位\n日期：答辩日期占位", 858, 180, 256, 118, {
    size: 18,
    color: INK,
    face: BODY_FACE,
    role: "cover info body",
  });
  addText(slide, slideNo, "封面字段已留空，方便答辩前直接修改。", 858, 316, 250, 24, {
    size: 13,
    color: MUTED,
    face: BODY_FACE,
    role: "cover info note",
  });

  addPanel(slide, slideNo, 832, 420, 316, 144, "cover summary");
  addText(slide, slideNo, "本次汇报重点", 858, 444, 140, 24, {
    size: 21,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "cover summary title",
  });
  addText(slide, slideNo, "功能覆盖尽量完整，图片后续由项目截图和开发板照片替换。", 858, 482, 248, 50, {
    size: 17,
    color: INK,
    face: BODY_FACE,
    role: "cover summary body",
  });

  addText(slide, slideNo, "01 / 12", 1088, 632, 86, 18, {
    size: 12,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    align: "right",
    checkFit: false,
    role: "cover page",
  });
  addText(slide, slideNo, "答辩用图示、照片与代码截图位置均已预留", 112, 626, 320, 18, {
    size: 11,
    color: MUTED,
    face: BODY_FACE,
    checkFit: false,
    role: "cover footer",
  });
  addNotes(slide, "封面页说明项目名称、模块范围和答辩信息占位字段。", ["appUi", "medical", "game2048", "album"]);
}

async function slide02Background(presentation) {
  const slideNo = 2;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "项目目标", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "项目背景与目标", "把 LVGL、文件 I/O 与多页面交互组合成一个完整课程项目。");

  addBulletPanel(slide, slideNo, 70, 218, 640, 404, "为什么做成综合项目", [
    "综合使用 LVGL、文件 I/O 和多页面 UI。",
    "用一个工程串起登录、业务、娱乐和数据持久化。",
    "在 Ubuntu 仿真环境里先验证界面与逻辑。",
    "为后续 RK3568 适配保留运行入口和资源准备。"
  ]);

  addPlaceholder(slide, slideNo, 750, 218, 460, 282, "功能模块结构图占位", "建议后续放项目功能框图或菜单层级图。");
  addBulletPanel(slide, slideNo, 750, 504, 460, 118, "本次汇报主线", [
    "项目目标 → 系统结构 → 功能模块 → 核心算法 → 工程配置与问题解决。"
  ], { fontSize: 16 });

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "说明课程项目目标和为何把多个模块收敛到同一套 LVGL 工程里。", ["appUi", "userData", "medical", "game2048", "album", "runtime"]);
}

async function slide03Architecture(presentation) {
  const slideNo = 3;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "系统结构", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "系统整体结构与页面调度", "页面关系围绕登录入口、主菜单分发和子页面回退展开。");

  addPanel(slide, slideNo, 70, 212, 700, 430, "flow zone");
  addText(slide, slideNo, "页面流转关系", 94, 234, 180, 24, {
    size: 22,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "flow title",
  });
  addStepCard(slide, slideNo, 108, 286, 160, 92, 1, "登录页", "输入用户名和密码。\n注册、登录都从这里进入。");
  addFlowBar(slide, slideNo, 268, 328, 48);
  addStepCard(slide, slideNo, 316, 286, 170, 92, 2, "主菜单", "统一进入 2048、电子相册、医疗叫号和排行榜。");
  addFlowBar(slide, slideNo, 396, 378, 8, 28);
  addStepCard(slide, slideNo, 92, 432, 148, 94, 3, "2048", "滑动交互、分数与最高分更新。");
  addStepCard(slide, slideNo, 258, 432, 148, 94, 4, "电子相册", "本地 BMP 浏览和自动轮播。");
  addStepCard(slide, slideNo, 424, 432, 148, 94, 5, "医疗叫号", "队列、医生信息、添加病人和公告。");
  addStepCard(slide, slideNo, 590, 432, 148, 94, 6, "排行榜", "读取用户最高分并排序展示。");

  addPanel(slide, slideNo, 806, 212, 404, 430, "right zone");
  addPlaceholder(slide, slideNo, 830, 236, 356, 204, "主菜单或流程图占位", "建议后续放主菜单界面截图。");
  addBulletPanel(slide, slideNo, 830, 466, 356, 146, "页面调度亮点", [
    "主页面跳转使用 lv_screen_load_anim。",
    "退出登录前先切页，再异步销毁子页面。",
    "排行榜页面支持按需创建和刷新。"
  ], { fontSize: 16 });

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "说明页面关系、主菜单分发逻辑，以及 app_ui.c 中的页面创建与回退方式。", ["appUi"]);
}

async function slide04UserSystem(presentation) {
  const slideNo = 4;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "用户模块", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "用户管理与排行榜", "注册、登录、TXT 持久化和最高分排行构成项目的基础数据层。");

  addBulletPanel(slide, slideNo, 70, 218, 500, 404, "实现要点", [
    "user_register 和 user_login 负责账号校验。",
    "系统启动时会兜底创建默认 admin 账号。",
    "用户数据写入 user_data.txt，断电后可恢复。",
    "2048 破纪录后会同步刷新当前用户最高分。",
    "排行榜按 best_score 降序排序展示。"
  ]);
  addKeyValueCard(slide, slideNo, 92, 500, 210, 92, "默认账号", "admin / 123456");
  addKeyValueCard(slide, slideNo, 316, 500, 230, 92, "存储形式", "username,password,best_score");

  addPanel(slide, slideNo, 600, 218, 610, 404, "screens zone");
  addPlaceholder(slide, slideNo, 626, 244, 558, 166, "登录界面截图位", "建议放账号输入、密码输入和软键盘截图。");
  addPlaceholder(slide, slideNo, 626, 432, 558, 166, "排行榜界面截图位", "建议放当前用户和排行列表截图。");

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "说明用户层的注册登录、TXT 持久化和排行榜来源。", ["userData", "appUi", "game2048"]);
}

async function slide05MedicalOverview(presentation) {
  const slideNo = 5;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "医疗模块 01", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "医疗叫号系统概览", "医疗模块不只是界面展示，还包含排队状态、医生信息和底部操作控制。");

  addPlaceholder(slide, slideNo, 70, 218, 720, 404, "医疗主界面截图位", "建议放当前叫号大卡、等候队列、医生信息和底部按钮的完整页面。");

  addPanel(slide, slideNo, 820, 218, 390, 404, "medical facts");
  addText(slide, slideNo, "核心数据点", 844, 240, 140, 24, {
    size: 22,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "medical fact title",
  });
  addKeyValueCard(slide, slideNo, 844, 280, 164, 108, "current_patient", "当前叫号病人信息");
  addKeyValueCard(slide, slideNo, 1026, 280, 164, 108, "wait_queue[10]", "固定长度等待队列");
  addKeyValueCard(slide, slideNo, 844, 404, 164, 108, "doctors[2]", "按诊室切换医生资料");
  addKeyValueCard(slide, slideNo, 1026, 404, 164, 108, "next_number", "新病人号码自增分配");
  addText(slide, slideNo, "底部操作", 844, 536, 120, 20, {
    size: 16,
    color: ACCENT_DARK,
    bold: true,
    face: MONO_FACE,
    checkFit: false,
    role: "medical action title",
  });
  addTag(slide, slideNo, "模拟叫号", 844, 566, 96);
  addTag(slide, slideNo, "紧急公告", 954, 566, 96, GOLD);
  addTag(slide, slideNo, "添加病人", 1064, 566, 96, CORAL);

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "展示医疗主界面的结构组成，并强调 current_patient、wait_queue、doctors、next_number 四个关键状态。", ["medical"]);
}

async function slide06MedicalInteraction(presentation) {
  const slideNo = 6;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "医疗模块 02", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "医疗叫号交互细节", "添加病人弹窗把输入、遮罩、防误触和拼音输入整合到一套交互流程中。");

  addPlaceholder(slide, slideNo, 70, 218, 620, 404, "添加病人表单 + 拼音候选区占位", "建议放半透明遮罩、表单面板、软键盘和拼音候选区截图。");
  addBulletPanel(slide, slideNo, 720, 218, 490, 404, "交互亮点", [
    "创建黑色半透明遮罩，避免底层按钮误触。",
    "姓名输入框、诊室下拉框和急诊开关集中在一张弹窗里。",
    "软键盘默认隐藏，点击输入框后再弹出。",
    "接入 lv_ime_pinyin，实现中文拼音输入。",
    "公告弹窗和 30 秒闲置计时器共同维护使用体验。"
  ]);
  addTag(slide, slideNo, "诊室选择：1 / 2", 744, 560, 132);
  addTag(slide, slideNo, "急诊开关：支持插队", 888, 560, 148, GOLD);
  addTag(slide, slideNo, "IME：拼音候选", 1048, 560, 138, CORAL);

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "说明弹窗交互、防误触、软键盘与拼音输入法的接入。", ["medical", "runtime"]);
}

async function slide07MedicalLogic(presentation) {
  const slideNo = 7;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "医疗模块 03", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "医疗叫号核心逻辑", "这一页重点讲急诊插队、命令解析以及叫号后的联动刷新。");

  addPanel(slide, slideNo, 70, 218, 530, 404, "medical flow");
  addText(slide, slideNo, "逻辑流程", 94, 240, 140, 24, {
    size: 22,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "logic title",
  });
  addStepCard(slide, slideNo, 94, 282, 482, 92, 1, "添加病人", "读取姓名、诊室和急诊开关，决定插入位置。");
  addFlowBar(slide, slideNo, 332, 382, 8, 24);
  addStepCard(slide, slideNo, 94, 410, 482, 92, 2, "队列更新", "普通病人追加到队尾；急诊病人触发数组整体右移。");
  addFlowBar(slide, slideNo, 332, 510, 8, 24);
  addStepCard(slide, slideNo, 94, 538, 482, 82, 3, "叫号联动", "刷新当前叫号、队列列表、医生信息，再触发闪烁和终端 TTS。", ALERT);

  addCodeBlock(slide, slideNo, 628, 218, 582, 238, "急诊插队核心摘录", CODE_SNIPPETS.urgent, "来自 code/medical.c 的 on_add_form_ok 逻辑。");
  addCard(slide, slideNo, 628, 456, 278, 166, "命令式叫号", "medical_parse_cmd 支持解析 @call,12,张三,1诊室 形式的字符串，并同步清理原队列位置。", { accent: ACCENT, iconKind: "flow" });
  addCard(slide, slideNo, 932, 456, 278, 166, "声光提示", "叫号完成后会执行 start_blink() 与 tts_speak()，把界面高亮和终端播报串起来。", { accent: ALERT, iconKind: "signal" });

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "突出医疗模块的数组移位插队、命令解析、闪烁和 TTS 联动。", ["medical"]);
}

async function slide08GameOverview(presentation) {
  const slideNo = 8;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "2048 模块 01", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "2048 模块界面与玩法", "2048 页面承接了触摸滑动、分数显示、胜负弹窗和返回重置等交互。");

  addPlaceholder(slide, slideNo, 70, 218, 620, 404, "2048 主界面截图位", "建议放棋盘、分数卡片、重置按钮和胜负弹窗截图。");
  addBulletPanel(slide, slideNo, 720, 218, 490, 404, "玩法与交互", [
    "使用 4x4 棋盘矩阵保存当前状态。",
    "手指滑动会映射到上、下、左、右四个方向。",
    "界面同时显示当前分数和历史最高分。",
    "支持返回菜单和重新开始两类按钮操作。",
    "当出现 2048 或无路可走时弹出提示框。"
  ]);
  addKeyValueCard(slide, slideNo, 744, 514, 138, 86, "棋盘", "4x4", ACCENT);
  addKeyValueCard(slide, slideNo, 896, 514, 138, 86, "方向", "4 个", GOLD);
  addKeyValueCard(slide, slideNo, 1048, 514, 138, 86, "联动", "BEST", CORAL);

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "先展示 2048 的可见界面和基本玩法，再在下一页展开算法实现。", ["game2048", "userData"]);
}

async function slide09GameLogic(presentation) {
  const slideNo = 9;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "2048 模块 02", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "2048 算法与成绩联动", "核心思想是把每次滑动统一成去零、合并、二次去零，再决定是否生成新数字。");

  addPanel(slide, slideNo, 70, 218, 520, 404, "2048 flow");
  addText(slide, slideNo, "矩阵处理步骤", 94, 240, 170, 24, {
    size: 22,
    color: INK,
    bold: true,
    face: TITLE_FACE,
    role: "2048 flow title",
  });
  addStepCard(slide, slideNo, 94, 282, 472, 82, 1, "去零", "把当前行或列中的非 0 元素先压紧。");
  addFlowBar(slide, slideNo, 320, 372, 8, 20);
  addStepCard(slide, slideNo, 94, 402, 472, 82, 2, "合并", "相邻且相等的数字相加，同时累计当前分数。");
  addFlowBar(slide, slideNo, 320, 492, 8, 20);
  addStepCard(slide, slideNo, 94, 518, 472, 82, 3, "再去零并生成新数", "如果盘面发生变化，就调用 rand_num 生成新的 2 或 4。", GOLD);

  addCodeBlock(slide, slideNo, 620, 218, 590, 240, "合并与刷新摘录", CODE_SNIPPETS.game, "来自 mycode/2048.c 的矩阵更新逻辑。");
  addBulletPanel(slide, slideNo, 620, 462, 590, 160, "成绩联动", [
    "四个方向滑动通过行列重映射复用同一套处理思路。",
    "update_best_score 会在分数突破时回写用户最高分。",
    "排行榜页面读取的就是这里持久化后的 best_score。"
  ], { fontSize: 17 });

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "说明 2048 的矩阵算法和与 user_data.c 的最高分联动。", ["game2048", "userData"]);
}

async function slide10Album(presentation) {
  const slideNo = 10;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "相册模块", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "电子相册模块", "相册页面围绕本地 BMP 图片浏览、自动播放和退出时的资源回收展开。");

  addPlaceholder(slide, slideNo, 70, 218, 620, 404, "电子相册界面截图位", "建议放大图区域、上一张/下一张按钮和自动播放按钮截图。");
  addBulletPanel(slide, slideNo, 720, 218, 490, 270, "实现要点", [
    "固定读取 build/pic 下的 a.bmp、b.bmp、c.bmp。",
    "上一张和下一张都支持循环切换。",
    "底部会同步显示当前页码信息。",
    "点击自动播放后，每 3000ms 自动切一张。"
  ]);
  addKeyValueCard(slide, slideNo, 744, 512, 140, 88, "图片数量", "3 张 BMP");
  addKeyValueCard(slide, slideNo, 896, 512, 140, 88, "自动播放", "3000ms");
  addKeyValueCard(slide, slideNo, 1048, 512, 138, 88, "退出回收", "lv_timer_del");

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "介绍相册的本地图片路径、手动翻页和自动播放定时器清理。", ["album"]);
}

async function slide11Engineering(presentation) {
  const slideNo = 11;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "工程事实", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "工程配置、适配准备与问题解决", "这一页只讲当前仓库中真实存在的运行方式、适配入口和问题处理办法。");

  addBulletPanel(slide, slideNo, 70, 218, 520, 404, "工程现状", [
    "当前仓库启用 SDL 仿真，lv_conf.h 中 LV_USE_SDL = 1。",
    "main.c 仍保留 framebuffer /dev/fb0 与触摸 event6 的入口。",
    "FreeType、拼音输入法和 2 MB LVGL 内存池都已开启。",
    "CMakeLists.txt 里能看到 SDL 和交叉编译相关配置痕迹。",
    "资源路径仍含 Ubuntu 共享目录硬编码，因此 RK3568 只写适配准备或部分验证。"
  ]);
  addCodeBlock(slide, slideNo, 620, 218, 590, 214, "页面销毁与定时器清理摘录", CODE_SNIPPETS.cleanup);
  addCard(slide, slideNo, 620, 430, 282, 192, "问题 1：切屏崩溃", "退出登录时如果在动画过程中直接删旧页面，容易触发段错误。当前做法是先切回登录页，再用 lv_obj_delete_async 异步回收子页面。", { accent: ACCENT, iconKind: "flow" });
  addCard(slide, slideNo, 928, 430, 282, 192, "问题 2：内存与定时器", "字库和 IME 会增加内存压力，因此配置了 2 MB LVGL 内存池；相册和医疗模块在页面销毁时都会主动删定时器，避免野指针。", { accent: ALERT, iconKind: "layers" });

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "这一页如实说明 SDL、fbdev 入口、FreeType/IME、路径硬编码，以及用 async delete 和定时器清理规避问题。", ["runtime", "appUi", "album", "medical"]);
}

async function slide12Summary(presentation) {
  const slideNo = 12;
  const slide = presentation.slides.add();
  await addPlate(slide, slideNo, true);
  addHeader(slide, slideNo, "总结展望", slideNo, TOTAL_SLIDES);
  addTitleBlock(slide, slideNo, "总结与展望", "项目价值体现在：功能完整、实现真实、工程细节可讲、后续扩展方向清晰。");

  addCard(slide, slideNo, 86, 228, 250, 166, "页面组织", "从登录页、主菜单到多个子页面，已经形成完整的多页面 LVGL 工程结构。", { accent: ACCENT, iconKind: "flow" });
  addCard(slide, slideNo, 358, 228, 250, 166, "数据持久化", "用户信息和 2048 最高分写入 TXT 文件，体现了文件 I/O 在项目中的实际落地。", { accent: GOLD, iconKind: "layers" });
  addCard(slide, slideNo, 630, 228, 250, 166, "算法实现", "医疗队列的插队和 2048 的矩阵处理，让项目不仅有界面，也有底层数据逻辑。", { accent: CORAL, iconKind: "signal" });
  addCard(slide, slideNo, 902, 228, 250, 166, "工程思考", "SDL、字库、内存池、定时器回收和板端入口准备，体现了工程层面的完整性。", { accent: ACCENT, iconKind: "layers" });
  addBulletPanel(slide, slideNo, 86, 432, 1066, 168, "后续扩展方向", [
    "把资源路径抽象成相对路径或统一配置，减少平台切换成本。",
    "补齐真实板端资源部署与实物运行验证，进一步完善 RK3568 适配。",
    "把命令式叫号继续扩展成真实 TTS 或网络消息输入。"
  ], { fontSize: 18 });

  addReferenceCaption(slide, slideNo);
  addNotes(slide, "收束整个答辩：总结本项目体现的页面组织、文件 I/O、算法和工程思考，再给出后续扩展方向。", ["appUi", "userData", "medical", "game2048", "album", "runtime"]);
}

async function createDeck() {
  await ensureDirs();
  const presentation = Presentation.create({ slideSize: { width: W, height: H } });
  await slide01Cover(presentation);
  await slide02Background(presentation);
  await slide03Architecture(presentation);
  await slide04UserSystem(presentation);
  await slide05MedicalOverview(presentation);
  await slide06MedicalInteraction(presentation);
  await slide07MedicalLogic(presentation);
  await slide08GameOverview(presentation);
  await slide09GameLogic(presentation);
  await slide10Album(presentation);
  await slide11Engineering(presentation);
  await slide12Summary(presentation);
  return presentation;
}

async function saveBlobToFile(blob, filePath) {
  const bytes = new Uint8Array(await blob.arrayBuffer());
  await fs.writeFile(filePath, bytes);
}

async function writeInspectArtifact(presentation) {
  inspectRecords.unshift({
    kind: "deck",
    id: DECK_ID,
    slideCount: presentation.slides.count,
    slideSize: { width: W, height: H },
  });
  presentation.slides.items.forEach((slide, index) => {
    inspectRecords.splice(index + 1, 0, {
      kind: "slide",
      slide: index + 1,
      id: slide?.id || `slide-${index + 1}`,
    });
  });
  const lines = inspectRecords.map((record) => JSON.stringify(record)).join("\n") + "\n";
  await fs.writeFile(INSPECT_PATH, lines, "utf8");
}

async function currentRenderLoopCount() {
  const logPath = path.join(VERIFICATION_DIR, "render_verify_loops.ndjson");
  if (!(await pathExists(logPath))) return 0;
  const previous = await fs.readFile(logPath, "utf8");
  return previous.split(/\r?\n/).filter((line) => line.trim()).length;
}

async function nextRenderLoopNumber() {
  return (await currentRenderLoopCount()) + 1;
}

async function appendRenderVerifyLoop(presentation, previewPaths, pptxPath) {
  const logPath = path.join(VERIFICATION_DIR, "render_verify_loops.ndjson");
  const priorCount = await currentRenderLoopCount();
  const record = {
    kind: "render_verify_loop",
    deckId: DECK_ID,
    loop: priorCount + 1,
    maxLoops: MAX_RENDER_VERIFY_LOOPS,
    capReached: priorCount + 1 >= MAX_RENDER_VERIFY_LOOPS,
    timestamp: new Date().toISOString(),
    slideCount: presentation.slides.count,
    previewCount: previewPaths.length,
    previewDir: PREVIEW_DIR,
    inspectPath: INSPECT_PATH,
    pptxPath,
  };
  await fs.appendFile(logPath, JSON.stringify(record) + "\n", "utf8");
  return record;
}

async function verifyAndExport(presentation) {
  await ensureDirs();
  const nextLoop = await nextRenderLoopNumber();
  if (nextLoop > MAX_RENDER_VERIFY_LOOPS) {
    throw new Error(
      `Render/verify/fix loop cap reached: ${MAX_RENDER_VERIFY_LOOPS} total renders are allowed. ` +
        "Do not rerender; note any remaining visual issues in the final response.",
    );
  }
  await writeInspectArtifact(presentation);
  const previewPaths = [];
  for (let idx = 0; idx < presentation.slides.items.length; idx += 1) {
    const slide = presentation.slides.items[idx];
    const preview = await presentation.export({ slide, format: "png", scale: 1 });
    const previewPath = path.join(PREVIEW_DIR, `slide-${String(idx + 1).padStart(2, "0")}.png`);
    await saveBlobToFile(preview, previewPath);
    previewPaths.push(previewPath);
  }
  const pptxBlob = await PresentationFile.exportPptx(presentation);
  const pptxPath = OUTPUT_PPTX_PATH;
  await pptxBlob.save(pptxPath);
  await fs.copyFile(pptxPath, FINAL_PPTX_PATH);
  const loopRecord = await appendRenderVerifyLoop(presentation, previewPaths, pptxPath);
  return { pptxPath, finalPptxPath: FINAL_PPTX_PATH, loopRecord };
}

const presentation = await createDeck();
const result = await verifyAndExport(presentation);
console.log(result.pptxPath);
