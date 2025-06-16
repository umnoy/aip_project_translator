#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QDebug>

#ifndef BUILD_GUI_ONLY
#include "translator.hpp"
#include "online_translators.hpp"
#include "tokenizer.hpp"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void translateText();
    void clearFields();

private:
    Ui::MainWindow *ui;
#ifndef BUILD_GUI_ONLY
    Tokenizer tokenizer;
    Translator* translator;
    OnlineTranslatorsManager* translatorManager_;
#endif

    QString detectLanguage(const QString &text);
    bool validateInput(const QString &text, const QString &language);
};

#endif // MAINWINDOW_H
