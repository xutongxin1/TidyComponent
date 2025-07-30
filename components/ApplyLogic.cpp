#include "mainwindow.h"
//
// Created by xtx on 25-5-20.
//

void MainWindow::UpdateApplyLogic() {
    QModelIndexList selectedIndexes = tableView->selectionModel()->selection().indexes();
    if (selectedIndexes.isEmpty()) {
        _returnTipsB53->hide();
        _applyButton->hide();
        _apply_LightButton->hide();
        _apply_Light_VoiceButton->hide();
        _delComponentButton->setToolTip("请先选择一个元器件");
        _delComponentButton->setEnabled(false);

        return;
    }

    if (const QString cid = selectedIndexes[0].sibling(selectedIndexes[0].row(), 4).data(Qt::DisplayRole).toString()
        ; model->
        component_record_Hash_cid.contains(cid)) {
        UpdateApplyLogic(model->component_record_Hash_cid.value(cid));
    }
}

void MainWindow::UpdateApplyLogic(component_record_struct *record) {
    //进这个逻辑必然有东西选中
    _noReturnTips->hide();
    _applyButton->show();
    _apply_LightButton->show();
    _apply_Light_VoiceButton->show();
    //逻辑分解：在取出状态下，在申请归还状态下
    if (record->isApply == ComponentState_OUT) {
        _delComponentButton->setToolTip("");
        _delComponentButton->setEnabled(true);
        disconnect(_delComponentButton, &ElaToolButton::clicked, this, nullptr);
        connect(_delComponentButton, &ElaToolButton::clicked, this, [=] {
            delComponentLogic(record); //删除逻辑
        });
        //B53不可以在取出状态下申请
        if (record->device_type == DeviceType_B53) {
            _returnTipsB53->show(); //提示B53归还方法
            _applyButton->setEnabled(false);
            _apply_LightButton->setEnabled(false);
            _apply_Light_VoiceButton->setEnabled(false);
            _applyButton->setToolTip("对于B53元器件，请归还器件后再申请");
            _apply_LightButton->setToolTip("对于B53元器件，请归还器件后再申请");
            _apply_Light_VoiceButton->setToolTip("对于B53元器件，请归还器件后再申请");
        } else {
            _returnTipsB53->hide();
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
                ApplyComponentIN(record, apply_type_normal, LED_MODE_STATIC); //申请逻辑
            });
            connect(_apply_LightButton, &ElaToolButton::clicked, this, [=] {
                ApplyComponentIN(record, apply_type_light, LED_MODE_STATIC); //申请逻辑
            });
            connect(_apply_Light_VoiceButton, &ElaToolButton::clicked, this, [=] {
                ApplyComponentIN(record, apply_type_voice, LED_MODE_STATIC); //申请逻辑
            });
        }
    } else if (record->isApply == ComponentState_APPLYIN) {
        _returnTipsB53->hide();
        _applyButton->setEnabled(false);
        _apply_LightButton->setEnabled(false);
        _apply_Light_VoiceButton->setEnabled(false);
        _applyButton->setToolTip("请归还器件后再申请");
        _apply_LightButton->setToolTip("请归还器件后再申请");
        _apply_Light_VoiceButton->setToolTip("请归还器件后再申请");
        _delComponentButton->setEnabled(false);
        _delComponentButton->setToolTip("请先取出元器件再删除");
    } else if (record->isApply == ComponentState_APPLYOUT) {
        _returnTipsB53->hide();
        _applyButton->setEnabled(false);
        _apply_LightButton->setEnabled(false);
        _apply_Light_VoiceButton->setEnabled(false);
        _applyButton->setToolTip("正在申请取出，请勿重复申请");
        _apply_LightButton->setToolTip("正在申请取出，请勿重复申请");
        _apply_Light_VoiceButton->setToolTip("正在申请取出，请勿重复申请");
        _delComponentButton->setEnabled(false);
        _delComponentButton->setToolTip("请先取出元器件再删除");
    } else {
        _delComponentButton->setEnabled(false);
        _delComponentButton->setToolTip("请先取出元器件再删除");
        _returnTipsB53->hide();
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
                ApplyComponentOUT(record, apply_type_normal, LED_MODE_STATIC); //申请逻辑
            });
            connect(_apply_LightButton, &ElaToolButton::clicked, this, [=] {
                ApplyComponentOUT(record, apply_type_light, LED_MODE_STATIC); //申请逻辑
            });
            connect(_apply_Light_VoiceButton, &ElaToolButton::clicked, this, [=] {
                ApplyComponentOUT(record, apply_type_voice, LED_MODE_STATIC); //申请逻辑
            });
        } else {
            _applyButton->setEnabled(false);
            _apply_LightButton->setEnabled(false);
            _apply_Light_VoiceButton->setEnabled(false);
            _applyButton->setToolTip("请先插入用户侧完成连接");
            _apply_LightButton->setToolTip("请先插入用户侧完成连接");
            _apply_Light_VoiceButton->setToolTip("请先插入用户侧完成连接");
        }
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
    _returnTipsB53->hide();
    _applyButton->hide();
    _apply_LightButton->hide();
    _apply_Light_VoiceButton->hide();
}

