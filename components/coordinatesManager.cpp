#include "mainwindow.h"

// 1. 仅添加设备的函数
bool MainWindow::addDevice(const QString &MAC, const DeviceType &type) {
    // 检查MAC是否已存在
    if (_device_config.deviceMap.contains(MAC)) {
        qWarning() << "设备MAC已存在:" << MAC;
        return false;
    }

    // 创建新设备信息
    DeviceInfo device;
    device.MAC = MAC;
    device.type = type;
    // coordinates留空，后续可以通过其他函数分配

    // 添加到配置中
    _device_config.devices.append(device);
    _device_config.deviceMap.insert(MAC, &_device_config.devices.last());

    qDebug() << "成功添加设备 - MAC:" << MAC << ", Type:" << type;
    return true;
}

// 初始化坐标缓存（在构造函数或初始化时调用）
void MainWindow::initializeCoordinatesCache() {
    _typeCoordinatesCache.clear();

    // 获取所有设备类型
    QSet<DeviceType> types;
    for (const auto &device : _device_config.devices) {
        types.insert(device.type);
    }

    // 为每种类型生成并缓存坐标
    for (const auto &type : types) {
        _typeCoordinatesCache[type] = generateAllCoordinatesForType(type);
    }
}

// 根据设备类型生成所有可能的坐标
QList<int> MainWindow::generateAllCoordinatesForType(const DeviceType &type) {
    QList<int> coordinates;

    if (type == DeviceType_B53) {
        // B53: 111-115, 121-125, 131-135, 211-215, 221-225, ..., 831-835
        for (int row = 1; row <= 8; row++) {
            // 8行
            for (int group = 1; group <= 3; group++) {
                // 每行3组
                for (int slot = 1; slot <= 5; slot++) {
                    // 每组5个位置
                    int coord = row * 100 + group * 10 + slot;
                    coordinates.append(coord);
                }
            }
        }
    } else if (type == DeviceType_A42) {
        // A43: 11-13, 21-23, 31-33, 41-43
        for (int row = 1; row <= 4; row++) {
            // 4行
            for (int slot = 1; slot <= 3; slot++) {
                // 每行3个位置
                int coord = row * 10 + slot;
                coordinates.append(coord);
            }
        }
    } else if (type == DeviceType_A21) {
        // A2: 11, 12, 21, 22
        for (int row = 1; row <= 2; row++) {
            // 2行
            for (int slot = 1; slot <= 2; slot++) {
                // 每行2个位置
                int coord = row * 10 + slot;
                coordinates.append(coord);
            }
        }
    }

    return coordinates;
}

// 获取特定类型的已使用坐标数
int MainWindow::getUsedCoordinatesCountForType(const DeviceType &type) {
    int usedCount = 0;

    for (const auto &device : _device_config.devices) {
        if (device.type == type) {
            usedCount += device.coordinates.size();
        }
    }

    return usedCount;
}

// 辅助函数：获取特定类型的设备数量
int MainWindow::getDeviceCountForType(const DeviceType &type) {
    int count = 0;
    for (const auto &device : _device_config.devices) {
        if (device.type == type) {
            count++;
        }
    }
    return count;
}

// 获取特定类型的所有可用坐标（返回所有设备的可用坐标列表）
QList<int> MainWindow::getAvailableCoordinatesForType(const DeviceType &type) {
    QList<int> availableCoords;

    if (!_typeCoordinatesCache.contains(type)) {
        _typeCoordinatesCache[type] = generateAllCoordinatesForType(type);
    }

    const QList<int> &allCoords = _typeCoordinatesCache[type];

    // 遍历该类型的所有设备
    for (const auto &device : _device_config.devices) {
        if (device.type == type) {
            // 将已使用的坐标转换为Set
            QSet<int> usedCoords(device.coordinates.begin(), device.coordinates.end());

            // 找出该设备未使用的坐标
            for (const auto &coord : allCoords) {
                if (!usedCoords.contains(coord)) {
                    availableCoords.append(coord);
                }
            }
        }
    }

    return availableCoords;
}

// 为指定类型分配坐标（可指定偏好坐标）
QPair<QString, int> MainWindow::allocateCoordinateForType(const DeviceType &type) {
    QPair<QString, int> result; // <MAC, coordinate>

    for (DeviceInfo &device : _device_config.devices) {
        if (device.type != type) {
            continue;
        }
        if (device.coordinates.size() >= device.coordinates_total) {
            continue; // 该设备已满
        }
        QList<int> Coordinates_All = _typeCoordinatesCache[type];
        if (type == DeviceType_B53) {
            //清除大于B53_N*100的坐标
            Coordinates_All.erase(std::remove_if(Coordinates_All.begin(), Coordinates_All.end(),
                                                 [device](int coord) { return coord > (device.B53_N+1) * 100; }),
                                  Coordinates_All.end());
        }

        // 找到第一个未使用的坐标
        for (const int &coord : Coordinates_All) {
            if (!device.coordinates.contains(coord)) {
                // 找到一个未使用的坐标
                device.coordinates.append(coord);
                result.first = device.MAC;
                result.second = coord;
                qDebug() << "Allocated coordinate:" << coord
                         << "on device:" << device.MAC
                         << "of type:" << type;
                saveDeviceConfig();
                return result; // 成功分配
            }
        }
    }

    // 如果没有设备可以分配坐标，返回空结果
    qDebug() << "No available coordinates for type:" << type;
    return result;
}

