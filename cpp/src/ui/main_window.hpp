#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMap>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

namespace DevPlanner {

class NodeCanvas;
class GlassmorphismWidget;
class ModernButton;
class LiveBackground;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

protected:
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void onNewProject();
  void onProjectSelected(QListWidgetItem *item);
  void onProjectContextMenu(const QPoint &pos);
  void renameProject(QListWidgetItem *item);
  void deleteProject(QListWidgetItem *item);
  void scheduleAutosave();
  void autosave();
  void updateStats();
  void updateZoomLabel(int percent);
  void clearAll();

private:
  void setupUI();
  void setupTitleBar(QVBoxLayout *mainLayout);
  void setupToolbar(QVBoxLayout *mainLayout);
  void setupContent(QVBoxLayout *mainLayout);
  void setupProjectsPanel(QSplitter *splitter);
  void setupVersionLabel();
  void updateVersionPosition();
  void refreshProjectList();
  void saveCurrentProject();
  void loadProjects();

  LiveBackground *m_liveBg = nullptr;
  QListWidget *m_projectList = nullptr;
  NodeCanvas *m_canvas = nullptr;
  ModernButton *m_zoomLabelBtn = nullptr;
  ModernButton *m_noteModeBtn = nullptr;
  QMap<QString, QLabel *> m_statsLabels;
  QLabel *m_versionLabel = nullptr;
  GlassmorphismWidget *m_projectsPanel = nullptr;

  QJsonObject m_projects;
  QString m_currentProject;
  QTimer *m_autosaveTimer = nullptr;
  bool m_noteMode = false;
};

} // namespace DevPlanner

#endif
