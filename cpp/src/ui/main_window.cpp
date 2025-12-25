#include "main_window.hpp"
#include "ai_chat_panel.hpp"
#include "core/config.hpp"
#include "core/storage.hpp"
#include "glassmorphism_widget.hpp"
#include "live_background.hpp"
#include "modern_button.hpp"
#include "node_canvas.hpp"
#include "task_node.hpp"
#include <QCloseEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>
#include <cmath>
#include <random>

namespace DevPlanner {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Dev Planner");
  setMinimumSize(800, 500);
  resize(1400, 900);
  loadProjects();
  m_autosaveTimer = new QTimer(this);
  m_autosaveTimer->setSingleShot(true);
  connect(m_autosaveTimer, &QTimer::timeout, this, &MainWindow::autosave);
  setupUI();
  refreshProjectList();
  if (m_projectList->count() > 0) {
    m_projectList->setCurrentRow(0);
    onProjectSelected(m_projectList->item(0));
  }
}

MainWindow::~MainWindow() = default;

void MainWindow::loadProjects() { m_projects = Storage::loadAllProjects(); }

void MainWindow::setupUI() {
  auto *central = new QWidget(this);
  setCentralWidget(central);
  m_liveBg = new LiveBackground(central);
  auto *layout = new QVBoxLayout(central);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  setupTitleBar(layout);
  setupToolbar(layout);
  setupContent(layout);
  setupVersionLabel();
}

void MainWindow::setupTitleBar(QVBoxLayout *layout) {
  auto *bar = new QWidget(this);
  bar->setFixedHeight(40);
  bar->setAttribute(Qt::WA_TranslucentBackground);
  bar->setStyleSheet("background: transparent;");
  auto *l = new QHBoxLayout(bar);
  l->setContentsMargins(20, 0, 20, 0);
  auto *title = new QLabel("DEV PLANNER", this);
  title->setAttribute(Qt::WA_TranslucentBackground);
  title->setStyleSheet("background: transparent; color: #ffffff; font-weight: "
                       "900; letter-spacing: 5px; font-size: 14px;");
  l->addWidget(title);
  l->addStretch();
  layout->addWidget(bar);
}

void MainWindow::setupToolbar(QVBoxLayout *layout) {
  auto *bar = new QWidget(this);
  bar->setFixedHeight(60);
  bar->setStyleSheet("background: transparent;");
  auto *l = new QHBoxLayout(bar);
  l->setContentsMargins(20, 0, 20, 0);
  l->setSpacing(10);

  auto *zOut = new ModernButton("−", QColor(255, 255, 255, 20), this);
  zOut->setFixedSize(32, 32);
  connect(zOut, &QPushButton::clicked, this, [this]() { m_canvas->zoomOut(); });
  m_zoomLabelBtn = new ModernButton("100%", QColor(255, 255, 255, 10), this);
  m_zoomLabelBtn->setFixedSize(60, 32);
  connect(m_zoomLabelBtn, &QPushButton::clicked, this,
          [this]() { m_canvas->zoomReset(); });
  auto *zIn = new ModernButton("+", QColor(255, 255, 255, 20), this);
  zIn->setFixedSize(32, 32);
  connect(zIn, &QPushButton::clicked, this, [this]() { m_canvas->zoomIn(); });

  l->addWidget(zOut);
  l->addWidget(m_zoomLabelBtn);
  l->addWidget(zIn);
  l->addSpacing(20);

  auto *exp = new ModernButton("EXPORT", QColor(217, 0, 255, 100), this);
  connect(exp, &QPushButton::clicked, this, &MainWindow::exportTZ);
  auto *clr = new ModernButton("CLEAR", QColor(255, 0, 85, 100), this);
  connect(clr, &QPushButton::clicked, this, &MainWindow::clearAll);
  m_noteModeBtn = new ModernButton("NOTE", QColor(0, 200, 150, 100), this);
  m_noteModeBtn->setCheckable(true);
  connect(m_noteModeBtn, &QPushButton::toggled, this, [this](bool checked) {
    m_noteMode = checked;
    m_noteModeBtn->setText(checked ? "NOTE ✓" : "NOTE");
    if (m_canvas)
      m_canvas->setNoteMode(checked);
  });
  l->addWidget(exp);
  l->addWidget(clr);
  l->addWidget(m_noteModeBtn);
  l->addStretch();

  for (auto it = getStatuses().begin(); it != getStatuses().end(); ++it) {
    auto *cnt = new QLabel("0", this);
    cnt->setAttribute(Qt::WA_TranslucentBackground);
    cnt->setAttribute(Qt::WA_NoSystemBackground);
    cnt->setStyleSheet(
        QString("background: transparent; background-color: transparent; "
                "border: none; color: %1; "
                "font-weight: 900; font-size: 14px; padding: 0 4px;")
            .arg(it.value().color));
    l->addWidget(cnt);
    m_statsLabels[it.key()] = cnt;
  }
  layout->addWidget(bar);
}

