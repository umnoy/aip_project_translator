#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "mainwindow.h"
#include <QApplication>
#include <QtTest/QTest>
#include <QSignalSpy>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QListWidget>
#include <QMessageBox>
#include <QTimer>

TEST_SUITE("MainWindow Tests") {
    TEST_CASE("Initialization") {
        int argc = 1;
        char *argv[] = {(char*)"test"};
        QApplication app(argc, argv);
        
        MainWindow window;
        
        SUBCASE("Language comboboxes initialization") {
            REQUIRE(window.findChild<QComboBox*>("sourceLangCombo") != nullptr);
            REQUIRE(window.findChild<QComboBox*>("targetLangCombo") != nullptr);
            
            auto sourceCombo = window.findChild<QComboBox*>("sourceLangCombo");
            auto targetCombo = window.findChild<QComboBox*>("targetLangCombo");
            
            CHECK(sourceCombo->count() == 3);
            CHECK(targetCombo->count() == 2);
            
            CHECK(sourceCombo->itemText(0) == "Автоопределение");
            CHECK(sourceCombo->itemText(1) == "Русский");
            CHECK(sourceCombo->itemText(2) == "Английский");
            
            CHECK(targetCombo->itemText(0) == "Русский");
            CHECK(targetCombo->itemText(1) == "Английский");
        }
        
        SUBCASE("Text input/output fields initialization") {
            REQUIRE(window.findChild<QPlainTextEdit*>("inputTextEdit") != nullptr);
            REQUIRE(window.findChild<QTextBrowser*>("outputTextBrowser") != nullptr);
            REQUIRE(window.findChild<QListWidget*>("variantsList") != nullptr);
            
            auto inputField = window.findChild<QPlainTextEdit*>("inputTextEdit");
            auto outputField = window.findChild<QTextBrowser*>("outputTextBrowser");
            auto variantsList = window.findChild<QListWidget*>("variantsList");
            
            CHECK(inputField->toPlainText().isEmpty());
            CHECK(outputField->toPlainText().isEmpty());
            CHECK(variantsList->count() == 0);
            CHECK(inputField->placeholderText() == "Введите текст для перевода...");
            CHECK(outputField->isReadOnly());
            CHECK_FALSE(variantsList->isEnabled());
        }
        
        SUBCASE("Buttons initialization") {
            REQUIRE(window.findChild<QPushButton*>("translateButton") != nullptr);
            REQUIRE(window.findChild<QPushButton*>("clearButton") != nullptr);
            
            auto translateBtn = window.findChild<QPushButton*>("translateButton");
            auto clearBtn = window.findChild<QPushButton*>("clearButton");
            
            CHECK(translateBtn->text() == "Перевести");
            CHECK(clearBtn->text() == "Очистить");
        }
    }
    
    TEST_CASE("Language Detection") {
        int argc = 1;
        char *argv[] = {(char*)"test"};
        QApplication app(argc, argv);
        
        MainWindow window;
        
        SUBCASE("Russian text detection") {
            QString russianText = "Привет, мир!";
            CHECK(window.detectLanguage(russianText) == "Русский");
        }
        
        SUBCASE("English text detection") {
            QString englishText = "Hello, world!";
            CHECK(window.detectLanguage(englishText) == "Английский");
        }
        
        SUBCASE("Mixed text detection") {
            QString mixedText = "Hello, мир!";
            WARN(window.detectLanguage(mixedText) == "Неизвестный");
        }
        
        SUBCASE("Empty text detection") {
            QString emptyText = "";
            WARN(window.detectLanguage(emptyText) == "Неизвестный");
        }
    }
    
    TEST_CASE("Input Validation") {
        int argc = 1;
        char *argv[] = {(char*)"test"};
        QApplication app(argc, argv);
        
        MainWindow window;
        
        SUBCASE("Valid input validation") {
            QString validText = "Hello";
            QString validLang = "Русский";
            CHECK(window.validateInput(validText, validLang));
        }
        
        SUBCASE("Empty text validation") {
            QString emptyText = "";
            QString validLang = "Русский";
            CHECK_FALSE(window.validateInput(emptyText, validLang));
        }
        
        SUBCASE("Long text validation") {
            QString longText = QString(501, 'a');
            QString validLang = "Русский";
            CHECK_FALSE(window.validateInput(longText, validLang));
        }
        
        SUBCASE("Empty language validation") {
            QString validText = "Hello";
            QString emptyLang = "";
            CHECK_FALSE(window.validateInput(validText, emptyLang));
        }
    }
    
    TEST_CASE("Clear Functionality") {
        int argc = 1;
        char *argv[] = {(char*)"test"};
        QApplication app(argc, argv);
        
        MainWindow window;
        
        SUBCASE("Clear button clears all fields") {
            auto inputField = window.findChild<QPlainTextEdit*>("inputTextEdit");
            auto outputField = window.findChild<QTextBrowser*>("outputTextBrowser");
            auto variantsList = window.findChild<QListWidget*>("variantsList");
            auto sourceCombo = window.findChild<QComboBox*>("sourceLangCombo");
            
            inputField->setPlainText("Test text");
            outputField->setPlainText("Translated text");
            variantsList->addItem("Alternative translation");
            sourceCombo->setCurrentIndex(1);
            
            QTest::mouseClick(window.findChild<QPushButton*>("clearButton"), Qt::LeftButton);
            
            CHECK(inputField->toPlainText().isEmpty());
            CHECK(outputField->toPlainText().isEmpty());
            CHECK(variantsList->count() == 0);
            CHECK(sourceCombo->currentIndex() == 0);
        }
    }
    
    TEST_CASE("Translation Output") {
        int argc = 1;
        char *argv[] = {(char*)"test"};
        QApplication app(argc, argv);
        
        MainWindow window;
        
        SUBCASE("Translation output in GUI-only mode") {
            auto inputField = window.findChild<QPlainTextEdit*>("inputTextEdit");
            auto outputField = window.findChild<QTextBrowser*>("outputTextBrowser");
            auto variantsList = window.findChild<QListWidget*>("variantsList");
            auto sourceCombo = window.findChild<QComboBox*>("sourceLangCombo");
            auto targetCombo = window.findChild<QComboBox*>("targetLangCombo");
            
            inputField->setPlainText("Hello");
            sourceCombo->setCurrentIndex(2);
            targetCombo->setCurrentIndex(0);
            
            QTimer::singleShot(100, [&]() {
                auto msgBox = qApp->activeModalWidget();
                if (msgBox && msgBox->inherits("QMessageBox")) {
                    QTest::keyClick(msgBox, Qt::Key_Enter);
                }
            });
            
            QTest::mouseClick(window.findChild<QPushButton*>("translateButton"), Qt::LeftButton);
            
            CHECK(outputField->toPlainText() == "Translation functionality is disabled in GUI-only mode");
            CHECK(variantsList->count() == 1);
            if (variantsList->count() > 0) {
                CHECK(variantsList->item(0)->text() == "Translation functionality is disabled in GUI-only mode");
            }
        }
    }
}