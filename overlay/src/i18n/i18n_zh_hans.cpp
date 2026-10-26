/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "i18n.h"

namespace alefbet::pctrl::i18n {

/*! @brief Simplified Chinese on-screen strings (keys are the English source strings). */
const StringMap& chineseSimplified() {
    static const StringMap table = {
        // Common frame titles / subtitles
        {"Parental Control", "家长控制"},
        {"Administration", "管理"},
        {"Debug options", "调试选项"},
        {"Usage history", "使用记录"},
        {"Set limits", "设置限制"},
        {"Setup PIN", "设置 PIN"},
        {"Verify PIN", "验证 PIN"},
        {"Set limits for ", "设置限制："},
        // Main menu
        {"Parental Control is Enabled", "家长控制已启用"},
        {"Parental Control is Disabled", "家长控制已禁用"},
        {"Parental Control is not installed", "家长控制未安装"},
        {"Database tampered!", "数据库已被篡改！"},
        {"Database needs upgrade!", "数据库需要升级！"},
        {"User: ", "用户："},
        {"Playing: ", "正在游玩："},
        {"Usage: ", "已使用："},
        {"Bedtime: ", "就寝时间："},
        {"Play from: ", "游戏开始："},
        {"Remaining: ", "剩余："},
        {"No user / app started", "无用户或应用运行"},
        {"Debug", "调试"},
        {"Admin", "管理员"},
        // Admin menu
        {"State", "状态"},
        {"Enabled", "启用"},
        {"Disabled", "禁用"},
        {"Working mode", "工作模式"},
        {"Blocking", "拦截"},
        {"Notify remaining time", "通知剩余时间"},
        {"Log level", "日志级别"},
        {"DEBUG", "DEBUG"},
        {"INFO", "INFO"},
        {"Set PIN", "设置 PIN"},
        {"Versions", "版本"},
        {"Overlay", "Overlay"},
        {"Sysmodule", "Sysmodule"},
        // Debug menu
        {"Run test", "运行测试"},
        {"Current user: ", "当前用户："},
        {"Current title: ", "当前游戏："},
        // Usage history
        {"Choose a user", "选择用户"},
        {"Today", "今天"},
        {"Usage", "已使用"},
        {"Remaining", "剩余"},
        // Setup limits
        {"Please use \u2190 and \u2192 to move.", "请使用 \u2190 和 \u2192 移动。"},
        {"Modify the limits with \u2191 and \u2193.", "请使用 \u2191 和 \u2193 修改限制。"},
        {"Play limits", "游戏限制"},
        {"Daily", "每日"},
        {"mn", "分钟"},
        {"h", "小时"},
        {"Playtime window", "游戏时段"},
        {"Set time to 0h 0mn to disable.", "将时间设为 0小时0分钟以禁用。"},
        {"Allow games from", "游戏开始"},
        {"Stop games after", "游戏结束"},
        // PIN
        {"Please enter the new PIN", "请输入新 PIN"},
        {"New PIN", "新 PIN"},
        {"PINs do not match", "两次输入的 PIN 不一致"},
        {"unknown error", "未知错误"},
        {"Press A to retry, B to cancel", "按 A 重试，按 B 取消"},
        {"Please enter Admin PIN", "请输入管理员 PIN"},
        {"Admin PIN", "管理员 PIN"},
        {"PIN is incorrect", "PIN 不正确"}
    };

    return table;
}

}