void MainWindow::setupContent(QVBoxLayout *layout) {
  auto *s = new QSplitter(Qt::Horizontal, this);
  s->setStyleSheet("QSplitter::handle { background: rgba(255,255,255,0.05); }");
  setupProjectsPanel(s);
  m_canvas = new NodeCanvas(this);
  connect(m_canvas, &NodeCanvas::changed, this, &MainWindow::scheduleAutosave);
  connect(m_canvas, &NodeCanvas::changed, this, [this]() {
    QString ctx;
    int idx = 1;
    for (auto *n : m_canvas->nodes()) {
      ctx +=
          QString("%1. %2 [%3]\n").arg(idx++).arg(n->title()).arg(n->status());
    }
    m_aiChat->setTasksInfo(ctx);
    m_aiChat->setTaskCounter(m_canvas->nodes().size());
  });
  connect(m_canvas, &NodeCanvas::zoomChanged, this,
          &MainWindow::updateZoomLabel);
  s->addWidget(m_canvas);
  setupAIPanel(s);
  s->setSizes({220, 800, 380});
  layout->addWidget(s, 1);
}

void MainWindow::setupProjectsPanel(QSplitter *s) {
  m_projectsPanel = new GlassmorphismWidget(this);
  auto *l = new QVBoxLayout(m_projectsPanel);
  l->setContentsMargins(15, 20, 15, 15);
  auto *lbl = new QLabel("PROJECTS", this);
  lbl->setStyleSheet("color: rgba(255,255,255,0.4); font-weight: 900; "
                     "letter-spacing: 2px; font-size: 10px;");
  l->addWidget(lbl);
  auto *btn = new ModernButton("+ NEW", QColor(255, 255, 255, 15), this);
  connect(btn, &QPushButton::clicked, this, &MainWindow::onNewProject);
  l->addWidget(btn);
  m_projectList = new QListWidget(this);
  m_projectList->setStyleSheet(
      "QListWidget { background: transparent; border: none; outline: none; } "
      "QListWidget::item { background: rgba(255,255,255,0.03); color: #ffffff; "
      "padding: 12px; border-radius: 10px; margin-bottom: 5px; } "
      "QListWidget::item:selected { background: rgba(217,0,255,0.2); border: "
      "1px solid rgba(217,0,255,0.4); }");
  connect(m_projectList, &QListWidget::itemClicked, this,
          &MainWindow::onProjectSelected);
  connect(m_projectList, &QListWidget::customContextMenuRequested, this,
          &MainWindow::onProjectContextMenu);
  m_projectList->setContextMenuPolicy(Qt::CustomContextMenu);
  l->addWidget(m_projectList, 1);
  s->addWidget(m_projectsPanel);
}

void MainWindow::setupAIPanel(QSplitter *s) {
  m_aiPanel = new GlassmorphismWidget(this);
  auto *l = new QVBoxLayout(m_aiPanel);
  l->setContentsMargins(0, 0, 0, 0);
  m_aiChat = new AIChatPanel(this);
  connect(m_aiChat, &AIChatPanel::taskCreated, this,
          &MainWindow::onAITaskCreated);
  connect(m_aiChat, &AIChatPanel::tasksConnect, this,
          &MainWindow::onAITasksConnect);
  connect(m_aiChat, &AIChatPanel::taskUpdateStatus, this,
          &MainWindow::onAITaskUpdateStatus);
  connect(m_aiChat, &AIChatPanel::taskUpdateDesc, this,
          &MainWindow::onAITaskUpdateDesc);
  connect(m_aiChat, &AIChatPanel::taskRename, this,
          &MainWindow::onAITaskRename);
  connect(m_aiChat, &AIChatPanel::taskDelete, this,
          &MainWindow::onAITaskDelete);
  connect(m_aiChat, &AIChatPanel::tasksDeleteMany, this,
          &MainWindow::onAITasksDeleteMany);
  connect(m_aiChat, &AIChatPanel::clearAllTasks, this,
          &MainWindow::onAIClearAll);
  connect(m_aiChat, &AIChatPanel::requestTasks, this,
          &MainWindow::onAIRequestTasks);
  connect(m_aiChat, &AIChatPanel::arrangeTasks, this,
          &MainWindow::onAIArrangeTasks);
  connect(m_aiChat, &AIChatPanel::disconnectTasks, this,
          &MainWindow::onAIDisconnectTasks);
  l->addWidget(m_aiChat);
  s->addWidget(m_aiPanel);
}

