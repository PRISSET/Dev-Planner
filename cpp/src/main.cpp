#include "ui/main_window.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QApplication::setApplicationName("Dev Planner");
  QApplication::setApplicationVersion("2.1.0");
  QApplication::setOrganizationName("DevPlanner");
  QApplication::setOrganizationDomain("dev-planner.local");

  app.setStyle("Fusion");

  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(26, 26, 30));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(20, 20, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(30, 30, 35));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(35, 35, 40));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(0, 255, 255));
  darkPalette.setColor(QPalette::Highlight, QColor(0, 255, 255));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);
  app.setPalette(darkPalette);

  DevPlanner::MainWindow window;
  window.show();

  return app.exec();
}
