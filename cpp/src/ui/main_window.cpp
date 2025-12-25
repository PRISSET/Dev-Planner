#include "main_window.hpp"
#include "core/config.hpp"
#include "core/storage.hpp"
#include "glassmorphism_widget.hpp"
#include "live_background.hpp"
#include "modern_button.hpp"
#include "node_canvas.hpp"
#include "task_node.hpp"
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>

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
  connect(m_canvas, &NodeCanvas::zoomChanged, this,
          &MainWindow::updateZoomLabel);
  s->addWidget(m_canvas);
  s->setSizes({220, 1180});
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
  if (m_versionLabel)
    m_versionLabel->move(10, centralWidget()->height() - 20);
}

void MainWindow::refreshProjectList() {
  m_projectList->clear();
  for (auto it = m_projects.begin(); it != m_projects.end(); ++it) {
    auto *i = new QListWidgetItem(it.key());
    i->setData(Qt::UserRole, it.key());
    m_projectList->addItem(i);
  }
}

void MainWindow::saveCurrentProject() {
  if (m_currentProject.isEmpty())
    return;
  QJsonObject d;
  d["canvas"] = m_canvas->getProjectData();
  m_projects[m_currentProject] = d;
  Storage::saveAllProjects(m_projects);
}

void MainWindow::onNewProject() {
  bool ok;
  QString n =
      QInputDialog::getText(this, "New", "Name:", QLineEdit::Normal, "", &ok);
  if (ok && !n.isEmpty() && !m_projects.contains(n)) {
    saveCurrentProject();
    m_projects[n] = QJsonObject();
    Storage::saveAllProjects(m_projects);
    refreshProjectList();
    for (int i = 0; i < m_projectList->count(); ++i) {
      if (m_projectList->item(i)->data(Qt::UserRole).toString() == n) {
        m_projectList->setCurrentRow(i);
        onProjectSelected(m_projectList->item(i));
        break;
      }
    }
  }
}

void MainWindow::onProjectSelected(QListWidgetItem *i) {
  if (!i)
    return;
  saveCurrentProject();
  m_currentProject = i->data(Qt::UserRole).toString();
  if (m_projects.contains(m_currentProject)) {
    QJsonObject d = m_projects[m_currentProject].toObject();
    m_canvas->loadProjectData(d["canvas"].toObject());
  }
  updateStats();
}

void MainWindow::onProjectContextMenu(const QPoint &p) {
  auto *i = m_projectList->itemAt(p);
  if (!i)
    return;
  QMenu m;
  m.setStyleSheet("QMenu { background: #1a1a1e; color: #ffffff; border: 1px "
                  "solid rgba(255,255,255,0.1); } "
                  "QMenu::item:selected { background: rgba(217,0,255,0.3); }");
  auto *r = m.addAction("Rename");
  auto *d = m.addAction("Delete");
  auto *a = m.exec(m_projectList->mapToGlobal(p));
  if (a == r)
    renameProject(i);
  else if (a == d)
    deleteProject(i);
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

} // namespace DevPlanner
