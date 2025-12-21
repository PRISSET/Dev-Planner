#include "storage.hpp"
#include "config.hpp"
#include <QDir>
#include <QFile>
#include <QTextStream>

namespace DevPlanner {

void Storage::ensureDataDir() {
  QDir dir(getDataDir());
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  QDir contextDir(getContextDir());
  if (!contextDir.exists()) {
    contextDir.mkpath(".");
  }
}

QJsonObject Storage::loadAllProjects() {
  ensureDataDir();
  QFile file(getProjectsFile());
  if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
      return doc.object();
    }
  }
  return QJsonObject();
}

void Storage::saveAllProjects(const QJsonObject &projects) {
  ensureDataDir();
  QFile file(getProjectsFile());
  if (file.open(QIODevice::WriteOnly)) {
    QJsonDocument doc(projects);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  }
}

QString Storage::loadApiKey() {
  QFile file(getApiKeyFile());
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    QString key = in.readAll().trimmed();
    file.close();
    return key;
  }
  return QString();
}

void Storage::saveApiKey(const QString &apiKey) {
  ensureDataDir();
  QFile file(getApiKeyFile());
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out << apiKey;
    file.close();
  }
}

QStringList Storage::loadModels(QString &selectedModel) {
  QStringList defaultModels = {"openai/gpt-4o-mini", "openai/gpt-4o",
                               "anthropic/claude-3.5-sonnet",
                               "google/gemini-pro-1.5"};
  selectedModel = "openai/gpt-4o-mini";

  QFile file(getModelsFile());
  if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
      QJsonObject obj = doc.object();
      if (obj.contains("selected")) {
        selectedModel = obj["selected"].toString();
      }
      if (obj.contains("models")) {
        QStringList models;
        for (const auto &m : obj["models"].toArray()) {
          models.append(m.toString());
        }
        if (!models.isEmpty()) {
          return models;
        }
      }
    }
  }
  return defaultModels;
}

void Storage::saveModels(const QStringList &models, const QString &selected) {
  ensureDataDir();
  QFile file(getModelsFile());
  if (file.open(QIODevice::WriteOnly)) {
    QJsonObject obj;
    obj["selected"] = selected;
    QJsonArray arr;
    for (const auto &m : models) {
      arr.append(m);
    }
    obj["models"] = arr;
    QJsonDocument doc(obj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  }
}

QJsonArray Storage::loadContext(const QString &projectName) {
  ensureDataDir();
  QString path = getContextDir() + "/" + projectName + ".json";
  QFile file(path);
  if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
      return doc.array();
    }
  }
  return QJsonArray();
}

void Storage::saveContext(const QString &projectName,
                          const QJsonArray &messages) {
  ensureDataDir();
  QString path = getContextDir() + "/" + projectName + ".json";
  QFile file(path);
  if (file.open(QIODevice::WriteOnly)) {
    // Keep only last 100 messages
    QJsonArray toSave = messages;
    while (toSave.size() > 100) {
      toSave.removeFirst();
    }
    QJsonDocument doc(toSave);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  }
}

} // namespace DevPlanner