void MainWindow::setupVersionLabel() {
  m_versionLabel = new QLabel(APP_VERSION, centralWidget());
  m_versionLabel->setStyleSheet(
      "color: rgba(255,255,255,0.1); font-size: 10px;");
  updateVersionPosition();
}

void MainWindow::resizeEvent(QResizeEvent *e) {
  QMainWindow::resizeEvent(e);
  if (m_liveBg)
    m_liveBg->setGeometry(centralWidget()->rect());
  updateVersionPosition();
}
void MainWindow::updateVersionPosition() {
  if (m_versionLabel) {
    m_versionLabel->adjustSize();
    m_versionLabel->move(15, centralWidget()->height() - 25);
  }
}

void MainWindow::refreshProjectList() {
  m_projectList->clear();
  for (auto it = m_projects.begin(); it != m_projects.end(); ++it) {
    auto *i = new QListWidgetItem(it.key());
    i->setData(Qt::UserRole, it.key());
    m_projectList->addItem(i);
  }
}

void MainWindow::onNewProject() {
  bool ok;
  QString n =
      QInputDialog::getText(this, "New", "Name:", QLineEdit::Normal, "", &ok);
  if (ok && !n.isEmpty()) {
    if (m_projects.contains(n))
      return;
    QJsonObject p;
    p["nodes"] = QJsonArray();
    p["connections"] = QJsonArray();
    p["scale"] = 1.0;
    p["offset_x"] = 0;
    p["offset_y"] = 0;
    m_projects[n] = p;
    Storage::saveAllProjects(m_projects);
    refreshProjectList();
  }
}

void MainWindow::onProjectSelected(QListWidgetItem *i) {
  if (!m_currentProject.isEmpty())
    saveCurrentProject();
  QString n = i->data(Qt::UserRole).toString();
  m_currentProject = n;
  if (m_projects.contains(n))
    m_canvas->loadProjectData(m_projects[n].toObject());
  m_aiChat->setProject(n);
  m_aiChat->setTaskCounter(m_canvas->nodes().size());
  updateStats();
}

void MainWindow::saveCurrentProject() {
  if (!m_currentProject.isEmpty()) {
    m_projects[m_currentProject] = m_canvas->getProjectData();
    Storage::saveAllProjects(m_projects);
  }
}

void MainWindow::onProjectContextMenu(const QPoint &p) {
  auto *i = m_projectList->itemAt(p);
  if (!i)
    return;
  QMenu m(this);
  m.setStyleSheet(
      "QMenu { background: rgba(30,30,40,250); border: 1px solid "
      "rgba(255,255,255,0.1); border-radius: 10px; color: #ffffff; padding: "
      "5px; } QMenu::item:selected { background: rgba(217,0,255,0.3); }");
  connect(m.addAction("Rename"), &QAction::triggered, this,
          [this, i]() { renameProject(i); });
  connect(m.addAction("Delete"), &QAction::triggered, this,
          [this, i]() { deleteProject(i); });
  m.exec(m_projectList->mapToGlobal(p));
}

void MainWindow::renameProject(QListWidgetItem *i) {
  QString old = i->data(Qt::UserRole).toString();
  bool ok;
  QString n = QInputDialog::getText(this, "Rename", "Name:", QLineEdit::Normal,
                                    old, &ok);
  if (ok && !n.isEmpty() && n != old) {
    if (m_projects.contains(n))
      return;
    m_projects[n] = m_projects[old];
    m_projects.remove(old);
    if (m_currentProject == old)
      m_currentProject = n;
    Storage::saveAllProjects(m_projects);
    refreshProjectList();
  }
}

