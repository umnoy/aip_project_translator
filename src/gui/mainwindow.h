#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QDebug>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QPushButton>

#ifndef BUILD_GUI_ONLY
#include "translator.hpp"
#include "online_translators.hpp"
#include "tokenizer.hpp"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Главное окно приложения переводчика
 * 
 * Класс MainWindow представляет собой основное окно приложения,
 * содержащее все элементы интерфейса для перевода текста.
 * Поддерживает как локальный перевод через ONNX Runtime,
 * так и онлайн-перевод через различные сервисы.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT 

public:
    /**
     * @brief Конструктор главного окна
     * @param parent Родительский виджет
     */
    MainWindow(QWidget *parent = nullptr); 
    
    /**
     * @brief Деструктор
     */
    ~MainWindow();

    /**
     * @brief Определяет язык введенного текста
     * @param text Текст для определения языка
     * @return Строка с названием языка ("Русский", "Английский" или "Неизвестный")
     */
    QString detectLanguage(const QString &text);
    
    /**
     * @brief Проверяет корректность входных данных
     * @param text Текст для проверки
     * @param language Язык для проверки
     * @return true если данные корректны, false в противном случае
     */
    bool validateInput(const QString &text, const QString &language);

private slots:
    /**
     * @brief Слот для перевода текста
     * 
     * Обрабатывает нажатие кнопки "Перевести".
     * Выполняет перевод текста с использованием выбранных переводчиков.
     */
    void translateText();
    
    /**
     * @brief Слот для очистки полей ввода/вывода
     * 
     * Очищает все поля ввода и вывода, сбрасывает выбор языков.
     */
    void clearFields();

private:
    Ui::MainWindow *ui;
#ifndef BUILD_GUI_ONLY
    Tokenizer tokenizer; 
    Translator* translator; 
    OnlineTranslatorsManager* translatorManager_; 
#endif
};

#endif // MAINWINDOW_H