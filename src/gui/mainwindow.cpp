#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
#ifndef BUILD_GUI_ONLY
    , tokenizer("/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/core/opus-mt-en-ru/vocab.json")
    , translatorManager_(new OnlineTranslatorsManager("/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/requests/api_keys.json"))
    , translator(nullptr)
#endif
{
    ui->setupUi(this);

    // Настройка интерфейса
    ui->inputTextEdit->setPlaceholderText("Введите текст для перевода...");
    ui->outputTextBrowser->setReadOnly(true);

    // Подключение сигналов
    connect(ui->translateButton, &QPushButton::clicked, this, &MainWindow::translateText);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::clearFields);

#ifndef BUILD_GUI_ONLY
    try {
        translator = new Translator(
            tokenizer,
            "/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/core/opus-mt-en-ru/encoder.onnx",
            "/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/core/opus-mt-en-ru/decoder.onnx",
            0,  // pad_token_id
            2   // eos_token_id
        );
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", "Failed to initialize translator: " + QString(e.what()));
        translator = nullptr;
    }
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
#ifndef BUILD_GUI_ONLY
    delete translator;
    delete translatorManager_;
#endif
}

void MainWindow::translateText()
{
    QString inputText = ui->inputTextEdit->toPlainText().trimmed();
    inputText = inputText.replace("\n", " ").simplified();
    QString sourceLang = ui->sourceLangCombo->currentText();
    QString targetLang = ui->targetLangCombo->currentText(); // Исправлено: sourceLangCombo -> targetLangCombo

    qDebug() << "Input text:" << inputText << "Source lang:" << sourceLang << "Target lang:" << targetLang;

    if (inputText.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter text to translate");
        return;
    }

    if (sourceLang == "Автоопределение") {
        sourceLang = detectLanguage(inputText);
        qDebug() << "Detected language:" << sourceLang;
        int index = ui->sourceLangCombo->findText(sourceLang);
        if (index != -1) {
            ui->sourceLangCombo->setCurrentIndex(index);
        } else {
            QMessageBox::warning(this, "Warning", "Не удалось определить язык");
            return;
        }
    }

    if (sourceLang == targetLang) {
        QMessageBox::warning(this, "Warning", "Исходный и целевой языки не должны совпадать");
        return;
    }

    ui->outputTextBrowser->clear();
    ui->variantsList->clear(); // Очистка списка вариантов

#ifdef BUILD_GUI_ONLY
    ui->outputTextBrowser->setPlainText("Translation functionality is disabled in GUI-only mode");
    ui->variantsList->addItem("Translation functionality is disabled in GUI-only mode");
    return;
#else
    // Локальный перевод
    try {
        delete translator;
        translator = nullptr;
        if (sourceLang == "Русский" && targetLang == "Английский") {
            translator = new Translator(
                tokenizer,
                "/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/core/opus-mt-ru-en/encoder.onnx",
                "/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/core/opus-mt-ru-en/decoder.onnx",
                62517, 0, 50, 3
            );
        } else if (sourceLang == "Английский" && targetLang == "Русский") {
            translator = new Translator(
                tokenizer,
                "/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/core/opus-mt-en-ru/encoder.onnx",
                "/Users/mihailmedvedev/Desktop/PROJ_4MODULE/last_git_proj/aip_project_translator/src/core/opus-mt-en-ru/decoder.onnx",
                62517, 0, 50, 3
            );
        }
        if (translator) {
            QString neuralTranslation = QString::fromStdString(translator->run(inputText.toStdString()));
            ui->outputTextBrowser->append("[Локальный] " + neuralTranslation);
            ui->variantsList->addItem("[Локальный] " + neuralTranslation); // Добавление в variantsList
        } else {
            QMessageBox::critical(this, "Error", "Переводчик не инициализирован");
        }
    } catch (const std::exception &e) {
        qDebug() << "Local translation error:" << e.what();
        QMessageBox::warning(this, "Warning", QString("Ошибка локального перевода: %1").arg(e.what()));
    }

    // Онлайн-переводчики
    try {
        QString sourceLangCode = sourceLang == "Английский" ? "en" : "ru";
        QString targetLangCode = targetLang == "Русский" ? "ru" : "en";
        std::vector<TranslationResult> translations = translatorManager_->GetTranslations(
            inputText.toStdString(), sourceLangCode.toStdString(), targetLangCode.toStdString()
        );

        for (const auto& result : translations) {
            if (result.translator_name == "Yandex.Cloud" || result.translator_name == "LibreTranslate" || result.translator_name == "DeepL") {
                QString translatorName = QString::fromStdString(
                    result.translator_name == "Yandex.Cloud" ? "Яндекс" :
                    result.translator_name == "LibreTranslate" ? "LibreTranslate" : "DeepL"
                );
                if (result.success) {
                    QString translatedText = QString::fromStdString(result.translated_text);
                    ui->outputTextBrowser->append("[" + translatorName + "] " + translatedText);
                    ui->variantsList->addItem("[" + translatorName + "] " + translatedText); // Добавление в variantsList
                } else {
                    QString errorMessage = QString::fromStdString(result.error_message);
                    qDebug() << translatorName << "translation error:" << errorMessage;
                    QMessageBox::warning(this, "Warning", QString("Ошибка перевода %1: %2").arg(translatorName, errorMessage));
                }
            }
        }
    } catch (const std::exception &e) {
        qDebug() << "Online translation error:" << e.what();
        QMessageBox::critical(this, "Error", QString("Ошибка онлайн-перевода: %1").arg(e.what()));
    }
#endif
}

void MainWindow::clearFields()
{
    ui->inputTextEdit->clear();
    ui->outputTextBrowser->clear();
    ui->variantsList->clear(); // Очистка списка вариантов
    ui->sourceLangCombo->setCurrentIndex(0);
}

QString MainWindow::detectLanguage(const QString &text)
{
    if (text.isEmpty()) {
        return "Неизвестный";
    }

    bool hasCyrillic = false;
    bool hasLatin = false;

    for (const QChar &c : text) {
        if (c.unicode() >= 0x0400 && c.unicode() <= 0x04FF) { // Кириллица
            hasCyrillic = true;
        } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) { // Латиница
            hasLatin = true;
        }
    }

    if (hasCyrillic && !hasLatin) {
        return "Русский";
    } else if (hasLatin && !hasCyrillic) {
        return "Английский";
    } else {
        return "Неизвестный";
    }
}

bool MainWindow::validateInput(const QString &text, const QString &language)
{
    return !text.isEmpty() && !language.isEmpty() && text.length() <= 500;
}