// 为指定类型分配下一个可用坐标
QPair<QString, int> MainWindow::allocateNextAvailableCoordinateForType(const DeviceType &type) {
    return allocateCoordinateForType(type);
}

// 释放指定类型的坐标（需要遍历所有该类型设备）
bool MainWindow::releaseCoordinate(const DeviceType &type, const int &coordinate) {
    for (auto &device : _device_config.devices) {
        if (device.type == type) {
            int index = device.coordinates.indexOf(coordinate);
            if (index != -1) {
                device.coordinates.remove(index);
                qDebug() << "Released coordinate:" << coordinate
                    << "from device:" << device.MAC
                    << "of type:" << type;
                return true;
            }
        }
    }

    qDebug() << "Coordinate not found:" << coordinate << "for type:" << type;
    return false;
}

// 通过MAC释放坐标
bool MainWindow::releaseCoordinateByMAC(const QString &MAC, const int &coordinate) {
    if (!_device_config.deviceMap.contains(MAC)) {
        qDebug() << "Device not found:" << MAC;
        return false;
    }

    DeviceInfo *device = _device_config.deviceMap[MAC];
    int index = device->coordinates.indexOf(coordinate);
    if (index == -1) {
        qDebug() << "Coordinate not found:" << coordinate << "for device:" << MAC;
        return false;
    }

    device->coordinates.remove(index);
    qDebug() << "Released coordinate:" << coordinate << "from device:" << MAC;
    return true;
}

// 获取指定类型的所有设备及其坐标使用情况
QVector<QPair<QString, int> > MainWindow::getDeviceUsageForType(const DeviceType &type) {
    QVector<QPair<QString, int> > usage;

    for (const auto &device : _device_config.devices) {
        if (device.type == type) {
            usage.append(qMakePair(device.MAC, device.coordinates.size()));
        }
    }

    return usage;
}
int MainWindow::getTotalCoordinatesCountForType(DeviceType type) {
    return _device_config.typeCoordinateTotal[type];
}
void MainWindow::updateTypeCoordinateTotal() {
    _device_config.typeCoordinateTotal.clear();
    _device_config.typeCoordinateTotal[DeviceType_B53] = 0;
    _device_config.typeCoordinateTotal[DeviceType_A42] = 0;
    _device_config.typeCoordinateTotal[DeviceType_A21] = 0;
    for (auto &device : _device_config.devices) {
        if (device.type == DeviceType_B53) {
            _device_config.typeCoordinateTotal[DeviceType_B53] += 15 * device.B53_N;
            device.coordinates_total = 15 * device.B53_N;
        } else if (device.type == DeviceType_A42) {
            _device_config.typeCoordinateTotal[DeviceType_A42] += 8;
            device.coordinates_total = 8;
        } else if (device.type == DeviceType_A21) {
            _device_config.typeCoordinateTotal[DeviceType_A21] += 2;
            device.coordinates_total = 2;
        }
    }
}
// 更新类型统计信息
void MainWindow::updateTypeStatistics() {
    QHash<DeviceType, QPair<int, int> > stats = getAllTypeStatistics();

    qDebug() << "=== Type Coordinate Statistics ===";
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        DeviceType type = it.key();
        int used = it.value().first;
        int total = it.value().second;
        int available = total - used;
        double usageRate = total > 0 ? (double) used / total * 100.0 : 0.0;

        qDebug() << QString("Type %1: Used %2/%3 (%4%), Available %5")
            .arg(type)
            .arg(used)
            .arg(total)
            .arg(usageRate)
            .arg(available);

        // 显示该类型各设备的使用情况
        QVector<QPair<QString, int> > deviceUsage = getDeviceUsageForType(type);
        for (const auto &du : deviceUsage) {
            int deviceTotal = getTotalCoordinatesCountForType(type);
            double deviceRate = deviceTotal > 0 ? (double) du.second / deviceTotal * 100.0 : 0.0;
            qDebug() << QString("  - Device %1: %2/%3 (%4%)")
                .arg(du.first)
                .arg(du.second)
                .arg(deviceTotal)
                .arg(deviceRate);
        }
    }
}

// 获取所有类型的统计信息(包含已使用的信息)
QHash<DeviceType, QPair<int, int> > MainWindow::getAllTypeStatistics() {
    QHash<DeviceType, QPair<int, int> > stats; // type -> (used, total)

    stats[DeviceType_B53] = qMakePair(0, 0);
    stats[DeviceType_A42] = qMakePair(0, 0);
    stats[DeviceType_A21] = qMakePair(0, 0);
    // 按类型统计
    for (const auto &device : _device_config.devices) {
        stats[device.type].second += device.coordinates_total;
        stats[device.type].first += device.coordinates.size();
    }

    return stats;
}

// 批量分配坐标
QVector<QPair<QString, int> > MainWindow::allocateMultipleCoordinatesForType(const DeviceType &type, int count) {
    QVector<QPair<QString, int> > allocated;

    for (int i = 0; i < count; i++) {
        QPair<QString, int> result = allocateNextAvailableCoordinateForType(type);
        if (!result.first.isEmpty()) {
            allocated.append(result);
        } else {
            qDebug() << "Could only allocate" << i << "coordinates out of" << count << "requested";
            break;
        }
    }

    return allocated;
}

// 查找指定坐标在哪个设备上
QString MainWindow::findDeviceByCoordinate(const DeviceType &type, const int &coordinate) {
    for (const auto &device : _device_config.devices) {
        if (device.type == type && device.coordinates.contains(coordinate)) {
            return device.MAC;
        }
    }
    return QString();
}