void MainWindow::deleteProject(QListWidgetItem *i) {
  QString n = i->data(Qt::UserRole).toString();
  if (QMessageBox::Yes ==
      QMessageBox::question(this, "Delete", "Delete project?")) {
    m_projects.remove(n);
    if (m_currentProject == n) {
      m_currentProject.clear();
      m_canvas->clearAll();
    }
    Storage::saveAllProjects(m_projects);
    refreshProjectList();
  }
}

void MainWindow::scheduleAutosave() { m_autosaveTimer->start(1000); }
void MainWindow::autosave() {
  saveCurrentProject();
  updateStats();
}
void MainWindow::updateStats() {
  auto s = m_canvas->getStats();
  for (auto it = s.begin(); it != s.end(); ++it)
    if (m_statsLabels.contains(it.key()))
      m_statsLabels[it.key()]->setText(QString::number(it.value()));
}
void MainWindow::updateZoomLabel(int p) {
  m_zoomLabelBtn->setText(QString("%1%").arg(p));
}

void MainWindow::exportTZ() {
  if (m_canvas->nodes().isEmpty())
    return;
  QString t = "# TZ: " + m_currentProject + "\n\n";
  int idx = 1;
  for (auto *n : m_canvas->nodes())
    t += QString("## %1. %2\n%3\n\n")
             .arg(idx++)
             .arg(n->title())
             .arg(n->description());
  QString path =
      QFileDialog::getSaveFileName(this, "Save", "", "Markdown (*.md)");
  if (!path.isEmpty()) {
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
      QTextStream(&f) << t;
      f.close();
    }
  }
}

void MainWindow::clearAll() {
  if (!m_currentProject.isEmpty() &&
      QMessageBox::Yes ==
          QMessageBox::question(this, "Clear", "Clear all tasks?")) {
    m_canvas->clearAll();
    saveCurrentProject();
    updateStats();
  }
}
void MainWindow::closeEvent(QCloseEvent *e) {
  saveCurrentProject();
  e->accept();
}

void MainWindow::onAITaskCreated(const QString &t, const QString &d,
                                 const QString &s, int x, int y) {
  auto *n = m_canvas->addNode(x, y);
  n->setTitle(t);
  n->setDescription(d);
  if (getStatuses().contains(s))
    n->setStatus(s);
  updateStats();
}
void MainWindow::onAITasksConnect(int f, int t) {
  auto n = m_canvas->nodes();
  if (f >= 0 && f < n.size() && t >= 0 && t < n.size())
    m_canvas->addConnection(n[f], n[t]);
}
void MainWindow::onAITaskUpdateStatus(int idx, const QString &s) {
  auto n = m_canvas->nodes();
  if (idx >= 0 && idx < n.size()) {
    n[idx]->setStatus(s);
    updateStats();
  }
}
void MainWindow::onAITaskUpdateDesc(int idx, const QString &d) {
  auto n = m_canvas->nodes();
  if (idx >= 0 && idx < n.size())
    n[idx]->setDescription(d);
}
void MainWindow::onAITaskRename(int idx, const QString &t) {
  auto n = m_canvas->nodes();
  if (idx >= 0 && idx < n.size())
    n[idx]->setTitle(t);
}
void MainWindow::onAITaskDelete(int idx) {
  auto n = m_canvas->nodes();
  if (idx >= 0 && idx < n.size()) {
    m_canvas->removeNode(n[idx]);
    updateStats();
  }
}
void MainWindow::onAITasksDeleteMany(const QList<int> &ids) {
  auto s = ids;
  std::sort(s.begin(), s.end(), std::greater<int>());
  for (int i : s)
    if (i >= 0 && i < m_canvas->nodes().size())
      m_canvas->removeNode(m_canvas->nodes()[i]);
  updateStats();
}
void MainWindow::onAIClearAll() {
  m_canvas->clearAll();
  updateStats();
}
void MainWindow::onAIRequestTasks() {
  QString i;
  int idx = 1;
  for (auto *n : m_canvas->nodes())
    i += QString("%1. %2\n").arg(idx++).arg(n->title());
  m_aiChat->setTasksInfo(i);
}
void MainWindow::onAIArrangeTasks(const QString &t) {
  m_canvas->update();
} // Placeholder
void MainWindow::onAIDisconnectTasks(int f, int t) {
  auto n = m_canvas->nodes();
  if (f >= 0 && f < n.size() && t >= 0 && t < n.size())
    m_canvas->removeConnection(n[f], n[t]);
}

} // namespace DevPlanner
