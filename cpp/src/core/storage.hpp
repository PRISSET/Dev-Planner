#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace DevPlanner {

class Storage {
public:
  static void ensureDataDir();

  // Projects
  static QJsonObject loadAllProjects();
  static void saveAllProjects(const QJsonObject &projects);

  // API Key
  static QString loadApiKey();
  static void saveApiKey(const QString &apiKey);

  // Models
  static QStringList loadModels(QString &selectedModel);
  static void saveModels(const QStringList &models, const QString &selected);

  // Chat contexts
  static QJsonArray loadContext(const QString &projectName);
  static void saveContext(const QString &projectName,
                          const QJsonArray &messages);
};

} // namespace DevPlanner

#endif // STORAGE_HPP
