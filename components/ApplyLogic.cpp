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
    }
}

void MainWindow::UpdateApplyReturnUI() {
    //仅处理归还所有和无字提示的逻辑，其他逻辑在UpdateApplyLogic
    if (ApplyComponentNum==0) {
        _return_ALLButton->setDisabled(true);
    }
    else {
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
