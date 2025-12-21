#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QColor>
#include <QDir>
#include <QMap>
#include <QStandardPaths>
#include <QString>

namespace DevPlanner {

struct TaskStatus {
  QString color;
  QString name;
};

inline const QMap<QString, TaskStatus> &getStatuses() {
  static const QMap<QString, TaskStatus> STATUSES = {
      {"none", {"#666666", "Без статуса"}},
      {"done", {"#00ff9d", "Готово"}},
      {"progress", {"#ffcc00", "В процессе"}},
      {"todo", {"#ff0055", "Не сделано"}},
      {"cancelled", {"#555555", "Отменено"}}};
  return STATUSES;
}

inline QString getDataDir() {
  QString homeDir =
      QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  return homeDir + "/.devchain_planner";
}

inline QString getProjectsFile() { return getDataDir() + "/projects.json"; }

inline QString getApiKeyFile() { return getDataDir() + "/api_key.txt"; }

inline QString getModelsFile() { return getDataDir() + "/models.json"; }

inline QString getContextDir() { return getDataDir() + "/contexts"; }

// App version
constexpr const char *APP_VERSION = "v2.0.0-cpp";

// --- Color Palette: Dark Purple / Neon (Space Cyberpunk) ---
namespace Colors {
inline QColor textMuted() { return QColor("#555555"); }
} // namespace Colors

} // namespace DevPlanner

#endif // CONFIG_HPP
