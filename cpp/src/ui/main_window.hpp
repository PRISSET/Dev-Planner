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
class AIChatPanel;
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
  void exportTZ();
  void clearAll();

  // AI Chat slots
  void onAITaskCreated(const QString &title, const QString &description,
                       const QString &status, int x, int y);
  void onAITasksConnect(int fromIdx, int toIdx);
  void onAITaskUpdateStatus(int taskIdx, const QString &status);
  void onAITaskUpdateDesc(int taskIdx, const QString &description);
  void onAITaskRename(int taskIdx, const QString &title);
  void onAITaskDelete(int taskIdx);
  void onAITasksDeleteMany(const QList<int> &tasks);
  void onAIClearAll();
  void onAIRequestTasks();
  void onAIArrangeTasks(const QString &type);
  void onAIDisconnectTasks(int fromIdx, int toIdx);

private:
  void setupUI();
  void setupTitleBar(QVBoxLayout *mainLayout);
  void setupToolbar(QVBoxLayout *mainLayout);
  void setupContent(QVBoxLayout *mainLayout);
  void setupProjectsPanel(QSplitter *splitter);
  void setupAIPanel(QSplitter *splitter);
  void setupVersionLabel();
  void updateVersionPosition();
  void refreshProjectList();
  void saveCurrentProject();
  void loadProjects();

  // UI Elements
  LiveBackground *m_liveBg = nullptr;
  QListWidget *m_projectList = nullptr;
  NodeCanvas *m_canvas = nullptr;
  AIChatPanel *m_aiChat = nullptr;
  ModernButton *m_zoomLabelBtn = nullptr;
  QMap<QString, QLabel *> m_statsLabels;
  QLabel *m_versionLabel = nullptr;
  GlassmorphismWidget *m_projectsPanel = nullptr;
  GlassmorphismWidget *m_aiPanel = nullptr;

  QJsonObject m_projects;
  QString m_currentProject;
  QTimer *m_autosaveTimer = nullptr;
};

} // namespace DevPlanner

#endif // MAIN_WINDOW_HPP
