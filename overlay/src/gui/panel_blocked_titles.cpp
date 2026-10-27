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

#include "panel_blocked_titles.h"
#include <switch.h>
#include <algorithm>
#include "logger.h"
#include "helpers/ipc_helpers.h"
#include "helpers/switch_helpers.h"
#include "i18n/i18n.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::helpers;
using namespace alefbet::pctrl::i18n;

/*! @brief Constructs a BlockedTitlesUsersPanel instance. */
BlockedTitlesUsersPanel::BlockedTitlesUsersPanel() {
}

/*! @brief Destructs a BlockedTitlesUsersPanel instance. */
BlockedTitlesUsersPanel::~BlockedTitlesUsersPanel() {
}

/*! @brief Creates and returns the root UI element of the user selection panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* BlockedTitlesUsersPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame(tr("Parental Control"), tr("Blocked games"));
    rootList_ = new tsl::elm::List();

    rebuildUI();

    return rootFrame_;
}

/*! @brief Rebuilds the user selection list. */
void BlockedTitlesUsersPanel::rebuildUI() {
    rootList_->addItem(new tsl::elm::CategoryHeader(tr("Choose a user")));

    const auto& users = helpers::getUsersList();

    for(const auto& user: users) {
        auto entryUser = new tsl::elm::ListItem(user.nickname);
        rootList_->addItem(entryUser);
        entryUser->setClickListener([user](u64 keys) {
            if(keys & HidNpadButton_A) {
                tsl::changeTo<BlockedTitlesUserPanel>(user);
                return true;
            }

            return false;
        });
    }

    rootFrame_->setContent(rootList_);
}

/*! @brief Updates the panel state each frame. */
void BlockedTitlesUsersPanel::update() {
}

/*! @brief Handles user input not captured by other UI elements.
 * @return true if the input was handled, false otherwise. */
bool BlockedTitlesUsersPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}

/*! @brief Constructs a BlockedTitlesUserPanel for the given user.
 * @param user The user whose blocked games are set. */
BlockedTitlesUserPanel::BlockedTitlesUserPanel(const UserData& user) {
    user_ = user;
}

/*! @brief Destructs a BlockedTitlesUserPanel instance. */
BlockedTitlesUserPanel::~BlockedTitlesUserPanel() {
}

/*! @brief Creates and returns the root UI element of the blocked games panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* BlockedTitlesUserPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame(tr("Parental Control"), std::string(tr("Blocked games for ")) + user_.nickname);
    rootList_ = new tsl::elm::List();

    rebuildUI();

    return rootFrame_;
}

/*! @brief Rebuilds the list of installed games with their blocked state.
 *
 * Each toggle blocks or unblocks the game immediately through IPC. */
void BlockedTitlesUserPanel::rebuildUI() {
    const auto& installedTitles = helpers::getInstalledTitles();
    const auto& blockedTitles = ipc::getBlockedTitles(user_);

    if(installedTitles.empty()) {
        rootList_->addItem(new tsl::elm::ListItem(tr("No game installed")));
    }

    const UserData user = user_;
    for(const auto& title: installedTitles) {
        const bool blocked = std::find(blockedTitles.begin(), blockedTitles.end(), title.titleId) != blockedTitles.end();
        auto entryTitle = new tsl::elm::ToggleListItem(title.name, blocked, tr("Blocked"), tr("Allowed"));
        rootList_->addItem(entryTitle);

        const u64 titleId = title.titleId;
        entryTitle->setStateChangedListener([entryTitle, user, titleId](bool state) -> bool {
            if(!ipc::setTitleBlocked(user, titleId, state)) {
                entryTitle->setState(!state);
            }

            return true;
        });
    }

    rootFrame_->setContent(rootList_);
}

/*! @brief Updates the panel state each frame. */
void BlockedTitlesUserPanel::update() {
}

/*! @brief Handles user input not captured by other UI elements.
 * @return true if the input was handled, false otherwise. */
bool BlockedTitlesUserPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}
