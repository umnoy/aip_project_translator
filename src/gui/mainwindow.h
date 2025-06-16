#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QStringList>
#include "../core/src/tokenizer/tokenizer.hpp"
#include "../core/src/translator/traslator.hpp"
#include "../requests/include/online_translators.hpp"

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
    void toggleHistoryPanel();

private:
    Ui::MainWindow *ui;
    Translator *translator;
    OnlineTranslatorsManager *translatorManager_;
    QPropertyAnimation *panelAnimation;
    bool isPanelVisible;
    QStringList translationHistory;
    Tokenizer tokenizer;

    void resizeEvent(QResizeEvent *event) override;
    void updatePanelPosition();
    QString detectLanguage(const QString &text);
    bool validateInput(const QString &text, const QString &language);
};

#endif // MAINWINDOW_H