void MainWindow::ApplyComponentOUT(component_record_struct *record, apply_type apply_type, led_mode_t led_mode) {
    QColor color = colorAllocator->allocateColor(LED_MODE_STATIC);
    record->color = color.name();
    model->updateColumnWithRoles(0);

    QString tmp = record->MAC + " " + QString::number(record->coordinate) + " " + color.name() + " " +
        QString::number(led_mode);
    //TODO:分配颜色方式w
    QString group;
    if (record->device_type == DeviceType_B53) {
        group = "C301 ";
    } else if (record->device_type == DeviceType_A42) {
        group = "C101 ";
    } else if (record->device_type == DeviceType_A21) {
        group = "C201 ";
    }

    if (apply_type == apply_type_normal) {
        tmp = group + QString::number(10) + " " + tmp;
    } else if (apply_type == apply_type_light) {
        tmp = group + QString::number(11) + " " + tmp;
    } else if (apply_type == apply_type_voice) {
        tmp = group + QString::number(12) + " " + tmp;
    }
    if (serialManager->writeData(tmp)) {
    }
    record->isApply = ComponentState_APPLYOUT;
    UpdateApplyLogic(record); //刷新申请逻辑
}
void MainWindow::ApplyComponentIN(component_record_struct *record, apply_type apply_type, led_mode_t led_mode) {
    //TODO:灯状态还没想的很清楚
    QColor color = colorAllocator->allocateColor(LED_MODE_FLASH_FAST_3);
    record->color = color.name();
    model->updateColumnWithRoles(0);

    QString tmp = record->MAC + " " + QString::number(record->coordinate) + " " + color.name() + " " +
        QString::number(led_mode);
    //TODO:分配颜色方
    QString group;
    if (record->device_type == DeviceType_B53) {
        group = "C301 ";
    } else if (record->device_type == DeviceType_A42) {
        group = "C101 ";
    } else if (record->device_type == DeviceType_A21) {
        group = "C201 ";
    }

    if (apply_type == apply_type_normal) {
        tmp = group + QString::number(20) + " " + tmp;
    } else if (apply_type == apply_type_light) {
        tmp = group + QString::number(21) + " " + tmp;
    } else if (apply_type == apply_type_voice) {
        tmp = group + QString::number(22) + " " + tmp;
    }
    if (serialManager->writeData(tmp)) {
    }
    record->isApply = ComponentState_APPLYIN;
    UpdateApplyLogic(record); //刷新申请逻辑
}
void MainWindow::ApplyComponentIN_AddingCompnent(component_record_struct *record) {
    QString tmp = record->MAC + " " + QString::number(record->coordinate) + " #0000FF " + QString::number(
        LED_MODE_FLASH_FAST_1);
    //TODO:分配颜色方

    if (record->device_type == DeviceType_B53) {
        tmp = "C301 " + QString::number(30) + " " + tmp;
    } else if (record->device_type == DeviceType_A42) {
        tmp = "C101 " + QString::number(30) + " " + tmp;
    } else if (record->device_type == DeviceType_A21) {
        tmp = "C201 " + QString::number(30) + " " + tmp;
    }

    if (serialManager->writeData(tmp)) {
    }
    record->isApply = ComponentState_APPLYIN;
}
