#ifndef REGISTER_H
#define REGISTER_H

#pragma once

#include <QDialog>
#include <QEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QMovie>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QScreen>
#include <QGuiApplication>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "DisplayAnimation.h"
#include "SocketBusiness.h"
namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();
    int maskWidth() const;
    void setMaskWidth(int width);

private slots:
    void onRegisterSuccess();
    void onRegisterFailed(const QString& reason);
    void onRegisterButtonClicked();
    void onReturnButtonClicked();
    void on_closeToolButton_clicked();

signals:
    void registerSuccess();
    void backToLogin();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent  *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::Register *ui;
    QPoint mDragPos;
    bool mDragging = false;
    QLabel *gifLabel = nullptr;
    QMovie *movie = nullptr;
    DisplayAnimation *display;
};

#endif // REGISTER_H
