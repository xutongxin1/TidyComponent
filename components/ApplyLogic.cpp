#include "mainwindow.h"
//
// Created by xtx on 25-5-20.
//

void MainWindow::UpdateApplyLogic(const component_record_struct &record) {
    //进这个逻辑必然有东西选中
    _noReturnTips->hide();
    if (record.isApply) {
        _returnButton->show();
        _applyButton->hide();
        _apply_LightButton->hide();
        _apply_Light_VoiceButton->hide();
    } else {
        _returnButton->hide();
        _applyButton->show();
        _apply_LightButton->show();
        _apply_Light_VoiceButton->show();
        if (isConnectedToMesh) {
            _applyButton->setEnabled(true);
            _apply_LightButton->setEnabled(true);
            _apply_Light_VoiceButton->setEnabled(true);
            _applyButton->setToolTip("");
            _apply_LightButton->setToolTip("");
            _apply_Light_VoiceButton->setToolTip("");
            disconnect(_applyButton, &ElaToolButton::clicked, this, nullptr);
            disconnect(_apply_LightButton, &ElaToolButton::clicked, this, nullptr);
            disconnect(_apply_Light_VoiceButton, &ElaToolButton::clicked, this, nullptr);
            connect(_applyButton, &ElaToolButton::clicked, this, [=] {
                ApplyComponent(record, apply_type_normal); //申请逻辑
            });
            connect(_applyButton, &ElaToolButton::clicked, this, [=] {
                ApplyComponent(record, apply_type_light); //申请逻辑
            });
            connect(_applyButton, &ElaToolButton::clicked, this, [=] {
                ApplyComponent(record, apply_type_voice); //申请逻辑
            });
        } else {
            _applyButton->setEnabled(false);
            _apply_LightButton->setEnabled(false);
            _apply_Light_VoiceButton->setEnabled(false);
            _applyButton->setToolTip("请先插入用户侧完成连接");
            _apply_LightButton->setToolTip("请先插入用户侧完成连接");
            _apply_Light_VoiceButton->setToolTip("请先插入用户侧完成连接");
        }
        //TODO: 申请逻辑
    }
}

void MainWindow::UpdateApplyReturnUI() {
    //仅处理归还所有和无字提示的逻辑，其他逻辑在UpdateApplyLogic
    if (ApplyComponentNum == 0) {
        _return_ALLButton->setDisabled(true);
    } else {
        _return_ALLButton->setDisabled(false);
    }
}
///初始化时调用
void MainWindow::InitApplyReturnUI() {
    _return_ALLButton->hide();
    _returnButton->hide();
    _applyButton->hide();
    _apply_LightButton->hide();
    _apply_Light_VoiceButton->hide();
}

void MainWindow::ApplyComponent(const component_record_struct &record, apply_type apply_type) {
}
